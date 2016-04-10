#include "Target/Satisfy.h"
#include "Target/Liveness.h"
#include "Target/RegAlloc.h"
#include <assert.h>
#include <stdio.h>

// =============================
// Remap register to accumulator
// =============================

// Return an instruction to move the contents of a register to an
// accumulator, and change the use of that register in the given
// instruction to the given accumulator.

Instr remapAToAccum(Instr* instr, RegId acc)
{
  assert(instr->ALU.srcA.tag == REG);

  AssignCond always;
  always.tag = ALWAYS;

  Instr move;
  move.tag                   = ALU;
  move.ALU.setFlags          = false;
  move.ALU.cond              = always;
  move.ALU.dest.tag          = ACC;
  move.ALU.dest.regId        = acc;
  move.ALU.srcA.tag          = REG;
  move.ALU.srcA.reg          = instr->ALU.srcA.reg;
  move.ALU.op                = A_BOR;
  move.ALU.srcB.tag          = REG;
  move.ALU.srcB.reg          = instr->ALU.srcA.reg;

  instr->ALU.srcA.reg.tag    = ACC;
  instr->ALU.srcA.reg.regId  = acc;

  return move;
}

Instr remapBToAccum(Instr* instr, RegId acc)
{
  assert(instr->ALU.srcB.tag == REG);

  AssignCond always;
  always.tag = ALWAYS;

  Instr move;
  move.tag                   = ALU;
  move.ALU.setFlags          = false;
  move.ALU.cond              = always;
  move.ALU.dest.tag          = ACC;
  move.ALU.dest.regId        = acc;
  move.ALU.srcA.tag          = REG;
  move.ALU.srcA.reg          = instr->ALU.srcB.reg;
  move.ALU.op                = A_BOR;
  move.ALU.srcB.tag          = REG;
  move.ALU.srcB.reg          = instr->ALU.srcB.reg;

  instr->ALU.srcB.reg.tag   = ACC;
  instr->ALU.srcB.reg.regId = acc;

  return move;
}

// ==============================
// Resolve register file conflict
// ==============================

// Determine reg file of given register.

RegTag regFileOf(Reg r)
{
  if (r.tag == REG_A) return REG_A;
  if (r.tag == REG_B) return REG_B;
  if (r.tag == SPECIAL) {
    if (r.regId == SPECIAL_ELEM_NUM) return REG_A;
    if (r.regId == SPECIAL_QPU_NUM) return REG_B;
    if (r.regId == SPECIAL_DMA_LD_WAIT) return REG_A;
    if (r.regId == SPECIAL_DMA_ST_WAIT) return REG_B;
  }
  return NONE;
}

// When an instruction uses two (different) registers that are mapped
// to the same register file, then remap one of them to an
// accumulator.

bool resolveRegFileConflict(Instr* instr, Instr* newInstr)
{
  if (instr->tag == ALU && instr->ALU.srcA.tag == REG
                        && instr->ALU.srcB.tag == REG) {
    int rfa = regFileOf(instr->ALU.srcA.reg);
    int rfb = regFileOf(instr->ALU.srcB.reg);
    if (rfa != NONE && rfb != NONE) {
      bool conflict = rfa == rfb && instr->ALU.srcA.reg.regId !=
                                    instr->ALU.srcB.reg.regId;
      if (conflict) {
        *newInstr = remapAToAccum(instr, 0);
        return true;
      }
    }
  }
  return false;
}

// =============================
// Satisfy VideoCore constraints
// =============================

// Transform an instruction sequence to satisfy various VideoCore
// constraints, including:
//
//   1. fill branch delay slots with NOPs;
//
//   2. introduce accumulators for operands mapped to the same
//      register file;
//
//   3. introduce accumulators for horizontal rotation operands;
//
//   4. insert NOPs to account for data hazards: a destination
//      register (assuming it's not an accumulator) cannot be read by
//      the next instruction.

// First pass: insert move-to-accumulator instructions.

static void insertMoves(Seq<Instr>* instrs, Seq<Instr>* newInstrs)
{
  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    RegId r; RegTag rt;

    if (instr.tag == ALU && instr.ALU.op == M_ROTATE) {
      // Insert moves for horizontal rotate operations
      newInstrs->append(remapAToAccum(&instr, 0));
      if (instr.ALU.srcB.tag == REG)
        newInstrs->append(remapBToAccum(&instr, 5));
      newInstrs->append(nop());
    }
    else if (instr.tag == ALU && instr.ALU.srcA.tag == IMM &&
             instr.ALU.srcB.tag == REG &&
             regFileOf(instr.ALU.srcB.reg) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs->append(remapBToAccum(&instr, 0));
    }
    else if (instr.tag == ALU && instr.ALU.srcB.tag == IMM &&
             instr.ALU.srcA.tag == REG &&
             regFileOf(instr.ALU.srcA.reg) == REG_B) {
      // Insert moves for an operation with a small immediate whose
      // register operand must reside in reg file B.
      newInstrs->append(remapAToAccum(&instr, 0));
    }
    else {
      // Insert moves for operands that are mapped to the same reg file
      Instr move;
      if (resolveRegFileConflict(&instr, &move))
        newInstrs->append(move);
    }
    
    // Put current instruction into the new sequence
    newInstrs->append(instr);
  }
}

// Second pass: insert NOPs
static void insertNops(Seq<Instr>* instrs, Seq<Instr>* newInstrs)
{
  // Use/def sets
  UseDefReg mySet, prevSet;

  // Previous instruction
  Instr prev = nop();

  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    RegId r; RegTag rt;

    // Insert NOPs to avoid data hazards
    useDefReg(prev, &prevSet);
    useDefReg(instr, &mySet);
    for (int j = 0; j < prevSet.def.numElems; j++) {
      Reg defReg = prevSet.def.elems[j];
      bool needNop = defReg.tag == REG_A || defReg.tag == REG_B;
      if (needNop && mySet.use.member(defReg)) {
        newInstrs->append(nop());
        break;
      }
    }

    // Put current instruction into the new sequence
    newInstrs->append(instr);

    // Insert NOPs in branch delay slots
    if (instr.tag == BRL || instr.tag == END) {
      for (int j = 0; j < 3; j++)
        newInstrs->append(nop());
      prev = nop();
    }

    // Update previous instruction
    if (instr.tag != LAB) prev = instr;
  }

}

// Combine passes

void satisfy(Seq<Instr>* instrs)
{
  // New instruction sequence
  Seq<Instr> newInstrs(instrs->numElems * 2);

  // Apply passes
  insertMoves(instrs, &newInstrs);
  instrs->clear();
  insertNops(&newInstrs, instrs);
}

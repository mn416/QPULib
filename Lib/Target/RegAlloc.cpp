#include <stdio.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/RegAlloc.h"
#include "Target/Subst.h"
#include "Target/Liveness.h"

// ============================================================================
// Accumulator allocation
// ============================================================================

// This is a simple peephole optimisation, captured by the following
// rewrite rule:
//
//   i:  x <- f(...)
//   j:  g(..., x, ...)
// 
// ===> if x not live-out of j
// 
//   i:  acc <- f(...)
//   j:  g(..., acc, ...)

void introduceAccum(CFG* cfg, Liveness* live, Seq<Instr>* instrs)
{
  UseDef useDefPrev, useDefCurrent;
  LiveSet liveOut;

  Reg acc;
  acc.tag = ACC;
  acc.regId = 1;

  for (int i = 1; i < instrs->numElems; i++) {
    Instr prev  = instrs->elems[i-1];
    Instr instr = instrs->elems[i];

    // Compute vars defined by prev
    useDef(prev, &useDefPrev);

    if (useDefPrev.def.numElems > 0) {
      RegId def = useDefPrev.def.elems[0];

      // Compute vars used by instr
      useDef(instr, &useDefCurrent);

      // Compute vars live-out of instr
      computeLiveOut(cfg, live, i, &liveOut);

      // Check that write is non-conditional
      bool always = (prev.tag == LI && prev.LI.cond.tag == ALWAYS)
                 || (prev.tag == ALU && prev.ALU.cond.tag == ALWAYS);

      if (always &&
          useDefCurrent.use.member(def)  &&
          !liveOut.member(def)) {
        renameDest(&prev, REG_A, def, ACC, 1);
        renameUses(&instr, REG_A, def, ACC, 1);
        instrs->elems[i-1] = prev;
        instrs->elems[i]   = instr;
      }
    }
  }
}

// ============================================================================
// Register allocation
// ============================================================================

void regAlloc(CFG* cfg, Seq<Instr>* instrs)
{
  // Step 0
  // Perform liveness analysis
  Liveness live;
  liveness(instrs, cfg, &live);

  // Optimisation pass that introduces accumulators
  introduceAccum(cfg, &live, instrs);

  // Step 1
  // For each variable, determine a preference for register file A or B.
  int n = getFreshVarCount();
  int* prefA = new int [n];
  int* prefB = new int [n];
  UseDef useDefSet;
  for (int i = 0; i < n; i++) prefA[i] = prefB[i] = 0;

  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    Reg ra, rb;
    if (getTwoUses(instr, &ra, &rb) && ra.tag == REG_A && rb.tag == REG_A) {
      RegId x = ra.regId;
      RegId y = rb.regId;
      if (prefA[x] > prefA[y] || prefB[y] > prefB[x])
        { prefA[x]++; prefB[y]++; }
      else
        { prefA[y]++; prefB[x]++; }
    }
    else if (instr.tag == ALU &&
             instr.ALU.srcA.tag == REG &&
             instr.ALU.srcA.reg.tag == REG_A &&
             instr.ALU.srcB.tag == IMM) {
      prefA[instr.ALU.srcA.reg.regId]++;
    }
    else if (instr.tag == ALU &&
             instr.ALU.srcB.tag == REG &&
             instr.ALU.srcB.reg.tag == REG_A &&
             instr.ALU.srcA.tag == IMM) {
      prefA[instr.ALU.srcB.reg.regId]++;
    }
  }

  // Step 2
  // For each variable, determine all variables ever live at same time
  LiveSet* liveWith = new LiveSet [n];
  LiveSet liveOut;
  for (int i = 0; i < instrs->numElems; i++) {
    computeLiveOut(cfg, &live, i, &liveOut);
    useDef(instrs->elems[i], &useDefSet);
    for (int j = 0; j < liveOut.numElems; j++) {
      RegId rx = liveOut.elems[j];
      for (int k = 0; k < liveOut.numElems; k++) {
        RegId ry = liveOut.elems[k];
        if (rx != ry) liveWith[rx].insert(ry);
      }
      for (int k = 0; k < useDefSet.def.numElems; k++) {
        RegId rd = useDefSet.def.elems[k];
        if (rd != rx) {
          liveWith[rx].insert(rd);
          liveWith[rd].insert(rx);
        }
      }
    }
  }

  // Step 3
  // Allocate a register to each variable
  RegTag prevChosenRegFile = REG_B;
  Reg* alloc = new Reg [n];
  for (int i = 0; i < n; i++) alloc[i].tag = NONE;

  const int NUM_REGS = 32;
  bool possibleA[NUM_REGS];
  bool possibleB[NUM_REGS];

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < NUM_REGS; j++)
      possibleA[j] = possibleB[j] = true;

    // Eliminate impossible choices of register for this variable
    LiveSet* set = &liveWith[i];
    for (int j = 0; j < set->numElems; j++) {
      Reg neighbour = alloc[set->elems[j]];
      if (neighbour.tag == REG_A) possibleA[neighbour.regId] = false;
      if (neighbour.tag == REG_B) possibleB[neighbour.regId] = false;
    }

    // Find possible register in each register file
    RegId chosenA = -1;
    RegId chosenB = -1;
    for (int j = 0; j < NUM_REGS; j++)
      if (possibleA[j]) { chosenA = j; break; }
    for (int j = 0; j < NUM_REGS; j++)
      if (possibleB[j]) { chosenB = j; break; }

    // Choose a register file
    RegTag chosenRegFile;
    if (chosenA < 0 && chosenB < 0) {
      printf("QPULib: register allocation failed, insufficient capacity\n");
      exit(EXIT_FAILURE);
    }
    else if (chosenA < 0) chosenRegFile = REG_B;
    else if (chosenB < 0) chosenRegFile = REG_A;
    else {
      if (prefA[i] > prefB[i]) chosenRegFile = REG_A;
      else if (prefA[i] < prefB[i]) chosenRegFile = REG_B;
      else chosenRegFile = prevChosenRegFile == REG_A ? REG_B : REG_A;
    }
    prevChosenRegFile = chosenRegFile;

    // Finally, allocate a register to the variable
    alloc[i].tag = chosenRegFile;
    alloc[i].regId = chosenRegFile == REG_A ? chosenA : chosenB;
  }

  // Step 4
  // Apply the allocation to the code
  for (int i = 0; i < instrs->numElems; i++) {
    useDef(instrs->elems[i], &useDefSet);
    Instr* instr = &instrs->elems[i];
    for (int j = 0; j < useDefSet.def.numElems; j++) {
      RegId r = useDefSet.def.elems[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;
      renameDest(instr, REG_A, r, tmp, alloc[r].regId);
    }
    for (int j = 0; j < useDefSet.use.numElems; j++) {
      RegId r = useDefSet.use.elems[j];
      RegTag tmp = alloc[r].tag == REG_A ? TMP_A : TMP_B;
      renameUses(instr, REG_A, r, tmp, alloc[r].regId);
    }
    substRegTag(instr, TMP_A, REG_A);
    substRegTag(instr, TMP_B, REG_B);
  }

  // Free memory
  delete [] prefA;
  delete [] prefB;
  delete [] liveWith;
}




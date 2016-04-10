#include "Target/Encode.h"
#include "Target/Satisfy.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ===========
// ALU opcodes
// ===========

uint32_t encodeAddOp(ALUOp op)
{
  switch (op) {
    case NOP:       return 0;
    case A_FADD:    return 1;
    case A_FSUB:    return 2;
    case A_FMIN:    return 3;
    case A_FMAX:    return 4;
    case A_FMINABS: return 5;
    case A_FMAXABS: return 6;
    case A_FtoI:    return 7;
    case A_ItoF:    return 8;
    case A_ADD:     return 12;
    case A_SUB:     return 13;
    case A_SHR:     return 14;
    case A_ASR:     return 15;
    case A_ROR:     return 16;
    case A_SHL:     return 17;
    case A_MIN:     return 18;
    case A_MAX:     return 19;
    case A_BAND:    return 20;
    case A_BOR:     return 21;
    case A_BXOR:    return 22;
    case A_BNOT:    return 23;
    case A_CLZ:     return 24;
    case A_V8ADDS:  return 30;
    case A_V8SUBS:  return 31;
  }
  fprintf(stderr, "QPULib: unknown add op\n");
  exit(EXIT_FAILURE);
}

uint32_t encodeMulOp(ALUOp op)
{
  switch (op) {
    case NOP:      return 0;
    case M_FMUL:   return 1;
    case M_MUL24:  return 2;
    case M_V8MUL:  return 3;
    case M_V8MIN:  return 4;
    case M_V8MAX:  return 5;
    case M_V8ADDS: return 6;
    case M_V8SUBS: return 7;
  }
  fprintf(stderr, "QPULib: unknown mul op\n");
  exit(EXIT_FAILURE);
}

// ===============
// Condition flags
// ===============

uint32_t encodeAssignCond(AssignCond cond)
{
  switch (cond.tag) {
    case NEVER:  return 0;
    case ALWAYS: return 1;
    case FLAG:
      switch (cond.flag) {
        case ZS: return 2;
        case ZC: return 3;
        case NS: return 4;
        case NC: return 5;
     }
  }
  fprintf(stderr, "QPULib: missing case in encodeAssignCond\n");
  exit(EXIT_FAILURE);
}

// =================
// Branch conditions
// =================

uint32_t encodeBranchCond(BranchCond cond)
{
  switch (cond.tag) {
    case COND_NEVER:
      fprintf(stderr, "QPULib: 'never' condition not supported\n");
      exit(EXIT_FAILURE);
    case COND_ALWAYS: return 15;
    case COND_ALL:
      switch (cond.flag) {
        case ZS: return 0;
        case ZC: return 1;
        case NS: return 4;
        case NC: return 5;
        default: break;
      }
    case COND_ANY:
      switch (cond.flag) {
        case ZS: return 2;
        case ZC: return 3;
        case NS: return 6;
        case NC: return 7;
        default: break;
      }
  }
  fprintf(stderr, "QPULib: missing case in encodeBranchCond\n");
  exit(EXIT_FAILURE);
}

// ================
// Register encoder
// ================

uint32_t encodeDestReg(Reg reg, RegTag* file)
{
  switch (reg.tag) {
    case REG_A:
      assert(reg.regId >= 0 && reg.regId < 32);
      *file = REG_A; return reg.regId;
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32);
      *file = REG_B; return reg.regId;
    case ACC:
      assert(reg.regId >= 0 && reg.regId <= 5);
      *file = reg.regId == 5 ? REG_B : REG_A; 
      return 32 + reg.regId;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_RD_SETUP:    *file = REG_A; return 49;
        case SPECIAL_WR_SETUP:    *file = REG_B; return 49;
        case SPECIAL_DMA_LD_ADDR: *file = REG_A; return 50;
        case SPECIAL_DMA_ST_ADDR: *file = REG_B; return 50;
        case SPECIAL_VPM_WRITE:   *file = REG_A; return 48;
        case SPECIAL_HOST_INT:    *file = REG_A; return 38;
        case SPECIAL_TMU0_S:      *file = REG_A; return 56;
        default:                  break;
      }
    case NONE: *file = REG_A; return 39;
  }
  fprintf(stderr, "QPULib: missing case in encodeDestReg\n");
  exit(EXIT_FAILURE);
}

uint32_t encodeSrcReg(Reg reg, RegTag file, uint32_t* mux)
{
  assert (file == REG_A || file == REG_B);
  switch (reg.tag) {
    case REG_A:
      assert(reg.regId >= 0 && reg.regId < 32 && file == REG_A);
      *mux = 6; return reg.regId;
    case REG_B:
      assert(reg.regId >= 0 && reg.regId < 32 && file == REG_B);
      *mux = 7; return reg.regId;
    case ACC:
      assert(reg.regId >= 0 && reg.regId <= 4);
      *mux = reg.regId; return 0;
    case NONE:
      *mux = file == REG_A ? 6 : 7; return 39;
    case SPECIAL:
      switch (reg.regId) {
        case SPECIAL_UNIFORM:  *mux = file == REG_A ? 6 : 7; return 32;
        case SPECIAL_ELEM_NUM: assert(file == REG_A); *mux = 6; return 38;
        case SPECIAL_QPU_NUM:  assert(file == REG_B); *mux = 7; return 38;
        case SPECIAL_VPM_READ: *mux = file == REG_A ? 6 : 7; return 48;
        case SPECIAL_DMA_LD_WAIT:
          assert(file == REG_A); *mux = 6; return 50;
        case SPECIAL_DMA_ST_WAIT:
          assert(file == REG_B); *mux = 7; return 50;
      }
  }
  fprintf(stderr, "QPULib: missing case in encodeSrcReg\n");
  exit(EXIT_FAILURE);
}

// ===================
// Instruction encoder
// ===================

void encodeInstr(Instr instr, uint32_t* high, uint32_t* low)
{
  // Convert intermediate instruction into core instruction
  switch (instr.tag) {
    case IRQ:
      instr.tag           = LI;
      instr.LI.setFlags   = false;
      instr.LI.cond.tag   = ALWAYS;
      instr.LI.dest.tag   = SPECIAL;
      instr.LI.dest.regId = SPECIAL_HOST_INT;
      instr.LI.imm.tag    = IMM_INT32;
      instr.LI.imm.intVal = 1;
      break;
    case LD2:
    case ST3: {
      RegId src = instr.tag == LD2 ? SPECIAL_DMA_LD_WAIT :
                  SPECIAL_DMA_ST_WAIT;
      instr.tag                   = ALU;
      instr.ALU.setFlags          = false;
      instr.ALU.cond.tag          = NEVER;
      instr.ALU.op                = A_BOR;
      instr.ALU.dest.tag          = NONE;
      instr.ALU.srcA.tag          = REG;
      instr.ALU.srcA.reg.tag      = SPECIAL;
      instr.ALU.srcA.reg.regId    = src;
      instr.ALU.srcB.tag          = REG;
      instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
      break;
    }
    case LD4: {
      Reg dest = instr.LD4.dest;
      instr.tag                   = ALU;
      instr.ALU.setFlags          = false;
      instr.ALU.cond.tag          = ALWAYS;
      instr.ALU.op                = A_BOR;
      instr.ALU.dest              = dest;
      instr.ALU.srcA.tag          = REG;
      instr.ALU.srcA.reg.tag      = SPECIAL;
      instr.ALU.srcA.reg.regId    = SPECIAL_VPM_READ;
      instr.ALU.srcB.tag          = REG;
      instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
      break;
    }
    case ST1: {
      Reg src = instr.ST1.data;
      instr.tag                   = ALU;
      instr.ALU.setFlags          = false;
      instr.ALU.cond.tag          = ALWAYS;
      instr.ALU.op                = A_BOR;
      instr.ALU.dest.tag          = SPECIAL;
      instr.ALU.dest.regId        = SPECIAL_VPM_WRITE;
      instr.ALU.srcA.tag          = REG;
      instr.ALU.srcA.reg          = src;
      instr.ALU.srcB.tag          = REG;
      instr.ALU.srcB.reg          = src;
      break;
    }
  }

  // Encode core instrcution
  switch (instr.tag) {
    // Load immediate
    case LI: {
      RegTag file;
      uint32_t cond = encodeAssignCond(instr.LI.cond) << 17;
      uint32_t waddr_add = encodeDestReg(instr.LI.dest, &file) << 6;
      uint32_t waddr_mul = 39;
      uint32_t ws   = (file == REG_A ? 0 : 1) << 12;
      uint32_t sf   = (instr.LI.setFlags ? 1 : 0) << 13;
      *high         = 0xe0000000 | cond | ws | sf | waddr_add | waddr_mul;
      *low          = (uint32_t) instr.LI.imm.intVal;
      return;
    }

    // Branch
    case BR: {
      // Register offset not yet supported
      assert(! instr.BR.target.useRegOffset);

      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t cond = encodeBranchCond(instr.BR.cond) << 20;
      uint32_t rel  = (instr.BR.target.relative ? 1 : 0) << 19;
      *high = 0xf0000000 | cond | rel | waddr_add | waddr_mul;
      *low  = (uint32_t) 8*instr.BR.target.immOffset;
      return;
    }

    // ALU
    case ALU: {
      RegTag file;
      bool isMul     = isMulOp(instr.ALU.op);
      bool hasImm    = instr.ALU.srcA.tag == IMM || instr.ALU.srcB.tag == IMM;
      bool isRot     = instr.ALU.op == M_ROTATE;
      uint32_t sig   = ((hasImm || isRot) ? 13 : 1) << 28;
      uint32_t cond  = encodeAssignCond(instr.ALU.cond) << (isMul ? 14 : 17);
      uint32_t dest  = encodeDestReg(instr.ALU.dest, &file);
      uint32_t waddr_add, waddr_mul, ws;
      if (isMul) {
        waddr_add = 39 << 6;
        waddr_mul = dest;
        ws        = (file == REG_B ? 0 : 1) << 12;
      }
      else {
        waddr_add = dest << 6;
        waddr_mul = 39;
        ws        = (file == REG_A ? 0 : 1) << 12;
      }
      uint32_t sf    = (instr.ALU.setFlags ? 1 : 0) << 13;
      *high          = sig | cond | ws | sf | waddr_add | waddr_mul;

      if (instr.ALU.op == M_ROTATE) {
        assert(instr.ALU.srcA.tag == REG && instr.ALU.srcA.reg.tag == ACC &&
               instr.ALU.srcA.reg.regId == 0);
        assert(instr.ALU.srcB.tag == REG ?
               instr.ALU.srcB.reg.tag == ACC && instr.ALU.srcA.reg.regId == 5
               : true);
        uint32_t mulOp = encodeMulOp(M_V8MIN) << 29;
        uint32_t raddrb;
        if (instr.ALU.srcB.tag == REG) {
          raddrb = 48;
        }
        else {
          uint32_t n = (uint32_t) instr.ALU.srcB.smallImm.val;
          assert(n >= 1 || n <= 15);
          raddrb = 48 + n;
        }
        uint32_t raddra = 39;
        *low = mulOp | (raddrb << 12) | (raddra << 18);
        return;
      }
      else {
        uint32_t mulOp = (isMul ? encodeMulOp(instr.ALU.op) : 0) << 29;
        uint32_t addOp = (isMul ? 0 : encodeAddOp(instr.ALU.op)) << 24;

        uint32_t muxa, muxb;
        uint32_t raddra, raddrb;

        // Both operands are registers
        if (instr.ALU.srcA.tag == REG && instr.ALU.srcB.tag == REG) {
          RegTag aFile = regFileOf(instr.ALU.srcA.reg);
          RegTag bFile = regFileOf(instr.ALU.srcB.reg);
          RegTag aTag  = instr.ALU.srcA.reg.tag;
          RegTag bTag  = instr.ALU.srcB.reg.tag;

          // If operands are the same register
          if (aTag != NONE && aTag == bTag &&
                instr.ALU.srcA.reg.regId == instr.ALU.srcB.reg.regId) {
            if (aFile == REG_A) {
              raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
              muxb = muxa; raddrb = 39;
            }
            else {
              raddrb = encodeSrcReg(instr.ALU.srcA.reg, REG_B, &muxa);
              muxb = muxa; raddra = 39;
            }
          }
          else {
            // Operands are different registers
            assert(aFile == NONE || bFile == NONE || aFile != bFile);
            if (aFile == REG_A || bFile == REG_B) {
              raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
              raddrb = encodeSrcReg(instr.ALU.srcB.reg, REG_B, &muxb);
            }
            else {
              raddrb = encodeSrcReg(instr.ALU.srcA.reg, REG_B, &muxa);
              raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
            }
          }
        }
        else if (instr.ALU.srcB.tag == IMM) {
          // Second operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcA.reg, REG_A, &muxa);
          raddrb = (uint32_t) instr.ALU.srcB.smallImm.val;
          muxb   = 7;
        }
        else if (instr.ALU.srcA.tag == IMM) {
          // First operand is a small immediate
          raddra = encodeSrcReg(instr.ALU.srcB.reg, REG_A, &muxb);
          raddrb = (uint32_t) instr.ALU.srcA.smallImm.val;
          muxa   = 7;
        }
        else {
          // Both operands are small immediates
          assert(false);
        }
        *low = mulOp | addOp | (raddra << 18) | (raddrb << 12)
                     | (muxa << 9) | (muxb << 6)
                     | (muxa << 3) | muxb;
        return;
      }
    }

    // Halt
    case END:
    case TMU0_TO_ACC4: {
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t raddra = 39 << 18;
      uint32_t raddrb = 39 << 12;
      uint32_t sig = instr.tag == END ? 0x30000000 : 0xa0000000;
      *high  = sig | waddr_add | waddr_mul;
      *low   = raddra | raddrb;
      return;
    }

    // Semaphore increment/decrement
    case SINC:
    case SDEC: {
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      uint32_t sig = 0xe8000000;
      uint32_t incOrDec = (instr.tag == SINC ? 0 : 1) << 4;
      *high = sig | waddr_add | waddr_mul;
      *low = incOrDec | instr.semaId;
      return;
    }

    // No-op & print instructions (ignored)
    case NO_OP:
    case PRI:
    case PRS:
    case PRF:
      uint32_t waddr_add = 39 << 6;
      uint32_t waddr_mul = 39;
      *high  = 0xe0000000 | waddr_add | waddr_mul;
      *low   = 0;
      return;
  }

  fprintf(stderr, "QPULib: missing case in encodeInstr\n");
  exit(EXIT_FAILURE);
}

// =================
// Top-level encoder
// =================

void encode(Seq<Instr>* instrs, Seq<uint32_t>* code)
{
  uint32_t high, low;
  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    encodeInstr(instr, &high, &low);
    code->append(low);
    code->append(high);
  }
}

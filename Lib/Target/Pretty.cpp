#include "Target/Syntax.h"
#include "Target/SmallLiteral.h"

void pretty(SubWord sw)
{
  switch (sw) {
    case A8:  printf("[7:0]"); return;
    case B8:  printf("[15:8]"); return;
    case C8:  printf("[23:16]"); return;
    case D8:  printf("[31:24]"); return;
    case A16: printf("[15:0]"); return;
    case B16: printf("[31:16]"); return;
  }
}

const char* specialStr(RegId rid)
{
  Special s = (Special) rid;
  switch (s) {
    case SPECIAL_UNIFORM:      return "UNIFORM";
    case SPECIAL_ELEM_NUM:     return "ELEM_NUM";
    case SPECIAL_QPU_NUM:      return "QPU_NUM";
    case SPECIAL_RD_SETUP:     return "RD_SETUP";
    case SPECIAL_WR_SETUP:     return "WR_SETUP";
    case SPECIAL_DMA_ST_ADDR:  return "DMA_ST_ADDR";
    case SPECIAL_DMA_LD_ADDR:  return "DMA_LD_ADDR";
    case SPECIAL_VPM_READ:     return "VPM_READ";
    case SPECIAL_VPM_WRITE:    return "VPM_WRITE";
    case SPECIAL_HOST_INT:     return "HOST_INT";
    case SPECIAL_TMU0_S:       return "TMU0_S";
  }

  // Unreachable
  assert(false);
}

void pretty(Reg r)
{
  switch (r.tag) {
    case REG_A:
      printf("A%i", r.regId);
      return;
    case REG_B: printf("B%i", r.regId); return;
    case ACC: printf("ACC%i", r.regId); return;
    case SPECIAL: printf("S[%s]", specialStr(r.regId)); return;
    case NONE: printf("_"); return;
  }
}

void pretty(Flag flag)
{
  switch (flag) {
    case ZS: printf("ZS"); return;
    case ZC: printf("ZC"); return;
    case NS: printf("NS"); return;
    case NC: printf("NC"); return;
  }
}

void pretty(BranchCond cond)
{
  switch (cond.tag) {
    case COND_ALL:
      printf("all(");
      pretty(cond.flag);
      printf(")");
      return;
    case COND_ANY:
      printf("any(");
      pretty(cond.flag);
      printf(")");
      return;
    case COND_ALWAYS:
      printf("always");
      return;
    case COND_NEVER:
      printf("never");
      return;
  }
}

void pretty(AssignCond cond)
{
  switch (cond.tag) {
    case ALWAYS: printf("always"); return;
    case NEVER: printf("never"); return;
    case FLAG: pretty(cond.flag); return;
  }
}

void pretty(Imm imm) {
  switch (imm.tag) {
    case IMM_INT32:
      printf("%d", imm.intVal);
      return;
    case IMM_FLOAT32:
      printf("%f", imm.floatVal);
      return;
    case IMM_MASK:
      int b = imm.intVal;
      for (int i = 0; i < 16; i++) {
        printf("%i", b&1 ? 1 : 0);
        b >>= 1;
      }
      return;
  }
}

void pretty(SmallImm imm)
{
  switch (imm.tag) {
    case SMALL_IMM: printSmallLit(imm.val); return;
    case ROT_ACC: printf("ROT(ACC5)"); return;
    case ROT_IMM: printf("ROT(%i)", imm.val); return;
  }
}

void pretty(RegOrImm r)
{
  switch (r.tag) {
    case REG: pretty(r.reg); return;
    case IMM: pretty(r.smallImm); return;
  }
}

void pretty(ALUOp op)
{
  switch (op) {
    case NOP:       printf("nop"); return;
    case A_FADD:    printf("addf"); return;
    case A_FSUB:    printf("subf"); return;
    case A_FMIN:    printf("minf"); return;
    case A_FMAX:    printf("maxf"); return;
    case A_FMINABS: printf("minabsf"); return;
    case A_FMAXABS: printf("maxabsf"); return;
    case A_FtoI:    printf("ftoi"); return;
    case A_ItoF:    printf("itof"); return;
    case A_ADD:     printf("add"); return;
    case A_SUB:     printf("sub"); return;
    case A_SHR:     printf("shr"); return;
    case A_ASR:     printf("asr"); return;
    case A_ROR:     printf("ror"); return;
    case A_SHL:     printf("shl"); return;
    case A_MIN:     printf("min"); return;
    case A_MAX:     printf("max"); return;
    case A_BAND:    printf("and"); return;
    case A_BOR:     printf("or"); return;
    case A_BXOR:    printf("xor"); return;
    case A_BNOT:    printf("not"); return;
    case A_CLZ:     printf("clz"); return;
    case A_V8ADDS:  printf("addsatb"); return;
    case A_V8SUBS:  printf("subsatb"); return;
    case M_FMUL:    printf("mulf"); return;
    case M_MUL24:   printf("mul24"); return;
    case M_V8MUL:   printf("mulb"); return;
    case M_V8MIN:   printf("minb"); return;
    case M_V8MAX:   printf("maxb"); return;
    case M_V8ADDS:  printf("m_addsatb"); return;
    case M_V8SUBS:  printf("m_subsatb"); return;
    case M_ROTATE:  printf("rotate"); return;
  }
}

void pretty(BranchTarget target)
{
  if (target.relative)
    printf("PC+1+");
  if (target.useRegOffset)
    printf("A%i+", target.regOffset);
  printf("%i", target.immOffset);
}

void pretty(BufferAorB buffer)
{
  if (buffer == A) printf("A");
  if (buffer == B) printf("B");
}

void pretty(Instr instr)
{
  switch (instr.tag) {
    case LI:
      if (instr.LI.cond.tag != ALWAYS) {
        printf("where ");
        pretty(instr.LI.cond);
        printf(": ");
      }
      pretty(instr.LI.dest);
      printf(" <-%s ", instr.LI.setFlags ? "{sf}" : "");
      pretty(instr.LI.imm);
      printf("\n");
      return;
    case ALU:
      if (instr.ALU.cond.tag != ALWAYS) {
        printf("where ");
        pretty(instr.ALU.cond);
        printf(": ");
      }
      pretty(instr.ALU.dest);
      printf(" <-%s ", instr.ALU.setFlags ? "{sf}" : "");
      pretty(instr.ALU.op);
      printf("(");
      pretty(instr.ALU.srcA);
      printf(", ");
      pretty(instr.ALU.srcB);
      printf(")\n");
      return;
    case END:
      printf("END\n");
      return;
    case BR:
      printf("if ");
      pretty(instr.BR.cond);
      printf(" goto ");
      pretty(instr.BR.target);
      printf("\n");
      return;
    case BRL:
      printf("if ");
      pretty(instr.BRL.cond);
      printf(" goto L%i\n", instr.BRL.label);
      return;
    case LAB:
      printf("L%i:\n", instr.label);
      return;
    case NO_OP:
      printf("NOP\n");
      return;
    case LD1:
      pretty(instr.LD1.buffer);
      printf(" <- LD1(");
      pretty(instr.LD1.addr);
      printf(")\n");
      return;
    case LD2:
      printf("LD2\n");
      return;
    case LD3:
      printf("LD3(");
      pretty(instr.LD3.buffer);
      printf(")\n");
      return;
    case LD4:
      pretty(instr.LD4.dest);
      printf(" <- LD4\n");
      return;
    case ST1:
      printf("ST1(");
      pretty(instr.ST1.buffer);
      printf(") <- ");
      pretty(instr.ST1.data);
      printf("\n");
      return;
    case ST2:
      printf("ST2(");
      pretty(instr.ST2.buffer);
      printf(", ");
      pretty(instr.ST2.addr);
      printf(")\n");
      return;
    case ST3:
      printf("ST3\n");
      return;
    case PRS:
      printf("PRS(\"%s\")", instr.PRS);
      return;
    case PRI:
      printf("PRI(");
      pretty(instr.PRI);
      printf(")\n");
      return;
    case PRF:
      printf("PRF(");
      pretty(instr.PRF);
      printf(")\n");
      return;
    case RECV:
      printf("RECV(");
      pretty(instr.RECV.dest);
      printf(")\n");
      return;
    case TMU0_TO_ACC4:
      printf("TMU0_TO_ACC4\n");
      return;
    case SINC:
      printf("SINC %i\n", instr.semaId);
      return;
    case SDEC:
      printf("SDEC %i\n", instr.semaId);
      return;
    case IRQ:
      printf("IRQ\n");
      return;
  }
}

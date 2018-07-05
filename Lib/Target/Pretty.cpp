#include "Target/Syntax.h"
#include "Target/SmallLiteral.h"

namespace QPULib {

#ifdef NOT_USED
void pretty(SubWord sw)
{
  switch (sw) {
    case A8:  return "[7:0]";
    case B8:  return "[15:8]";
    case C8:  return "[23:16]";
    case D8:  return "[31:24]";
    case A16: return "[15:0]";
    case B16: return "[31:16]";
    default:  assert(false); return "";
  }
}
#endif  // NOT_USED


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

void pretty(FILE *f, Reg r)
{
  switch (r.tag) {
    case REG_A:
      fprintf(f, "A%i", r.regId);
      return;
    case REG_B: fprintf(f, "B%i", r.regId); return;
    case ACC: fprintf(f, "ACC%i", r.regId); return;
    case SPECIAL: fprintf(f, "S[%s]", specialStr(r.regId)); return;
    case NONE: fprintf(f, "_"); return;
  }
}

void pretty(FILE *f, Flag flag)
{
  switch (flag) {
    case ZS: fprintf(f, "ZS"); return;
    case ZC: fprintf(f, "ZC"); return;
    case NS: fprintf(f, "NS"); return;
    case NC: fprintf(f, "NC"); return;
  }
}

void pretty(FILE *f, BranchCond cond)
{
  switch (cond.tag) {
    case COND_ALL:
      fprintf(f, "all(");
      pretty(f, cond.flag);
      fprintf(f, ")");
      return;
    case COND_ANY:
      fprintf(f, "any(");
      pretty(f, cond.flag);
      fprintf(f, ")");
      return;
    case COND_ALWAYS:
      fprintf(f, "always");
      return;
    case COND_NEVER:
      fprintf(f, "never");
      return;
  }
}

void pretty(FILE *f, AssignCond cond)
{
  switch (cond.tag) {
    case ALWAYS: fprintf(f, "always"); return;
    case NEVER: fprintf(f, "never"); return;
    case FLAG: pretty(f, cond.flag); return;
  }
}

void pretty(FILE *f, Imm imm) {
  switch (imm.tag) {
    case IMM_INT32:
      fprintf(f, "%d", imm.intVal);
      return;
    case IMM_FLOAT32:
      fprintf(f, "%f", imm.floatVal);
      return;
    case IMM_MASK:
      int b = imm.intVal;
      for (int i = 0; i < 16; i++) {
        fprintf(f, "%i", b&1 ? 1 : 0);
        b >>= 1;
      }
      return;
  }
}

void pretty(FILE *f, SmallImm imm)
{
  switch (imm.tag) {
    case SMALL_IMM: printSmallLit(f, imm.val); return;
    case ROT_ACC: fprintf(f, "ROT(ACC5)"); return;
    case ROT_IMM: fprintf(f, "ROT(%i)", imm.val); return;
  }
}

void pretty(FILE *f, RegOrImm r)
{
  switch (r.tag) {
    case REG: pretty(f, r.reg); return;
    case IMM: pretty(f, r.smallImm); return;
  }
}

void pretty(FILE *f, ALUOp op)
{
  switch (op) {
    case NOP:       fprintf(f, "nop"); return;
    case A_FADD:    fprintf(f, "addf"); return;
    case A_FSUB:    fprintf(f, "subf"); return;
    case A_FMIN:    fprintf(f, "minf"); return;
    case A_FMAX:    fprintf(f, "maxf"); return;
    case A_FMINABS: fprintf(f, "minabsf"); return;
    case A_FMAXABS: fprintf(f, "maxabsf"); return;
    case A_FtoI:    fprintf(f, "ftoi"); return;
    case A_ItoF:    fprintf(f, "itof"); return;
    case A_ADD:     fprintf(f, "add"); return;
    case A_SUB:     fprintf(f, "sub"); return;
    case A_SHR:     fprintf(f, "shr"); return;
    case A_ASR:     fprintf(f, "asr"); return;
    case A_ROR:     fprintf(f, "ror"); return;
    case A_SHL:     fprintf(f, "shl"); return;
    case A_MIN:     fprintf(f, "min"); return;
    case A_MAX:     fprintf(f, "max"); return;
    case A_BAND:    fprintf(f, "and"); return;
    case A_BOR:     fprintf(f, "or"); return;
    case A_BXOR:    fprintf(f, "xor"); return;
    case A_BNOT:    fprintf(f, "not"); return;
    case A_CLZ:     fprintf(f, "clz"); return;
    case A_V8ADDS:  fprintf(f, "addsatb"); return;
    case A_V8SUBS:  fprintf(f, "subsatb"); return;
    case M_FMUL:    fprintf(f, "mulf"); return;
    case M_MUL24:   fprintf(f, "mul24"); return;
    case M_V8MUL:   fprintf(f, "mulb"); return;
    case M_V8MIN:   fprintf(f, "minb"); return;
    case M_V8MAX:   fprintf(f, "maxb"); return;
    case M_V8ADDS:  fprintf(f, "m_addsatb"); return;
    case M_V8SUBS:  fprintf(f, "m_subsatb"); return;
    case M_ROTATE:  fprintf(f, "rotate"); return;
  }
}

void pretty(FILE *f, BranchTarget target)
{
  if (target.relative)
    fprintf(f, "PC+1+");
  if (target.useRegOffset)
    fprintf(f, "A%i+", target.regOffset);
  fprintf(f, "%i", target.immOffset);
}

void pretty(FILE *f, BufferAorB buffer)
{
  if (buffer == A) fprintf(f, "A");
  if (buffer == B) fprintf(f, "B");
}

void pretty(FILE *f, Instr instr)
{
  assert(f != nullptr);

  switch (instr.tag) {
    case LI:
      if (instr.LI.cond.tag != ALWAYS) {
        fprintf(f, "where ");
        pretty(f, instr.LI.cond);
        fprintf(f, ": ");
      }
      pretty(f, instr.LI.dest);
      fprintf(f, " <-%s ", instr.LI.setFlags ? "{sf}" : "");
      pretty(f, instr.LI.imm);
      fprintf(f, "\n");
      return;
    case ALU:
      if (instr.ALU.cond.tag != ALWAYS) {
        fprintf(f, "where ");
        pretty(f, instr.ALU.cond);
        fprintf(f, ": ");
      }
      pretty(f, instr.ALU.dest);
      fprintf(f, " <-%s ", instr.ALU.setFlags ? "{sf}" : "");
      pretty(f, instr.ALU.op);
      fprintf(f, "(");
      pretty(f, instr.ALU.srcA);
      fprintf(f, ", ");
      pretty(f, instr.ALU.srcB);
      fprintf(f, ")\n");
      return;
    case END:
      fprintf(f, "END\n");
      return;
    case BR:
      fprintf(f, "if ");
      pretty(f, instr.BR.cond);
      fprintf(f, " goto ");
      pretty(f, instr.BR.target);
      fprintf(f, "\n");
      return;
    case BRL:
      fprintf(f, "if ");
      pretty(f, instr.BRL.cond);
      fprintf(f, " goto L%i\n", instr.BRL.label);
      return;
    case LAB:
      fprintf(f, "L%i:\n", instr.label);
      return;
    case NO_OP:
      fprintf(f, "NOP\n");
      return;
    case LD1:
      pretty(f, instr.LD1.buffer);
      fprintf(f, " <- LD1(");
      pretty(f, instr.LD1.addr);
      fprintf(f, ")\n");
      return;
    case LD2:
      fprintf(f, "LD2\n");
      return;
    case LD3:
      fprintf(f, "LD3(");
      pretty(f, instr.LD3.buffer);
      fprintf(f, ")\n");
      return;
    case LD4:
      pretty(f, instr.LD4.dest);
      fprintf(f, " <- LD4\n");
      return;
    case ST1:
      fprintf(f, "ST1(");
      pretty(f, instr.ST1.buffer);
      fprintf(f, ") <- ");
      pretty(f, instr.ST1.data);
      fprintf(f, "\n");
      return;
    case ST2:
      fprintf(f, "ST2(");
      pretty(f, instr.ST2.buffer);
      fprintf(f, ", ");
      pretty(f, instr.ST2.addr);
      fprintf(f, ")\n");
      return;
    case ST3:
      fprintf(f, "ST3\n");
      return;
    case PRS:
      fprintf(f, "PRS(\"%s\")", instr.PRS);
      return;
    case PRI:
      fprintf(f, "PRI(");
      pretty(f, instr.PRI);
      fprintf(f, ")\n");
      return;
    case PRF:
      fprintf(f, "PRF(");
      pretty(f, instr.PRF);
      fprintf(f, ")\n");
      return;
    case RECV:
      fprintf(f, "RECV(");
      pretty(f, instr.RECV.dest);
      fprintf(f, ")\n");
      return;
    case TMU0_TO_ACC4:
      fprintf(f, "TMU0_TO_ACC4\n");
      return;
    case SINC:
      fprintf(f, "SINC %i\n", instr.semaId);
      return;
    case SDEC:
      fprintf(f, "SDEC %i\n", instr.semaId);
      return;
    case IRQ:
      fprintf(f, "IRQ\n");
      return;
  }
}

}  // namespace QPULib

#include "Target/Subst.h"

// Rename a destination register in an instruction
void renameDest(Instr* instr, RegTag vt, RegId v,
                              RegTag wt, RegId w)
{
  switch (instr->tag) {
    // Load immediate
    case LI:
      if (instr->LI.dest.tag == vt && instr->LI.dest.regId == v) {
        instr->LI.dest.tag = wt;
        instr->LI.dest.regId = w;
      }
      return;

    // ALU operation
    case ALU:
      if (instr->ALU.dest.tag == vt && instr->ALU.dest.regId == v) {
        instr->ALU.dest.tag = wt;
        instr->ALU.dest.regId = w;
      }
      return;

    // LD4 instruction
    case LD4:
      if (instr->LD4.dest.tag == vt && instr->LD4.dest.regId == v) {
        instr->LD4.dest.tag = wt;
        instr->LD4.dest.regId = w;
      }
      return;

    // RECV instruction
    case RECV:
      if (instr->RECV.dest.tag == vt && instr->RECV.dest.regId == v) {
        instr->RECV.dest.tag = wt;
        instr->RECV.dest.regId = w;
      }
      return;
  }
}

// Renamed a used register in an instruction
void renameUses(Instr* instr, RegTag vt, RegId v,
                              RegTag wt, RegId w)
{
  switch (instr->tag) {
    // ALU operation
    case ALU:
      if (instr->ALU.srcA.tag == REG && instr->ALU.srcA.reg.tag == vt &&
          instr->ALU.srcA.reg.regId == v) {
        instr->ALU.srcA.reg.tag = wt;
        instr->ALU.srcA.reg.regId = w;
      }

      if (instr->ALU.srcB.tag == REG && instr->ALU.srcB.reg.tag == vt &&
          instr->ALU.srcB.reg.regId == v) {
        instr->ALU.srcB.reg.tag = wt;
        instr->ALU.srcB.reg.regId = w;
      }
      return;

    // LD1 instruction
    case LD1:
      if (instr->LD1.addr.tag == vt && instr->LD1.addr.regId == v) {
        instr->LD1.addr.tag = wt;
        instr->LD1.addr.regId = w;
      }
      return;

    // ST1 instruction
    case ST1:
      if (instr->ST1.data.tag == vt && instr->ST1.data.regId == v) {
        instr->ST1.data.tag = wt;
        instr->ST1.data.regId = w;
      }
      return;

    // ST2 instruction
    case ST2:
      if (instr->ST2.addr.tag == vt && instr->ST2.addr.regId == v) {
        instr->ST2.addr.tag = wt;
        instr->ST2.addr.regId = w;
      }
      return;

    // Print integer instruction
    case PRI:
      if (instr->PRI.tag == vt && instr->PRI.regId == v) {
        instr->PRI.tag = wt;
        instr->PRI.regId = w;
      }
      return;

    // Print float instruction
    case PRF:
      if (instr->PRF.tag == vt && instr->PRF.regId == v) {
        instr->PRF.tag = wt;
        instr->PRF.regId = w;
      }
      return;
  }
}

// Globally change register tag vt to wt in given instruction
void substRegTag(Instr* instr, RegTag vt, RegTag wt)
{
  switch (instr->tag) {
    // Load immediate
    case LI:
      if (instr->LI.dest.tag == vt)
        instr->LI.dest.tag = wt;
      return;

    // ALU operation
    case ALU:
      if (instr->ALU.dest.tag == vt)
        instr->ALU.dest.tag = wt;
      if (instr->ALU.srcA.tag == REG && instr->ALU.srcA.reg.tag == vt)
        instr->ALU.srcA.reg.tag = wt;
      if (instr->ALU.srcB.tag == REG && instr->ALU.srcB.reg.tag == vt)
        instr->ALU.srcB.reg.tag = wt;
      return;

    // LD1 instruction
    case LD1:
      if (instr->LD1.addr.tag == vt)
        instr->LD1.addr.tag = wt;
      return;

    // LD4 instruction
    case LD4:
      if (instr->LD4.dest.tag == vt)
        instr->LD4.dest.tag = wt;
      return;

    // ST1 instruction
    case ST1:
      if (instr->ST1.data.tag == vt)
        instr->ST1.data.tag = wt;
      return;

    // ST2 instruction
    case ST2:
      if (instr->ST2.addr.tag == vt)
        instr->ST2.addr.tag = wt;
      return;

    // Print integer instruction
    case PRI:
      if (instr->PRI.tag == vt)
        instr->PRI.tag = wt;
      return;

    // Print float instruction
    case PRF:
      if (instr->PRF.tag == vt)
        instr->PRF.tag = wt;
      return;

    // RECV instruction
    case RECV:
      if (instr->RECV.dest.tag == vt)
        instr->RECV.dest.tag = wt;
      return;

  }
}

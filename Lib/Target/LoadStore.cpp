#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/LoadStore.h"

// =============================================================================
// Stride setup
// =============================================================================

// Generate instructions to set the read stride.

void genSetReadStride(Seq<Instr>* instrs, int stride)
{
  int pitch = (stride+1)*4;
  assert(pitch < 8192);
  int setup = 0x90000000 | pitch;
  Reg dst; dst.tag = REG_A; dst.regId = RSV_READ_STRIDE;
  Instr instr = genLI(dst, setup);
  instrs->append(instr);
}

void genSetReadStride(Seq<Instr>* instrs, Reg stride)
{
  Reg pitch = freshReg();
  Reg tmp = freshReg();
  instrs->append(genIncr(pitch, stride, 1));
  instrs->append(genLI(tmp, 0x90000000));
  instrs->append(genLShift(pitch, pitch, 2));

  Reg dst; dst.tag = REG_A; dst.regId = RSV_READ_STRIDE;
  instrs->append(genOR(dst, tmp, pitch));
}

// Generate instructions to set the write stride.

void genSetWriteStride(Seq<Instr>* instrs, int stride)
{
  int strideBytes = stride*4;
  assert(strideBytes < 8192);
  int setup = 0xc0010000 | strideBytes;
  Reg dst; dst.tag = REG_A; dst.regId = RSV_WRITE_STRIDE;
  Instr instr = genLI(dst, setup);
  instrs->append(instr);
}

void genSetWriteStride(Seq<Instr>* instrs, Reg stride)
{
  Reg tmp0 = freshReg();
  Reg tmp1 = freshReg();
  instrs->append(genLShift(tmp0, stride, 2));
  instrs->append(genLI(tmp1, 0xc0010000));

  Reg dst; dst.tag = REG_A; dst.regId = RSV_WRITE_STRIDE;
  instrs->append(genOR(dst, tmp0, tmp1));
}

// =============================================================================
// DMA setup
// =============================================================================

// Generate instructions to setup DMA load.

void assignDMALoadSetup(Seq<Instr>* instrs, Reg dst, BufferAorB b, Reg qpuId)
{
  int setup = 0x80101800;
  int buffIdx = (16 * (b == A ? 0 : 1)) << 4;
  setup |= buffIdx;

  Reg tmp = freshReg();
  instrs->append(genLI(tmp, setup));
  instrs->append(genOR(dst, qpuId, tmp));
}

// Generate instructions to setup DMA store.

void assignDMAStoreSetup(Seq<Instr>* instrs, Reg dst, BufferAorB b, Reg qpuId)
{
  int setup = 0x88014000;
  int buffIdx = (16 * (b == A ? 2 : 3)) << 7;
  setup |= buffIdx;

  Reg tmp0 = freshReg();
  instrs->append(genLI(tmp0, setup));

  Reg tmp1 = freshReg();
  instrs->append(genLShift(tmp1, qpuId, 3));

  instrs->append(genOR(dst, tmp0, tmp1));
}

// =============================================================================
// VPM setup
// =============================================================================

// Generate instructions to setup VPM load.

void assignVPMLoadSetup(Seq<Instr>* instrs, Reg dst, BufferAorB b, Reg qpuId)
{
  int setup = 0x00100200;
  int buffIdx = (b == A ? 0 : 1) << 4;
  setup |= buffIdx;

  Reg tmp = freshReg();
  instrs->append(genLI(tmp, setup));
  instrs->append(genOR(dst, qpuId, tmp));
}

// Generate instructions to setup VPM store.

void genSetupVPMStore(Seq<Instr>* instrs, BufferAorB b, Reg qpuId)
{
  int setup = 0x00100200;
  int buffIdx = (b == A ? 2 : 3) << 4;
  setup |= buffIdx;

  Reg tmp = freshReg();
  instrs->append(genLI(tmp, setup));

  Reg dst;
  dst.tag   = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;
  instrs->append(genOR(dst, qpuId, tmp));
}

// ============================================================================
// Load/Store pass
// ============================================================================

void loadStorePass(Seq<Instr>* instrs)
{
  Seq<Instr> newInstrs(instrs->numElems*2);

  // Put QPU number in a register
  Reg qpuId = freshReg();
  Reg qpuNum; qpuNum.tag = SPECIAL; qpuNum.regId = SPECIAL_QPU_NUM;
  newInstrs.append(genMove(qpuId, qpuNum));

  // Initialise strides
  genSetReadStride(&newInstrs, 0);
  genSetWriteStride(&newInstrs, 0);

  // Initialise load/store setup registers
  Reg vpmLoadSetup  = freshReg();
  Reg dmaLoadSetup  = freshReg();
  Reg dmaStoreSetup = freshReg();

  assignDMALoadSetup(&newInstrs, dmaLoadSetup, A, qpuId);
  assignDMAStoreSetup(&newInstrs, dmaStoreSetup, A, qpuId);
  assignVPMLoadSetup(&newInstrs, vpmLoadSetup, A, qpuId);

  genSetupVPMStore(&newInstrs, A, qpuId);

  // Elaborate LD1, LD3 and ST2 intermediate instructions
  Reg sp; sp.tag = SPECIAL;
  Reg src; src.tag = REG_A;
  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    switch (instr.tag) {
      case LD1:
        sp.regId = SPECIAL_RD_SETUP;
        src.regId = RSV_READ_STRIDE;
        newInstrs.append(genMove(sp, src));
        newInstrs.append(genMove(sp, dmaLoadSetup));
        sp.regId = SPECIAL_DMA_LD_ADDR;
        newInstrs.append(genMove(sp, instr.LD1.addr));
        break;
      case LD3:
        sp.regId = SPECIAL_RD_SETUP;
        newInstrs.append(genMove(sp, vpmLoadSetup));
        for (int j = 0; j < 3; j++)
          newInstrs.append(nop());
        break;
      case ST2:
        sp.regId = SPECIAL_WR_SETUP;
        src.regId = RSV_WRITE_STRIDE;
        newInstrs.append(genMove(sp, src));
        newInstrs.append(genMove(sp, dmaStoreSetup));
        sp.regId = SPECIAL_DMA_ST_ADDR;
        newInstrs.append(genMove(sp, instr.ST2.addr));
        break;
      case RECV: {
        instr.tag = TMU0_TO_ACC4;
        newInstrs.append(instr);

        Instr move;
        move.tag                = ALU;
        move.ALU.setFlags       = false;
        move.ALU.cond.tag       = ALWAYS;
        move.ALU.dest           = instr.RECV.dest;
        move.ALU.srcA.tag       = REG;
        move.ALU.srcA.reg.tag   = ACC;
        move.ALU.srcA.reg.regId = 4;
        move.ALU.op             = A_BOR;
        move.ALU.srcB.tag       = REG;
        move.ALU.srcB.reg.tag   = ACC;
        move.ALU.srcB.reg.regId = 4;
        newInstrs.append(move);
        break;
      }
      default:
        newInstrs.append(instr);
        break;
    }
  }

  // Update original instruction sequence
  instrs->clear();
  for (int i = 0; i < newInstrs.numElems; i++)
    instrs->append(newInstrs.elems[i]);
}

#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/LoadStore.h"

namespace QPULib {

// =============================================================================
// VPM setup
// =============================================================================

static int vpmSetupReadCode(int n, int hor, int stride)
{
  assert(n >= 1 && n <= 16); // A max of 16 vectors can be read
  assert(stride >= 1 && stride <= 64); // Valid stride
  assert(hor == 0 || hor == 1); // Horizontal or vertical

  // Max values encoded as 0
  if (n == 16) n = 0;
  if (stride == 64) stride = 0;

  // Setup code
  int code = n << 20;
  code |= stride << 12;
  code |= hor << 11;
  code |= 2 << 8;

  return code;
}

static int vpmSetupWriteCode(int hor, int stride)
{
  assert(stride >= 1 && stride <= 64); // Valid stride
  assert(hor == 0 || hor == 1); // Horizontal or vertical

  // Max values encoded as 0
  if (stride == 64) stride = 0;
  
  // Setup code
  int code = stride << 12;
  code |= hor << 11;
  code |= 2 << 8;
  
  return code;
}

// Generate instructions to setup VPM load.

void genSetupVPMLoad(Seq<Instr>* instrs, int n, int addr, int hor, int stride)
{
  assert(addr < 256);

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;

  int setup = vpmSetupReadCode(n, hor, stride) | (addr & 0xff);
  instrs->append(genLI(dst, setup));

  Instr instr;
  instr.tag = VPM_STALL;
  instrs->append(instr);
}

void genSetupVPMLoad(Seq<Instr>* instrs, int n, Reg addr, int hor, int stride)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;

  Reg tmp = freshReg();
  int setup = vpmSetupReadCode(n, hor, stride);
  instrs->append(genLI(tmp, setup));
  instrs->append(genOR(dst, addr, tmp));

  Instr instr;
  instr.tag = VPM_STALL;
  instrs->append(instr);
}

// Generate instructions to setup VPM store.

void genSetupVPMStore(Seq<Instr>* instrs, int addr, int hor, int stride)
{
  assert(addr < 256);

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;

  int setup = vpmSetupWriteCode(hor, stride) | (addr & 0xff);
  instrs->append(genLI(dst, setup));
}

void genSetupVPMStore(Seq<Instr>* instrs, Reg addr, int hor, int stride)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;

  Reg tmp = freshReg();
  int setup = vpmSetupWriteCode(hor, stride);
  instrs->append(genLI(tmp, setup));
  instrs->append(genOR(dst, addr, tmp));
}

// =============================================================================
// DMA setup
// =============================================================================

// (rowLen in bytes)
static int dmaSetupStoreCode(int numRows, int rowLen, int hor)
{
  assert(numRows > 0 && numRows <= 128);
  assert(rowLen > 0 && rowLen <= 128);
  if (numRows == 128) numRows = 0;
  if (rowLen == 128) rowLen = 0;

  int setup = 0x80000000;
  setup |= numRows << 23;
  setup |= rowLen << 16;
  setup |= hor << 14;
  return setup;
}

// (rowLen in 32-bit words)
static int dmaSetupLoadCode(int numRows, int rowLen, int hor, int vpitch)
{
  assert(numRows > 0 && numRows <= 16);
  assert(rowLen > 0 && rowLen <= 16);
  assert(vpitch > 0 && vpitch <= 16);
  if (numRows == 16) numRows = 0;
  if (rowLen == 16) rowLen = 0;
  if (vpitch == 16) vpitch = 0;

  int setup = 0x80000000;
  setup |= rowLen << 20;
  setup |= numRows << 16;
  setup |= vpitch << 12;
  setup |= (hor == 0 ? 1 : 0) << 11;
  return setup;
}

// Generate instructions to setup DMA load.

void genSetupDMALoad(
  Seq<Instr>* instrs, int numRows, int rowLen,
                      int hor, int vpitch, int vpmAddr)
{
  assert(vpmAddr < 2048);
  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);
  setup |= vpmAddr;

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;
  instrs->append(genLI(dst, setup));
}

void genSetupDMALoad(
  Seq<Instr>* instrs, int numRows, int rowLen,
                      int hor, int vpitch, Reg vpmAddr)
{
  int setup = dmaSetupLoadCode(numRows, rowLen, hor, vpitch);

  Reg tmp = freshReg();
  instrs->append(genLI(tmp, setup));

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;
  instrs->append(genOR(dst, vpmAddr, tmp));
}

void genStartDMALoad(Seq<Instr>* instrs, Reg memAddr)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_DMA_LD_ADDR;
  instrs->append(genOR(dst, memAddr, memAddr));
}

void genWaitDMALoad(Seq<Instr>* instrs)
{
  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond.tag          = NEVER;
  instr.ALU.op                = A_BOR;
  instr.ALU.dest.tag          = NONE;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg.tag      = SPECIAL;
  instr.ALU.srcA.reg.regId    = SPECIAL_DMA_LD_WAIT;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
  instrs->append(instr);
}

// Generate instructions to do DMA store.

void genSetupDMAStore(
  Seq<Instr>* instrs, int numRows, int rowLen,
                      int hor, int vpmAddr)
{
  assert(vpmAddr < 2048);
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);
  setup |= vpmAddr << 3;

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;
  instrs->append(genLI(dst, setup));
}

void genSetupDMAStore(
  Seq<Instr>* instrs, int numRows, int rowLen,
                      int hor, Reg vpmAddr)
{
  int setup = dmaSetupStoreCode(numRows, rowLen, hor);

  Reg tmp0 = freshReg();
  instrs->append(genLI(tmp0, setup));

  Reg tmp1 = freshReg();
  instrs->append(genLShift(tmp1, vpmAddr, 3));

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;
  instrs->append(genOR(dst, tmp0, tmp1));
}

void genStartDMAStore(Seq<Instr>* instrs, Reg memAddr)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_DMA_ST_ADDR;
  instrs->append(genOR(dst, memAddr, memAddr));
}

void genWaitDMAStore(Seq<Instr>* instrs)
{
  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond.tag          = NEVER;
  instr.ALU.op                = A_BOR;
  instr.ALU.dest.tag          = NONE;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg.tag      = SPECIAL;
  instr.ALU.srcA.reg.regId    = SPECIAL_DMA_ST_WAIT;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
  instrs->append(instr);
}

// =============================================================================
// DMA stride setup
// =============================================================================

// Generate instructions to set the DMA read pitch.

void genSetReadPitch(Seq<Instr>* instrs, int pitch)
{
  assert(pitch < 8192);
  int setup = 0x90000000 | pitch;
  Reg dst; dst.tag = SPECIAL; dst.regId = SPECIAL_RD_SETUP;
  instrs->append(genLI(dst, setup));
}

void genSetReadPitch(Seq<Instr>* instrs, Reg pitch)
{
  Reg tmp = freshReg();
  instrs->append(genLI(tmp, 0x90000000));

  Reg dst; dst.tag = SPECIAL; dst.regId = SPECIAL_RD_SETUP;
  instrs->append(genOR(dst, tmp, pitch));
}

// Generate instructions to set the DMA write stride.

void genSetWriteStride(Seq<Instr>* instrs, int stride)
{
  assert(stride < 8192);
  int setup = 0xc0000000 | stride;
  Reg dst; dst.tag = SPECIAL; dst.regId = SPECIAL_WR_SETUP;
  Instr instr = genLI(dst, setup);
  instrs->append(instr);
}

void genSetWriteStride(Seq<Instr>* instrs, Reg stride)
{
  Reg tmp = freshReg();
  instrs->append(genLI(tmp, 0xc0000000));

  Reg dst; dst.tag = SPECIAL; dst.regId = SPECIAL_WR_SETUP;
  instrs->append(genOR(dst, tmp, stride));
}

// ============================================================================
// Load/Store pass
// ============================================================================

void loadStorePass(Seq<Instr>* instrs)
{
  Seq<Instr> newInstrs(instrs->numElems*2);

  // Put QPU number in a register
  //Reg qpuId = freshReg();
  //Reg qpuNum; qpuNum.tag = SPECIAL; qpuNum.regId = SPECIAL_QPU_NUM;
  //newInstrs.append(genMove(qpuId, qpuNum));

  for (int i = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    switch (instr.tag) {
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

}  // namespace QPULib

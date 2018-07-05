#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/LoadStore.h"

namespace QPULib {

// =============================================================================
// VPM setup
// =============================================================================

inline int vpmSetupReadCode(int n, int hor, int stride)
{
  assert(n >= 1 && n <= 16); // A max of 16 vectors can be read
  assert(stride >= 1 && stide <= 64); // Valid stride
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

inline int vpmSetupWriteCode(int hor, int stride)
{
  assert(stride >= 1 && stide <= 64); // Valid stride
  assert(hor == 0 || hor == 1); // Horizontal or vertical

  // Max values encoded as 0
  if (n == 16) n = 0;
  if (stride == 64) stride = 0;
  
  // Setup code
  int code = stride << 12;
  code |= hor << 11;
  code |= 2 << 8;
  
  return code;
}

// Generate instructions to setup VPM load.

void genSetupVPMLoad(Seq<Instr>* instrs, int n, int addr, bool hor, int stride)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;

  int setup = vpmSetupReadCode(n, hor, stride);
  instrs->append(genLI(dst, setup));
}

void genSetupVPMLoad(Seq<Instr>* instrs, int n, Reg addr, bool hor, int stride)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_RD_SETUP;

  Reg tmp = freshReg();
  int setup = vpmSetupReadCode(n, hor, stride);
  instrs->append(genLI(tmp, setup));
  instrs->append(genOR(dst, addr, tmp));
}

// Generate instructions to setup VPM store.

void genSetupVPMStore(Seq<Instr>* instrs, int addr, bool hor, int stride)
{
  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_WR_SETUP;

  int setup = vpmSetupWriteCode(hor, stride);
  instrs->append(genLI(dst, setup));
}

void genSetupVPMStore(Seq<Instr>* instrs, Reg addr, bool hor, int stride)
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

// (vpmRowLen in bytes)
int dmaSetupStoreCode(int vpmNumRows, int vpmRowLen, Dir vpmDir)
{
  assert(vpmNumRows > 0 && vpmNumRows <= 128);
  assert(vpmRowLen > 0 && vpmRowLen <= 128);
  if (vpmNumRows == 128) vpmNumRows = 0;
  if (vpmRowLen == 128) vpmRowLen = 0;

  int setup = 0x80000000;
  setup |= vpmNumRows << 23;
  setup |= vpmRowLen << 16;
  setup |= (dir == HORIZ ? 1 : 0) << 14;
  return setup;
}

// (vpmRowLen in 32-bit words)
int dmaSetupLoadCode(int vpmNumRows, int vpmRowLen, Dir dir, int vpitch)
{
  assert(vpmNumRows > 0 && vpmNumRows <= 16);
  assert(vpmRowLen > 0 && vpmRowLen <= 16);
  assert(vpitch > 0 && vpitch <= 16);
  if (vpmNumRows == 16) vpmNumRows = 0;
  if (vpmRowLen == 16) vpmRowLen = 0;
  if (vpitch == 16) vpitch = 0;

  int setup = 0x80000000;
  setup |= 1 << 28; // Use separate stride register
  setup |= vpmRowLen << 20;
  setup |= vpmNumRows << 16;
  setup |= vpitch << 12;
  setup |= (dir == VERT ? 1 : 0) << 12;
  return setup;
}

// Generate instructions to setup DMA load.

void genDMALoad(
  Seq<Instr>* instrs, int vpmNumRows, int vpmRowLen,
                      Dir vpmDir, int vpitch, int vpmAddr, Reg memAddr)
{
  assert(vpmAddr < 2048);
  int setup = dmaSetupLoadCode(vpmNumRows, vpmRowLen, vpmDir, vpitch);
  setup |= vpmAddr;

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_DMA_LD_SETUP;
  instrs->append(genLI(dst, setup));

  dst.regId = SPECIAL_DMA_LD_ADDR;
  instrs->append(genOR(dst, memAddr, memAddr));
}

// TODO version with Reg vpmAddr

// Generate instructions to do DMA store.

void genDMAStore(
  Seq<Instr>* instrs, int vpmNumRows, int vpmRowLen,
                      Dir vpmDir, int vpmAddr, Reg memAddr)
{
  assert(vpmAddr < 2048);
  int setup = dmaSetupStoreCode(vpmNumRows, vpmRowLen, vpmDir);
  setup |= vpmAddr << 3;

  Reg dst;
  dst.tag = SPECIAL;
  dst.regId = SPECIAL_DMA_ST_SETUP;
  instrs->append(genLI(dst, setup));

  dst.regId = SPECIAL_DMA_ST_ADDR;
  instrs->append(genOR(dst, memAddr, memAddr));
}

// TODO version with Reg vpmAddr

// =============================================================================
// DMA stride setup
// =============================================================================

// Generate instructions to set the DMA read pitch.

void genSetReadPitch(Seq<Instr>* instrs, int pitch)
{
  assert(stride < 8192);
  int setup = 0x90000000 | stride;
  Reg dst; dst.tag = REG_A; dst.regId = RSV_READ_STRIDE;
  Instr instr = genLI(dst, setup);
  instrs->append(instr);
}

void genSetReadPitch(Seq<Instr>* instrs, Reg pitch)
{
  Reg tmp = freshReg();
  instrs->append(genLI(tmp, 0x90000000));

  Reg dst; dst.tag = REG_A; dst.regId = RSV_READ_STRIDE;
  instrs->append(genOR(dst, tmp, pitch));
}

// Generate instructions to set the DMA write stride.

void genSetWriteStride(Seq<Instr>* instrs, int stride)
{
  assert(stride < 8192);
  int setup = 0xc0010000 | stride;
  Reg dst; dst.tag = REG_A; dst.regId = RSV_WRITE_STRIDE;
  Instr instr = genLI(dst, setup);
  instrs->append(instr);
}

void genSetWriteStride(Seq<Instr>* instrs, Reg stride)
{
  Reg tmp = freshReg();
  instrs->append(genLI(tmp, 0xc0010000));

  Reg dst; dst.tag = REG_A; dst.regId = RSV_WRITE_STRIDE;
  instrs->append(genOR(dst, tmp, stride));
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

}  // namespace QPULib

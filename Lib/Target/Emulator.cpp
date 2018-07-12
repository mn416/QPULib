#include "Target/Emulator.h"
#include "Target/Syntax.h"
#include "Target/SmallLiteral.h"
#include "VideoCore/SharedArray.h"

#include <math.h>
#include <string.h>

namespace QPULib {

// ============================================================================
// Globals
// ============================================================================

// Heap used in emulation mode.  See 'Pointer.h' for more details.

uint32_t emuHeapEnd = 0;
int32_t* emuHeap    = NULL;

// ============================================================================
// Horizontal/Vertical VPM access
// ============================================================================

// Convert horizontal VPM index to vertical VPM index
static uint32_t flip(uint32_t index) {
  uint32_t x = index % 16;
  uint32_t y = (index >> 4) % 64;
  return (x << 6) | y;
}

// Convert vertical VPM index to horizontal VPM index
static uint32_t unflip(uint32_t index) {
  uint32_t x = (index >> 6) % 16;
  uint32_t y = index % 64;
  return (y << 4) | x;
}

// ============================================================================
// Read a vector register
// ============================================================================

Vec readReg(QPUState* s, State* g, Reg reg)
{
  Vec v;
  int r = reg.regId;
  switch (reg.tag) {
    case REG_A:
      assert(r >= 0 && r < s->sizeRegFileA);
      return s->regFileA[r];
    case REG_B:
      assert(r >= 0 && r < s->sizeRegFileB);
      return s->regFileB[reg.regId];
    case ACC:
      assert(r >= 0 && r <= 5);
      return s->accum[r];
    case SPECIAL:
      if (reg.regId == SPECIAL_ELEM_NUM) {
        for (int i = 0; i < NUM_LANES; i++)
          v.elems[i].intVal = i;
        return v;
      }
      else if (reg.regId == SPECIAL_UNIFORM) {
        assert(s->nextUniform < g->uniforms->numElems);
        for (int i = 0; i < NUM_LANES; i++)
          if (s->nextUniform == -2)
            v.elems[i].intVal = s->id;
          else if (s->nextUniform == -1)
            v.elems[i].intVal = s->numQPUs;
          else
            v.elems[i].intVal = g->uniforms->elems[s->nextUniform];
        s->nextUniform++;
        return v;
      }
      else if (reg.regId == SPECIAL_QPU_NUM) {
        for (int i = 0; i < NUM_LANES; i++)
          v.elems[i].intVal = s->id;
        return v;
      }
      else if (reg.regId == SPECIAL_VPM_READ) {
        // Make sure there's a VPM load request waiting
        assert(! s->vpmLoadQueue.isEmpty());
        VPMLoadReq* req = s->vpmLoadQueue.first();
        assert(req->numVecs > 0);
        if (req->hor) {
          // Horizontal load
          for (int i = 0; i < NUM_LANES; i++) {
            int index = (16*req->addr+i);
            assert(index < VPM_SIZE);
            v.elems[i] = g->vpm[index];
          }
        }
        else {
          // Vertical load
          for (int i = 0; i < NUM_LANES; i++) {
            uint32_t x = req->addr & 0xf;
            uint32_t y = req->addr >> 4;
            int index = (y*16*16 + x + i*16);
            assert(index < VPM_SIZE);
            v.elems[i] = g->vpm[index];
          }
        }
        req->numVecs--;
        req->addr = req->addr + req->stride;
        if (req->numVecs == 0) s->vpmLoadQueue.deq(); 
        return v;
      }
      else if (reg.regId == SPECIAL_DMA_LD_WAIT) {
        // Perform DMA load to completion
        if (s->dmaLoad.active == false) return v;
        DMALoadReq* req = &s->dmaLoadSetup;
        uint32_t base = req->vpmAddr;
        if (req->hor) {
          // Horizontal access
          for (int r = 0; r < req->numRows; r++) {
            for (int i = 0; i < req->rowLen; i++) {
              int addr = s->dmaLoad.addr.intVal + (r * s->readPitch) + i*4;
              int index = (base+i) % VPM_SIZE;
              g->vpm[index].intVal = emuHeap[addr >> 2];
            }
            base = (base + (16*req->vpitch)) % VPM_SIZE;
          }
        }
        else {
          // Vertical access
          base = flip(base);
          for (int r = 0; r < req->numRows; r++) {
            for (int i = 0; i < req->rowLen; i++) {
              int addr = s->dmaLoad.addr.intVal + (r * s->readPitch) + i*4;
              int index = unflip(base+i);
              g->vpm[index].intVal = emuHeap[addr >> 2];
            }
            base = (base + (64*req->vpitch)) % VPM_SIZE;
          }
        }
        s->dmaLoad.active = false;
        return v; // Return value unspecified
      }
      else if (reg.regId == SPECIAL_DMA_ST_WAIT) {
        // Perform DMA store to completion
        if (s->dmaStore.active == false) return v;
        DMAStoreReq* req = &s->dmaStoreSetup;
        uint32_t memAddr = s->dmaStore.addr.intVal;
        uint32_t base = req->vpmAddr;
        if (req->hor) {
          // Horizontal access
          for (int r = 0; r < req->numRows; r++) {
            for (int i = 0; i < req->rowLen; i++) {
              int index = base % VPM_SIZE;
              emuHeap[memAddr >> 2] = g->vpm[index].intVal;
              base = (base+1) % VPM_SIZE;
              memAddr += 4;
            }
            memAddr += s->writeStride;
          }
        }
        else {
          // Vertical access
          base = flip(base);
          for (int r = 0; r < req->numRows; r++) {
            for (int i = 0; i < req->rowLen; i++) {
              int index = unflip(base);
              emuHeap[memAddr >> 2] = g->vpm[index].intVal;
              base = (base+1) % VPM_SIZE;
              memAddr += 4;
            }
            memAddr += s->writeStride;
          }
        }
        s->dmaStore.active = false;
        return v; // Return value unspecified
      }
      printf("QPULib: can't read special register\n");
      abort();
    case NONE:
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = 0;
      return v;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Check condition flags
// ============================================================================

// Given an assignment condition and an vector index, determine if the
// condition is true at that index using the implicit condition flags.

inline bool checkAssignCond(QPUState* s, AssignCond cond, int i)
{
  switch (cond.tag) {
    case NEVER:  return false;
    case ALWAYS: return true;
    case FLAG:
      switch (cond.flag) {
        case ZS: return s->zeroFlags[i];
        case ZC: return !s->zeroFlags[i];
        case NS: return s->negFlags[i];
        case NC: return !s->negFlags[i];
      }
  }

  // Unreachable
  assert(false);
}

// Given a branch condition, determine if it evaluates to true using
// the implicit condition flags.

inline bool checkBranchCond(QPUState* s, BranchCond cond)
{
  bool bools[NUM_LANES];
  switch (cond.tag) {
    case COND_ALWAYS: return true;
    case COND_NEVER: return false;
    case COND_ALL:
    case COND_ANY:
      for (int i = 0; i < NUM_LANES; i++)
        switch (cond.flag) {
          case ZS: bools[i] = s->zeroFlags[i];  break;
          case ZC: bools[i] = !s->zeroFlags[i]; break;
          case NS: bools[i] = s->negFlags[i];   break;
          case NC: bools[i] = !s->negFlags[i];  break;
          default: assert(false); break;
        }
      if (cond.tag == COND_ALL) {
        for (int i = 0; i < NUM_LANES; i++)
          if (! bools[i]) return false;
        return true;
      }
      else if (cond.tag == COND_ANY) {
        for (int i = 0; i < NUM_LANES; i++)
          if (bools[i]) return true;
        return false;
      }
  }
}

// ============================================================================
// Write a vector to a register
// ============================================================================

void writeReg(QPUState* s, State* g, bool setFlags,
                AssignCond cond, Reg dest, Vec v)
{
  switch (dest.tag) {
    case REG_A:
    case REG_B:
    case ACC:
    case NONE:
      Vec* w;

      if (dest.tag == REG_A) {
        assert(dest.regId >= 0 && dest.regId < s->sizeRegFileA);
        w = &s->regFileA[dest.regId];
      }
      else if (dest.tag == REG_B) {
        assert(dest.regId >= 0 && dest.regId < s->sizeRegFileB);
        w = &s->regFileB[dest.regId];
      }
      else if (dest.tag == ACC) {
        assert(dest.regId >= 0 && dest.regId <= 5);
        w = &s->accum[dest.regId];
      }
      
      for (int i = 0; i < NUM_LANES; i++)
        if (checkAssignCond(s, cond, i)) {
          Word x = v.elems[i];
          if (dest.tag != NONE) w->elems[i] = x;
          if (setFlags) {
            s->zeroFlags[i] = x.intVal == 0;
            s->negFlags[i]  = x.intVal < 0;
          }
        }

      return;
    case SPECIAL:
      switch (dest.regId) {
        case SPECIAL_RD_SETUP: {
          int setup = v.elems[0].intVal;
          if ((setup & 0xf0000000) == 0x90000000) {
            // Set read pitch
            int pitch = (setup & 0x1fff);
            s->readPitch = pitch;
            return;
          }
          else if ((setup & 0xc0000000) == 0) {
            // QPU only allows two VPM loads queued at a time
            assert(! s->vpmLoadQueue.isFull());
            // Create VPM load request
            VPMLoadReq req;
            req.numVecs = (setup >> 20) & 0xf;
            if (req.numVecs == 0) req.numVecs = 16;
            req.hor = ((setup >> 11) & 1);
            req.addr = setup & 0xff;
            req.stride = (setup >> 12) & 0x3f;
            if (req.stride == 0) req.stride = 64;
            // Add VPM load request to queue
            s->vpmLoadQueue.enq(req);
            return;
          }
          else if (setup & 0x80000000) {
            // DMA load setup
            DMALoadReq* req = &s->dmaLoadSetup;
            req->rowLen = (setup >> 20) & 0xf;
            if (req->rowLen == 0) req->rowLen = 16;
            req->numRows = (setup >> 16) & 0xf;
            if (req->numRows == 0) req->numRows = 16;
            req->vpitch = (setup >> 12) & 0xf;
            if (req->vpitch == 0) req->vpitch = 16;
            req->hor = (setup & 0x800) ? false : true;
            req->vpmAddr = (setup & 0x7ff);
            return;
          }
          break;
        }
        case SPECIAL_WR_SETUP: {
          int setup = v.elems[0].intVal;
          if ((setup & 0xc0000000) == 0xc0000000) {
            // Set write stride
            int stride = setup & 0x1fff;
            s->writeStride = stride;
            return;
          }
          else if ((setup & 0xc0000000) == 0x80000000) {
            // DMA write setup
            DMAStoreReq* req = &s->dmaStoreSetup;
            req->rowLen = (setup >> 16) & 0x7f;
            if (req->rowLen == 0) req->rowLen = 128;
            req->numRows = (setup >> 23) & 0x7f;
            if (req->numRows == 0) req->numRows = 128;
            req->hor = (setup & 0x4000);
            req->vpmAddr = (setup >> 3) & 0x7ff;
            return;
          }
          else if ((setup & 0xc0000000) == 0) {
            VPMStoreReq req;
            req.hor = (setup >> 11) & 1;
            req.addr = setup & 0xff;
            req.stride = (setup >> 12) & 0x3f;
            if (req.stride == 0) req.stride = 64;
            s->vpmStoreSetup = req;
            return;
          }
          break;
        }
        case SPECIAL_VPM_WRITE: {
          VPMStoreReq* req = &s->vpmStoreSetup;
          if (req->hor) {
            // Horizontal store
            for (int i = 0; i < NUM_LANES; i++) {
              int index = (16*req->addr+i);
              assert(index < VPM_SIZE);
              g->vpm[index] = v.elems[i];
            }
          }
          else {
            // Vertical store
            uint32_t x = req->addr & 0xf;
            uint32_t y = req->addr >> 4;
            for (int i = 0; i < NUM_LANES; i++) {
              int index = (y*16*16 + x + i*16);
              assert(index < VPM_SIZE);
              g->vpm[index] = v.elems[i];
            }
          }
          req->addr = req->addr + req->stride;
          return;
        }
        case SPECIAL_DMA_LD_ADDR: {
          // Initiate DMA load
          assert(!s->dmaLoad.active);
          s->dmaLoad.active = true;
          s->dmaLoad.addr   = v.elems[0];
          return;
        }
        case SPECIAL_DMA_ST_ADDR: {
          // Initiate DMA store
          assert(!s->dmaStore.active);
          s->dmaStore.active = true;
          s->dmaStore.addr   = v.elems[0];
          return;
        }
        case SPECIAL_HOST_INT: {
          return;
        }
        case SPECIAL_TMU0_S: {
          assert(s->loadBuffer->numElems < 4);
          Vec val;
          for (int i = 0; i < NUM_LANES; i++) {
            uint32_t a = (uint32_t) v.elems[i].intVal;
            val.elems[i].intVal = emuHeap[a>>2];
          }
          s->loadBuffer->append(val);
          return;
        }
        default:
          break;
      }

      printf("QPULib: can't write to special register\n");
      abort();
      return;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Interpret an immediate operaand
// ============================================================================

Vec evalImm(Imm imm)
{
  Vec v;
  switch (imm.tag) {
    case IMM_INT32:
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = imm.intVal;
      return v;
    case IMM_FLOAT32:
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].floatVal = imm.floatVal;
      return v;
    case IMM_MASK:
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = (imm.intVal >> i) & 1;
      return v;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Rotate a vector
// ============================================================================

Vec rotate(Vec v, int n)
{
  Vec w;
  for (int i = 0; i < NUM_LANES; i++)
    w.elems[(i+n) % NUM_LANES] = v.elems[i];
  return w;
}

// ============================================================================
// Interpret a small immediate operand
// ============================================================================

Vec evalSmallImm(QPUState* s, SmallImm imm)
{
  Vec v;
  switch (imm.tag) {
    case SMALL_IMM: {
      Word w = decodeSmallLit(imm.val);
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i] = w;
      return v;
    }
    case ROT_ACC:
    case ROT_IMM:
      int amount = (imm.tag == ROT_IMM)
                 ? imm.val : (int) s->accum[4].elems[0].intVal;
      return rotate(v, amount);
  }

  // Unreachable
  assert(false);
}

Vec readRegOrImm(QPUState* s, State* g, RegOrImm src)
{
  switch (src.tag) {
    case REG: return readReg(s, g, src.reg);
    case IMM: return evalSmallImm(s, src.smallImm);
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// ALU helpers
// ============================================================================

// Rotate right
inline int32_t rotRight(int32_t x, int32_t n)
{
  uint32_t ux = (uint32_t) x;
  return (ux >> n) | (x << (32-n));
}

// Count leading zeros
inline int32_t clz(int32_t x)
{
  int32_t count = 0;
  int32_t n = (int32_t) (sizeof(int)*8);
  for (int32_t i = 0; i < n; i++) {
    if (x & (1 << (n-1))) break;
    else count++;
    x <<= 1;
  }
  return count;
}

// ============================================================================
// ALU
// ============================================================================

Vec alu(QPUState* s, State* g,
        RegOrImm srcA, ALUOp op, RegOrImm srcB)
{
  // First, obtain vector operands
  Vec x, y, z;
  x = readRegOrImm(s, g, srcA);
  if (srcA.tag == REG && srcB.tag == REG && srcA.reg == srcB.reg)
    y = x;
  else
    y = readRegOrImm(s, g, srcB);
  Word* a = x.elems;
  Word* b = y.elems;
  Word* c = z.elems;

  // Now evaluate the operation
  switch (op) {
    case NOP:
      // No-op
      break;
    case A_FADD:
      // Floating-point add
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal + b[i].floatVal;
      break;
    case A_FSUB:
      // Floating-point subtract
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal - b[i].floatVal;
      break;
    case A_FMIN:
      // Floating-point min
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal < b[i].floatVal
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case A_FMAX:
      // Floating-point max
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal > b[i].floatVal
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case A_FMINABS:
      // Floating-point min of absolute values
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = fabs(a[i].floatVal) < fabs(b[i].floatVal)
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case A_FMAXABS:
      // Floating-point max of absolute values
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = fabs(a[i].floatVal) > fabs(b[i].floatVal)
                      ? a[i].floatVal : b[i].floatVal;
      break;
    case A_FtoI:
      // Float to signed integer
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (int) a[i].floatVal;
      break;
    case A_ItoF:
      // Signed integer to float
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = (float) a[i].intVal;
      break;
    case A_ADD:
      // Integer add
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal + b[i].intVal;
      break;
    case A_SUB:
      // Integer subtract
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal - b[i].intVal;
      break;
    case A_SHR:
      // Integer shift right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (int32_t) ((uint32_t) a[i].intVal >> b[i].intVal);
      break;
    case A_ASR:
      // Integer arithmetic shift right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal >> b[i].intVal;
      break;
     case A_ROR:
      // Integer rotate right
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = rotRight(a[i].intVal, b[i].intVal);
      break;
     case A_SHL:
      // Integer shift left
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal << b[i].intVal;
      break;
    case A_MIN:
      // Integer min
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal < b[i].intVal
                    ? a[i].intVal : b[i].intVal;
      break;
    case A_MAX:
      // Integer max
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal > b[i].intVal
                    ? a[i].intVal : b[i].intVal;
      break;
    case A_BAND:
      // Bitwise and
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal & b[i].intVal;
      break;
    case A_BOR:
      // Bitwise or
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal | b[i].intVal;
      break;
    case A_BXOR:
      // Bitwise xor
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = a[i].intVal ^ b[i].intVal;
      break;
    case A_BNOT:
      // Bitwise not
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = ~a[i].intVal;
      break;
    case A_CLZ:
      // Count leading zeros
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = clz(a[i].intVal);
      break;
    case M_FMUL:
      // Floating-point multiply
      for (int i = 0; i < NUM_LANES; i++)
        c[i].floatVal = a[i].floatVal * b[i].floatVal;
      break;
    case M_MUL24:
      // Integer multiply (24-bit)
      for (int i = 0; i < NUM_LANES; i++)
        c[i].intVal = (a[i].intVal & 0xffffff) * (b[i].intVal & 0xffffff);
      break;
    case M_ROTATE:
      // Vector rotation
      z = rotate(x, (int) b[0].intVal);
      break;
    case A_V8ADDS:
    case A_V8SUBS:
    case M_V8MUL:
    case M_V8MIN:
    case M_V8MAX:
    case M_V8ADDS:
    case M_V8SUBS:
    default:
      printf("QPULib: unsupported operator %i\n", op);
      abort();
  }
  return z;
}

// ============================================================================
// Printing routines
// ============================================================================

void emitChar(Seq<char>* out, char c)
{
  if (out == NULL) printf("%c", c);
  else out->append(c);
}

void emitStr(Seq<char>* out, const char* s)
{
  if (out == NULL)
    printf("%s", s);
  else
    for (int i = 0; i < strlen(s); i++)
      out->append(s[i]);
}

void printIntVec(Seq<char>* out, Vec x)
{
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%i", x.elems[i].intVal);
    for (int j = 0; j < strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}

void printFloatVec(Seq<char>* out, Vec x)
{
  char buffer[1024];
  emitChar(out, '<');
  for (int i = 0; i < NUM_LANES; i++) {
    snprintf(buffer, sizeof(buffer), "%f", x.elems[i].floatVal);
    for (int j = 0; j < strlen(buffer); j++) emitChar(out, buffer[j]);
    if (i != NUM_LANES-1) emitChar(out, ',');
  }
  emitChar(out, '>');
}

// ============================================================================
// Emulator
// ============================================================================

void emulate
  ( int numQPUs            // Number of QPUs active
  , Seq<Instr>* instrs     // Instruction sequence
  , int maxReg             // Max reg id used
  , Seq<int32_t>* uniforms // Kernel parameters
  , Seq<char>* output      // Output from print statements
                           // (if NULL, stdout is used)
  )
{
  State state;
  state.output = output;
  state.uniforms = uniforms;

  // Initialise state
  for (int i = 0; i < numQPUs; i++) {
    QPUState q;
    memset(&q, 0, sizeof(QPUState));
    q.id                 = i;
    q.numQPUs            = numQPUs;
    q.pc                 = 0;
    q.running            = true;
    q.regFileA           = new Vec [maxReg+1];
    q.sizeRegFileA       = maxReg+1;
    q.regFileB           = new Vec [maxReg+1];
    q.sizeRegFileB       = maxReg+1;
    q.nextUniform        = -2;
    q.dmaLoad.active     = false;
    q.dmaStore.active    = false;
    q.readPitch          = 0;
    q.writeStride        = 0;
    q.loadBuffer         = new SmallSeq<Vec>;
    state.qpu[i]         = q;
  }
  // Initialise semaphores
  for (int i = 0; i < 16; i++) state.sema[i] = 0;

  bool anyRunning = true;
  while (anyRunning) {
    anyRunning = false;

    // Execute an instruction in each active QPU
    for (int i = 0; i < numQPUs; i++) {
      QPUState* s = &state.qpu[i];
      if (s->running) {
        anyRunning = true;
        assert(s->pc < instrs->numElems);
        Instr instr = instrs->elems[s->pc++];
        switch (instr.tag) {
          // Load immediate
          case LI: {
            Vec imm = evalImm(instr.LI.imm);
            writeReg(s, &state, instr.LI.setFlags,
                       instr.LI.cond, instr.LI.dest, imm);
            break;
          }
          // ALU operation
          case ALU: {
            Vec result = alu(s, &state, instr.ALU.srcA,
                             instr.ALU.op, instr.ALU.srcB);
            if (instr.ALU.op != NOP)
              writeReg(s, &state, instr.ALU.setFlags, instr.ALU.cond,
                       instr.ALU.dest, result);
            break;
          }
          // End program (halt)
          case END: {
            s->running = false;
            break;
          }
          // Branch to target
          case BR: {
            if (checkBranchCond(s, instr.BR.cond)) {
              BranchTarget t = instr.BR.target;
              if (t.relative && !t.useRegOffset) {
                s->pc += 3+t.immOffset;
              }
              else {
                printf("QPULib: found unsupported form of branch target\n");
                abort();
              }
            }
            break;
          }
          // Branch to label
          case BRL:
          // Label
          case LAB:
            printf("QPULib: emulator does not support labels\n");
            abort();
          // No-op
          case NO_OP:
            break;
          // PRS: print string
          case PRS: {
            emitStr(state.output, instr.PRS);
            break;
          }
          // PRI: print integer
          case PRI: {
            Vec x = readReg(s, &state, instr.PRI);
            printIntVec(state.output, x);
            break;
          }
          // PRF: print integer
          case PRF: {
            Vec x = readReg(s, &state, instr.PRF);
            printFloatVec(state.output, x);
            break;
          }
          // RECV: receive load-via-TMU response
          case RECV: {
            assert(s->loadBuffer->numElems > 0);
            Vec val = s->loadBuffer->remove(0);
            AssignCond always;
            always.tag = ALWAYS;
            writeReg(s, &state, false, always, instr.RECV.dest, val);
            break;
          }
          // Read from TMU0 into accumulator 4
          case TMU0_TO_ACC4: {
            assert(s->loadBuffer->numElems > 0);
            Vec val = s->loadBuffer->remove(0);
            AssignCond always;
            always.tag = ALWAYS;
            Reg dest;
            dest.tag = ACC;
            dest.regId = 4;
            writeReg(s, &state, false, always, dest, val);
            break;
          }
          // Host IRQ
          case IRQ:
            break;
          // Semaphore increment
          case SINC: {
            assert(instr.semaId >= 0 && instr.semaId <= 15);
            if (state.sema[instr.semaId] == 15) s->pc--;
            else state.sema[instr.semaId]++;
            break;
          }
          // Semaphore decrement
          case SDEC: {
            assert(instr.semaId >= 0 && instr.semaId <= 15);
            if (state.sema[instr.semaId] == 0) s->pc--;
            else state.sema[instr.semaId]--;
            break;
          }
          // Unreachable
          default: assert(false);
        }
      }
    }
  }

  // Deallocate state
  for (int i = 0; i < numQPUs; i++) {
    delete [] state.qpu[i].regFileA;
    delete [] state.qpu[i].regFileB;
  }
}

}  // namespace QPULib


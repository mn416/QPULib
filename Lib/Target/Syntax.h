#ifndef _TARGET_SYNTAX_H_
#define _TARGET_SYNTAX_H_

#include <stdint.h>

// Syntax of the QPU target language.

// This abstract syntax is a balance between a strict and relaxed
// definition of the target language:
// 
//   a "strict" definition would allow only instructions that can run on
//   the target machine to be expressed, whereas a "relaxed" one allows
//   instructions that have no direct mapping to machine instructions.
// 
// A relaxed definition allows the compilation process to be incremental:
// after each pass, the target code gets closer to being executable, by
// transforming away constructs that do not have a direct mapping to
// hardware.  However, we do not want to be too relaxed, otherwise we
// loose scope for the type checker to help us.
// 
// For example, the definition below allows an instruction to read two
// operands from the *same* register file.  In fact, two operands must be
// taken from different register files in the target language.  It is the
// job of a compiler pass to enforce such a constraint.

// ============================================================================
// Sub-word selectors
// ============================================================================

// A sub-word selector allows a 32, 16, or 8-bit portion of each vector
// word to be selected.

enum SubWord {
    A8     // Bits 7..0
  , B8     // Bits 15..8
  , C8     // Bits 23..16
  , D8     // Bits 31..24
  , A16    // Bits 15..0
  , B16    // Bits 31..16
  , A32    // Bits 31..0
};

// ============================================================================
// Registers
// ============================================================================

typedef int RegId;

// Different kinds of registers
enum RegTag {
    REG_A           // In register file A (0..31)
  , REG_B           // In register file B (0..31)
  , ACC             // Accumulator register
  , SPECIAL         // Special register
  , NONE            // No read/write
  , TMP_A           // Used in intermediate code
  , TMP_B           // Used in intermediate code
};

inline bool isRegAorB(RegTag rt)
  { return rt == REG_A || rt == REG_B; }

// Special registers
enum Special {
    // Read-only
    SPECIAL_UNIFORM
  , SPECIAL_ELEM_NUM
  , SPECIAL_QPU_NUM
  , SPECIAL_VPM_READ

    // Write-only
  , SPECIAL_RD_SETUP
  , SPECIAL_WR_SETUP
  , SPECIAL_DMA_ST_ADDR
  , SPECIAL_DMA_ST_WAIT
  , SPECIAL_DMA_LD_ADDR
  , SPECIAL_DMA_LD_WAIT
  , SPECIAL_VPM_WRITE
  , SPECIAL_HOST_INT
  , SPECIAL_TMU0_S
};

struct Reg {
  // What kind of register is it?
  RegTag tag;

  // Register identifier
  RegId regId;
};

inline bool operator==(Reg ra, Reg rb)
  { return ra.tag == rb.tag && ra.regId == rb.regId; }

// ============================================================================
// Conditions
// ============================================================================

enum Flag {
    ZS              // Zero set
  , ZC              // Zero clear
  , NS              // Negative set
  , NC              // Negative clear
};

// Branch conditions

enum BranchCondTag {
    COND_ALL         // Reduce vector of bits to a single
  , COND_ANY         // bit using AND/OR reduction
  , COND_ALWAYS
  , COND_NEVER
};

struct BranchCond {
  // ALL or ANY reduction?
  BranchCondTag tag;

  // Condition flag
  Flag flag;
};

// Assignment conditions

enum AssignCondTag {
    NEVER
  , ALWAYS
  , FLAG
};

struct AssignCond {
  // Kind of assignment condition
  AssignCondTag tag;

  // Condition flag
  Flag flag;
};

// ============================================================================
// Immediates
// ============================================================================

// Different kinds of immediate
enum ImmTag {
    IMM_INT32    // 32-bit word
  , IMM_FLOAT32  // 32-bit float
  , IMM_MASK     // 1 bit per vector element (0 to 0xffff)
};

struct Imm {
  ImmTag tag;

  union {
    int intVal;
    float floatVal;
  };
};

// Different kinds of small immediates
enum SmallImmTag {
    SMALL_IMM  // Small immediate
  , ROT_ACC    // Rotation amount taken from accumulator 5
  , ROT_IMM    // Rotation amount 1..15
};

struct SmallImm {
  // What kind of small immediate is it?
  SmallImmTag tag;
  
  // Immediate value
  int val;
};

// A register or a small immediate operand?
enum RegOrImmTag { REG, IMM };

struct RegOrImm {
  // Register id or small immediate?
  RegOrImmTag tag;

  union {
    // A register
    Reg reg;
    
    // A small immediate
    SmallImm smallImm;
  };
};

// ============================================================================
// ALU operations
// ============================================================================

// Add operators
enum ALUOp {
    NOP            // No op

  // Opcodes for the 'add' ALU
  , A_FADD         // Floating-point add
  , A_FSUB         // Floating-point subtract
  , A_FMIN         // Floating-point min
  , A_FMAX         // Floating-point max
  , A_FMINABS      // Floating-point min of absolute values
  , A_FMAXABS      // Floating-point max of absolute values
  , A_FtoI         // Float to signed integer
  , A_ItoF         // Signed integer to float
  , A_ADD          // Integer add
  , A_SUB          // Integer subtract
  , A_SHR          // Integer shift right
  , A_ASR          // Integer arithmetic shift right
  , A_ROR          // Integer rotate right
  , A_SHL          // Integer shift left
  , A_MIN          // Integer min
  , A_MAX          // Integer max
  , A_BAND         // Bitwise and
  , A_BOR          // Bitwise or
  , A_BXOR         // Bitwise xor
  , A_BNOT         // Bitwise not
  , A_CLZ          // Count leading zeros
  , A_V8ADDS       // Add with saturation per 8-bit element
  , A_V8SUBS       // Subtract with saturation per 8-bit element

  // Opcodes for the 'mul' ALU
  , M_FMUL        // Floating-point multiply
  , M_MUL24       // 24-bit integer multiply
  , M_V8MUL       // Multiply per 8-bit element
  , M_V8MIN       // Min per 8-bit element
  , M_V8MAX       // Max per 8-bit element
  , M_V8ADDS      // Add with saturation per 8-bit element
  , M_V8SUBS      // Subtract with saturation per 8-bit element
  , M_ROTATE      // Rotation (intermediate op-code)

};

inline bool isMulOp(ALUOp op)
{
  return op == M_FMUL   || op == M_MUL24 || op == M_V8MUL  ||
         op == M_V8MIN  || op == M_V8MAX || op == M_V8ADDS ||
         op == M_V8SUBS || op == M_ROTATE;
}

// ============================================================================
// Branch targets
// ============================================================================

struct BranchTarget {
  // Branch is absolute or relative to PC+4
  bool relative;

  // Plus value from register file A (optional)
  bool useRegOffset;
  RegId regOffset;

  // Plus 32-bit immediate value
  int immOffset;
};

// We allow labels for branching, represented by integer identifiers.  These
// will be translated to actual branch targets in a linking phase.

typedef int Label;

// ============================================================================
// Loads/store buffering
// ============================================================================

// We reserve two load buffers and two store buffers for each QPU in the VPM
// (shared local) memory.  The reason for two of each is to allow double
// buffering.  We refer to a double buffer as A and B buffers.

enum BufferAorB { A, B };

// ============================================================================
// Instructions
// ============================================================================

// QPU instruction tags
enum InstrTag {
    LI            // Load immediate
  , ALU           // ALU operation
  , BR            // Conditional branch to target
  , END           // Program end (halt)

  // ==================================================
  // The remainder are intermediate-language constructs
  // ==================================================

  , BRL           // Conditional branch to label
  , LAB           // Label
  , NO_OP         // No-op

  // Load instructions
  // -----------------
  //
  // Four instructions are used to implement a memory load.

  , LD1           // First, DMA vector in DRAM into VPM (local) memory
  , LD2           // Second, wait for DMA completion
  , LD3           // Third, setup a read from VPM memory
  , LD4           // Fourth, transfer from VPM into given register

  // Rules for loads:
  //   * An LD1 must be followed (eventually) by a corresponding LD2
  //   * Ditto for LD3 and LD4
  //   * There must be at least 3 instructions between an LD3 and an LD4
  //   * An LD1/LD2 need not be followed by a corresponding LD3/LD4,
  //     thus can be issued speculatively
  //   * A new LD1 can be issued after an LD2, allowing double buffering

  // Store instructions
  // ------------------
  //
  // Three instructions are required to perform a memory store.

  , ST1           // First, write the vector to VPM (local) memory.
  , ST2           // Second, DMA from the VPM out to DRAM.
  , ST3           // Third, wait for DMA to complete.

  // Semaphores
  // ----------

  , SINC          // Increment semaphore
  , SDEC          // Decrement semaphore

  // Send IRQ to host
  // ----------------

  , IRQ

  // Load receive via TMU
  // --------------------

  , RECV
  , TMU0_TO_ACC4

  // Print instructions
  // ------------------

  , PRS           // Print string
  , PRI           // Print integer
  , PRF           // Print float
};

// QPU instructions
struct Instr {
  // What kind of instruction is it?
  InstrTag tag;

  union {
    // Load immediate
    struct { bool setFlags; AssignCond cond; Reg dest; Imm imm; } LI;

    // ALU operation
    struct { bool setFlags; AssignCond cond; Reg dest;
             RegOrImm srcA; ALUOp op; RegOrImm srcB; } ALU;

    // Conditional branch (to target)
    struct { BranchCond cond; BranchTarget target; } BR;

    // ==================================================
    // The remainder are intermediate-language constructs
    // ==================================================

    // Conditional branch (to label)
    struct { BranchCond cond; Label label; } BRL;

    // Labels, denoting branch targets
    Label label;

    // Load instructions
    // -----------------

    // DMA vector at address specifed by register from DRAM into VPM
    // (local) memory.  To allow double buffering, i.e. the VPM to be
    // filled by DMA while also being read by a QPU, a flag is used to
    // indicate which one of two buffers in the VPM to use for the load
    struct { Reg addr; BufferAorB buffer; } LD1;

    // LD2 (wait for DMA read completion) has no parameters

    // Setup a read from VPM memory.  A flag indicates which one of
    // two buffers in the VPM is being used for the load
    struct { BufferAorB buffer; } LD3;
    
    // Transfer from VPM into given register
    struct { Reg dest; } LD4;

    // Store instructions
    // ------------------

    // Write the vector to VPM (local) memory using specified buffer
    struct { Reg data; BufferAorB buffer; } ST1;

    // DMA from the VPM out to DRAM at the address in given register.
    struct { Reg addr; BufferAorB buffer; } ST2;

    // ST3 (wait for DMA write completion) has no parameters

    // Semaphores
    // ----------

    // Semaphore id (range 0..15)
    int semaId;

    // Load receive via TMU
    // --------------------

    // Destination register for load receive
    struct { Reg dest; } RECV;

    // Print instructions
    // ------------------

    // Print string
    const char* PRS;

    // Print integer
    Reg PRI;

    // Print float
    Reg PRF;
  };
};

// Instruction id: also the index of an instruction
// in the main instruction sequence
typedef int InstrId;

// ============================================================================
// Handy functions
// ============================================================================

// Determine if instruction is a conditional assignment
bool isCondAssign(Instr* instr);

// Make a no-op
inline Instr nop()
  { Instr instr; instr.tag = NO_OP; return instr; }

// Instruction constructors
Instr genLI(Reg dst, int i);
Instr genMove(Reg dst, Reg src);
Instr genOR(Reg dst, Reg srcA, Reg srcB);
Instr genLShift(Reg dst, Reg srcA, int n);
Instr genIncr(Reg dst, Reg srcA, int n);

// Is last instruction in a basic block?
bool isLast(Instr instr);

// =========================
// Fresh variable generation
// =========================

Reg freshReg();

// ======================
// Fresh label generation
// ======================

// Obtain a fresh label
Label freshLabel();

// Number of fresh labels used
int getFreshLabelCount();

// Reset fresh label generator
void resetFreshLabelGen();
void resetFreshLabelGen(int val);

#endif

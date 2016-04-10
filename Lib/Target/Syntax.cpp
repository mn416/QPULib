#include <assert.h>
#include "Source/Syntax.h"
#include "Target/Syntax.h"

// =======
// Globals
// =======

// Used for fresh label generation
static int globalLabelId = 0;

// ======================
// Handy syntax functions
// ======================

// Determine if instruction is a conditional assignment
bool isCondAssign(Instr* instr)
{
  if (instr->tag == LI && instr->LI.cond.tag != ALWAYS)
    return true;
  if (instr->tag == ALU && instr->ALU.cond.tag != ALWAYS)
    return true;
  return false;
}

// Generate load-immediate instruction.

Instr genLI(Reg dst, int i)
{
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag           = LI;
  instr.LI.setFlags   = false;
  instr.LI.cond       = always;
  instr.LI.dest       = dst;
  instr.LI.imm.tag    = IMM_INT32;
  instr.LI.imm.intVal = i;
 
  return instr;
}

// Generate move instruction.

Instr genMove(Reg dst, Reg src)
{
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag           = ALU;
  instr.ALU.setFlags  = false;
  instr.ALU.cond      = always;
  instr.ALU.dest      = dst;
  instr.ALU.srcA.tag  = REG;
  instr.ALU.srcA.reg  = src;
  instr.ALU.op        = A_BOR;
  instr.ALU.srcB.tag  = REG;
  instr.ALU.srcB.reg  = src;

  return instr;
}

// Generate bitwise-or instruction.

Instr genOR(Reg dst, Reg srcA, Reg srcB)
{
  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag           = ALU;
  instr.ALU.setFlags  = false;
  instr.ALU.cond      = always;
  instr.ALU.dest      = dst;
  instr.ALU.srcA.tag  = REG;
  instr.ALU.srcA.reg  = srcA;
  instr.ALU.op        = A_BOR;
  instr.ALU.srcB.tag  = REG;
  instr.ALU.srcB.reg  = srcB;

  return instr;
}

// Generate left-shift instruction.

Instr genLShift(Reg dst, Reg srcA, int n)
{
  assert(n >= 0 && n <= 15);

  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond              = always;
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcA;
  instr.ALU.op                = A_SHL;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = n;

  return instr;
}

// Generate increment instruction.

Instr genIncr(Reg dst, Reg srcA, int n)
{
  assert(n >= 0 && n <= 15);

  AssignCond always;
  always.tag = ALWAYS;

  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = false;
  instr.ALU.cond              = always;
  instr.ALU.dest              = dst;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcA;
  instr.ALU.op                = A_ADD;
  instr.ALU.srcB.tag          = IMM;
  instr.ALU.srcB.smallImm.tag = SMALL_IMM;
  instr.ALU.srcB.smallImm.val = n;

  return instr;
}

// Is last instruction in a basic block?
bool isLast(Instr instr)
{
  return instr.tag == BRL || instr.tag == BR || instr.tag == END;
}

// =========================
// Fresh variable generation
// =========================

// Obtain a fresh variable
Reg freshReg()
{
  Var v = freshVar();
  Reg r;
  r.tag = REG_A;
  r.regId = v.id;
  return r;
}

// Obtain a fresh label
Label freshLabel()
{
  return globalLabelId++;
}

// Number of fresh labels
int getFreshLabelCount()
{
  return globalLabelId;
}

// Reset fresh label generator
void resetFreshLabelGen()
{
  globalLabelId = 0;
}

// Reset fresh label generator to specified value
void resetFreshLabelGen(int val)
{
  globalLabelId = val;
}

#include "Source/Int.h"
#include "Source/Stmt.h"
#include "Source/Float.h"

// ============================================================================
// Type 'IntExpr'
// ============================================================================

// Constructors

IntExpr::IntExpr() { this->expr = NULL; }

IntExpr::IntExpr(int x) { this->expr = mkIntLit(x); }

// Helper constructor

inline IntExpr mkIntExpr(Expr* e) { IntExpr x; x.expr = e; return x; }
inline FloatExpr mkFloatExpr(Expr* e) { FloatExpr x; x.expr = e; return x; }

// ============================================================================
// Type 'Int'
// ============================================================================

// Constructors

Int::Int() {
  Var v    = freshVar();
  this->expr = mkVar(v);
}

Int::Int(int x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, mkIntLit(x));
}

Int::Int(IntExpr e) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, e.expr);
}

// Copy constructors

Int::Int(Int& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}

Int::Int(const Int& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}

// Cast to an IntExpr

Int::operator IntExpr() { return mkIntExpr(this->expr); }

// Assignment

Int& Int::operator=(Int& rhs)
  { assign(this->expr, rhs.expr); return rhs; }

IntExpr Int::operator=(IntExpr rhs)
  { assign(this->expr, rhs.expr); return rhs; };

// ============================================================================
// Generic operations
// ============================================================================

inline IntExpr mkIntApply(IntExpr a,Op op,IntExpr b)
{
  Expr* e = mkApply(a.expr, op, b.expr);
  return mkIntExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Int from the UNIFORM FIFO.
IntExpr getUniformInt()
{
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = UNIFORM;
  return mkIntExpr(e);
}

// A vector containing integers 0..15
IntExpr index()
{
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = ELEM_NUM;
  return mkIntExpr(e);
}

// A vector containing the QPU id
IntExpr me()
{
  // There is reserved var holding the QPU ID.
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = STANDARD;
  e->var.id  = RSV_QPU_ID;
  return mkIntExpr(e);
}

// A vector containing the QPU count
IntExpr numQPUs()
{
  // There is reserved var holding the QPU count.
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = STANDARD;
  e->var.id  = RSV_NUM_QPUS;
  return mkIntExpr(e);
}

// Vector rotation
IntExpr rotate(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(ROTATE, INT32), b); }

FloatExpr rotate(FloatExpr a, IntExpr b)
{
  Expr* e = mkApply(a.expr, mkOp(ROTATE, FLOAT), b.expr);
  return mkFloatExpr(e);
}

// Add
IntExpr operator+(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(ADD, INT32), b); }

// Increment
void Int::operator++(int)
  { *this = *this + 1; }

// Subtract
IntExpr operator-(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(SUB, INT32), b); }

// Multiply
IntExpr operator*(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(MUL, INT32), b); }

// Min
IntExpr min(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(MIN, INT32), b); }

// Max
IntExpr max(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(MAX, INT32), b); }

// Shift left
IntExpr operator<<(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(SHL, INT32), b); }

// Shift Right
IntExpr operator>>(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(SHR, INT32), b); }

// Bitwise AND
IntExpr operator&(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(BAND, INT32), b); }

// Bitwise OR
IntExpr operator|(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(BOR, INT32), b); }

// Bitwise XOR
IntExpr operator^(IntExpr a, IntExpr b)
  { return mkIntApply(a, mkOp(BXOR, INT32), b); }

// Bitwise NOT
IntExpr operator~(IntExpr a)
  { return mkIntApply(a, mkOp(BNOT, INT32), a); }

// Conversion to Int
IntExpr toInt(FloatExpr a)
{
  Expr* e = mkApply(a.expr, mkOp(FtoI, INT32), mkIntLit(0));
  return mkIntExpr(e);
}

// Conversion to Float
FloatExpr toFloat(IntExpr a)
{
  Expr* e = mkApply(a.expr, mkOp(ItoF, FLOAT), mkIntLit(0));
  return mkFloatExpr(e);
}

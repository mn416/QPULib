#include "Source/Float.h"
#include "Source/Stmt.h"

// ============================================================================
// Type 'FloatExpr'
// ============================================================================

// Constructors

FloatExpr::FloatExpr() { this->expr = NULL; }

FloatExpr::FloatExpr(float x) { this->expr = mkFloatLit(x); }

// Helper constructor

inline FloatExpr mkFloatExpr(Expr* e) { FloatExpr x; x.expr = e; return x; }

// ============================================================================
// Type 'Float'
// ============================================================================

// Constructors

Float::Float() {
  Var v    = freshVar();
  this->expr = mkVar(v);
}

Float::Float(float x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, mkFloatLit(x));
}

Float::Float(FloatExpr e) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, e.expr);
}

// Copy constructors

Float::Float(Float& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}

Float::Float(const Float& x) {
  Var v    = freshVar();
  this->expr = mkVar(v);
  assign(this->expr, x.expr);
}

// Cast to an FloatExpr

Float::operator FloatExpr() { return mkFloatExpr(this->expr); }

// Assignment

Float& Float::operator=(Float& rhs)
  { assign(this->expr, rhs.expr); return rhs; }

FloatExpr Float::operator=(FloatExpr rhs)
  { assign(this->expr, rhs.expr); return rhs; };

// ============================================================================
// Generic operations
// ============================================================================

inline FloatExpr mkFloatApply(FloatExpr a,Op op,FloatExpr b)
{
  Expr* e = mkApply(a.expr, op, b.expr);
  return mkFloatExpr(e);
}

// ============================================================================
// Specific operations
// ============================================================================

// Read an Float from the UNIFORM FIFO.
FloatExpr getUniformFloat()
{
  Expr* e    = mkExpr();
  e->tag     = VAR;
  e->var.tag = UNIFORM;
  return mkFloatExpr(e);
}

// Add
FloatExpr operator+(FloatExpr a, FloatExpr b)
  { return mkFloatApply(a, mkOp(ADD, FLOAT), b); }

// Subtract
FloatExpr operator-(FloatExpr a, FloatExpr b)
  { return mkFloatApply(a, mkOp(SUB, FLOAT), b); }

// Multiply
FloatExpr operator*(FloatExpr a, FloatExpr b)
  { return mkFloatApply(a, mkOp(MUL, FLOAT), b); }

// Min
FloatExpr min(FloatExpr a, FloatExpr b)
  { return mkFloatApply(a, mkOp(MIN, FLOAT), b); }

// Max
FloatExpr max(FloatExpr a, FloatExpr b)
  { return mkFloatApply(a, mkOp(MAX, FLOAT), b); }

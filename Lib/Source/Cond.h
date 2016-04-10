#ifndef _SOURCE_COND_H_
#define _SOURCE_COND_H_

#include "Source/Syntax.h"
#include "Source/Int.h"

// ============================================================================
// Types                   
// ============================================================================

struct Cond
{
  // Abstract syntax tree
  CExpr* cexpr;
  // Constructor
  Cond(CExpr* c) { cexpr = c; }
};

struct BoolExpr
{
  // Abstract syntax tree
  BExpr* bexpr;
  // Constructor
  BoolExpr(BExpr* b) { bexpr = b; }
  // Cast to Cond
  //operator Cond();
};

// ============================================================================
// Generic 'Int' comparison
// ============================================================================

inline BoolExpr mkIntCmp(IntExpr a, CmpOp op, IntExpr b)
  { return BoolExpr(mkCmp(a.expr, op, b.expr)); }

// ============================================================================
// Specific 'Int' comparisons
// ============================================================================

inline BoolExpr operator==(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(EQ, INT32), b); }

inline BoolExpr operator!=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(NEQ, INT32), b); }

inline BoolExpr operator<(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(LT, INT32), b); }

inline BoolExpr operator<=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(LE, INT32), b); }

inline BoolExpr operator>(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(GT, INT32), b); }

inline BoolExpr operator>=(IntExpr a, IntExpr b)
  { return mkIntCmp(a, mkCmpOp(GE, INT32), b); }

// ============================================================================
// Generic 'Float' comparison
// ============================================================================

inline BoolExpr mkFloatCmp(FloatExpr a, CmpOp op, FloatExpr b)
  { return BoolExpr(mkCmp(a.expr, op, b.expr)); }

// ============================================================================
// Specific 'Float' comparisons
// ============================================================================

inline BoolExpr operator==(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(EQ, FLOAT), b); }

inline BoolExpr operator!=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(NEQ, FLOAT), b); }

inline BoolExpr operator<(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(LT, FLOAT), b); }

inline BoolExpr operator<=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(LE, FLOAT), b); }

inline BoolExpr operator>(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(GT, FLOAT), b); }

inline BoolExpr operator>=(FloatExpr a, FloatExpr b)
  { return mkFloatCmp(a, mkCmpOp(GE, FLOAT), b); }

// ============================================================================
// Boolean operators
// ============================================================================

inline BoolExpr operator!(BoolExpr a)
  { return BoolExpr(mkNot(a.bexpr)); }

inline BoolExpr operator&&(BoolExpr a, BoolExpr b)
  { return BoolExpr(mkAnd(a.bexpr, b.bexpr)); }

inline BoolExpr operator||(BoolExpr a, BoolExpr b)
  { return BoolExpr(mkOr(a.bexpr, b.bexpr)); }

inline Cond any(BoolExpr a)
  { return Cond(mkAny(a.bexpr)); }

inline Cond all(BoolExpr a)
  { return Cond(mkAll(a.bexpr)); }

#endif

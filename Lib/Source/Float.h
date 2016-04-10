// This module defines type 'Float' for a vector of 16 x 32-bit floats.

#ifndef _SOURCE_FLOAT_H_
#define _SOURCE_FLOAT_H_

#include <assert.h>
#include "Source/Syntax.h"

// ============================================================================
// Types                   
// ============================================================================

// An 'FloatExpr' defines an float vector expression which can
// only be used on the RHS of assignment statements.

struct FloatExpr {
  // Abstract syntax tree
  Expr* expr;
  // Constructors
  FloatExpr();
  FloatExpr(float x);
};

// An 'Float' defines a float vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Float {
  // Abstract syntax tree
  Expr* expr;

  // Constructors
  Float();
  Float(float x);
  Float(FloatExpr e);

  // Copy constructors
  Float(Float& x);
  Float(const Float& x);

  // Cast to an FloatExpr
  operator FloatExpr();

  // Assignment
  Float& operator=(Float& rhs);
  FloatExpr operator=(FloatExpr rhs);
};

// ============================================================================
// Operations
// ============================================================================

FloatExpr getUniformFloat();

FloatExpr operator+(FloatExpr a, FloatExpr b);
FloatExpr operator-(FloatExpr a, FloatExpr b);
FloatExpr operator*(FloatExpr a, FloatExpr b);
FloatExpr min(FloatExpr a, FloatExpr b);
FloatExpr max(FloatExpr a, FloatExpr b);

#endif

// This module defines type 'Int' for a vector of 16 x 32-bit integers.

#ifndef _SOURCE_INT_H_
#define _SOURCE_INT_H_

#include <assert.h>
#include "Source/Syntax.h"
#include "Source/Float.h"

// ============================================================================
// Types                   
// ============================================================================

// An 'IntExpr' defines an integer vector expression which can
// only be used on the RHS of assignment statements.

struct IntExpr {
  // Abstract syntax tree
  Expr* expr;
  // Constructors
  IntExpr();
  IntExpr(int x);
};

// An 'Int' defines an integer vector variable which can be used in
// both the LHS and RHS of an assignment.

struct Int {
  // Abstract syntax tree
  Expr* expr;

  // Constructors
  Int();
  Int(int x);
  Int(IntExpr e);

  // Copy constructors
  Int(Int& x);
  Int(const Int& x);

  // Cast to an IntExpr
  operator IntExpr();

  // Assignment
  Int& operator=(Int& rhs);
  IntExpr operator=(IntExpr rhs);

  // Increment
  void operator++(int);
};

// ============================================================================
// Operations
// ============================================================================

IntExpr getUniformInt();
IntExpr index();
IntExpr me();
IntExpr numQPUs();

IntExpr rotate(IntExpr a, IntExpr b);
FloatExpr rotate(FloatExpr a, IntExpr b);

IntExpr operator+(IntExpr a, IntExpr b);
IntExpr operator-(IntExpr a, IntExpr b);
IntExpr operator*(IntExpr a, IntExpr b);
IntExpr min(IntExpr a, IntExpr b);
IntExpr max(IntExpr a, IntExpr b);
IntExpr operator<<(IntExpr a, IntExpr b);
IntExpr operator>>(IntExpr a, IntExpr b);
IntExpr operator&(IntExpr a, IntExpr b);
IntExpr operator|(IntExpr a, IntExpr b);
IntExpr operator^(IntExpr a, IntExpr b);
IntExpr operator~(IntExpr a);
IntExpr toInt(FloatExpr a);
FloatExpr toFloat(IntExpr a);

#endif

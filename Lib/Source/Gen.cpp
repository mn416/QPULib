// A random source-program generator

#include <stdlib.h>
#include "Source/Gen.h"

// ============================================================================
// Types of expressions
// ============================================================================

// The types of the expressions we can generate are:

enum TypeTag { INT_TYPE, FLOAT_TYPE, PTR_TYPE, PTR2_TYPE };

struct Type {
  TypeTag tag;
  TypeTag ptrTo;
};

// ============================================================================
// Utilities for generating random numbers
// ============================================================================

// Return random integer in given range
int randRange(int min, int max)
{
  int r = rand();
  int n = r % (1+max-min);
  return min+n;
}

// ============================================================================
// Random literals
// ============================================================================

// Generate random integer literal
int genIntLit()
{
  if (randRange(0,10) == 0)
    return rand();
  else
    return randRange(-50, 50);
}

// Generate random float literal
float genFloatLit()
{
  float num   = (float) randRange(0, 1000);
  float denom = (float) randRange(1, 100);
  return num/denom;
}

// ============================================================================
// Random variables
// ============================================================================

Expr* pickVar(GenOptions* o, Type t)
{
  Var v;
  v.tag = STANDARD;
  int intHigh   = o->numIntArgs + o->numIntVars;
  int floatLow  = intHigh   + o->depth;
  int floatHigh = floatLow  + o->numFloatArgs + o->numFloatVars;
  int ptrHigh   = floatHigh + o->numPtrArgs;
  int ptr2High  = ptrHigh   + o->numPtr2Args;
  switch (t.tag) {
    case INT_TYPE:   v.id = randRange(0, intHigh-1); break;
    case FLOAT_TYPE: v.id = randRange(floatLow, floatHigh-1); break;
    case PTR_TYPE:   v.id = randRange(floatHigh, ptrHigh-1); break;
    case PTR2_TYPE:  v.id = randRange(ptrHigh, ptr2High-1); break;
    default:         assert(false);
  }
  return mkVar(v);
}

Expr* genVar(GenOptions* opts, Type t)
{
  // Sometimes pick a QPU special variable (namely ELEM_NUM)
  if (t.tag == INT_TYPE) {
    if (randRange(0, 5) == 0) {
      Var v;
      v.tag = ELEM_NUM;
      return mkVar(v);
    }
  }

  // Otherwise, pick a standard variable
  return pickVar(opts, t);
}

Expr* genLVar(GenOptions* opts, Type t)
{
  return pickVar(opts, t);
}

// ============================================================================
// Random operators
// ============================================================================

Op genOp(GenOptions* opts, Type t)
{
  Op op;
  switch (t.tag) {
    case INT_TYPE:
    case PTR_TYPE:
    case PTR2_TYPE:
      op.type = INT32;
      op.op   = (OpId) randRange(opts->genRotate ? ROTATE : ADD, BNOT);
      return op;

    case FLOAT_TYPE:
      op.type = FLOAT;
      op.op   = (OpId) randRange(opts->genRotate ? ROTATE : ADD, MAX);
      return op;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Random arithmetic expressions
// ============================================================================

Expr* genExpr(GenOptions* opts, Type t, int depth)
{
  switch (randRange(0, 3)) { 
    // Literal
    case 0: {
      if (t.tag == FLOAT_TYPE)
        return mkFloatLit(genFloatLit());
      else if (t.tag == INT_TYPE)
        return mkIntLit(genIntLit());
    }

    // Dereference
    case 1:
      if (depth > 0 && opts->genDeref && t.tag != PTR2_TYPE) {
        Type newType;
        if (t.tag == PTR_TYPE) {
          newType.tag = PTR2_TYPE;
          newType.ptrTo = t.ptrTo;
        }
        else {
          newType.tag = PTR_TYPE;
          newType.ptrTo = t.tag;
        }
        return mkDeref(genExpr(opts, newType, depth-1));

        if (randRange(0, 1) == 0) {
          // Generate: *p where p is a pointer variable
          return mkDeref(genVar(opts, newType));
        }
        else {
          // Generate: *(p+offset) where offset is a bounded expression
          Type intType;
          intType.tag = INT_TYPE;
          Expr* offset = mkApply(genExpr(opts, intType, depth-1),
                                 mkOp(BAND, INT32),
                                 mkIntLit(opts->derefOffsetMask));
          Expr* ptr    = mkApply(genVar(opts, newType),
                                 mkOp(ADD, INT32),
                                 offset);
          return mkDeref(ptr);
        }
      }

    // Application
    case 2:
      if (depth > 0) {
        // Sometimes generate a type conversion operation
        if (randRange(0, 9) == 0) {
          if (t.tag == INT_TYPE && opts->genFloat) {
            Op op = mkOp(FtoI, INT32);
            Type floatType;
            floatType.tag = FLOAT_TYPE;
            Expr* e = genExpr(opts, floatType, depth-1);
            return mkApply(e, op, mkIntLit(0));
          }
          else if (t.tag == FLOAT_TYPE) {
            Op op = mkOp(ItoF, FLOAT);
            Type intType;
            intType.tag = INT_TYPE;
            Expr* e = genExpr(opts, intType, depth-1);
            return mkApply(e, op, mkIntLit(0));
          }
        }
        // Otherwise, generate random operator application
        Expr* e1 = genExpr(opts, t, depth-1);
        Expr* e2 = genExpr(opts, t, depth-1);
        return mkApply(e1, genOp(opts, t), e2);
      }

    // Variable 
    case 3: return genVar(opts, t);
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Random boolean expressions
// ============================================================================

BExpr* genBExpr(GenOptions* opts, int depth)
{
  switch (randRange(NOT, CMP)) {
    // Negation
    case NOT:
      if (depth > 0)
        return mkNot(genBExpr(opts, depth-1));

    // Conjunction
    case AND:
      if (depth > 0)
        return mkAnd(genBExpr(opts, depth-1),
                     genBExpr(opts, depth-1));

    // Disjunction
    case OR:
      if (depth > 0)
        return mkOr(genBExpr(opts, depth-1),
                    genBExpr(opts, depth-1));

    // Comparison
    case CMP: {
      CmpOp op = mkCmpOp((CmpOpId) randRange(EQ, GE), INT32);
      if (opts->genFloat && randRange(0, 2) == 0) {
        // Floating-point comparison
        op.type = FLOAT;
        Type floatType; floatType.tag = FLOAT_TYPE;
        return mkCmp(genExpr(opts, floatType, depth-1), op,
                     genExpr(opts, floatType, depth-1));
      }
      else {
        // Integer comparison
        Type intType; intType.tag = INT_TYPE;
        return mkCmp(genExpr(opts, intType, depth-1), op,
                     genExpr(opts, intType, depth-1));
      }
    }
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Random conditional expressions
// ============================================================================

CExpr* genCExpr(GenOptions* opts, int depth)
{
  switch (randRange(ALL, ANY)) {
    case ALL: return mkAll(genBExpr(opts, depth));
    case ANY: return mkAny(genBExpr(opts, depth));
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Random types
// ============================================================================

Type genType(GenOptions* opts, bool allowPointers)
{
  Type t;
  if (randRange(0,1) == 0 && opts->genFloat)
    t.tag = FLOAT_TYPE;
  else
    t.tag = INT_TYPE;

  // Sometimes create a pointer type
  if (opts->genDeref && allowPointers) {
    switch (randRange(0, 5)) {
      // Pointer
      case 0:
        t.ptrTo = t.tag;
        t.tag   = PTR_TYPE;
        break;

      // Pointer to a pointer
      case 1:
        t.ptrTo = t.tag;
        t.tag   = PTR2_TYPE;
        break;
    }
  }

  return t;
}

// ============================================================================
// Random assignment statements
// ============================================================================

// Generate left-hand side of assignment statement
Expr* genLValue(GenOptions* opts, Type t, int depth)
{
  // Disallow modification of pointers
  // (that would make automated testing rather tricky)
  assert(t.tag == INT_TYPE || t.tag == FLOAT_TYPE);

  if (randRange(0, 1) == 0 && opts->genDeref) {
    // Generate dereferenced expression
    t.tag   = PTR_TYPE;
    t.ptrTo = t.tag;
    return mkDeref(genExpr(opts, t, depth-1));
  }
  else {
    // Generate variable
    return genLVar(opts, t);
  }
}

// Generate assignment statement
Stmt* genAssign(GenOptions* opts, int depth)
{
  // Generate random type (disallowing pointer types)
  Type t = genType(opts, false);

  // Generate assignment statment
  return mkAssign(genLValue(opts, t, depth),
                  genExpr(opts, t, depth));
}

// ============================================================================
// Random conditional assignments
// ============================================================================

Stmt* genWhere(GenOptions* opts, int depth, int length)
{
  switch (randRange(0, 3)) {
    // Sequential composition
    case 0:
      if (length > 0)
        return mkSeq(genWhere(opts, depth, 0),
                     genWhere(opts, depth, length-1));

    // Nested where
    case 1:
      if (depth > 0)
        return mkWhere(genBExpr(opts, depth),
                       genWhere(opts, depth-1, opts->length),
                       genWhere(opts, depth-1, opts->length));

    // Skip
    case 2:
      return mkSkip();

    // Assignment
    case 3:
      return genAssign(opts, depth);
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Random while loops
// ============================================================================

// Prototype for function to be defined later.
Stmt* genStmt(GenOptions* opts, int depth, int length);

Stmt* genWhile(GenOptions* o, int depth)
{
  assert(depth > 0 && depth <= o->depth);

  // Obtain a loop variable
  int firstLoopVar = o->numIntArgs + o->numIntVars;
  Var var;
  var.tag = STANDARD;
  var.id  = firstLoopVar + (depth-1);
  Expr* v = mkVar(var);

  // Create random condition with loop bound
  BExpr* b = mkAnd(genBExpr(o, depth),
               mkCmp(v, mkCmpOp(LT, INT32),
                          mkIntLit(o->loopBound)));
  CExpr* c = randRange(0, 1) == 0 ? mkAny(b) : mkAll(b);

  // Initialise loop variable
  Stmt* init = mkAssign(v, mkIntLit(0));

  // Create loop increment
  Stmt* inc = mkAssign(v, mkApply(v, mkOp(ADD, INT32), mkIntLit(1)));

  // Create random loop body with loop increment
  Stmt* body = mkSeq(genStmt(o, depth-1, o->length), inc);

  return mkSeq(init, mkWhile(c, body));
}

// ============================================================================
// Random print statements
// ============================================================================

Stmt* genPrint(GenOptions* opts, int depth)
{
  // Generate random type (disallowing pointer types)
  Type t = genType(opts, false);
  // Generate random expression and print its value
  return mkPrint(t.tag == INT_TYPE ? PRINT_INT : PRINT_FLOAT,
                   genExpr(opts, t, depth));
}

// ============================================================================
// Random statements
// ============================================================================

// Generate statement
Stmt* genStmt(GenOptions* opts, int depth, int length)
{
  switch (randRange(SKIP, PRINT)) {
    // Sequential composition
    case SEQ:
      if (length > 0)
        return mkSeq(genStmt(opts, depth, 0),
                     genStmt(opts, depth, length-1));

    // Where statement
    case WHERE:
      if (length > 0 && depth > 0)
        return mkWhere(genBExpr(opts, depth),
                       genWhere(opts, depth-1, opts->length),
                       genWhere(opts, depth-1, opts->length));

    // If statement
    case IF:
      if (length > 0 && depth > 0)
        return mkIf(genCExpr(opts, depth),
                    genStmt(opts, depth-1, opts->length),
                    genStmt(opts, depth-1, opts->length));

    // While statement
    case WHILE:
      if (length > 0 && depth > 0)
        return genWhile(opts, depth);

    // Print statement
    case PRINT:
      return genPrint(opts, depth);

    // No-op
    case SKIP:
      return mkSkip();

    // Assignment
    case ASSIGN:
      return genAssign(opts, depth);

  }

  // Not reachable
  assert(false);
}

// ============================================================================
// Top-level program generator
// ============================================================================

Stmt* progGen(GenOptions* opts, int* numVars)
{
  // Initialise variables
  Stmt* pre  = mkSkip();
  Stmt* post = mkSkip();

  // Argument FIFO
  Var uniform;
  uniform.tag = UNIFORM;
  Expr* fifo  = mkVar(uniform);

  // Next unused var id
  VarId next = 0;
  Var v;
  v.tag = STANDARD;

  // Read int args
  for (int i = 0; i < opts->numIntArgs; i++) {
    v.id  = next++;
    pre   = mkSeq(pre, mkAssign(mkVar(v), fifo));
    post  = mkSeq(post, mkPrint(PRINT_INT, mkVar(v)));
  }

  // Initialise int vars and loop vars
  for (int i = 0; i < opts->numIntVars + opts->depth; i++) {
    v.id  = next++;
    pre   = mkSeq(pre, mkAssign(mkVar(v), mkIntLit(genIntLit())));
    post  = mkSeq(post, mkPrint(PRINT_INT, mkVar(v)));
  }

  // Read float args
  for (int i = 0; i < opts->numFloatArgs; i++) {
    v.id  = next++;
    pre   = mkSeq(pre, mkAssign(mkVar(v), fifo));
    post  = mkSeq(post, mkPrint(PRINT_FLOAT, mkVar(v)));
  }

  // Initialise float vars
  for (int i = 0; i < opts->numFloatVars; i++) {
    v.id  = next++;
    pre   = mkSeq(pre, mkAssign(mkVar(v), mkFloatLit(genFloatLit())));
    post  = mkSeq(post, mkPrint(PRINT_FLOAT, mkVar(v)));
  }
 
  // Read pointer args
  for (int i = 0; i < opts->numPtrArgs + opts->numPtr2Args; i++) {
    v.id  = next++;
    pre   = mkSeq(pre, mkAssign(mkVar(v), fifo));
  }

  // Generate statement
  Stmt* s = genStmt(opts, opts->depth, opts->length);
  *numVars = next;

  return mkSeq(pre, mkSeq(s, post));
}

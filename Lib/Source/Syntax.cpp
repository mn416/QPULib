#include "Source/Syntax.h"
#include "Common/Heap.h"
#include "Common/Stack.h"
#include "Params.h"

// ============================================================================
// Globals
// ============================================================================

// Used for fresh variable generation
static int globalVarId = 0;

// Used for constructing abstract syntax trees
Heap        astHeap("abstract syntax tree", AST_HEAP_SIZE);
Stack<Stmt> stmtStack;
Stack<Stmt> controlStack;

// ============================================================================
// Functions on global variables
// ============================================================================

// Obtain a fresh variable
Var freshVar()
{
  // Return a fresh standard variable
  Var v;
  v.tag = STANDARD;
  v.id  = globalVarId++;
  return v;
}

// Number of fresh vars
int getFreshVarCount()
{
  return globalVarId;
}

// Reset fresh variable generator
void resetFreshVarGen()
{
  globalVarId = 0;
}

// Reset fresh variable generator to specified value
void resetFreshVarGen(int val)
{
  globalVarId = val;
}

// ============================================================================
// Functions on operators
// ============================================================================

Op mkOp(OpId op, BaseType type) {
  Op o;
  o.op   = op;
  o.type = type;
  return o;
}

CmpOp mkCmpOp(CmpOpId op, BaseType type) {
  CmpOp o;
  o.op   = op;
  o.type = type;
  return o;
}

// Is operator unary?
bool isUnary(Op op)
{
  return (op.op == BNOT || op.op == ItoF || op.op == FtoI);
}

// Is given operator commutative?
bool isCommutative(Op op)
{
  if (op.type != FLOAT) {
    return op.op == ADD
        || op.op == MUL
        || op.op == BOR
        || op.op == BAND
        || op.op == BXOR
        || op.op == MIN
        || op.op == MAX;
  }
  return false;
}

// ============================================================================
// Functions on expressions
// ============================================================================

// Function to allocate an expression
Expr* mkExpr()
{
  return astHeap.alloc<Expr>();
}

// Make an integer literal
Expr* mkIntLit(int lit)
{
  Expr* e   = mkExpr();
  e->tag    = INT_LIT;
  e->intLit = lit;
  return e;
}

// Make a float literal
Expr* mkFloatLit(float lit)
{
  Expr* e     = mkExpr();
  e->tag      = FLOAT_LIT;
  e->floatLit = lit;
  return e;
}

// Make a variable
Expr* mkVar(Var var)
{
  Expr* e = mkExpr();
  e->tag  = VAR;
  e->var  = var;
  return e;
}

// Make an operator application
Expr* mkApply(Expr* lhs, Op op, Expr* rhs)
{
  Expr* e      = mkExpr();
  e->tag       = APPLY;
  e->apply.lhs = lhs;
  e->apply.op  = op;
  e->apply.rhs = rhs;
  return e;
}

// Make a pointer dereference
Expr* mkDeref(Expr* ptr)
{
  Expr* e      = mkExpr();
  e->tag       = DEREF;
  e->deref.ptr = ptr;
  return e;
}

// Is an expression a literal?
bool isLit(Expr* e)
{
  return (e->tag == INT_LIT) || (e->tag == FLOAT_LIT);
}

// ============================================================================
// Functions on boolean expressions
// ============================================================================

// Allocate a boolean expression
BExpr* mkBExpr()
{
  return astHeap.alloc<BExpr>();
}

BExpr* mkNot(BExpr* neg)
{
  BExpr *b    = mkBExpr();
  b->tag      = NOT;
  b->neg      = neg;
  return b;
}

BExpr* mkAnd(BExpr* lhs, BExpr* rhs)
{
  BExpr *b    = mkBExpr();
  b->tag      = AND;
  b->conj.lhs = lhs;
  b->conj.rhs = rhs;
  return b;
}

BExpr* mkOr(BExpr* lhs, BExpr* rhs)
{
  BExpr *b    = mkBExpr();
  b->tag      = OR;
  b->disj.lhs = lhs;
  b->disj.rhs = rhs;
  return b;
}

BExpr* mkCmp(Expr* lhs, CmpOp op, Expr*  rhs)
{
  BExpr *b    = mkBExpr();
  b->tag      = CMP;
  b->cmp.lhs  = lhs;
  b->cmp.op   = op;
  b->cmp.rhs  = rhs;
  return b;
}

// ============================================================================
// Functions on conditionals
// ============================================================================

CExpr* mkCExpr()
{
  return astHeap.alloc<CExpr>();
}

CExpr* mkAll(BExpr* bexpr)
{
  CExpr* c = mkCExpr();
  c->tag   = ALL;
  c->bexpr = bexpr;
  return c;
}

CExpr* mkAny(BExpr* bexpr)
{
  CExpr* c = mkCExpr();
  c->tag   = ANY;
  c->bexpr = bexpr;
  return c;
}

// ============================================================================
// Functions on statements
// ============================================================================

// Functions to allocate a statement
Stmt* mkStmt()
{
  return astHeap.alloc<Stmt>();
}

// Make a skip statement
Stmt* mkSkip()
{
  Stmt* s = mkStmt();
  s->tag = SKIP;
  return s;
}

// Make an assignment statement
Stmt* mkAssign(Expr* lhs, Expr* rhs)
{
  Stmt* s       = mkStmt();
  s->tag        = ASSIGN;
  s->assign.lhs = lhs;
  s->assign.rhs = rhs;
  return s;
}

// Make a sequential composition
Stmt* mkSeq(Stmt *s0, Stmt* s1)
{
  Stmt* s   = mkStmt();
  s->tag    = SEQ;
  s->seq.s0 = s0;
  s->seq.s1 = s1;
  return s;
}

Stmt* mkWhere(BExpr* cond, Stmt* thenStmt, Stmt* elseStmt)
{
  Stmt* s           = mkStmt();
  s->tag            = WHERE;
  s->where.cond     = cond;
  s->where.thenStmt = thenStmt;
  s->where.elseStmt = elseStmt;
  return s;
}

Stmt* mkIf(CExpr* cond, Stmt* thenStmt, Stmt* elseStmt)
{
  Stmt* s            = mkStmt();
  s->tag             = IF;
  s->ifElse.cond     = cond;
  s->ifElse.thenStmt = thenStmt;
  s->ifElse.elseStmt = elseStmt;
  return s;
}

Stmt* mkWhile(CExpr* cond, Stmt* body)
{
  Stmt* s      = mkStmt();
  s->tag       = WHILE;
  s->loop.cond = cond;
  s->loop.body = body;
  return s;
}

Stmt* mkFor(CExpr* cond, Stmt* inc, Stmt* body)
{
  Stmt* s         = mkStmt();
  s->tag          = FOR;
  s->forLoop.cond = cond;
  s->forLoop.inc  = inc;
  s->forLoop.body = body;
  return s;
}

Stmt* mkPrint(PrintTag t, Expr* e)
{
  Stmt* s       = mkStmt();
  s->tag        = PRINT;
  s->print.tag  = t;
  s->print.expr = e;
  return s;
}

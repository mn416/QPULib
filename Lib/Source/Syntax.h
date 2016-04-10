// This module defines the abstract syntax of the QPU language.

#ifndef _SOURCE_SYNTAX_H_
#define _SOURCE_SYNTAX_H_

#include "Common/Heap.h"
#include "Common/Stack.h"

// ============================================================================
// Operators
// ============================================================================

// Operator id
// (Note: order of operators is important to the random generator.)
enum OpId {
  // Int & Float operators:
  ROTATE, ADD, SUB, MUL, MIN, MAX,

  // Int only operators:
  SHL, SHR, BOR, BAND, BXOR, BNOT,

  // Conversion operators:
  ItoF, FtoI
};

// Every operator has a type associated with it
enum BaseType { UINT8, INT16, INT32, FLOAT };

// Pair containing operator and base type
struct Op { OpId op; BaseType type; };

// Construct an 'Op'
Op mkOp(OpId op, BaseType type);

// Is operator unary?
bool isUnary(Op op);

// Is operator commutative?
bool isCommutative(Op op);

// ============================================================================
// Variables
// ============================================================================

// What kind of variable is it
enum VarTag {
    STANDARD     // A standard variable that can be stored
                 // in a general-purpose register on a QPU
  , UNIFORM      // (Read-only.)  Reading this variable will consume a value
                 // (replicated 16 times) from the QPU's UNIFORM FIFO
                 // (this is how parameters are passed to kernels).
  , QPU_NUM      // (Read-only.) Reading this variable will yield the
                 // QPU's unique id (replicated 16 times).
  , ELEM_NUM     // (Read-only.) Reading this variable will yield a vector
                 // containing the integers from 0 to 15.
  , TMU0_ADDR    // (Write-only.) Initiate load via TMU
};

typedef int VarId;

struct Var {
  VarTag tag;

  // A unique identifier for a standard variable
  VarId id;
};

// Reserved general-purpose vars
enum ReservedVarId {
  RSV_QPU_ID       = 0,
  RSV_NUM_QPUS     = 1,
  RSV_READ_STRIDE  = 2,
  RSV_WRITE_STRIDE = 3
};

// ============================================================================
// Expressions    
// ============================================================================

// What kind of expression is it?
enum ExprTag { INT_LIT, FLOAT_LIT, VAR, APPLY, DEREF };

struct Expr {
  // What kind of expression is it?
  ExprTag tag;

  union {
    // Integer literal
    int intLit;

    // Float literal
    float floatLit;

    // Variable identifier
    Var var;

    // Application of a binary operator
    struct { Expr* lhs; Op op; Expr* rhs; } apply;

    // Dereference a pointer
    struct { Expr* ptr; } deref;
  };
};

// Functions to construct expressions
Expr* mkExpr();
Expr* mkIntLit(int lit);
Expr* mkFloatLit(float lit);
Expr* mkVar(Var var);
Expr* mkApply(Expr* lhs, Op op, Expr* rhs);
Expr* mkDeref(Expr* ptr);

// Is an expression a literal?
bool isLit(Expr* e);

// ============================================================================
// Comparison operators
// ============================================================================

// Comparison operators
enum CmpOpId { EQ, NEQ, LT, GT, LE, GE };

// Pair containing comparison operator and base type
struct CmpOp { CmpOpId op; BaseType type; };

// Construct an 'Op'
CmpOp mkCmpOp(CmpOpId op, BaseType type);

// ============================================================================
// Boolean expressions
// ============================================================================

// Kinds of boolean expressions
enum BExprTag { NOT, AND, OR, CMP };

struct BExpr {
  // What kind of boolean expression is it?
  BExprTag tag;

  union {
    // Negation
    BExpr* neg;

    // Conjunction
    struct { BExpr* lhs; BExpr* rhs; } conj;

    // Disjunction
    struct { BExpr* lhs; BExpr* rhs; } disj;

    // Comparison
    struct { Expr* lhs; CmpOp op; Expr* rhs; } cmp;
  };
};

// Functions to construct boolean expressions
BExpr* mkBExpr();
BExpr* mkNot(BExpr* neg);
BExpr* mkAnd(BExpr* lhs, BExpr* rhs);
BExpr* mkOr (BExpr* lhs, BExpr* rhs);
BExpr* mkCmp(Expr*  lhs, CmpOp op, Expr*  rhs);

// ============================================================================
// Conditional expressions
// ============================================================================

// Kinds of conditional expressions
enum CExprTag { ALL, ANY };

struct CExpr {
  // What kind of boolean expression is it?
  CExprTag tag;

  // This is either a scalar boolean expression, or a reduction of a vector
  // boolean expressions using 'any' or 'all' operators.
  BExpr* bexpr;
};

// Functions to construct conditional expressions
CExpr* mkCExpr();
CExpr* mkAll(BExpr* bexpr);
CExpr* mkAny(BExpr* bexpr);

// ============================================================================
// 'print' statements
// ============================================================================

// For displaying values in emulation
enum PrintTag { PRINT_INT, PRINT_FLOAT, PRINT_STR };

struct PrintStmt {
  PrintTag tag;
  union {
    const char* str;
    Expr* expr;
  };
};

// ============================================================================
// Statements
// ============================================================================

// What kind of statement is it?
enum StmtTag {
  SKIP, ASSIGN, SEQ, WHERE,
  IF, WHILE, PRINT, FOR,
  SET_READ_STRIDE, SET_WRITE_STRIDE,
  LOAD_RECEIVE, STORE_REQUEST, FLUSH,
  SEND_IRQ_TO_HOST, SEMA_INC, SEMA_DEC };

struct Stmt {
  // What kind of statement is it?
  StmtTag tag;

  union {
    // Assignment
    struct { Expr* lhs; Expr* rhs; } assign;

    // Sequential composition
    struct { Stmt* s0; Stmt* s1; } seq;

    // Where
    struct { BExpr* cond; Stmt* thenStmt; Stmt* elseStmt; } where;

    // If
    struct { CExpr* cond; Stmt* thenStmt; Stmt* elseStmt; } ifElse;

    // While
    struct { CExpr* cond; Stmt* body; } loop;

    // For (only used intermediately during AST construction)
    struct { CExpr* cond; Stmt* inc; Stmt* body; } forLoop;

    // Print
    PrintStmt print;

    // Set stride
    Expr* stride;

    // Load receive destination
    Expr* loadDest;

    // Store request
    struct { Expr* data; Expr* addr; } storeReq;

    // Semaphore id for increment / decrement
    int semaId;
  };
};

// Functions to construct statements
Stmt* mkStmt();
Stmt* mkSkip();
Stmt* mkAssign(Expr* lhs, Expr* rhs);
Stmt* mkSeq(Stmt* s0, Stmt* s1);
Stmt* mkWhere(BExpr* cond, Stmt* thenStmt, Stmt* elseStmt);
Stmt* mkIf(CExpr* cond, Stmt* thenStmt, Stmt* elseStmt);
Stmt* mkWhile(CExpr* cond, Stmt* body);
Stmt* mkFor(CExpr* cond, Stmt* inc, Stmt* body);
Stmt* mkPrint(PrintTag t, Expr* e);

// ============================================================================
// Global variables
// ============================================================================

// Obtain a fresh variable
Var freshVar();

// Number of fresh vars used
int getFreshVarCount();

// Reset fresh variable generator
void resetFreshVarGen();
void resetFreshVarGen(int val);

// Used for constructing abstract syntax trees
extern Heap        astHeap;
extern Stack<Stmt> stmtStack;
extern Stack<Stmt> controlStack;

#endif

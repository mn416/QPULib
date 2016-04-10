#include "Source/Syntax.h"
#include "Target/Syntax.h"
#include "Target/SmallLiteral.h"
#include "Target/LoadStore.h"
#include "Common/Seq.h"

// ============================================================================
// Opcodes and operands
// ============================================================================

// Translate source operator to target opcode
ALUOp opcode(Op op)
{
  if (op.type == FLOAT) {
    switch (op.op) {
      case ADD:    return A_FADD;
      case SUB:    return A_FSUB;
      case MUL:    return M_FMUL;
      case MIN:    return A_FMIN;
      case MAX:    return A_FMAX;
      case ItoF:   return A_ItoF;
      case ROTATE: return M_ROTATE;
      default:     assert(false);
    }
  }
  else {
    switch (op.op) {
      case ADD:    return A_ADD;
      case SUB:    return A_SUB;
      case MUL:    return M_MUL24;
      case MIN:    return A_MIN;
      case MAX:    return A_MAX;
      case FtoI:   return A_FtoI;
      case SHL:    return A_SHL;
      case SHR:    return A_ASR;
      case BAND:   return A_BAND;
      case BOR:    return A_BOR;
      case BXOR:   return A_BXOR;
      case BNOT:   return A_BNOT;
      case ROTATE: return M_ROTATE;
      default:     assert(false);
    }
  }
}

// Translate variable to source register.
Reg srcReg(Var v)
{
  Reg r;
  switch (v.tag) {
    case UNIFORM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_UNIFORM;
      return r;
    case QPU_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_QPU_NUM;
      return r;
    case ELEM_NUM:
      r.tag     = SPECIAL;
      r.regId   = SPECIAL_ELEM_NUM;
      return r;
    case STANDARD:
      r.tag   = REG_A;
      r.regId = v.id;
      return r;
  }

  // Not reachable
  assert(false);
}

// Translate variable to target register.
Reg dstReg(Var v)
{
  Reg r;
  switch (v.tag) {
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
      printf("QPULib: writing to read-only special register is forbidden\n");
      assert(false);
    case STANDARD:
      r.tag   = REG_A;
      r.regId = v.id;
      return r;
    case TMU0_ADDR:
      r.tag = SPECIAL;
      r.regId = SPECIAL_TMU0_S;
      return r;
  }

  // Not reachable
  assert(false);
}

// Translate the argument of an operator (either a variable or a small
// literal) to a target operand.

RegOrImm operand(Expr* e)
{
  RegOrImm x;
  if (e->tag == VAR) {
    x.tag = REG;
    x.reg = srcReg(e->var);
    return x;
  }
  int enc = encodeSmallLit(e);
  assert(enc >= 0);
  x.tag          = IMM;
  x.smallImm.tag = SMALL_IMM;
  x.smallImm.val = enc;
  return x;
}

// ============================================================================
// 'Simple' expressions
// ============================================================================

// An expression is 'simple' if it is a small literal (see
// Target/SmallLiteral.cpp) or a variable.

bool isSimple(Expr* e)
{
  return (e->tag == VAR) || isSmallLit(e);
}

// Translate an expression to a simple expressions, generating
// instructions along the way.  (Prototype here, implementation below,
// since this and 'varAssign' are mutually recursive.)

Expr* simplify(Seq<Instr>* seq, Expr* e);

// Similar to 'simplify' but ensure that the result is a variable.

Expr* putInVar(Seq<Instr>* seq, Expr* e);

// ============================================================================
// Variable assignments
// ============================================================================

// Translate the conditional assignment of a variable to an expression.

void varAssign( Seq<Instr>* seq   // Target instruction sequence to extend
              , AssignCond cond   // Condition on assignment
              , Var v             // Variable on LHS
              , Expr* expr        // Expression on RHS
              )
{
  Expr e = *expr;

  // -----------------------------------------
  // Case: v := w, where v and w are variables
  // -----------------------------------------
  if (e.tag == VAR) {
    Var w = e.var;
    Instr instr;
    instr.tag                   = ALU;
    instr.ALU.setFlags          = false;
    instr.ALU.cond              = cond;
    instr.ALU.dest              = dstReg(v);
    instr.ALU.srcA.tag          = REG;
    instr.ALU.srcA.reg          = srcReg(w);
    instr.ALU.op                = A_BOR;
    instr.ALU.srcB.tag          = REG;
    instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
    seq->append(instr);
    return;
  }

  // -------------------------------------------
  // Case: v := i, where i is an integer literal
  // -------------------------------------------
  if (e.tag == INT_LIT) {
    int i = e.intLit;
    Instr instr;
    instr.tag           = LI;
    instr.LI.setFlags   = false;
    instr.LI.cond       = cond;
    instr.LI.dest       = dstReg(v);
    instr.LI.imm.tag    = IMM_INT32;
    instr.LI.imm.intVal = i;
    seq->append(instr);
    return;
  }

  // ----------------------------------------
  // Case: v := f, where f is a float literal
  // ----------------------------------------
  if (e.tag == FLOAT_LIT) {
    float f = e.floatLit;
    Instr instr;
    instr.tag             = LI;
    instr.LI.setFlags     = false;
    instr.LI.cond         = cond;
    instr.LI.dest         = dstReg(v);
    instr.LI.imm.tag      = IMM_FLOAT32;
    instr.LI.imm.floatVal = f;
    seq->append(instr);
    return;
  }

  // ----------------------------------------------
  // Case: v := x op y, where x or y are not simple
  // ----------------------------------------------
  if (e.tag == APPLY &&
            (!isSimple(e.apply.lhs) ||
             !isSimple(e.apply.rhs))) {
    e.apply.lhs = simplify(seq, e.apply.lhs);
    e.apply.rhs = simplify(seq, e.apply.rhs);
  }

  // --------------------------------------------------
  // Case: v := x op y, where x and y are both literals
  // --------------------------------------------------
  if (e.tag == APPLY && isLit(e.apply.lhs) && isLit(e.apply.rhs)) {
    Var tmpVar = freshVar();
    varAssign(seq, cond, tmpVar, e.apply.lhs);
    e.apply.lhs = mkVar(tmpVar);
  }
 
  // -------------------------------------------
  // Case: v := x op y, where x and y are simple
  // -------------------------------------------
  if (e.tag == APPLY) {
    Instr instr;
    instr.tag            = ALU;
    instr.ALU.setFlags   = false;
    instr.ALU.cond       = cond;
    instr.ALU.dest       = dstReg(v);
    instr.ALU.srcA       = operand(e.apply.lhs);
    instr.ALU.op         = opcode(e.apply.op);
    instr.ALU.srcB       = operand(e.apply.rhs);
    seq->append(instr);
    return;
  }

  // ---------------------------------------
  // Case: v := *w where w is not a variable
  // ---------------------------------------
  if (e.tag == DEREF &&
           e.deref.ptr->tag != VAR) {
    assert(!isLit(e.deref.ptr));
    e.deref.ptr = simplify(seq, e.deref.ptr);
  }

  // -----------------------------------
  // Case: v := *w where w is a variable
  // -----------------------------------
  //
  // Restriction: we disallow dereferencing in conditional ('where')
  // assignments for simplicity.  In most (all?) cases it should be
  // trivial to lift these outside the 'where'.
  //
  if (e.tag == DEREF) {
    if (cond.tag != ALWAYS) {
      printf("QPULib: dereferencing not yet supported inside 'where'\n");
      assert(false);
    }
    Instr instr;
    instr.tag        = LD1;
    instr.LD1.addr   = srcReg(e.deref.ptr->var);
    instr.LD1.buffer = A;
    seq->append(instr);
    instr.tag        = LD2;
    seq->append(instr);
    instr.tag        = LD3;
    instr.LD3.buffer = A;
    seq->append(instr);
    instr.tag        = LD4;
    instr.LD4.dest   = dstReg(v);
    seq->append(instr);
    return;
  }

  // This case should not be reachable
  assert(false);
}

// Translate an expression to a simple expression, generating
// instructions along the way.

Expr* simplify(Seq<Instr>* seq, Expr* e)
{
  if (!isSimple(e)) {
    AssignCond always;
    always.tag = ALWAYS;
    Var tmp = freshVar();
    varAssign(seq, always, tmp, e);
    return mkVar(tmp);
  }
  else
    return e;
}

// Similar to 'simplify' but ensure that the result is a variable.

Expr* putInVar(Seq<Instr>* seq, Expr* e)
{
  if (e->tag != VAR) {
    AssignCond always;
    always.tag = ALWAYS;
    Var tmp = freshVar();
    varAssign(seq, always, tmp, e);
    return mkVar(tmp);
  }
  else
    return e;
}

// ============================================================================
// Assignment statements
// ============================================================================

void assign( Seq<Instr>* seq   // Target instruction sequence to extend
           , Expr *lhsExpr     // Expression on left-hand side
           , Expr *rhs         // Expression on right-hand side
           )
{
  Expr lhs = *lhsExpr;

  AssignCond always;
  always.tag = ALWAYS;

  // -----------------------------------------------------------
  // Case: v := rhs, where v is a variable and rhs an expression
  // -----------------------------------------------------------
  if (lhs.tag == VAR) {
    varAssign(seq, always, lhs.var, rhs);
    return;
  }

  // ---------------------------------------------------------
  // Case: *lhs := rhs where lhs is not a var or rhs not a var
  // ---------------------------------------------------------
  if (lhs.tag == DEREF &&
        (lhs.deref.ptr->tag != VAR ||
         rhs->tag != VAR)) {
    assert(!isLit(lhs.deref.ptr));
    lhs.deref.ptr = simplify(seq, lhs.deref.ptr);
    rhs = putInVar(seq, rhs);
  }

  // -------------------------------------------------
  // Case: *v := rhs where v is a var and rhs is a var
  // -------------------------------------------------
  if (lhs.tag == DEREF) {
    Instr instr;
    instr.tag        = ST1;
    instr.ST1.data   = srcReg(rhs->var);
    instr.ST1.buffer = A;
    seq->append(instr);
    instr.tag        = ST2;
    instr.ST2.addr   = srcReg(lhs.deref.ptr->var);
    instr.ST2.buffer = A;
    seq->append(instr);
    instr.tag        = ST3;
    seq->append(instr);
    return;
  }

  // This case should not be reachable
  assert(false);
}

// ============================================================================
// Condition flag helpers
// ============================================================================

// Each QPU contains an implicit condition vector which can answer
// various questions about each element of a vector:
//
//   * ZS - is zero?
//   * ZC - is non-zero?
//   * NS - is negative?
//   * NC - is non-negative?
//
// The condition vector is modified when the 'setFlags' field of
// an ALU instruction is 'true'.  The condition vector can be read
// from an assignment condition or in a branch condition.

// Function to negate a condition flag

Flag negFlag(Flag flag)
{
  switch(flag) {
    case ZS: return ZC;
    case ZC: return ZS;
    case NS: return NC;
    case NC: return NS;
  }

  // Not reachable
  assert(false);
}

// Function to negate an assignment condition.

AssignCond negAssignCond(AssignCond cond)
{
  switch (cond.tag) {
    case NEVER:  cond.tag = ALWAYS; return cond;
    case ALWAYS: cond.tag = NEVER;  return cond;
    case FLAG:   cond.flag = negFlag(cond.flag); return cond;
  }

  // Not reachable
  assert(false);
}

// Function to negate a branch condition.

BranchCond negBranchCond(BranchCond cond)
{
  switch (cond.tag) {
    case COND_NEVER:  cond.tag  = COND_ALWAYS; return cond;
    case COND_ALWAYS: cond.tag  = COND_NEVER;  return cond;
    case COND_ANY:    cond.tag  = COND_ALL;
                      cond.flag = negFlag(cond.flag);
                      return cond;
    case COND_ALL:    cond.tag  = COND_ANY;
                      cond.flag = negFlag(cond.flag);
                      return cond;
  }

  // Not reachable
  assert(false);
}



// Return a value that will cause the specified flag bit to be set in
// the condition vector.

int setFlag(Flag f)
{
  switch (f) {
    case ZS: return 0;
    case ZC: return 1;
    case NS: return -1;
    case NC: return 0;
  }

  // Not reachable
  assert(false);
}

// Set the condition vector using given variable.

Instr setCond(Var v)
{
  AssignCond always;
  always.tag = ALWAYS;
  Reg r;
  r.tag = NONE;
  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = true;
  instr.ALU.cond              = always;
  instr.ALU.dest              = r;
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcReg(v);
  instr.ALU.op                = A_BOR;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
  return instr;
}

// A shorthand 'move' instruction is handy later.

Instr move(Var dst, Var src, bool setFlags)
{
  AssignCond always;
  always.tag = ALWAYS;
  Instr instr;
  instr.tag                   = ALU;
  instr.ALU.setFlags          = setFlags;
  instr.ALU.cond              = always;
  instr.ALU.dest              = dstReg(dst);
  instr.ALU.srcA.tag          = REG;
  instr.ALU.srcA.reg          = srcReg(src);
  instr.ALU.op                = A_BOR;
  instr.ALU.srcB.tag          = REG;
  instr.ALU.srcB.reg          = instr.ALU.srcA.reg;
  return instr;
}

// ============================================================================
// Boolean expressions
// ============================================================================

// Evaluating a vector boolean expression results in a condition
// pair <condVar,condFlag> where
//
//  * condVar is a variable containing a vector of values
//
//  * condFlag is a condition flag in set {ZS,ZC,NS,NC} (see above)
//
// If 'condVar' is assigned to a register and the 'setFlags' field of
// the assignment is 'true', the implicit condition vector is updated.
// The implicit condition vector can then be queried using 'condFlag'
// to determine the truth of elements in the boolean vector.
//
// For example, assuming vectors of size 4 for simplicity, the result
// of evaluating
//
//   [1,2,3,4] <= [4,1,3,6]
//
// might be the condition pair <[-3,1,0,-2], NC>.
//
// Given two condition pairs <condVarA, condFlagA> and <condVarB,
// condFlagB> we would like to compute the logical disjunction.
//
// Pre-condition: we are in a state where the implicit condition
// vector has been set using the value of condVarB, hence we don't
// need the value of condVarB as an argument.
//
// The 'modify' flag defines whether or not to update the implicit
// condition vector with the final result.
//
// The value of condVarA will be overwritten with the 'condVar' of the
// disjunction, and the corresponding condFlag will be returned as a
// result.

AssignCond boolOr( Seq<Instr>* seq
                 , AssignCond condA
                 , Var condVarA
                 , AssignCond condB
                 , bool modify )
{
  if (condA.tag == ALWAYS) return condA;
  else if (condB.tag == ALWAYS) return condB;
  else if (condB.tag == NEVER) {
    if (modify) seq->append(setCond(condVarA));
    return condA;
  }
  else if (condA.tag == NEVER) {
    Instr instr;
    instr.tag           = LI;
    instr.LI.setFlags   = false;
    instr.LI.cond       = condB;
    instr.LI.dest       = dstReg(condVarA);
    instr.LI.imm.tag    = IMM_INT32;
    instr.LI.imm.intVal = setFlag(condB.flag);
    seq->append(instr);
    return condB;
  }
  else {
    Instr instr;
    instr.tag           = LI;
    instr.LI.setFlags   = false;
    instr.LI.cond       = condB;
    instr.LI.dest       = dstReg(condVarA);
    instr.LI.imm.tag    = IMM_INT32;
    instr.LI.imm.intVal = setFlag(condA.flag);
    seq->append(instr);

    if (modify) seq->append(setCond(condVarA));
    return condA;
  }
}

// Conjunction is now easy thanks to De Morgan's law:

AssignCond boolAnd( Seq<Instr>* seq
                  , AssignCond condA
                  , Var condVarA
                  , AssignCond condB
                  , bool modify )
{
  return negAssignCond(
           boolOr(seq, negAssignCond(condA), condVarA,
                       negAssignCond(condB), modify));
}

// Now the translation scheme for general boolean expressions.
// The interface is:
// 
//   * a boolean expression to evaluate;
//
//   * a condVar 'v' to which the evaluated expression will be written
//     to; the return value will contain the corresponding condFlag.
//
//   * if the modify-bit is true, then the implicit condition vector
//     will be set using with the result of the expression.  (This is a
//     one-way 'if': you cannot rely on the condition vector not
//     being mutated even if this bit is false.)
//
//   * instructions to evaluate the expression are appended to the
//     given instruction sequence.

AssignCond boolExp( Seq<Instr>* seq
                  , BExpr* bexpr
                  , Var v
                  , bool modify
                  )
{
  BExpr b = *bexpr;

  AssignCond always;
  always.tag = ALWAYS;

  // -------------------------------
  // Case: x > y, replace with y < x
  // -------------------------------
  if (b.tag == CMP && b.cmp.op.op == GT) {
    Expr* e     = b.cmp.lhs;
    b.cmp.lhs   = b.cmp.rhs;
    b.cmp.rhs   = e;
    b.cmp.op.op = LT;
  }

  // ---------------------------------
  // Case: x <= y, replace with y >= x
  // ---------------------------------
  if (b.tag == CMP && b.cmp.op.op == LE) {
    Expr* e     = b.cmp.lhs;
    b.cmp.lhs   = b.cmp.rhs;
    b.cmp.rhs   = e;
    b.cmp.op.op = GE;
  }

  // -----------------------------------
  // Case: x op y, where x is not simple
  // -----------------------------------
  if (b.tag == CMP && !isSimple(b.cmp.lhs)) {
    b.cmp.lhs = simplify(seq, b.cmp.lhs);
  }

  // -----------------------------------
  // Case: x op y, where y is not simple
  // -----------------------------------
  if (b.tag == CMP && !isSimple(b.cmp.rhs)) {
    b.cmp.rhs = simplify(seq, b.cmp.rhs);
  }

  // ---------------------------------------------
  // Case: x op y, where x and y are both literals
  // ---------------------------------------------
  if (b.tag == CMP && isLit(b.cmp.lhs) && isLit(b.cmp.rhs)) {
    Var tmpVar = freshVar();
    varAssign(seq, always, tmpVar, b.cmp.lhs);
    b.cmp.lhs = mkVar(tmpVar);
  }

  // --------------------------------------
  // Case: x op y, where x and y are simple
  // --------------------------------------
  if (b.tag == CMP) {
    // Compute condition flag
    AssignCond cond;
    cond.tag = FLAG;
    switch(b.cmp.op.op) {
      case EQ:  cond.flag = ZS; break;
      case NEQ: cond.flag = ZC; break;
      case LT:  cond.flag = NS; break;
      case GE:  cond.flag = NC; break;
      default:  assert(false);
    }

    // Implement comparison using subtraction instruction
    Op op;
    op.type = b.cmp.op.type;
    op.op   = SUB;
    Instr instr;
    instr.tag          = ALU;
    instr.ALU.setFlags = true;
    instr.ALU.cond     = always;
    instr.ALU.dest     = dstReg(v);
    instr.ALU.srcA     = operand(b.cmp.lhs);
    instr.ALU.op       = opcode(op);
    instr.ALU.srcB     = operand(b.cmp.rhs);
    seq->append(instr);

    return cond;
  }

  // -----------------------------------------
  // Case: !b, where b is a boolean expression
  // -----------------------------------------
  if (b.tag == NOT) {
    AssignCond cond = boolExp(seq, b.neg, v, modify);
    return negAssignCond(cond);
  }

  // ------------------------------------------------
  // Case: a || b, where a, b are boolean expressions
  // ------------------------------------------------
  if (b.tag == OR) {
    Var w = freshVar();
    AssignCond condA = boolExp(seq, b.disj.lhs, v, false);
    AssignCond condB = boolExp(seq, b.disj.rhs, w, true);
    return boolOr(seq, condA, v, condB, true);
  }

  // ------------------------------------------------
  // Case: a && b, where a, b are boolean expressions
  // ------------------------------------------------
  if (b.tag == AND) {
    // Use De Morgan's law
    BExpr* demorgan = mkNot(mkOr(mkNot(b.conj.lhs), mkNot(b.conj.rhs)));
    return boolExp(seq, demorgan, v, modify);
  }

  // Not reachable
  assert(false);
}


// ============================================================================
// Conditional expressions
// ============================================================================

BranchCond condExp(Seq<Instr>* seq, CExpr* c)
{
  Var v = freshVar();
  AssignCond cond = boolExp(seq, c->bexpr, v, true);

  BranchCond bcond;
  if (cond.tag == ALWAYS) { bcond.tag = COND_ALWAYS; return bcond; }
  if (cond.tag == NEVER) { bcond.tag = COND_NEVER; return bcond; }

  assert(cond.tag == FLAG);

  bcond.flag = cond.flag;
  if (c->tag == ANY) {
    bcond.tag = COND_ANY;
    return bcond;
  }
  else if (c->tag == ALL) {
    bcond.tag = COND_ALL;
    return bcond;
  }

  // Not reachable
  assert(false);
}

// ============================================================================
// Where statements
// ============================================================================

void whereStmt( Seq<Instr>* seq
              , Stmt* s
              , Var condVar
              , AssignCond cond
              , bool saveRestore )
{
  if (s == NULL) return;

  // ----------
  // Case: skip
  // ----------
  if (s->tag == SKIP) return;

  // ------------------------------------------------------
  // Case: v = e, where v is a variable and e an expression
  // ------------------------------------------------------
  if (s->tag == ASSIGN && s->assign.lhs->tag == VAR) {
    varAssign(seq, cond, s->assign.lhs->var, s->assign.rhs);
    return;
  }

  // ---------------------------------------------
  // Case: s0 ; s1, where s0 and s1 are statements
  // ---------------------------------------------
  if (s->tag == SEQ) {
    whereStmt(seq, s->seq.s0, condVar, cond, true);
    whereStmt(seq, s->seq.s1, condVar, cond, saveRestore);
    return;
  }

  // ----------------------------------------------------------
  // Case: where (b) s0 s1, where b is a boolean expression and
  //                        s0 and s1 are statements.
  // ----------------------------------------------------------
  if (s->tag == WHERE) {
    if (cond.tag == ALWAYS) {
      // This case has a cheaper implementation

      // Compile new boolean expression
      AssignCond newCond = boolExp(seq, s->where.cond, condVar, true);

      // Compile 'then' statement
      if (s->where.thenStmt != NULL)
        whereStmt(seq, s->where.thenStmt, condVar, newCond,
          s->where.elseStmt != NULL);

      // Compile 'else' statement
      if (s->where.elseStmt != NULL)
        whereStmt(seq, s->where.elseStmt, condVar,
                    negAssignCond(newCond), false);
    }
    else {
      // Save condVar
      Var savedCondVar = freshVar();
      if (saveRestore || s->where.elseStmt != NULL)
        seq->append(move(savedCondVar, condVar, false));

      // Compile new boolean expression
      Var newCondVar = freshVar();
      AssignCond newCond = boolExp(seq, s->where.cond, newCondVar, true);

      if (s->where.thenStmt != NULL) {
        // AND new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, cond, condVar, newCond, true);

        // Compile 'then' statement
        whereStmt(seq, s->where.thenStmt, condVar, andCond, false);
      }

      if (saveRestore || s->where.elseStmt != NULL)
        seq->append(move(condVar, savedCondVar, true));

      if (s->where.elseStmt != NULL) {
        // AND negation of new boolean expression with original condition
        AssignCond andCond = boolAnd(seq, negAssignCond(newCond), newCondVar,
                               cond, true);
  
        // Compile 'else' statement
        whereStmt(seq, s->where.elseStmt, newCondVar, andCond, false);
  
        // Restore condVar and implicit condition vector
        if (saveRestore)
          seq->append(move(condVar, savedCondVar, true));
      }
    }

    return;
  }

  printf("QPULib: only assignments and nested 'where' \
          statements can occur in a 'where' statement\n");
  assert(false);
}

// ============================================================================
// Print statements
// ============================================================================

void printStmt(Seq<Instr>* seq, PrintStmt s)
{
  Instr instr;
  switch (s.tag) {
    case PRINT_INT:
    case PRINT_FLOAT: {
      AssignCond always;
      always.tag = ALWAYS;
      Var tmpVar = freshVar();
      varAssign(seq, always, tmpVar, s.expr);
      if (s.tag == PRINT_INT) {
        instr.tag = PRI;
        instr.PRI = srcReg(tmpVar);
      }
      else {
        instr.tag = PRF;
        instr.PRF = srcReg(tmpVar);
      }
      seq->append(instr);
      return;
    }
    case PRINT_STR:
      instr.tag = PRS;
      instr.PRS = s.str;
      seq->append(instr);
      return;
  }

  assert(false);
}

// ============================================================================
// Set-stride statements
// ============================================================================

void setStrideStmt(Seq<Instr>* seq, StmtTag tag, Expr* e)
{
  if (e->tag == INT_LIT) {
    if (tag == SET_READ_STRIDE)
      genSetReadStride(seq, e->intLit);
    else
      genSetWriteStride(seq, e->intLit);
  }
  else if (e->tag == VAR) {
    if (tag == SET_READ_STRIDE)
      genSetReadStride(seq, srcReg(e->var));
    else
      genSetWriteStride(seq, srcReg(e->var));
  }
  else {
    AssignCond always;
    always.tag = ALWAYS;
    Var v = freshVar();
    varAssign(seq, always, v, e);
    if (tag == SET_READ_STRIDE)
      genSetReadStride(seq, srcReg(v));
    else
      genSetWriteStride(seq, srcReg(v));
  }
}

// ============================================================================
// Load receive statements
// ============================================================================

void loadReceive(Seq<Instr>* seq, Expr* dest)
{
  assert(dest->tag == VAR);
  Instr instr;
  instr.tag = RECV;
  instr.RECV.dest = dstReg(dest->var);
  seq->append(instr);
}

// ============================================================================
// Store request
// ============================================================================

// A 'store' operation of data to addr is almost the same as
// *addr = data.  The difference is that a 'store' waits until
// outstanding DMAs have completed before performing a write rather
// than after a write.  This enables other operations to happen in
// parallel with the write.

void storeRequest(Seq<Instr>* seq, Expr* data, Expr* addr)
{
  if (data->tag != VAR || addr->tag != VAR) {
    data = putInVar(seq, data);
    addr = putInVar(seq, addr);
  }

  Instr instr;
  instr.tag        = ST3;
  seq->append(instr);
  instr.tag        = ST1;
  instr.ST1.data   = srcReg(data->var);
  instr.ST1.buffer = A;
  seq->append(instr);
  instr.tag        = ST2;
  instr.ST2.addr   = srcReg(addr->var);
  instr.ST2.buffer = A;
  seq->append(instr);
}

// ============================================================================
// Semaphores
// ============================================================================

void semaphore(Seq<Instr>* seq, StmtTag tag, int semaId)
{
  Instr instr;
  instr.tag = tag == SEMA_INC ? SINC : SDEC;
  instr.semaId = semaId;
  seq->append(instr);
}

// ============================================================================
// Host IRQ
// ============================================================================

void sendIRQToHost(Seq<Instr>* seq)
{
  Instr instr;
  instr.tag = IRQ;
  seq->append(instr);
}

// ============================================================================
// Statements
// ============================================================================

void stmt(Seq<Instr>* seq, Stmt* s)
{
  if (s == NULL) return;

  // ----------
  // Case: skip
  // ----------
  if (s->tag == SKIP) return;

  // --------------------------------------------------
  // Case: lhs = rhs, where lhs and rhs are expressions
  // --------------------------------------------------
  if (s->tag == ASSIGN) {
    assign(seq, s->assign.lhs, s->assign.rhs);
    return;
  }

  // ---------------------------------------------
  // Case: s0 ; s1, where s1 and s2 are statements
  // ---------------------------------------------

  if (s->tag == SEQ) {
    stmt(seq, s->seq.s0);
    stmt(seq, s->seq.s1);
    return;
  }

  // -------------------------------------------------------------------
  // Case: if (c) s0 s1, where c is a condition, and lhs,rhs expressions
  // -------------------------------------------------------------------
  if (s->tag == IF) {
    Instr instr;
    Label elseLabel  = freshLabel();
    Label endifLabel = freshLabel();
    // Compile condition
    BranchCond cond  = condExp(seq, s->ifElse.cond);
    
    // Branch to 'else' statement
    Instr branchElse;
    instr.tag       = BRL;
    instr.BRL.cond  = negBranchCond(cond);
    instr.BRL.label = elseLabel;
    seq->append(instr);

    // Compile 'then' statement
    stmt(seq, s->ifElse.thenStmt);

    // Branch to endif
    instr.tag          = BRL;
    instr.BRL.cond.tag = COND_ALWAYS;
    instr.BRL.label    = endifLabel;
    if (s->ifElse.elseStmt != NULL) seq->append(instr);

    // Label for 'else' statement
    instr.tag   = LAB;
    instr.label = elseLabel;
    seq->append(instr);

    // Compile 'else' statement
    stmt(seq, s->ifElse.elseStmt);

    // Label for endif
    instr.tag   = LAB;
    instr.label = endifLabel;
    seq->append(instr);

    return;
  }

  // -----------------------------------------------------------
  // Case: while (c) s where c is a condition, and s a statement
  // -----------------------------------------------------------
  if (s->tag == WHILE) {
    Instr instr;
    Label startLabel = freshLabel();
    Label endLabel   = freshLabel();
    // Compile condition
    BranchCond cond  = condExp(seq, s->loop.cond);
 
    // Branch over loop body
    Instr branchEnd;
    instr.tag       = BRL;
    instr.BRL.cond  = negBranchCond(cond);
    instr.BRL.label = endLabel;
    seq->append(instr);

    // Start label
    instr.tag   = LAB;
    instr.label = startLabel;
    seq->append(instr);

    // Compile body
    if (s->loop.body != NULL) stmt(seq, s->loop.body);

    // Compute condition again
    condExp(seq, s->loop.cond);

    // Branch to start
    instr.tag       = BRL;
    instr.BRL.cond  = cond;
    instr.BRL.label = startLabel;
    seq->append(instr);

    // End label
    instr.tag   = LAB;
    instr.label = endLabel;
    seq->append(instr);

    return;
  }

  // ----------------------------------------------------------------------
  // Case: where (b) s0 s1 where c is a boolean expr, and s0, s1 statements
  // ----------------------------------------------------------------------
  if (s->tag == WHERE) {
    Var condVar = freshVar();
    AssignCond always;
    always.tag = ALWAYS;
    whereStmt(seq, s, condVar, always, false);
    return;
  }

  // ---------------------------------------------
  // Case: print(e) where e is an expr or a string
  // ---------------------------------------------
  if (s->tag == PRINT) {
    printStmt(seq, s->print);
    return;
  }

  // --------------------------------------------------------------
  // Case: setReadStride(e) or setWriteStride(e) where e is an expr
  // --------------------------------------------------------------
  if (s->tag == SET_READ_STRIDE || s->tag == SET_WRITE_STRIDE) {
    setStrideStmt(seq, s->tag, s->stride);
    return;
  }

  // -----------------------------------
  // Case: receive(e) where e is an expr
  // -----------------------------------
  if (s->tag == LOAD_RECEIVE) {
    loadReceive(seq, s->loadDest);
    return;
  }

  // ---------------------------------------------
  // Case: store(e0, e1) where e1 and e2 are exprs
  // ---------------------------------------------
  if (s->tag == STORE_REQUEST) {
    storeRequest(seq, s->storeReq.data, s->storeReq.addr);
    return;
  }

  // -------------
  // Case: flush()
  // -------------
  if (s->tag == FLUSH) {
    // Flush outstanding stores
    Instr instr; instr.tag = ST3;
    seq->append(instr);
    return;
  }

  // ---------------------------------------------------------------
  // Case: semaInc(n) or semaDec(n) where n is an int (semaphore id)
  // ---------------------------------------------------------------
  if (s->tag == SEMA_INC || s->tag == SEMA_DEC) {
    semaphore(seq, s->tag, s->semaId);
    return;
  }

  // ---------------
  // Case: hostIRQ()
  // ---------------
  if (s->tag == SEND_IRQ_TO_HOST) {
    sendIRQToHost(seq);
    return;
  }

  // Not reachable
  assert(false);
}

// ============================================================================
// End code
// ============================================================================

void insertEndCode(Seq<Instr>* seq)
{
  Instr instr;

  // Insert 'end' instruction
  instr.tag = END;
  seq->append(instr);
}

// ============================================================================
// Interface
// ============================================================================

// Top-level translation function for statements.

void translateStmt(Seq<Instr>* seq, Stmt* s)
{
  stmt(seq, s);
  insertEndCode(seq);
}

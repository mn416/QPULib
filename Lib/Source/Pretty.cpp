#include "Source/Pretty.h"
#include <cassert>

// ============================================================================
// Operators
// ============================================================================

const char* opToString(Op op)
{
  switch (op.op) {
    case ADD:    return "+";
    case SUB:    return "-";
    case MUL:    return "*";
    case MIN:    return " min ";
    case MAX:    return " max ";
    case ROTATE: return " rotate ";
    case SHL:    return " << ";
    case SHR:    return " >> ";
    case USHR:   return " _>> ";
    case ROR:    return " ror ";
    case BOR:    return " | ";
    case BAND:   return " & ";
    case BXOR:   return " ^ ";
    case BNOT:   return "~";
    case ItoF:   return "(Float) ";
    case FtoI:   return "(Int) ";
  }

  // Not reachable
  assert(false);
}

const char* cmpOpToString(CmpOp op)
{
  switch (op.op) {
    case EQ : return "==";
    case NEQ: return "!=";
    case LT : return "<";
    case LE : return "<=";
    case GT : return ">";
    case GE : return ">=";
  }

  // Not reachable
  assert(false);
}

// ============================================================================
// Expressions
// ============================================================================

void pretty(FILE *f, Expr* e)
{
  assert(f != nullptr);
  if (e == NULL) return;

  switch (e->tag) {
    // Integer literals
    case INT_LIT:
      printf("%i", e->intLit);
      break;

    // Float literals
    case FLOAT_LIT:
      printf("%f", e->floatLit);
      break;

    // Variables
    case VAR:
      if (e->var.tag == STANDARD)
        printf("v%i", e->var.id);
      else if (e->var.tag == UNIFORM)
        printf("UNIFORM");
      else if (e->var.tag == QPU_NUM)
        printf("QPU_NUM");
      else if (e->var.tag == ELEM_NUM)
        printf("ELEM_NUM");
      else if (e->var.tag == TMU0_ADDR)
        printf("TMU0_ADDR");
      break;

    // Applications
    case APPLY:
      if (isUnary(e->apply.op)) {
        printf("(");
        printf("%s", opToString(e->apply.op));
        pretty(f, e->apply.lhs);
        printf(")");
      }
      else {
        printf("(");
        pretty(f, e->apply.lhs);
        printf("%s", opToString(e->apply.op));
        pretty(f, e->apply.rhs);
        printf(")");
      }
      break;

    // Dereference
    case DEREF:
      printf("*");
      pretty(f, e->deref.ptr);
      break;

  }
}

// ============================================================================
// Boolean expressions
// ============================================================================

void pretty(FILE *f, BExpr* b)
{
  assert(f != nullptr);
  if (b == NULL) return;

  switch (b->tag) {
    // Negation
    case NOT:
      printf("!");
      pretty(f, b->neg);
      break;

    // Conjunction
    case AND:
      printf("(");
      pretty(f, b->conj.lhs);
      printf(" && ");
      pretty(f, b->conj.rhs);
      printf(")");
      break;

    // Disjunction
    case OR:
      printf("(");
      pretty(f, b->disj.lhs);
      printf(" || ");
      pretty(f, b->disj.rhs);
      printf(")");
      break;

    // Comparison
    case CMP:
      pretty(f, b->cmp.lhs);
      printf("%s", cmpOpToString(b->cmp.op));
      pretty(f, b->cmp.rhs);
      break;
  }
}

// ============================================================================
// Conditional expressions
// ============================================================================

void pretty(FILE *f, CExpr* c)
{
  assert(f != nullptr);
  if (c == NULL) return;

  switch (c->tag) {
    // Reduce using 'any'
    case ANY: printf("any("); break;

    // Reduce using 'all'
    case ALL: printf("all("); break;
  }

  pretty(f, c->bexpr);
  printf(")");
}

// ============================================================================
// Statements
// ============================================================================

void indentBy(int indent) {
  for (int i = 0; i < indent; i++) printf(" ");
}

void pretty(FILE *f, int indent, Stmt* s)
{
  assert(f != nullptr);
  if (s == NULL) return;

  switch (s->tag) {
    // Skip
    case SKIP: break;

    // Assignment
    case ASSIGN:
      indentBy(indent);
      pretty(f, s->assign.lhs);
      printf(" = ");
      pretty(f, s->assign.rhs);
      fprintf(f, ";\n");
      break;

    // Sequential composition
    case SEQ:
      pretty(f, indent, s->seq.s0);
      pretty(f, indent, s->seq.s1);
      break;

    // Where statement
    case WHERE:
      indentBy(indent);
      printf("Where (");
      pretty(f, s->where.cond);
      printf(")\n");
      pretty(f, indent+2, s->where.thenStmt);
      if (s->where.elseStmt != NULL) {
        indentBy(indent);
        fprintf(f, "Else\n");
        pretty(f, indent+2, s->where.elseStmt);
      }
      indentBy(indent);
      printf("End\n");
      break;

    // If statement
    case IF:
      indentBy(indent);
      printf("If (");
      pretty(f, s->ifElse.cond);
      printf(")\n");
      pretty(f, indent+2, s->ifElse.thenStmt);
      if (s->where.elseStmt != NULL) {
        indentBy(indent);
        printf("Else\n");
        pretty(f, indent+2, s->ifElse.elseStmt);
      }
      indentBy(indent);
      printf("End\n");
      break;

    // While statement
    case WHILE:
      indentBy(indent);
      printf("While (");
      pretty(f, s->loop.cond);
      printf(")\n");
      pretty(f, indent+2, s->loop.body);
      indentBy(indent);
      printf("End\n");
      break;

    // Print statement
    case PRINT:
      indentBy(indent);
      printf("Print (");
      if (s->print.tag == PRINT_STR) {
        // Ideally would print escaped string here
        printf("\"%s\"", s->print.str);
      }
      else
        pretty(f, s->print.expr);
      printf(")\n");
      break;

    // Set read stride
    case SET_READ_STRIDE:
      indentBy(indent);
      printf("setReadStride(");
      pretty(f, s->stride);
      printf(")\n");
      break;

    // Set write stride
    case SET_WRITE_STRIDE:
      indentBy(indent);
      printf("setWriteStride(");
      pretty(f, s->stride);
      printf(")\n");
      break;

    // Load receive
    case LOAD_RECEIVE:
      indentBy(indent);
      printf("receive(");
      pretty(f, s->loadDest);
      printf(")\n");
      break;

    // Store request
    case STORE_REQUEST:
      indentBy(indent);
      printf("store(");
      pretty(f, s->storeReq.data);
      fprintf(f, ", ");
      pretty(f, s->storeReq.addr);
      printf(")\n");
      break;

    // Flush outstanding stores
    case FLUSH:
      indentBy(indent);
      printf("flush()\n");
      break;

    // Increment semaphore
    case SEMA_INC:
      indentBy(indent);
      printf("semaInc(%i)\n", s->semaId);
      break;

    // Decrement semaphore
    case SEMA_DEC:
      indentBy(indent);
      printf("semaDec(%i)\n", s->semaId);
      break;

    // Host IRQ
    case SEND_IRQ_TO_HOST:
      indentBy(indent);
      printf("hostIRQ()\n");
      break;

    // Not reachable
    default:
      assert(false);
  }
}

void pretty(FILE *f, Stmt* s)
{
  assert(f != nullptr);
  pretty(f, 0, s);
}

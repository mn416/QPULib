#include "Source/Interpreter.h"
#include "Target/Emulator.h"

// ============================================================================
// Evaluate a variable
// ============================================================================

Vec evalVar(CoreState* s, Var v)
{
  switch (v.tag) {
    // Normal variable
    case STANDARD:
      assert(v.id < s->sizeEnv);
      return s->env[v.id];

    // Return next uniform
    case UNIFORM: {
      assert(s->nextUniform < s->uniforms->numElems);
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        if (s->nextUniform == -2)
          x.elems[i].intVal = s->id;
        else if (s->nextUniform == -1)
          x.elems[i].intVal = s->numCores;
        else
          x.elems[i].intVal = s->uniforms->elems[s->nextUniform];
      s->nextUniform++;
      return x;
    }

    // Return core id
    case QPU_NUM: {
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        x.elems[i].intVal = s->id;
      return x;
    }

    // Return vector of integers 0..15 inclusive
    case ELEM_NUM: {
      Vec x;
      for (int i = 0; i < NUM_LANES; i++)
        x.elems[i].intVal = i;
      return x;
    }

    default:
      printf("QPULib: reading from write-only variable\n");
  }

  assert(false);
}

// ============================================================================
// Evaluate an arithmetic expression
// ============================================================================

Vec eval(CoreState* s, Expr* e)
{
  Vec v;
  switch (e->tag) {
    // Integer literal
    case INT_LIT:
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = e->intLit;
      return v;

    // Float literal
    case FLOAT_LIT:
       for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].floatVal = e->floatLit;
      return v;
   
    // Variable
    case VAR:
      return evalVar(s, e->var);

    // Operator application
    case APPLY: {
      Vec a = eval(s, e->apply.lhs);
      Vec b = eval(s, e->apply.rhs);
      if (e->apply.op.op == ROTATE) {
        // Vector rotation
        v = rotate(a, b.elems[0].intVal);
      }
      else if (e->apply.op.type == FLOAT) {
        // Floating-point operation
        for (int i = 0; i < NUM_LANES; i++) {
          float x = a.elems[i].floatVal;
          float y = b.elems[i].floatVal;
          switch (e->apply.op.op) {
            case ADD:  v.elems[i].floatVal = x+y; break;
            case SUB:  v.elems[i].floatVal = x-y; break;
            case MUL:  v.elems[i].floatVal = x*y; break;
            case ItoF: v.elems[i].floatVal = (float) a.elems[i].intVal; break;
            case FtoI: v.elems[i].intVal   = (int) a.elems[i].floatVal; break;
            case MIN:  v.elems[i].floatVal = x<y?x:y; break;
            case MAX:  v.elems[i].floatVal = x>y?x:y; break;
            default: assert(false);
          }
        }
      }
      else {
        // Integer operation
        for (int i = 0; i < NUM_LANES; i++) {
          int32_t x = a.elems[i].intVal;
          int32_t y = b.elems[i].intVal;
          switch (e->apply.op.op) {
            case ADD:  v.elems[i].intVal = x+y; break;
            case SUB:  v.elems[i].intVal = x-y; break;
            case MUL:  v.elems[i].intVal = (x&0xffffff)*(y&0xffffff); break;
            case SHL:  v.elems[i].intVal = x<<y; break;
            case SHR:  v.elems[i].intVal = x>>y; break;
            case ItoF: v.elems[i].floatVal = (float) a.elems[i].intVal; break;
            case FtoI: v.elems[i].intVal   = (int) a.elems[i].floatVal; break;
            case MIN:  v.elems[i].intVal = x<y?x:y; break;
            case MAX:  v.elems[i].intVal = x>y?x:y; break;
            case BOR:  v.elems[i].intVal = x|y; break;
            case BAND: v.elems[i].intVal = x&y; break;
            case BXOR: v.elems[i].intVal = x^y; break;
            case BNOT: v.elems[i].intVal = ~x; break;
            default: assert(false);
          }
        }
      }
      return v;
    }

    // Dereference pointer
    case DEREF:
      Vec a = eval(s, e->deref.ptr);
      int hp = a.elems[0].intVal;
      Vec v;
      for (int i = 0; i < NUM_LANES; i++) {
        v.elems[i].intVal = emuHeap[hp>>2];
        hp += 4*(s->readStride+1);
      }
      return v;
  }
  assert(false);
}

// ============================================================================
// Evaluate boolean expression
// ============================================================================

Vec evalBool(CoreState* s, BExpr* e)
{
  Vec v;
  switch (e->tag) {
    // Negation
    case NOT:
      v = evalBool(s, e->neg);
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = !v.elems[i].intVal;
      return v;

    // Conjunction
    case AND: {
      Vec a = evalBool(s, e->conj.lhs);
      Vec b = evalBool(s, e->conj.rhs);
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = a.elems[i].intVal && b.elems[i].intVal;
      return v;
    }

    // Disjunction
    case OR: {
      Vec a = evalBool(s, e->disj.lhs);
      Vec b = evalBool(s, e->disj.rhs);
      for (int i = 0; i < NUM_LANES; i++)
        v.elems[i].intVal = a.elems[i].intVal || b.elems[i].intVal;
      return v;
    }

    // Comparison
    case CMP: {
      Vec a = eval(s, e->cmp.lhs);
      Vec b = eval(s, e->cmp.rhs);
      if (e->cmp.op.type == FLOAT) {
        // Floating-point comparison
        for (int i = 0; i < NUM_LANES; i++) {
          float x = a.elems[i].floatVal;
          float y = b.elems[i].floatVal;
          switch (e->cmp.op.op) {
            case EQ:  v.elems[i].intVal = x == y; break;
            case NEQ: v.elems[i].intVal = x != y; break;
            case LT:  v.elems[i].intVal = x <  y; break;
            case GT:  v.elems[i].intVal = x >  y; break;
            case LE:  v.elems[i].intVal = x <= y; break;
            case GE:  v.elems[i].intVal = x >= y; break;
            default:  assert(false);
          }
        }
        return v;
      }
      else {
        // Integer comparison
        for (int i = 0; i < NUM_LANES; i++) {
          int32_t x = a.elems[i].intVal;
          int32_t y = b.elems[i].intVal;
          switch (e->cmp.op.op) {
            case EQ:  v.elems[i].intVal = x == y; break;
            case NEQ: v.elems[i].intVal = x != y; break;
            // Ideally compiler would implement:
            // case LT:  v.elems[i].intVal = x <  y; break;
            // case GT:  v.elems[i].intVal = x >  y; break;
            // case LE:  v.elems[i].intVal = x <= y; break;
            // case GE:  v.elems[i].intVal = x >= y; break;
            // But currently it implements:
            case LT: v.elems[i].intVal = ((x-y) & 0x80000000) != 0; break;
            case GE: v.elems[i].intVal = ((x-y) & 0x80000000) == 0; break;
            case LE: v.elems[i].intVal = ((y-x) & 0x80000000) == 0; break;
            case GT: v.elems[i].intVal = ((y-x) & 0x80000000) != 0; break;
            default:  assert(false);
          }
        }
        return v;
      }
    }
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Evaulate condition
// ============================================================================

bool evalCond(CoreState* s, CExpr* e)
{
  Vec v = evalBool(s, e->bexpr);

  switch (e->tag) {
    case ALL: {
      bool b = true;
      for (int i = 0; i < NUM_LANES; i++)
        b = b && v.elems[i].intVal;
      return b;
    }
      
    case ANY: {
      bool b = false;
      for (int i = 0; i < NUM_LANES; i++)
        b = b || v.elems[i].intVal;
      return b;
    }
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Assign to a variable
// ============================================================================

void assignToVar(CoreState* s, Vec cond, Var v, Vec x)
{
  switch (v.tag) {
    // Normal variable
    case STANDARD:
      for (int i = 0; i < NUM_LANES; i++)
        if (cond.elems[i].intVal) {
          s->env[v.id].elems[i] = x.elems[i];
        }
      return;

    // Load via TMU
    case TMU0_ADDR: {
      assert(s->loadBuffer->numElems < 8);
      Vec w;
      for (int i = 0; i < NUM_LANES; i++) {
        uint32_t addr = (uint32_t) x.elems[i].intVal;
        w.elems[i].intVal = emuHeap[addr>>2];
      }
      s->loadBuffer->append(w);
      return;
    }

    // Others are read-only
    case UNIFORM:
    case QPU_NUM:
    case ELEM_NUM:
      printf("QPULib: writing to read-only variable\n");
  }

  assert(false);
}

// ============================================================================
// Execute assignment
// ============================================================================

void execAssign(CoreState* s, Vec cond, Expr* lhs, Expr* rhs)
{
  // Evaluate RHS
  Vec val = eval(s, rhs);

  switch (lhs->tag) {
    // Variable
    case VAR:
      assignToVar(s, cond, lhs->var, val);
      return;

    // Dereferenced pointer
    case DEREF: {
      Vec index = eval(s, lhs->deref.ptr);
      int hp = index.elems[0].intVal;
      for (int i = 0; i < NUM_LANES; i++) {
        emuHeap[hp>>2] = val.elems[i].intVal;
        hp += 4*(s->writeStride+1);
      }
      return;
    }
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Condition vector auxiliaries
// ============================================================================

// Condition vector containing all trues
Vec vecAlways()
{
  Vec always;
  for (int i = 0; i < NUM_LANES; i++)
    always.elems[i].intVal = 1;
  return always;
}

// Negate a condition vector
Vec vecNeg(Vec cond)
{
  Vec v;
  for (int i = 0; i < NUM_LANES; i++)
    v.elems[i].intVal = !cond.elems[i].intVal;
  return v;
}

// And two condition vectors
Vec vecAnd(Vec x, Vec y)
{
  Vec v;
  for (int i = 0; i < NUM_LANES; i++)
    v.elems[i].intVal = x.elems[i].intVal && y.elems[i].intVal;
  return v;
}

// ============================================================================
// Execute where statement
// ============================================================================

void execWhere(CoreState* s, Vec cond, Stmt* stmt)
{
  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case SKIP:
      return;

    // Sequential composition
    case SEQ:
      execWhere(s, cond, stmt->seq.s0);
      execWhere(s, cond, stmt->seq.s1);
      return;

    // Assignment
    case ASSIGN:
      if (stmt->assign.lhs->tag != VAR) {
        printf("QPULib: only var assignments permitted in 'where'\n");
        assert(false);
      }
      execAssign(s, cond, stmt->assign.lhs, stmt->assign.rhs);
      return;

    // Nested where
    case WHERE: {
      Vec b = evalBool(s, stmt->where.cond);
      execWhere(s, vecAnd(b, cond), stmt->where.thenStmt);
      execWhere(s, vecAnd(vecNeg(b), cond), stmt->where.elseStmt);
      return;
    }
  }

  printf("QPULib: only assignments and nested 'where' \
          statements can occur in a 'where' statement\n");
  assert(false);
}

// ============================================================================
// Execute print statement
// ============================================================================

void execPrint(CoreState* s, PrintStmt p)
{
  switch (p.tag) {
    // Integer
    case PRINT_INT: {
      Vec x = eval(s, p.expr);
      printIntVec(s->output, x);
      return;
    }

    // Float
    case PRINT_FLOAT: {
      Vec x = eval(s, p.expr);
      printFloatVec(s->output, x);
      return;
    }

    // String
    case PRINT_STR:
      emitStr(s->output, p.str);
      return;
  }
}

// ============================================================================
// Execute set-stride statements
// ============================================================================

void execSetStride(CoreState* s, StmtTag tag, Expr* e)
{
  Vec v = eval(s, e);
  if (tag == SET_READ_STRIDE)
    s->readStride = v.elems[0].intVal;
  else
    s->writeStride = v.elems[0].intVal;
}

// ============================================================================
// Execute load receive & store request statements
// ============================================================================

void execLoadReceive(CoreState* s, Expr* e)
{
  assert(s->loadBuffer->numElems > 0);
  assert(e->tag == VAR);
  Vec val = s->loadBuffer->remove(0);
  assignToVar(s, vecAlways(), e->var, val);
}

void execStoreRequest(CoreState* s, Expr* data, Expr* addr) {
  Vec val = eval(s, data);
  Vec index = eval(s, addr);
  int hp = index.elems[0].intVal;
  for (int i = 0; i < NUM_LANES; i++) {
    emuHeap[hp>>2] = val.elems[i].intVal;
    hp += 4*(s->writeStride+1);
  }
}

// ============================================================================
// Execute code
// ============================================================================

void exec(InterpreterState* state, CoreState* s)
{
  // Control stack must be non-empty
  assert(s->stack->numElems > 0);

  // Pop the statement at the top of the stack
  Stmt* stmt = s->stack->pop();

  if (stmt == NULL) return;

  switch (stmt->tag) {
    // No-op
    case SKIP:
      return;

    // Assignment
    case ASSIGN:
      execAssign(s, vecAlways(), stmt->assign.lhs, stmt->assign.rhs);
      return;

    // Sequential composition
    case SEQ:
      s->stack->push(stmt->seq.s1);
      s->stack->push(stmt->seq.s0);
      return;

    // Conditional assignment
    case WHERE: {
      Vec b = evalBool(s, stmt->where.cond);
      execWhere(s, b, stmt->where.thenStmt);
      execWhere(s, vecNeg(b), stmt->where.elseStmt);
      return;
    }

    // If statement
    case IF:
      if (evalCond(s, stmt->ifElse.cond))
        s->stack->push(stmt->ifElse.thenStmt);
      else
        s->stack->push(stmt->ifElse.elseStmt);
      return;

    // While statement
    case WHILE:
      if (evalCond(s, stmt->loop.cond)) {
        s->stack->push(stmt);
        s->stack->push(stmt->loop.body);
      }
      return;

    // Print statement
    case PRINT:
      execPrint(s, stmt->print);
      return;

    // Set read stride
    case SET_READ_STRIDE:
      execSetStride(s, SET_READ_STRIDE, stmt->stride);
      return;

    // Set write stride
    case SET_WRITE_STRIDE:
      execSetStride(s, SET_WRITE_STRIDE, stmt->stride);
      return;

    // Load receive
    case LOAD_RECEIVE:
      execLoadReceive(s, stmt->loadDest);
      return;

    // Store request
    case STORE_REQUEST:
      execStoreRequest(s, stmt->storeReq.data, stmt->storeReq.addr);
      return;

    // Host IRQ
    case SEND_IRQ_TO_HOST:
      return;

    // Increment semaphore
    case SEMA_INC:
      assert(stmt->semaId >= 0 && stmt->semaId < 16);
      if (state->sema[stmt->semaId] == 15) s->stack->push(stmt);
      else state->sema[stmt->semaId]++;
      return;
 
    // Decrement semaphore
    case SEMA_DEC:
      assert(stmt->semaId >= 0 && stmt->semaId < 16);
      if (state->sema[stmt->semaId] == 0) s->stack->push(stmt);
      else state->sema[stmt->semaId]--;
      return;

    // Flush outstanding stores
    case FLUSH: return;
  }

  // Unreachable
  assert(false);
}

// ============================================================================
// Interpreter
// ============================================================================

void interpreter
  ( int numCores           // Number of cores active
  , Stmt* stmt             // Source code
  , int maxVar             // Max var id used in source
  , Seq<int32_t>* uniforms // Kernel parameters
  , Seq<char>* output      // Output from print statements
                           // (if NULL, stdout is used)
  )
{
  InterpreterState state;

  // Initialise state
  for (int i = 0; i < numCores; i++) {
    CoreState s;
    s.id          = i;
    s.numCores    = numCores;
    s.nextUniform = -2;
    s.uniforms    = uniforms;
    s.readStride  = 0;
    s.writeStride = 0;
    s.env         = new Vec [maxVar+1];
    s.sizeEnv     = maxVar+1;
    s.stack       = new Seq<Stmt*>;
    s.output      = output;
    s.loadBuffer  = new SmallSeq<Vec>;
    state.core[i] = s;
  }

  // Initialise semaphores
  for (int i = 0; i < 16; i++) state.sema[i] = 0;

  // Put statement on each core's control stack
  for (int i = 0; i < numCores; i++)
   state.core[i].stack->push(stmt);

  // Run code
  bool running = true;
  while (running) {
    running = false;
    for (int i = 0; i < numCores; i++) {
      if (state.core[i].stack->numElems > 0) {
        running = true;
        exec(&state, &state.core[i]);
      }
    }
  }

  // Deallocate state
  for (int i = 0; i < numCores; i++) {
    delete state.core[i].stack;
    delete [] state.core[i].env;
  }
}

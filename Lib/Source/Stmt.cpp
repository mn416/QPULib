#include <stdio.h>
#include "Common/Stack.h"
#include "Source/Stmt.h"
#include "Source/Int.h"


namespace QPULib {

// Interface to the embedded language.

//=============================================================================
// Assignment token
//=============================================================================

void assign(Expr* lhs, Expr* rhs) {
  Stmt* s = mkAssign(lhs, rhs);
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

//=============================================================================
// 'If' token
//=============================================================================

void If_(Cond c)
{
  Stmt* s = mkIf(c.cexpr, NULL, NULL);
  controlStack.push(s);
  stmtStack.push(mkSkip());
}

void If_(BoolExpr b)
{
  If_(any(b));
}

//=============================================================================
// 'Else' token
//=============================================================================

void Else_()
{
  int ok = 0;
  if (controlStack.size > 0) {
    Stmt* s = controlStack.top();
    if (s->tag == IF && s->ifElse.thenStmt == NULL) {
      s->ifElse.thenStmt = stmtStack.top();
      stmtStack.replace(mkSkip());
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == NULL) {
      s->where.thenStmt = stmtStack.top();
      stmtStack.replace(mkSkip());
      ok = 1;
    }
  }

  if (!ok) {
    printf("Syntax error: 'Else' without preceeding 'If' or 'Where'\n");
    exit(-1);
  }
}

//=============================================================================
// 'End' token
//=============================================================================

void End_()
{
  int ok = 0;
  if (controlStack.size > 0) {
    Stmt* s = controlStack.top();
    if (s->tag == IF && s->ifElse.thenStmt == NULL) {
      s->ifElse.thenStmt = stmtStack.top();
      ok = 1;
    }
    else if (s->tag == IF && s->ifElse.elseStmt == NULL) {
      s->ifElse.elseStmt = stmtStack.top();
      ok = 1;
    }
    if (s->tag == WHERE && s->where.thenStmt == NULL) {
      s->where.thenStmt = stmtStack.top();
      ok = 1;
    }
    else if (s->tag == WHERE && s->where.elseStmt == NULL) {
      s->where.elseStmt = stmtStack.top();
      ok = 1;
    }
    if (s->tag == WHILE && s->loop.body == NULL) {
      s->loop.body = stmtStack.top();
      ok = 1;
    }
    if (s->tag == FOR && s->forLoop.body == NULL) {
      // Convert 'for' loop to 'while' loop
      CExpr* whileCond = s->forLoop.cond;
      Stmt* whileBody = mkSeq(stmtStack.top(), s->forLoop.inc);
      s->tag = WHILE;
      s->loop.body = whileBody;
      s->loop.cond = whileCond;
      ok = 1;
    }

    if (ok) {
      stmtStack.pop();
      stmtStack.replace(mkSeq(stmtStack.top(), s));
      controlStack.pop();
    }
  }

  if (!ok) {
    printf("Syntax error: unexpected 'End'\n");
    exit(-1);
  }
}

//=============================================================================
// 'While' token
//=============================================================================

void While_(Cond c)
{
  Stmt* s = mkWhile(c.cexpr, NULL);
  controlStack.push(s);
  stmtStack.push(mkSkip());
}

void While_(BoolExpr b)
{
  While_(any(b));
}

//=============================================================================
// 'Where' token
//=============================================================================

void Where__(BExpr* b)
{
  Stmt* s = mkWhere(b, NULL, NULL);
  controlStack.push(s);
  stmtStack.push(mkSkip());
}

//=============================================================================
// 'For' token
//=============================================================================

void For_(Cond c)
{
  Stmt* s = mkFor(c.cexpr, NULL, NULL);
  controlStack.push(s);
  stmtStack.push(mkSkip());
}

void For_(BoolExpr b)
{
  For_(any(b));
}

void ForBody_()
{
  Stmt* s = controlStack.top();
  s->forLoop.inc = stmtStack.top();
  stmtStack.pop();
  stmtStack.push(mkSkip());
}

//=============================================================================
// 'Print' token
//=============================================================================

void Print(const char* str)
{
  Stmt* s = mkStmt();
  s->tag = PRINT;
  s->print.tag = PRINT_STR;
  s->print.str = str;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void Print(IntExpr x)
{
  Stmt* s = mkStmt();
  s->tag = PRINT;
  s->print.tag = PRINT_INT;
  s->print.expr = x.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

//=============================================================================
// VPM Setup
//=============================================================================

static void vpmSetupReadCore(int n, IntExpr addr, bool hor, int stride)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_VPM_READ;
  s->setupVPMRead.numVecs = n;
  s->setupVPMRead.stride = stride;
  s->setupVPMRead.hor = hor;
  s->setupVPMRead.addr = addr.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

static void vpmSetupWriteCore(IntExpr addr, bool hor, int stride)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_VPM_WRITE;
  s->setupVPMWrite.stride = stride;
  s->setupVPMWrite.hor = hor;
  s->setupVPMWrite.addr = addr.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void vpmSetupRead(Dir d, int n, IntExpr addr, int stride)
{
  vpmSetupReadCore(n, addr, d == HORIZ ? 1 : 0, stride);
}

void vpmSetupWrite(Dir d, IntExpr addr, int stride)
{
  vpmSetupWriteCore(addr, d == HORIZ ? 1 : 0, stride);
}

// ============================================================================
// DMA
// ============================================================================

void dmaSetReadPitch(IntExpr stride)
{
  Stmt* s = mkStmt();
  s->tag = SET_READ_STRIDE;
  s->stride = stride.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void dmaSetWriteStride(IntExpr stride)
{
  Stmt* s = mkStmt();
  s->tag = SET_WRITE_STRIDE;
  s->stride = stride.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr,
                    int rowLen, int vpitch)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_DMA_READ;
  s->setupDMARead.hor = dir == HORIZ ? 1 : 0;
  s->setupDMARead.numRows = numRows;
  s->setupDMARead.rowLen = rowLen;
  s->setupDMARead.vpitch = vpitch;
  s->setupDMARead.vpmAddr = vpmAddr.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen)
{
  Stmt* s = mkStmt();
  s->tag = SETUP_DMA_WRITE;
  s->setupDMAWrite.hor = dir == HORIZ ? 1 : 0;
  s->setupDMAWrite.numRows = numRows;
  s->setupDMAWrite.rowLen = rowLen;
  s->setupDMAWrite.vpmAddr = vpmAddr.expr;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void dmaReadWait()
{
  Stmt* s = mkStmt();
  s->tag = DMA_READ_WAIT;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

void dmaWriteWait()
{
  Stmt* s = mkStmt();
  s->tag = DMA_WRITE_WAIT;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

// ============================================================================
// QPU code for clean exit
// ============================================================================

void kernelFinish()
{
  // Ensure outstanding DMAs have completed
  dmaReadWait();
  dmaWriteWait();

  // QPU 0 waits until all other QPUs have finished
  // before sending a host IRQ.
  If (me() == 0)
    Int n = numQPUs()-1;
    For (Int i = 0, i < n, i++)
      semaDec(15);
    End
    hostIRQ();
  Else
    semaInc(15);
  End
}

}  // namespace QPULib

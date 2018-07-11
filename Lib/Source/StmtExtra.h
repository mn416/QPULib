#ifndef _QPULIB_SOURCE_STMTEXTRA_H_
#define _QPULIB_SOURCE_STMTEXTRA_H_

namespace QPULib {

//=============================================================================
// Host IRQ
//=============================================================================

inline void hostIRQ()
{
  Stmt* s = mkStmt();
  s->tag = SEND_IRQ_TO_HOST;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

//=============================================================================
// Semaphore access
//=============================================================================

inline void semaInc(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_INC;
  s->semaId = semaId;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

inline void semaDec(int semaId)
{
  Stmt* s = mkStmt();
  s->tag = SEMA_DEC;
  s->semaId = semaId;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

//=============================================================================
// VPM operations
//=============================================================================

inline void vpmPutExpr(Expr* e)
{
  Var v; v.tag = VPM_WRITE;
  Stmt* s = mkAssign(mkVar(v), e);
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

inline void vpmPut(IntExpr data)
  { vpmPutExpr(data.expr); }

inline void vpmPut(FloatExpr data)
  { vpmPutExpr(data.expr); }

template <typename T> inline void vpmPut(PtrExpr<T> data)
  { vpmPutExpr(data.expr); }

template <typename T> inline void vpmPut(Ptr<T> &data)
  { vpmPutExpr(data.expr); }

inline void dmaStartReadExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = DMA_START_READ;
  s->startDMARead = e;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

template <typename T> inline void dmaStartRead(PtrExpr<T> memAddr)
  { dmaStartReadExpr(memAddr.expr); }

template <typename T> inline void dmaStartRead(Ptr<T> &memAddr)
  { dmaStartReadExpr(memAddr.expr); }

inline void dmaStartWriteExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = DMA_START_WRITE; 
  s->startDMAWrite = e;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

template <typename T> inline void dmaStartWrite(PtrExpr<T> memAddr)
  { dmaStartWriteExpr(memAddr.expr); }

template <typename T> inline void dmaStartWrite(Ptr<T> &memAddr)
  { dmaStartWriteExpr(memAddr.expr); }

//=============================================================================
// Receive, request, store operations
//=============================================================================

inline void gatherExpr(Expr* e)
{
  Var v; v.tag = TMU0_ADDR;
  Stmt* s = mkAssign(mkVar(v), e);
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

template <typename T> inline void gather(PtrExpr<T> addr)
  { gatherExpr(addr.expr); }

template <typename T> inline void gather(Ptr<T>& addr)
  { gatherExpr(addr.expr); }

inline void receiveExpr(Expr* e)
{
  Stmt* s = mkStmt();
  s->tag = LOAD_RECEIVE;
  s->loadDest = e;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

inline void receive(Int& dest)
  { receiveExpr(dest.expr); }

inline void receive(Float& dest)
  { receiveExpr(dest.expr); }

template <typename T> inline void receive(Ptr<T>& dest)
  { receiveExpr(dest.expr); }

inline void storeExpr(Expr* e0, Expr* e1)
{
  Stmt* s = mkStmt();
  s->tag = STORE_REQUEST;
  s->storeReq.data = e0;
  s->storeReq.addr = e1;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

inline void store(IntExpr data, PtrExpr<Int> addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(FloatExpr data, PtrExpr<Float> addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(IntExpr data, Ptr<Int> &addr)
  { storeExpr(data.expr, addr.expr); }

inline void store(FloatExpr data, Ptr<Float> &addr)
  { storeExpr(data.expr, addr.expr); }

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_STMTEXTRA_H_

#ifndef _SOURCE_STMTEXTRA_H_
#define _SOURCE_STMTEXTRA_H_

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

inline void flush()
{
  Stmt* s = mkStmt();
  s->tag = FLUSH;
  stmtStack.replace(mkSeq(stmtStack.top(), s));
}

#endif

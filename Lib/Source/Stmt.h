#ifndef _QPULIB_SOURCE_STMT_H_
#define _QPULIB_SOURCE_STMT_H_

#include "../Source/Cond.h"
#include "../Source/Syntax.h"
#include "../Source/Ptr.h"
#include "../Source/StmtExtra.h"

namespace QPULib {

//=============================================================================
// Statement macros
//=============================================================================

#define If(c)    If_(c); {
#define Else     } Else_(); {
#define End      } End_();
#define While(c) While_(c); {
#define Where(b) Where_(b); {
#define For(init, cond, inc) \
  { init;                    \
    For_(cond);              \
      inc;                   \
    ForBody_();

//=============================================================================
// Statement tokens
//=============================================================================

void assign(Expr* lhs, Expr* rhs);
void If_(Cond c);
void If_(BoolExpr c);
void Else_();
void End_();
void While_(Cond c);
void While_(BoolExpr b);
void Where__(BExpr* b);
inline void Where_(BoolExpr b) { Where__(b.bexpr); }
void For_(Cond c);
void For_(BoolExpr b);
void ForBody_();
void Print(const char *);
void Print(IntExpr x);
void dmaSetReadPitch(IntExpr n);
void dmaSetWriteStride(IntExpr n);
void dmaSetupRead(Dir dir, int numRows, IntExpr vpmAddr,
                    int rowLen = 16, int vpitch = 1);
void dmaSetupWrite(Dir dir, int numRows, IntExpr vpmAddr, int rowLen = 16);
void dmaWaitRead();
void dmaWaitWrite();
void vpmSetupRead(Dir dir, int n, IntExpr addr, int stride = 1);
void vpmSetupWrite(Dir dir, IntExpr addr, int stride = 1);
void kernelFinish();

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_STMT_H_

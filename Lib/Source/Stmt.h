#ifndef _SOURCE_STMT_H_
#define _SOURCE_STMT_H_

#include "Source/Cond.h"
#include "Source/Syntax.h"
#include "Source/Ptr.h"
#include "Source/StmtExtra.h"

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
void setReadStride(IntExpr n);
void setWriteStride(IntExpr n);
void kernelFinish();

#endif

#ifndef _QPULIB_SOURCE_PRETTY_H_
#define _QPULIB_SOURCE_PRETTY_H_

#include "Source/Syntax.h"

namespace qpulib {

// Pretty printer for the QPULib source language
void pretty(Expr* e);
void pretty(BExpr* b);
void pretty(CExpr* c);
void pretty(Stmt* s);

}  // namespace qpulib

#endif  // _QPULIB_SOURCE_PRETTY_H_

#ifndef _QPULIB_SOURCE_PRETTY_H_
#define _QPULIB_SOURCE_PRETTY_H_

#include "../Source/Syntax.h"

namespace QPULib {

// Pretty printer for the QPULib source language
void pretty(FILE *f, Expr* e);
void pretty(FILE *f, BExpr* b);
void pretty(FILE *f, CExpr* c);
void pretty(FILE *f, Stmt* s);
void pretty(Stmt* s);

}  // namespace QPULib

#endif  // _QPULIB_SOURCE_PRETTY_H_

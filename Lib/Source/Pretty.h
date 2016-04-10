#ifndef _SOURCE_PRETTY_H_
#define _SOURCE_PRETTY_H_

#include "Source/Syntax.h"

// Pretty printer for the QPULib source language
void pretty(Expr* e);
void pretty(BExpr* b);
void pretty(CExpr* c);
void pretty(Stmt* s);

#endif

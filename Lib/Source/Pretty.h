#ifndef _SOURCE_PRETTY_H_
#define _SOURCE_PRETTY_H_

#include "Source/Syntax.h"

// Pretty printer for the QPULib source language
void pretty(FILE *f, Expr* e);
void pretty(FILE *f, BExpr* b);
void pretty(FILE *f, CExpr* c);
void pretty(FILE *f, Stmt* s);

#endif

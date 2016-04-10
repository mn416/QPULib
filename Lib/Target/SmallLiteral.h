#ifndef _SMALL_LITERAL_H_
#define _SMALL_LITERAL_H_

#include "Source/Syntax.h"
#include "Target/Emulator.h"

int encodeSmallLit(Expr* e);
bool isSmallLit(Expr* e);
void printSmallLit(int x);
Word decodeSmallLit(int x);

#endif

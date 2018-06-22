#ifndef _QPULIB_SMALL_LITERAL_H_
#define _QPULIB_SMALL_LITERAL_H_

#include "Source/Syntax.h"
#include "Target/Emulator.h"

namespace qpulib {

int encodeSmallLit(Expr* e);
bool isSmallLit(Expr* e);
void printSmallLit(int x);
Word decodeSmallLit(int x);

}  // namespace qpulib

#endif _QPULIB_SMALL_LITERAL_H_

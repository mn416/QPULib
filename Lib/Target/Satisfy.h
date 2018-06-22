#ifndef _QPULIB_SATISFY_H_
#define _QPULIB_SATISFY_H_

#include "Target/Syntax.h"
#include "Target/CFG.h"

namespace qpulib {

RegTag regFileOf(Reg r);
void satisfy(Seq<Instr>* instrs);

}  // namespace qpulib

#endif  // _QPULIB_SATISFY_H_

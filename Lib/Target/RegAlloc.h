#ifndef _QPULIB_REGALLOC_H_
#define _QPULIB_REGALLOC_H_

#include "../Target/CFG.h"
#include "../Target/Liveness.h"
#include "../Target/Syntax.h"
#include "../Common/Seq.h"

namespace QPULib {

void regAlloc(CFG* cfg, Seq<Instr>* instrs);

}  // namespace QPULib

#endif  // _QPULIB_REGALLOC_H_

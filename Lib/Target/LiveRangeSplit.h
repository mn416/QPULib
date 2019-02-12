#ifndef _QPULIB_LIVERANGESPLIT_H_
#define _QPULIB_LIVERANGESPLIT_H_

#include "../Common/Seq.h"
#include "../Target/CFG.h"
#include "../Target/Syntax.h"

namespace QPULib {

void liveRangeSplit(Seq<Instr>* instrs, CFG* cfg);

}  // namespace QPULib

#endif  // _QPULIB_LIVERANGESPLIT_H_

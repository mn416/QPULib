#ifndef _QPULIB_REMOVELABELS_H_
#define _QPULIB_REMOVELABELS_H_

#include "../Target/Syntax.h"
#include "../Target/CFG.h"
#include "../Target/Liveness.h"
#include "../Common/Seq.h"

namespace QPULib {

// Remove all labels, replacing absolute branch-label instructions
// with relative branch-target instructions.
void removeLabels(Seq<Instr>* instrs);

}  // namespace QPULib

#endif  // _QPULIB_REMOVELABELS_H_

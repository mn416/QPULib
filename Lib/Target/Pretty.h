#ifndef _QPULIB_TARGET_PRETTY_H_
#define _QPULIB_TARGET_PRETTY_H_

#include "Target/Syntax.h"

namespace qpulib {

// Pretty printer for the QPULib target language
void pretty(FILE *f, Instr instr);

}  // namespace qpulib

#endif  // _QPULIB_TARGET_PRETTY_H_

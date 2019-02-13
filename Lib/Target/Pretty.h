#ifndef _QPULIB_TARGET_PRETTY_H_
#define _QPULIB_TARGET_PRETTY_H_

#include <stdio.h>
#include "../Target/Syntax.h"

namespace QPULib {

// Pretty printer for the QPULib target language
void pretty(FILE *f, Instr instr);

}  // namespace QPULib

#endif  // _QPULIB_TARGET_PRETTY_H_

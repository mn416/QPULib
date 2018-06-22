#ifndef _QPULIB_ENCODE_H_
#define _QPULIB_ENCODE_H_

#include <stdint.h>
#include "Target/Syntax.h"
#include "Common/Seq.h"

namespace qpulib {

void encode(Seq<Instr>* instrs, Seq<uint32_t>* code);

}  // namespace qpulib

#endif  // _QPULIB_ENCODE_H_

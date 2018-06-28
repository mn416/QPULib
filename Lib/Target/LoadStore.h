#ifndef _QPULIB_LOADSTORE_H_
#define _QPULIB_LOADSTORE_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"

namespace QPULib {

void genSetReadStride(Seq<Instr>* instrs, int stride);
void genSetReadStride(Seq<Instr>* instrs, Reg stride);
void genSetWriteStride(Seq<Instr>* instrs, int stride);
void genSetWriteStride(Seq<Instr>* instrs, Reg stride);
void loadStorePass(Seq<Instr>* instrs);

}  // namespace QPULib

#endif  // _QPULIB_LOADSTORE_H_

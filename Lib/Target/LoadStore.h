#ifndef _LOADSTORE_H_
#define _LOADSTORE_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"

void genSetReadStride(Seq<Instr>* instrs, int stride);
void genSetReadStride(Seq<Instr>* instrs, Reg stride);
void genSetWriteStride(Seq<Instr>* instrs, int stride);
void genSetWriteStride(Seq<Instr>* instrs, Reg stride);
void loadStorePass(Seq<Instr>* instrs);

#endif

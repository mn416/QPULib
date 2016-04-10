#ifndef _REGALLOC_H_
#define _REGALLOC_H_

#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Target/Syntax.h"
#include "Common/Seq.h"

void regAlloc(CFG* cfg, Seq<Instr>* instrs);

#endif

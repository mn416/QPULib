#ifndef _LIVERANGESPLIT_H_
#define _LIVERANGESPLIT_H_

#include "Common/Seq.h"
#include "Target/CFG.h"
#include "Target/Syntax.h"

void liveRangeSplit(Seq<Instr>* instrs, CFG* cfg);

#endif

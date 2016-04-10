#ifndef _SATISFY_H_
#define _SATISFY_H_

#include "Target/Syntax.h"
#include "Target/CFG.h"

RegTag regFileOf(Reg r);
void satisfy(Seq<Instr>* instrs);

#endif

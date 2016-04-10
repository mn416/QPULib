#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include "Common/Seq.h"
#include "Source/Syntax.h"
#include "Target/Syntax.h"

void insertEndCode(Seq<Instr>* seq);
void translateStmt(Seq<Instr>* seq, Stmt* s);

#endif

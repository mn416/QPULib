// Liveness analysis

#ifndef _LIVENESS_H_
#define _LIVENESS_H_

#include "Common/Seq.h"
#include "Target/Syntax.h"
#include "Target/CFG.h"

// 'use' and 'def' sets:
//   * 'use' set: the variables read by an instruction
//   * 'def' set: the variables modified by an instruction

struct UseDefReg {
  SmallSeq<Reg> use;
  SmallSeq<Reg> def;
};   
     
struct UseDef {
  SmallSeq<RegId> use;
  SmallSeq<RegId> def;
};   

// Compute 'use' and 'def' sets for a given instruction

void useDefReg(Instr instr, UseDefReg* out);
void useDef(Instr instr, UseDef* out);
bool getTwoUses(Instr instr, Reg* r1, Reg* r2);

// A live set containts the variables
// that are live-in to an instruction.

typedef SmallSeq<RegId> LiveSet;

// The result of liveness analysis is a set
// of live variables for each instruction.

typedef Seq<LiveSet> Liveness;

// Determine the liveness sets for each instruction.

void liveness(Seq<Instr>* instrs, CFG* cfg, Liveness* liveness);
void computeLiveOut(CFG* cfg, Liveness* live, InstrId i, LiveSet* liveOut);

#endif

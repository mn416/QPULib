// Reaching definitions analysis

#ifndef _QPULIB_REACHINGDEFS_H_
#define _QPULIB_REACHINGDEFS_H_

#include "../Common/Seq.h"
#include "../Target/Syntax.h"
#include "../Target/CFG.h"

namespace QPULib {

// A reach set contains the instruction id's
// that reach an instruction.

typedef SmallSeq<InstrId> ReachSet;

// The result of the analysis is a set of
// instruction ids that reach each instruction.

typedef Seq<ReachSet> ReachingDefs;

// Determine the live definitions reaching each instruction.

void reachingDefs(Seq<Instr>* instrs, CFG* cfg, ReachingDefs* defs);

// Determine the instructions reached-by each definition.

void computeReachedBy(Seq<Instr>* instrs, CFG* cfg, ReachingDefs* reachedBy);

// Compute a mapping from each register id to a set of instruction ids
// that assign to that register.
typedef Seq<ReachSet> DefsOf;
void computeDefsOf(Seq<Instr>* instrs, DefsOf* defsOf);

}  // namespace QPULib

#endif  // _QPULIB_REACHINGDEFS_H_

#include <assert.h>
#include "Source/Syntax.h"
#include "Target/ReachingDefs.h"
#include "Target/Subst.h"
#include "Target/Liveness.h"

// ============================================================================
// Live-range splitter
// ============================================================================

// First, a helper function that renames the variable v defined by an
// instruction to w, along with all uses of that variable reached-by
// the instruction, and recursively all definitions of that variable
// that reach one of these uses.

void renameDef(Seq<Instr>* instrs,
               InstrId i,
               RegId v,
               RegId w,
               bool* visited,
               ReachingDefs* reachedBy,
               DefsOf* defsOf)
{
  // If we haven't previously visited instruction i then proceeed
  if (visited[i]) return;
  visited[i] = true;
  
  // Rename destination register from v to w
  Instr* instr = &instrs->elems[i];
  renameDest(instr, REG_A, v, REG_B, w);

  ReachSet* reached = &reachedBy->elems[i];
  // For each instruction reached by i
  for (int j = 0; j < reached->numElems; j++) {
    InstrId rid = reached->elems[j];
    Instr* r = &instrs->elems[rid];

    // Rename uses of v to w
    renameUses(r, REG_A, v, REG_B, w);

    // For each instruction d defining v
    SmallSeq<InstrId>* ds = &defsOf->elems[v];
    for (int k = 0; k < ds->numElems; k++) {
      InstrId d = ds->elems[k];
      // If r is reached-by d
      if (reachedBy->elems[d].member(rid)
            || (d == rid && isCondAssign(r)))
        // Recursively modify definition d to define w
        renameDef(instrs, d, v, w, visited, reachedBy, defsOf);
    }
  }
}

// Now for the top-level routine.

void liveRangeSplit(Seq<Instr>* instrs, CFG* cfg)
{
  // Determine for each variable, the instructions that assign to it
  DefsOf defsOf;
  computeDefsOf(instrs, &defsOf);

  // Determine instructions reached by each definition
  ReachingDefs reachedBy;
  computeReachedBy(instrs, cfg, &reachedBy);

  // Keep track of which instructions we've visisted
  bool* visited = new bool [instrs->numElems];

  // Initialise visited array
  for (int i = 0; i < instrs->numElems; i++)
    visited[i] = false;

  // Unique register id
  RegId next = 0;

  for (int i = 0; i < instrs->numElems; i++)
    if (!visited[i]) {
      // Compute vars defined by instruction
      UseDef set;
      useDef(instrs->elems[i], &set);

      // For each var defined by instruction
      for (int j = 0; j < set.def.numElems; j++)
        renameDef(instrs, i, set.def.elems[j], next++,
                  visited, &reachedBy, &defsOf);
    }

  // Every instruction should now soley use register file B.
  // Go through and make them use register file A instead.
  for (int i = 0; i < instrs->numElems; i++)
    substRegTag(&instrs->elems[i], REG_B, REG_A);

  // Update fresh var counter
  resetFreshVarGen(next);

  // Free memory
  delete [] visited;
}

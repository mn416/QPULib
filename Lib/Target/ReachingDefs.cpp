// Reaching definitions analysis

#include "Source/Syntax.h"
#include "Target/ReachingDefs.h"
#include "Target/Liveness.h"

// ============================================================================
// Compute 'defsOf' mapping
// ============================================================================

// Compute a mapping from each register id to a set of instruction ids
// that assign to that register.

typedef Seq<SmallSeq<InstrId>> DefsOf;

void computeDefsOf(Seq<Instr>* instrs, DefsOf* defsOf)
{
  int numVars = getFreshVarCount();
  defsOf->setCapacity(numVars);
  defsOf->numElems = numVars;

  for (int i = 0; i < instrs->numElems; i++) {
    UseDef set;
    useDef(instrs->elems[i], &set);
    for (int j = 0; j < set.def.numElems; j++) {
      RegId r = set.def.elems[j];
      defsOf->elems[r].insert(i);
    }
  }
}

// ============================================================================
// Compute 'usesOf' mapping
// ============================================================================

// Compute a mapping from each register id to a set of instruction ids
// that use that register.

typedef Seq<SmallSeq<InstrId>> UsesOf;

void computeUsesOf(Seq<Instr>* instrs, UsesOf* usesOf)
{
  int numVars = getFreshVarCount();
  usesOf->setCapacity(numVars);
  usesOf->numElems = numVars;

  for (int i = 0; i < instrs->numElems; i++) {
    UseDef set;
    useDef(instrs->elems[i], &set);
    for (int j = 0; j < set.use.numElems; j++) {
      RegId r = set.use.elems[j];
      usesOf->elems[r].insert(i);
    }
  }
}

// ============================================================================
// Compute 'gen' and 'kill' sets
// ============================================================================

// 'gen' set:  an instruction labelled x 'generates' x if it
//             modifies any variable.
// 'kill' set: an instruction labelled x that modifies a register y
//             kills all instructions that modify y, except x.

struct GenKill {
  SmallSeq<InstrId> gen;
  SmallSeq<InstrId> kill;
};

// Compute 'gen' and 'kill' sets for a given instruction

void computeGenKill(InstrId id, Instr instr, DefsOf* defsOf, GenKill* genKill)
{
  // Make the 'gen' and 'kill' sets empty
  genKill->gen.clear();
  genKill->kill.clear();

  // Does instruction modify a reg?
  bool isDef = false;

  // If so, which reg?
  RegId defReg;
  
  switch (instr.tag) {
    // Load immediate
    case LI:
      // Add destination reg to 'def' set
      if (instr.LI.dest.tag == REG_A) {
        isDef  = true;
        defReg = instr.LI.dest.regId;
      }
      break;

    // ALU operation
    case ALU:
      // Add destination reg to 'def' set
      if (instr.ALU.dest.tag == REG_A) {
        isDef  = true;
        defReg = instr.ALU.dest.regId;
      }
      break;

    // LD4 instruction
    case LD4:
      // Add dest reg to 'def' set
      if (instr.LD4.dest.tag == REG_A) {
        isDef  = true;
        defReg = instr.LD4.dest.regId;
      }
      break;
  }

  if (isDef) {
    genKill->gen.insert(id);
    SmallSeq<InstrId>* defs = &defsOf->elems[defReg];
    for (int i = 0; i < defs->numElems; i++)
      if (defs->elems[i] != id)
        genKill->kill.insert(defs->elems[i]);
  }
}

// ============================================================================
// Compute live reaching definitions for each instruction
// ============================================================================

// Helper function: given the reaching-out definitions, compute the
// reaching-in set for a given instruction.  For efficiency reasons,
// we only return live definitions that reach-in, but on the down-side
// this means we have to perform liveness analysis first.

void computeReachIn(Seq<Instr>* instrs, CFG* preds, Liveness* live,
                    ReachingDefs* defs, InstrId i, ReachSet* reachIn)
{
  LiveSet* liveIn = &live->elems[i];
  reachIn->clear();
  Succs* p = &preds->elems[i];
  for (int j = 0; j < p->numElems; j++) {
    ReachSet* set = &defs->elems[p->elems[j]];
    for (int k = 0; k < set->numElems; k++) {
      InstrId d = set->elems[k];
      // Compute vars defined by instruction
      UseDef useDefSet;
      useDef(instrs->elems[d], &useDefSet);
      // Only add live definitions to the set
      for (int n = 0; n < useDefSet.def.numElems; n++) {
        if (liveIn->member(useDefSet.def.elems[n])) {
          reachIn->insert(d);
          break;
        }
      }
    }
  }
}

void reachingOutDefs(Seq<Instr>* instrs, Liveness* live,
                     CFG* preds, ReachingDefs* defs)
{
  // Make sure defs is large enough
  defs->setCapacity(instrs->numElems);
  defs->numElems = instrs->numElems;

  // Find all definitions of each register
  DefsOf defsOf;
  computeDefsOf(instrs, &defsOf);

  // For storing the 'gen' and 'kill' sets of each instruction
  GenKill genKillSets;

  // For temporarily storing reaching-in and reaching-out definitions
  ReachSet reachIn;
  ReachSet reachOut;

  // Has a change been made to the reaching-definitions mapping?
  bool changed = true;

  // Iterate until no change, i.e. fixed point
  while (changed) {
    changed = false;

    // Propagate reaching definitions forward
    for (int i = 0; i < instrs->numElems; i++) {
      // Compute 'gen' and 'kill' sets
      Instr instr = instrs->elems[i];
      computeGenKill(i, instr, &defsOf, &genKillSets);

      // Compute reaching-in definitions
      computeReachIn(instrs, preds, live, defs, i, &reachIn);

      // Remove the 'kill' set from the reach-in set to give reach-out set
      reachOut.clear();
      for (int j = 0; j < reachIn.numElems; j++) {
        if (! genKillSets.kill.member(reachIn.elems[j]))
          reachOut.insert(reachIn.elems[j]);
      }

      // Add the 'gen' set to the reach-out set
      for (int j = 0; j < genKillSets.gen.numElems; j++)
        reachOut.insert(genKillSets.gen.elems[j]);

      // Insert the reach-out variables into the map
      for (int j = 0; j < reachOut.numElems; j++) {
        bool inserted = defs->elems[i].insert(reachOut.elems[j]);
        changed = changed || inserted;
      }
    }
  }
}

void reachingDefs(Seq<Instr>* instrs, CFG* cfg, ReachingDefs* defs)
{
  // For efficiency, perform liveness analysis first
  Liveness live;
  liveness(instrs, cfg, &live);

  // Reverse the arrows in the CFG
  CFG preds;
  reverseCFG(cfg, &preds);

  // Make sure defs is large enough
  defs->setCapacity(instrs->numElems);
  defs->numElems = instrs->numElems;

  // Compute defs reaching-out of each instruction
  ReachingDefs out;
  reachingOutDefs(instrs, &live, &preds, &out);
 
  // Compute defs reaching-in to each instruction
  for (int i = 0; i < defs->numElems; i++)
    computeReachIn(instrs, &preds, &live, &out, i, &defs->elems[i]);
}

// ============================================================================
// Compute instructions reached-by each definition
// ============================================================================

void computeReachedBy(Seq<Instr>* instrs, CFG* cfg, ReachingDefs* reachedBy)
{
  // Make sure reachedBy is large enough
  reachedBy->setCapacity(instrs->numElems);
  reachedBy->numElems = instrs->numElems;

  // Find all uses of each register
  UsesOf usesOf;
  computeUsesOf(instrs, &usesOf);

  // Compute definitions reaching each instruction
  ReachingDefs defs;
  reachingDefs(instrs, cfg, &defs);

  for (int i = 0; i < instrs->numElems; i++) {
    // Compute def set
    UseDef useDefSet;
    useDef(instrs->elems[i], &useDefSet);

    for (int j = 0; j < useDefSet.def.numElems; j++) {
      RegId r = useDefSet.def.elems[j];
      SmallSeq<InstrId>* uses = &usesOf.elems[r];
      for (int k = 0; k < uses->numElems; k++) {
        InstrId u = uses->elems[k];
        if (defs.elems[u].member(i))
          reachedBy->elems[i].insert(u);
      }
    }
  }
}

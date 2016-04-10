#include "Target/RemoveLabels.h"

// ============================================================================
// Remove labels
// ============================================================================

// Remove all labels, replacing absolute branch-label instructions
// with relative branch-target instructions.

void removeLabels(Seq<Instr>* instrs)
{
  Seq<Instr> newInstrs;

  // The number of labels in the instruction sequence
  int numLabels = getFreshLabelCount();

  // A mapping from labels to instruction ids
  InstrId* labels = new InstrId [numLabels];

  // Initialise label mapping
  for (int i = 0; i < numLabels; i++)
    labels[i] = -1;

  // First, remove labels, remembering the index of the instruction
  // pointed to by each label.
  for (int i = 0, j = 0; i < instrs->numElems; i++) {
    Instr instr = instrs->elems[i];
    if (instr.tag == LAB) {
      labels[instr.label] = j;
    }
    else {
      newInstrs.append(instr);
      j++;
    }
  }

  // Second, remove branch-label instructions.
  instrs->numElems = newInstrs.numElems;
  for (int i = 0; i < newInstrs.numElems; i++) {
    Instr instr = newInstrs.elems[i];
    if (instr.tag == BRL) {
      assert(instr.BRL.label >= 0 && instr.BRL.label < numLabels);
      int dest = labels[instr.BRL.label];
      assert (dest >= 0);
      BranchTarget t;
      t.relative       = true;
      t.useRegOffset   = false;
      t.immOffset      = dest - 4 - i;
      instr.tag        = BR;
      instr.BR.target  = t;
      instrs->elems[i] = instr;
    }
    else {
      instrs->elems[i] = instr;
    }
  }

  delete [] labels;
}

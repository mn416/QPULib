#ifndef _QPULIB_SUBST_H_
#define _QPULIB_SUBST_H_

#include "../Target/Syntax.h"

namespace QPULib {

// Rename a destination register in an instruction
void renameDest(Instr* instr, RegTag vt, RegId v,
                              RegTag wt, RegId w);

// Renamed a used register in an instruction
void renameUses(Instr* instr, RegTag vt, RegId v,
                              RegTag wt, RegId w);

// Globally change register tag vt to wt in given instruction
void substRegTag(Instr* instr, RegTag vt, RegTag wt);

}  // namespace QPULib

#endif  // _QPULIB_SUBST_H_

#include "Source/Translate.h"
#include "Target/Emulator.h"
#include "Target/RemoveLabels.h"
#include "Target/CFG.h"
#include "Target/Liveness.h"
#include "Target/ReachingDefs.h"
#include "Target/LiveRangeSplit.h"
#include "Target/RegAlloc.h"
#include "Target/Satisfy.h"
#include "Target/LoadStore.h"
#include "Target/Encode.h"

namespace qpulib {

// ============================================================================
// Compile kernel
// ============================================================================

void compileKernel(Seq<Instr>* targetCode, Stmt* body)
{
  // Translate to target code
  translateStmt(targetCode, body);

  // Load/store pass
  loadStorePass(targetCode);

  // Construct control-flow graph
  CFG cfg;
  buildCFG(targetCode, &cfg);

  // Apply live-range splitter
  //liveRangeSplit(targetCode, &cfg);

  // Perform register allocation
  regAlloc(&cfg, targetCode);

  // Satisfy target code constraints
  satisfy(targetCode);

  // Translate branch-to-labels to relative branches
  removeLabels(targetCode);
}

}  // namespace qpulib

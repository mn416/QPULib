#include "Source/Pretty.h"
#include "Source/Translate.h"
#include "Target/Pretty.h"
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

// ============================================================================
// Compile kernel
// ============================================================================

void compileKernel(Seq<Instr>* targetCode, Stmt* body)
{
  #ifdef DEBUG
    printf("Source code\n");
    printf("===========\n\n");
    pretty(body);
    printf("\n");
  #endif

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

  #ifdef DEBUG
    printf("Target code\n");
    printf("===========\n\n");
    for (int i = 0; i < targetCode->numElems; i++) {
      printf("%i: ", i);
      pretty(targetCode->elems[i]);
    }
    printf("\n");
  #endif

  // Translate branch-to-labels to relative branches
  removeLabels(targetCode);
}

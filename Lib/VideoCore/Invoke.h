#ifdef QPU_MODE

#ifndef _INVOKE_H_
#define _INVOKE_H_

#include "Common/Seq.h"
#include "VideoCore/SharedArray.h"
#include <stdint.h>

void invoke(
  int numQPUs,
  SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params);

#endif
#endif

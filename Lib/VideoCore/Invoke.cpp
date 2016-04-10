#ifdef QPU_MODE

#include "VideoCore/Invoke.h"
#include "VideoCore/Mailbox.h"
#include "VideoCore/VideoCore.h"

#define QPU_TIMEOUT 10000

void invoke(
  int numQPUs,
  SharedArray<uint32_t> &codeMem,
  int qpuCodeMemOffset,
  Seq<int32_t>* params)
{
  // Open mailbox for talking to VideoCore
  int mb = getMailbox();

  // Number of 32-bit words needed for kernel code & parameters
  int numWords = qpuCodeMemOffset + (params->numElems+2)*numQPUs + 2*numQPUs;
  assert(numWords < codeMem.size);

  // Pointer to start of code
  uint32_t* qpuCodePtr = codeMem.getPointer();

  // Copy parameters to instruction memory
  int offset = qpuCodeMemOffset;
  uint32_t** paramsPtr = new uint32_t* [numQPUs];
  for (int i = 0; i < numQPUs; i++) {
    paramsPtr[i] = qpuCodePtr + offset;
    codeMem[offset++] = (uint32_t) i; // Unique QPU ID
    codeMem[offset++] = (uint32_t) numQPUs; // QPU count
    for (int j = 0; j < params->numElems; j++)
      codeMem[offset++] = params->elems[j];
  }

  // Copy launch messages
  uint32_t* launchMsgsPtr = qpuCodePtr + offset;
  for (int i = 0; i < numQPUs; i++) {
    codeMem[offset++] = (uint32_t) paramsPtr[i];
    codeMem[offset++] = (uint32_t) qpuCodePtr;
  }

  // Launch QPUs
  unsigned result = 
    execute_qpu(mb, numQPUs, (uint32_t) launchMsgsPtr, 1, QPU_TIMEOUT);

  if (result != 0) {
    printf("Failed to invoke kernel on QPUs\n");
  }
}

#endif

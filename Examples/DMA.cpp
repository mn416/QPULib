#include "QPULib.h"

using namespace QPULib;

void dma(Ptr<Int> p)
{
  // Setup load of 16 vectors into VPM, starting at word address 0
  dmaSetupRead(HORIZ, 16, 0);
  // Start loading from memory at address 'p'
  dmaStartRead(p);
  // Wait until load complete
  dmaWaitRead();

  // Setup load of 16 vectors from VPM, starting at vector address 0
  vpmSetupRead(HORIZ, 16, 0);
  // Setup store to VPM, starting at vector address 16
  vpmSetupWrite(HORIZ, 16);

  // Read each vector, increment it, and write it back
  for (int i = 0; i < 16; i++)
    vpmPut(vpmGetInt() * 2);

  // Setup store of 16 vectors into VPM, starting at word address 256
  dmaSetupWrite(HORIZ, 16, 256);
  // Start writing to memory at address 'p'
  dmaStartWrite(p);
  // Wait until store complete
  dmaWaitWrite();
}

int main()
{
  // Construct kernel
  auto k = compile(dma);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(256);
  for (int i = 0; i < 256; i++)
    array[i] = i;

  // Invoke the kernel and display the result
  k(&array);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++)
      printf("%i ", array[16*i + j]);
    printf("\n");
  }
  
  return 0;
}

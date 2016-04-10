#include "QPULib.h"

// Define function that runs on the GPU.

void hello(Ptr<Int> p)
{
  *p = 1;
}

int main()
{
  // Construct kernel
  auto k = compile(hello);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16);
  for (int i = 0; i < 16; i++)
    array[i] = 100;

  // Invoke the kernel and display the result
  k(&array);
  for (int i = 0; i < 16; i++) {
    printf("%i: %i\n", i, array[i]);
  }
  
  return 0;
}

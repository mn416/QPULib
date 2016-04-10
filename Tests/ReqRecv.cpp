#include "QPULib.h"

// Define function that runs on the GPU.

void test(Ptr<Int> p)
{
  Int x, y;
  gather(p+index());
  gather(p+16+index());
  receive(x);
  receive(y);
  *p = x+y;
}

int main()
{
  // Construct kernel
  auto k = compile(test);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(32);
  for (int i = 0; i < 32; i++)
    array[i] = i;

  // Invoke the kernel and display the result
  k(&array);
  for (int i = 0; i < 16; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}

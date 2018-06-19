#include "QPULib.h"

// Define function that runs on the GPU.

void tri(Ptr<Int> p)
{
  Int n = *p;
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum+n;
      n = n-1;
    End
  End
  *p = sum;
}

int main()
{
  // Construct kernel
  auto k = compile(tri);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(16);
  for (int i = 0; i < 16; i++)
    array[i] = i;

  // Invoke the kernel and display the result
  k(&array);
  for (int i = 0; i < 16; i++)
    printf("%i: %i\n", i, array[i]);
  
  return 0;
}

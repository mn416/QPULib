#include <QPULib.h>

// Define function that runs on the GPU.

void hello(Ptr<Int> p)
{
  p = p + (me() << 4);
  *p = me();
}

int main()
{
  // Construct kernel
  auto k = compile(hello);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(192);
  for (int i = 0; i < 192; i++)
    array[i] = 0;

  // Invoke the kernel and display the result
  k.setNumQPUs(12);
  k(&array);
  for (int i = 0; i < 192; i++) {
    printf("%i: %i\n", i, array[i]);
  }
  
  return 0;
}

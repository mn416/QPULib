#include "QPULib.h"

// Odd/even transposition sorter for a 32-element array

void oet(Ptr<Int> p)
{
  setReadStride(1);
  setWriteStride(1);

  Int evens = *p;
  Int odds  = *(p+1);

  For (Int count = 0, count < 16, count++)
    Int evens2 = min(evens, odds);
    Int odds2  = max(evens, odds);

    Int evens3 = rotate(evens2, 15);
    Int odds3  = odds2;

    Where (index() != 15)
      odds2 = min(evens3, odds3);
    End

    Where (index() != 0)
      evens2 = rotate(max(evens3, odds3), 1);
    End

    evens = evens2;
    odds  = odds2;
  End

  *p     = evens;
  *(p+1) = odds;
}

int main()
{
  // Construct kernel
  auto k = compile(oet);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> a(32);
  for (int i = 0; i < 32; i++)
    a[i] = 100-i;

  // Invoke the kernel and display the result
  k.call(&a);
  for (int i = 0; i < 32; i++)
    printf("%i: %i\n", i, a[i]);
  
  return 0;
}

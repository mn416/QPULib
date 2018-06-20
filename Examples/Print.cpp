#include "QPULib.h"

void loop(Int n)
{
  For (Int i = 0, i < n, i++)
    Print(i);
    Print("\n");
  End
}

int main()
{
  // Construct kernel
  auto k = compile(loop);

  // Invoke kernel with argument 20
  k(20);
  
  return 0;
}

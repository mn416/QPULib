#include <sys/time.h>
#include <math.h>
#include "Rot3DKernels.h"

using namespace QPULib;

// #define USE_SCALAR_VERSION

// ============================================================================
// Main
// ============================================================================

int main()
{
  // Timestamps
  timeval tvStart, tvEnd, tvDiff;

  // Number of vertices and angle of rotation
  const int N = 19200; // 192000
  const float THETA = (float) 3.14159;

#ifdef USE_SCALAR_VERSION
  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }
#else
  // Construct kernel
  auto k = compile(rot3D_3);

  // Use 12 QPUs
  k.setNumQPUs(12);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }
#endif

  gettimeofday(&tvStart, NULL);
#ifdef USE_SCALAR_VERSION
  rot3D(N, cosf(THETA), sinf(THETA), x, y);
#else
  k(N, cosf(THETA), sinf(THETA), &x, &y);
#endif
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  //for (int i = 0; i < N; i++)
  //  printf("%f %f\n", x[i], y[i]);
 
  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);

  return 0;
}

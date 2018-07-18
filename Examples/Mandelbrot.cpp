#include <sys/time.h>
#include <QPULib.h>


using namespace QPULib;

#define USE_SCALAR_VERSION


#ifdef USE_SCALAR_VERSION
void output_pgm(int *result, int width, int height, int numIteratiosn) {
#else
void output_pgm(SharedArray<int> &result, int width, int height, int numIteratiosn) {
#endif
  FILE *fd = fopen("mandelbrot.pgm", "w") ;
  if (fd == nullptr) {
    printf("can't open file for pgm output\n");
    return;
  }

  // Write header
  fprintf(fd, "P2\n");
  fprintf(fd, "%d %d\n", width, height);
  fprintf(fd, "%d\n", numIteratiosn);

  int count; // Limit output to 10 elements per line
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      fprintf(fd, "%d ", result[x + width*y]);
      count++;
      if (count >= 10) {
        fprintf(fd, "\n");
        count = 0;
      }
    }
    fprintf(fd, "\n");
  }

  fclose(fd);
}


// ============================================================================
// Scalar version
// ============================================================================

void mandelbrot(
  float topLeftReal, float topLeftIm,
  float bottomRightReal, float bottomRightIm,
  int numStepsWidth, int numStepsHeight,
  int numiterations,
  int *result)
{
  float offsetX = (bottomRightReal - topLeftReal)/((float) numStepsWidth - 1);
  float offsetY = (topLeftIm - bottomRightIm)/((float) numStepsHeight - 1);

  for (int xStep = 0; xStep < numStepsWidth; xStep++) {
    for (int yStep = 0; yStep < numStepsHeight; yStep++) {
      float realC = topLeftReal   + ((float) xStep)*offsetX;
      float imC   = bottomRightIm + ((float) yStep)*offsetY;

      int count = 0;
      float real = realC;
      float im   = imC;
      float radius = (real*real + im*im);
      while (radius < 4 && count < numiterations) {
        float tmpReal = real*real - im*im;
        float tmpIm   = 2*real*im;
        real = tmpReal + realC;
        im   = tmpIm + imC;

        radius = (real*real + im*im);
        count++;
      }

      result[xStep + yStep*numStepsWidth] = count;
    }
  }
}


// ============================================================================
// Vector version
// ============================================================================


void prepare(
  float topLeftReal, float topLeftIm,
  float bottomRightReal, float bottomRightIm,
  int numStepsWidth, int numStepsHeight,
  SharedArray<float> &real, SharedArray<float> &im)
{
  float offsetX = (bottomRightReal - topLeftReal)/((float) numStepsWidth - 1);
  float offsetY = (topLeftIm - bottomRightIm)/((float) numStepsHeight - 1);

  for (int xStep = 0; xStep < numStepsWidth; xStep++) {
    for (int yStep = 0; yStep < numStepsHeight; yStep++) {
      float realC = topLeftReal   + ((float) xStep)*offsetX;
      float imC   = bottomRightIm + ((float) yStep)*offsetY;

      real[xStep + yStep*numStepsWidth] = realC;
      im[xStep + yStep*numStepsWidth] = imC;
    }
  }
}


void mandelbrot_1(
  Ptr<Float> inReal, Ptr<Float> inIm,
  Int num_items,
  Int numiterations,
  Ptr<Int> result)
{
  For (Int i = 0, i < num_items, i = i+16)
    Float realC = inReal[i];
    Float imC = inIm[i];

    Float real = realC;
    Float im = imC;
    Int count = 0;

    Float radius = (real*real + im*im);
    BoolExpr condition = (radius < 4 && count < numiterations);

    While (any(condition))
      Where (condition)
        Float tmpReal = real*real - im*im;
        Float tmpIm   = 2*real*im;
        real = tmpReal + realC;
        im   = tmpIm + imC;

        radius = (real*real + im*im);
        count++;
      End
    End

    result[i] = count;
  End
}


// ============================================================================
// Main
// ============================================================================

int main()
{
  // Timestamps
  timeval tvStart, tvEnd, tvDiff;

  // Initialize constants for the kernels
  const int numStepsWidth  = 192; //1024;
  const int numStepsHeight = 192; //1024;
  const int numiterations  = 1024;
  const float topLeftReal = -2.5f;
  const float topLeftIm =  2.0f;
  const float bottomRightReal = 1.5f;
  const float bottomRightIm = -2.0f;


#ifdef USE_SCALAR_VERSION
  // Allocate and initialise
  int *result = new int [numStepsWidth*numStepsHeight];
#else
  const int NUM_ITEMS = numStepsWidth*numStepsHeight;

  SharedArray<float> real(NUM_ITEMS);
  SharedArray<float> im(NUM_ITEMS);
  SharedArray<int>   result(NUM_ITEMS);

  // Initialize array values
  prepare(
    topLeftReal, topLeftIm,
    bottomRightReal, bottomRightIm,
    numStepsWidth, numStepsHeight,
    real, im);

  // Construct kernel
  auto k = compile(mandelbrot_1);

#endif

  gettimeofday(&tvStart, NULL);
#ifdef USE_SCALAR_VERSION
  mandelbrot(topLeftReal, topLeftIm,bottomRightReal, bottomRightIm, numStepsWidth, numStepsHeight, numiterations, result);
#else
  // TODO: run kernels
  k(
    &real, &im,
    NUM_ITEMS,
    numiterations,
    &result);
#endif
  output_pgm(result, numStepsWidth, numStepsHeight, numiterations);

  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);

  return 0;
}

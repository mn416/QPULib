/*
#include <sys/ioctl.h>
//nclude <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
*/
#include <sys/time.h>
#include <QPULib.h>


using namespace QPULib;

#define USE_SCALAR_VERSION


void output_pgm(int *result, int width, int height, int numIteratiosn) {
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

// TODO: Vector versions


// ============================================================================
// Main
// ============================================================================

int main()
{
  // Timestamps
  timeval tvStart, tvEnd, tvDiff;

  // Number of vertices and angle of rotation
  const int numStepsWidth  = 1024;
  const int numStepsHeight = 1024;
  const int numiterations  = 1024;


#ifdef USE_SCALAR_VERSION
  // Allocate and initialise
  int *result = new int [numStepsWidth*numStepsHeight];
#else
  // TODO: compile kernels, allocate memory
#endif

  gettimeofday(&tvStart, NULL);
#ifdef USE_SCALAR_VERSION
  mandelbrot(-2.5f, 2.0f, 1.5f, -2.0f, numStepsWidth, numStepsHeight, numiterations, result);
  output_pgm(result, numStepsWidth, numStepsHeight, numiterations);
#else
  // TODO: run kernels
#endif
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);

  return 0;
}

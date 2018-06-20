#include <QPULib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Heat dissapation constant
#define K 0.25

// ============================================================================
// Vector version
// ============================================================================

struct Cursor {
  Ptr<Float> addr;
  Float prev, current, next;

  void init(Ptr<Float> p) {
    gather(p);
    current = 0;
    addr = p+16;
  }

  void prime() {
    receive(next);
    gather(addr);
  }

  void advance() {
    addr = addr+16;
    prev = current;
    gather(addr);
    current = next;
    receive(next);
  }

  void finish() {
    receive(next);
  }

  void shiftLeft(Float& result) {
    result = rotate(current, 15);
    Float nextRot = rotate(next, 15);
    Where (index() == 15)
      result = nextRot;
    End
  }

  void shiftRight(Float& result) {
    result = rotate(current, 1);
    Float prevRot = rotate(prev, 1);
    Where (index() == 0)
      result = prevRot;
    End
  }
};

void step(Ptr<Float> map, Ptr<Float> mapOut, Int pitch, Int width, Int height)
{
  Cursor row[3];
  map = map + pitch*me() + index();

  // Skip first row of output map
  mapOut = mapOut + pitch;

  For (Int y = me(), y < height, y=y+numQPUs())

    // Point p to the output row
    Ptr<Float> p = mapOut + y*pitch;

    // Initilaise three cursors for the three input rows
    for (int i = 0; i < 3; i++) row[i].init(map + i*pitch);
    for (int i = 0; i < 3; i++) row[i].prime();

    // Compute one output row
    For (Int x = 0, x < width, x=x+16)

      for (int i = 0; i < 3; i++) row[i].advance();

      Float left[3], right[3];
      for (int i = 0; i < 3; i++) {
        row[i].shiftLeft(right[i]);
        row[i].shiftRight(left[i]);
      }

      Float sum = left[0] + row[0].current + right[0] +
                  left[1] +                  right[1] +
                  left[2] + row[2].current + right[2];

      store(row[1].current - K * (row[1].current - sum * 0.125), p);
      p = p + 16;

    End

    // Cursors are finished for this row
    for (int i = 0; i < 3; i++) row[i].finish();

    // Move to the next input rows
    map = map + pitch*numQPUs();

  End
}

// ============================================================================
// Main
// ============================================================================

int main()
{
  // Size of 2D heat map is WIDTH*HEIGHT:
  //   * with zero padding, it is NROWS*NCOLS
  //   * i.e. there is constant cold at the edges
  //   * NCOLs should be a multiple of 16
  //   * HEIGHT should be a multiple of NQPUS
  const int NQPUS  = 1;
  const int WIDTH  = 512-16;
  const int NCOLS  = WIDTH+16;
  const int HEIGHT = 504;
  const int NROWS  = HEIGHT+2;
  const int NSPOTS = 10;
  const int NSTEPS = 1500;

  // Timestamps
  timeval tvStart, tvEnd, tvDiff;

  // Allocate and initialise input and output maps
  SharedArray<float> mapA(NROWS*NCOLS), mapB(NROWS*NCOLS);
  for (int y = 0; y < NROWS; y++)
    for (int x = 0; x < NCOLS; x++) {
      mapA[y*NCOLS+x] = 0;
      mapB[y*NCOLS+x] = 0;
    }

  // Inject hot spots
  srand(0);
  for (int i = 0; i < NSPOTS; i++) {
    int t = rand() % 256;
    int x = rand() % WIDTH;
    int y = 1 + rand() % HEIGHT;
    mapA[y*NCOLS+x] = (float) (1000*t);
  }

  // Compile kernel
  auto k = compile(step);

  // Invoke kernel
  k.setNumQPUs(NQPUS);
  gettimeofday(&tvStart, NULL);
  for (int i = 0; i < NSTEPS; i++) {
    if (i & 1)
      k(&mapB, &mapA, NCOLS, WIDTH, HEIGHT);
    else
      k(&mapA, &mapB, NCOLS, WIDTH, HEIGHT);
  }
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  printf("P2\n%i %i\n255\n", WIDTH, HEIGHT);
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++) {
      int t = (int) mapB[(y+1)*NCOLS+x];
      t = t < 0   ? 0 : t;
      t = t > 255 ? 255 : t;
      printf("%d\n", t);
    }

  // Run-time of simulation
  printf("# %ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);

  return 0;
}

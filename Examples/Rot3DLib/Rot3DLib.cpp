#include <sys/time.h>
#include <unistd.h>  // sleep()
#include <math.h>
#include "VideoCore/PerformanceCounters.h"
#include "Rot3DKernels.h"

using namespace Rot3DLib;
using PC = PerformanceCounters;
using Generator = decltype(rot3D_1);  // All kernel functions except scalar have same prototype


// Number of vertices and angle of rotation
const int N = 19200; // 192000
const float THETA = (float) 3.14159;


timeval runScalar() {
	printf("Running scalar\n");

  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  timeval tvStart, tvEnd, tvDiff;
  gettimeofday(&tvStart, NULL);

  rot3D(N, cosf(THETA), sinf(THETA), x, y);

  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  //for (int i = 0; i < N; i++)
  //  printf("%f %f\n", x[i], y[i]);

	delete x;
	delete y;

	return tvDiff;
}


timeval runKernel(int index) {
	printf("Running kernel %d\n", index);

	Generator *kGenerator = nullptr;

	switch (index) {
	  case 1: kGenerator = rot3D_1; break;
	  case 2: kGenerator = rot3D_2; break;
	  case 3: kGenerator = rot3D_3; break;
		default:
			printf("ERROR: No kernel with index %d\n", index);
			exit(-1);
  };
	

  // Construct kernel
  auto k = compile(kGenerator);

  // Use 12 QPUs
  k.setNumQPUs(12);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  timeval tvStart, tvEnd, tvDiff;
  gettimeofday(&tvStart, NULL);

  k(N, cosf(THETA), sinf(THETA), &x, &y);

  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  // Display results
  //for (int i = 0; i < N; i++)
  //  printf("%f %f\n", x[i], y[i]);

	return tvDiff;
}


/**
 * @brief Enable the counters we are interested in
 */
void initPerfCounters() {
	PC::Init list[] = {
		{ 0, PC::QPU_INSTRUCTIONS },
		{ 1, PC::QPU_STALLED_TMU },
		{ 2, PC::L2C_CACHE_HITS },
		{ 3, PC::L2C_CACHE_MISSES },
		{ 4, PC::QPU_INSTRUCTION_CACHE_HITS },
		{ 5, PC::QPU_INSTRUCTION_CACHE_MISSES },
		{ 6, PC::QPU_CACHE_HITS },
		{ 7, PC::QPU_CACHE_MISSES },
		{ 8, PC::QPU_IDLE },
		{ PC::END_MARKER, PC::END_MARKER }
	};

	PC::enable(list);
	PC::clear(PC::enabled());

	//printf("Perf Count mask: %0X\n", PC::enabled());

	// The following will show zeroes for all counters, *except*
	// for QPU_IDLE, because this was running from the clear statement.
	// Perhaps there are more counters like that.
	//std::string output = PC::showEnabled();
	//printf("%s\n", output.c_str());
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[])
{
	initPerfCounters();

  timeval tvDiff;

	int index = 3;

	if (argc > 1) {
 		index = atoi(argv[1]);
  }

	if (index == 0) {
		tvDiff = runScalar();
  } else {
		tvDiff = runKernel(index);
  }

	// Show values current counters
	std::string output = PC::showEnabled();
	printf("%s\n", output.c_str());

  printf("%ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);

  return 0;
}

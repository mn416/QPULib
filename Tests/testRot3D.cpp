#include "catch.hpp"
#include <math.h>
#include "../Examples/Rot3DKernels.h"

using namespace QPULib;
using namespace Rot3D;

// ============================================================================
// Support routines
// ============================================================================

void initSharedArrays(SharedArray<float> &x, SharedArray<float> &y, int size) {
  for (int i = 0; i < size; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }
}


template<typename Array1, typename Array2>
void compareResults(
	Array1 &x1,
	Array1 &y1,
	Array2 &x2,
	Array2 &y2,
	int size,
	const char *label,
	bool compare_exact = true) {
  for (int i = 0; i < size; i++) {
		INFO("Comparing " << label << " for index " << i);
		if (compare_exact) {
			REQUIRE(x1[i] == x2[i]);
			REQUIRE(y1[i] == y2[i]);
		} else {
			REQUIRE(x1[i] == Approx(x2[i]).epsilon(0.001));
			REQUIRE(y1[i] == Approx(y2[i]).epsilon(0.001));
		}
  }
}


// ============================================================================
// The actual tests
// ============================================================================

TEST_CASE("Test working of Rot3D example", "[rot3d]") {
  // Number of vertices and angle of rotation
  const int N = 19200; // 192000
  const float THETA = (float) 3.14159;


	/**
	 * Check that the Rot3D kernels return precisely what we expect.
	 *
	 * The scalar version of the algorithm may return slightly different
	 * values than the actual QPU's, but they should be close
	 */
	SECTION("All kernel versions should return the same") {
		//
		// Run the scalar version
		//

	  // Allocate and initialise
	  float* x_scalar = new float [N];
	  float* y_scalar = new float [N];
	  for (int i = 0; i < N; i++) {
	    x_scalar[i] = (float) i;
	    y_scalar[i] = (float) i;
	  }

	  rot3D(N, cosf(THETA), sinf(THETA), x_scalar, y_scalar);

  	// Allocate and arrays shared between ARM and GPU
	  SharedArray<float> x_1(N), y_1(N);
	  SharedArray<float> x(N), y(N);


		// Compare scalar with QPU output - will not be exact
		{
	  	auto k = compile(rot3D_1);
			initSharedArrays(x_1, y_1, N);
  		k(N, cosf(THETA), sinf(THETA), &x_1, &y_1);
			compareResults(x_scalar, y_scalar, x_1, y_1, N, "Rot3D_1 with Scalar", false);
		}


		// Compare outputs of all the kernel versions. This *should* be exact		
		{
	  	auto k = compile(rot3D_2);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_1, y_1, x, y, N, "Rot3D_2");
		}

		{
	  	auto k = compile(rot3D_3);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_1, y_1, x, y, N, "Rot3D_3");
		}

		// Do rot3D_3 with multiple QPU's
		{
	  	auto k = compile(rot3D_3);
  		k.setNumQPUs(4);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_1, y_1, x, y, N, "Rot3D_3 4 QPU's");
		}
	}


	SECTION("Multiple kernel definitions should be possible") {
	  	auto k_1 = compile(rot3D_1);
	  	SharedArray<float> x_1(N), y_1(N);
			initSharedArrays(x_1, y_1, N);
  		k_1(N, cosf(THETA), sinf(THETA), &x_1, &y_1);

	  	auto k_2 = compile(rot3D_2);
	  	SharedArray<float> x_2(N), y_2(N);
			initSharedArrays(x_2, y_2, N);
  		k_2(N, cosf(THETA), sinf(THETA), &x_2, &y_2);

			compareResults(x_1, y_1, x_2, y_2, N, "Rot3D_3 4 QPU's");
	}
}

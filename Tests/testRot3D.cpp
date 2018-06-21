#include "catch.hpp"
#include <QPULib.h>
#include <math.h>

//
// The following is taken from the Rot3D example, for the purpose of illustrating
// the unit tests. 
// TODO: DRY this code and the example.
//

// ============================================================================
// Scalar version
// ============================================================================

void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y)
{
  for (int i = 0; i < n; i++) {
    float xOld = x[i];
    float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  }
}

// ============================================================================
// Vector version 1
// ============================================================================

void rot3D_1(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  For (Int i = 0, i < n, i = i+16)
    Float xOld = x[i];
    Float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  End
}

// ============================================================================
// Vector version 2
// ============================================================================

void rot3D_2(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = 16;
  Ptr<Float> p = x + index();
  Ptr<Float> q = y + index();
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
    store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}


// ============================================================================
// Vector version 3
// ============================================================================

void rot3D_3(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = numQPUs() << 4;
  Ptr<Float> p = x + index() + (me() << 4);
  Ptr<Float> q = y + index() + (me() << 4);
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
    store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}



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
	Array1 &x_scalar,
	Array2 &y_scalar,
	SharedArray<float> &x,
	SharedArray<float> &y,
	int size,
	const char *label) {
  for (int i = 0; i < size; i++) {
		INFO("Comparing " << label << " for index " << i);
		REQUIRE(x_scalar[i] == x[i]);
		REQUIRE(y_scalar[i] == y[i]);
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
	 * The scalar version of the algorithm is used as reference here.
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
	  SharedArray<float> x(N), y(N);

		{
	  	auto k = compile(rot3D_1);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_scalar, y_scalar, x, y, N, "Rot3D_1");
		}

		{
	  	auto k = compile(rot3D_2);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_scalar, y_scalar, x, y, N, "Rot3D_2");
		}

		{
	  	auto k = compile(rot3D_3);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_scalar, y_scalar, x, y, N, "Rot3D_3");
		}

		// Do rot3D_3 with multiple QPU's
		{
	  	auto k = compile(rot3D_3);
  		k.setNumQPUs(4);
			initSharedArrays(x, y, N);
  		k(N, cosf(THETA), sinf(THETA), &x, &y);
			compareResults(x_scalar, y_scalar, x, y, N, "Rot3D_3 4 QPU's");
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

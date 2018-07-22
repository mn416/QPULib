#include "catch.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <QPULib.h>

using namespace QPULib;
using namespace std;


//=============================================================================
// Helper methods
//=============================================================================

string showResult(SharedArray<int> &result, int index) {
  ostringstream buf;

  buf << "result  : ";
  for (int j = 0; j < 16; j++) {
    buf << result[16*index + j] << " ";
  }
  buf << "\n";

  return buf.str();
}


string showExpected(const std::vector<int> &expected) {
  ostringstream buf;

  buf << "expected: ";
  for (int j = 0; j < 16; j++) {
    buf << expected[j] << " ";
  }
  buf << "\n";

  return buf.str();
}


//=============================================================================
// Kernel definition(s)
//=============================================================================

void out(Int &res, Int &outIndex, Ptr<Int> &result) {
  result[outIndex] = res;
  outIndex = outIndex + 16;
}


void test(Cond cond, Int &outIndex, Ptr<Int> &result) {
  Int res = -1;

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, outIndex, result);
}


/**
 * @brief Overload for BoolExpr
 *
 * TODO: Why is distinction BoolExpr <-> Cond necessary? Almost the same
 */
void test(BoolExpr cond, Int &outIndex, Ptr<Int> &result) {
  Int res = -1;

  If (cond)
    res = 1;
  Else
    res = 0;
  End

  out(res, outIndex, result);
}


/**
 * @brief Kernel for testing If and When
 */
void kernelIfWhen(Ptr<Int> result) {
  Int outIndex = index();
  Int a = index();
  Int res = -1;  // temp variable for result of condition, -1 is unexpected value

  // any
  test(any(a <   0), outIndex, result);
  test(any(a <   8), outIndex, result);
  test(any(a <=  0), outIndex, result);  // Boundary check
  test(any(a >= 15), outIndex, result);  // Boundary check
  test(any(a <  32), outIndex, result);
  test(any(a >  32), outIndex, result);

  // all
  test(all(a <   0), outIndex, result);
  test(all(a <   8), outIndex, result);
  test(all(a <=  0), outIndex, result);  // Boundary check
  test(all(a >= 15), outIndex, result);  // Boundary check
  test(all(a <  32), outIndex, result);
  test(all(a >  32), outIndex, result);

  // Just If - should be same as any
  test((a <  0),     outIndex, result);
  test((a <  8),     outIndex, result);
  test((a <=  0),    outIndex, result);  // Boundary check
  test((a >= 15),    outIndex, result);  // Boundary check
  test((a <  32),    outIndex, result);
  test((a >  32),    outIndex, result);

  // When
  res = -1;
  Where (a < 0) res = 1; Else res = 0; End
  out(res, outIndex, result);

  res = -1;
  Where (a <= 0) res = 1; Else res = 0; End  // Boundary check
  out(res, outIndex, result);

  res = -1;
  Where (a >= 15) res = 1; Else res = 0; End  // Boundary check
  out(res, outIndex, result);

  res = -1;
  Where (a < 8) res = 1; Else res = 0; End
  out(res, outIndex, result);

  res = -1;
  Where (a >= 8) res = 1; Else res = 0; End
  out(res, outIndex, result);

  res = -1;
  Where (a < 32) res = 1; Else res = 0; End
  out(res, outIndex, result);

  res = -1;
  Where (a > 32) res = 1; Else res = 0; End
  out(res, outIndex, result);
}


//=============================================================================
// Unit tests
//=============================================================================

TEST_CASE("Test correct working DSL", "[dsl]") {
  vector<int> allZeroes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  vector<int> allOnes   = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

  auto assertResult = [] (
    SharedArray<int> &result,
    int index,
    const std::vector<int> &expected) {
    INFO("showResult index: " << index);

    if (result.size != 16) {
      INFO("length result: " << result.size);
      REQUIRE(result.size == 16);
    }

    if (expected.size() != 16) {
      INFO("length expected: " << expected.size());
      REQUIRE(expected.size() == 16);
    }

    bool passed = true;
    for (int j = 0; j < 16; ++j) {
      if (result[16*index + j] != expected[j]) {
        passed = false;
        break;
      }
    }

    INFO(showResult(result, index) << showExpected(expected));
    REQUIRE(passed);
  };


  //
  // Test all variations of If and When
  //
  SECTION("Conditionals work as expected") {
    const int N = 1;  // Number of expected result vectors

    // Construct kernel
    auto k = compile(kernelIfWhen);

    // Allocate and array for result values
    SharedArray<int> result(16*N);
    for (int i = 0; i < N; i++) {
      result[i] = -1;  // Initialize to unexpected value
    }
    SharedArray<float> x(N), y(N);

    // Run kernel
    k(&result);

    // any
    assertResult(result,  0, allZeroes);
    assertResult(result,  1, allOnes);
    assertResult(result,  2, allOnes);
    assertResult(result,  3, allOnes);
    assertResult(result,  4, allOnes);
    assertResult(result,  5, allZeroes);
    // all
    assertResult(result,  6, allZeroes);
    assertResult(result,  7, allZeroes);
    assertResult(result,  8, allZeroes);
    assertResult(result,  9, allZeroes);
    assertResult(result, 10, allOnes);
    assertResult(result, 11, allZeroes);
    // Just If - should be same as any
    assertResult(result, 12, allZeroes);
    assertResult(result, 13, allOnes);
    assertResult(result, 14, allOnes);
    assertResult(result, 15, allOnes);
    assertResult(result, 16, allOnes);
    assertResult(result, 17, allZeroes);
    // where
    assertResult(result, 18, allZeroes);
    assertResult(result, 19, {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
    assertResult(result, 20, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1});
    assertResult(result, 21, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0});
    assertResult(result, 22, {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1});
    assertResult(result, 23, allOnes);
    assertResult(result, 24, allZeroes);
  }
}

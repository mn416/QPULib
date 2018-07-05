//
// Unit tests using the Catch(2) framework.
//
// This file serves as a placeholder for main(), which is generated by Catch.
// General unit tests could be added here.
//
// See the tutorial for an overview: https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md
// This also links to the reference pages.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <cstdlib>
#ifdef QPU_MODE
#include "../Lib/Target/Emulator.h"  // MAX_QPUS
#include "../Lib/VideoCore/RegisterMap.h"

using RegMap = QPULib::RegisterMap;
#endif  // QPU_MODE


//
// get the base directory right for calling AutoTest
//
#ifdef DEBUG
	#define POSTFIX_DEBUG "-debug"
#else
	#define POSTFIX_DEBUG ""
#endif

#ifdef QPU_MODE
//	#pragma message "QPU mode enabled"
	#define POSTFIX_QPU "-qpu"
#else
	#define POSTFIX_QPU ""
#endif

const char *AUTOTEST_PATH = "obj" POSTFIX_DEBUG POSTFIX_QPU "/bin/AutoTest";


//
// This is a good place to put simple, global tests
//

TEST_CASE("Check random specifications for interpreter and emulator2", "[specs]") {
	printf("Running AutoTest from '%s'\n", AUTOTEST_PATH);
	REQUIRE(system(AUTOTEST_PATH) == 0);
}


#ifdef QPU_MODE

TEST_CASE("Test correct working of RegisterMap", "[regmap]") {

	//
	// The purpose of this test is two-fold:
  //
  // - Check that the register indeed returns the expected value.
  // - Warn the value ever changes. The number of QPU's has been 12 from the beginning for Pi,
  //   but should it ever change I want to know.
	//
	//
	SECTION("Check num QPU's") {
		// printf("Checking num QPU's\n");
		REQUIRE(MAX_QPUS == RegMap::numSlices()*RegMap::numQPUPerSlice());
	}
}

#endif  // QPU_MODE

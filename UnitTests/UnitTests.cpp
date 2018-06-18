//
// Unit tests using the CATCH2 framework
//
// See the tutorial for an overview: https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md
//

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"


unsigned int Factorial( unsigned int number ) {
//	return number <= 1 ? number : Factorial(number-1)*number;  // Fails unit test for number == 0
  return number > 1 ? Factorial(number-1)*number : 1;          // Passes unit test
}


TEST_CASE( "Example unit test taken from the CATCH2 github" ) {
	// This place can be used to define test-wide variables
	const int FINAL_VALUE = 3628800;

	REQUIRE( FINAL_VALUE > 1024);  // Can also test here

	SECTION( "Factorials are computed", "[factorial]" ) {
	    REQUIRE( Factorial(0) == 1 );
	    REQUIRE( Factorial(1) == 1 );
	    REQUIRE( Factorial(10) == FINAL_VALUE );
	}

	SECTION( "More factorials are computed", "[factorial]" ) {
	    REQUIRE( Factorial(2) == 2 );
	    REQUIRE( Factorial(3) == 6 );
	    REQUIRE( Factorial(10) == FINAL_VALUE );
	}
}

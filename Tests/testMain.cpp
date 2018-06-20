//
// Unit tests using the CATCH2 framework
//
// See the tutorial for an overview: https://github.com/catchorg/Catch2/blob/master/docs/tutorial.md
// This also links to the reference pages.
//

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

// Below is an example unit test which demonstrates the essential parts.
// It will be removed as soon as serious tests are added.


unsigned int Factorial( unsigned int number ) {
	//return number <= 1 ? number : Factorial(number-1)*number;  // Fails unit test for number == 0
    return number > 1 ? Factorial(number-1)*number : 1;          // Passes unit test
}


TEST_CASE( "Example unit test taken from the CATCH2 github" ) {
	// Test-wide declarations can be put here
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

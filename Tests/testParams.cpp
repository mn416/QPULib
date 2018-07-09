#include "catch.hpp"
#include "../Examples/Rot3DLib/Support/CmdParameter.h"
#include "Support/cout_redirect.h"
#include "TestData/TestParameters.h"


//////////////////////////////////////////////////
// The actual unit tests
//////////////////////////////////////////////////

const char *PROG = "TestProg";  // Name of dummy executable


/**
 * TODO: check border cases for param's with limits
 */
TEST_CASE("Test Command Line parameters", "[params]") {
	cout_redirect redirect;
	TestParameters params;  // NOTE: putting this in global space causes a segfault


	SECTION("Check help") {
		redirect.clear();

		int argc1 = 2;
		const char *argv1[] = { PROG, "-h"	};
		REQUIRE(params.handle_commandline(argc1, argv1, false));

		redirect.close();
		std::cout << "redirect: " << redirect.str() << std::endl;

		int argc2 = 8;
		const char *argv2[] = { PROG, "it", "doesn't", "matter",
														"-h", "what", "I", "put", "here"	};
		REQUIRE(params.handle_commandline(argc2, argv2, false));
	}


	SECTION("Check defaults of parameters") {
		int argc1 = 2;
		const char *argv1[] = {
			PROG,
			"input_file.txt",
		};

		REQUIRE(params.handle_commandline(argc1, argv1, false));

		// Test illegal cases
		REQUIRE(params.m_positive  != -1);
		REQUIRE(params.m_unsigned  != -1);
		REQUIRE(params.m_float     != -1.0f);

		REQUIRE(params.m_positive  == 123);
		REQUIRE(params.m_unsigned  == 42);
		REQUIRE(params.m_float     == Approx(3.1419).epsilon(0.00001));
		REQUIRE(params.input_file  == "input_file.txt");
		REQUIRE(params.output_file == "output_file.txt");
		REQUIRE(params.m_bool == true);
		REQUIRE(params.bool_detected == true);
	}


	SECTION("Check good parameters") {
		redirect.clear();

		int argc1 = 7;
		const char *argv1[] = {
			PROG,
			"-positive=123",
			"-unsigned=42",
			"-float=3.1419",
			"input_file.txt",
			"-output=output_file.txt",
			"-bool"
		};

		params.handle_commandline(argc1, argv1, false);
		//REQUIRE(params.handle_commandline(argc1, argv1, false));

		redirect.close();
		std::cout << "redirect: " << redirect.str() << std::endl;

		REQUIRE(params.m_positive  == 123);
		REQUIRE(params.m_unsigned  == 42);
		REQUIRE(params.m_float     == Approx(3.1419).epsilon(0.00001));
		REQUIRE(params.input_file  == "input_file.txt");
		REQUIRE(params.output_file == "output_file.txt");
		REQUIRE(params.m_bool == true);
		REQUIRE(params.bool_detected == true);
	}


	SECTION("Check bad parameters") {
		int argc1 = 3;
		const char *argv1[] = { PROG, "-positive=-1", "input_file.txt"	};
		REQUIRE(!params.handle_commandline(argc1, argv1, false));

		int argc2 = 3;
		const char *argv2[] = { PROG, "-float=3.14string", "input_file.txt"	};
		REQUIRE(!params.handle_commandline(argc2, argv2, false));
	}


	SECTION("Same names for parameter definitions should not be allowed") {
		const char *usage = "blurb";

		DefParameter double_params[] = {
			{	"Name not unique", "", UNNAMED,	"" },
			{	"Name not unique", "", UNNAMED,	"" },
			nullptr
		};

		REQUIRE(!CmdParameter::init_params(usage, double_params));
	}
}

#include "TestParameters.h"
#include "../Examples/Rot3DLib/Support/CmdParameter.h"

static const char *usage =
"TestProg text of unit testing command line parameters\n"
"\n"
"Usage:\n"
"    Rot3DLib input_file {options}\n"
"\n"
"Where:\n"
"    input_file         - filename not passed in as parameter\n"
;



DefParameter defined_parameters[] = {
  {
    "Unsigned integer",
    "-unsigned=",
    UNSIGNED_INTEGER,
    "Some value which must not be negative"
  },
  {
    "Positive integer",
    "-positive=",
    POSITIVE_INTEGER,
    "It can't be negative and also not zero."
  },
  {
    "Positive float",
    "-float=",
    POSITIVE_FLOAT,
    "A float which can't be zero or smaller"
  },
  {
    "Output file",
    "-output=",
    STRING,
    "The name of some file we want to output to"
  },
  {
    "A boolean value",
    "-bool",
    NONE,
    "This value is just plain set. There is no value"
  },
  {
    "Input file",
    "first_file",
    UNNAMED,
    "The first unnamed parameter, which we take to be a filename"
  },

  nullptr   // End marker
};


//////////////////////////////////////////////////
// Class TestParameters
//////////////////////////////////////////////////


bool TestParameters::handle_commandline(
  int argc,
  const char* argv[],
  bool show_help_on_error) {

  if (!CmdParameter::init_params(usage, defined_parameters)) {
    return false;
  }

  if (CmdParameter::handle_commandline(argc, argv, show_help_on_error)) {
    pass_params();
    return true;
  }

  return false;
}


void TestParameters::pass_params() {
  CmdParameter::List &parameters = CmdParameter::parameters;

  // This is the weak spot in the cmd handling;
  // need indexes into the array.
  // TODO: Find something better.
  m_unsigned  = parameters[0]->get_int_value();
  m_positive  = parameters[1]->get_int_value();
  m_float     = parameters[2]->get_float_value();
  output_file = parameters[3]->get_string_value();
  m_bool      = parameters[4]->get_bool_value();
  input_file  = parameters[5]->get_string_value();

  bool_detected = parameters[4]->detected();
}


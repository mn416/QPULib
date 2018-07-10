/*
 * Parameters.cpp
 *
 * Command line parameter definitions for the `Rot3DLib` tool.
 *
 * ------------------------------------------------------------
 * ## TODO
 *
 * ## Add support for function specifiers
 *
 * e.g.  `Rot3DLib profile <params and options>`
 *
 * - functions can have their own params and options
 * - global options should be possible - must not conflict with function options
 * - if functions present, they *must* always be defined.
 *   I.e. no global run and functions at the same time.
 */
#include "Parameters.h"
#include <cassert>
#include <sstream>
#include <iostream>

using namespace std;

static const char *usage =
"Rot3DLib - A testbed for QPULib functionality\n"
"\n"
"This application serves as a platform for new and extended functionality for `QPULib`\n"
"It uses the kernels of the `Rot3D` example.\n"
"\n"
"Usage:\n"
"    Rot3DLib {options}\n"
;



DefParameter defined_parameters[] = {
  {
    "Kernel index",
    "-kernel-index=",
    INTEGER,
    "The index of the kernel to run."
    "Index zero is the scalar kernel, which runs on the CPU.\n"
    "Valid values: 0,1,2,3,4 ",
    3  // default value
  },
  nullptr   // End marker
};




//////////////////////////////////////////////////
// Class Parameters
//////////////////////////////////////////////////

bool Parameters::handle_commandline(
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


void Parameters::pass_params() {
  CmdParameter::List &parameters = CmdParameter::parameters;

  kernel_index  = parameters[0]->get_int_value();
}

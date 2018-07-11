#ifndef ITERATOR_PARAMETERS_H
#define ITERATOR_PARAMETERS_H
#include <string>
#include <vector>
#include "CmdParameter/CmdParameter.h"

class Parameters {
public:
  void handle_commandline(int argc, const char* argv[]);

  // Parameter values
  int kernel_index;

private:
  void pass_params();
};

#endif // ITERATOR_PARAMETERS_H

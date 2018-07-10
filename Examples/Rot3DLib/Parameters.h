#ifndef ITERATOR_PARAMETERS_H
#define ITERATOR_PARAMETERS_H
#include <string>
#include <vector>
#include "CmdParameter/CmdParameter.h"


class Parameters {
public:
  bool handle_commandline(
    int argc,
    const char* argv[],
    bool show_help_on_error = true);

  // Parameter values
  int kernel_index;

private:
  void pass_params();
};


#endif // ITERATOR_PARAMETERS_H

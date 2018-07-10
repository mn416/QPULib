#include "NoneParameter.h"
#include <cassert>
#include <string>

using std::string;

bool NoneParameter::parse_param_internal(const std::string &in_value) {
  return parse_bool_param(in_value);
}

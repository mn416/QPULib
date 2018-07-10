#include "IntParameter.h"
#include <cassert>
#include <string>
#include <sstream>


bool IntParameter::parse_param_internal(const std::string &in_value) {
  assert(!in_value.empty());

  int value = get_int_value(in_value);

  int_value = value;
  m_detected = true;
  return true;
}


void IntParameter::default_indicator(std::ostringstream &os) {
  os << def_param.int_default;
}

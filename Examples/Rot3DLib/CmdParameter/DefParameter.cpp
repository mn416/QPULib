/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:21 +0200
 */
#include "DefParameter.h"
#include <cassert>
#include <limits>
#include <cmath>


const int   DefParameter::INT_NOT_SET   = std::numeric_limits<int>::min();

/**
 * @brief Contructor accepting null pointer as single parameter
 *
 * Used to indicate and detect the end of a parameterdefinition array.
 */
DefParameter::DefParameter(const char *in_name) :
  name(in_name),
  prefix(nullptr),
  param_type(NONE),
  usage(nullptr) {
  handle_defaults();
}


DefParameter::DefParameter(
  const char *in_name,
  const char *in_prefix,
  ParamType   in_param_type,
  const char *in_usage) :
  name(in_name),
  prefix(in_prefix),
  param_type(in_param_type),
  usage(in_usage) {
  handle_defaults();
}


DefParameter::DefParameter(
  const char *in_name,
  const char *in_prefix,
  ParamType   in_param_type,
  const char *in_usage,
  int default_value) :
  name(in_name),
  prefix(in_prefix),
  param_type(in_param_type),
  usage(in_usage),
  int_default(default_value) {
  handle_defaults();
}




bool DefParameter::is_int_type() const {
  switch(param_type) {
  case INTEGER:
    return true;

  default:
    return false;
  }
}


void DefParameter::handle_defaults() {
  switch(param_type) {
  case INTEGER:
    if (int_default == INT_NOT_SET) {
      int_default = 0;
    }
  break;
  default:
    // do nothing
    break;
  }
}


bool DefParameter::has_default() const {
  switch(param_type) {
  case INTEGER:
    return (int_default != INT_NOT_SET);

  default:
    // Nothing special here
    break;
  }

  return true;
}

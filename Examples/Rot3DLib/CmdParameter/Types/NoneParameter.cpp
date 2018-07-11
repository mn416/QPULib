/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:21 +0200
 */
#include "NoneParameter.h"
#include <cassert>
#include <string>

using std::string;

bool NoneParameter::parse_param_internal(const std::string &in_value) {
  return parse_bool_param(in_value);
}

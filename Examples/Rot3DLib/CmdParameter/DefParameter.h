/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:22 +0200
 */
#ifndef DEFPARAMETER_H
#define DEFPARAMETER_H
#include "Types/ParamType.h"
#include <string>


/**
 * @brief Structure for the definitions of the parameters
 */
struct DefParameter {
  static const int   INT_NOT_SET;

  DefParameter(const char *in_name);

  DefParameter(
    const char *in_name,
    const char *in_prefix,
    ParamType in_param_type,
    const char *in_usage
  );

  DefParameter(
    const char *in_name,
    const char *in_prefix,
    ParamType   in_param_type,
    const char *in_usage,
    int default_value
  );

  const char *name;       //> The name or short description of the parameter.
                          //  This value is used as label and must be unique
  const char *prefix;
  ParamType   param_type; //> Parameter type
  const char *usage;      //> Long description of the parameter, displayed in the help text

  int          int_default{INT_NOT_SET};

  bool is_int_type() const;
  bool has_default() const;

private:
  void handle_defaults();
};


#endif // DEFPARAMETER_H

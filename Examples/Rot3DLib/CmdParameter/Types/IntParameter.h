/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:22 +0200
 */
#ifndef INTPARAMETER_H
#define INTPARAMETER_H
#include "../CmdParameter.h"

struct IntParameter : public CmdParameter {
public:
  IntParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return "<num>"; }
  void default_indicator(std::ostringstream &os) override;

private:
  bool parse_param_internal(const std::string &in_value) override;
};

#endif // INTPARAMETER_H

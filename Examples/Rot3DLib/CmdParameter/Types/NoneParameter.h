/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:21 +0200
 */
#ifndef NONEPARAMETER_H
#define NONEPARAMETER_H
#include "../CmdParameter.h"


struct NoneParameter : public CmdParameter {
public:
  NoneParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return ""; }
  void default_indicator(std::ostringstream &os) override { /* do nothing */ }

private:
  bool parse_param_internal(const std::string &in_value) override;
	bool takes_value() const override { return false; }
};


#endif // NONEPARAMETER_H

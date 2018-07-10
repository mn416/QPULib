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
};


#endif // NONEPARAMETER_H

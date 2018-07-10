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

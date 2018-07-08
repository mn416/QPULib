#ifndef CMDPARAMETER_H
#define CMDPARAMETER_H
#include <memory>
#include <string>
#include <vector>

enum ParamType {
  NONE,             //> Parameter without a value, e.g. `-enabled`
  UNSIGNED_INTEGER, //> Parameter takes an integer >= 0
  POSITIVE_INTEGER, //> Parameter takes a integer > 0
  POSITIVE_FLOAT,   //> Parameter takes a float > 0
  STRING,           //> Parameter takes a string, e.g. `-output=filename.txt`
  UNNAMED           //> Not a switch parameter, but expected on the command line.
                    //  Order is important
};


/**
 * @brief Structure for the definitions of the parameters
 */
struct DefParameter {
  DefParameter(const char *in_name);

  DefParameter(
    const char *in_name,
    const char *in_prefix,
    ParamType in_param_type,
    const char *in_usage
  );

  DefParameter(
    const char *in_name,
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

  bool         bool_default;
  int          int_default;
  float        float_default;
  std::string  string_default;
};


/**
 * @brief Defines and processes command line parameters.
 *
 * ----------------------------------------------------------
 * ## NOTES
 *
 * * The goal of this class is to make the definitions and usage of
 *   the parameters easy. The downside is that this code becomes
 *   more complex. I think this is a worthwhile tradeoff.
 */
struct CmdParameter {
  using List = std::vector<std::unique_ptr<CmdParameter>>;


  const char *name;
  const char *prefix;
  ParamType   param_type;
  const char *usage;

  CmdParameter(DefParameter &var);

  bool        get_bool_value()   { return bool_value; }
  int         get_int_value()    { return int_value; }
  float       get_float_value()  { return float_value; }
  std::string get_string_value() { return string_value; }

  bool detected() const { return m_detected; }

  bool parse_param(const char *curarg);

  static bool handle_commandline(
    int argc,
    const char* argv[],
    bool show_help_on_error = true);

  static bool init_params(const char *in_usage, DefParameter params[]);
  static bool process_option(List &parameters, const char *curarg);
  static bool process_unnamed(List &parameters, const char *curarg);
  static void show_usage();

  // TODO: make private
  static List parameters;

protected:
  bool m_detected;

  // Yes, crappy. Go ahead and enjoy your nausea.
  bool         bool_value;
  bool         bool_default;
  int          int_value;
  int          int_default;
  float        float_value;
  float        float_default;
  std::string  string_value;
  std::string  string_default;

  bool parse_bool_param  (const std::string &in_value);
  bool parse_int_param   (const std::string &in_value);
  bool parse_float_param (const std::string &in_value);
  bool parse_string_param(const std::string &in_value);

  int         get_int_value   (const std::string &param);
  float       get_float_value (const std::string &param);
  std::string get_string_value(const std::string &param) {
    return get_param(param.c_str());
  }

  std::string get_param(const char *curarg);

private:

  static const char *usage_text;

  virtual bool parse_param_internal(const std::string &in_value);  // = 0;
  virtual const char *value_indicator() const;  // = 0;

  static std::string pad(const std::string &str, unsigned width);
};


struct NoneParameter : public CmdParameter {
public:
  NoneParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return ""; }

private:
  bool parse_param_internal(const std::string &in_value) override {
    return parse_bool_param(in_value);
  }
};


struct UnsignedIntParameter : public CmdParameter {
public:
  UnsignedIntParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return "<num>"; }

private:
  bool parse_param_internal(const std::string &in_value) override;
};


struct PositiveIntParameter : public CmdParameter {
public:
  PositiveIntParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return "<num>"; }

private:
  bool parse_param_internal(const std::string &in_value) override;
};


struct PositiveFloatParameter : public CmdParameter {
public:
  PositiveFloatParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return "<float>"; }

private:
  bool parse_param_internal(const std::string &in_value) override;
};


struct UnnamedParameter : public CmdParameter {
public:
  UnnamedParameter(DefParameter &var) : CmdParameter(var) {}
  const char *value_indicator() const override { return ""; }

private:
  bool parse_param_internal(const std::string &in_value) override {
    return parse_string_param(in_value);
  }
};

#endif // CMDPARAMETER_H

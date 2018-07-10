#ifndef CMDPARAMETER_H
#define CMDPARAMETER_H
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include "DefParameter.h"


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
  class List : public std::vector<std::unique_ptr<CmdParameter>> {
   public:
    CmdParameter *operator[] (int index);
    CmdParameter *operator[] (const char *key);
  };

  DefParameter &def_param;

  CmdParameter(DefParameter &var);

  int         get_int_value()    { return int_value; }
  bool detected() const { return m_detected; }
  bool parse_param(const char *curarg);

  static bool handle_commandline(
    int argc,
    const char *argv[],
    bool show_help_on_error = true);

  static bool init_params(const char *in_usage, DefParameter params[]);
  static bool process_option(List &parameters, const char *curarg);
  static void show_usage();

  // TODO: make private
  static List parameters;

protected:
  bool m_detected{false};
  int  int_value{-1};

  int  get_int_value(const std::string &param);
  bool parse_bool_param(const std::string &in_value);
  std::string get_param(const char *curarg);

private:
  static DefParameter help_def;
  static const char *usage_text;

  bool set_default();

  virtual bool parse_param_internal(const std::string &in_value) = 0;
  virtual const char *value_indicator() const = 0;
  virtual void default_indicator(std::ostringstream &os) = 0;

  static bool handle_help(int argc, const char *argv[]);
  static std::string pad(const std::string &str, unsigned width);
};

#endif // CMDPARAMETER_H

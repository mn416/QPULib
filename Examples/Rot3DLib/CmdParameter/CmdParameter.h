/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:22 +0200
 */
#ifndef CMDPARAMETER_H
#define CMDPARAMETER_H
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include "DefParameter.h"


/**
 * @brief Defines and processes command line parameters.
 */
struct CmdParameter {
	enum ExitCode {
		ALL_IS_WELL   = -2,
		EXIT_NO_ERROR =  0,
		EXIT_ERROR    =  1
	};

	using string = std::string; // TODO adjust definitions

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

  static ExitCode handle_commandline(
    const char *in_usage,
    DefParameter params[],
    int argc,
    const char* argv[],
    bool show_help_on_error = true);

  static bool handle_commandline(
    int argc,
    const char *argv[],
    bool show_help_on_error = true);

  static bool init_params(const char *in_usage, DefParameter params[]);
  static bool process_option(List &parameters, const char *curarg);
  static void show_usage();
  static bool has_errors() { return m_has_errors; }

  // TODO: make private
  static List parameters;

protected:
	string m_prefix;
  bool   m_detected{false};
  int    int_value{-1};

  int  get_int_value(const string &param);
  bool parse_bool_param(const string &in_value);
	void error(const string &msg) const;
  string get_param(const char *curarg);

private:
  static bool m_has_errors;
  static DefParameter help_def;
  static const char *usage_text;

  bool set_default();

  virtual bool parse_param_internal(const string &in_value) = 0;
  virtual const char *value_indicator() const = 0;
  virtual void default_indicator(std::ostringstream &os) = 0;
	virtual bool takes_value() const { return true; }

  static bool handle_help(int argc, const char *argv[]);
  static string pad(const string &str, unsigned width);
};

#endif // CMDPARAMETER_H

/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:21 +0200
 */
/**
 * Classes for the handling of command line parameters.
 */
#include "CmdParameter.h"
#include <cassert>
#include <iostream>
#include <sstream>   // ostringstream
#include <vector>
#include <cstring>   // strcmp
#include "Support/Strings.h"
#include "Types/Types.h"

using namespace std;


//////////////////////////////////////////////
// Class CmdParameter::List
//////////////////////////////////////////////

/**
 * Grumbl need to redefine this after adding the key version.
 */
CmdParameter *CmdParameter::List::operator[] (int index) {
  return at(index).get();
}


CmdParameter *CmdParameter::List::operator[] (const char *key) {
  assert(key != nullptr);

  for (auto &ptr : *this) {
    auto &field = *ptr.get();
    if (0 == strcmp(field.def_param.name, key)) {
      return &field;
    }
  }

  string msg = "'";
  throw msg + key + "' is not a short name of a parameter.";
  return nullptr;
}


//////////////////////////////////////////////
// Class CmdParameter
//////////////////////////////////////////////

// Internal definition of help switch
DefParameter CmdParameter::help_def(
  "Help switch",
  "-h",
  NONE,
  "Show this information. Overrides all other parameters"
);

bool CmdParameter::m_has_errors = false;
CmdParameter::List CmdParameter::parameters;
const char *CmdParameter::usage_text = nullptr;


CmdParameter::CmdParameter(DefParameter &var) :
  def_param(var),
  m_detected(false) {

	// Remove '=' from the field prefix
  m_prefix = Strings::explode(var.prefix, '=')[0];
	assert(!m_prefix.empty());

  set_default();
}


void CmdParameter::error(const string &msg) const {
	string pre("Parameter '");
	pre += def_param.name;
	pre += "' (" + m_prefix + ") ";
	throw string(pre + msg);
}


bool CmdParameter::parse_param(const char *curarg) {
  if (!Strings::starts_with(curarg, m_prefix)) {
    return false;
  }

  string value = get_param(curarg);

	if (takes_value()) {
    if (value.empty()) {
			error("takes a value, none specified.");
		} else {
			// Disallow whitespace after '='
			if (std::isspace(static_cast<unsigned char>(value[0]))) {
				error(" has unexpected whitespace after '='.");
			}
		}
	} else {
		if (Strings::contains(curarg, "=") || !value.empty()) {
     error("has value specified, shouldn't have one.");
    }
  }

/*
  // All param's except type NONE should have a value specified
    if (value.empty()) {
      if (def_param.has_default()) {
        return true;  // All is well, we have a default
      } else {
        throw string(msg + " no value present and default not specified.");
      }
    }
*/

  return parse_param_internal(value);
}


int CmdParameter::get_int_value(const string &param) {
  int value = -1;
  const char *str = param.c_str();
  char *end = nullptr;
  value = (int) strtol(str, &end, 10);

  if (end == str || *end != '\0') {
    string msg("The value for field '");
    throw string(msg + def_param.name + "' is not a number.");
  }

  return value;
}


string CmdParameter::get_param(const char *curarg) {
  string ret;

  string::size_type loc = string(curarg).find("=");
  if (loc != string::npos) {
    ret = string(curarg).substr(loc + 1);
  }

  return ret;
}


string CmdParameter::pad(const string &str, unsigned width) {
  string out;

  for (auto i = str.length(); i < width; ++ i) {
    out += " ";
  }

  return str + out;
}


bool CmdParameter::process_option(List &parameters, const char *curarg) {
  for (auto &item: parameters) {
    CmdParameter &param = *item;

    if (param.parse_param(curarg)) {
      return true;
    }
  }

  // No option detected
  if (Strings::starts_with(curarg, "-")) {
    // Flag anything else that looks like an option param as an error
    throw string("Unknown command line option '") + (curarg + 1) + "'";
  }

  return false;
}


bool CmdParameter::parse_bool_param(const string &in_value) {
	assert(in_value.empty());
	assert(def_param.param_type == NONE);

  m_detected = true;
  return true;
}
/**
 * @brief Create help usage text for all the defined optional param's.
 *
 * Output per option has the form:
 *
 *   switch[=<param_type>] description[<text for default>]
 *
 * Some effort is done to keep the descriptions aligned.
 */
void CmdParameter::show_usage() {
  cout << usage_text;

  vector<string> disp_defaults;
  vector<string> disp_params;

  auto add_param = [&] (
    CmdParameter &param,
    const string &value_indicator,
    std::ostringstream &default_indicator) {
    string tmp;
    if (param.def_param.prefix != nullptr) {
      tmp = param.def_param.prefix;
    }
    tmp += value_indicator;
    disp_params.push_back(tmp);

    if (default_indicator.str().empty()) {
      disp_defaults.push_back(".");
    } else {
      std::ostringstream buf;
      buf << "; default '" << default_indicator.str() << "'.";
      disp_defaults.push_back(buf.str());
    }
  };


  // Help switch
  NoneParameter help_switch(help_def);
  string value_indicator;
  std::ostringstream default_indicator;

  add_param(help_switch, value_indicator, default_indicator);
  // end help switch

  //
  // Prepare the switch and value part of the usage line,
  // so that we can determine their max width for formatting.
  //
  // Also create display of defaults.
  //
  for (auto &item : parameters) {
    CmdParameter &param = *item;

    string value_indicator;
    std::ostringstream default_indicator;

    value_indicator = param.value_indicator();
    param.default_indicator(default_indicator);

    add_param(param, value_indicator, default_indicator);
  }

  // Determine max width of displayed param's
  unsigned width = 0;
  for (unsigned i = 0; i < disp_params.size(); ++i) {
    string &str = disp_params[i];
    // CRAP: Following fails when 'int width = -1', due to signedness
    if (str.length() > width) {
      width = (unsigned) str.length();
    }
  }

  cout << "\n"
          "options:\n"
          "    (Can appear in any position on the command line after the program name)\n";


  auto output_padded = [width] (
    CmdParameter &param,
    const string &disp_param,
    const string &disp_default) {
    // Ensure line endings have proper indent
    string usage = param.def_param.usage;
    string indent("\n");
    string mt;
    indent += pad(mt, width + 4 + 3);
    usage = Strings::implode(Strings::explode(usage, '\n'), indent.c_str());

    cout << "    " << pad(disp_param, width) << " - " << usage << disp_default << endl;
  };

  int index = 0;
  output_padded(help_switch, disp_params[index], disp_defaults[index]);
  index++;

  for (auto &item: parameters) {
    CmdParameter &param = *item;

    output_padded(param, disp_params[index], disp_defaults[index]);
    index++;
  }
}


/**
 * @brief Process the main text blurb and the parameter definitions
 *
 * The parameter definitions are converted to an internal representation, better
 * suited for parsing the actual values on the command line.
 *
 * @param in_usage main text to use for the help description
 * @param params   array of parameter definitions, ended by a 'nullptr' field
 *
 * @return true if all went well, false if an error occured during conversion.
 */
bool CmdParameter::init_params(const char *in_usage, DefParameter params[]) {
  parameters.clear();
  usage_text = in_usage;

  for (int index = 0; params[index].name != nullptr; ++index) {
    auto &item = params[index];
    CmdParameter *p = DefParameter_factory(item);
    parameters.emplace_back(p);
  }

  return true;
}


/**
 * @brief Scan for '-h' and handle if present.
 *
 * @return true '-h' handled, false otherwise
 */
bool CmdParameter::handle_help(int argc, const char *argv[]) {
  int curindex = 0;

	while (true) {
		curindex++;
		if (curindex >= argc) break;

		const char *curarg = argv[curindex];

		if (string("-h") == curarg) {
			CmdParameter::show_usage();
			return true;
		}
	}

	return false;
}


/**
 * @brief Override of `handle_commandline()` which also does `init_params()`.
 *
 * @return ALL_IS_WELL   if all is well,
 *         EXIT_NO_ERROR if should stop without errors,
 *         EXIT_ERROR    if should stop with errors
 */
CmdParameter::ExitCode CmdParameter::handle_commandline(
  const char *usage,
  DefParameter params[],
  int argc,
  const char* argv[],
  bool show_help_on_error) {
  if (!CmdParameter::init_params(usage, params)) {
    return EXIT_ERROR;
  }

  if (!CmdParameter::handle_commandline(argc, argv, show_help_on_error)) {
    if(CmdParameter::has_errors()) {
      return EXIT_ERROR;
    } else {
      return EXIT_NO_ERROR;
    }
  }

  return ALL_IS_WELL;
}


/**
 * @brief Handle the command line and initialize files/dir's.
 *
 * If help detected, this will return true but there should
 * be no further processing. The caller should check with
 * `has_errors()` if execution can continue.
 *
 * @return true if execution can continue, false otherwise
 */
bool CmdParameter::handle_commandline(
	int argc,
	const char *argv[],
	bool show_help_on_error) {
	ostringstream errors;

	m_has_errors = false;

	// Prescan for '-h'; this overrides everything
	if (handle_help(argc, argv)) return false;

	int curindex = 0;

	try {
		while (true) {
			curindex++;
			if (curindex >= argc) break;

			const char *curarg = argv[curindex];

			if (!CmdParameter::process_option(CmdParameter::parameters, curarg)) {
				errors << "  Unknown parameter '" << curarg << "'.\n";
			}
		}
	} catch (string &error) {
		if (error != "all is well") {
			errors << "  " << error.c_str() << endl;
		}
	}

	string err_string = errors.str();

	if (!err_string.empty()) {
		cout << "Error(s) on command line:\n" << err_string.c_str() << endl;

		if (show_help_on_error) {
			CmdParameter::show_usage();
		} else {
			cout << "  Use switch '-h' to view options\n"  << endl;
		}

		m_has_errors = true;
	}

	return !m_has_errors;
}


bool CmdParameter::set_default() {
  if (def_param.is_int_type()) {
    if (def_param.int_default != DefParameter::INT_NOT_SET) {
      // Use default instead
      int_value = def_param.int_default;
      return true;
    }
  } else {
    // All other cases for now: not handled
    // TODO: see if explicit default settings is needed for these types
    return false;
  }

  return false;
}

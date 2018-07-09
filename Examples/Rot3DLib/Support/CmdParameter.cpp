/**
 * Classes for the handling of command line parameters.
 *
 * The goals are:
 *
 *  - to make this as simple as possible *for usage*
 *  - to make is easy to add new parameter types
 *
 * Note that these are very different goals from making this
 * a simple class system.
 *
 * ------------------------------------------------------------
 * ## TODO
 *
 * <none right now>
 * ------------------------------------------------------------
 * ## TESTS
 *
 * - parameter labels must be unique
 * - unnamed params: required or not/arbitrary number or limit
 * - nonexistent param
 * - '-h' not at first
 * - unsigned int param bad or out of range (eg. -2, 0, 'meow')
 */
#include "CmdParameter.h"
#include <cassert>
#include <iostream>
#include <sstream>   // ostringstream
#include <vector>
#include "Strings.h"

using namespace std;


//////////////////////////////////////
// class DefParameter
//////////////////////////////////////

/**
 * @brief Contructor accepting null pointer as single parameter
 *
 * Used to indicate and detect the end of a parameterdefinition array.
 */
DefParameter::DefParameter(const char *in_name) :
  name(in_name),
  prefix(nullptr),
  param_type(NONE),
  usage(nullptr),
  bool_default(false),
  int_default(0),
  float_default(0.0)
{
}


DefParameter::DefParameter(
  const char *in_name,
  const char *in_prefix,
  ParamType   in_param_type,
  const char *in_usage
) :
  name(in_name),
  prefix(in_prefix),
  param_type(in_param_type),
  usage(in_usage),
  bool_default(false),
  int_default(0),
  float_default(0.0)
{}

DefParameter::DefParameter(
  const char *in_name,
  const char *in_prefix,
  ParamType   in_param_type,
  const char *in_usage,
  int default_value
) :
  name(in_name),
  prefix(in_prefix),
  param_type(in_param_type),
  usage(in_usage),
  bool_default(false),
  int_default(default_value),
  float_default(0.0)
{}


// TODO: check if really needed
DefParameter::DefParameter(
  const char *in_name,
  ParamType   in_param_type,
  const char *in_usage
) :
  name(in_name),
  prefix(nullptr),
  param_type(in_param_type),
  usage(in_usage),
  bool_default(false),
  int_default(0),
  float_default(0.0)
{}



//////////////////////////////////////
// class CmdParameter
//////////////////////////////////////

CmdParameter::List CmdParameter::parameters;
const char *CmdParameter::usage_text = nullptr;


CmdParameter::CmdParameter(DefParameter &var) :
  name(var.name),
  prefix(var.prefix),
  param_type(var.param_type),
  usage(var.usage),
  m_detected(false),
  bool_value(false),
  bool_default(var.bool_default),
  int_value(-1),
  int_default(var.int_default),
  float_value(-1.0f),
  float_default(var.float_default)
{ }


bool CmdParameter::parse_param(const char *curarg) {
  bool ret = false;

  if (!Strings::starts_with(curarg, prefix)) {
    return false;
  }

  string msg = name;
  string value = get_param(curarg);

  // All param's except type NONE should have a value specified
  if (param_type == NONE) {
    if (!value.empty()) {
      throw string( msg + " value specified, shouldn't have one.");
    }
  } else {
    if (value.empty()) {
      throw string( msg + " value not specified.");
    }
  }

  switch(param_type) {
  case NONE:
    ret = parse_bool_param(value);
    break;

  case STRING:
    ret = parse_string_param(value);
    break;

  case POSITIVE_INTEGER:
  case UNSIGNED_INTEGER:
  case POSITIVE_FLOAT:
    ret = parse_param_internal(value);
    break;

  default:
    throw string("Unknown parameter type detected.");
  }

  return ret;
}


bool CmdParameter::parse_param_internal(const std::string &in_value) {
  assert(false);  // Should be abstract eventually, this for development
}


const char *CmdParameter::value_indicator() const {
  assert(false);  // Should be abstract eventually, this for development
}


bool CmdParameter::parse_bool_param(const string &in_value) {
	assert(in_value.empty());
	assert(param_type == NONE);

  bool_value = true;
  m_detected = true;
  return true;
}


bool CmdParameter::parse_int_param(const string &in_value) {
	assert(!in_value.empty());

	int value = get_int_value(in_value);

  switch(param_type) {
  default:
		assert(false);
	}

  int_value = value;
  m_detected = true;
  return true;
}


bool CmdParameter::parse_float_param(const string &in_value) {
	assert(!in_value.empty());

	string msg = name;

	float value = get_float_value(in_value);

  switch(param_type) {
  case POSITIVE_FLOAT:
    if (value <= 0) {
      throw string(msg + " value must be positive.");
    }
    break;

	default:
		assert(false);
	}

  float_value = value;
  m_detected = true;
  return true;
}


bool CmdParameter::parse_string_param(const string &in_value) {
	assert(!in_value.empty());
	assert(param_type == STRING);

  string_value = in_value;
  m_detected = true;
  return true;
}



int CmdParameter::get_int_value(const string &param) {
  int value = -1;
  const char *str = param.c_str();
  char *end = nullptr;
  value = (int) strtol(str, &end, 10);

  if (end == str || *end != '\0') {
    string msg(name);
    throw string(msg + " value not a number.");
  }

  return value;
}


float CmdParameter::get_float_value(const string &param) {
  float value = -1;
  const char *str = param.c_str();
  char *end = nullptr;
  value = (float)strtod(str, &end);

  if (end == str || *end != '\0') {
    string msg(name);
    throw string(msg + " value not a float.");
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


bool CmdParameter::process_unnamed(List &parameters, const char *curarg) {
  for (auto &item: parameters) {
    CmdParameter &param = *item;
    if (param.param_type != UNNAMED) continue;
    if (param.string_value.empty()) {
      param.string_value = curarg;
      return true;
    }
  }

  return false;
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
    if (param.prefix != nullptr) {
      tmp = param.prefix;
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
  DefParameter help_def(
        "Help switch",
        "-h",
        NONE,
        "Show this information. Overrides all other parameters"
        );
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

    switch (param.param_type) {
      case NONE:
        break;
      case POSITIVE_FLOAT:
        value_indicator = "<float>";
        default_indicator << param.float_default;
        break;
      case STRING:
        value_indicator = "<string>";
        if (!param.string_default.empty()) {
          default_indicator << param.string_default;
        }
        break;

    case POSITIVE_INTEGER:
    case UNSIGNED_INTEGER:
      value_indicator = param.value_indicator();
      default_indicator << param.int_default;
      break;

      case UNNAMED:
        break;

      default:
        assert(false);
        string msg = param.name;
        throw string("Unknown parameter type for parameter '" + msg + "'");
    };

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
    string usage = param.usage;
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
  int length = 0;
  for (length = 0; params[length].name != nullptr; ++length) {}

  // Check that labels are unique
  bool labels_unique = true;
  for (int index1 = 0; index1 < length - 1; ++index1) {
    for (int index2 = index1 + 1; index2 < length; ++index2) {
      if (params[index1].name == params[index2].name) {
        std::ostringstream msg;
        msg << "Error: Multiple parameter definitions with name '" << params[index1].name << "'; "
            << "names should be unique";
        std::cout << msg.str() << std::endl;
        labels_unique = false;
      }
    }
  }

  if (!labels_unique) {
    return false;
  }


  parameters.clear();
  usage_text = in_usage;

  for (int index = 0; params[index].name != nullptr; ++index) {
    auto &item = params[index];

    CmdParameter *p = nullptr;
    switch (item.param_type) {
    case UNSIGNED_INTEGER: p = new UnsignedIntParameter(item);   break;
    case POSITIVE_INTEGER: p = new PositiveIntParameter(item);   break;
    case POSITIVE_FLOAT:   p = new PositiveFloatParameter(item); break;
    case  UNNAMED:         p = new UnnamedParameter(item);       break;

    // TODO
    case NONE:
    case STRING:
      p = new CmdParameter(item);
      break;

    default: {
        std::ostringstream msg;
        msg << "Error: Unhandled parameter type " << item.param_type;
        std::cout << msg.str() << std::endl;
        return false;
      }
    }

    assert(p != nullptr);
    parameters.emplace_back(p);

  }

  return true;
}


/**
 * @brief Handle the command line and initialize files/dir's.
 *
 * Will abort if an error is detected.
 */
bool CmdParameter::handle_commandline(
	int argc,
	const char* argv[],
	bool show_help_on_error) {
	bool has_error      = false;
	ostringstream errors;
	int curindex = 0;

	// Prescan for '-h'; this overrides everything
	while (true) {
		curindex++;

		if (curindex >= argc) {
			break;
		}

		const char *curarg = argv[curindex];

		if (string("-h") == curarg) {
			CmdParameter::show_usage();
			return true;
		}
	}

	try {
		curindex = 0;

		while (true) {
			curindex++;
			if (curindex >= argc) break;

			const char *curarg = argv[curindex];

			if (!CmdParameter::process_option(CmdParameter::parameters, curarg)) {
				// It's not one of the defined options, so it must be unnamed input
				if (!CmdParameter::process_unnamed(CmdParameter::parameters, curarg)) {
					errors << "  Too many unnamed parameters on command line, '" << curarg << " unexpected.\n";
				}
			}
		}

		// Check if all unnamed fields have a value
		for (auto &ptr : CmdParameter::parameters) {
			auto &field = *ptr.get();
			if (field.param_type != UNNAMED) continue;

			if (field.get_string_value().empty()) {
				errors << "  No " << field.name << " specified.\n";
			}
		}

	} catch (string &error) {
		if (error != "all is well") {
			errors << "  " << error.c_str() << endl;
			has_error = true;
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

		return false;
	}

	return true;
}


//////////////////////////////////////
// class UnsignedIntParameter
//////////////////////////////////////

bool UnsignedIntParameter::parse_param_internal(const std::string &in_value) {
  assert(!in_value.empty());

  int value = get_int_value(in_value);

  if (value < 0) {
    string msg = "Field '";
    throw string(msg + name + "' value must be zero or positive.");
  }

  int_value = value;
  m_detected = true;
  return true;
}


//////////////////////////////////////
// class PositiveIntParameter
//////////////////////////////////////

bool PositiveIntParameter::parse_param_internal(const std::string &in_value) {
  assert(!in_value.empty());

  int value = get_int_value(in_value);

  if (value <= 0) {
    string msg = "Field '";
    throw string(msg + name + "' value must be positive.");
  }

  int_value = value;
  m_detected = true;
  return true;
}


//////////////////////////////////////
// class PositiveFloatParameter
//////////////////////////////////////

bool PositiveFloatParameter::parse_param_internal(const std::string &in_value) {
  assert(!in_value.empty());

  float value = get_float_value(in_value);

  if (value <= 0.0f) {
    string msg = "Field '";
    throw string(msg + name + "' value must be positive.");
  }

  float_value = value;
  m_detected = true;
  return true;
}


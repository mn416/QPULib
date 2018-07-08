/*
 * Strings.h
 *
 *  Created on: May 26, 2014
 *      Author: wim
 */

#ifndef STRINGS_H_
#define STRINGS_H_
//#include "common/gabor_string.h"


/**
 * @brief Useful string handling methods.
 *
 * Shamelessly copied (and adjusted) from GaborLib/common
 */
class Strings {
public:
	static bool is_number(const std::string& s);

	static std::string toString(int val, const int MAX_WIDTH = 1);
	static std::string to_lower(const std::string &str);

	static bool contains( std::string const& haystack, std::string const& needle);

	/**
	 * Determine if the given string haystack has needle as prefix.
	 *
	 * @return true if prefix present, else false.
	 */
	static bool starts_with(std::string const& haystack, std::string const& needle);

	static bool ends_with (std::string const &fullString, std::string const &ending);
	static bool ends_with(std::string const &fullString, const char *ending);
	static bool remove_ending(std::string &str, const std::string &ending);
	static bool remove_ending(std::string &str, const char *ending);

	static std::string trim(const std::string& s, const std::string& delimiters = " \f\n\r\t\v");

	static std::vector<std::string> explode(const std::string& str, char sep = ',');
//	static std::vector<std::string> explode(const gabor::string& str, char sep = ',');

	static std::string implode( const std::vector<std::string>& elements, const char* const separator = ", ");

	static float to_float(std::istream &is);

	static bool includes(const char *arr[], const char *val);
	static bool includes(const char *arr[], const std::string &val);
};

#endif /* STRINGS_H_ */

/*
 * Strings.cpp
 *
 *  Created on: May 26, 2014
 *      Author: wim
 */
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include "Strings.h"


using namespace std;

/////////////////////////////////
// class Strings
/////////////////////////////////


bool Strings::is_number(const std::string& s) {
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}


/**
 * Utility function for converting iteration number to string usable as folder name.
 *
 * Values are converted with leading zeroes to a fixed width.
 */
string Strings::toString(int val, const int MAX_WIDTH) {
	ostringstream convert;

	convert << setw(MAX_WIDTH) << setfill('0') << val;

	return convert.str();
}


/**
 * @brief Return lower case version of input string.
 */
std::string Strings::to_lower(const std::string &str) {
	string data = str;

	std::transform(data.begin(), data.end(), data.begin(), ::tolower);

	return data;
}


/**
 * Determine if the given string haystack has needle as substring.
 *
 * @return true if substring present, else false.
 */
bool Strings::contains(std::string const& haystack, std::string const& needle) {
	return (haystack.find(needle) != string::npos);
}


/**
 * Determine if the given string haystack has needle as prefix.
 *
 * @return true if prefix present, else false.
 */
bool Strings::starts_with(std::string const& haystack, std::string const& needle) {
	return needle.length() <= haystack.length() &&
			std::equal(needle.begin(), needle.end(), haystack.begin());
}


/**
 * Determine if the given string fullString has ending as postfix.
 *
 * @return true if postfix present, else false.
 */
bool Strings::ends_with(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}


bool Strings::ends_with(std::string const &fullString, const char *ending) {
	if (ending == 0) return false;
	return ends_with(fullString, std::string(ending));
}


/**
 * @brief Remove given ending from string, if present.
 *
 * Value of parameter 'str' is changed on success
 *
 * @return true if string stripped of postfix, false otherwise
 */
bool Strings::remove_ending(std::string &str, const std::string &ending) {
	bool ret = false;

	// Source: http://stackoverflow.com/a/10532518
	if ( str != ending &&
			str.size() > ending.size() &&
			str.substr(str.size() - ending.size()) == ending )
	{
	   // if so then strip them off
		str = str.substr(0, str.size() - ending.size());

		ret = true;
	}

	return ret;
}


bool Strings::remove_ending(std::string &str, const char *ending) {
	if (ending == 0) {
		return false;
	}

	std::string tmp(ending);
	return remove_ending(str, tmp);
}

string Strings::trim(const std::string& s, const std::string& delimiters) {
	size_t pos = s.find_last_not_of( delimiters );

	string tmp;

	if ( string::npos == pos) {
		tmp = s;
	} else {
		tmp = s.substr(0, pos + 1);	// trim right
	}

	pos = tmp.find_first_not_of(delimiters);
	if ( string::npos != pos) {
		tmp = tmp.substr(pos);	// trim left
	}

	return tmp;
}


/**
 * Split a string into an array on the separation character.
 *
 * Source: http://stackoverflow.com/questions/890164/how-can-i-split-a-string-by-a-delimiter-into-an-array
 */
std::vector<std::string> Strings::explode(const std::string& str, char sep) {
	string next;
	vector<string> result;

	// For each character in the string
	for (unsigned i = 0 ; i < str.length() ; ++ i)
	{
		const char& it = str[i] ;

		// If we've hit the terminal character
		if (it == sep) {
			// If we have some characters accumulated
			if (!next.empty()) {
				// Add them to the result vector
				result.push_back(next);
				next.clear();
			}
		} else {
			// Accumulate the next character into the sequence
			next += it;
		}
	}
	if (!next.empty())
		result.push_back(next);

	// Remove whitespace on front and back
	for (unsigned i = 0 ; i < result.size() ; ++ i)
	{
		string &it  = result[i] ;
		it = trim(it);
	}

	return result;
}


/*
std::vector<std::string> Strings::explode(const gabor::string &str, char sep) {
	std::string tmp(str.c_str());
	return explode(tmp, sep);
}
*/


/**
 * Source: http://stackoverflow.com/questions/5288396/c-ostream-out-manipulation/5289170#5289170
 */
string Strings::implode(const vector<string>& elements, const char* const separator) {
	switch (elements.size())
	{
	case 0:
		return "";
	case 1:
		return elements[0];
	default:
		ostringstream os;
		copy(elements.begin(), elements.end()-1, ostream_iterator<string>(os, separator));
		os << *elements.rbegin();
		return os.str();
	}
}


/**
 * @brief read in a double value from an input stream, taking nan and inf into account.
 *
 */
float Strings::to_float(std::istream &is) {
	string input;

	is >> input;
	
	double tmp = strtod (input.c_str(),0);
	return (float) tmp;
}


/**
 * @brief Check if given value if present in given string array.
 *
 * Array should be terminated with a null value
 */
bool Strings::includes(const char *arr[], const std::string &val) {
	bool ret = false;

	const char **it = &arr[0];
	while (*it != 0) {
		if (val == string(*it)) {
			ret = true;
			break;
		}
		it++;
	}

	return ret;
}


bool Strings::includes(const char *arr[], const char *val) {
	if (val == 0) {
		return false;
	}

	std::string tmp(val);
	return includes(arr, tmp);
}

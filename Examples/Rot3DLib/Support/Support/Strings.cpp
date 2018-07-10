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
 * Split a string into an array on the separation character.
 *
 * Source: http://stackoverflow.com/questions/890164/how-can-i-split-a-string-by-a-delimiter-into-an-array
 */
std::vector<std::string> Strings::explode(const std::string& str, char sep) {
	string next;
	vector<string> result;

	// For each character in the string
	for (unsigned i = 0 ; i < str.length() ; ++ i) {
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
	if (!next.empty()) result.push_back(next);

	// Remove whitespace on front and back
	for (unsigned i = 0 ; i < result.size() ; ++ i)	{
		string &it  = result[i] ;
		it = trim(it);
	}

	return result;
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

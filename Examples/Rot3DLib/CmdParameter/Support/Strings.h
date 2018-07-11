/*
 * CmdParameter Lite v0.1.0
 * Copyright (c) 2018 Wim Rijnders
 *
 * Distributed under the MIT License,
 * see https://github.com/wimrijnders/CmdParameter/blob/master/LICENSE
 * --------------------------------------------------------------------
 * Generated on: 2018-07-11 12:16:21 +0200
 */
#ifndef STRINGS_H_
#define STRINGS_H_


/**
 * @brief Useful string handling methods.
 *
 */
class Strings {
public:

	static bool starts_with(std::string const& haystack, std::string const& needle);
	static std::string implode( const std::vector<std::string>& elements, const char* const separator = ", ");
	static std::vector<std::string> explode(const std::string& str, char sep = ',');
	static std::string trim(const std::string& s, const std::string& delimiters = " \f\n\r\t\v");
	static bool contains( std::string const& haystack, std::string const& needle);

};

#endif  // STRINGS_H_

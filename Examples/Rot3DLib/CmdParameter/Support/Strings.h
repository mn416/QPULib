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

};

#endif  // STRINGS_H_

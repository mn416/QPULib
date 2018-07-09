#ifndef COUT_REDIRECT_H
#define COUT_REDIRECT_H
#include <string>
#include <iostream>
#include <sstream>


/**
 * @brief RAII for stdout interception
 *
 * Redirects everything sent to cout to a streambuffer,
 * which can be read inline.
 *
 * Goals:
 *
 *  - Reduce console output during unit testing
 *  - Be able to check if console output is/contains what we expect
 *
 * --------------------------------------------------------
 * ## NOTES
 *
 * * Derived from : https://stackoverflow.com/a/6211644/1223531
 *
 * * **All* output to cout is intercepted, as long as an instance
 *   is active. This means that consecutive tests in a SECTION-block
 *   will have merged console output.
 *
 *   To clear out the captured text till now:
 *
 * ```
 *   cout_redirect redirect;
 *   redirect.clear();
 * ```
 *
 * * Following won't work:
 *
 * ```
 *   cout_redirect redirect;
 *   cout << redirect.str();
 * ```
 *
 *   Since `redirect` is intercepting cout, the output goes
 *   into the buffer of `redirect`.
 *
 *   To show output, the cout redirection must be stopped first:
 *
 * ```
 *   redirect.close();
 *   cout << redirect.str();
 * ```
 *
 *   After this, cout is not intercepted any more. if this is
 *   required, you need to create a new instance (in the current
 *   implementation).
 *
 * --------------------------------------------------------
 * ## TODO
 *
 * * Ensure that only one single instance can be active at any time.
 */
class cout_redirect {
public:
	cout_redirect();
	~cout_redirect() { close();	}

	std::string str() { return strCout.str(); }
	void close();
	void clear();

private:
	std::streambuf* oldCoutStreamBuf{nullptr};
	std::ostringstream strCout;
};

#endif // COUT_REDIRECT_H

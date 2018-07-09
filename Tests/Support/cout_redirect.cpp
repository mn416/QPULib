#include "cout_redirect.h"

cout_redirect::cout_redirect() {
	// Redirect cout.
	oldCoutStreamBuf = std::cout.rdbuf();
	std::cout.rdbuf(strCout.rdbuf());
}


void cout_redirect::close() {
	if (oldCoutStreamBuf == nullptr) return;  // Already closed

	// Restore old cout.
	std::cout.rdbuf(oldCoutStreamBuf);
	oldCoutStreamBuf = nullptr;
	//std::cout << "redirect: " << str() << std::endl;
}


/**
 * @brief Empty the current buffer
 */
void cout_redirect::clear() {
  strCout.str("");
  strCout.clear();
}

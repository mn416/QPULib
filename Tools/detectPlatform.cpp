#include <unistd.h>  // geteuid()
#include <string>
#include <fstream>
#include <streambuf>
#include "QPULib.h"
#include "VideoCore/RegisterMap.h"

using namespace QPULib;


/**
 * @brief read the entire contents of a file into a string
 *
 * @param filename name of file to read
 * @param out_str  output parameter; place to store the file contents
 *
 * @return true if all went well, false if file could not be read.
 */
bool loadFileInString(const char *filename, std::string & out_str) {
	std::ifstream t(filename);
	if (!t.is_open()) {
		return false;
	}

	std::string str((std::istreambuf_iterator<char>(t)),
  	               std::istreambuf_iterator<char>());

	if (str.empty()) {
		return false;
	}

	out_str = str;
	return true;
}


/**
 * @brief Detect if this is running on a Rpi.
 *
 * @returns 0 if this is so, 1 if it's a different platform.
 */
int main(int argc, char *argv[]) {
	const char *filename = "/sys/firmware/devicetree/base/model";

	std::string content;
	bool success = loadFileInString(filename, content);
	if (success && !content.empty()) {
		printf("Detected platform: %s\n", content.c_str());
	} else {
		printf("This is not a RPi platform\n");
		return 1;
	}

#ifdef QPU_MODE
	// Show hardware revision code
	int mb = getMailbox();	
	unsigned revision = get_version(mb);
	printf("Hardware revision: %04x\n", revision);

	bool wasEnabled = RegisterMap::enabled();
	if (!wasEnabled) {
		// VideoCore needs to be enabled, otherwise the registers can't be accessed.
		//printf("VideoCore not running, enabling for this app\n");
		qpu_enable(mb, 1);
	}

	if (geteuid() == 0) {  // Only do this as root (sudo)
		printf("Number of slices: %d\n", RegisterMap::numSlices());
		printf("Number of QPU's per slice: %d\n", RegisterMap::numQPUPerSlice());
	} else {
		printf("You can see more if you use sudo\n");
  }

	if (!wasEnabled) {
		//printf("Disabling the VideoCore again\n");
		qpu_enable(mb, 0);
	}
#endif  // QPU_MODE

	return 0;
}

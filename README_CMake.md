# Bulding with CMake
## Introduction

This document explains how to make a project with QPULib and CMake.
An example CMake project has been set up 
[here](https://github.com/m516/PiGPGPU)
as reference and to be used as a template for new projects.

The general file structure for a C++ project with QPULib is like so:
```
lib/
    QPULib/
       QPULib source files
    <Other libraries here>
src/
    <Put your project source files here>
    CMakeLists.txt
CMakeLists.txt
```

If prepared properly, your project can be easily built on Windows,
Mac OS, and Linux platforms, even though QPULib targets Raspberry Pi
hardware. The example project has been tested with Windows 10,
Raspbian Buster on a Pi 0W, and Ubuntu with minimal configuration on
all platforms.

## Writing CMake Scripts
CMake scripts are a pain in the butt to set up, but they make the build
process easy. CMake reads text files with the name `CMakeLists.txt`
and generates Makefiles. The Makefiles are interpreted by Make to
build your project with the C++ compiler on your OS.

The `CMakeLists.txt` file in the root directory should be used to 
set up the project, store the list of libraries, and start the build
processes for all your subprojects.
```CMake
# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project (PiGPGPU)

# Include libraries
add_subdirectory("lib")


#Compile differently for the Raspberry Pi
find_path(HAS_BCMHOST bcm_host.h PATHS /opt/vc/include/)
if (HAS_BCMHOST) #This is a Pi
	message(STATUS "This is a Pi")
	message(STATUS "Targeting VideoCore IV GPU")
	#Set precompiler flags
	add_definitions(-DQPU_MODE)

else() #Not compiling on the Pi
	message(STATUS "This is not a Pi")
	message(STATUS "Emulating Pi's GPU")
	#Add precompiler definitions
	add_definitions(-DEMULATION_MODE)
endif()

#Include QPULib
add_subdirectory("lib/QPULib")
include_directories("${PROJECT_SOURCE_DIR}/lib/QPULib/Lib")

add_subdirectory("src")
```

## Conclusion
If you have any suggestions or issues, I'd love to hear them and help!
The best way to contact me is the issues tab of the Github repository.
Good luck and happy hacking!

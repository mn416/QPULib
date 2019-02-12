cmake_minimum_required(VERSION 3.1)
project(QPULib CXX)
set(CMAKE_CXX_STANDARD 14)

option(BUILD_EXAMPLES "Build all examples in the example directory" OFF)
option(ENABLE_DEBUG "Output debug information from QPULib" OFF)
option(ENABLE_QPU "Enable QPU (rpi only), otherwise emulate" OFF)

if(${ENABLE_DEBUG})
	set(QPULIB_DEBUG 1)
endif()

if(${ENABLE_QPU})
	set(QPULIB_QPU_MODE 1)
else()
	set(QPULIB_EMULATION_MODE 1)
endif()

STRING(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Lib/qpulib_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/qpulib_config.h)
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/Lib
	${CMAKE_CURRENT_BINARY_DIR}
)

set(
	SOURCES

	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Kernel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Syntax.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Int.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Float.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Stmt.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Pretty.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Translate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Interpreter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Source/Gen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Syntax.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/SmallLiteral.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Pretty.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/RemoveLabels.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/CFG.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Liveness.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/RegAlloc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/ReachingDefs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Subst.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/LiveRangeSplit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Satisfy.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/LoadStore.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Emulator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/Target/Encode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/VideoCore/Mailbox.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/VideoCore/Invoke.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/VideoCore/VideoCore.cpp
)

add_library(qpu.a STATIC ${SOURCES})
add_library(qpu.so SHARED ${SOURCES})

set_target_properties(qpu.a PROPERTIES OUTPUT_NAME qpu)
set_target_properties(qpu.so PROPERTIES OUTPUT_NAME qpu)

install(TARGETS
	qpu.a qpu.so
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/Lib/
	DESTINATION include/QPULib
	FILES_MATCHING PATTERN "*.h"
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/qpulib_config.h
	DESTINATION include
)

if(${BUILD_EXAMPLES})
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Examples)
endif()



#QPULib uses assert statements that throw errors when compiling with g++ on platforms other than the Pi
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_definitions(-fpermissive)
endif()



#file(GLOB_RECURSE QPULIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
#message(STATUS "QPULib library source files: ${QPULIB_SOURCES}")
#file(GLOB_RECURSE QPULIB_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.hpp")



add_library(QPULib "")

#Compile differently for the Raspberry Pi
find_path(HAS_BCMHOST bcm_host.h PATHS /opt/vc/include/)
if (HAS_BCMHOST) #This is a Pi
	message(STATUS "This is a Pi")
	#Set precompiler flags
	add_definitions(-DEMULATION_MODE)
	#Compile stuff
	target_sources(QPULib
		PRIVATE
		Lib/Kernel.cpp
		Lib/Source/Float.cpp
		Lib/Source/Gen.cpp
		Lib/Source/Int.cpp
		Lib/Source/Interpreter.cpp
		Lib/Source/Pretty.cpp
		Lib/Source/Stmt.cpp
		Lib/Source/Syntax.cpp
		Lib/Source/Translate.cpp
		Lib/Target/CFG.cpp
		Lib/Target/Emulator.cpp
		Lib/Target/Encode.cpp
		Lib/Target/LiveRangeSplit.cpp
		Lib/Target/Liveness.cpp
		Lib/Target/LoadStore.cpp
		Lib/Target/Pretty.cpp
		Lib/Target/ReachingDefs.cpp
		Lib/Target/RegAlloc.cpp
		Lib/Target/RemoveLabels.cpp
		Lib/Target/Satisfy.cpp
		Lib/Target/SmallLiteral.cpp
		Lib/Target/Subst.cpp
		Lib/Target/Syntax.cpp
		Lib/VideoCore/Invoke.cpp
		Lib/VideoCore/Mailbox.cpp
		Lib/VideoCore/VideoCore.cpp
		)

else() #Not compiling on the Pi
	message(STATUS "This is not a Pi")
	#Add precompiler definitions
	add_definitions(-DEMULATION_MODE)
		#Compile stuff
	target_sources(QPULib
		PRIVATE
		Lib/Kernel.cpp
		Lib/Source/Float.cpp
		Lib/Source/Gen.cpp
		Lib/Source/Int.cpp
		Lib/Source/Interpreter.cpp
		Lib/Source/Pretty.cpp
		Lib/Source/Stmt.cpp
		Lib/Source/Syntax.cpp
		Lib/Source/Translate.cpp
		Lib/Target/CFG.cpp
		Lib/Target/Emulator.cpp
		Lib/Target/Encode.cpp
		Lib/Target/LiveRangeSplit.cpp
		Lib/Target/Liveness.cpp
		Lib/Target/LoadStore.cpp
		Lib/Target/Pretty.cpp
		Lib/Target/ReachingDefs.cpp
		Lib/Target/RegAlloc.cpp
		Lib/Target/RemoveLabels.cpp
		Lib/Target/Satisfy.cpp
		Lib/Target/SmallLiteral.cpp
		Lib/Target/Subst.cpp
		Lib/Target/Syntax.cpp
		)

endif()



target_include_directories(QPULib
  PUBLIC
	Lib/
  )
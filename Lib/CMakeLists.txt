message(STATUS "Creating Makefile for QPULib")

#QPULib uses assert statements that throw errors when compiling with g++ on platforms other than the Pi
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_definitions(-fpermissive)
endif()

#file(GLOB_RECURSE QPULIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
#message(STATUS "QPULib library source files: ${QPULIB_SOURCES}")
#file(GLOB_RECURSE QPULIB_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.hpp")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")


# Compile Invoke, Mailbox, VideoCore if not emulating
# Check if this is a Pi
# Compile differently for the Raspberry Pi
find_path(HAS_BCMHOST bcm_host.h PATHS /opt/vc/include/)
if (HAS_BCMHOST) #This is a Pi
	add_definitions(-DQPU_MODE)
	SET(QPULIB_SOURCES
		${QPULIB_SOURCES}
		VideoCore/Invoke.cpp
		VideoCore/Mailbox.cpp
		VideoCore/VideoCore.cpp
		VideoCore/Invoke.h
		VideoCore/Mailbox.h
		VideoCore/VideoCore.h
		)

else() #Not compiling on the Pi
	add_definitions(-DEMULATION_MODE)

endif()

SET(QPULIB_SOURCES
	${QPULIB_SOURCES}
	#Source files
	Kernel.cpp
	Source/Float.cpp
	Source/Gen.cpp
	Source/Int.cpp
	Source/Interpreter.cpp
	Source/Pretty.cpp
	Source/Stmt.cpp
	Source/Syntax.cpp
	Source/Translate.cpp
	Target/CFG.cpp
	Target/Emulator.cpp
	Target/Encode.cpp
	Target/Liveness.cpp
	Target/LiveRangeSplit.cpp
	Target/LoadStore.cpp
	Target/Pretty.cpp
	Target/ReachingDefs.cpp
	Target/RegAlloc.cpp
	Target/RemoveLabels.cpp
	Target/Satisfy.cpp
	Target/SmallLiteral.cpp
	Target/Subst.cpp
	Target/Syntax.cpp
	#Header files
	Common/Heap.h
	Common/Seq.h
	Common/Stack.h
	Kernel.h
	Params.h
	QPULib.h
	Source/Cond.h
	Source/Float.h
	Source/Gen.h
	Source/Int.h
	Source/Interpreter.h
	Source/Pretty.h
	Source/Ptr.h
	Source/Stmt.h
	Source/StmtExtra.h
	Source/Syntax.h
	Source/Translate.h
	Target/CFG.h
	Target/Emulator.h
	Target/Encode.h
	Target/Liveness.h
	Target/LiveRangeSplit.h
	Target/LoadStore.h
	Target/Pretty.h
	Target/ReachingDefs.h
	Target/RegAlloc.h
	Target/RemoveLabels.h
	Target/Satisfy.h
	Target/SmallLiteral.h
	Target/Subst.h
	Target/Syntax.h
	VideoCore/SharedArray.h
)


add_library(QPULib 
STATIC
${QPULIB_SOURCES}
)

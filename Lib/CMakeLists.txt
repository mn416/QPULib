

#QPULib uses assert statements that throw errors when compiling with g++ on platforms other than the Pi
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_definitions(-fpermissive)
endif()



#file(GLOB_RECURSE QPULIB_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
#message(STATUS "QPULib library source files: ${QPULIB_SOURCES}")
#file(GLOB_RECURSE QPULIB_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.hpp")



add_library(QPULib "")

#Compile differently for the Raspberry Pi
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h BCMHOST)
if (BCMHOST) #This is a Pi
    message(STATUS "This is a Pi")
    #Set precompiler flags
    add_definitions(-DEMULATION_MODE)
    #Compile stuff
    target_sources(QPULib
        PRIVATE
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
        Target/LiveRangeSplit.cpp
        Target/Liveness.cpp
        Target/LoadStore.cpp
        Target/Pretty.cpp
        Target/ReachingDefs.cpp
        Target/RegAlloc.cpp
        Target/RemoveLabels.cpp
        Target/Satisfy.cpp
        Target/SmallLiteral.cpp
        Target/Subst.cpp
        Target/Syntax.cpp
        VideoCore/Invoke.cpp
        VideoCore/Mailbox.cpp
        VideoCore/VideoCore.cpp
        )
else() #Not compiling on the Pi
    message(STATUS "This is not a Pi")
    #Add precompiler definitions
    add_definitions(-DEMULATION_MODE)
        #Compile stuff
    target_sources(QPULib
        PRIVATE
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
        Target/LiveRangeSplit.cpp
        Target/Liveness.cpp
        Target/LoadStore.cpp
        Target/Pretty.cpp
        Target/ReachingDefs.cpp
        Target/RegAlloc.cpp
        Target/RemoveLabels.cpp
        Target/Satisfy.cpp
        Target/SmallLiteral.cpp
        Target/Subst.cpp
        Target/Syntax.cpp
        )
endif()

target_include_directories(QPULib
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Source
    ${CMAKE_CURRENT_LIST_DIR}/Common
    ${CMAKE_CURRENT_LIST_DIR}/Target
    ${CMAKE_CURRENT_LIST_DIR}/VideoCore
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
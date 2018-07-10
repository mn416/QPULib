#
# There are four builds possible, with output directories:
#
#   obj              - using emulator
#   obj-debug        - output debug info, using emulator
#   obj-qpu          - using hardware
#   obj-debug-qpu    - output debug info, using hardware
#
# To compile for debugging, add flag '-g' to CXX_FLAGS.
#
###########################################################

# Root directory of QPULib repository
ROOT = Lib

# Compiler and default flags
CXX = g++

CXX_FLAGS = -Wconversion -std=c++0x -I $(ROOT) -MMD -MP -MF"$(@:%.o=%.d)" -g  # Add debug info: -g

# Object directory
OBJ_DIR = obj

# Debug mode
ifeq ($(DEBUG), 1)
  CXX_FLAGS += -DDEBUG
  OBJ_DIR := $(OBJ_DIR)-debug
endif

# QPU or emulation mode
ifeq ($(QPU), 1)

# Check platform before building. Can't be indented, otherwise make complains.
RET := $(shell Tools/detectPlatform.sh 1>/dev/null && echo "yes" || echo "no")
#$(info  info: '$(RET)')
ifneq ($(RET), yes)
$(error QPU-mode specified on a non-Pi platform; aborting)
else
$(info Building on a Pi platform)
endif

  # -I is for access to bcm functionality
  CXX_FLAGS += -DQPU_MODE -I /opt/vc/include
  OBJ_DIR := $(OBJ_DIR)-qpu
	LIBS := -L /opt/vc/lib -l bcm_host
else
  CXX_FLAGS += -DEMULATION_MODE
endif


# Library Object files
OBJ =                         \
  Kernel.o                    \
  Source/Syntax.o             \
  Source/Int.o                \
  Source/Float.o              \
  Source/Stmt.o               \
  Source/Pretty.o             \
  Source/Translate.o          \
  Source/Interpreter.o        \
  Source/Gen.o                \
  Target/Syntax.o             \
  Target/SmallLiteral.o       \
  Target/Pretty.o             \
  Target/RemoveLabels.o       \
  Target/CFG.o                \
  Target/Liveness.o           \
  Target/RegAlloc.o           \
  Target/ReachingDefs.o       \
  Target/Subst.o              \
  Target/LiveRangeSplit.o     \
  Target/Satisfy.o            \
  Target/LoadStore.o          \
  Target/Emulator.o           \
  Target/Encode.o             \
  VideoCore/RegisterMap.o     \
  VideoCore/Mailbox.o         \
  VideoCore/Invoke.o          \
  VideoCore/VideoCore.o

LIB = $(patsubst %,$(OBJ_DIR)/%,$(OBJ))


# All programs in the Examples directory
# NOTE: detectPlatform is in the 'Tools' directory, not in 'Examples'
EXAMPLES =  \
	detectPlatform \
	Tri       \
	GCD       \
	Print     \
	MultiTri  \
	AutoTest  \
	OET       \
	Hello     \
	ReqRecv   \
	Rot3D     \
	Rot3DLib  \
	ID        \
	HeatMap

EXAMPLE_TARGETS = $(patsubst %,$(OBJ_DIR)/bin/%,$(EXAMPLES))


# Rot3DLib object files
ROT3DLIB_OBJ := \
	Rot3DLib/Rot3DKernels.o    \
	Rot3DLib/Parameters.o      \
	Rot3DLib/CmdParameter/Support/Strings.o \
	Rot3DLib/CmdParameter/Types/IntParameter.o \
	Rot3DLib/CmdParameter/Types/NoneParameter.o \
	Rot3DLib/CmdParameter/Types/Types.o \
	Rot3DLib/CmdParameter/DefParameter.o \
	Rot3DLib/CmdParameter/CmdParameter.o


ROT3DLIB_OBJ := $(patsubst %,$(OBJ_DIR)/Examples/%,$(ROT3DLIB_OBJ))
#$(info $(ROT3DLIB_OBJ))

# Dependencies from list of object files
DEPS := $(LIB:.o=.d)
#$(info $(DEPS))
-include $(DEPS)

# Dependencies for the include files in the Examples directory.
# Basically, every .h file under examples has a .d in the build directory
ROT3DLIB_DEPS = $(ROT3DLIB_OBJ:.o=.d)
#$(info $(ROT3DLIB_DEPS))
-include $(ROT3DLIB_DEPS)


# Top-level targets

.PHONY: help clean all lib test $(EXAMPLES)

# Following prevents deletion of object files after linking
# Otherwise, deletion happens for targets of the form '%.o'
.PRECIOUS: $(OBJ_DIR)/%.o  \
	$(OBJ_DIR)/Source/%.o    \
	$(OBJ_DIR)/Target/%.o    \
	$(OBJ_DIR)/VideoCore/%.o \
	$(OBJ_DIR)/Examples/%.o


help:
	@echo 'Usage:'
	@echo
	@echo '    make [QPU=1] [DEBUG=1] [target]*'
	@echo
	@echo 'Where target:'
	@echo
	@echo '    help          - Show this text'
	@echo '    all           - Build all test programs'
	@echo '    clean         - Delete all interim and target files'
	@echo '    test          - Run the unit tests'
	@echo
	@echo '    one of the test programs - $(EXAMPLES)'
	@echo
	@echo 'Flags:'
	@echo
	@echo '    QPU=1         - Output code for hardware. If not specified, the code is compiled for the emulator'
	@echo '    DEBUG=1       - If specified, the source code and target code is shown on stdout when running a test'
	@echo

all: $(EXAMPLE_TARGETS) $(OBJ_DIR)

clean:
	rm -rf obj obj-debug obj-qpu obj-debug-qpu


#
# Targets for QPULib static library
#

QPU_LIB=$(OBJ_DIR)/libQPULib.a
#$(info LIB: $(LIB))

$(QPU_LIB): $(LIB)
	@echo Creating $@
	@ar rcs $@ $^

$(OBJ_DIR)/%.o: $(ROOT)/%.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)


#
# Targets for Examples and Tools
#

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(CXX) $(CXX_FLAGS) $^ $(LIBS) -o $@

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Tools/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(CXX) $(CXX_FLAGS) $^ $(LIBS) -o $@

# General compilation of cpp files
# Keep in mind that the % will take into account subdirectories under OBJ_DIR.
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c $(CXX_FLAGS) -o $@ $<

$(EXAMPLES) :% :$(OBJ_DIR)/bin/%


#
# Targets for Rot3DLib
#

ROT3DLIB_LIB=$(OBJ_DIR)/libRot3DLib.a

# Create static library
$(ROT3DLIB_LIB): $(ROT3DLIB_OBJ)
	@echo Creating $@
	@ar rcs $@ $^

$(OBJ_DIR)/bin/Rot3DLib: $(ROT3DLIB_LIB)

# Create executable
$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/Rot3DLib/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(CXX) $(CXX_FLAGS) $^ -o $@


#
# Targets for Unit Tests
#

RUN_TESTS := $(OBJ_DIR)/bin/runTests

# sudo required for QPU-mode on Pi
ifeq ($(QPU), 1)
	RUN_TESTS := sudo $(RUN_TESTS)
endif


# Source files for unit tests to include in compilation
UNIT_TESTS = \
	Tests/testMain.cpp  \
	Tests/testRot3D.cpp

#
# For some reason, doing an interim step to .o results in linkage errors (undefined references).
# So this target compiles the source files directly to the executable.
#
# Flag `-Wno-psabi` is to surpress a superfluous warning when compiling with GCC 6.3.0
#
$(OBJ_DIR)/bin/runTests: $(UNIT_TESTS) $(ROT3DLIB_LIB) | $(QPU_LIB)
	@echo Compiling unit tests
	@$(CXX) $(CXX_FLAGS) -Wno-psabi $^ -L$(OBJ_DIR) -lRot3DLib -lQPULib $(LIBS) -o $@

make_test: $(OBJ_DIR)/bin/runTests
	@echo building unit tests

test : $(OBJ_DIR)/bin/runTests | make_test AutoTest
	@echo Running unit tests with '$(RUN_TESTS)'
	@$(RUN_TESTS)


#
# Other targets
#

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)/Source
	@mkdir -p $(OBJ_DIR)/Target
	@mkdir -p $(OBJ_DIR)/VideoCore
	@mkdir -p $(OBJ_DIR)/Examples/Rot3DLib/CmdParameter/Support  # Creates all directories in path as well
	@mkdir -p $(OBJ_DIR)/Examples/Rot3DLib/CmdParameter/Types
	@mkdir -p $(OBJ_DIR)/Tools
	@mkdir -p $(OBJ_DIR)/bin

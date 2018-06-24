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
  CXX_FLAGS += -DQPU_MODE
  OBJ_DIR := $(OBJ_DIR)-qpu
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
  VideoCore/Mailbox.o         \
  VideoCore/Invoke.o          \
  VideoCore/VideoCore.o


# All programs in the Examples directory
EXAMPLES =  \
	Tri       \
	GCD       \
	Print     \
	MultiTri  \
	AutoTest  \
	OET       \
	Hello     \
	ReqRecv   \
	Rot3D     \
	ID        \
	HeatMap

EXAMPLE_TARGETS = $(patsubst %,$(OBJ_DIR)/bin/%,$(EXAMPLES))
LIB = $(patsubst %,$(OBJ_DIR)/%,$(OBJ))

# List of dependencies defined from list of object files
# Note that the example programs in Examples are not included here
DEPS := $(LIB:.o=.d)
#$(info $(DEPS))

-include $(DEPS)


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
# Targets for static library
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
# Targets for Examples
#

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Examples/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(CXX) $(CXX_FLAGS) $^ -o $@

$(OBJ_DIR)/Examples/%.o: Examples/%.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c $(CXX_FLAGS) -o $@ $<

$(EXAMPLES) :% :$(OBJ_DIR)/bin/%


#
# Targets for Unit Tests
#

#RUN_TESTS := @$(OBJ_DIR)/bin/runTests
RUN_TESTS := @sudo $(OBJ_DIR)/bin/runTests   # Running unit tests on the Pi hardware requires sudo, use this instead

# Source files with unit tests to include in compilation
UNIT_TESTS =          \
	Tests/testMain.cpp  \
	Tests/testRot3D.cpp \
	Tests/AutoTest.cpp 

# For some reason, doing an interim step to .o results in linkage errors (undefined references).
# So this target compiles the source files directly to the executable.
$(OBJ_DIR)/bin/runTests: $(UNIT_TESTS) | $(QPU_LIB)
	@$(CXX) $(CXX_FLAGS) $^ -L$(OBJ_DIR) -lQPULib -o $@

test : $(OBJ_DIR)/bin/runTests
	@echo Running unit tests
	@$(RUN_TESTS)

#
# Other targets
#

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)/Source
	@mkdir -p $(OBJ_DIR)/Target
	@mkdir -p $(OBJ_DIR)/VideoCore
	@mkdir -p $(OBJ_DIR)/Examples
	@mkdir -p $(OBJ_DIR)/bin

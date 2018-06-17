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
CXX_FLAGS = -Wconversion -std=c++0x -I $(ROOT)

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

# Object files
OBJ =                         \
  Kernel.o                    \
  Debug.o                     \
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


# All programs in the Tests directory
TESTS =     \
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

TEST_TARGETS = $(patsubst %,$(OBJ_DIR)/bin/%,$(TESTS))

# Top-level targets

.PHONY: help clean all lib $(TESTS)

# Following prevents deletion of object files after linking
# Otherwise, deletion happens for targets of the form '%.o'
.PRECIOUS: $(OBJ_DIR)/%.o  \
	$(OBJ_DIR)/Source/%.o    \
	$(OBJ_DIR)/Target/%.o    \
	$(OBJ_DIR)/VideoCore/%.o \
	$(OBJ_DIR)/Tests/%.o


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
	@echo
	@echo '    one of the test programs - $(TESTS)'
	@echo
	@echo 'Flags:'
	@echo
	@echo '    QPU=1         - Output code for hardware. If not specified, the code is compiled for the emulator'
	@echo '    DEBUG=1       - If specified, the source code and target code is shown on stdout when running a test'
	@echo

all: $(TEST_TARGETS) $(OBJ_DIR)

clean:
	rm -rf obj obj-debug obj-qpu obj-debug-qpu

LIB = $(patsubst %,$(OBJ_DIR)/%,$(OBJ))


#
# Targets for static library
#

QPU_LIB=$(OBJ_DIR)/libQPULib.a
#$(info LIB: $(LIB))

$(QPU_LIB): $(LIB)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(ROOT)/%.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)


#
# Targets for Tests
#

$(OBJ_DIR)/bin/%: $(OBJ_DIR)/Tests/%.o $(QPU_LIB)
	@echo Linking $@...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

$(OBJ_DIR)/Tests/%.o: Tests/%.cpp | $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)

$(TESTS) :% :$(OBJ_DIR)/bin/%


#
# Other targets
#

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)/Source
	@mkdir -p $(OBJ_DIR)/Target
	@mkdir -p $(OBJ_DIR)/VideoCore
	@mkdir -p $(OBJ_DIR)/Tests
	@mkdir -p $(OBJ_DIR)/bin

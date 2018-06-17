# Root directory of QPULib repository
ROOT = ../Lib

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

# Top-level targets

.PHONY: top clean

top:
	@echo Please supply a target to build, e.g. \'make GCD\'
	@echo

clean:
	rm -rf obj obj-debug obj-qpu obj-debug-qpu
	rm -f Tri GCD Print MultiTri AutoTest OET Hello ReqRecv Rot3D ID *.o
	rm -f HeatMap

LIB = $(patsubst %,$(OBJ_DIR)/%,$(OBJ))

Hello: Hello.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

ID: ID.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

Tri: Tri.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

Print: Print.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

GCD: GCD.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

AutoTest: AutoTest.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

MultiTri: MultiTri.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

OET: OET.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

ReqRecv: ReqRecv.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

Rot3D: Rot3D.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

HeatMap: HeatMap.o $(LIB)
	@echo Linking...
	@$(CXX) $^ -o $@ $(CXX_FLAGS)

# Intermediate targets

$(OBJ_DIR)/%.o: $(ROOT)/%.cpp $(OBJ_DIR)
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)

%.o: %.cpp
	@echo Compiling $<
	@$(CXX) -c -o $@ $< $(CXX_FLAGS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/Source
	@mkdir -p $(OBJ_DIR)/Target
	@mkdir -p $(OBJ_DIR)/VideoCore

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <stdint.h>
#include "Common/Seq.h"
#include "Target/Syntax.h"

#define VPM_SIZE 2048
#define NUM_LANES 16
#define MAX_QPUS 12
#define EMULATOR_HEAP_SIZE 65536

// This is a type for representing the values in a vector
union Word {
  int32_t intVal;
  float floatVal; 
};

// Vector values
struct Vec {
  Word elems[NUM_LANES];
};

// In-flight DMA requests
struct DMAReq {
  bool active;
  Word addr;
  BufferAorB buffer;
};

// VPM load queue (max 2 elements)
struct VPMLoadQueue {
  int addrs[3];
  int front, back;
};

// State of a single QPU.
struct QPUState {
  int id;                    // QPU id
  int numQPUs;               // QPU count
  bool running;              // Is QPU active, or has it halted?
  int pc;                    // Program counter
  Vec* regFileA;             // Register file A
  int sizeRegFileA;          // (and size)
  Vec* regFileB;             // Register file B
  int sizeRegFileB;          // (and size)
  Vec accum[6];              // Accumulator registers
  bool negFlags[NUM_LANES];  // Negative flags
  bool zeroFlags[NUM_LANES]; // Zero flags
  int nextUniform;           // Pointer to next uniform to read
  DMAReq dmaLoad;            // In-flight DMA load
  DMAReq dmaStore;           // In-flight DMA store
  VPMLoadQueue vpmLoadQueue; // VPM load queue
  int readStride;            // Read stride
  int writeStride;           // Write stride
  SmallSeq<Vec>* loadBuffer; // Load buffer for loads via TMU
};

// State of the VideoCore.
struct State {
  QPUState qpu[MAX_QPUS]; // State of each QPU
  Word vpm[VPM_SIZE];     // Shared VPM memory
  Seq<char>* output;      // Output for print statements
  int sema[16];           // Semaphores
};

// Emulator
void emulate
  ( int numQPUs            // Number of QPUs active
  , Seq<Instr>* instrs     // Instruction sequence
  , int maxReg             // Max reg id used
  , Seq<int32_t>* uniforms // Kernel parameters
  , Seq<char>* output      // Output from print statements
                           // (if NULL, stdout is used)
  );

// Heap used in emulation mode.
extern uint32_t emuHeapEnd;
extern int32_t* emuHeap;

// Rotate a vector
Vec rotate(Vec v, int n);

// Printing routines
void emitChar(Seq<char>* out, char c);
void emitStr(Seq<char>* out, const char* s);
void printIntVec(Seq<char>* out, Vec x);
void printFloatVec(Seq<char>* out, Vec x);

#endif

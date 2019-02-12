#ifndef _QPULIB_EMULATOR_H_
#define _QPULIB_EMULATOR_H_

#include <stdint.h>
#include "../Common/Seq.h"
#include "../Common/Queue.h"
#include "../Target/Syntax.h"

#define NUM_LANES 16
#define MAX_QPUS 12
#define EMULATOR_HEAP_SIZE 1024*65536
#define VPM_SIZE 1024

namespace QPULib {

// This is a type for representing the values in a vector
union Word {
  int32_t intVal;
  float floatVal; 
};

// Vector values
struct Vec {
  Word elems[NUM_LANES];
};

// In-flight DMA request
struct DMAAddr {
  bool active;
  Word addr;
};

// VPM load request
struct VPMLoadReq {
  int numVecs;  // Number of vectors to load
  bool hor;     // Horizintal or vertical access?
  int addr;     // Address in VPM to load from
  int stride;   // Added to address after every vector read
};

// VPM store request
struct VPMStoreReq {
  bool hor;     // Horizintal or vertical access?
  int addr;     // Address in VPM to load from
  int stride;   // Added to address after every vector written
};

// DMA load request
struct DMALoadReq {
  bool hor;     // Horizintal or vertical access?
  int numRows;  // Number of rows in memory
  int rowLen;   // Length of each row in memory
  int vpmAddr;  // VPM address to write to
  int vpitch;   // Added to vpmAddr after each vector loaded
};

// DMA store request
struct DMAStoreReq {
  bool hor;     // Horizintal or vertical access?
  int numRows;  // Number of rows in memory
  int rowLen;   // Length of each row in memory
  int vpmAddr;  // VPM address to load from
};

// State of a single QPU.
struct QPUState {
  int id;                              // QPU id
  int numQPUs;                         // QPU count
  bool running;                        // Is QPU active, or has it halted?
  int pc;                              // Program counter
  Vec* regFileA;                       // Register file A
  int sizeRegFileA;                    // (and size)
  Vec* regFileB;                       // Register file B
  int sizeRegFileB;                    // (and size)
  Vec accum[6];                        // Accumulator registers
  bool negFlags[NUM_LANES];            // Negative flags
  bool zeroFlags[NUM_LANES];           // Zero flags
  int nextUniform;                     // Pointer to next uniform to read
  DMAAddr dmaLoad;                     // DMA load address
  DMAAddr dmaStore;                    // DMA store address
  DMALoadReq dmaLoadSetup;             // DMA load setup register
  DMAStoreReq dmaStoreSetup;           // DMA store setup register
  Queue<2, VPMLoadReq> vpmLoadQueue;   // VPM load queue
  VPMStoreReq vpmStoreSetup;           // VPM store setup
  int readPitch;                       // Read pitch
  int writeStride;                     // Write stride
  SmallSeq<Vec>* loadBuffer;           // Load buffer for loads via TMU
};

// State of the VideoCore.
struct State {
  QPUState qpu[MAX_QPUS];  // State of each QPU
  Seq<int32_t>* uniforms;  // Kernel parameters
  Word vpm[VPM_SIZE];      // Shared VPM memory
  Seq<char>* output;       // Output for print statements
  int sema[16];            // Semaphores
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

}  // namespace QPULib

#endif  // _QPULIB_EMULATOR_H_

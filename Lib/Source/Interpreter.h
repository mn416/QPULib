#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stdint.h>
#include "Common/Seq.h"
#include "Source/Syntax.h"

// The interpreter works in a similar way to the emulator.  The
// difference is that the former operates on source code and the
// latter on target code.  We reuse a number of concepts of the
// emulator in the interpreter.

#include "Target/Emulator.h"

// State of a single core.
struct CoreState {
  int id;                    // Core id
  int numCores;              // Core count
  Seq<int32_t>* uniforms;    // Arguments to kernel
  int nextUniform;           // Pointer to next uniform to read
  int readStride;            // Read stride
  int writeStride;           // Write stride
  Vec* env;                  // Environment mapping vars to values
  int sizeEnv;               // Size of the environment
  Seq<char>* output;         // Output from print statements
  Seq<Stmt*>* stack;         // Control stack
  Seq<Vec>* loadBuffer;      // Load buffer
};

// State of the Interpreter.
struct InterpreterState {
  CoreState core[MAX_QPUS];  // State of each core
  Word vpm[VPM_SIZE];        // Shared VPM memory
  int sema[16];              // Semaphores
};

// Interpreter
void interpreter
  ( int numCores           // Number of cores active
  , Stmt* s                // Source code
  , int numVars            // Max var id used in source
  , Seq<int32_t>* uniforms // Kernel parameters
  , Seq<char>* output      // Output from print statements
                           // (if NULL, stdout is used)
  );

#endif

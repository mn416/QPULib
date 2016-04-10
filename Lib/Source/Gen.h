// A random source-program generator

#ifndef _GEN_H_
#define _GEN_H_

#include "Common/Seq.h"
#include "Source/Syntax.h"

// Options for the program generator
struct GenOptions {
  // Limit on depth of nesting of statements and expressions
  int depth;

  // Limit on length of statement sequences
  int length;

  // Number of arguments to use when generating kernels
  int numIntArgs;
  int numFloatArgs;
  int numPtrArgs;
  int numPtr2Args;

  // Number of variables to use when generating kernels
  int numIntVars;
  int numFloatVars;

  // Loop bound (max iterations of any loop)
  int loopBound;

  // Generate float operations?
  bool genFloat;

  // Generate rotate operations?
  bool genRotate;

  // Generate pointer-dereferencing operations?
  bool genDeref;
  bool genDeref2;

  // We can dereference expressions of the form p+e where p is a 
  // pointer variable and e is an integer expression that is bitwise
  // anded with derefOffsetMask.
  int derefOffsetMask;

  // Allow loads and stores to be strided?
  bool genStrided;
};

// Generate random literals
int genIntLit();
float genFloatLit();

// Generate a random source program
Stmt* progGen(GenOptions* opts, int* numVars);

#endif

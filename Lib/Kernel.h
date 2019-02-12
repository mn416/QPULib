#ifndef _QPULIB_KERNEL_H_
#define _QPULIB_KERNEL_H_

#include "qpulib_config.h"

#include "Source/Interpreter.h"
#include "Target/Emulator.h"
#include "Target/Encode.h"
#include "VideoCore/SharedArray.h"
#include "VideoCore/Invoke.h"
#include "VideoCore/VideoCore.h"
#include "Source/Pretty.h"
#include "Target/Pretty.h"

namespace QPULib {

// ============================================================================
// Modes of operation
// ============================================================================

// Two important compile-time macros are QPULIB_EMULATION_MODE and QPULIB_QPU_MODE.
// With -D QPULIB_EMULATION_MODE, QPULib can be compiled for any architecture.
// With -D QPULIB_QPU_MODE, QPULib can be compiled only for the Raspberry Pi.
// At least one of these macros must be defined.

// IN QPULIB_EMULATION_MODE a memory pool is used for allocating data that
// can be read by kernels.  Otherwise, a mailbox interface to the
// VideoCore is used to allocate memory.  In both cases, see
// 'VideoCore/SharedArray.h'.

// The 'Kernel' class provides various ways to invoke a kernel:
//
//   * qpu(...)        invoke kernel on physical QPUs
//                     (only available in QPULIB_QPU_MODE)
//   * emulate(...)    invoke kernel using target code emulator
//                     (only available in QPULIB_EMULATION_MODE)
//   * interpret(...)  invoke kernel using source code interpreter
//                     (only available in QPULIB_EMULATION_MODE)
//   * call(...)       in QPULIB_EMULATION_MODE, same as emulate(...)
//                     in QPULIB_QPU_MODE, same as qpu(...)
//                     in QPULIB_EMULATION_MODE *and* QPULIB_QPU_MODE, same as emulate(...)

// Notice it is OK to compile with both -D QPULIB_EMULATION_MODE *and*
// -D QPULIB_QPU_MODE.  This feature is provided for doing equivalance
// testing between the physical QPU and the QPU emulator.  However,
// QPULIB_EMULATION_MODE introduces a performance penalty and should be used
// only for testing and debugging purposes.

// Maximum number of kernel parameters allowed
#define MAX_KERNEL_PARAMS 128

// ============================================================================
// Kernel arguments
// ============================================================================

// Construct an argument of QPU type 't'.

template <typename t> inline t mkArg();

template <> inline Int mkArg<Int>() {
  Int x;
  x = getUniformInt();
  return x;
}

template <> inline Float mkArg<Float>() {
  Float x;
  x = getUniformFloat();
  return x;
}

template <> inline Ptr<Int> mkArg< Ptr<Int> >() {
  Ptr<Int> x;
  x = getUniformPtr<Int>();
  return x;
}

template <> inline Ptr<Float> mkArg< Ptr<Float> >() {
  Ptr<Float> x;
  x = getUniformPtr<Float>();
  return x;
}

// ============================================================================
// Parameter passing
// ============================================================================

template <typename... ts> inline void nothing(ts... args) {}

// Pass argument of ARM type 'u' as parameter of QPU type 't'.

template <typename t, typename u> inline bool
  passParam(Seq<int32_t>* uniforms, u x);

// Pass an int
template <> inline bool passParam<Int, int>
  (Seq<int32_t>* uniforms, int x)
{
  uniforms->append((int32_t) x);
  return true;
}

// Pass a float
template <> inline bool passParam<Float, float>
  (Seq<int32_t>* uniforms, float x)
{
  int32_t* bits = (int32_t*) &x;
  uniforms->append(*bits);
  return true;
}

// Pass a SharedArray<int>*
template <> inline bool passParam< Ptr<Int>, SharedArray<int>* >
  (Seq<int32_t>* uniforms, SharedArray<int>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<int*>*
template <> inline bool passParam< Ptr<Ptr<Int>>, SharedArray<int*>* >
  (Seq<int32_t>* uniforms, SharedArray<int*>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<float>*
template <> inline bool passParam< Ptr<Float>, SharedArray<float>* >
  (Seq<int32_t>* uniforms, SharedArray<float>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

// Pass a SharedArray<float*>*
template <> inline bool passParam< Ptr<Ptr<Float>>, SharedArray<float*>* >
  (Seq<int32_t>* uniforms, SharedArray<float*>* p)
{
  uniforms->append(p->getAddress());
  return true;
}

// ============================================================================
// Functions on kernels
// ============================================================================

// Compile a kernel
void compileKernel(Seq<Instr>* targetCode, Stmt* s);

// ============================================================================
// Kernels
// ============================================================================

// A kernel is parameterised by a list of QPU types 'ts' representing
// the types of the parameters that the kernel takes.

// The kernel constructor takes a function with parameters of QPU
// types 'ts'.  It applies the function to constuct an AST.

template <typename... ts> struct Kernel {
  // AST representing the source code
  Stmt* sourceCode;

  // AST representing the target code
  Seq<Instr> targetCode;

  // Parameters to be passed to kernel
  Seq<int32_t> uniforms;

  // The number of variables in the source code
  int numVars;

  // Number of QPUs to run on
  int numQPUs;

  // Memory region for QPU code and parameters
  #ifdef QPULIB_QPU_MODE
  SharedArray<uint32_t>* qpuCodeMem;
  int qpuCodeMemOffset;
  #endif

  // Construct kernel out of C++ function
  Kernel(void (*f)(ts... params)) {
    numQPUs = 1;

    // We can clear the AST heap if we're sure the source program is not being
    // used any more. However, to implement Kernel::pretty(), we keep the
    // source program so we better not clear the heap.
    // astHeap.clear();

    controlStack.clear();
    stmtStack.clear();
    stmtStack.push(mkSkip());
    resetFreshVarGen();
    resetFreshLabelGen();

    // Reserved general-purpose variables
    Int qpuId, qpuCount;
    qpuId = getUniformInt();
    qpuCount = getUniformInt();

    // Construct the AST
    f(mkArg<ts>()...);

    // QPU code to cleanly exit
    kernelFinish();

    // Obtain the AST
    Stmt* body = stmtStack.top();
    stmtStack.pop();

    // For QPULIB_EMULATION_MODE, the following is needed in the interpreter
    // For QPULIB_QPU_MODE, it is here in case a pretty-print is requested
    sourceCode = body;

    // Compile
    compileKernel(&targetCode, body);

    // Remember the number of variables used
    numVars = getFreshVarCount();

    #ifdef QPULIB_QPU_MODE
    enableQPUs();

    // Allocate code mem
    qpuCodeMem = new SharedArray<uint32_t>;

    // Encode target instrs into array of 32-bit ints
    Seq<uint32_t> code;
    encode(&targetCode, &code);

    // Allocate memory for QPU code and parameters
    int numWords = code.numElems + 12*MAX_KERNEL_PARAMS + 12*2;
    qpuCodeMem->alloc(numWords);

    // Copy kernel to code memory
    int offset = 0;
    for (int i = 0; i < code.numElems; i++) {
      (*qpuCodeMem)[offset++] = code.elems[i];
    }
    qpuCodeMemOffset = offset;
    #endif
  }

  #ifdef QPULIB_EMULATION_MODE
  template <typename... us> void emu(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

    emulate
      ( numQPUs          // Number of QPUs active
      , &targetCode      // Instruction sequence
      , numVars          // Number of vars in source
      , &uniforms        // Kernel parameters
      , NULL             // Use stdout
      );
  }
  #endif

  // Invoke the interpreter
  #ifdef QPULIB_EMULATION_MODE
  template <typename... us> void interpret(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

    interpreter
      ( numQPUs          // Number of QPUs active
      , sourceCode       // Source program
      , numVars          // Number of vars in source
      , &uniforms        // Kernel parameters
      , NULL             // Use stdout
      );
  }
  #endif

  // Invoke kernel on physical QPU hardware
  #ifdef QPULIB_QPU_MODE
  template <typename... us> void qpu(us... args) {
    // Pass params, checking arguments types us against parameter types ts
    uniforms.clear();
    nothing(passParam<ts, us>(&uniforms, args)...);

    // Invoke kernel on QPUs
    invoke(numQPUs, *qpuCodeMem, qpuCodeMemOffset, &uniforms);
  }
  #endif
 
  // Invoke the kernel
  template <typename... us> void call(us... args) {
    #ifdef QPULIB_EMULATION_MODE
      emu(args...);
    #else
      #ifdef QPULIB_QPU_MODE
        qpu(args...);
      #endif
    #endif
  };

  // Overload function application operator
  template <typename... us> void operator()(us... args) {
    call(args...);
  }

  // Set number of QPUs to use
  void setNumQPUs(int n) {
    numQPUs = n;
  }

  // Deconstructor
  ~Kernel() {
    #ifdef QPULIB_QPU_MODE
      delete qpuCodeMem;
      disableQPUs();
    #endif
  }


  /**
   * @brief Output a human-readable representation of the source and target code.
   *
   * @param filename  if specified, print the output to this file. Otherwise, print to stdout
   */
  void pretty(const char *filename = nullptr)
  {
    FILE *f = nullptr;

    if (filename == nullptr)
      f = stdout;
    else
    {
      f = fopen(filename, "w");
      if (f == nullptr)
      {
        fprintf(stderr, "ERROR: could not open file '%s' for pretty output\n", filename);
        return;
      }
    }


    // Emit source code
    fprintf(f, "Source code\n");
    fprintf(f, "===========\n\n");
    if (sourceCode == nullptr)
      fprintf(stderr, "<No source code to print>");
    else
      QPULib::pretty(f, sourceCode);

    fprintf(f, "\n");
    fflush(f);

    // Emit target code
    fprintf(f, "Target code\n");
    fprintf(f, "===========\n\n");
    for (int i = 0; i < targetCode.numElems; i++)
    {
      fprintf(f, "%i: ", i);
      QPULib::pretty(f, targetCode.elems[i]);
    }
    fprintf(f, "\n");
    fflush(f);

    if (filename != nullptr) {
      assert(f != nullptr);
      assert(f != stdout);
      fclose(f);
    }
  }
};

// Initialiser

template <typename... ts> Kernel<ts...> compile(void (*f)(ts... params))
{
  Kernel<ts...> k(f);
  return k;
}

}  // namespace QPULib

#endif  // _QPULIB_KERNEL_H_

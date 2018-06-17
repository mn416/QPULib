#ifndef _SHAREDARRAY_H_
#define _SHAREDARRAY_H_

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "VideoCore/Mailbox.h"
#include "VideoCore/VideoCore.h"

#ifdef EMULATION_MODE

// ============================================================================
// Emulation mode
// ============================================================================

// When in EMULATION_MODE allocate memory from a pre-allocated pool.

#include "Target/Emulator.h"

// Implementation
template <typename T> class SharedArray {
 private:
   // Disallow assignment
   void operator=(SharedArray<T> a);
   void operator=(SharedArray<T>& a);

 public:

  uint32_t address;
  uint32_t size;

  // Allocation
  void alloc(uint32_t n) {
    if (emuHeap == NULL) {
      emuHeapEnd = 0;
      emuHeap = new int32_t [EMULATOR_HEAP_SIZE];
    }
    if (emuHeapEnd+n >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: heap overflow (increase EMULATOR_HEAP_SIZE)\n");
      abort();
    }
    else {
      address = emuHeapEnd;
      emuHeapEnd += n;
      size = n;
    }
  }

  // Constructor
  SharedArray(uint32_t n) {
    alloc(n);
  }

  uint32_t getAddress() {
    return address*4;
  }

  T* getPointer() {
    return (T*) &emuHeap[address];
  }

  // Deallocation (does nothing in emulation mode)
  void dealloc() {}

  // Subscript
  T& operator[] (int i) {
    if (address+i >= EMULATOR_HEAP_SIZE) {
      printf("QPULib: accessing off end of heap\n");
      exit(EXIT_FAILURE);
    }
    else
      return (T&) emuHeap[address+i];
  }
};

#else

// ============================================================================
// Not emulation mode
// ============================================================================

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_MEM_MAP 0x0 // cached=0x0; direct=0x20000000

template <typename T> class SharedArray {
 private:
  // Disallow assignment & copying
  void operator=(SharedArray<T> a);
  void operator=(SharedArray<T>& a);
  SharedArray(const SharedArray<T>& a);

  uint32_t handle;
  void* arm_base;
  void* gpu_base;

 public:
  uint32_t size;

  /* Allocate GPU memory and map it into ARM address space */
  void alloc(uint32_t n) {
    // Mailbox, for talking to VideoCore
    int mb = getMailbox();

    // Allocate memory
    handle = mem_alloc(mb, n*4, 4096, GPU_MEM_FLG);
    if (!handle) {
      fprintf(stderr, "Failed to allocate GPU memory.");
      exit(EXIT_FAILURE);
    }
    size = n;
    gpu_base = (void*) mem_lock(mb, handle);
    arm_base = mapmem(BUS_TO_PHYS((uint32_t) gpu_base+GPU_MEM_MAP), n*4);
  }

  // Constructor
  SharedArray() {
    size = handle = 0;
    arm_base = gpu_base = NULL;
  }

  // Constructor
  SharedArray(uint32_t n) {
    size = handle = 0;
    alloc(n);
  }  

  uint32_t getAddress() {
    return (uint32_t) gpu_base;
  }

  T* getPointer() {
    return (T*) gpu_base;
  }

  // Deallocation
  void dealloc() {
    // Mailbox, for talking to VideoCore
    int mb = getMailbox();

    // Free memory
    if (arm_base) unmapmem(arm_base, size);
    if (handle) {
      mem_unlock(mb, handle);
      mem_free(mb, handle);
    }
    size = handle = 0;
    gpu_base = NULL;
    arm_base = NULL;
  }

  // Subscript
  inline T& operator[] (int i) {
    uint32_t* base = (uint32_t*) arm_base;
    return (T&) base[i];
  }

  // Destructor
  ~SharedArray() {
    if (arm_base != NULL) dealloc();
  }
};

#endif

#endif

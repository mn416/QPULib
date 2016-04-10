#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

class Heap
{
  public:
    uint8_t *base;
    unsigned long size, capacity;
    const char* heapName;

    // Construct an empty heap
    Heap()
    {
      base     = NULL;
      capacity = 0;
      size     = 0;
      heapName = "unnamed";
    }

    // Create a heap of a given number of bytes
    void create(unsigned int heapCapacityInBytes)
    {
      if (base != NULL) free(base);
      capacity = heapCapacityInBytes;
      base     = (uint8_t*) malloc(heapCapacityInBytes);
      size     = 0;
    }

    Heap(unsigned int heapCapacityInBytes)
    {
      heapName = "";
      create(heapCapacityInBytes);
    }

    Heap(const char* name, unsigned int heapCapacityInBytes)
    {
      heapName = name;
      create(heapCapacityInBytes);
    }

    // Allocate 'n' elements of type T on the heap
    template <class T> T* alloc(unsigned long n)
    {
      unsigned long nbytes = sizeof(T) * n;
      if (size + nbytes >= capacity) {
        printf("QPULib error: heap '%s' is full.\n", heapName);
        exit(-1);
        return (T*) NULL;
      }
      else {
        uint8_t* p = base + size;
        size += nbytes;
        return (T*) p;
      }
    }

    // Allocate one element of type T on the heap
    template <class T> T* alloc()
    {
      return alloc<T>(1);
    }

    // Free all the structures allocated on the heap
    void clear()
    {
      size = 0;
    }

    // Destroy the heap
    void destroy()
    {
      free(base);
      base     = NULL;
      capacity = 0;
      size     = 0;
    }

    // Destructor
    ~Heap()
    {
      destroy();
    }
};

#endif

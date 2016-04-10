#include "QPULib.h"

void oet(Ptr<Int> p)
{
  setReadStride(1);
  setWriteStride(1);

  Int evens = *p;
  Int odds  = *(p+1);

  For (Int count = 0, count < 16, count++)
    Int evens2 = min(evens, odds);
    Int odds2  = max(evens, odds);

    Int evens3 = rotate(evens2, 15);
    Int odds3  = odds2;

    Where (index() != 15)
      odds2 = min(evens3, odds3);
    End

    Where (index() != 0)
      evens2 = rotate(max(evens3, odds3), 1);
    End

    evens = evens2;
    odds  = odds2;
  End

  *p     = evens;
  *(p+1) = odds;
}

int main()
{
  // Allocate and initialise array shared between ARM and GPU
  auto k = compile(oet);

  // Allocate some data for the kernel to process
  SharedArray<int> a(32);
  for (int i = 0; i < 32; i++)
    a[i] = 100-i;

  // Invoke the kernel and display the result
  k(&a);
  for (int i = 0; i < 32; i++)
    printf("%i: %i\n", i, a[i]);
  
  return 0;
}

/*

Periodic sorter
===============

// Implementation of periodic sorter from:
//
//   Design and verification of a sorter core
//   K. Claessen, M. Sheeran, S. Singh
//
// The 'qfly' network is easy to vectorise due to its simple structure, however
// MergeSort.c should be quicker although slightly more complicated.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Sort 2^N numbers
#define N 20

// All compare-and-swaps in a pass can run in parallel
void pass(int n, int* a, int s0, int s1, int stride)
{
  int end = 1 << n;
  while (s1 < end) {
    if (a[s0] > a[s1]) {
      int tmp = a[s0];
      a[s0] = a[s1];
      a[s1] = tmp;
    }
    s0 += stride;
    s1 += stride;
  }
}

void sort(int n, int* a)
{
  for (int i = 0; i < n; i++) {
    pass(n, a, 0, 1, 2);
    for (int j = 1 << (n-1); j > 1; j >>= 1)
      pass(n, a, 1, j, 2);
  }
}

int main()
{
  // Create random array
  srand(time(NULL));
  int* a = new int [1 << N];
  for (int i = 0; i < 1 << N; i++)
    a[i] = rand() % 65536;

  // Sort it
  sort(N, a);

  // Check that it's sorted
  bool sorted = true;
  for (int i = 1; i < (1 << N); i++)
    sorted = sorted && (a[i-1] <= a[i]);
  printf("sorted = %s\n", sorted ? "true" : "false");
}

Merge sorter
============

// An implementation of Batcher's odd/even merge sort.  This should be
// straightforward to vectorise, and it should allow a hybrid apporach
// in which a odd/even transposition sort is used to sort small blocks
// of elements quickly before merging the results.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Sort 2^N numbers
#define N 20

// All compare-and-swaps in a pass can run in parallel
void pass(int n, int* a, int s0, int s1)
{
  int count = 1;
  int g = s0;
  while (s1 < n) {
    if (a[s0] > a[s1]) {
      int tmp = a[s0];
      a[s0] = a[s1];
      a[s1] = tmp;
    }
    if (count == g) {
      count = 1;
      s0 += g+1;
      s1 += g+1;
    }
    else {
      s0++;
      s1++;
      count++;
    }
  }
}

void merge(int n, int* a) {
  pass(n, a, 0, n>>1);
  for (int i = n; i > 1; i = i >> 1)
    pass(n, a, i>>1, i);
}

void sort(int n, int* a)
{
  for (int i = 2; i <= n; i = i << 1)
    // All merges in this loop can run in parallel
    for (int start = 0; start < n; start += i)
      merge(i, &a[start]);
}

int main()
{
  // Create random array
  srand(time(NULL));
  int* a = new int [1 << N];
  for (int i = 0; i < 1 << N; i++)
    a[i] = rand() % 65536;

  // Sort it
  sort(1<<N, a);

  // Check that it's sorted
  bool sorted = true;
  for (int i = 1; i < (1 << N); i++)
    sorted = sorted && (a[i-1] <= a[i]);
  printf("sorted = %s\n", sorted ? "true" : "false");
}

*/

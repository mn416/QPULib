#include "Target/SmallLiteral.h"
#include <stdio.h>

// Small literals are literals that fit in the small immediate field
// of the VideoCore-IV instruction set.

const int NUM_SMALL_FLOATS = 16;
const float smallFloats[NUM_SMALL_FLOATS] = {
    1.0
  , 2.0
  , 4.0
  , 8.0
  , 16.0
  , 32.0
  , 64.0
  , 128.0
  , 0.00390625
  , 0.0078125
  , 0.015625
  , 0.03125
  , 0.0625
  , 0.125
  , 0.25
  , 0.5
};

// Encode a small literal according to Table 5 of the VideoCore-IV
// manual. Returns -1 if expression cannot be encoded as a small
// literal.

int encodeSmallLit(Expr* e)
{
  if (e->tag == INT_LIT) { 
    if (e->intLit >= 0 && e->intLit <= 15)
      return e->intLit;
    else if (e->intLit >= -16 && e->intLit <= -1)
      return 32 + e->intLit;
  } 
  else if (e->tag == FLOAT_LIT) {
    if (e->floatLit == 0.0)
      return 0;
    else {
      int index = -1;
      for (int i = 0; i < NUM_SMALL_FLOATS; i++)
        if (smallFloats[i] == e->floatLit) {
          index = i;
          break;
        }
      if (index != -1)
        return 32 + index;
    }
  }
  return -1;
}

// Determine if a given expression (source language) can be stored in
// a small immediate.

bool isSmallLit(Expr* e)
{
  return encodeSmallLit(e) >= 0;
}

// Decode a small literal.

Word decodeSmallLit(int x)
{
  Word w;
  if (x >= 32) {
    w.floatVal = smallFloats[x-32];
    return w;
  }
  else if (x >= 16) {
    w.intVal = x-32;
    return w;
  }
  else if (x >= 0) {
    w.intVal = x;
    return w;
  }

  // Unreachable
  assert(false);
}

// Display a small literal.

void printSmallLit(int x)
{
  if (x >= 32)
    printf("%f", smallFloats[x-32]);
  else if (x >= 16)
    printf("%i", x-32);
  else if (x >= 0)
    printf("%i", x);
}

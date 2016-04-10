#ifndef _ENCODE_H_
#define _ENCODE_H_

#include <stdint.h>
#include "Target/Syntax.h"
#include "Common/Seq.h"

void encode(Seq<Instr>* instrs, Seq<uint32_t>* code);

#endif

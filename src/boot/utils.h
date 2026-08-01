#pragma once
#include <int.h>

#include "term/term.h"

#define panic(msg) { clearTerm(); print("(%s) -> (%s) -> (line %i) -> %s", __FILE__, __FUNCTION__, __LINE__, msg); for (;;) ;}

void* memset(void* ptr, int value, u32 count);

int x86InstructionLength(const uint8_t* code);

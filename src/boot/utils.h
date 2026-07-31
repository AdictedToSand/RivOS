#pragma once
#include <int.h>

#include "term/term.h"

#define panic(msg) { clearTerm(); print("(%s) -> (%s) -> (line %i) -> %s", __FILE__, __FUNCTION__, __LINE__, msg); for (;;) ;}

static void* memset(void* ptr, int value, u32 count) {
    u8* p = (u8*) ptr;

    for (u32 i = 0; i < count; i++)
        p[i] = (u8) value;

    return ptr;
}

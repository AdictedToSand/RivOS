#pragma once
#include <int.h>

static inline u32 strlen(const char* s) {
    u32 i = 0;
    while (s[i++]);
    return i - 1;
}


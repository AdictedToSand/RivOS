#pragma once
#include <stddef.h>

#include <int.h>

// Credits: https://www.geeksforgeeks.org/cpp/write-memcpy/ 
static inline void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i]; // Simple forward copy
    }
    return dest;
}

static inline void* memset(void* b, int c, int len){
    unsigned char* p = (unsigned char*) b;
    while(len > 0) {
        *p = c;
        p++;
        len--;
    }
    return b;
}

static auto memcmp(const void* a, const void* b, size_t size) -> int {
    const u8* x = (const u8*) a;
    const u8* y = (const u8*) b;

    for (size_t i = 0; i < size; i++) {
        if (x[i] != y[i])
            return x[i] - y[i];
    }

    return 0;
}

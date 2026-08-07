#include "utils.h"

#include <int.h>

// Credits: https://www.geeksforgeeks.org/cpp/write-memcpy/ 
void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i]; // Simple forward copy
    }
    return dest;
}

void* memset(void* b, int c, int len) {
    unsigned char* p = (unsigned char*) b;
    while(len > 0) {
        *p = c;
        p++;
        len--;
    }
    return b;
}

void* memmove(void* dst, const void* src, u32 count) {
    u8* d = (u8*)dst;
    const u8* s = (const u8*)src;

    if (d < s) {
        for (u32 i = 0; i < count; i++)
            d[i] = s[i];
    } else if (d > s) {
        for (u32 i = count; i != 0; i--)
            d[i - 1] = s[i - 1];
    }

    return dst;
}

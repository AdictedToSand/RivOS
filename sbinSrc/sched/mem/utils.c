#include "utils.h"

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

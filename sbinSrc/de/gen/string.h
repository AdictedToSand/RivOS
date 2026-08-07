#pragma once
#include <int.h>

static inline u32 strlen(const char* s) {
    u32 i = 0;
    while (s[i++]) ;
    return i - 1;
}

static inline u32 strlenSpecChar(const char* s, const char term) {
    u32 i = 0;
    while (s[i++] != term) ;
    return i - 1;
}

static inline char* strcpy(char* dest, const char* src) {
    char* ret = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return ret;
}

static inline u32 countOccurence(const char* s, char c) {
    u32 count = 0;
    for (u32 i = 0; s[i]; i++) {
        if (s[i] == c) count++;
    }

    return count;
}

static inline void strcpyLen(char* dest, const char* src, u32 len) {
    for (u32 i = 0; i < len; i++) {
        dest[i] = src[i];
    }
    dest[len] = 0;
}

static inline int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static inline u8 streq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}



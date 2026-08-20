#pragma once
#include <stddef.h>
#include <stdbool.h>

#include <gen/alpha.hpp>

static inline size_t strlen(const char* s) {
    size_t i = 0;
    while (s[i++]);
    return i - 1;
}

static inline int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static inline bool streq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

static inline bool streqi(const char* a, const char* b) {
    while (*a && *b) {
        if (toUpper(*a) != toUpper(*b)) return false;
        a++;
        b++;
    }
    return *a == *b; // both must hit '\0' at the same time
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
static inline void strcpyLen(char* dest, const char* src, u32 len) {
    for (u32 i = 0; i < len; i++) {
        dest[i] = src[i];
    }
    dest[len] = 0;
}
static inline auto strcat(char* dest, const char* src) -> char* {
    char* end = dest;

    while (*end)
        end++;

    while ((*end++ = *src++))
        ;

    return dest;
}

static inline char* stradd(char* dest, const char* src) {
    char* ret = dest;

    while (*dest != '\0') {
        dest++;
    }

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    return ret;
}



static inline bool strBeginsWith(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str++ != *prefix++)
            return false;
    }

    return true;
}

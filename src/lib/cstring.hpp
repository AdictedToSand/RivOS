#pragma once
#include <mem/alloc.hpp>

#include <stddef.h>
#include <stdbool.h>

#include <gen/alpha.hpp>

#include <mem/utils.hpp>

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

static inline u32 strlenSpecChar(const char* str, char target) {
    u32 len = 0;

    while (str[len] && str[len] != target)
        len++;

    return len;
}
static inline u32 findChar(const char* str, char target) {
    for (u32 i = 0; str[i]; i++) {
        if (str[i] == target)
            return i;
    }

    return 0;
}

static inline u32 countOccurence(const char* s, char c) {
    u32 count = 0;
    for (u32 i = 0; s[i]; i++) {
        if (s[i] == c) count++;
    }

    return count;
}
static inline auto heapCopyStr(const char* s) -> char* {
    u32 len   = strlen(s) + 1;
    char* n = (char*) KernelAllocator::alloc(len);
    memcpy(n, s, len);
    return n;
}

#pragma once
#include <stdbool.h>
#include <stddef.h>

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

static inline char toUpper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    return c;
}

static inline char toLower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}
static inline bool isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline void strToUpper(char* s) {
    while (*s) {
        *s = toUpper(*s);
        s++;
    }
}

static inline bool streqi(const char* a, const char* b) {
    while (*a && *b) {
        if (toUpper(*a) != toUpper(*b)) return false;
        a++;
        b++;
    }
    return *a == *b; // both must hit '\0' at the same time
}


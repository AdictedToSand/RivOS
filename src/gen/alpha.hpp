#pragma once

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

static inline void strToUpper(char* s) {
    while (*s) {
        *s = toUpper(*s);
        s++;
    }
}

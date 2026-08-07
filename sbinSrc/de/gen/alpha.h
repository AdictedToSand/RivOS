#pragma once
#include <int.h>
#include <stdbool.h>

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

static inline u8 isdigit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool strIsDigit(const char* str) {
    if (*str == '\0')
        return false;

    while (*str) {
        if (!isdigit(*str))
            return false;

        str++;
    }

    return true;
}

static inline i32 stoi(const char* str) {
    i32 result = 0;
    bool negative = false;

    if (*str == '-') {
        negative = true;
        str++;
    }

    while (*str) {
        if (!isdigit(*str))
            return 0;

        result = result * 10 + (*str - '0');
        str++;
    }

    return negative ? -result : result;
}


static inline bool strIsNumber(const char* str) {
    if (*str == '\0')
        return false;

    if (*str == '-' || *str == '+')
        str++;

    if (*str == '\0')
        return false;

    while (*str) {
        if (*str < '0' || *str > '9')
            return false;

        str++;
    }

    return true;
}

static inline u32 stou(const char* s) {
    u32 result = 0;

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return result;
}

static bool isLower(char c) {
    return c >= 'a' && c <= 'z';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

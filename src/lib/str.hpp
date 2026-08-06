#pragma once
#include <stddef.h>

#include <mem/utils.hpp>
#include <mem/alloc.hpp>

#include <cstring.hpp>

struct Str {
private:
    char* cStr;
    u32 len;
    u32 capacity;
    static constexpr size_t MINIMUM_STR_SIZE = 5;
public:
    Str() {
        cStr = (char*) KernelAllocator::alloc(MINIMUM_STR_SIZE);

        len = 0;
        capacity = MINIMUM_STR_SIZE;

        memset(cStr, 0, 5);
    }
    Str(const char* s) {
        const size_t allocSize = (strlen(s) >= MINIMUM_STR_SIZE ? strlen(s) + MINIMUM_STR_SIZE : MINIMUM_STR_SIZE);
        cStr = (char*) KernelAllocator::alloc(allocSize);

        len = strlen(s);
        capacity = allocSize;

        strcpy(cStr, s);
    }

    ~Str() {
        KernelAllocator::free(cStr);
    }
    inline const char* toCStr() const {
        return cStr;
    }
    inline auto size() const -> u32 {
        return len;
    }
    inline auto fstrlen() const -> u32 {
        return len;
    }
    inline operator const char*() const {
        return cStr;
    }

    inline auto operator+=(const char* s) -> void {
        const size_t strlenOfS = strlen(s);

        while (len + strlenOfS >= capacity) {
            char* newArr = (char*) KernelAllocator::alloc(capacity * 2);

            strcpy(newArr, cStr);

            KernelAllocator::free(cStr);
            cStr = newArr;
            capacity *= 2;
        }
        len += strlenOfS;
        stradd(cStr, s);
    }
    auto operator+=(const char c) -> void {
        size_t curLen = strlen(cStr);
        if (++len >= capacity) {
            char* newArr = (char*) KernelAllocator::alloc(capacity * 2);
            strcpy(newArr, cStr);
            KernelAllocator::free(cStr);
            cStr = newArr;
            capacity *= 2;
        }
        cStr[curLen] = c;
        cStr[curLen + 1] = '\0';
    }
    auto operator+=(Str s) -> void {
        *this += s.toCStr();
    }
    inline auto pushBack(const char c) -> void {
        *this += c; // Use the operator+= alr defined
    }
    inline auto add(const char* s) -> void {
        *this += s;
    }
    auto fromi(i32 n) -> void {
        len = 0;
        cStr[0] = '\0';

        if (n == 0) {
            *this += '0';
            return;
        }

        if (n < 0) {
            *this += '-';
            n = -n;
        }

        char buffer[12];
        size_t index = 0;

        while (n > 0) {
            buffer[index++] = '0' + (n % 10);
            n /= 10;
        }

        while (index > 0) {
            *this += buffer[--index];
        }
    }

    auto fromu(u32 n) -> void {
        len = 0;
        cStr[0] = '\0';

        if (n == 0) {
            *this += '0';
            return;
        }

        char buffer[11];
        size_t index = 0;

        while (n > 0) {
            buffer[index++] = '0' + (n % 10);
            n /= 10;
        }

        while (index > 0) {
            *this += buffer[--index];
        }
    }
};

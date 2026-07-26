#pragma once
#include <stddef.h>

#include <mem/alloc.hpp>

#include <string.hpp>

struct Str {
private:
    char* cStr;
    size_t len;
    size_t capacity;
    static constexpr size_t MINIMUM_STR_SIZE = 5;
public:
    Str(const char* s) {
        const size_t allocSize = (strlen(s) > MINIMUM_STR_SIZE ? strlen(s) + MINIMUM_STR_SIZE : MINIMUM_STR_SIZE);
        cStr = (char*) KernelAllocator::alloc(allocSize);

        len = 0;
        capacity = allocSize;

        strcpy(cStr, s);
    }

    ~Str() {
        KernelAllocator::free(cStr);
    }
    inline const char* toCStr() const {
        return cStr;
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
        if (++len >= capacity) {
            char* newArr = (char*) KernelAllocator::alloc(capacity * 2);

            strcpy(newArr, cStr);

            KernelAllocator::free(cStr);
            cStr = newArr;
            capacity *= 2;
        }

        cStr[strlen(cStr)] = c;
        cStr[strlen(cStr)] = '\0';
    }
    inline auto pushBack(const char c) -> void {
        *this += c; // Use the operator+= alr defined
    }

    //TODO: popBack and eraseAt
};

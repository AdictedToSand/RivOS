#pragma once
#include <stddef.h>

namespace std {
    template<typename T>
    class initializer_list {
    private:
        const T* arr;
        size_t len;

        // Compiler calls this private constructor directly when it
        // lowers a { a, b, c } braced-init-list — must stay private,
        // must stay this exact shape (const T*, size_t).
        constexpr initializer_list(const T* iarr, size_t ilen) : arr(iarr), len(ilen) {}

    public:
        constexpr initializer_list() noexcept : arr(nullptr), len(0) {}

        constexpr auto size() const noexcept -> size_t { return len; }
        constexpr auto begin() const noexcept -> const T* { return arr; }
        constexpr auto end() const noexcept -> const T* { return arr + len; }
    };
}

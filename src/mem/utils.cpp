#include <mem/utils.hpp>

auto memcmp(const void* a, const void* b, size_t size) -> int {
    const u8* x = (const u8*) a;
    const u8* y = (const u8*) b;

    for (size_t i = 0; i < size; i++) {
        if (x[i] != y[i])
            return x[i] - y[i];
    }

    return 0;
}

auto operator new(size_t, void* ptr) noexcept -> void* {
    return ptr;
}

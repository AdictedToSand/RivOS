#pragma once
#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <int.h>
#include <cstring.hpp>

struct StringView {
    const char* raw;
    u32 len;
    StringView() : raw(""), len(0) {};
    StringView(const char* src) {
        raw = src;
        len = strlen(src);
    }
    StringView(const char* src, u32 ilen) {
        raw = src;
        len = ilen;
    }

    auto toCStr() -> const char* {
        char* buf = (char*) KernelAllocator::alloc(len + 1);
        memset(buf, 0, len + 1);
        for (u32 i = 0; i < len; i++) {
            buf[i] = raw[i]; 
        }
        return buf;
    }
    inline auto getLen() -> u32 {
        return len;
    }
    auto startsWith(StringView sv) -> bool {
        if (sv.len > len) return false;
        for (u32 i = 0; i < sv.len && i < len; i++) {
            if (sv.raw[i] != raw[i]) return false;
        }
        return true;
    }
};

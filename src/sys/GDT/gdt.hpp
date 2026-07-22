#pragma once

extern "C" void gdtFlush();
struct Gdt {
    static inline auto init() -> void {
        gdtFlush();
    }
};

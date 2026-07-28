#pragma once
#include <stdint.h>

#include <gen/io.hpp>

// Credits: https://wiki.osdev.org/Reboot
[[gnu::noreturn]]
static inline auto reboot() -> void {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    asm volatile ("hlt");

    __builtin_unreachable();
}

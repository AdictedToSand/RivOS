#pragma once
#include <stdint.h>

// Credits: https://wiki.osdev.org/Inline_Assembly/Examples

static inline auto outb(uint16_t port, uint8_t val) -> void {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline auto inb(uint16_t port) -> uint8_t {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void ioWait() {
    outb(0x80, 0);
}

#pragma once
#include "mem/alloc.hpp"
#include <int.h>

#include <mem/utils.hpp>

// Credits: https://wiki.osdev.org/Inline_Assembly/Examples

static inline auto outb(u16 port, u8 val) -> void {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline auto inb(u16 port) -> u8 {
    uint8_t ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


static inline auto inw(u16 port) -> u16 {
    uint16_t ret;
    asm volatile ( "inw %w1, %w0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline auto outw(u16 port, u16 val) -> void {
    asm volatile ("outw %0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline auto ioWait() -> void {
    outb(0x80, 0);
}

auto getSc(void) -> u8;

static char* scancodeMap = (char*) KernelAllocator::alloc(128);
static char* scancodeMapShift = (char*) KernelAllocator::alloc(128);

auto getc() -> char;

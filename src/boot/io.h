#pragma once
#include <int.h>

static void outb(u16 port, u8 val) {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

u8 inb(u16 port);

u16 inw(u16 port);

static inline void ioWait(void) {
    outb(0x80, 0);
}

#pragma once
#include <int.h>

static void outb(u16 port, u8 val) {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static u8 inb(u16 port) {
    u8 ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


static u16 inw(u16 port) {
    u16 ret;
    asm volatile ( "inw %w1, %w0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static void ioWait() {
    outb(0x80, 0);
}


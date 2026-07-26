#pragma once
#include <stdint.h>

//TODO: Better structure in organization

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ( "inw %w1, %w0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void ioWait() {
    outb(0x80, 0);
}

extern char bootloaderEnd[];
extern char bootloaderStart[];

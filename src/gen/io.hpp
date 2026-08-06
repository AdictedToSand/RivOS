#pragma once
#include "mem/alloc.hpp"
#include <int.h>

#include <mem/utils.hpp>

// Credits: https://wiki.osdev.org/Inline_Assembly/Examples

static inline auto outb(u16 port, u8 val) -> void {
    asm volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline auto inb(u16 port) -> u8 {
    u8 ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


static inline auto inw(u16 port) -> u16 {
    u16 ret;
    asm volatile ( "inw %w1, %w0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline auto outw(u16 port, u16 val) -> void {
    asm volatile ("outw %0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}
static inline u32 inl(u16 port) {
    u32 value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline auto ioWait() -> void {
    outb(0x80, 0);
}

auto getSc(void) -> u8;

static char* scancodeMap = (char*) KernelAllocator::alloc(128);
static char* scancodeMapShift = (char*) KernelAllocator::alloc(128);

auto getc() -> char;

static inline void scInit(void) {
    for (u32 i = 0; i < 128; i++) {
        scancodeMap[i] = 0;
        scancodeMapShift[i] = 0;
    }

    scancodeMap[0x02] = '1';
    scancodeMap[0x03] = '2';
    scancodeMap[0x04] = '3';
    scancodeMap[0x05] = '4';
    scancodeMap[0x06] = '5';
    scancodeMap[0x07] = '6';
    scancodeMap[0x08] = '7';
    scancodeMap[0x09] = '8';
    scancodeMap[0x0A] = '9';
    scancodeMap[0x0B] = '0';

    scancodeMap[0x10] = 'q';
    scancodeMap[0x11] = 'w';
    scancodeMap[0x12] = 'e';
    scancodeMap[0x13] = 'r';
    scancodeMap[0x14] = 't';
    scancodeMap[0x15] = 'y';
    scancodeMap[0x16] = 'u';
    scancodeMap[0x17] = 'i';
    scancodeMap[0x18] = 'o';
    scancodeMap[0x19] = 'p';

    scancodeMap[0x1E] = 'a';
    scancodeMap[0x1F] = 's';
    scancodeMap[0x20] = 'd';
    scancodeMap[0x21] = 'f';
    scancodeMap[0x22] = 'g';
    scancodeMap[0x23] = 'h';
    scancodeMap[0x24] = 'j';
    scancodeMap[0x25] = 'k';
    scancodeMap[0x26] = 'l';

    scancodeMap[0x2C] = 'z';
    scancodeMap[0x2D] = 'x';
    scancodeMap[0x2E] = 'c';
    scancodeMap[0x2F] = 'v';
    scancodeMap[0x30] = 'b';
    scancodeMap[0x31] = 'n';
    scancodeMap[0x32] = 'm';

    scancodeMap[0x39] = ' ';
    scancodeMap[0x1C] = '\n';
    scancodeMap[0x0E] = '\b';
    scancodeMap[0x0F] = '\t';

    scancodeMapShift[0x02] = '!';
    scancodeMapShift[0x03] = '@';
    scancodeMapShift[0x04] = '#';
    scancodeMapShift[0x05] = '$';
    scancodeMapShift[0x06] = '%';
    scancodeMapShift[0x07] = '^';
    scancodeMapShift[0x08] = '&';
    scancodeMapShift[0x09] = '*';
    scancodeMapShift[0x0A] = '(';
    scancodeMapShift[0x0B] = ')';

    scancodeMapShift[0x10] = 'Q';
    scancodeMapShift[0x11] = 'W';
    scancodeMapShift[0x12] = 'E';
    scancodeMapShift[0x13] = 'R';
    scancodeMapShift[0x14] = 'T';
    scancodeMapShift[0x15] = 'Y';
    scancodeMapShift[0x16] = 'U';
    scancodeMapShift[0x17] = 'I';
    scancodeMapShift[0x18] = 'O';
    scancodeMapShift[0x19] = 'P';

    scancodeMapShift[0x1E] = 'A';
    scancodeMapShift[0x1F] = 'S';
    scancodeMapShift[0x20] = 'D';
    scancodeMapShift[0x21] = 'F';
    scancodeMapShift[0x22] = 'G';
    scancodeMapShift[0x23] = 'H';
    scancodeMapShift[0x24] = 'J';
    scancodeMapShift[0x25] = 'K';
    scancodeMapShift[0x26] = 'L';

    scancodeMapShift[0x2C] = 'Z';
    scancodeMapShift[0x2D] = 'X';
    scancodeMapShift[0x2E] = 'C';
    scancodeMapShift[0x2F] = 'V';
    scancodeMapShift[0x30] = 'B';
    scancodeMapShift[0x31] = 'N';
    scancodeMapShift[0x32] = 'M';

    scancodeMapShift[0x39] = ' ';
    scancodeMapShift[0x1C] = '\n';
    scancodeMapShift[0x0E] = '\b';
    scancodeMapShift[0x0F] = '\t';
}

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

static auto getSc(void) -> u8 {
    while ((inb(0x64) & 0x01) == 0) {
        // wait
    }
    return inb(0x60);
}

static inline bool caps;
static inline bool shift;

static char* scancodeMap = (char*) KernelAllocator::alloc(128);
static char* scancodeMapShift = (char*) KernelAllocator::alloc(128);

// Hehe poor chatgpt
static auto ioInit() -> void {
    memset(scancodeMap, 0, sizeof(scancodeMap));
    memset(scancodeMapShift, 0, sizeof(scancodeMapShift));

    scancodeMap[0x1C] = '\n';
    scancodeMap[0x39] = ' ';
    scancodeMap[0x0E] = '\b';
    scancodeMap[0x0F] = '\t';

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

    scancodeMap[0x33] = ',';
    scancodeMap[0x34] = '.';
    scancodeMap[0x35] = '/';
    scancodeMap[0x2B] = '\\';
    scancodeMap[0x0C] = '-';
    scancodeMap[0x0D] = '=';
    scancodeMap[0x27] = ';';
    scancodeMap[0x28] = '\'';
    scancodeMap[0x29] = '`';
    scancodeMap[0x1A] = '[';
    scancodeMap[0x1B] = ']';

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

    scancodeMapShift[0x33] = '<';
    scancodeMapShift[0x34] = '>';
    scancodeMapShift[0x35] = '?';
    scancodeMapShift[0x2B] = '|';
    scancodeMapShift[0x0C] = '_';
    scancodeMapShift[0x0D] = '+';
    scancodeMapShift[0x27] = ':';
    scancodeMapShift[0x28] = '"';
    scancodeMapShift[0x29] = '~';
    scancodeMapShift[0x1A] = '{';
    scancodeMapShift[0x1B] = '}';
}

static inline auto getc(void) -> char {
    unsigned char sc = getSc();

    if (sc & 0x80) {
        sc &= 0x7F;
        if (sc == 0x2A || sc == 0x36)
            shift = 0;
        return 0;
    }

    if (sc == 0x2A || sc == 0x36) {
        shift = 1;
        return 0;
    }

    if (sc == 0x3A) {
        caps = !caps;
        return 0;
    }

    char c = shift ? scancodeMapShift[sc] : scancodeMap[sc];
    if (!c)
        return 0;

    if (c >= 'a' && c <= 'z' && caps ^ shift) // For sume fucking reason caps+shift == upercase???
        c -= 32;
    
    return c;
}

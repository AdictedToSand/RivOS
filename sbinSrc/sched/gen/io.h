#pragma once
#include <int.h>

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

typedef u32 fd_t;

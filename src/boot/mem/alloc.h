#pragma once
#include <int.h>

extern char bootEnd[];
static u8* heapPtr = (u8*) bootEnd;

static void* alloc(u32 size) {
    // Right now, we can assume we *Probably* have enough memory for our bootloader needs
    void* p = heapPtr;
    heapPtr += (size + 15) & ~15;
    return p;
}

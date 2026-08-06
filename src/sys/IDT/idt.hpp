// Sorry for any style incosistenties in this file!

#pragma once
#include <gen/err.hpp>

#include <terminal/terminal.hpp>

#include <stdint.h>

// Credits: https://wiki.osdev.org/Interrupts_Tutorial

/*
struc InterruptFrame
    .vector: resd 1
endstruc*/

struct InterruptFrame {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 vector;
    u32 errorCode;

    u32 eip;
    u32 cs;
    u32 eflags;
    u32 userEsp;
    u32 ss;
};

extern "C" // Disable name mangling
    void exceptionHandler(InterruptFrame* ifrm); // Must be defined in a .cpp due to static linking shit

struct [[gnu::packed]] IdtEntry {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
};

struct [[gnu::packed]] Idtr {
    uint16_t limit;
    uint32_t base;
};

extern "C" void* isr_stub_table[];
struct Idt {
    __attribute__((aligned(0x10)))
    inline static IdtEntry idt[256];
    inline static Idtr idtr;

    static constexpr size_t IDT_MAX_DESCRIPTORS = 256;
    inline static bool vectors[IDT_MAX_DESCRIPTORS];

    static auto setDescriptor(uint8_t vector, void* isr, uint8_t flags) -> void {
        IdtEntry* descriptor = &idt[vector];

        descriptor->isr_low = (uint32_t) isr & 0xFFFF;
        descriptor->kernel_cs = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
        descriptor->attributes = flags;
        descriptor->isr_high = (uint32_t) isr >> 16;
        descriptor->reserved = 0;
    }

    static auto init() -> void {
        idtr.base = (uintptr_t) &idt[0];
        idtr.limit = (uint16_t) sizeof(IdtEntry) * IDT_MAX_DESCRIPTORS - 1;

        for (uint16_t vector = 0; vector < 32; vector++) {
            setDescriptor(vector, isr_stub_table[vector], 0x8E);
            vectors[vector] = true;
        }
        setDescriptor(0x80, isr_stub_table[0x80], 0x8E);

        __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
        //__asm__ volatile ("sti"); // set the interrupt flag
    }
};

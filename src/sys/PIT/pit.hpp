#pragma once
#include <sys/PIC/pic.hpp>

#include <sys/IDT/idt.hpp>

extern volatile u64 ticks;

extern "C" void irq0Stub();

struct PIT {
private:
    static inline u32 frequency;

    static constexpr u32 BASE_FREQ = 1193182;
public:
    static auto init(u32 freqInputted) -> void {
        frequency = freqInputted; 
        const u32 divisor = BASE_FREQ / frequency;

        outb(0x43, 0x36);

        outb(0x40, divisor & 0xFF);
        outb(0x40, divisor >> 8);
    }
};

/*irq0Stub*/

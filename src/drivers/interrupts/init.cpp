#include "init.hpp"

auto HardwareInterrupts::init() -> void {
    // Why the fuck is this needed, and why only now???
    outb(0x64, 0x20);
    u8 config = inb(0x60);

    config |= 1; // IRQ1 enable

    outb(0x64, 0x60);
    outb(0x60, config);

    Idt::setDescriptor(0x20, (void*) pitStub, 0x8E);
    Idt::setDescriptor(0x21, (void*) keyboardStub, 0x8E);
    Idt::setDescriptor(0x80, (void*) syscallStub, 0x8E);

    PIC::irqClearMask(0);
    PIC::irqClearMask(1);

    asm volatile ("STI");
}

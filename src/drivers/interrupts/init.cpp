#include "init.hpp"

auto HardwareInterrupts::init() -> void {
        Idt::setDescriptor(0x20, (void*) pitStub, 0x8E);
        Idt::setDescriptor(0x21, (void*) keyboardStub, 0x8E);

        PIC::irqClearMask(0);
        PIC::irqClearMask(1);

        asm volatile ("STI");
}
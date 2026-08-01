// Credits: https://wiki.osdev.org/8259_PIC
#pragma once
#include <int.h>

#include <sys/IDT/idt.hpp>

#include <gen/io.hpp>

extern "C" void irq1Stub();

struct PIC {
private:
    static constexpr u8 PIC1 = 0x20;
    static constexpr u8 PIC2 = 0xA0;
    static constexpr u8 PIC1_CMD = PIC1;
    static constexpr u8 PIC1_DATA = PIC1 + 1;
    static constexpr u8 PIC2_CMD = PIC2;
    static constexpr u8 PIC2_DATA = PIC2 + 1;

    static constexpr u8 PIC_EOI = 0x20;

    static constexpr u8 ICW1_ICW4 = 0x01; // Indicates that ICW4 will be present
    static constexpr u8 ICW1_SINGLE = 0x02; // Single (cascade) mode
    static constexpr u8 ICW1_INTERVAL4 = 0x04; // Call address interval 4 (8)
    static constexpr u8 ICW1_LEVEL = 0x08; // Level triggered (edge) mode
    static constexpr u8 ICW1_INIT = 0x10; // Initialization (required)
                                          
    static constexpr u8 ICW4_8086 = 0x01; // 8086/77 (MCS-80/85) mode
    static constexpr u8 ICW4_AUTO = 0x02; // Auto (normal) EOI
    static constexpr u8 ICW4_BUF_SLAVE = 0x08; // Buffered mode/slave
    static constexpr u8 ICW4_BUF_MASTER = 0x0C; // Buffered mode/master
    static constexpr u8 ICW4_SFNM = 0x10; // Special fully nested (not)
                                          
    static constexpr u8 CASCADE_IRQ = 2; 

    static auto remap(i32 offset1, i32 offset2) -> void {
        outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
        ioWait();
        outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
        ioWait();
        outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
        ioWait();
        outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
        ioWait();
        outb(PIC1_DATA, 1 << CASCADE_IRQ);        // ICW3: tell Master PIC that there is a slave PIC at IRQ2
        ioWait();
        outb(PIC2_DATA, CASCADE_IRQ);             // ICW3: tell Slave PIC its cascade identity
        ioWait();
        
        outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
        ioWait();
        outb(PIC2_DATA, ICW4_8086);
        ioWait();

        // Unmask both PICs.
        outb(PIC1_DATA, 0);
        outb(PIC2_DATA, 0);
    }

    static auto irqSetMask(u8 IRQline) -> void {
        u16 port;
        u8 value;

        if(IRQline < 8) {
            port = PIC1_DATA;
        } else {
            port = PIC2_DATA;
            IRQline -= 8;
        }
        value = inb(port) | (1 << IRQline);
        outb(port, value);        
    }

    static auto irqClearMask(u8 IRQline) -> void {
        uint16_t port;
        uint8_t value;

        if(IRQline < 8) {
            port = PIC1_DATA;
        } else {
            port = PIC2_DATA;
            IRQline -= 8;
        }
        value = inb(port) & ~(1 << IRQline);
        outb(port, value);        
    }

public:
    static auto init() -> void {
        remap(0x20, 0x28);

        // Unmask everything (Mainly PIT is important)
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);

        irqClearMask(1);
        Idt::setDescriptor(0x21, (void*) irq1Stub, 0x8E);

        asm volatile ("STI");
    }
    static auto sendEoi(u8 irq) -> void {
        if (irq > 8)
            outb(PIC2_CMD, PIC_EOI);
        outb(PIC1_CMD, PIC_EOI);
    }
    // Call this when enabling APIC
    static auto disable() -> void {
        outb(PIC1_DATA, 0xff);
        outb(PIC2_DATA, 0xff);
    }
};

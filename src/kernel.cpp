#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Terminal::writeStr("Hello, kernel world!\nHi");

    Gdt::init();
    Idt::init();
    asm volatile ("int $0x00");

    for (;;);
}

} // extern "C"

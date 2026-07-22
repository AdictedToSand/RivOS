#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>

extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Terminal::writeStr("Hello, kernel world!\n");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    Storage::init();

    

    for (;;);
}

} // extern "C"

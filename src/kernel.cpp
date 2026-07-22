#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>


extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Terminal::writeStr("Hello, kernel world!\nHi");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    Vector<int> vec;
    vec.pushBack(5);
    Terminal::printi(vec[0]);
    vec[0] = 0;
    Terminal::printi(vec[0]);

    for (;;);
}

} // extern "C"

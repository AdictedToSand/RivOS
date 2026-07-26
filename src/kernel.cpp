#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/fs.hpp>

//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    Terminal::writeStr("RivBoot worked yay\n");

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    Storage::init();

    FileSystem::init();

    for (;;);
}

} // extern "C"

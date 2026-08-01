#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>
#include <gen/map.hpp>
#include <gen//reboot.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>
#include <sys/PIC/pic.hpp>
#include <sys/PIT/pit.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>
#include <mem/mmu/mmu.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/fs.hpp>
#include <drivers/fs/file.hpp>
#include <drivers/interrupts/init.hpp>

#include <ACPI/ACPI.hpp>

#include <str.hpp>

typedef void (*ctor_t)();

extern "C" ctor_t ctorsStart[];
extern "C" ctor_t ctorsEnd[];

void callGlobalConstructors() {
    // .ctors is stored in REVERSE order, walk backwards
    for (ctor_t* ctor = ctorsEnd - 1; ctor >= ctorsStart; ctor--) {
        if (*ctor != (ctor_t)-1) { // skip sentinel if present
            (*ctor)();
        }
    }
}
extern "C" void irq1Handler() {
    if (inb(0x60) & 0x80) {

    }
    else {
        Terminal::printf("Keypress");
    }
    PIC::sendEoi(1);
}

auto disableHardwareCursor() -> void {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

//TODO: i32 inst of int32_t
//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

auto kernelMain() -> void {
    asm volatile ("CLI");
    Terminal::init();
    disableHardwareCursor();

    ACPI::init();

    ioInit();

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    callGlobalConstructors();

    Storage::init();

    FileSystem::init();

    PIC::init();

    PIT::init(1000);

    Mmu::init();

    HardwareInterrupts::init();

    Terminal::printf("End of kernel reached");
    for (;;) ;
}

} // extern "C"

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

#include <proc/ELF/loader.hpp>
#include <proc/process.hpp>

#include <str.hpp>

#include <bootInfo.hpp>

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

auto disableHardwareCursor() -> void {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

//TODO: i32 inst of int32_t
//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

auto kernelMain(u32 magic) -> void {
    asm volatile ("CLI");
    if (magic == 0x2BADB002) {
        bootinfo.bootloader = BootloaderKinds::GRUB;
    }
    else {
        bootinfo.bootloader = BootloaderKinds::RivBoot;
    }
    Terminal::init();
    disableHardwareCursor();
    Terminal::printf("Bootloader: %s\n", (bootinfo.bootloader == BootloaderKinds::GRUB ? "GRUB" : "RivBoot"));

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

    ElfExecutable scheduler;
    if (scheduler.fromFile("/krn/bin/sched")) { kpanic("Scheduler not found!!!"); }
    if (!scheduler.isValid()) { kpanic("Scheduler was not a valid ELF"); }
    scheduler.load("RivOS_Sched", ProcessPriveledgeLevel::Kernel);
    Process* proc = processes[0];
    Terminal::printf("ProcessName: %s, Pid: %u, srcFp: %s\n", proc->pname, proc->pid, proc->srcFp.toCStr());

    Terminal::printf("End of kernel reached");
    for (;;) ;
}

} // extern "C"

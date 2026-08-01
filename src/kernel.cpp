#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>
#include <gen/map.hpp>
#include <gen//reboot.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/fs.hpp>
#include <drivers/fs/file.hpp>

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

auto disableHardwareCursor() -> void {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

//TODO: i32 inst of int32_t
//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

auto kernelMain() -> void {
    Terminal::init();
    disableHardwareCursor();

    ioInit();

    Terminal::writeStr("RivBoot worked yay\n");

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    callGlobalConstructors();

    Storage::init();

    FileSystem::init();

    fd_t stdout = FileSystem::open("/dev/stdout");
    char* msg = (char*) "WRITE\n";
    FileSystem::write(stdout, msg, strlen(msg));

    fd_t bootf = FileSystem::open("/boot/sector=1"); 
    if (!bootf) kpanic("BootFile not found???");
    char* buf = (char*) KernelAllocator::alloc(512);
    memset(buf, 0, 512);
    FileSystem::write(bootf, buf, 512);

    Terminal::printf("%s\n", buf);

    Terminal::printf("End of kernel reached");
    getc();
    reboot();
    for (;;) ;
}

} // extern "C"

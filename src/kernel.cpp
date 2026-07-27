#include "drivers/fs/driver.hpp"
#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>
#include <gen/map.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>
#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/fs.hpp>

#include <str.hpp>

/*
extern "C" void _init_array_start();
extern "C" void _init_array_end();
*/

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

//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>
extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    Terminal::writeStr("RivBoot worked yay\n");

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    Gdt::init();
    Idt::init();

    KernelAllocator::init();

    callGlobalConstructors();

    Storage::init();

    FileSystem::init();

    fd_t stdout = FileSystem::open("/dev/stdout");

    char* buf = (char*) KernelAllocator::alloc(256);
    memset(buf, 0, 256);
    
    strcpy(buf, "This is a write to stdout\n");
    if (FileSystem::write(stdout, buf, strlen(buf)) == FileSystemDriver::SuccessCodes::Error) {
        kpanic("");
    }

    memset(buf, 0, 256);
    FileSystem::read(stdout, buf, 256);
    Terminal::printf("Conts: '%s'", buf);
    
    for (;;) ;
}

} // extern "C"

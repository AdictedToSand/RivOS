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

    File f = FileSystem::open("/bin/hi.txt");

    if (!f.exists) {
        kpanic("File did not exist");
    }

    char* buf = (char*) KernelAllocator::alloc(256);
    memset(buf, 0, 256);
    FileSystem::read(f, buf, 256);
    Terminal::printf("Conts: %s", buf);

    Map<int, int> map;
    map[4] = 3;

    Terminal::printf("Sum: %i\n", map[4]);

    for (;;);
}

} // extern "C"

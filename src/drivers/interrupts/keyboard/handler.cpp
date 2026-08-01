#include <terminal/terminal.hpp>

#include <drivers/fs/fs.hpp>

#include <int.h>

#include <sys/PIC/pic.hpp>

#include <gen/io.hpp>

extern "C" auto keyboardHandler() -> void {
    const u8 sc = inb(0x60);
    ioInit();
    if (sc & 0x80) {

    }
    else {
        fd_t stdinFd = FileSystem::open("/dev/stdin");
        FileSystem::write(stdinFd, &scancodeMap[sc], 1);
        FileSystem::close(stdinFd);
    }

    PIC::sendEoi(1);
}

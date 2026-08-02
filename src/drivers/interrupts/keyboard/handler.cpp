#include "mem/alloc.hpp"
#include <terminal/terminal.hpp>

#include <drivers/fs/fs.hpp>

#include <int.h>

#include <sys/PIC/pic.hpp>

#include <gen/io.hpp>
#include <gen/alpha.hpp>

bool shift = false;
bool capslock = false;

extern "C" auto keyboardHandler() -> void {
    const u8 sc = inb(0x60);
    ioInit();
    if (sc & 0x80) {
        if (sc == 0xAA || sc == 0xB6) {
            shift = false;
        }
    }
    else {
        if (sc == 0x2A || sc == 0x36) { // Left or right shift
            shift = true;

            goto endOfFunction;
        }
        else if (sc == 0x3A) {
            capslock = !capslock;

            goto endOfFunction;
        }

        fd_t stdinFd = FileSystem::open("/dev/stdin");
        char addedC = scancodeMap[sc];
        if (isLower(addedC)) {
            if (capslock ^ shift) addedC = toUpper(addedC);
        }
        else if (shift) {
            addedC = scancodeMapShift[sc];
        }
        if (addedC == '\b') {
            char* const tmpStdinBuf = (char*) KernelAllocator::alloc(4096);

            FileSystem::read(stdinFd, tmpStdinBuf, 4096);
            if (u32 slen = strlen(tmpStdinBuf); slen > 0)
                tmpStdinBuf[slen - 1] = 0;

            KernelAllocator::free(tmpStdinBuf);
        }

        FileSystem::write(stdinFd, &addedC, 1);
        FileSystem::close(stdinFd);
    }
endOfFunction:
    PIC::sendEoi(1);
}

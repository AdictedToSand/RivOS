#include <terminal/terminal.hpp>

#include <drivers/fs/fs.hpp>

#include <int.h>

#include <sys/PIC/pic.hpp>

#include <gen/io.hpp>
#include <gen/alpha.hpp>

#include <mem/alloc.hpp>

#include <proc/process.hpp>

#include <sys/sysmod.hpp>

#include <gen/io.hpp>

enum {
    SC_ESCAPE      = 0x01,
    SC_LEFT_CTRL   = 0x1D,
    SC_LEFT_SHIFT  = 0x2A,
    SC_RIGHT_SHIFT = 0x36,
    SC_LEFT_ALT    = 0x38,
    SC_CAPS_LOCK   = 0x3A,
    SC_F1          = 0x3B, // F1..F10 are 0x3B..0x44 contiguous
    SC_NUM_LOCK    = 0x45,
    SC_SCROLL_LOCK = 0x46,
    SC_F11         = 0x57,
    SC_F12         = 0x58,
};

enum {
    SC_ESCAPE_RELEASE      = 0x81,
    SC_LEFT_CTRL_RELEASE   = 0x9D,
    SC_LEFT_SHIFT_RELEASE  = 0xAA,
    SC_RIGHT_SHIFT_RELEASE = 0xB6,
    SC_LEFT_ALT_RELEASE    = 0xB8,
    SC_CAPS_LOCK_RELEASE   = 0xBA,
    SC_F1_RELEASE          = 0xBB,
    SC_NUM_LOCK_RELEASE    = 0xC5,
    SC_SCROLL_LOCK_RELEASE = 0xC6,
    SC_F11_RELEASE         = 0xD7,
    SC_F12_RELEASE         = 0xD8,
};

struct {
    bool shift;
    bool capslock;
} keyboardState;

// Genuinely what the fuck is up with PS/2
// TODO: Shift & Capslock stopped working
extern "C" auto keyboardHandler() -> void {
    scInit();

    const u8 sc = inb(0x60);
    if (sc & 0x80) {
        if (sc == SC_LEFT_SHIFT_RELEASE || sc == SC_RIGHT_SHIFT_RELEASE) keyboardState.shift = false;
    }
    else {
        if (sc == SC_LEFT_SHIFT || sc == SC_RIGHT_SHIFT) {
            keyboardState.shift = true;
            PIC::sendEoi(1);
            return;
        }
        if (sc == SC_CAPS_LOCK) {
            keyboardState.capslock = !keyboardState.capslock;
            PIC::sendEoi(1);
            return;
        }

        fd_t stdinFd = FileSystem::open("/dev/stdin");
        if (!stdinFd) kpanic("Stdin did not exist");
        char addedC = scancodeMap[sc];
        if (isLower(addedC)) {
            if (keyboardState.capslock ^ keyboardState.shift) addedC = toUpper(addedC);
        }
        else if (keyboardState.shift) {
            addedC = scancodeMapShift[sc];
        }
        if (addedC == '\b') {
            char* const tmpStdinBuf = (char*) KernelAllocator::alloc(4096);

            FileSystem::read(stdinFd, tmpStdinBuf, 4096);
            const u32 slen = strlen(tmpStdinBuf);
            if (slen > 0)
                tmpStdinBuf[slen - 1] = 0;

            KernelAllocator::free(tmpStdinBuf);
        }

        FileSystem::write(stdinFd, &addedC, 1);
        // Display character on screen (this is temporary)
        const fd_t stdout = FileSystem::open("/dev/stdout");
        FileSystem::write(stdout, &addedC, 1);
        FileSystem::close(stdout);
        FileSystem::close(stdinFd);

    }


    PIC::sendEoi(1);
}

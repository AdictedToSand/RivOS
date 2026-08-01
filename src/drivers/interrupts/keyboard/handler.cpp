#include <int.h>

#include <sys/PIC/pic.hpp>

extern "C" auto keyboardHandler() -> void {
    const u8 sc = inb(0x60);
    if (sc & 0x80) {

    }
    else {
        Terminal::printf("KEYy");
    }

    PIC::sendEoi(1);
}
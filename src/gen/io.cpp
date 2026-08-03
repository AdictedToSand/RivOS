#include <gen/io.hpp>

auto getc() -> char {
    return getSc(); // getc() should not be used, use /dev/stdin instead. However to keep waiting consistent 
}

auto getSc(void) -> u8 {
    while ((inb(0x64) & 0x01) == 0) {
        // wait
    }
    return inb(0x60);
}

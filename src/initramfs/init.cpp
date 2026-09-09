#include <initramfs/init.hpp>

auto tinyStub(u32 p1, u32 p2, u32 p3) -> void {
    Terminal::printf("I WAS CALLED w/ %x %x %x", p1, p2, p3);
}

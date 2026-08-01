#include <sys/PIT/pit.hpp>

volatile u64 ticks = 0;

extern "C" auto pitHandler() -> void {
    ticks++;
    PIC::sendEoi(0);
}

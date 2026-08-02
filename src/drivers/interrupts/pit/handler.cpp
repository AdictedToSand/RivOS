#include <sys/PIT/pit.hpp>

volatile u64 ticks = 0;

// Proper stack setup is not required here
extern "C" auto pitHandler() -> void {
    ticks++;
    PIC::sendEoi(0);
}

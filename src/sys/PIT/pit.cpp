#include <sys/PIT/pit.hpp>

volatile u64 ticks = 0;

extern "C" void irq0Handler() {
    ticks++;
    PIC::sendEoi(0);
}

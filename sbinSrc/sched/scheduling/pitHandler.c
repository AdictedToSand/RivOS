#include "pitHandler.h"

#include "../sys/stdio.h"

#define PROCESS_INTERVAL 50

u64 ticks = 0;
void pitHandler(void) {
    ticks++;
    if (!(ticks % 1000))  {
        //ctxtSwitch();
    }
}

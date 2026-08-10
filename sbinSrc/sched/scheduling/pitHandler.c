#include "pitHandler.h"

#include "../sys/stdio.h"

u64 ticks = 0;
void pitHandler(void) {
    ticks++;
    if (!(ticks % 1000)) 
        printf("TICK");
}

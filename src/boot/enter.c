#include <stdint.h>

uint16_t test = 0x0F41;

typedef struct Terminal {
    uint16_t* vgaCursor;
} Terminal;

Terminal term;

void initTerm() {
    term.vgaCursor = (uint16_t*) 0xB8000;
}
void putc(char c) {
    term.vgaCursor[0] = (c | (0x0F << 8));
}

void startBoot() {
    initTerm();

    putc('A');

    for (;;) ;
}
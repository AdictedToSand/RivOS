#include <stdint.h>

#include "utils.h"
#include "term/term.h"

void startBoot() {
    initTerm();

    putsColor("Welcome to RivBoot. Currently we are finding a kernel.\n", VGA_COLOR_GREEN);
    char c = 'A';
    const char* s = "SSS";
    int i = 30505;
    unsigned int u = 2147483648;
    void* p = &u;
    int x = 0xA5BF;
    print("\nChar: %c\nString: %s\nInt: %i\nUint: %u\nPtr: %p\nHex: %x\nMod: %%\n", c, s, i, u, p, x);

    panic("Very bad panic");

    for (;;) ;
}

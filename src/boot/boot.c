#include <stdint.h>
#include <stddef.h>

typedef struct Terminal {
    size_t cursor;
    volatile uint16_t* vga;
} Terminal;
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

Terminal term;

void terminit(void) {
    term.cursor = 0;
    term.vga = (volatile uint16_t*) 0xB8000;

    for (int x = 0; x < VGA_WIDTH; x++) {
        for (int y = 0; y < VGA_HEIGHT; y++) {
            term.vga[x + y * VGA_WIDTH] =  ' ' | 0x0F << 8;
        }
    }
}

void putc(char c) {
    if (c == '\n') {
        term.cursor = (term.cursor / VGA_WIDTH + 1) * VGA_WIDTH;
    }
    else 
        term.vga[term.cursor++] = c | 0x0F << 8;

    if (term.cursor > VGA_HEIGHT * VGA_WIDTH) {
        term.cursor = 0;
    }
}

void puts(const char* s) {
    while (*s) putc(*s++);
}

// Credits: https://www.geeksforgeeks.org/c/print-long-int-number-c-using-putchar/
void printi(long n) {

    if (n < 0) {
        putc('-');
        n = -n;
    }

    if (n/10) {
        printi(n/10);
    }

    putc(n%10 + '0');
}

/* struc KernelInfo
    .drive: resb 1
    .sector: resw 1
    .sizeAddr: resw 1 ;
    .entryPointAddr: resw 1 ; Where to look for these variabless
endstruc
*/

typedef struct [[gnu::packed]] FoundKernel_initial {
    uint8_t drive;
    uint16_t sector;
    uint16_t sizeAddr;
    uint16_t entryPointAddr;
} FoundKernel_initial;

void startBoot(FoundKernel_initial* fkrnel) {
    terminit();

    printi((long) fkrnel->drive);

    puts("Hello world\nhi");

    for (;;);
}

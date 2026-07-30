#include "term.h"

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

Terminal term;

void initTerm() {
    term.activeColor = VGA_COLOR_WHITE;
    term.vgaCursor = VGA_MEMORY;

    for (uint16_t i = 0; i < VGA_SIZE; i++) {
        term.vgaCursor[i] = ' ';
    }
}

void clearTerm() {
    term.vgaCursor = VGA_MEMORY;

    for (uint16_t i = 0; i < VGA_SIZE; i++) term.vgaCursor[i] = ' ';
}

void putc(char c) {
    if (c == '\n') {
        uintptr_t index = (uintptr_t) (term.vgaCursor - VGA_MEMORY);
        index += VGA_WIDTH - (index % VGA_WIDTH);
        term.vgaCursor = &VGA_MEMORY[index];
    }
    else {
        *term.vgaCursor = (c | (term.activeColor << 8));
        term.vgaCursor++;
    }

    if ((uintptr_t) term.vgaCursor - (uintptr_t) VGA_MEMORY > VGA_SIZE) 
        clearTerm();
}
void puts(const char* s) {
    while (*s)
        putc(*s++);
}
void putsColor(const char* s, uint8_t color) {
    // Color will be used as a temporary for term.previousActiveColor
    // We will use the triple XOR swap
    color ^= term.activeColor;
    term.activeColor ^= color;
    color ^= term.activeColor;

    puts(s);
    term.activeColor = color; // Remember: color contaisn term.previousActiveColor
}


inline void setTermColor(uint8_t color) {
    term.activeColor = color;
}

inline uint8_t vgaEntry(VgaColor fg, VgaColor bg) {
    return (uint8_t) fg | ((uint8_t) bg << 4);
}

void puti(int n) {
    if (n < 0) {
        putc('-');
        n = -n;
    }
    if (n / 10) puti(n / 10);

    putc(n % 10 + '0');
}
void putu(const unsigned int n) {
    if (n / 10) putu(n / 10);

    putc(n % 10 + '0');
}
void putx(const unsigned int n) {
    const char hexChars[] = "0123456789ABCDEF";
      
    puts("0x");

    for (int i = (sizeof(unsigned int) * 8) - 4; i >= 0; i -= 4) {
        putc(hexChars[(n >> i) & 0x0F]);
    }
}
void putp(const void* const p) {
    const char hexChars[] = "0123456789ABCDEF";
      
    puts("0x");

    for (int i = (sizeof(void*) * 8) - 4; i >= 0; i -= 4) {
        putc(hexChars[((uintptr_t) p >> i) & 0x0F]);
    }
}



void print(const char* s, ...) {
    va_list args;
    va_start(args, s);

    while (*s) {
        if (*s == '%') {
            s++;
            switch (*s) {
                case 'c': putc(va_arg(args, int)); break;
                case 's': puts(va_arg(args, char*)); break;
                case 'i': puti(va_arg(args, int)); break;
                case 'u': putu(va_arg(args, unsigned int)); break;
                case 'p': putp(va_arg(args, void*)); break;
                case 'x': putx(va_arg(args, unsigned int)); break;

                case '%': putc('%'); break;
                case '\0': putc('%'); return;
                default: putc(*s);
            }
        }
        else {
            putc(*s);
        }

        s++;
    }

    va_end(args);
}

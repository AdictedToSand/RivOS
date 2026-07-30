#pragma once
#include <stdint.h>

typedef enum VgaColor {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GRAY = 7,
    VGA_COLOR_DARK_GRAY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
} VgaColor;

typedef struct Terminal {
    uint16_t* vgaCursor;
    uint8_t activeColor;
} Terminal;

extern Terminal term;

#define VGA_WIDTH (80)
#define VGA_LENGTH (25)
#define VGA_SIZE (VGA_WIDTH * VGA_LENGTH)
#define VGA_MEMORY ((uint16_t*) 0xB8000)

void initTerm();

void clearTerm();

void putc(char c);
void puts(const char* s);
void putsColor(const char* s, uint8_t color);

uint8_t vgaEntry(VgaColor fg, VgaColor bg);
void setTermColor(uint8_t color);
void print(const char* s, ...); // Ofc the compiler gives a warning for redefining a library function (if printf). With no library!

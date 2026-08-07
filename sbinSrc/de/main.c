#include "sys/stdio.h"
#include "sys/sys.h"

#include "rap/rap.h"

#include "lisp/lisp.h"

RapFile rap;

void _start() {
    sysInit();

    claim("Framebuffer");
    rap = parseRap();

    void (*putPixel)(u32 argb, u32 x, u32 y) = getRapAddr(&rap, "putPixel");
    u32 (*getScreenWidth)(void) = getRapAddr(&rap, "getScreenWidth");
    u32 (*getScreenHeight)(void) = getRapAddr(&rap, "getScreenHeight");

    for (u32 x = 0; x < getScreenWidth(); x++) {
        for (u32 y = 0; y < getScreenHeight(); y++) {
            const u32 colorx = y > 0 ? x % y : 0;
            const u32 colory = x > 0 ? y % x : 0;
            putPixel(colorx ^ colory, x, y);
        }
    }

    lispRun("(+ 145 2)\n"
    "(print \"Hello world\")");
    //"(print \"Hello, world!\")\n");

    for (;;) ;
}

#include "sys/stdio.h"
#include "sys/sys.h"

#include "rap/rap.h"

#include "lisp/lisp.h"

#include "mem/utils.h"

RapFile rap;

u32 screenWidth;
u32 screenHeight;

void (*putPixel)(u32 argb, u32 x, u32 y);

void _start() {
    sysInit();

    claim("Framebuffer");
    rap = parseRap();

    putPixel = getRapAddr(&rap, "putPixel");
    u32 (*getScreenWidth)(void) = getRapAddr(&rap, "getScreenWidth");
    u32 (*getScreenHeight)(void) = getRapAddr(&rap, "getScreenHeight");

    screenHeight = getScreenHeight(), screenWidth = getScreenWidth();

    for (u32 x = 0; x < getScreenWidth(); x++) {
        for (u32 y = 0; y < getScreenHeight(); y++) {
            const u32 colorx = y > 0 ? x % y : 0;
            const u32 colory = x > 0 ? y % x : 0;
            putPixel(colorx ^ colory, x, y);
        }
    }
    fd_t shaderSrc = open("/krn/de/shder.lsp");
    if (!shaderSrc) {
        exit(1);
    }
    #define DEF_FILESIZE 100
    char* buf = mmap(DEF_FILESIZE);
    memset(buf, 0, DEF_FILESIZE);
    read(shaderSrc, buf, DEF_FILESIZE);
    printf("Src='%s'", buf);
    lispRun(buf);
    close(shaderSrc);

    munmap(buf);
    for (;;) ;
}

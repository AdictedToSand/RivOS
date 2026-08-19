#include "sys/stdio.h"
#include "sys/sys.h"

#include "rap/rap.h"

#include "lisp/lisp.h"

#include "mem/utils.h"

RapFile rap;

u32 screenWidth;
u32 screenHeight;

void (*putPixelRap)(u32 argb, u32 x, u32 y);

void putPixel(u32 argb, u32 x, u32 y) {
    putPixelRap(argb, x, y);
}

void _start() {
    sysInit();

    claim("Framebuffer");
    rap = parseRap();

    putPixelRap = getRapAddr(&rap, "putPixel");
    u32 (*getScreenWidth)(void) = getRapAddr(&rap, "getScreenWidth");
    u32 (*getScreenHeight)(void) = getRapAddr(&rap, "getScreenHeight");

    screenHeight = getScreenHeight(), screenWidth = getScreenWidth();

    fd_t shaderSrcFd = open("/etc/shder.lsp");
    if (!shaderSrcFd) {
        exit(1);
    }
    #define DEF_FILESIZE 1000
    char* buf = mmap(DEF_FILESIZE);
    memset(buf, 0, DEF_FILESIZE);
    read(shaderSrcFd, buf, DEF_FILESIZE);
    lispRun(buf);
    close(shaderSrcFd);
    munmap(buf);

    printf("WE ARE DONE\n");
    u32 ticks = 0;
    for (;;) {
        ticks++;
        if (ticks % 0xA000000 == 0) {
            printf("Vela was called");
        }
    }   
}

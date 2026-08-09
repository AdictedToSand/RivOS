#include "sys/stdio.h"
#include "sys/sys.h"

#include "rap/rap.h"

#include "lisp/lisp.h"

#include "mem/utils.h"

#include "shaderLang/lang.h"

RapFile rap;

u32 screenWidth;
u32 screenHeight;
u32 fbPitch;

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
    u32 (*getFbPitch)(void) = getRapAddr(&rap, "getFbPitch");

    fbPitch = getFbPitch();
    screenHeight = getScreenHeight(), screenWidth = getScreenWidth();

    const u8 shaderSrc[] = {
        MAKE_OPCODE(MOV, MOV_REGTOCONST), 1, 0, 0, 0, 1, 
        MAKE_OPCODE(LOAD, LOAD_REG), 1,
        0
    };
    decompile(shaderSrc);

    /*fd_t shaderSrc = open("/krn/de/shder.lsp");
    if (!shaderSrc) {
        exit(1);
    }
    #define DEF_FILESIZE 1000
    char* buf = mmap(DEF_FILESIZE);
    memset(buf, 0, DEF_FILESIZE);
    read(shaderSrc, buf, DEF_FILESIZE);
    printf("Src='%s'", buf);
    lispRun(buf);
    close(shaderSrc);
    munmap(buf);
*/

    for (;;) ;
}

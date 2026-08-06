#include <int.h>
#include <stdbool.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "gen/string.h"

#include "rap/rap.h"

void pitHandler(void) {
}

RapFile rap;

typedef void (*PutPixelT)(u32 argb, u32 x, u32 y);

[[gnu::noreturn]]
void _start() {
    sysInit();
    if (claim("PIT")) {
        puts("Unable to claim PIT");
        exit(1);
    };
    setFunc("PIT", pitHandler);
    if (claim("Framebuffer") != 0) {
        puts("Could not claim Framebuffer");
        exit(1);
    }          
    rap = parseRap();
    printRap(&rap);

    PutPixelT putPixel = getRapAddr(&rap, "putPixel");
    if (!putPixel) exit(1);
    u32 (*getScreenWidth)(void) = getRapAddr(&rap, "getScreenWidth");
    u32 (*getScreenHeight)(void) = getRapAddr(&rap, "getScreenHeight");
    logf("ScreenWidth=%u", getScreenWidth());
    logf("ScreenHeight=%u", getScreenHeight());

    const u32 screenWidth = getScreenWidth();
    const u32 screenHeight = getScreenHeight();

    for (u32 x = 0; x < screenWidth; x++) {
        for (u32 y = 0; y < screenHeight; y++) {
            putPixel(0x0000FF00, x, y); 
        }
    }

    for (;;) ;
}

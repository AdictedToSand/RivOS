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

    /*[LOG]: Function=getScreenWidth                                                                                                                                               
[LOG]:          Addr=1062948                                                                                                                                                 
[LOG]: Function=getFbPhysAddr                                                                                                                                                
[LOG]:          Addr=1062858                                                                                                                                                 
[LOG]: Function=getScreenHeight                                                                                                                                              
[LOG]:          Addr=1062918    */

    //PutPixelT putp = getRapAddr(&rap, "putPixel");
    //if (!putp) exit(1);
    //u32 (*getScreenWidth)(void) = getRapAddr();

    for (;;) ;
}

#include <int.h>
#include <stdbool.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "gen/string.h"

#include "keyboard/keyboard.h"

void pitHandler(void) {
}

void _start() {
    sysInit();
    stdout = open("/dev/stdout"); 

    keyboardInit();

    claim("Keyboard_PS2");
    setFunc("Keyboard_PS2", keyboardHandler);
    claim("PIT");
    setFunc("PIT", pitHandler);
    if (claim("Framebuffer") != 0) {
        puts("TODO: Framebuffer, .rap, backspace terminal\n");
        exit(1);
    }

    for (;;) ;
}

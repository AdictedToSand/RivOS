#include <int.h>
#include <stdbool.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "gen/string.h"

#include "rap/rap.h"

#include "keyboard/keyboard.h"

void pitHandler(void) {
}

RapFile rap;

[[gnu::noreturn]]
void _start() {
    sysInit();
    keyboardInit();

    claim("Keyboard_PS2");
    setFunc("Keyboard_PS2", keyboardHandler);
    claim("PIT");
    setFunc("PIT", pitHandler);
    if (claim("Framebuffer") != 0) {
        exit(1);
    }          
    rap = parseRap();
    printRap(&rap);

    for (;;) ;
}

#include <int.h>
#include <stdbool.h>

#include "sys/sys.h"
#include "keyboard/keyboard.h"

void pitHandler(void) {
    
}

void _start() {
    sysInit();
    stdout = open("/dev/stdout"); 

    keyboardInit();

    claim("Keyboard_PS2");
    setFunc("Keyboard_PS2", keyboardHandler);

    //exit(0);

    for (;;) ;
}

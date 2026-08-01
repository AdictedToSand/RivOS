#include "menu.h"

#include "../utils.h"
#include "../term/term.h"

#include <stddef.h>
#include <int.h>

#define ARRUP_CHAR '>'
#define ARRDOWN_CHAR '<'

char inputScMap[256] = {
    [0x1C] = '\n',
    [0x48] = ARRUP_CHAR,
    [0x50] = ARRDOWN_CHAR
};

char getInput(void) {
    return inputScMap[getSc()];
}

void printMenuFmt(MenuItem* menitem, u32 itemCount) {
    const char* fmt = menitem->fmt;

    for (; *fmt; fmt++) {
        if (*fmt == '%') {
            fmt++;

            switch (*fmt) {
                case '\0': return;
                case 'c':
                    print("%u", itemCount);
                    break;
                case 's':
                    print("%s", menitem->str);
                    break;
                default:
                    print("%%%c", *fmt);
                    break;
            }
        }
        else {
            putc(*fmt);
        }
    }
    putc('\n');
}

u32 getMenLen(Menu* men) {
    for (u32 i = 0; ; i++) {
        if (men->items[i].isLastItem) return i;
    }
}

void displayMenu(Menu* men, const char* displaySplash) {
    size_t focusedOption = 0;
    while (true) {
        clearTerm();
        print(displaySplash, men->str);

        for (u32 i = 0; !men->items[i].isLastItem; i++) {
            if (i == focusedOption) setTermColor(vgaEntry(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GRAY));
            printMenuFmt(&men->items[i], i + 1);
            setTermColor(VGA_COLOR_WHITE);
        }

        const char inp = getInput();

        switch (inp) {
            case '\n': {
                men->items[focusedOption].onPressed(&men->items[focusedOption]);
                break;
            }
            case ARRUP_CHAR: {
                if (focusedOption > 0) focusedOption--;
                else focusedOption = getMenLen(men) - 1;
                break;
            }
            case ARRDOWN_CHAR: {
                if (focusedOption < getMenLen(men) - 1) focusedOption++;
                else focusedOption = 0;
                break;    
            }
        }
    }

    for (;;) ;
}

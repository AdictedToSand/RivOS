#include <stdbool.h>

#include "../gen/io.h"

#include "../gen/alpha.h"
#include "../gen/scMap.h"
#include "../gen/string.h"

#include "../sys/sys.h"

struct {
    bool shift;
    bool capslock;
} keyboardState;

void keyboardHandler(void) {
    const u8 sc = inb(0x60);
    if (sc & 0x80) {
        if (sc == SC_LEFT_SHIFT_RELEASE || sc == SC_RIGHT_SHIFT_RELEASE) keyboardState.shift = false;
    }
    else {
        if (sc == SC_LEFT_SHIFT || sc == SC_RIGHT_SHIFT) {
            keyboardState.shift = true;
            return;
        }
        if (sc == SC_CAPS_LOCK) {
            keyboardState.capslock = !keyboardState.capslock;
            return;
        }

        fd_t stdinFd = open("/dev/stdin");
        if (!stdinFd) exit(1);
        char addedC = scancodeMap[sc];
        if (isLower(addedC)) {
            if (keyboardState.capslock ^ keyboardState.shift) addedC = toUpper(addedC);
        }
        else if (keyboardState.shift) {
            addedC = scancodeMapShift[sc];
        }
        if (addedC == '\b') {
            char* const tmpStdinBuf = (char*) mmap(4096);

            read(stdinFd, tmpStdinBuf, 4096);
            const u32 slen = strlen(tmpStdinBuf);
            if (slen > 0)
                tmpStdinBuf[slen - 1] = 0;

            munmap(tmpStdinBuf);
        }

        write(stdinFd, &addedC, 1);
        // Display character on screen (this is temporary)
        write(stdout, &addedC, 1);
        close(stdinFd);

    }
}
void keyboardInit(void) {
     keyboardState.shift = false;
     keyboardState.capslock = false;
}


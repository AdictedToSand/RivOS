#include <stdint.h>

#include "scMap.h"

typedef uint8_t u8;
typedef uint16_t u16;

typedef uint32_t u32;

typedef u8 bool;
#define true 1
#define false 0
typedef u32 fd_t;

extern fd_t open(const char* fp);
extern u8 write(fd_t fd, const char* conts, u32 len);
extern u8 claim(const char* mod);
extern u8 release(const char* mod);
extern u8 setFunc(const char* mod, void (*)(void));
extern void* mmap(u32 size);
extern void munmap(void* mem);

u32 strlen(const char* s) {
    u32 i = 0;
    while (s[i++]);
    return i - 1;
}

fd_t stdout = 1;

void pitHandler(void) {
}

/*const u8 sc = inb(0x60);
    ioInit();
    if (sc & 0x80) {
        if (sc == 0xAA || sc == 0xB6) {
            shift = false;
        }
    }
    else {
        if (sc == 0x2A || sc == 0x36) { // Left or right shift
            shift = true;

            goto endOfFunction;
        }
        else if (sc == 0x3A) {
            capslock = !capslock;

            goto endOfFunction;
        }

        fd_t stdinFd = FileSystem::open("/dev/stdin");
        if (!stdinFd) {
            kpanic("Stdin not found");
        }
        char addedC = scancodeMap[sc];
        if (isLower(addedC)) {
            if (capslock ^ shift) addedC = toUpper(addedC);
        }
        else if (shift) {
            addedC = scancodeMapShift[sc];
        }
        if (addedC == '\b') {
            char* const tmpStdinBuf = (char*) KernelAllocator::alloc(4096);

            FileSystem::read(stdinFd, tmpStdinBuf, 4096);
            if (u32 slen = strlen(tmpStdinBuf); slen > 0)
                tmpStdinBuf[slen - 1] = 0;

            KernelAllocator::free(tmpStdinBuf);
        }

        FileSystem::write(stdinFd, &addedC, 1);
        FileSystem::close(stdinFd);
*/

static inline u8 inb(u16 port) {
    uint8_t ret;
    asm volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

struct {
    bool shift;
    bool capslock;
} keyboardState;

void keyboardHandler() {
    const u8 sc = inb(0x60);
    if (sc & 0x80) {
        if (sc == SC_LEFT_SHIFT_RELEASE || sc == SC_RIGHT_SHIFT_RELEASE) keyboardState.shift = false;
    }
    else {
        if (sc == SC_LEFT_SHIFT || sc == SC_RIGHT_SHIFT) {
            keyboardState.shift = true;
            return;
        }
    }
}
void keyboardInit() {
     
}

const u8 i = 8;

void _start() {
    stdout = open("/dev/stdout"); 

    claim("Keyboard_PS2");
    setFunc("Keyboard_PS2", keyboardHandler);

    const u8 j = i + '0';
    write(stdout, (const char*) &j, 1);
  
    for (;;) ;
}

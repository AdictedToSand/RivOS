#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

typedef uint32_t u32;

typedef u32 fd_t;

extern fd_t open(const char* fp);
extern u8 write(fd_t fd, const char* conts, u32 len);
extern u8 claim(const char* mod);
extern u8 setFunc(const char* mod, void (*)(void));

u32 strlen(const char* s) {
    u32 i = 0;
    while (s[i++]);
    return i - 1;
}

fd_t stdout = 1;

void pitHandler(void) {
    const char* msg = "Hi\n";
    write(stdout, msg, strlen(msg));
}

void _start() {
    stdout = open("/dev/stdout"); 
    const char* msg = "Hello, world!";
    write(stdout, msg, strlen(msg));

    claim("Keyboard_PS2");
    setFunc("Keyboard_PS2", pitHandler);

    for (;;) ;
}

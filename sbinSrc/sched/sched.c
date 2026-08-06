#include <int.h>
#include <stdbool.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "gen/string.h"

#include "rap/rap.h"

typedef struct Str {
    char* cStr;
    u32 len;
    u32 capacity;
} Str;

enum {
    PROC_PRIV_LVL_User,
    PROC_PRIV_LVL_Driver,
    PROC_PRIV_LVL_Kernel,
};

typedef u8 ProcessPriveledgeLevel;

#define VEC(T) struct { \
    T* arr; \
    u32 len; \
    u32 capacity; \
}

typedef struct PhysToVirt {
    void* phys;
    void* virt;
} PhysToVirt;

// MAKE SURE THIS IS SYNCED
typedef struct RegisterState {
    /*
    u32 eip;
    u32 esp;
    u32 ebp;
    Right now, not required, however this should be done later
    */
} RegisterState;

enum {
    PROCIMPT_REQ,
    PROCIMPT_OPT,
};

typedef u8 ProcessImportance;

typedef struct Process {
    pid_t pid;
    const char* pname;
    Str srcFp;
    ProcessPriveledgeLevel priveledge;
    VEC(PhysToVirt) pages;
    VEC(PhysToVirt) stackPages;
    RegisterState state;
    void* entryPoint;
    ProcessImportance importance;
    u32* pageDirectory;
} Process;


void pitHandler(void) {
}

RapFile rap;

typedef void (*PutPixelT)(u32 argb, u32 x, u32 y);
typedef Process* (*LoadProcessT)(const char* fp, const char* pname, ProcessPriveledgeLevel privlvl);

void perPixel(u32 (*calc)(u32 x, u32 y), PutPixelT putPixel, u32 screenWidth, u32 screenHeight) {
    for (u32 x = 0; x < screenWidth; x++) {
        for (u32 y = 0; y < screenHeight; y++) {
            putPixel(calc(x, y), x, y);
        } 
    }
}

u32 calcPixel(u32 x, u32 y) {
   u32 colorx = y > 0 ? x % y : 0;
    u32 colory = x > 0 ? y % x : 0;
    return (colorx ^ colory);
}

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

    PutPixelT putPixel = getRapAddr(&rap, "putPixel");
    if (!putPixel) exit(1);
    u32 (*getScreenWidth)(void) = getRapAddr(&rap, "getScreenWidth");
    u32 (*getScreenHeight)(void) = getRapAddr(&rap, "getScreenHeight");
    LoadProcessT loadProcess = getRapAddr(&rap, "loadProcess");

    loadProcess("/krn/bin/de", "DesktopEnviroment", PROC_PRIV_LVL_Kernel);

    const u32 screenWidth = getScreenWidth();
    const u32 screenHeight = getScreenHeight();

    perPixel(calcPixel, putPixel, screenWidth, screenHeight);

    for (;;) ;
}

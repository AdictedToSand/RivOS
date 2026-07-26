#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "bootutils.h"

typedef enum VgaColor {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
} VgaColor;

typedef struct Terminal {
    size_t cursor;
    volatile uint16_t* vga;
    VgaColor currentTermColor;
} Terminal;
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

Terminal term;

void disableCursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void terminit(void) {
    disableCursor();
    term.cursor = 0;
    term.currentTermColor = VGA_COLOR_WHITE;
    term.vga = (volatile uint16_t*) 0xB8000;

    for (int x = 0; x < VGA_WIDTH; x++) {
        for (int y = 0; y < VGA_HEIGHT; y++) {
            term.vga[x + y * VGA_WIDTH] =  ' ' | 0x0F << 8;
        }
    }
}



void putc(char c) {
    if (c == '\n') {
        term.cursor = (term.cursor / VGA_WIDTH + 1) * VGA_WIDTH;
    }
    else 
        term.vga[term.cursor++] = c | term.currentTermColor << 8;

    if (term.cursor >= VGA_HEIGHT * VGA_WIDTH) {
        term.cursor = 0;
    }
}

void puts(const char* s) {
    while (*s) putc(*s++);
}

// Credits: https://www.geeksforgeeks.org/c/print-long-int-number-c-using-putchar/
void printi(long n) {

    if (n < 0) {
        putc('-');
        n = -n;
    }

    if (n/10) {
        printi(n/10);
    }

    putc(n%10 + '0');
}

// This struct is the inital struct send over by stage2.asm 
// This exists because the kernel is found in real mode, so we can not transfer 32bit values
typedef struct [[gnu::packed]] FoundKernel_initial {
    uint8_t drive;
    uint16_t sector;
    uint16_t sizeAddr;
    uint16_t entryPointAddr;
    uint16_t kernelStartAddr;
    uint16_t osnameAddr;
} FoundKernel_initial;

// This is the actual data of a FoundKernel
typedef struct [[gnu::packed]] FoundKernel {
    uint8_t drive;
    uint16_t sector;
    uint32_t size;
    uint32_t entryPoint;
    uint32_t kernelStart; // Where the kernel wants to be loaded at
    char* osName;
} FoundKernel;

#define SECTOR_SIZE 512

#define ATA_DATAPORT 0x1F0
#define ATA_SECTORCOUNTPORT 0x1F2
#define ATA_LBALOPORT 0x1F3
#define ATA_LBAMIPORT 0x1F4
#define ATA_LBAHIPORT 0x1F5
#define ATA_DRIVESELECTPORT 0x1F6
#define ATA_CMDPORT 0x1F7
#define ATA_STATUSPORT 0x1F7

#define ATA_CMDIDENTIFY 0xEC
#define ATA_CMDREADSECTORS 0x20

#define ATA_DRIVESELMASTER 0xE0
#define ATA_DRIVESELSLAVE 0xF0

#define ATA_STATUSBSY 0x80
#define ATA_STATUSDRQ 0x08

bool ataReadSector(uint16_t* buf, size_t buflen, size_t sector) {
    outb(ATA_DRIVESELECTPORT, ATA_DRIVESELMASTER | ((sector >> 24) & 0x0F));

    // ~400 ns delay
    // For further info, look at https://wiki.osdev.org/ATA_PIO_Mode
    inb(ATA_STATUSPORT);
    inb(ATA_STATUSPORT);
    inb(ATA_STATUSPORT);
    inb(ATA_STATUSPORT);

    outb(ATA_SECTORCOUNTPORT, 1);
    outb(ATA_LBALOPORT, sector & 0xFF);
    outb(ATA_LBAMIPORT, (sector >> 8) & 0xFF);
    outb(ATA_LBAHIPORT, (sector >> 16) & 0xFF);
    outb(ATA_CMDPORT, ATA_CMDREADSECTORS);

    if (inb(ATA_STATUSPORT) == 0) {
        puts("Unable to communcate via drive with ATA_PIO protocol");
        return false;
    }
    while (inb(ATA_STATUSPORT) & 0x80) ;
    while (!(inb(ATA_STATUSPORT) & 0x08)) ;

    for (uint16_t i = 0; i < 256 && i < buflen; i++) {
        buf[i] = inw(ATA_DATAPORT);
    }

    return true;
}

#define PS2_DATAPORT 0x60
#define PS2_STATUSPORT 0x64

#define PS2_NEWLINESC 0x1C

#define PS2_ARROWUP 0x48
#define PS2_ARROWDOWN  0x50

#define CHAR_ARRDOWN '<'
#define CHAR_ARRUP '>'

char scancodeToAscii(char sc) {
    if (sc == PS2_NEWLINESC) return '\n';

    if (sc == PS2_ARROWUP) return CHAR_ARRUP;
    if (sc == PS2_ARROWDOWN) return CHAR_ARRDOWN;

    return '\0';
}

char getc(void) {
    // Wait until output buffer is full (bit 0 set)
    while (!(inb(PS2_STATUSPORT) & 0x01)) ;
    uint8_t scancode = inb(PS2_DATAPORT);
    return scancodeToAscii(scancode); // you'd need a scancode->ascii table
}

static inline uint8_t vgaEntryColor(VgaColor fg, VgaColor bg) {
    return fg | (bg << 4);
}

void displayMenu(FoundKernel* fkrnl) {
    int activeOption = 0;
    while (true) {
        terminit(); // Clear the screen

        puts("Welcome to RivBoot\nAn OS was found: ");
        puts(fkrnl->osName);
        puts("\nPlease select one of the following options: \n");
        term.currentTermColor = (activeOption == 0 ? vgaEntryColor(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY) : VGA_COLOR_WHITE);
        puts("1) boot into "); puts(fkrnl->osName); putc(10);
        term.currentTermColor = (activeOption == 1 ? vgaEntryColor(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY) : VGA_COLOR_WHITE);
        puts("2) reboot\n");
        term.currentTermColor = VGA_COLOR_WHITE;

        char c = getc();

        if (c == '\n') {
            break;
        }

        if (c == CHAR_ARRUP || c == CHAR_ARRDOWN) {
            activeOption = !activeOption;
        }
    }
    if (activeOption == 3) {}
    if (activeOption == 2) asm volatile ("UD2");
}

[[gnu::noreturn]]
void startBoot(FoundKernel_initial* fkrnel_init) {
    terminit();

    FoundKernel fkern;
    fkern.entryPoint = *(uint32_t*)(uintptr_t) fkrnel_init->entryPointAddr;
    fkern.size = *(uint32_t*)(uintptr_t) fkrnel_init->sizeAddr;
    fkern.drive = fkrnel_init->drive;
    fkern.sector = fkrnel_init->sector - 1; // Translates well to ATA_PIO
    fkern.kernelStart = *(uint32_t*)(uintptr_t)  fkrnel_init->kernelStartAddr;
    fkern.osName = *(char**)(uintptr_t) fkrnel_init->osnameAddr;

    const uint32_t kernelSectorCount = ((uint32_t) fkern.size + SECTOR_SIZE - 1) / SECTOR_SIZE;

    uint16_t* sectorbuf = (uint16_t*) fkern.kernelStart;

    for (size_t i = 0; i < kernelSectorCount; i++) {
        ataReadSector(sectorbuf, 256, fkern.sector++);

        sectorbuf += 256; // sizeof(sector)
    }

    displayMenu(&fkern);

    void (*kernelEntry)(void) = (void (*)(void)) fkern.entryPoint;
    kernelEntry();

    __builtin_unreachable();    
}

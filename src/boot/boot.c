#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "bootutils.h"

typedef struct Terminal {
    size_t cursor;
    volatile uint16_t* vga;
} Terminal;
#define VGA_HEIGHT 25
#define VGA_WIDTH 80

Terminal term;

void terminit(void) {
    term.cursor = 0;
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
        term.vga[term.cursor++] = c | 0x0F << 8;

    if (term.cursor > VGA_HEIGHT * VGA_WIDTH) {
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
} FoundKernel_initial;

// This is the actual data of a FoundKernel
typedef struct [[gnu::packed]] FoundKernel {
    uint8_t drive;
    uint16_t sector;
    uint32_t size;
    uint32_t entryPoint;
    uint32_t kernelStart; // Where the kernel wants to be loaded at
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

[[gnu::noreturn]]
void startBoot(FoundKernel_initial* fkrnel_init) {
    terminit();

    FoundKernel fkern;
    fkern.entryPoint = *(uint32_t*)(uintptr_t) fkrnel_init->entryPointAddr;
    fkern.size = *(uint32_t*)(uintptr_t) fkrnel_init->sizeAddr;
    fkern.drive = fkrnel_init->drive;
    fkern.sector = fkrnel_init->sector - 1; // Translates well to ATA_PIO
    fkern.kernelStart = *(uint32_t*)(uintptr_t)  fkrnel_init->kernelStartAddr;

    const uint32_t kernelSectorCount = ((uint32_t) fkern.size + SECTOR_SIZE - 1) / SECTOR_SIZE;

    uint16_t* sectorbuf = (uint16_t*) fkern.kernelStart;

    for (size_t i = 0; i < kernelSectorCount; i++) {
        ataReadSector(sectorbuf, 256, fkern.sector++);

        sectorbuf += 256; // sizeof(sector)
    }

    void (*kernelEntry)(void) = (void (*)(void)) fkern.entryPoint;
    kernelEntry();

    __builtin_unreachable();    
}













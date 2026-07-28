// Credits: https://wiki.osdev.org/Setting_Up_Paging
#pragma once
#include <stdint.h>

#include <mem/utils.hpp>
#include <mem/pagealloc.hpp>

static inline uint32_t pageDirectory[1024] __attribute__((aligned(4096)));
static inline uint32_t firstPageTable[1024] __attribute__((aligned(4096)));

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

struct Mmu {
    static auto init() -> void {
        for (uint16_t i = 0; i < 1024; i++) {
            // This sets the following flags to the pages:
            //   Supervisor: Only kernel-mode can access them
            //   Write Enabled: It can be both read from and written to
            //   Not Present: The page table is not present
            pageDirectory[i] = 0x00000002;
        } 

        for (uint16_t i = 0; i < 1024; i++) {
            firstPageTable[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
        }
        // attributes: supervisor level, read/write, present
        pageDirectory[0] = ((unsigned int) firstPageTable) | 3;

        loadPageDirectory((unsigned int*) pageDirectory); // No idea why the fuck the typecast is neccesary
        enablePaging();
    }

    static constexpr uint32_t PAGE_PRESENT = 0x1;
    static constexpr uint32_t PAGE_WRITE = 0x2;
    static constexpr uint32_t PAGE_USER = 0x4;

    // Credits: https://forum.osdev.org/viewtopic.php?t=56183
    static void mapPage(void* physaddr, void* virtualaddr, unsigned int flags) {
        unsigned long pdindex = (unsigned long) virtualaddr >> 22;
        unsigned long ptindex = (unsigned long) virtualaddr >> 12 & 0x03FF;

        unsigned long* pd = (unsigned long*) 0xFFFFF000;
        unsigned long* pt = ((unsigned long*) 0xFFC00000) + (0x400 * pdindex);

        // PDE not present -> allocate a new page table for this 4MB region
        if (!(pd[pdindex] & PAGE_PRESENT)) {
            uint32_t newPtPhys = PhysicalMemoryManager::physAlloc();
            // Write the PDE first — pt only resolves via recursive mapping once this exists
            pd[pdindex] = newPtPhys | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
            memset(pt, 0, 4096); // now safe: pt is reachable through the recursive trick
        } else if (flags & PAGE_USER) {
            // PDE already existed (e.g. from an earlier kernel-only mapping) but this
            // mapping needs PAGE_USER — OR it in, don't silently leave it kernel-only
            pd[pdindex] |= PAGE_USER;
        }

        // Overwrite policy: last mapPage call for a given virtual address wins.
        // If you want to catch accidental double-maps instead, assert here:
        // if (pt[ptindex] & PAGE_PRESENT) panic("remap of already-mapped page");

        pt[ptindex] = ((unsigned long) physaddr) | (flags & 0xFFF) | PAGE_PRESENT;

        asm volatile("invlpg (%0)" :: "r"(virtualaddr) : "memory");
    }
};

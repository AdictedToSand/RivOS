// Credits: https://wiki.osdev.org/Setting_Up_Paging
#pragma once
#include <stdint.h>

#include <mem/utils.hpp>

#include <gen/err.hpp>

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

struct Mmu {
    static constexpr u32 FLAGS_PRESENT = 0x001;
    static constexpr u32 FLAGS_WRITABLE = 0x002;
    static constexpr u32 FLAGS_USER = 0x004;

    static inline uint32_t pageDirectory[1024] __attribute__((aligned(4096)));
    static inline uint32_t firstPageTable[1024] __attribute__((aligned(4096)));

    // Temporary page table pool until the physical allocator is ready
    static inline uint32_t pageTables[16][1024] __attribute__((aligned(4096)));
    static inline uint32_t nextPageTable = 0;

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

    static auto mapPage(void* phys, void* virt, uint32_t flgs) -> void {
        uint32_t physical = (uint32_t) phys;
        uint32_t virtualAddr = (uint32_t) virt;

        uint32_t directoryIndex = virtualAddr >> 22;
        uint32_t tableIndex = (virtualAddr >> 12) & 0x3FF;

        uint32_t* pageTable;

        if (!(pageDirectory[directoryIndex] & 1)) {
            if (nextPageTable >= 16) {
                kpanic("No more pagetables: Should fix this"); // No more temporary page tables available
            }

            pageTable = pageTables[nextPageTable++];

            for (uint16_t i = 0; i < 1024; i++) {
                pageTable[i] = 0x00000002;
            }

            pageDirectory[directoryIndex] = ((uint32_t) pageTable) | 3;
        } else {
            pageTable = (uint32_t*)(pageDirectory[directoryIndex] & 0xFFFFF000);
        }

        pageTable[tableIndex] = (physical & 0xFFFFF000) | flgs | 1;

        // Flush this virtual address from the TLB
        asm volatile("invlpg (%0)" :: "r"(virtualAddr) : "memory");
    }
};

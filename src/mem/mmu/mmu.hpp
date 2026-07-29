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

    static auto mapPage(void* phys, void* virt, uint32_t flgs) -> void {

    }
};

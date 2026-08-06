// Credits: https://wiki.osdev.org/Setting_Up_Paging
#pragma once
#include <stdint.h>

#include <mem/utils.hpp>

#include <gen/err.hpp>

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

extern "C" char stack_bottom[];
extern "C" char stack_top[];

struct Mmu {
    static constexpr u32 FLAGS_PRESENT = 0x001;
    static constexpr u32 FLAGS_WRITABLE = 0x002;
    static constexpr u32 FLAGS_USER = 0x004;

    static inline u32 pageDirectory[1024] __attribute__((aligned(4096)));
    static inline u32 firstPageTable[1024] __attribute__((aligned(4096)));

    static inline u32 pageTables[16][1024] __attribute__((aligned(4096)));
    static inline u32 nextPageTable = 0;

    static inline u32 processDirectories[16][1024] __attribute__((aligned(4096)));
    static inline u32 nextDirectory = 0;

    static inline u32* activeDirectory = nullptr;

    static auto init() -> void {
        for (u16 i = 0; i < 1024; i++) {
            pageDirectory[i] = 0x00000002;
        } 

        for (u16 i = 0; i < 1024; i++) {
            firstPageTable[i] = (i * 0x1000) | 3;
        }
        pageDirectory[0] = ((unsigned int) firstPageTable) | 3;

        activeDirectory = pageDirectory;

        loadPageDirectory((unsigned int*) pageDirectory);
        enablePaging();
    }

    static auto createAddressSpace() -> uint32_t* {
        if (nextDirectory >= 16) {
            kpanic("No more process address spaces: Should fix this");
        }
        uint32_t* dir = processDirectories[nextDirectory++];
        for (u16 i = 0; i < 1024; i++) {
            dir[i] = pageDirectory[i]; // inherit the kernel's current mappings
        }

        for (u32 addr = (u32) stack_bottom; addr < (u32) stack_top; addr += 0x1000) {
            mapPageIn(dir, (void*) addr, (void*) addr, FLAGS_WRITABLE);
        }

        return dir;
    }

    static auto switchAddressSpace(u32* dir) -> void {
        if (dir == activeDirectory) return; // no-op, also avoids a pointless TLB flush
        activeDirectory = dir;
        loadPageDirectory((unsigned int*) dir);
    }

    static auto mapPageIn(u32* dir, void* phys, void* virt, u32 flgs) -> void {
        u32 physical = (uint32_t) phys;
        u32 virtualAddr = (uint32_t) virt;

        u32 directoryIndex = virtualAddr >> 22;
        u32 tableIndex = (virtualAddr >> 12) & 0x3FF;

        u32* pageTable;

        if (!(dir[directoryIndex] & 1)) {
            if (nextPageTable >= 16) {
                kpanic("No more page tables: Should fix this");
            }

            pageTable = pageTables[nextPageTable++];

            for (u16 i = 0; i < 1024; i++) {
                pageTable[i] = 0x00000002;
            }

            dir[directoryIndex] = ((uint32_t) pageTable) | 3;
        }
        else {
            pageTable = (u32*) (dir[directoryIndex] & 0xFFFFF000);
        }

        pageTable[tableIndex] = (physical & 0xFFFFF000) | flgs | 1;

        if (dir == activeDirectory) {
            asm volatile("invlpg (%0)" :: "r"(virtualAddr) : "memory");
        }
    }

    static auto mapPage(void* phys, void* virt, u32 flgs) -> void {
        mapPageIn(pageDirectory, phys, virt, flgs);
    }

    static auto unmapPage(void* virt) -> void {
        // unchanged
        u32 virtualAddr = (u32) virt;

        u32 directoryIndex = virtualAddr >> 22;
        u32 tableIndex = (virtualAddr >> 12) & 0x3FF;

        if (!(pageDirectory[directoryIndex] & FLAGS_PRESENT))
            return;

        u32* pageTable = (u32*) (pageDirectory[directoryIndex] & 0xFFFFF000);

        pageTable[tableIndex] = 0;

        asm volatile("invlpg (%0)" :: "r"(virtualAddr) : "memory");
    }
};

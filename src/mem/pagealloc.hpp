#pragma once
#include <stdint.h>
#include <cstring.hpp>

#include <mem/utils.hpp>

#define PAGE_SIZE 4096

class PhysicalMemoryManager {
private:
    static inline uint32_t* bitmap;
    static inline size_t totalFrames;
    static inline size_t lastAllocatedIndex = 0;
    static inline auto setBit(size_t frame) -> void {
        bitmap[frame / 32] |= (1u << (frame % 32));
    }
    static inline auto clearBit(size_t frame) -> void {
        bitmap[frame / 32] &= ~(1u << (frame % 32));
    }
    static inline auto testBit(size_t frame) -> bool {
        return bitmap[frame / 32] & (1u << (frame % 32));
    }

public:
    // bitmapAddr: physical/identity-mapped address to store the bitmap
    // memSizeBytes: total usable physical memory reported by your memory map
    static auto init(void* bitmapAddr, size_t memSizeBytes) -> void {
        totalFrames = memSizeBytes / PAGE_SIZE;
        bitmap = (uint32_t*) bitmapAddr;
        memset(bitmap, 0, (totalFrames / 32) + 1); // 0 = all free initially
    }

    // Call this for every region you must NOT hand out: kernel image,
    // the bitmap itself, reserved/MMIO regions from your memory map, etc.
    static void markRegionUsed(uintptr_t physAddr, size_t sizeBytes) {
        size_t startFrame = physAddr / PAGE_SIZE;
        size_t frameCount = (sizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
        for (size_t i = 0; i < frameCount; i++) {
            setBit(startFrame + i);
        }
    }

    static uint32_t physAlloc() {
        for (size_t i = 0; i < totalFrames; i++) {
            size_t idx = (lastAllocatedIndex + i) % totalFrames;
            if (!testBit(idx)) {
                setBit(idx);
                lastAllocatedIndex = idx;
                return idx * PAGE_SIZE;
            }
        }
        return 0;
    }

    static void physFree(uint32_t physAddr) {
        clearBit(physAddr / PAGE_SIZE);
    }
};

inline uint32_t physAlloc() { return PhysicalMemoryManager::physAlloc(); }

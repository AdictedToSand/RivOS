#pragma once
#include <mem/utils.hpp>

#include <int.h>

struct PhysicalFrameAllocator {
    static inline u8* bitmap;
    static inline u32 totalFrames;

public:
    static constexpr u32 FRAME_SIZE = 4096;

    static void init(u32 memSizeBytes, u8* bitmapStorage) {
        totalFrames = memSizeBytes / FRAME_SIZE;
        memset(bitmapStorage, 0, (totalFrames + 7) / 8); // 0 = free
        bitmap = bitmapStorage;
    }

    static bool isUsed(u32 i) { return bitmap[i / 8] & (1 << (i % 8)); }
    static void markUsed(u32 i) { bitmap[i / 8] |= (1 << (i % 8)); }
    static void markFree(u32 i) { bitmap[i / 8] &= ~(1 << (i % 8)); }

    static void* allocFrame() {
        for (u32 i = 0; i < totalFrames; i++) {
            if (!isUsed(i)) { markUsed(i); return (void*) (i * FRAME_SIZE); }
        }
        return nullptr; // out of physical memory
    }

    static void freeFrame(void* addr) {
        markFree((u32) addr / FRAME_SIZE);
    }
};

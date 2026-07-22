#pragma once
#include <stddef.h>

extern "C" char heapStart[];
extern "C" char heapEnd[];

struct [[gnu::packed ]] /* Maximise heap size */ HeapBlock {
    size_t size;
    bool inUse;

    HeapBlock* next;
};

struct KernelAllocator {
    static inline HeapBlock* superBlock;

    static auto init() -> void {
        superBlock = (HeapBlock*) heapStart;
    
        superBlock->size = 0;
        superBlock->inUse = true;
        superBlock->next = nullptr;
    }

    static auto alloc(size_t bytes) -> void* {
        HeapBlock* currentBlock = superBlock;

        while (true) {
            if (!currentBlock->inUse && currentBlock->size >= bytes) {
                return currentBlock + 1;
            }
            if (currentBlock->next == nullptr) {
                HeapBlock* const next = (HeapBlock*) ((char*) currentBlock + currentBlock->size);

                if ((char*) next > heapEnd) {
                    return nullptr;
                }

                next->inUse = true;
                next->next = nullptr;
                next->size = bytes;

                currentBlock->next = next;
            
                return next + 1;
            }

            currentBlock = currentBlock->next;
        }

        return nullptr;
    }

    static auto free(void* mem) -> void {
        ((HeapBlock*) ((HeapBlock*) mem - 1))->inUse = false;
    }
};

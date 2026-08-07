#pragma once
#include <stddef.h>

#include <terminal/terminal.hpp>

extern "C" char heapStart[];
extern "C" char heapEnd[];

struct [[gnu::packed]] /* Maximise heap size */ HeapBlock {
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
                currentBlock->inUse = true;
                return currentBlock + 1;
            }
            if (currentBlock->next == nullptr) {
                HeapBlock* const next = (HeapBlock*) ((char*) currentBlock + sizeof(HeapBlock) + currentBlock->size);

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

        Terminal::printf("HeapAllocator out of memory");
        return nullptr;
    }

    static auto free(void* mem) -> void {
        ((HeapBlock*) ((HeapBlock*) mem - 1))->inUse = false;
    }
};

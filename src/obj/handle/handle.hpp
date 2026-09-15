#pragma once
#include <int.h>
#include <tsame.hpp>

#include <mem/alloc.hpp>

#include <gen/serial.hpp>
#include <gen/err.hpp>

//TODO: Move to src/obj/mcls.hpp
struct Writer {
private:
    void* addr;
    u32 len;
public:
    Writer(void* iaddr, u32 ilen) : addr(iaddr), len(ilen) {}

    // @Returns false if operation fails (Because its OOB)
    auto write(char* datastream, u32 ilen) -> bool {
#ifdef DEBUG
        if (ilen > len) Serial::logf("OOB write in Writer: %u but max is %u", ilen, len);
#endif
        for (u32 i = 0; i < ilen && i < len; i++) {
            *((char*) addr + i) = datastream[i];
        }
        if (ilen > len) return false;
        return true;
    }
};
struct Reader {
private:
    void* addr;
    u32 len;
public:
    Reader(void* iaddr, u32 ilen) : addr(iaddr), len(ilen) {}

    // @Returns false if operation fails (Prolly because its OOB)
    auto read(Writer& out, u32 ilen) -> bool {
        if (ilen > len) {
        #ifdef DEBUG
            Serial::logf("OOB read in Reader: %u but max is %u", ilen, len);           
        #endif
            out.write((char*) addr, len);
            return false;
        }
        return out.write((char*) addr, ilen);
    }
};

template<typename T>
struct Handle {
private:
    struct [[gnu::packed]] SharedData {
        u32 rcount;
        u32 allocatorInfo_size; // Not used on all Handles but it's only 4 bytes :/
        T* usrData;
    };
    SharedData* shrdData = nullptr; 
#ifdef DEBUG
    const char* hndlName;
#endif
    auto free() -> void {
        KernelAllocator::free(shrdData->usrData);
        KernelAllocator::free(shrdData);
    }
    auto assrtRcount() -> bool {
        if (shrdData->rcount == 0) { 
            free();
#ifdef DEBUG
            Serial::logf("Handle was freed (%s)", hndlName);
#endif
            shrdData->usrData = nullptr;
            return false;
        }
        return true;
    }
public:
    struct Allocator {};
    Handle() : shrdData((SharedData*) KernelAllocator::alloc(sizeof(SharedData))) {
        static_assert(!TypeIsSame<T, Allocator>::value, "Please use the Handle(u32 allocsize) constructor");
        shrdData->rcount = 1; // This object
        shrdData->allocatorInfo_size = 0;
        shrdData->usrData = (T*) KernelAllocator::alloc(sizeof(T));
    }
    Handle(u32 allocsize) : shrdData((SharedData*) KernelAllocator::alloc(sizeof(SharedData))){
        shrdData->allocatorInfo_size = allocsize;
        shrdData->rcount = 1;
        shrdData->usrData = (T*) KernelAllocator::alloc(allocsize / sizeof(Allocator));
        static_assert(TypeIsSame<T, Allocator>::value, "Can only use Handle(u32 allocsize) constructor when type == HandleAllocator, please use Handle() instd.");
    }
    auto setName(const char* name) -> void {
#ifdef DEBUG
        hndlName = name;
#else
        (void) name;
#endif
    }

    auto getDataWriter() -> Writer {
        // wtf why is this in C as _Static_assert
        static_assert(TypeIsSame<T, Allocator>::value, "getDataWriter doesn't allow for types other than HandleAllocator");
        return Writer(shrdData->usrData, shrdData->allocatorInfo_size);
    }
    auto getDataReader() -> Reader {
        static_assert(TypeIsSame<T, Allocator>::value, "getDataReader doesn't allow for types other than HandleAllocator");    
        return Reader(shrdData->usrData, shrdData->allocatorInfo_size);
    }
    auto getPtr() -> T* {
        if (!shrdData->usrData) kpanic("NULL usage of shrdData->usrData in handle");
        return shrdData->usrData;
    }

    ~Handle() {
        release();
    }

    Handle(const Handle& other) : shrdData(other.shrdData) {
        shrdData->rcount++;
    }
    Handle& operator=(const Handle& other) {
        if (this == &other) return *this;

        shrdData = other.shrdData;
        shrdData->rcount++;
        return *this;
    }
    auto release() -> void {
        shrdData->rcount--; 
        assrtRcount();
    }
};
using HandleAllocator = Handle<void>::Allocator;

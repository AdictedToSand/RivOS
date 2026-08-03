#pragma once
#include <drivers/fs/fs.hpp>

#include <mem/page.hpp>
#include <mem/mmu/mmu.hpp>
#include <mem/alloc.hpp>

#include <int.h>

#include <proc/process.hpp>

struct [[gnu::packed]] ElfHeader32 {
    char magic[4];
    u8 bitness; // 1 == 32 bit 2 == 64 bit
    u8 endianness; // 1 == little endian 2 == big endian
    u8 headerVersion;
    u8 osAbi; // Should be 0 for systemV
    u8 abiVersion;
    u8 padding[7];
    u16 type; // Should be 2 for executable
    u16 instructionSet;
    u32 version; // General ELF version
    u32 programEntry;
    u32 programHeaderTableOffset;
    u32 sectionHeaderTableOffset;
    u32 flags;
    u16 headerSize;
    u16 programHeaderEntrySize;
    u16 programHeaderTableSize;
    u16 sectionHeaderTableEntrySize;
    u16 entriesInSectionHeaderTable;
    u16 sectionIndexToSectionHeaderStringTable;
};

struct ElfExecutable {
private:
    static constexpr unsigned char ELF_MAGIC[] = {0x7F, 'E', 'L', 'F'};

    ElfHeader32* hdr;
    u8* postHeaderElf;
    const char* fp;
    Process* selfProc;

    static constexpr u8 BITNESS_32 = 1;
    static constexpr u8 BITNESS_64 = 2;

    static constexpr u8 ENDIAN_LIL = 1;
    static constexpr u8 ENDIAN_BIG = 2;

    static constexpr u8 VERSION_CURRENT = 1;

    static constexpr u8 OSABI_SYSTEMV = 0;
    static constexpr u8 OSABI_VERSCURRENT = 0;

    static constexpr u8 TYPE_EXECUTABLE = 2;

    static constexpr u8 INSTRSET_X86 = 0x03;

    struct [[gnu::packed]] ProgramHeader {
        static constexpr u32 SEGMENTTYPE_LOAD = 1; // Currently only this is supported

        u32 segmentKind;
        u32 contentsOffset;
        u32 virtualAddrStart;
        u32 physicalPosition; // This is unused: userspace should not control this
        u32 segmentFilesize;
        u32 memSize;
        u32 flags;
        u32 requiredAllignment;
    };
public:
    auto isValid() -> bool {
        if (memcmp(hdr->magic, ELF_MAGIC, 4)) return false;
        if (hdr->bitness != BITNESS_32) return false;
        if (hdr->endianness != ENDIAN_LIL) return false;
        if (hdr->version != VERSION_CURRENT) return false;

        if (hdr->osAbi != OSABI_SYSTEMV) return false;
        if (hdr->abiVersion != OSABI_VERSCURRENT) return false;

        if (hdr->type != TYPE_EXECUTABLE) return false;
        
        if (hdr->instructionSet != INSTRSET_X86) return false;
        if (hdr->version != VERSION_CURRENT) return false;

        if (hdr->programEntry == 0) return false;

        return true;
    }
    auto load(const char* const pname, const ProcessPriveledgeLevel priveledge) -> Process* {
        Process* proc = (Process*) KernelAllocator::alloc(sizeof(Process));
        proc->srcFp = fp;
        proc->pid = getNewPid();
        proc->pname = pname;
        proc->priveledge = priveledge;

        selfProc = proc;

        const u8* const base = (u8*) hdr;
        ProgramHeader* phdrs = (ProgramHeader*) (base + hdr->programHeaderTableOffset);

        for (u16 i = 0; i < hdr->programHeaderTableSize; i++) {
            ProgramHeader* ph = (ProgramHeader*) ((u8*) phdrs + i * hdr->programHeaderEntrySize);

            if (ph->segmentKind == ProgramHeader::SEGMENTTYPE_LOAD) {
                const u32 vaddrBase = ph->virtualAddrStart & ~0xFFF;
                const u32 vaddrEnd = (ph->virtualAddrStart + ph->memSize + 4095) & ~0xFFF;
                const u32 pageCount = (vaddrEnd - vaddrBase) / 4096;
                u8* const dest = (u8*) ph->virtualAddrStart;

                for (u32 j = 0; j < pageCount; j++) {
                    void* const frame = PhysicalFrameAllocator::allocFrame();
                    if (!frame) { kpanic("Out of memory"); }
                    Mmu::mapPage(frame, (void*) (vaddrBase + j * 4096), Mmu::FLAGS_PRESENT | Mmu::FLAGS_WRITABLE);
                }

                memcpy(dest, base + ph->contentsOffset, ph->segmentFilesize);
                memset(dest + ph->segmentFilesize, 0, ph->memSize - ph->segmentFilesize);
            }
            else {
                continue;
            }
        }
        proc->entryPoint = (void*) hdr->programEntry;

        free(); // We don't need the ELF anymore at this point.
        return proc;
    }
    auto fromFile(const char* ifp) -> u8 {
        fp = ifp;
        fd_t fd = FileSystem::open(fp);
        if (!fd) {
            return 1;
        }
        char* const tmpbuf = (char*) KernelAllocator::alloc(FileSystem::fileSize(fd));

        if (FileSystem::read(fd, tmpbuf, FileSystem::fileSize(fd)) == FsSuccessCodes::Error) {
            return 1;
        }
        hdr = (ElfHeader32*) tmpbuf;
        postHeaderElf = (u8*) (hdr + 1); // + 1 == + 1 * sizeof(ElfHeader32)

        return 0;
    }
    auto free() -> void {
        KernelAllocator::free(hdr);
    }
};

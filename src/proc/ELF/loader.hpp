#pragma once
#include <int.h>

struct [[gnu::packed]] ElfHeader32 {
    static constexpr char ELF_MAGIC[] = {0x7F, 'E', 'L', 'F'};
    
private:
    char magic[4];
    u8 bitness; // 1 == 32 bit 2 == 64 bit
    // 1 == little endian 2 == big endian
    

public:
};

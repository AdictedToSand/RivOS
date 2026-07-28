// Credits: https://wiki.osdev.org/APIC
#pragma once
#include <stdint.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define CPUID_FEAT_EDX_APIC (1 << 9)

inline void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0) // ecx=0 default (matters for leafs like 4/7)
    );
}

inline void cpuSetMSR(uint32_t msr, uint32_t eax, uint32_t edx) {
    asm volatile(
        "wrmsr"
        :
        : "a"(eax), "d"(edx), "c"(msr)
    );
}

struct Apic {
    static auto checkApic() -> bool {
        uint32_t eax, ebx, ecx, edx;
        cpuid(1, &eax, &ebx, &ecx, &edx);
        return edx & CPUID_FEAT_EDX_APIC;
    }
    static auto setBase(uintptr_t apic) -> void {
        uint32_t edx = 0, eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
        // This should be defined if we're on 64 bits.
        #ifdef __PHYSICAL_MEMORY_EXTENSION__
            edx = (apic >> 32) & 0x0f;
        #endif
        cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
    }
    static auto init() -> void {
        
    }
};

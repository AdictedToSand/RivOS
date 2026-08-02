#pragma once

enum class BootloaderKinds {
    RivBoot,
    GRUB,
};

struct BootInfo {
    BootloaderKinds bootloader;
};

static inline BootInfo bootinfo; // This is guaranteed to be set in kernelMain

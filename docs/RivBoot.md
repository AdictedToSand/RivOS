# The RivBoot bootloader protocol

The RivBoot bootloader is a bootloader made for RivOS. However the bootloader can be extended for other OS's

## Header

The header you might see in the RivOS kernel itself is:

~~~NASM

section .header align=1

%define ABI_VERSION(major, minor) (((major) << 16) | (minor))


initialMagic:
    db "BOOTABLE", 0     ; Magic required for RivBoot to see the kernel
    dd ABI_VERSION(0, 2) ; Version of RivBoot to use
    dd _start            ; Tells RivBoot where to jump to
    dd kernelSize        ; Tells RivBoot how big the kernel is
    dd kernelStart       ; Tells RivBoot where to start loading the kernel
    ; This value should be > 0x10000
    ; (Because thats where the bootloader lives)
    dd osName

    dd bssStart
    dd bssEnd
    ; Future fields for later versions os RivBoot should be added here!

osName:
    db "RivOS", 0

~~~

Let's walk trough it step by step:

### section

The section .header (name is kinda vague) is a section that should be placed at the very beginning of a sector (no exceptions). This is because when RivBoot looks for an OS to load, it very simply just checks at the start of a sector: does this contain "BOOTABLE\0".

### Macro

The macro is a tiny helper for making the version ABI more readable. The bitshifting magic comes down to:
major version: first 16 bits. The reason it's parenthesised is operator precedence: OR before SHL. An example layout:
0000000000000000_0000000000000001 (This stands for version 0.1 and is used in the current RivOS implementation.)

### Bootable string

This is the magic required for RivBoot to recognize the kernel. It should be null terminated like a C string.

### Abi version

The ABI version, at a very minimum should be 0.1. The 0.0 version didn't even have a version header!

### Entry point

The entry point. RivBoot will jump to this point when the kernel is loaded. It is important to note that because of this, your linker.ld does not need a ENTRY(_start).

### Kernel size

The size of the kernel. Should be calculated by a linker and be a 32bit unsigned integer. This is calculated in bytes, not sectors.

### Kernel start

Not to confuse with _start, this defines where the start of the kernel will be loaded. It is good practice to keep this value > 0x10000 because anywhere below that RivBoot could be loaded.

### OsName

A pointer to the name of the OS (This should be a C string.). This is a technically optional field, but please don't.

(Note that this is NOT represented in C as a char* but a char**.)

### BssStart

Start of the bss section that RivBoot will zero out. This is a version 0.2 extension.

### BssEnd

End of the bss section. This is a version 0.2 extension.

### Others

Further fields may be added in later versions of RivBoot.

## Start of _start
The start of _start should be something like:

~~~Nasm

    MOV esp, stackTop
    CALL kernelMain 
~~~

(Or at least a single instruction using esp and then a call. This is very much a bug in RivBoot and is planned to be fixed.)

## Downloading/Using RivBoot

Currently there is no RivBoot github repo, if you want to use it go to src/boot. 

## Compatability

### Requirement

- BIOS (or at least BIOS emulation)
- A Hard drive

## Plans for later

- Currently, RivBoot is pretty restricting on the hardware side. This is the first thing that RivBoot will change. In order of priority:
    - UEFI
    - More storage devices
With UEFI, the kernel will still operator in 32bit mode.

- Support for multiple headers. The headers RivBoot wants to support are:
    - Multiboot 1
    - Multiboot 2
    - The Linux/x86 Boot Protocol

- Support for a FAT32 filesystem. (Even if only readonly.) 

These features may not arrive fast, but are planned.

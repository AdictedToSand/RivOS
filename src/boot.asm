extern kernelSize
extern kernelStart
extern kernelMain
extern bssStart
extern bssEnd

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

section .bss

align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

section .multiboot

align 8
multiboot_header:
    dd 0xE85250D6        ; magic
    dd 0                 ; architecture i386
    dd multiboot_end - multiboot_header
    dd -(0xE85250D6 + 0 + (multiboot_end - multiboot_header))

    ; framebuffer tag
    align 8
    dw 5                 ; framebuffer tag type
    dw 0                 ; flags
    dd 20                ; size
    dd 1024              ; width
    dd 768               ; height
    dd 32                ; depth

    ; end tag
    align 8
    dw 0
    dw 0
    dd 8

multiboot_end:
; The linker script specifies _start as the entry point to the kernel and the
; bootloader will jump to this position once the kernel has been loaded. It
; doesn't make sense to return from this function as the bootloader is gone.

section .text

global _start


_start:
    ; Right now, RivBoot makes no guarantee that a proper stack is set up. So we define our own
    MOV esp, stack_top

    PUSH ebx
    PUSH eax
    CALL kernelMain

    ; If the system has nothing more to do, put the computer into an
    ; infinite loop. To do that:
    ; 1) Disable interrupts with cli (clear interrupt enable in eflags).
    ;    They are already disabled by the bootloader, so this is not needed.
    ;    Mind that you might later enable interrupts and return from
    ;    kernelMain (which is sort of nonsensical to do).
    ; 2) Wait for the next interrupt to arrive with hlt (halt instruction).
    ;    Since they are disabled, this will lock up the computer.
    ; 3) Jump to the hlt instruction if it ever wakes up due to a
    ;    non-maskable interrupt occurring or due to system management mode.

    CLI

.hang:
    HLT
    JMP .hang


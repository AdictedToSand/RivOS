BITS 32

global gdtFlush

section .data
align 8
gdtStart:
    ; null descriptor
    dq 0x0000000000000000
    ; kernel code segment: base=0, limit=0xFFFFF, 4KB gran, 32-bit, ring0
    dq 0x00CF9A000000FFFF
    ; kernel data segment
    dq 0x00CF92000000FFFF
gdtEnd:

gdtr:
    dw gdtEnd - gdtStart - 1   ; limit
    dd gdtStart                 ; base

section .text
gdtFlush:
    lgdt [gdtr]
    mov ax, 0x10        ; data selector (2nd entry * 8)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush      ; far jump to reload CS with code selector
.flush:
    ret

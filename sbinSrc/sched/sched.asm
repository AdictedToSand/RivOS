BITS 32
global _start

section .text
_start:
     MOV word [0xB8000], 0x0F41
.hlt:
    HLT
    JMP .hlt

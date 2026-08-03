BITS 32

global claim
global open
global write
global setFunc
global release
global mmap
global munmap
global exit
global read
global close

open:
    MOV edi, dword [esp+4]
    XOR eax, eax
    INT 0x80
    RET

write:
    MOV edi, dword [esp+4]
    MOV esi, dword [esp+8]
    MOV edx, dword [esp+12]

    MOV eax, 1
    INT 0x80
    RET

read:
    MOV edi, dword [esp+4]
    MOV esi, dword [esp+8]
    MOV edx, dword [esp+12]

    MOV eax, 2
    INT 0x80
    RET

close:
    MOV edi, dword [esp+4]
    MOV eax, 3
    INT 0x80
    RET

claim:  
    MOV edi, dword [esp+4]
    MOV eax, 4
    INT 0x80
    RET

setFunc:
    MOV edi, dword [esp+4]
    MOV esi, dword [esp+8]
    MOV eax, 5
    INT 0x80
    RET

release:
    MOV edi, dword [esp+4]
    MOV eax, 6
    INT 0x80
    RET
mmap:
    MOV edi, dword [esp+4]
    MOV eax, 7
    INT 0x80
    RET
munmap: 
    MOV edi, dword [esp+4]
    MOV eax, 8
    INT 0x80
    RET
exit:   
    MOV edi, dword [esp+4]
    MOV eax, 9
    INT 0x80
    RET

section .rodata

conts: db "Hello, world!", 10, 0
contsLen equ $ - conts
fp: db "/dev/stdout", 0

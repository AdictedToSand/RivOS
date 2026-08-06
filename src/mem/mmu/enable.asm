section .text
BITS 32
global loadPageDirectory
global enablePaging

extern stack_top

loadPageDirectory:
    PUSH ebp
    MOV ebp, esp
    MOV eax, [esp+8]
    MOV cr3, eax
    MOV esp, ebp
    POP ebp
    RET

enablePaging:
    PUSH ebp
    MOV ebp, esp
    MOV eax, cr0
    OR eax, 0x80000000
    MOV cr0, eax
    MOV esp, ebp
    POP ebp
    RET

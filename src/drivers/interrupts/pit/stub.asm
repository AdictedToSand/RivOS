BITS 32

section .bss
align 16
pitStackBottom: resb 4096
pitStackTop:

section .text
global pitStub
extern pitHandler

pitStub:
    PUSHA
    MOV eax, esp ; save the interrupted esp
    MOV esp, pitStackTop  ; switch onto our own scratch stack
    PUSH eax ; keep the interrupted esp so we can restore it
    CALL pitHandler
    POP eax
    MOV esp, eax ; restore the interrupted esp
    POPA
    IRETD

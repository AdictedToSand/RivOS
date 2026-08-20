section .bss
align 16
global syscallStackBottom
global syscallStackTop
syscallStackBottom: resb 16384
syscallStackTop:

section .text
global syscallStub
extern syscallHandler

syscallStub:
    PUSHA
    MOV eax, esp
    MOV esp, syscallStackTop
    PUSH eax ; save the interrupted esp so we can restore it after

    PUSH eax ; Push the esp of the process, not the current esp
    CALL syscallHandler
    ADD esp, 4

    POP eax ; restore the interrupted esp
    MOV esp, eax
    POPA
    IRETD

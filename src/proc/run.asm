BITS 32
global finalRun
; Does the final part of the work before jumping to the program
; Arguments:
;   - Program entry point
;   - Program stack start
finalRun:
    MOV eax, [esp+4] ; entry point
    MOV ecx, [esp+8] ; stack start
    ;JMP eax
    MOV esp, ecx ; switch to the process's stack
    JMP eax ; never returns -- no RET needed

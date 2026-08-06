BITS 32
global finalRun
; Does the final part of the work before jumping to the program
; Arguments:
;   - Program entry point
;   - Program stack start
;   - Page directory (physical address) to switch into
finalRun:
    MOV eax, [esp+4]  ; entry point
    MOV ecx, [esp+8]  ; stack start
    MOV edx, [esp+12] ; page directory
    MOV cr3, edx  ; switch address space -- still safely on the OLD stack here
    MOV esp, ecx      
    JMP eax

BITS 32
global finalRun
; Does the final part of the work before jumping to the program
; Arguments:
;   - Program entry point
;   - Program stack start
;   - Page directory (physical address) to switch into
finalRun:
    CLI ; Prevent a PIT being called with a incorrect address space
    MOV eax, [esp+4]  ; entry point
    MOV ecx, [esp+8]  ; stack start
    MOV edx, [esp+12] ; page directory
    MOV cr3, edx  ; switch address space -- still safely on the OLD stack here
    MOV esp, ecx      
    STI ; We have reached the process: we can reenable interrupts
    JMP eax

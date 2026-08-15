BITS 32
global finalRun
global resumeProcess
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


; cdecl: resumeProcess(RegisterState* state, void* pageDirectory)
resumeProcess:
    CLI
    MOV eax, [esp+4] ; state*
    MOV ecx, [esp+8] ; pagedir
    MOV cr3, ecx

    MOV esi, eax ; esi = state, base pointer for the rest of this routine
    MOV ecx, [esi+28] ; state->esp
    MOV esp, ecx

    PUSH DWORD [esi+36] ; state->eflags
    PUSH DWORD 0x08     ; cs (flat ring0 selector, matches this OS's model)
    PUSH DWORD [esi+32] ; state->eip

    ; Segments next, using ax as scratch before eax gets its real value
    MOVZX eax, WORD [esi+48]
    MOV ds, ax
    MOVZX eax, WORD [esi+52]
    MOV es, ax
    MOVZX eax, WORD [esi+56]
    MOV fs, ax
    MOVZX eax, WORD [esi+60]
    MOV gs, ax

    ; Real GPRs, loaded last so nothing after this can disturb them
    MOV ebx, [esi+4]
    MOV ecx, [esi+8]
    MOV edx, [esi+12]
    MOV edi, [esi+20]
    MOV ebp, [esi+24]
    MOV eax, [esi+0]
    MOV esi, [esi+16]   ; state->esi -- last, since esi was our base pointer

    IRETD

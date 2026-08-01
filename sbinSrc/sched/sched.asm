BITS 32
global _start

_start:
    
    CLI
.hlt:
    HLT
    JMP .hlt

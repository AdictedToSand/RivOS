#include "gen/serial.hpp"
#include <sys/IDT/idt.hpp>

#include <gen/reboot.hpp>

static inline auto vectorToExceptionName(uint32_t vector) -> const char* {
    switch (vector) {
        case 0: return "DE";
        case 1: return "DB";
        case 3: return "BP";
        case 4: return "OF";
        case 5: return "BR";
        case 6: return "UD";
        case 7: return "NM";
        case 8: return "DF";
        case 10: return "TS";
        case 11: return "NP";
        case 12: return "SS";
        case 13: return "GP";
        case 14: return "PF";
        case 15: return "RESERVED";
        case 16: return "MF";
        case 17: return "AC";
        case 18: return "MC";
        case 19: return "XM/#XF";
        case 20: return "VE";
        case 28: return "RV";
        case 29: return "VC";
        case 30: return "SX";
        default: return "UNKNOWN";
    }
}

enum class InterruptTypes : uint8_t {
    Abort,
    Trap,
    Fault,
    Regular,
};

inline auto interruptToType(uint32_t vector) -> InterruptTypes {
    switch (vector) {
        // Faults
        case 0:  // #DE Divide Error
        case 4:  // #OF Overflow
        case 5:  // #BR BOUND Range Exceeded
        case 6:  // #UD Invalid Opcode
        case 7:  // #NM Device Not Available
        case 10: // #TS Invalid TSS
        case 11: // #NP Segment Not Present
        case 12: // #SS Stack-Segment Fault
        case 13: // #GP General Protection
        case 14: // #PF Page Fault
        case 17: // #AC Alignment Check
        case 21: // #CP Control Protection
        case 28: // #HV Hypervisor Injection
        case 29: // #VC VMM Communication
        case 30: // #SX Security Exception
            return InterruptTypes::Fault;

        // Traps
        case 1:  // #DB Debug
        case 3:  // #BP Breakpoint
            return InterruptTypes::Trap;

        // Aborts
        case 8:  // #DF Double Fault
        case 18: // #MC Machine Check
            return InterruptTypes::Abort;

        // Everything else (reserved, NMI, IRQs, software ints...)
        default:
            return InterruptTypes::Regular;
    }
}

inline auto interruptTypeToStr(InterruptTypes t) -> const char* {
    switch (t) {
        case InterruptTypes::Abort: return "Abort";
        case InterruptTypes::Trap: return "Trap";
        case InterruptTypes::Fault: return "Fault";
        case InterruptTypes::Regular: return "Regular";
    }

    __builtin_unreachable(); // Why the fuck do i need to clarify ths
}

[[gnu::noreturn]]
auto handleAbort() -> void {
    Terminal::printf(R"(An interrupt of type "Abort" was called.
This is a fatal interrupt. The machine will reboot after any keypress
If the fault is of type #MC it is recommended to check hardware.
Press any key to continue... ")");
    while (getc() == 0) {
    
    }
    reboot();
}

auto handleTrap() -> void {
    Terminal::printf("A trap was triggered. The machine will reboot after any keypress... "); while (getc() == 0)
    reboot();
}

auto handleFault(int ft) -> void {
    Terminal::printf("A fault was triggered in the kernel. Execution will not continue");
#ifdef DEBUG
    if (ft == 14) { // #PF
        u32 cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        Serial::logf("\nCR2=%x", cr2);
    }
#endif
}

auto handleRegular() -> void {
}

void exceptionHandler(InterruptFrame* ifrm) {
    Serial::logf("EXCEPTION: #%s", vectorToExceptionName(ifrm->vector));
#ifndef DEBUG
    Terminal::clear();
#endif
    Serial::logf("EIP=%x CS=%x EFLAGS=%x", ifrm->eip, ifrm->cs, ifrm->eflags);
    Serial::logf("Error code=0x%x", ifrm->errorCode);

    const InterruptTypes itype = interruptToType(ifrm->vector);

    Terminal::printfColor(R"(An unhandled interrupt happened. The machine will abort.
The fault was: #%s
The fault is of type: %s
)"
        , (u8) Terminal::VgaColor::Red,vectorToExceptionName(ifrm->vector), interruptTypeToStr(itype));

    asm volatile ("CLI");

    switch (itype) {
        case InterruptTypes::Fault: handleFault(ifrm->vector); break;
        case InterruptTypes::Abort: handleAbort(); break;
        case InterruptTypes::Regular: handleRegular(); break;
        case InterruptTypes::Trap: handleTrap(); break;
    }

    for (;;) ;
}

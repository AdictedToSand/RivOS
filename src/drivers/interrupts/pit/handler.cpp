#include <sys/PIT/pit.hpp>

#include <sys/sysmod.hpp>

#include <gen/serial.hpp>

#include <proc/process.hpp>

volatile u64 ticks = 0;

struct RegisterStatePIT {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp; // original ESP before PUSHA
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 eip;
    u32 cs;
    u32 eflags;
};

// Proper stack setup is not required here
extern "C" auto pitHandler(RegisterStatePIT* state) -> void {
    asm volatile ("CLI");
    
    Process* proc = nullptr;
    for (auto& vproc : processes) {
        if (vproc->pid == activeProcessPid) {
            proc = vproc;
        }
    }
    if (proc) {
        proc->state->edi = state->edi;
        proc->state->esi = state->esi;
        proc->state->ebp = state->ebp;
        proc->state->esp = state->esp + 12; // The CPU pushes errcode and such, account for them
        proc->state->ebx = state->ebx;
        proc->state->edx = state->edx;
        proc->state->ecx = state->ecx;
        proc->state->eax = state->eax;
        proc->state->eip = state->eip;
        proc->state->cs = state->cs;
        proc->state->eflags = state->eflags;
    }
    auto pitEntry = SysModuleHandler::getFuncEntry(SysModuleId::PIT);
    if (pitEntry) {
        static RegisterStatePIT ownerVisibleState;
        ownerVisibleState = *state;

        u32* const interrupted = Mmu::activeDirectory;
        Mmu::switchAddressSpace(pitEntry->ownerDir);
        // TODO:...
        ((void (*)(void*)) (pitEntry->func)) (&ownerVisibleState);
        Mmu::switchAddressSpace(interrupted);
    }
    ticks++;
    PIC::sendEoi(0);
}

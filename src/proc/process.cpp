#include <proc/ELF/loader.hpp>
#include <proc/process.hpp>

#include <gen/serial.hpp>
#include <sys/PIC/pic.hpp>

pid_t latestPid = 1;
pid_t activeProcessPid = 0;

extern "C" [[gnu::noreturn]] void finalRun(void* entryPoint, void* stackTop, void* pageDirectory);

[[gnu::noreturn]]
auto Process::run(ProcessImportance iimportance) -> void {
    asm volatile ("CLI");
    Serial::logf("Process %s Ran as pid: %u", pname, pid);
    importance = iimportance;

    u32 stackPagesf = (STACK_SIZE + 4095) / 4096;
    u32 stackPde = ((u32) STACK_BEGIN) >> 22;
    pageDirectory[stackPde] = 0;

    for (u32 i = 0; i < stackPagesf; i++) {
        void* frame = PhysicalFrameAllocator::allocFrame();
        PhysToVirt stackaddr;
        stackaddr.virt = (void*) ((u32) STACK_BEGIN - i * 4096);
        stackaddr.phys = frame;
        stackPages.pushBack(stackaddr);
        Mmu::mapPageIn(pageDirectory, frame, (void*) ((u32) STACK_BEGIN - i * 4096), Mmu::FLAGS_PRESENT | Mmu::FLAGS_WRITABLE);
    }
    activeProcessPid = pid;

    processes.pushBack(this);

    Mmu::activeDirectory = pageDirectory;
    finalRun(entryPoint, STACK_BEGIN, pageDirectory);
}
auto Process::exit(u8 code) -> void {
    //TODO: Cleanup
    if (code != 0) {
        Serial::log("Process quited with exitcode nonzero");
    }
    if (importance == ProcessImportance::REQ) {
        Serial::log("Required process exited");
        Serial::logf("With exitcode=%u", code);

        Str msg = "Required process exited: ";
        msg += pname;
 
        kpanic(msg.toCStr());
    }
}

inline void setRegistersTo(RegisterState* s) {
    asm volatile(
        // Segment loads first: they use ax as scratch, so do this before eax
        // is given its real, final restored value below.
        "movw 48(%%esi), %%ax\n"
        "movw %%ax, %%ds\n"

        "movw 52(%%esi), %%ax\n"
        "movw %%ax, %%es\n"

        "movw 56(%%esi), %%ax\n"
        "movw %%ax, %%fs\n"

        "movw 60(%%esi), %%ax\n"
        "movw %%ax, %%gs\n"

        "movl  0(%%esi), %%eax\n"
        "movl  4(%%esi), %%ebx\n"
        "movl  8(%%esi), %%ecx\n"
        "movl 12(%%esi), %%edx\n"

        "movl 20(%%esi), %%edi\n"
        "movl 24(%%esi), %%ebp\n"

        "pushl 36(%%esi)\n"
        "popfl\n"

        "movl 16(%%esi), %%esi\n"
        : "+S"(s)
        :
        : "eax", "ebx", "ecx", "edx", "edi", "ebp", "memory"
    );
}
auto Process::ctxtSwitch() -> void {
    asm volatile ("CLI");
    activeProcessPid = pid;

    // Grab these BEFORE setRegistersTo() runs: it deliberately repurposes esi
    // as scratch, and we don't want that anywhere near the values finalRun()
    // depends on to know where it's jumping.
    void* const targetEip = (void*) state->eip;
    void* const targetEsp = (void*) state->esp;

    if (!targetEip || !targetEsp) {
        Serial::logf("ctxtSwitch: refusing to jump into pid %u with eip=%x esp=%x", pid, (u32) targetEip, (u32) targetEsp);
        kpanic("ctxtSwitch got a zeroed RegisterState");
    }

    setRegistersTo(state);

    PIC::sendEoi(0);

    Mmu::activeDirectory = pageDirectory;
    finalRun(targetEip, targetEsp, pageDirectory);
}

auto getNewPid() -> pid_t {
    return latestPid++;
}

Vector<Process*> processes;

auto loadProcessFromFile(const char* fp, const char* procname, ProcessPriveledgeLevel priv) -> Process* {
    ElfExecutable elf;
    elf.fromFile(fp);
    //TODO: Priviledge validation
    if (!elf.isValid()) return nullptr;
    Process* proc = elf.load(procname, priv);
    return proc;
}
auto runProcess(Process* proc) -> void {
    proc->run(ProcessImportance::REQ); //TODO
}
auto getProcessByPid(pid_t pid) -> Process*;

auto getProcessList() -> Vector<Process*>* {
    return &processes;
}

auto procCtxtSwitch(Process* proc) -> void {
    proc->ctxtSwitch();
}

[[gnu::noinline]]
auto getActiveProcessPid() -> pid_t {
    return activeProcessPid;
}

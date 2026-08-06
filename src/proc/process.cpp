#include "proc/ELF/loader.hpp"
#include <proc/process.hpp>

pid_t latestPid = 1;
pid_t activeProcessPid = 0;

[[gnu::noreturn]]
auto Process::run(ProcessImportance iimportance) -> void {
    importance = iimportance;

    u32 stackPagesf = (STACK_SIZE + 4095) / 4096;

    for (u32 i = 0; i < stackPagesf; i++) {
        void* frame = PhysicalFrameAllocator::allocFrame();
        PhysToVirt stackaddr;
        stackaddr.virt = (void*) ((u32) STACK_BEGIN - i * 4096);
        stackaddr.phys = frame;
        stackPages.pushBack(stackaddr);

        Mmu::mapPageIn(pageDirectory, frame, (void*) ((u32) STACK_BEGIN - i * 4096), Mmu::FLAGS_PRESENT | Mmu::FLAGS_WRITABLE);
    }
    activeProcessPid = pid;

    for (auto& p : pages) {
        Serial::logf("Phys=0x%x,Virt=0x%x", p.phys, p.virt); 
    }

    processes.pushBack(this);

    Mmu::switchAddressSpace(pageDirectory);
    finalRun(entryPoint, STACK_BEGIN);
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

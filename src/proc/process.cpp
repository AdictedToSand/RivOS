#include <proc/process.hpp>

pid_t latestPid = 1;
pid_t activeProcessPid = 0;

[[gnu::noreturn]]
auto Process::run(ProcessImportance iimportance) -> void {
    importance = iimportance;

    u32 stackPages = (STACK_SIZE + 4095) / 4096;

    for (u32 i = 0; i < stackPages; i++) {
        void* frame = PhysicalFrameAllocator::allocFrame();

        Mmu::mapPage(frame, (void*) ((u32) STACK_BEGIN - i * 4096), Mmu::FLAGS_PRESENT | Mmu::FLAGS_WRITABLE);
    }
    activeProcessPid = pid;

    processes.pushBack(this);
    finalRun(entryPoint, STACK_BEGIN);
}
auto Process::exit(u8 code) -> void {
    if (code != 0) {
        Serial::log("Process quited with exitcode nonzero");
    }
    if (importance == ProcessImportance::REQ) {
        Serial::log("Required process exited");
        if (code == 0) Serial::log("With exit code 0");
        else Serial::log("With a nonzero exitcode!");
            
        kpanic("Required process exited");
    }
}

auto getNewPid() -> pid_t {
    return latestPid++;
}

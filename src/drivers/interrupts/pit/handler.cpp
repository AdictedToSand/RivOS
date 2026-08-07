#include <sys/PIT/pit.hpp>

#include <sys/sysmod.hpp>

volatile u64 ticks = 0;

// Proper stack setup is not required here
extern "C" auto pitHandler() -> void {
 /*   asm volatile ("CLI");
    auto pitEntry = SysModuleHandler::getFuncEntry(SysModuleId::PIT);
    if (pitEntry) {
        u32* const interrupted = Mmu::activeDirectory;
        Mmu::switchAddressSpace(pitEntry->ownerDir);
        pitEntry->func();
        Mmu::switchAddressSpace(interrupted);
    }
    //SysModuleHandler::getFunc(SysModuleId::PIT)();
    // VERY temporary
    ticks++;
    asm volatile ("STI");
    PIC::sendEoi(0);
*/
    ticks++;
    PIC::sendEoi(0);
}

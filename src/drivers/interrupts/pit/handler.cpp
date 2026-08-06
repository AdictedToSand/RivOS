#include <sys/PIT/pit.hpp>

#include <sys/sysmod.hpp>

volatile u64 ticks = 0;

// Proper stack setup is not required here
extern "C" auto pitHandler() -> void {
    //SysModuleHandler::getFunc(SysModuleId::PIT)();
    // VERY temporary
    ticks++;
    PIC::sendEoi(0);
}

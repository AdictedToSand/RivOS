#include <vis/vis.hpp>

#include <proc/process.hpp>

#include <sys/sysmod.hpp>

auto VisualsPidEnforced::enforce() -> bool {
    const pid_t ownerOfFb = SysModuleHandler::getowner(SysModuleId::Framebuffer);

    if (!ownerOfFb) return 1;
    if (activeProcessPid != ownerOfFb) return 1;

    return 0;
}

auto VisualsPidEnforced::putPixel(u32 argb, u32 x, u32 y) -> bool {
    if (enforce()) return 1;

    Visuals::putPixel(argb, x, y);

    return 0;
}

auto VisualsPidEnforced::getFbPhysAddr() -> u32 {
    if (enforce()) return 0;

    return Visuals::getFbPhysAddr();
}
auto VisualsPidEnforced::getFbSizeBytes() -> u32 {
    if (enforce()) return 0;

    return Visuals::getFbSizeBytes();
}
auto VisualsPidEnforced::getScreenHeight() -> u32 {
    if (enforce()) return 0;

    return Visuals::getScreenHeight();
}
auto VisualsPidEnforced::getScreenWidth() -> u32 {
    if (enforce()) return 0;

    return Visuals::getScreenWidth();
}
auto VisualsPidEnforced::getPitch() -> u32 {
    if (enforce()) return 0;

    return Visuals::getPitch();
}

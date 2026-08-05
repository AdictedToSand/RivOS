#include <terminal/terminal.hpp>

#include <drivers/fs/fs.hpp>

#include <int.h>

#include <sys/PIC/pic.hpp>

#include <gen/io.hpp>
#include <gen/alpha.hpp>

#include <mem/alloc.hpp>

#include <proc/process.hpp>

#include <sys/sysmod.hpp>

bool shift = false;
bool capslock = false;

extern "C" auto keyboardHandler() -> void {
    SysModuleHandler::
    SysModuleHandler::getFunc(SysModuleId::Keyboard_PS2)();
    //inb(0x60);
    PIC::sendEoi(1);
}

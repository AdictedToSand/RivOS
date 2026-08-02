#pragma once
#include <sys/IDT/idt.hpp>
#include <sys/PIC/pic.hpp>

extern "C" void pitStub();
extern "C" void keyboardStub();
extern "C" void syscallStub(); 

struct HardwareInterrupts {
    static auto init() -> void;
};

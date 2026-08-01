#pragma once
#include <sys/IDT/idt.hpp>
#include <sys/PIC/pic.hpp>

extern "C" void pitStub();
extern "C" void keyboardStub();

struct HardwareInterrupts {
    static auto init() -> void;
};
#pragma once
#include <terminal/terminal.hpp>

static void kpanic(const char* msg) {
    Terminal::clear();

    Terminal::setColor(Terminal::VgaColor::Red);
    Terminal::writeStr("Kernel panic: ");
    Terminal::writeStr(msg);
}

static void kassrt(bool eval, const char* msg) {
    if (!eval) 
        kpanic(msg);
}

#include <terminal/terminal.hpp>

extern "C" { // Disable name mangling

void kernelMain() {
    Terminal::init();

    Terminal::writeStr("Hello, kernel world!\nHi");
}

} // extern "C"

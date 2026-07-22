#include <sys/IDT/idt.hpp>

void exceptionHandler() {
    kpanic("Exception handler reached");
}

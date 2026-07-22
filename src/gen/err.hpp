#pragma once
#include <terminal/terminal.hpp>

[[gnu::noreturn]]
static void kpanic(const char* msg) {
    Terminal::clear();

    Terminal::setColor(Terminal::VgaColor::Red);
    Terminal::writeStr("Kernel panic: ");
    Terminal::writeStr(msg);

    for (;;);
}

static void kassrt(bool eval, const char* msg) {
    if (!eval) 
        kpanic(msg);
}

// Wraps a value of type T. Use makeErr() to make it an err value, otherwise use = operator.
// Use .val() to get a reference to the value 
// If isErr a kpanic is triggered
template<typename T>
class Expected {
    bool isCorrect;

    T* correctVal;

public:
    inline auto val() -> T& {
        if (!isCorrect) //TODO: Better error handling (example: exceptions)
            kpanic("Called val() on an err value");

        return *correctVal;
    }

    auto isErr() -> bool {
        return !isCorrect;
    }
    auto isCorr() -> bool {
        return isCorrect;
    }

    auto makeErr() -> void {
        isCorrect = false;
    }

    Expected(T& ptrVal) {
        isCorrect = true;
        correctVal = &ptrVal;
    }

    enum class ErrorTypes {
        Error,
    };

    Expected(ErrorTypes _err) {
        isCorrect = false;
    }

    inline operator T&() {
        return val();    
    }
    inline auto operator=(const T& newVal) -> Expected& {
        val() = newVal;
        return *this;
    }
};


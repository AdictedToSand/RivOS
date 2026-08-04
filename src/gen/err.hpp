#pragma once
#include <terminal/terminal.hpp>

#ifndef DEBUG
#define kpanic(msg) do { \
    Terminal::clear(); \
    Terminal::enable(); \
    \
    Terminal::setColor((u8) Terminal::VgaColor::Red); \
    Terminal::printf("A kernel panic occurred at: (%s) -> (%s) -> (line %i) -> %s", __FILE__, __FUNCTION__, __LINE__, msg); \
    \
    for (;;); \
} while (0)
#else
#define kpanic(msg) do { \
    Terminal::enable(); \
    Terminal::printfColor("\nA kernel panic occurred at: (%s) -> (%s) -> (line %i): %s", (u32) Terminal::VgaColor::Red, \
        __FILE__, __FUNCTION__, __LINE__, msg); \
    for (;;) ; \
} while (0)
#endif

static inline void kassrt(bool eval, const char* msg) {
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
        (void) _err;
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


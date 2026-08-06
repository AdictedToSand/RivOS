#pragma once
#include <terminal/terminal.hpp>

static const char* KERN_PANICBANNER = R"(                       _         __
  ___   ___  _ __  ___(_) ___ _ / /
 / _ \ / _ \| '_ \/ __| |/ _ (_) |
| (_) | (_) | |_) \__ \ |  __/_| |
 \___/ \___/| .__/|___/_|\___(_) |
            |_|                 \_\)";

#define kpanic(msg) do { \
    asm volatile ("CLI"); \
    Terminal::clear(); \
    Visuals::fillScreen(0x000000CC); \
    Terminal::enable(); \
    Terminal::setColor(255);/* Evil hack */ \
    Terminal::printf("%s\n\n\n", KERN_PANICBANNER); \
    \
    Terminal::printf("A kernel panic occurred at: (%s) -> (%s) -> (line %i) -> %s", __FILE__, __FUNCTION__, __LINE__, msg); \
    \
    for (;;); \
} while (0)
/*
#else
#define kpanic(msg) do { \
    Terminal::enable(); \
    Terminal::printfColor("\nA kernel panic occurred at: (%s) -> (%s) -> (line %i): %s", (u32) Terminal::VgaColor::Red, \
        __FILE__, __FUNCTION__, __LINE__, msg); \
    for (;;) ; \
} while (0)
#endif*/

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


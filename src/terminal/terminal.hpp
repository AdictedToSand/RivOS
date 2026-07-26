#pragma once
#include <gen/serial.hpp>

#include <string.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Credits: https://wiki.osdev.org/Bare_Bones
struct Terminal {
    enum class VgaColor {
        Black = 0,
        Blue = 1,
        Green = 2,
        Cyan = 3,
        Red = 4,
        Magenta = 5,
        Brown = 6,
        LightGrey = 7,
        DarkGrey = 8,
        LightBlue = 9,
        LighGreen = 10,
        LightCyan = 11,
        LightRed = 12,
        LightMagenta = 13,
        LightBrown = 14,
    };

    inline static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000; // Would make this constexpr but fuck the C++ compiler ig
    inline static const size_t VGA_WIDTH = 80;
    inline static const size_t VGA_HEIGHT = 25;                                        

    inline static size_t terminalRow = 0;
    inline static size_t terminalColumn = 0;
    inline static uint8_t terminalColor = 0;
    inline static uint16_t* terminalBuffer = nullptr;

    static inline auto vgaEntryColor(VgaColor fg, VgaColor bg) -> uint8_t {
	    return (int) fg | (int) bg << 4;
    }
    static inline auto vgaEntry(unsigned char uc, uint8_t color) -> uint16_t {
	    return (uint16_t) uc | (uint16_t) color << 8;
    }

    static auto init() -> void {
        terminalBuffer = VGA_MEMORY;

	    terminalRow = 0;
	    terminalColumn = 0;
	    terminalColor = vgaEntryColor(VgaColor::LightGrey, VgaColor::Black);
	
	    for (size_t y = 0; y < VGA_HEIGHT; y++) {
		    for (size_t x = 0; x < VGA_WIDTH; x++) {
			    const size_t index = y * VGA_WIDTH + x;
			    terminalBuffer[index] = vgaEntry(' ', terminalColor);
		    }
	    }
    }

    static auto setColor(const VgaColor color) -> void {
        terminalColor = (uint8_t) color;
    }

    static auto putEntryAt(char c, uint8_t color, size_t x, size_t y) -> void {
        const size_t index = y * VGA_WIDTH + x;
        terminalBuffer[index] = vgaEntry(c, color);
    }
    static auto putChar(char c) -> void {
        if (c == '\n') {
            Serial::write(c);
            if (++terminalRow >= VGA_HEIGHT) {
                clear();
                return; //TODO: replace with proper scrolling
            }
            terminalColumn = 0;
        }
        else {
            Serial::write(c);
            putEntryAt(c, terminalColor, terminalColumn, terminalRow);
            if (++terminalColumn == VGA_WIDTH) {
                terminalColumn = 0;
                if (++terminalRow == VGA_HEIGHT)
                    terminalRow = 0;
            }
        }
    }
    static auto write(const char* const s, size_t len) -> void {
        for (size_t i = 0; i < len; i++)
            putChar(s[i]);
    }

    static auto writeStr(const char* const s) -> void {
        write(s, strlen(s));
    }
    static auto clear() -> void {
        init(); // Does the exact behaviour we need rn
    }

    // Credits: https://www.geeksforgeeks.org/c/print-long-int-number-c-using-putchar/
    static auto printi(int n) -> void {
        if (n < 0) {
            putChar('-');
            n = -n;
        }

        if (n / 10) {
            printi(n / 10);
        }

        putChar(n % 10 + '0');
    }
    static auto printHex(unsigned int n) -> void {
        const char hexChars[] = "0123456789ABCDEF";
      
        write("0x", 2);

        for (int i = (sizeof(unsigned int) * 8) - 4; i >= 0; i -= sizeof(unsigned int)) {
            putChar(hexChars[(n >> i) & 0x0F]);
        }
    }

    static auto printf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        for (size_t i = 0; fmt[i]; i++) {
            switch (fmt[i]) {
                case '%': {
                    const char fmtSpecifier = fmt[++i];

                    switch (fmtSpecifier) {
                        case '%': putChar('%'); break;
                        case 's': writeStr(va_arg(args, const char*)); break;
                        case 'x': printHex(va_arg(args, unsigned int)); break;
                        case 'c': putChar(va_arg(args, int));
                        case 'i': printi(va_arg(args, int));
                        default: break;
                    }

                    break;
                }
                default: {
                    putChar(fmt[i]);
                }
            }
        }

        va_end(args);
    }
};


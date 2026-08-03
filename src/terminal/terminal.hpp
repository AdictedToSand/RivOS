#pragma once
#include <terminal/font.hpp>

#include <gen/serial.hpp>

#include <cstring.hpp>

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <vis/vis.hpp>

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
    inline static uint8_t terminalColor = 0;

    static inline u32 terminalX;
    static inline u32 terminalY;

    static inline bool enabled;

    static inline auto vgaEntryColor(VgaColor fg, VgaColor bg) -> uint8_t {
	    return (int) fg | (int) bg << 4;
    }
    static inline auto vgaEntry(unsigned char uc, uint8_t color) -> uint16_t {
	    return (uint16_t) uc | (uint16_t) color << 8;
    }

    static auto init() -> void {
        Visuals::fillScreen(0x00101010); 
        terminalX = 0, terminalY = 0;
	    terminalColor = vgaEntryColor(VgaColor::LightGrey, VgaColor::Black);
        enabled = true;
    }

    static auto setColor(const u8 color) -> void {
        terminalColor = color;
    }
    static auto putChar(char c) -> void {
        if (!enabled) return;
        const int charWidth = 16;
        const int charHeight = 16;

        if (c == '\n') {
            terminalX = 0;
            terminalY += charHeight;
            return;
        }

        if (terminalX + charWidth > Visuals::getScreenWidth()) {
            terminalX = 0;
            terminalY += charHeight;
        }

        if (terminalY + charHeight > Visuals::getScreenHeight()) {
            clear();
            terminalX = 0;
            terminalY = 0;
        }

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                bool set = font8x8_basic[(unsigned char)c][y] & (1 << x);

                if (set) {
                    const int px = terminalX + x * 2;
                    const int py = terminalY + y * 2;

                    Visuals::putPixel(0x00FFFFFF, px, py);
                    Visuals::putPixel(0x00FFFFFF, px + 1, py);
                    Visuals::putPixel(0x00FFFFFF, px, py + 1);
                    Visuals::putPixel(0x00FFFFFF, px + 1, py + 1);
                }
            }
        }

        terminalX += charWidth;
    }
    static auto write(const char* const s, size_t len) -> void {
        for (size_t i = 0; i < len; i++)
            putChar(s[i]);
    }

    static auto writeStr(const char* const s) -> void {
        write(s, strlen(s));
    }
    static auto clear() -> void {
        Visuals::fillScreen(0x00101010);
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
    static auto printu(unsigned int n) -> void {
        if (n >= 10) printu(n / 10);
        putChar('0' + (n % 10));
    }
    static auto printHex(unsigned int n) -> void {
        const char hexChars[] = "0123456789ABCDEF";
      
        write("0x", 2);

        for (int i = (sizeof(unsigned int) * 8) - 4; i >= 0; i -= 4) {
            putChar(hexChars[(n >> i) & 0x0F]);
        }
    }
    static auto printPtr(void* ptr) {
        const char hexChars[] = "0123456789ABCDEF";

        write("0x", 2);

        for (int i = (sizeof(void*) * 8) - 4; i >= 0; i -= 4) {
            putChar(hexChars[((uintptr_t) ptr >> i) & 0x0F]);
        }
    }

    static auto printf_vaArgs(const char* fmt, va_list args) -> void {
        for (size_t i = 0; fmt[i]; i++) {
            switch (fmt[i]) {
                case '%': {
                    const char fmtSpecifier = fmt[++i];

                    switch (fmtSpecifier) {
                        case '%': putChar('%'); break;
                        case 's': writeStr(va_arg(args, const char*)); break;
                        case 'x': printHex(va_arg(args, unsigned int)); break;
                        case 'c': putChar(va_arg(args, int)); break;
                        case 'i': printi(va_arg(args, int)); break; 
                        case 'p': printPtr(va_arg(args, void*)); break;
                        case 'u': ; printu(va_arg(args, unsigned int)); break;
                        default: break;
                    }

                    break;
                }
                default: {
                    putChar(fmt[i]);
                }
            }
        }
    }
    static auto printf(const char* fmt, ...) -> void {
        va_list args;
        va_start(args, fmt);
        printf_vaArgs(fmt, args);
        va_end(args);
    }

    static auto printfColor(const char* fmt, u32 color, ...) -> void {
        const auto orginColor = terminalColor;
        setColor(color);
        va_list args;
        va_start(args, color);
        printf_vaArgs(fmt, args);
        va_end(args);
        setColor(orginColor);
    }
};


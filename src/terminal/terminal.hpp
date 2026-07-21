#pragma once
#include <string.hpp>

#include <stddef.h>
#include <stdint.h>

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

    static inline uint8_t vgaEntryColor(VgaColor fg, VgaColor bg)  {
	    return (int) fg | (int) bg << 4;
    }
    static inline uint16_t vgaEntry(unsigned char uc, uint8_t color) {
	    return (uint16_t) uc | (uint16_t) color << 8;
    }

    static void init() {
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

    static void setColor(const uint8_t color) {
        terminalColor = color;
    }

    static void putEntryAt(char c, uint8_t color, size_t x, size_t y) {
        const size_t index = y * VGA_WIDTH + x;
        terminalBuffer[index] = vgaEntry(c, color);
    }
    static void putChar(char c)  {
        if (c == '\n') {
            if (++terminalRow >= VGA_HEIGHT) {
                clear();
                return; //TODO: replace with proper scrolling
            }
            terminalColumn = 0;
        }
        else {
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
};


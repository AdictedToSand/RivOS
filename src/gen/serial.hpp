#pragma once
#include <gen/io.hpp>

#include <stdarg.h>

// Credits: https://wiki.osdev.org/Serial_Ports

#define PORT 0x3f8 // COM1

// This should only be used for debugging purposes
// For outputting to screen, try terminal/terminal.hpp
struct Serial {
    static auto init() -> int {
#ifdef DEBUG
        outb(PORT + 1, 0x00);    // Disable all interruptsa
        outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, 0x00);    //                  (hi byte)
        outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
        outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
        outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
        outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

        if(inb(PORT + 0) != 0xAE) {
            return 1;
        }

        outb(PORT + 4, 0x0F);
        return 0;
#endif
        return 0;
    }

    static auto received() -> int {
#ifdef DEBUG
        return inb(PORT + 5) & 0x01;
#else
        return 1;
#endif
    }

    static auto read() -> char {
#ifdef DEBUG
        while (received() == 0) ;

        return inb(PORT);
#endif
        return 0;
    }
    
    static auto isTransmitEmpty() -> int {
#ifdef DEBUG
        return inb(PORT + 5) & 0x20;
#else
        return 1;
#endif
    }

    static auto write(char a) -> void {
        (void) a;
#ifdef DEBUG
        while (isTransmitEmpty() == 0) ;
        
        outb(PORT, a);
#endif
    }
    static auto writes(const char* s) -> void {
        (void) s;
#ifdef DEBUG
        for (; *s; s++) {
            write(*s);
        }
    }
#endif
    static auto log(const char* mes) -> void {
        (void) mes;
#ifdef DEBUG
        writes("[LOG]: ");
        writes(mes);
        write('\n');
#endif
    }
    static auto logf(const char* fmt, ...) -> void {
#ifdef DEBUG
        char buffer[256];
        u32 pos = 0;

        va_list args;
        va_start(args, fmt);

        writes("[LOG]: ");

        for (; *fmt && pos < sizeof(buffer) - 1; fmt++) {
            if (*fmt != '%') {
                buffer[pos++] = *fmt;
                continue;
            }

            fmt++;

            if (*fmt == 's') {
                const char* s = va_arg(args, const char*);
                while (*s && pos < sizeof(buffer) - 1)
                    buffer[pos++] = *s++;
            }
            else if (*fmt == 'c') {
                buffer[pos++] = (char)va_arg(args, int);
            }
            else if (*fmt == 'u') {
                u32 n = va_arg(args, u32);
                char tmp[16];
                u32 i = 0;

                if (n == 0)
                    tmp[i++] = '0';

                while (n) {
                    tmp[i++] = '0' + (n % 10);
                    n /= 10;
                }

                while (i)
                    buffer[pos++] = tmp[--i];
            }
            else if (*fmt == 'x') {
                u32 n = va_arg(args, u32);
                char tmp[16];
                u32 i = 0;

                if (n == 0)
                    tmp[i++] = '0';

                while (n) {
                    u32 digit = n & 0xF;
                    tmp[i++] = digit < 10 ? '0' + digit : 'A' + digit - 10;
                    n >>= 4;
                }

                while (i)
                    buffer[pos++] = tmp[--i];
            }
        }

        va_end(args);

        buffer[pos] = 0;
        writes(buffer);
        write('\n');
#endif
    }
};

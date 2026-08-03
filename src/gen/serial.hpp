#pragma once
#include <gen/io.hpp>

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
    }
#endif
};

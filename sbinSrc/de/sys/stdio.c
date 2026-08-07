#include "stdio.h"

#include "sys.h"

#include "../gen/string.h"

void puts(const char* s) {
    write(stdout, s, strlen(s));
}

#include <stdarg.h>
#include <stddef.h>

void puts(const char* s);
void* mmap(size_t size);
void munmap(void* ptr);
size_t strlen(const char* s);

static void appendChar(char* buf, size_t* pos, char c) {
    buf[*pos] = c;
    (*pos)++;
}

static void appendStr(char* buf, size_t* pos, const char* s) {
    size_t len = strlen(s);
    for (size_t i = 0; i < len; i++) {
        buf[*pos] = s[i];
        (*pos)++;
    }
}

static void appendNum(char* buf, size_t* pos, unsigned long long n, unsigned base) {
    char tmp[64];
    size_t i = 0;

    if (n == 0) {
        appendChar(buf, pos, '0');
        return;
    }

    while (n > 0) {
        unsigned digit = n % base;
        tmp[i++] = digit < 10 ? ('0' + digit) : ('a' + digit - 10);
        n /= base;
    }

    while (i > 0) {
        appendChar(buf, pos, tmp[--i]);
    }
}

void printf(const char* fmt, ...) {
    char* out = (char*) mmap(4096);
    size_t pos = 0;

    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != 0; i++) {
        if (fmt[i] != '%') {
            appendChar(out, &pos, fmt[i]);
            continue;
        }

        i++;

        switch (fmt[i]) {
        case '%':
            appendChar(out, &pos, '%');
            break;

        case 's':
            appendStr(out, &pos, va_arg(args, const char*));
            break;

        case 'c':
            appendChar(out, &pos, (char) va_arg(args, int));
            break;

        case 'u':
            appendNum(out, &pos, va_arg(args, unsigned int), 10);
            break;

        case 'd': {
            int n = va_arg(args, int);
            if (n < 0) {
                appendChar(out, &pos, '-');
                n = -n;
            }
            appendNum(out, &pos, (unsigned) n, 10);
            break;
        }

        case 'x':
            appendNum(out, &pos, va_arg(args, unsigned int), 16);
            break;
        }
    }

    out[pos] = 0;

    va_end(args);

    puts(out);
    munmap(out);
}

void log(const char* s) {
    write(stdlog, s, strlen(s));
}

void logf(const char* fmt, ...) {
    char* out = (char*) mmap(4096);
    size_t pos = 0;

    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != 0; i++) {
        if (fmt[i] != '%') {
            appendChar(out, &pos, fmt[i]);
            continue;
        }

        i++;

        switch (fmt[i]) {
        case '%':
            appendChar(out, &pos, '%');
            break;

        case 's':
            appendStr(out, &pos, va_arg(args, const char*));
            break;

        case 'c':
            appendChar(out, &pos, (char) va_arg(args, int));
            break;

        case 'u':
            appendNum(out, &pos, va_arg(args, unsigned int), 10);
            break;

        case 'd': {
            int n = va_arg(args, int);
            if (n < 0) {
                appendChar(out, &pos, '-');
                n = -n;
            }
            appendNum(out, &pos, (unsigned) n, 10);
            break;
        }

        case 'x':
            appendNum(out, &pos, va_arg(args, unsigned int), 16);
            break;
        }
    }

    out[pos] = 0;

    va_end(args);

    log(out);
    munmap(out);
}


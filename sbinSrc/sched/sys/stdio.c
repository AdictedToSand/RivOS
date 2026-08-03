#include "stdio.h"

#include "sys.h"

#include "../gen/string.h"

void puts(const char* s) {
    write(stdout, s, strlen(s));
}

#include "sys.h"

fd_t stdout = 0;

void sysInit(void) {
    stdout = open("/dev/stdout");
}

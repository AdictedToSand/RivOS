#include "sys.h"

fd_t stdout = 0;
fd_t stdlog = 0;

void sysInit(void) {
    stdout = open("/dev/stdout");
    stdlog = open("/dev/stdlog");
    if (!stdlog || !stdout) {
        exit(1);
    }
}

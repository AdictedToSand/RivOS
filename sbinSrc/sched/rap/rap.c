#include "rap.h"

#include "../sys/sys.h"
#include "../sys/stdio.h"

#include "../mem/utils.h"

#include "../gen/alpha.h"

#define MEM_DEF_ALLOCSIZE 10000
RapFile parseRap(void) {
    RapFile ret;

    fd_t rapFd = open("/krn/virt/func.rap");
    if (!rapFd) {
        puts("RAP was not found");
        exit(1);
    }
    // We do not get to know the size of /krn/virt/func.rap (currently)
    // Just reserve a shit ton
    char* buf = mmap(MEM_DEF_ALLOCSIZE);

    if (!buf) {
        puts("Allocation failed");
        exit(1);
    }
    read(rapFd, buf, MEM_DEF_ALLOCSIZE);
    puts(buf);
    
       

    close(rapFd);
    munmap(buf);
    return ret;
}

void printRap(RapFile* rap) {

}

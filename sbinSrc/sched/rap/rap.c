#include "rap.h"

#include "../sys/sys.h"
#include "../sys/stdio.h"

#include "../mem/utils.h"

#include "../gen/alpha.h"
#include "../gen/string.h"

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
    memset(buf, 0, MEM_DEF_ALLOCSIZE);
    read(rapFd, buf, MEM_DEF_ALLOCSIZE);
    logf("rivapi=%s", buf);
     
    log("Read successfull, parsing RAP");

    const u32 rapEntries = countOccurence(buf, '\n'); 
    while (true) {
        u32 i = 0;
        const u32 strlenOfFuncName = strlenSpecChar(buf, '(');
        char* funcNameBuf = mmap(strlenOfFuncName + 1);
        memset(funcNameBuf, 0, strlenOfFuncName);
        for (u32 i = 0; i < strlenOfFuncName; i++) {
            funcNameBuf[i] = buf[i]; 
        }
        funcNameBuf[strlenOfFuncName] = 0;

        logf("Function=%s", funcNameBuf);

        buf += strlenOfFuncName + 1; // name + openparen

        while (*buf != ')') {
            const u32 strlenOfType = strlenSpecChar(buf, ' '); 
            char* const typebuf = mmap(strlenOfType + 1);
            strcpyLen(typebuf, buf, strlenOfType);
            if (streq(typebuf, "")) break;
            // For some reason only happens with empty variables
            logf("\t\tParamName=%s", typebuf);

            buf += strlenOfType + 1; // Type strlen + space
            
            const u32 strlenOfVarName = strlenSpecChar(buf, ' ');
            char* const namebuf = mmap(strlenOfVarName + 1);
            strcpyLen(namebuf, buf, strlenOfVarName);
            logf("\t\tVarName=%s", namebuf);
            buf += strlenOfVarName + 1; // Name strlen + space
        }
        while (*buf++ != '=') ;
        const u32 strlenOfAddr = strlenSpecChar(buf, '\n');
        char* sBuf = mmap(strlenOfAddr + 1);
        strcpyLen(sBuf, buf, strlenOfAddr);
        const u32 addr = stou(sBuf);
        logf("\t\tAddr=%u", addr);

        munmap(sBuf);
        buf += strlenSpecChar(buf, '\n') + 1;
        if (!(*buf))
            break;
    }

    logf("Parsing rap complete");
    close(rapFd);
    munmap(buf);
    return ret;
}

void printRap(RapFile* rap) {

}

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

    char* buf = mmap(MEM_DEF_ALLOCSIZE);

    if (!buf) {
        puts("Allocation failed");
        exit(1);
    }

    memset(buf, 0, MEM_DEF_ALLOCSIZE);
    read(rapFd, buf, MEM_DEF_ALLOCSIZE);

    log("Read successfull, parsing RAP");

    const u32 rapEntries = countOccurence(buf, '\n');

    ret.entriesArr.arr = mmap(rapEntries * sizeof(RapEntry));
    ret.entriesArr.len = rapEntries;

    u32 entryIndex = 0;

    while (*buf) {
        RapEntry* entry = &ret.entriesArr.arr[entryIndex];

        const u32 strlenOfFuncName = strlenSpecChar(buf, '(');

        char* funcNameBuf = mmap(strlenOfFuncName + 1);
        strcpyLen(funcNameBuf, buf, strlenOfFuncName);

        entry->functionName = funcNameBuf;
        entry->paramCount = 0;

        buf += strlenOfFuncName + 1;

        entry->paramArr = mmap(16 * sizeof(RapParameter));

        while (*buf != ')') {
            const u32 strlenOfType = strlenSpecChar(buf, ' ');

            if (strlenOfType == 0)
                break;

            RapParameter* param = &entry->paramArr[entry->paramCount++];

            char* typebuf = mmap(strlenOfType + 1);
            strcpyLen(typebuf, buf, strlenOfType);
            param->type = typebuf;

            buf += strlenOfType + 1;

            const u32 strlenOfVarName = strlenSpecChar(buf, ' ');

            char* namebuf = mmap(strlenOfVarName + 1);
            strcpyLen(namebuf, buf, strlenOfVarName);
            param->name = namebuf;

            buf += strlenOfVarName;

            if (*buf == ' ')
                buf++;
        }

        while (*buf++ != '=') {}

        const u32 strlenOfAddr = strlenSpecChar(buf, '\n');

        char* sBuf = mmap(strlenOfAddr + 1);
        strcpyLen(sBuf, buf, strlenOfAddr);

        entry->callLocation = (void*) stou(sBuf);

        munmap(sBuf);

        buf += strlenOfAddr;

        if (*buf == '\n')
            buf++;

        entryIndex++;
    }

    close(rapFd);
    munmap(buf);

    log("Parsing rap complete");

    return ret;
}

void printRap(RapFile* rap) {
    for (u32 i = 0; i < rap->entriesArr.len; i++) {
        RapEntry* entry = &rap->entriesArr.arr[i];

        logf("Function=%s", entry->functionName);
        logf("\t\tAddr=%u", (u32)entry->callLocation);

        for (u32 j = 0; j < entry->paramCount; j++) {
            RapParameter* param = &entry->paramArr[j];

            logf("\t\tParam=%s %s", param->type, param->name);
        }
    }
}

//TODO: Proper parameter checking
void* getRapAddr(RapFile* rap, const char* func) {
    for (u32 i = 0; i < rap->entriesArr.len; i++) {
        if (streq(rap->entriesArr.arr[i].functionName, func)) {
            return rap->entriesArr.arr[i].callLocation;
        }
    }

    return NULL;
}

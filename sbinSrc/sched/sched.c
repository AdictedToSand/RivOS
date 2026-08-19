#include <int.h>
#include <stdbool.h>
#include <stddef.h>

#include "gen/string.h"
#include "sys/sys.h"
#include "sys/stdio.h"

#include "rap/rap.h"

#include "scheduling/pitHandler.h"

#include "mem/utils.h"

RapFile rap;
const ProcessList* procList = NULL;

typedef Process* (*LoadProcessT)(const char* fp, const char* pname, ProcessPriveledgeLevel privlvl);

pid_t (*getpid)(void);
void (*procCtxtSwitch)(Process* proc);

[[gnu::noreturn]]
void _start() {
    sysInit();
    if (claim("PIT")) {
        puts("Unable to claim PIT");
        exit(1);
    }
    /*
    if (claim("SyscallDone")) {
        puts("Unable to claim SyscallDone");
        exit(1);
    }*/
    setFunc("PIT", (void*) pitHandler);
    rap = parseRap();

    LoadProcessT loadProcess = getRapAddr(&rap, "loadProcess");
    void (*runProcess)(const Process* proc) = getRapAddr(&rap, "runProcess");
    const ProcessList* (*getProcessList)(void) = getRapAddr(&rap, "getProcessList");
    procCtxtSwitch = getRapAddr(&rap, "procCtxtSwitch");
    getpid = getRapAddr(&rap, "getpid");
    //isInSyscall = getRapAddr(&rap, "isInSyscall");

    procList = getProcessList(); 
    const fd_t initList = open("/etc/init.lst");
    if (!initList) {
        printf("Sched: Could not find init list");
        exit(1);
    }
    const u32 initListFilesize = filesize(initList);
    char* const initListBuf = mmap(initListFilesize + 1);
    memset(initListBuf, 0, initListFilesize + 1);
    read(initList, initListBuf, initListFilesize);
    u32 initListIterator = 0;
    while (initListBuf[initListIterator]) {
        char* initialOffset = &initListBuf[initListIterator];
        while (initListBuf[initListIterator] != '\n') initListIterator++;
        // Right now initListBuf[initListIterator] == '\n'
        // loadProcess expects a const char* which is terminated by \0
        // Simply replace the newline with \0
        initListBuf[initListIterator] = '\0';
        printf("ProcessListEntry: '%s'", initialOffset);
        const u32 occurence = countOccurence(initialOffset, '$');
        if (occurence != 2) {
            printf("Invalid count of seperators ($) in /etc/init.lst: %u", occurence);
            exit(1);
        }
        char* const procPathBuf = mmap(strlenSpecChar(initialOffset, '$') + 1);
        memset(procPathBuf, 0, strlenSpecChar(initialOffset, '$') + 1);
        strcpyLen(procPathBuf, initialOffset, strlenSpecChar(initialOffset, '$'));
        printf("Process path: '%s'", procPathBuf);
        initialOffset += strlenSpecChar(initialOffset, '$') + 1; // Also skip seperator
        char* const procNameBuf = mmap(strlenSpecChar(initialOffset, '$') + 1);
        memset(procNameBuf, 0, strlenSpecChar(initialOffset, '$') + 1);
        strcpyLen(procNameBuf, initialOffset, strlenSpecChar(initialOffset, '$'));
        printf("ProcessName='%s'", procNameBuf);
        initialOffset += strlenSpecChar(initialOffset, '$') + 1; 
        if (strlen(initialOffset) != 3) {
            printf("Invalid length for last param, should be 3 (options:KRN/DRV/USR)");
            exit(1);
        }
        char privBuf[4] = {0};
        strcpy(privBuf, initialOffset);    
        ProcessPriveledgeLevel procPrivLvl;
        if (streq(privBuf, "KRN")) {
            procPrivLvl = PROC_PRIV_LVL_Kernel;
        }
        else if (streq(privBuf, "DRV")) {
            procPrivLvl = PROC_PRIV_LVL_Driver;
        }
        else if (streq(privBuf, "USR")) {
            procPrivLvl = PROC_PRIV_LVL_User;
        }
        else {
            printf("Invalid option for priveledge level: '%s'", privBuf);
            exit(1);
        }
        printf("Priv='%s'", privBuf);
        const Process* const proc = loadProcess(procPathBuf, procNameBuf, procPrivLvl);
        //runProcess(proc);

        munmap(procNameBuf);
        munmap(procPathBuf);
        initListBuf[initListIterator] = '\n';
        initListIterator++;
    }
    munmap(initListBuf);
    close(initList);

    u32 ticks = 0;
    for (;;) {
        ticks++;
        if (ticks % 0xA000000 == 0) {
            puts("Sched was called");
        }
    }
}

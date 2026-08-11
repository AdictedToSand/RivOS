#include <int.h>
#include <stdbool.h>
#include <stddef.h>

#include "sys/sys.h"
#include "sys/stdio.h"

#include "rap/rap.h"

#include "scheduling/pitHandler.h"

RapFile rap;
const ProcessList* procList = NULL;

typedef Process* (*LoadProcessT)(const char* fp, const char* pname, ProcessPriveledgeLevel privlvl);

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
    setFunc("PIT", pitHandler);
    rap = parseRap();

    LoadProcessT loadProcess = getRapAddr(&rap, "loadProcess");
    void (*runProcess)(const Process* proc) = getRapAddr(&rap, "runProcess");
    const ProcessList* (*getProcessList)(void) = getRapAddr(&rap, "getProcessList");
    //isInSyscall = getRapAddr(&rap, "isInSyscall");

    procList = getProcessList(); 

    const Process* const de = loadProcess("/krn/bin/de", "Vela", PROC_PRIV_LVL_Kernel);
    runProcess(de);
    for (;;) ;
}

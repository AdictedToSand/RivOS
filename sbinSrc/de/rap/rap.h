#pragma once
#include <int.h>

typedef struct RapParameter {
    const char* type;
    const char* name;
} RapParameter;

typedef struct RapEntry {
    const char* functionName;
    void* callLocation;
    u32 paramCount;
    RapParameter* paramArr;
} RapEntry;

typedef struct RapArr {
    u32 len;
    RapEntry* arr;
} RapArr;

typedef struct RapFile {
    RapArr entriesArr;
} RapFile;

RapFile parseRap(void);
void printRap(RapFile* rap);
void* getRapAddr(RapFile* rap, const char* func);

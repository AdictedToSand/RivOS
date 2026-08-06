#pragma once

typedef struct RapParameter {
    const char* type;
    const char* name;
} RapParameter;

typedef struct RapEntry {
    const char* functionName;
    void* callLocation;
    RapParameter* paramArr;
} RapEntry;

typedef struct RapFile {
    RapEntry* entriesArr;
} RapFile;

RapFile parseRap(void);
void printRap(RapFile* rap);

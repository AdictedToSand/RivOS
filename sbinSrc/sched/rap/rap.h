#pragma once

typedef struct RapEntry {
    const char* functionName;
    void* callLocation;
} RapEntry;

typedef struct RapFile {
    RapEntry* entries;
} RapFile;

RapFile parseRap(void);
void printRap(RapFile* rap);

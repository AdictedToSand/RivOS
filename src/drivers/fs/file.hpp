#pragma once
#include <stddef.h>

struct File {
    void* fsData;
    size_t size;
    bool exists;
};

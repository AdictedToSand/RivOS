#pragma once
#include <int.h>
#include <stddef.h>

#include "alloc.h"

typedef struct Vector {
    u32 typesize;
    void* arr;
    u32 len;
    u32 capac;
} Vector;

#define MIN_VEC_SIZE 10

static void vecInit(Vector* vec, u32 typesize) {
    vec->typesize = typesize;
    vec->len = 0;
    vec->capac = MIN_VEC_SIZE;
    vec->arr = alloc(typesize * vec->capac);
}

static u32 vecLen(Vector* vec) {
    return vec->len;
}

static void* vecAt(Vector* vec, u32 ind) {
    if (ind >= vec->len) {
        return NULL;
    }

    return (char*) vec->arr + (ind * vec->typesize);
}

static void vecGrow(Vector* vec) {
    u32 newCap = vec->capac * 2;

    void* newArr = alloc(newCap * vec->typesize);

    for (u32 i = 0; i < vec->len * vec->typesize; i++) {
        ((char*) newArr)[i] = ((char*) vec->arr)[i];
    }

    vec->arr = newArr;
    vec->capac = newCap;
}

static void vecPushBack(Vector* vec, void* item) {
    if (vec->len >= vec->capac) {
        vecGrow(vec);
    }

    char* dst = (char*) vec->arr + (vec->len * vec->typesize);
    char* src = (char*) item;

    for (u32 i = 0; i < vec->typesize; i++) {
        dst[i] = src[i];
    }

    vec->len++;
}

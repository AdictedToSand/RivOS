#pragma once
#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t n);

void* memset(void* b, int c, int len);
void* memmove(void* dst, const void* src, size_t count);

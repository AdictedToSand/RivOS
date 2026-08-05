#pragma once
#include <int.h>

typedef u32 fd_t;

extern fd_t open(const char* fp);
extern u8 write(fd_t fd, const char* conts, u32 len);
extern u8 claim(const char* mod);
extern u8 release(const char* mod);
extern u8 setFunc(const char* mod, void (*)(void));
extern void* mmap(u32 size);
extern void munmap(void* mem);
extern void exit(int exitcode);
extern u8 read(fd_t fd, char* obuf, u32 len);
extern void close(fd_t fd);
extern u32 filesize(fd_t fd);

extern fd_t stdout;
extern fd_t stdlog;

void sysInit(void);

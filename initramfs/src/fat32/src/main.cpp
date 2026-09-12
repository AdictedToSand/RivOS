#include <int.h>

struct FileData {
    u8 magic[5]; // Should be 'FATFS'
    bool exists; 
     
};

inline int liveSyscall3(int n, int a, int b, int c) {
    int ret;
    asm volatile("INT $0x30"
        : "=a"(ret)
        : "a"(n),"D"(a),"S"(b),"d"(c)
        : "memory"
    );
    return ret;
}

extern "C" {
    auto open(const char* fp) -> void* {
        liveSyscall3(3044, 0, 0, 0);       

        return nullptr;
    }
}

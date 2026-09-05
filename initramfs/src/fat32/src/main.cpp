#include <int.h>

struct FileData {
    u8 magic[5]; // Should be 'FATFS'
    bool exists; 
    
};

extern "C" {
    auto open() -> void* {
        volatile int x = 4;
        if (x == 5) {
            return (void*) 1;
        }
        return nullptr;
    }
}

#include <int.h>

struct FileData {
    u8 magic[5]; // Should be 'FATFS'
    bool exists; 
     
};

extern "C" {
    auto open() -> void* {
        return nullptr;
    }
}

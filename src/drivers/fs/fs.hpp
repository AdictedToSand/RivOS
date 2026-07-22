#pragma once


// Inherit from this class, make a global instance and call "addCandidate(&instance)" to add the driver as a candidate
struct FileSystemDriver {
    virtual auto init() -> void {
        // Do nothing
    }

    virtual auto getPriority() -> int {
        return -1;
    }


};

struct FileSystem {
    FileSystemDriver activeDriver;

    auto init() -> void {
        
    }
};

#pragma once
#include <drivers/fs/driver.hpp>

struct MountPoint {
    FileSystemDriver* fsDriver;
    
    virtual auto shouldActivate() -> bool = 0; // Fucking C++

    auto init() -> void {
        fsDriver->init();
    }
};

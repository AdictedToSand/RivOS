#pragma once
#include <stddef.h>

struct File {
    void* fsData;
    size_t size;
};

// Inherit from this class, make a global instance and call "addCandidate(&instance)" to add the driver as a candidate
struct FileSystemDriver {
    enum class SuccessCodes {
        Sucess,
        Error,
    };

    virtual auto init() -> void {
        // Do nothing
    }

    virtual auto getPriority() -> int {
        return 0;
    }

    virtual auto open(const char* fp) -> File {
        return {0, 0};
    }

    virtual auto write(File _f, char* _conts, size_t _len) -> SuccessCodes {
        return SuccessCodes::Error;
    }
    virtual auto read(File _f, char* _out, size_t _len) -> SuccessCodes {
        return SuccessCodes::Error;
    }
    virtual auto close(File _f) -> void {

    }
    virtual auto mkdir(const char* _fp) -> SuccessCodes {
        return SuccessCodes::Error;
    }
    virtual auto dirExists(const char* _fp) -> bool {
        return false;
    }
    virtual auto fileExists(const char* _fp) -> bool {
        return false;
    }

    virtual auto getDriverName() -> const char* {
        return ";";
    }
};

static inline FileSystemDriver defaultFsDriver;

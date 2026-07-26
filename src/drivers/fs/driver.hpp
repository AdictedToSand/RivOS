#pragma once
#include <stddef.h>

struct File {
    void* fsData;
    size_t size;
    bool exists;
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

    virtual auto open(const char* _fp, bool* _obufFileExists) -> File {
        (void) _fp; (void) _obufFileExists;
        return {};
    }

    virtual auto write(File _f, char* _conts, size_t _len) -> SuccessCodes {
        (void) _f; (void) _conts; (void) _len;
        return SuccessCodes::Error;
    }
    virtual auto read(File _f, char* _out, size_t _len) -> SuccessCodes {
        (void) _f; (void) _out; (void) _len;
        return SuccessCodes::Error;
    }
    virtual auto close(File _f) -> void {
        (void) _f; // Reduce warnings
    }
    virtual auto mkdir(const char* _fp) -> SuccessCodes {
        (void) _fp;
        return SuccessCodes::Error;
    }
    virtual auto dirExists(const char* _fp) -> bool {
        (void) _fp;
        return false;
    }
    virtual auto fileExists(const char* _fp) -> bool {
        (void) _fp;
        return false;
    }

    virtual auto getDriverName() -> const char* {
        return ";";
    }
    virtual auto free() -> void {

    }
};

static inline FileSystemDriver defaultFsDriver;

#pragma once
#include <drivers/fs/mountpoint.hpp>
#include <drivers/fs/driver.hpp>

#include <terminal/terminal.hpp>

/*struct FileSystemDriver {
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
        return "RivOS base driver";
    }
    virtual auto free() -> void {

    }
*/

struct DevMpDriver : FileSystemDriver {
    virtual auto init() -> void override {

    }
    virtual auto getPriority() -> int override {
        return 1;
    }

    auto close(File f) -> void override {
        
    }
    auto open(const char* fp, bool* obufFileExists) -> File override {
        *obufFileExists = true; // TODO

        Terminal::printf("Gotcha bitches\n");

        File ret;
        ret.size = 0;

        return ret;
    }
    virtual auto mkdir(const char* fp) -> SuccessCodes override {
        return FileSystemDriver::SuccessCodes::Error;
    }
    auto dirExists(const char* dp) -> bool override {
        return false; // TODO
    }
    auto fileExists(const char* fp) -> bool override {
        return false; // TODO
    }
    auto getDriverName() -> const char* override {
        return "RivOSFS_DeviceDriver";
    }
};

static inline DevMpDriver devDriver;

struct DevMp : MountPoint {
    auto shouldActivate() -> bool {
        return true;
    }
    DevMp() {
        fsDriver = &devDriver;
    }
};

static inline DevMp devMp;

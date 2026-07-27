#pragma once
#include <drivers/fs/mountpoint.hpp>
#include <drivers/fs/driver.hpp>

#include <terminal/terminal.hpp>

#include <gen/map.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <cstring.hpp>

struct DevSubDriver {
    virtual auto read(char* obuf, size_t len) -> FileSystemDriver::SuccessCodes = 0;
    virtual auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes = 0;
    virtual auto init() -> void = 0;
};

struct StdoutDevSubDriver : DevSubDriver {
    char stdout[4096];

    auto init() -> void override {
        Terminal::printf("I was called yay\n");
        memset(stdout, 0, 4096);
    }
    auto read(char* const obuf, const size_t len) -> FileSystemDriver::SuccessCodes override {
        for (size_t i = 0; i < len; i++) obuf[i] = stdout[i];

        return FileSystemDriver::SuccessCodes::Sucess;
    }
    auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes override {
        for (size_t i = 0; i < len; i++) stdout[i] = conts[i];
        
        Terminal::write(conts, len);

        return FileSystemDriver::SuccessCodes::Sucess;
    }
};

struct StrOperatorEquals {
    const char* conts;
    auto operator==(const char* lhs) const {
        return streq(conts, lhs);
    }
    inline operator const char*() const  {
        return conts;
    }
    StrOperatorEquals(const char* s) { conts = s; }
    StrOperatorEquals() { conts = nullptr; }
}; // Do not ask

static inline StdoutDevSubDriver stdoutDriver;

struct DevMpDriver : FileSystemDriver {
private:
    static inline Map<StrOperatorEquals, DevSubDriver*> mappings;
    static inline bool generalBool;

    struct FsData {
        char* fp;
    };
public:

    virtual auto init() -> void override {
        mappings["stdout"] = &stdoutDriver;

        for (auto& kv : mappings) {
            kv.getv()->init();
        }
    }
    virtual auto getPriority() -> int override {
        return 1;
    }

    auto close(File f) -> void override {
        KernelAllocator::free(((FsData*) f.fsData)->fp);
        KernelAllocator::free(f.fsData);
    }
    auto read(File f, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes override {
        if (!mappings.exists(((FsData*) f.fsData)->fp)) {
            return FileSystemDriver::SuccessCodes::Error;
        }

        return mappings[((FsData*) f.fsData)->fp]->read(obuf, len);
    }
    auto write(File f, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes override {
        // We do not get to use Map.exists() as that checks == and not streq
        const char* const fp = (((FsData*) f.fsData)->fp);
        if (!mappings.exists(fp)) return FileSystemDriver::SuccessCodes::Error;

        return mappings[fp]->write(obuf, len);
    }
    auto open(const char* fp, bool* obufFileExists) -> File override {
        *obufFileExists = true; // TODO

        fp += strlen("/dev/");

        File ret;
        ret.size = 0;
        ret.fsData = KernelAllocator::alloc(sizeof(FsData));
        ((FsData*) ret.fsData)->fp = (char*) KernelAllocator::alloc(strlen(fp) + 1);
        strcpy(((FsData*) ret.fsData)->fp, fp);

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
        return "RivOSFs_DeviceDriver";
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

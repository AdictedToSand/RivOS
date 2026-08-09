#pragma once
#include <drivers/fs/mountpoint.hpp>
#include <drivers/fs/driver.hpp>

#include <terminal/terminal.hpp>

#include <gen/map.hpp>
#include <gen/serial.hpp>

#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <cstring.hpp>
#include <str.hpp>

struct DevSubDriver {
    virtual auto read(char* obuf, size_t len) -> FileSystemDriver::SuccessCodes = 0;
    virtual auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes = 0;
    virtual auto init() -> void = 0;
};

struct StdoutDevSubDriver : DevSubDriver {
    char stdout[4096];

    auto init() -> void override {
        memset(stdout, 0, 4096);
    }
    auto read(char* const obuf, const size_t len) -> FileSystemDriver::SuccessCodes override {
        for (size_t i = 0; i < len && i < 4096; i++) obuf[i] = stdout[i];

        return FileSystemDriver::SuccessCodes::Sucess;
    }
    auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes override {
        for (size_t i = 0; i < len && i < 4096; i++) stdout[i] = conts[i];
        
        Terminal::write(conts, len);
        Serial::logf("[STDOUT WRITE]: %s", conts);

        return FileSystemDriver::SuccessCodes::Sucess;
    }
};
struct StdinDevSubDriver : DevSubDriver {
    char stdin[4096];
    u16 cursor;

    auto init() -> void override {
        cursor = 0;
        memset(stdin, 0, 4096);
    }
    auto read(char* const obuf, const size_t len) -> FileSystemDriver::SuccessCodes override {
        for (size_t i = 0; i < len && i < 4096; i++) obuf[i] = stdin[i];

        return FileSystemDriver::SuccessCodes::Sucess;
    }
    auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes override {
        if (len != 1) return FileSystemDriver::SuccessCodes::Error; // User input is one at a time

        if (cursor + 1 >= 4096) {
            memset(stdin, 0, 4096);
            cursor = 0;
        }
        stdin[cursor++] = conts[0];

        return FileSystemDriver::SuccessCodes::Sucess;
    }
};
struct StdLogSubDriver : DevSubDriver {
    auto read(char* const obuf, const size_t len) -> FileSystemDriver::SuccessCodes override {
        (void) obuf; (void) len;
        return FileSystemDriver::SuccessCodes::Error;
    }
    auto write(const char* conts, size_t len) -> FileSystemDriver::SuccessCodes override {
        Str s = conts;
        *(char*) (&s[len]) = 0;

        return FileSystemDriver::SuccessCodes::Sucess;
    }
    auto init() -> void override {}
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

struct DevMpDriver : FileSystemDriver {
private:
    static inline StdoutDevSubDriver stdoutDriver;
    static inline StdinDevSubDriver stdinDriver;
    static inline StdLogSubDriver stdlogDriver;

    static inline Map<StrOperatorEquals, DevSubDriver*> mappings;
    static inline bool generalBool;

    struct FsData {
        char* fp;
    };
public:

    virtual auto init() -> void override {
        mappings.insert("stdout", &stdoutDriver);
        mappings.insert("stdin", &stdinDriver);
        mappings.insert("stdlog", &stdlogDriver);

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
    auto read(File f, char* const obuf, size_t len) -> FileSystemDriver::SuccessCodes override {
        if (!mappings.exists(((FsData*) f.fsData)->fp)) {
            return FileSystemDriver::SuccessCodes::Error;
        }

        return mappings[((FsData*) f.fsData)->fp]->read(obuf, len);
    }
    auto write(File f, char* obuf, size_t len) -> FileSystemDriver::SuccessCodes override {
        const char* const fp = (((FsData*) f.fsData)->fp);
        if (!mappings.exists(fp)) return FileSystemDriver::SuccessCodes::Error;

        return mappings[fp]->write(obuf, len);
    }
    auto open(const char* fp, bool* obufFileExists) -> File override {
        fp += strlen("/dev/");

        File ret;
        ret.size = 0;
        ret.fsData = KernelAllocator::alloc(sizeof(FsData));
        ((FsData*) ret.fsData)->fp = (char*) KernelAllocator::alloc(strlen(fp) + 1);
        strcpy(((FsData*) ret.fsData)->fp, fp);
        ret.exists = mappings.exists(fp);
        *obufFileExists = ret.exists;

        return ret;
    }
    virtual auto mkdir(const char* fp) -> SuccessCodes override {
        (void) fp;
        return FileSystemDriver::SuccessCodes::Error;
    }
    auto dirExists(const char* fp) -> bool override {
        return mappings.exists(fp);
    }
    auto fileExists(const char* fp) -> bool override {
        return mappings.exists(fp);
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

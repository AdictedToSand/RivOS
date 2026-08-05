#pragma once
#include <mem/alloc.hpp>

#include <gen/map.hpp>
#include <str.hpp>

#include <drivers/fs/dev/dev.hpp>
#include <drivers/fs/driver.hpp>
#include <drivers/fs/mountpoint.hpp>

// Functions for /krn/virt/func.rap
#include <sys/IDT/idt.hpp>
#include <vis/vis.hpp>

struct VirtKrnSubDriver {
    virtual auto read(char* obuf, u32 len) -> FileSystemDriver::SuccessCodes = 0;
    virtual auto init() -> void = 0;
};

struct RapFileSubDriver : VirtKrnSubDriver {
    struct Param {
        const char* type;
        const char* name;    
        Param(const char* itype, const char* iname) {
            type = itype;
            name = iname;
        }
    };
    struct Function {
        void* raw;
        const char* name;
        Vector<Param> paraml;

        auto toStr(Str& out) const -> void  {
            out += name;
            out += '(';

            for (const auto& param : paraml) {
                out += param.type;
                out += ' ';
                out += param.name;
                out += ' ';
            }
            //TODO: Yeah...
            if (paraml.size() == 0) out += ' ';
            out += ')';
            out += '=';
            Str tempIntegerStorage;
            tempIntegerStorage.fromu((u32) raw);
            out += tempIntegerStorage;
        }
        Function(void* iraw, const char* iname, Vector<Param> iparaml) {
            raw = iraw;
            name = iname;
            paraml = iparaml;
        }
    };
    Vector<Function> functions;
public:
    auto read(char* obuf, u32 len) -> FileSystemDriver::SuccessCodes override {
        u32 currentInd = 0;
        for (const auto& func : functions) {
            Str s;
            func.toStr(s);
            const char* cstr = s.toCStr();
            u32 slen = s.fstrlen();

            for (u32 i = 0; i < slen && currentInd < len; i++) {
                obuf[currentInd++] = cstr[i];
            }
            if (currentInd < len) {
                obuf[currentInd++] = '\n';
            }
        }
        return FileSystemDriver::SuccessCodes::Sucess;
    }
    auto init() -> void override {
        functions.pushBack(Function(
            (void*) Idt::setDescriptor,
            "idtSetDescriptor",
            {
                Param("u8", "vector"),
                Param("ptr", "isr"),
                Param("u8", "flags"),
            }
        ));
        functions.pushBack(Function(
            (void*) VisualsPidEnforced::putPixel,
            "putPixel",
            {
                Param("u32", "argb"),
                Param("u32", "x"),
                Param("u32", "y"),
            }
        ));
        functions.pushBack(Function(
            (void*) VisualsPidEnforced::getScreenWidth,
            "getScreenWidth",
            {}
        ));
        functions.pushBack(Function(
            (void*) VisualsPidEnforced::getFbPhysAddr,
            "getFbPhysAddr",
            {}
        ));
        functions.pushBack(Function(
            (void*) VisualsPidEnforced::getScreenHeight,
            "getScreenHeight",
            {}
        ));
        functions.pushBack(Function(
            (void*) VisualsPidEnforced::getFbSizeBytes,
            "getFbSizeBytes",
            {}
        ));
        /*functions.pushBack(Function(
            (void*) 
        ));*/
    }
};

struct VirtKrnDriver : FileSystemDriver {
private:
    Map<StrOperatorEquals, VirtKrnSubDriver*> mapping;

    static inline RapFileSubDriver rapfile;

    struct FsData {
        char* fp;
    };
public:
    auto getPriority() -> int override {
        return 1;
    }
    auto init() -> void override {
        mapping["func.rap"] = &rapfile;

        for (MapEntry<StrOperatorEquals, VirtKrnSubDriver*>& kv : mapping) {
            kv.getv()->init();
        }

    }
    auto close(File f) -> void override {
        KernelAllocator::free(((FsData*) f.fsData)->fp);
        KernelAllocator::free(f.fsData);
    }
    auto write(File _f, char* _src, u32 _len) -> FileSystemDriver::SuccessCodes override {
        (void) _f; (void) _src; (void) _len;
        return FileSystemDriver::SuccessCodes::Error;
    }
    auto read(File f, char* obuf, u32 len) -> FileSystemDriver::SuccessCodes override {
        const char* const fp = ((FsData*) f.fsData)->fp;

        if (!mapping.exists(fp)) return FileSystemDriver::SuccessCodes::Error;

        return mapping[fp]->read(obuf, len);    
    }

    auto open(const char* fp, bool* obufFileExists) -> File override {
        File ret;
        fp += strlen("/krn/virt/");

        bool exists = false;
        for (auto& kv : mapping) {
            if (kv.getk() == fp) { exists = true; break; }
        }
        if (!exists) {
            *obufFileExists = false;
            ret.exists = false;
            return ret;
        }
        *obufFileExists = true;
        ret.exists = true;
        ret.fsData = KernelAllocator::alloc(sizeof(FsData));
        ((FsData*) ret.fsData)->fp = (char*) KernelAllocator::alloc(strlen(fp) + 1);
        strcpy(((FsData*) ret.fsData)->fp, fp);

        return ret;
    }
    auto mkdir(const char* _fp) -> SuccessCodes override {
        (void) _fp;
        return SuccessCodes::Error; 
    }
    auto dirExists(const char* _fp) -> bool override {
        (void) _fp;
        return false;
    }
    auto fileExists(const char* fp) -> bool override {
        return mapping.exists(fp);
    }
    auto getDriverName() -> const char* override {
        return "RivOSFs_VirtKrn";
    }
};

static inline VirtKrnDriver virtKrnDriver;

struct VirtKrnMp : MountPoint {
    auto shouldActivate() -> bool {
        return true;
    }
    VirtKrnMp() {
        fsDriver = &virtKrnDriver;
    }
};

static inline VirtKrnMp virtKrnMp;

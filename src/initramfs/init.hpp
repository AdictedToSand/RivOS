#pragma once
#include <mem/alloc.hpp>
#include <mem/utils.hpp>

#include <gen/err.hpp>
#include <gen/conf/conf.hpp>

#include <sv.hpp>
#include <int.h>

#include <terminal/terminal.hpp>

#include <drivers/fs/rivfs/rivfs.hpp>

#include <initializer_list.hpp>
#include <array.hpp>

#include <proc/ELF/loader.hpp>

#include <initramfs/sysclayer.hpp>

#include <rap/rap.hpp>

#include <vfs/vfs.hpp>

extern "C" void liveSyscStub(void);

inline int liveSyscall3(int n, int a, int b, int c) {
    int ret;
    asm volatile("INT $0x30"
        : "=a"(ret)
        : "a"(n),"D"(a),"S"(b),"d"(c)
        : "memory"
    );
    return ret;
}

auto tinyStub(u32 p1, u32 p2, u32 p3) -> void;

struct InitRamFs {
private:
    struct [[gnu::packed]] ModuleTag {
        u32 type;
        u32 size;

        u32 modStart;
        u32 modEnd;
        char cmdline[];
    };
    struct [[gnu::packed]] MultibootTag {
        u32 type;
        u32 size;
    };
    static inline ModuleTag* initramfsTag;

    static inline RivFs* fs;
public:
    static auto init(u32 mbiAddr) -> void {
        Idt::setDescriptor(LIVE_DRIVER_SYSC_NUMBER, (void*) liveSyscStub, 0x8E);

        initramfsTag = nullptr;
        MultibootTag* mbTag = (MultibootTag*) (mbiAddr + 8);
        
        while (mbTag->type != 0) {
            if (mbTag->type == 3) {
                initramfsTag = (ModuleTag*) mbTag;
                break;
            }

            mbTag = (MultibootTag*) ((u8*) mbTag + ((mbTag->size + 7) & ~7));
        }

        if (!initramfsTag) {
            kpanic("Unable to find initramfs module!");
        }

        Serial::logf("CmdLine='%s'", initramfsTag->cmdline);

        if (!streq(initramfsTag->cmdline, "IS_INITRAMFS")) {
            kpanic("A module was found, however the command line does NOT eq IS_INITRAMFS. Please check for extra modules or whether /boot/initramfs.img exists (and/or is loaded somewhere.) NOTE: This is TODO");            
        }

        u8* data = (u8*) initramfsTag->modStart;
        u32 size = initramfsTag->modEnd - initramfsTag->modStart;

        fs = (RivFs*) KernelAllocator::alloc(sizeof(RivFs));
        if (!fs) {
            kpanic("MemAlloc failed on initramfs");
        }

        *fs = RivFs(data, size);

        if (!fs->checkCorrectness()) {
            kpanic("Corrrupt initramfs. Serial log might have more information as to why.");
        }

        auto drvfsDiritExpected = fs->getDirIt("/drv/fs");
        if (drvfsDiritExpected.isErr()) {
            kpanic("Corrupt initramfs: /drv/fs did not exist");
        }
        auto drvfsDirit = drvfsDiritExpected.valUnchecked();

        while (true) {
            auto expected = fs->dirItGetNextDir(drvfsDirit);

            if (expected.isErr()) {
                break;
            }
            auto dirdata = expected.valUnchecked();

            Serial::logf("DirName='%s'", dirdata.dirname);

            StringView tmpDrvSuffix = StringView(".drv");
            if (!dirdata.dirname.endsWith(tmpDrvSuffix)) {
                continue;
            }

            const u32 len = strlen("/drv/fs/") + dirdata.dirname.len + strlen("/conf.cfg") + 2;
            char* filenameBuf = (char*) KernelAllocator::alloc(len);
            if (!filenameBuf) {
                kpanic("Alloc failed");
            }
            memset(filenameBuf, 0, len);
            strcat(filenameBuf, "/drv/fs/");
            char* tmp = (char*) dirdata.dirname.toCStr();
            strcat(filenameBuf, tmp);
            KernelAllocator::free(tmp);
            strcat(filenameBuf, "/conf.cfg");

            Expected<RivFs::File> configFileExpected = fs->open(filenameBuf);

            if (configFileExpected.isErr()) {
                kpanic("Driver module did not contain /conf.cfg");
            }
            RivFs::File configFile = configFileExpected.valUnchecked();

            const u32 configFilesize = fs->filesize(configFile) + 1;
            char* buf = (char*) KernelAllocator::alloc(configFilesize);
            memset(buf, 0, configFilesize);
            fs->read(configFile, buf);

            Config conf;
            conf.src = buf;
            conf.parseSrc();
            if (conf.isErr) {
                kpanic("Config in driver was invalid!");
            }

            // lmao
            constexpr const char* const REQUIRED_FIELDS[] = {
                "gen.name",
                "gen.vers",
                "gen.execfp",
                "custom.fstype",
            };

            for (u32 i = 0; i < sizeof(REQUIRED_FIELDS) / sizeof(char*); i++) {
                if (!conf.mapping.exists(REQUIRED_FIELDS[i])) {
                    Serial::logf("MISSING_FIELD: '%s'", *REQUIRED_FIELDS[i]);
                    kpanic("Missing field in driver");
                }
            }

            char* const drvName   = conf.mapping["gen.name"].strVal.toCStr();
            char* const drvVers   = conf.mapping["gen.vers"].strVal.toCStr();
            char* const drvExecFp = conf.mapping["gen.execfp"].strVal.toCStr();
            char* const drvFsType = conf.mapping["custom.fstype"].strVal.toCStr();

            Serial::logf("Driver(%s): on %s at %s (type: %s)", drvName, drvVers, drvExecFp, drvFsType);

            Str drvFullFp = "/drv/fs/";
            char* _tmp = dirdata.dirname.toCStr();
            drvFullFp.add(_tmp);
            drvFullFp.add("/");
            drvFullFp.add(drvExecFp);
            KernelAllocator::free(_tmp);
            Serial::logf("Fp=%s", drvFullFp.toCStr());

            Expected<RivFs::File> expectedExec = fs->open(drvFullFp);
            if (expectedExec.isErr()) {
                kpanic("Executable file in driver was incorrect (did not exist!)");
            }
            const u32 elfFilesize = fs->filesize(expectedExec.valUnchecked());
            char* elfbuf = (char*) KernelAllocator::alloc(elfFilesize);
            fs->read(expectedExec.val(), elfbuf);

            ElfExecutable elfhdr;
            elfhdr.fromSrc(drvFullFp, elfbuf);

            if (!elfhdr.isValid()) {
                kpanic("Executable file in driver was incorrect (is not a valid ELF!)");
            }
            Str procname = "__DriverSystem_Fs_"; procname.add(drvName);
            if (!elfhdr.load(procname, ProcessPriveledgeLevel::Kernel)) kpanic("Unable to load ELF");

            conf.freeLeftover();

            Str rapFullFp = "/drv/fs/";
            rapFullFp += dirdata.dirname;
            rapFullFp += "/fn.rap";
            Serial::logf("(RapFp=%s", rapFullFp.toCStr());

            // fn.rap parsing
            Expected<RivFs::File> expectedRap = fs->open(rapFullFp);

            if (expectedRap.isErr()) {
                Serial::logf("Fp=%s", rapFullFp.toCStr());
                kpanic("File in driver was not found: fn.rap");
            }
            RivFs::File rap = expectedRap.valUnchecked();

            const u32 rapFilesize = fs->filesize(rap);
            char* rapBuf = (char*) KernelAllocator::alloc(rapFilesize + 1); 
            memset(rapBuf, 0, rapFilesize + 1);
            fs->read(rap, rapBuf);

            Terminal::printf("Rap=%s\n", rapBuf);

            RapFile rapf(rapBuf);
            rapf.parseFile();
            rapf.dump();

            Expected<RivFs::File> expectedFsConf = fs->open("/etc/fs.cfg");
            if (expectedFsConf.isErr()) {
                kpanic("No /etc/fs.cfg found, unable to mount fs");
            }
            RivFs::File fsConf = expectedFsConf.valUnchecked();
            const u32 fsConfLen = fs->filesize(fsConf) + 1;
            char* const fsConfBuf = (char*) KernelAllocator::alloc(fsConfLen);
            memset(fsConfBuf, 0, fsConfLen);
            fs->read(fsConf, fsConfBuf);
            VFS::initFromSrc(fsConfBuf);
            Str drvDirFp = "/drv/fs/";
            drvDirFp += dirdata.dirname;
            VFS::registerForFs("fat32", drvFullFp, fs);

            KernelAllocator::free(rapBuf);
            KernelAllocator::free(drvName);
            KernelAllocator::free(drvVers);
            KernelAllocator::free(drvExecFp);
            KernelAllocator::free(drvFsType);
            KernelAllocator::free(buf);
            KernelAllocator::free(filenameBuf);
        }

        liveSyscall3(0, 4, (u32) (void*) tinyStub, 0);

        liveSyscall3(4, 0xDEADBEEF, 0xAF, 0xEAF);
    }
};



#include <terminal/terminal.hpp>

#include <gen/err.hpp>
#include <gen/serial.hpp>
#include <gen/vec.hpp>
#include <gen/map.hpp>
#include <gen/reboot.hpp>
#include <gen/conf/conf.hpp>

#include <sys/IDT/idt.hpp>
#include <sys/GDT/gdt.hpp>
#include <sys/PIC/pic.hpp>
#include <sys/PIT/pit.hpp>
#include <sys/sysmod.hpp>

#include <mem/page.hpp>
#include <mem/alloc.hpp>
#include <mem/utils.hpp>
#include <mem/mmu/mmu.hpp>

#include <drivers/storage/driver.hpp>
#include <drivers/storage/storage.hpp>

#include <drivers/fs/ext2/ext2.hpp>
#include <drivers/fs/FAT32/fat.hpp>
#include <drivers/fs/dev/dev.hpp>
#include <drivers/fs/fs.hpp>
#include <drivers/fs/file.hpp>
#include <drivers/interrupts/init.hpp>

#include <vis/vis.hpp>

#include <proc/ELF/loader.hpp>
#include <proc/process.hpp>

#include <str.hpp>
#include <bootInfo.hpp>
#include <kinfo.hpp>

#include <PCI/pci.hpp>

typedef void (*ctor_t)();

extern "C" ctor_t ctorsStart[];
extern "C" ctor_t ctorsEnd[];

void callGlobalConstructors() {
    // .ctors is stored in REVERSE order, walk backwards
    for (ctor_t* ctor = ctorsEnd - 1; ctor >= ctorsStart; ctor--) {
        if (*ctor != (ctor_t)-1) { // skip sentinel if present
            (*ctor)();
        }
    }
}

//TODO: i32 inst of int32_t
//TODO: some files still use <returnType> fn(...) instd of auto fn(...) -> <returnType>

static constexpr u32 FRAME_SIZE = 4096;
static constexpr u32 ASSUMED_MEM_BYTES = 128 * 1024 * 1024; // 128 MiB -- quick & dirty, replace with real detection later
static constexpr u32 MAX_FRAMES = ASSUMED_MEM_BYTES / FRAME_SIZE;
static u8 frameBitmapStorage[MAX_FRAMES / 8]; 

extern char kernelStart[];
extern char kernelEnd[];

//TODO: less CLI
extern "C" auto kernelMain(u32 magic, u32 mbiAddr) -> void {
    Gdt::init();
    Idt::init();

    Visuals::init(mbiAddr);

    asm volatile ("CLI");
    if (magic == 0x36D76289) {
        bootinfo.bootloader = BootloaderKinds::GRUB;
    }
    else {
        bootinfo.bootloader = BootloaderKinds::RivBoot;
    }
    Terminal::init();

    PhysicalFrameAllocator::init(ASSUMED_MEM_BYTES, frameBitmapStorage);
    PhysicalFrameAllocator::markUsed(0);
    for (u32 addr = 0; addr < (u32) kernelEnd; addr += 4096) // Mark all the other sections as already used
        PhysicalFrameAllocator::markUsed(addr / 4096);

    kassrt(Serial::init() == 0, "Unable to initalize serial");

    KernelAllocator::init();

    callGlobalConstructors();

    Storage::init();

    FileSystem::init();

    PCI::init();

    PIC::init();

    PIT::init(1000);

    Mmu::init();
    
    {
        u32 start = Visuals::getFbPhysAddr() & ~0xFFF;
        u32 end   = (Visuals::getFbPhysAddr() + Visuals::getFbSizeBytes() + 0xFFF) & ~0xFFF;
        Serial::logf("Framebuffer: addr=%x pitch=%u width=%u height=%u size=%x mapped=[%x,%x)", 
            Visuals::getFbPhysAddr(), Visuals::getPitch(), Visuals::getScreenWidth(), Visuals::getScreenHeight(),
            Visuals::getFbSizeBytes(), start, end);
        for (u32 addr = start; addr < end; addr += 4096)
            Mmu::mapPage((void*) addr, (void*) addr, Mmu::FLAGS_WRITABLE);
    }

    for (u32 addr = (u32) heapStart & ~0xFFF; addr < (u32) heapEnd; addr += 4096) {
        Mmu::mapPage((void*) addr, (void*) addr, Mmu::FLAGS_WRITABLE);
    }
    SysModuleHandler::init();

    HardwareInterrupts::init();

    Config initConf = {};
    initConf.fromFile("/krn/init.cfg");
    if (initConf.isErr) {
        Serial::logf("Invalid /krn/init.cfg. Doing nothing. (Err=%s)", initConf.o_err);
        goto startScheduler;
    }
    initConf.parseSrc();
    if (initConf.isErr) {
        Serial::logf("Invalid /krn/init.cfg. Doing nothing. (Err=%s)", initConf.o_err);
        goto startScheduler;
    }

startScheduler:
    initConf.freeLeftover();
    Map<StrOperatorEquals, void (*)(Config::Value)> initKeyMapping = {
        {"kernel.disableTerm", [](Config::Value v) -> void {
            if (v.state != Config::Value::States::Bool) {
                Serial::logf("kernel.disableInitTerm: invalid option");
                return;
            }
            if (v.boolVal) {
                Terminal::disable();
            }
        }},
        {"kernel.osname", [](Config::Value v) -> void {
            if (v.state != Config::Value::States::String) {
                Serial::logf("kernel.osname: invalid option");
                return;
            }
            osname = v.strVal.toCStr();
        }},
        {"kernel.bitness", [](Config::Value v) -> void {
            if (v.state != Config::Value::States::Int) {
                Serial::logf("kernel.bitness: invalid option");
                return;
            }
            bitness = v.intVal;
        }},
        {"kernel.dumpInitInfo", [](Config::Value v) -> void {
            if (v.state != Config::Value::States::Bool) { return; }
            if (v.boolVal) Serial::logf("Bootloader: '%s', Bitness: %u, osname: '%s', architecture: '%s'",
                bootinfo.bootloader == BootloaderKinds::GRUB ? "GRUB" : "RivBoot",
                bitness, osname, architecture);
        }},
    };
    for (auto& kv : initConf.mapping) {
        if (initKeyMapping.exists(kv.getk())) {
            initKeyMapping[kv.getk()](kv.getv());
        }
    }
    Terminal::printf("Welcome to %s!\n", osname);
    Serial::logf("Welcome to %s!", osname);

    ElfExecutable scheduler;
    if (scheduler.fromFile("/krn/bin/sched")) { kpanic("Scheduler not found!!!"); }
    if (!scheduler.isValid()) { kpanic("Scheduler was not a valid ELF"); }
    Process* proc = scheduler.load("RivOS_Sched", ProcessPriveledgeLevel::Kernel);
    if (!proc) { kpanic("Unable to load ELF"); }
    proc->run(ProcessImportance::REQ);

    kpanic("End of kernel reached");
    for (;;) ;
}

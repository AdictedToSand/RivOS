#pragma once
#include <drivers/fs/dev/dev.hpp>

#include <int.h>

#include <gen/vec.hpp>

#include <proc/process.hpp>

enum class SysModuleId {
    Unknown,

    PIT,
    Framebuffer,
};

struct SysModule {
    SysModuleId id;
    const char* name;

    auto operator==(const SysModuleId& lhs) -> bool {
        return id == lhs;
    }

    auto operator==(const SysModule& lhs) -> bool {
        return id == lhs.id;
    }

    SysModule(SysModuleId iid) {
        id = iid;

        if (id == SysModuleId::PIT) name = "PIT";
        else if (id == SysModuleId::Framebuffer) name = "Framebuffer";
        else name = "Unknown";
    }

    SysModule() {
        name = "Unknown";
        id = SysModuleId::Unknown;
    }
};

struct SysModuleHandler {
private:
    struct Entry {
        SysModule module;
        pid_t owner;
        void (*func)();
        u32* ownerDir;

        Entry(SysModule m) {
            ownerDir = nullptr;
            module = m;
            owner = 0;
            func = nullptr;
        }
    };

    static inline Vector<Entry> entries;

    static auto find(SysModuleId id) -> Entry* {
        for (auto& e : entries) {
            if (e.module.id == id)
                return &e;
        }

        return nullptr;
    }

public:
    static auto doNothing() -> void {}

    static auto init() -> void {
        entries.pushBack(Entry(SysModuleId::PIT));
        entries.pushBack(Entry(SysModuleId::Framebuffer));
    }

    static auto toSysMId(const StrOperatorEquals mod) -> SysModuleId {
        if (mod == "PIT") {
            return SysModuleId::PIT;
        }
        else if (mod == "Framebuffer") {
            return SysModuleId::Framebuffer;
        }

        return SysModuleId::Unknown;
    }

    static auto claim(const char* mod) -> u8 {
        SysModuleId modId = toSysMId(mod);

        if (modId == SysModuleId::Unknown)
            return 1;

        Entry* entry = find(modId);

        if (!entry || entry->owner)
            return 1;

        entry->owner = activeProcessPid;

        return 0;
    }

    static auto release(const char* mod) -> u8 {
        SysModuleId modId = toSysMId(mod);

        Entry* entry = find(modId);

        if (!entry)
            return 1;

        if (entry->owner != activeProcessPid)
            return 1;

        entry->owner = 0;
        entry->func = doNothing;

        return 0;
    }

    static auto setFunc(const char* mod, void (*func)()) -> u8 {
        SysModuleId modId = toSysMId(mod);
        if (modId == SysModuleId::Unknown)
            return 1;

        Entry* entry = find(modId);

        if (!entry)
            return 1;

        if (entry->owner != activeProcessPid)
            return 1;
        entry->func = func;
        entry->ownerDir = Mmu::activeDirectory;

        return 0;
    }

    static auto getFunc(SysModuleId id) -> void (*)() {
        Entry* entry = find(id);

        if (!entry || !entry->func)
            return doNothing;

        return entry->func;
    }
    static auto getFuncEntry(SysModuleId id) -> Entry* {
        Entry* entry = find(id);
        if (!entry || !entry->func) return nullptr;
        return entry;
    }

    static auto getowner(SysModuleId id) -> pid_t {
        Entry* entry = find(id);

        if (!entry)
            return 0;

        return entry->owner;
    }
};

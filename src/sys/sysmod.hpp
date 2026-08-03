#pragma once
#include <drivers/fs/dev/dev.hpp>

#include <int.h>

#include <gen/map.hpp>

#include <proc/process.hpp>

enum class SysModuleId {
    Unknown,

    PIT,

    Keyboard_PS2,
};

struct SysModule {
    SysModuleId id;
    const char* name;

    auto operator==(const SysModuleId& lhs) -> bool const {
        return id == lhs;
    }
    auto operator==(const SysModule& lhs) const -> bool {
        return id == lhs.id;
    }
    SysModule(SysModuleId iid) {
        id = iid;
        if (id == SysModuleId::PIT) name = "PIT";
        else if (id == SysModuleId::Keyboard_PS2) name = "Keyboard_PS2";
        else name = "Unknown";
    }
    SysModule() {
        name = "Unknown";
        id = SysModuleId::Unknown;
    }
};

struct SysModuleHandler {
private:
    static inline Map<SysModule, pid_t> owners;
    static inline Map<SysModuleId, void(*)()> sysModuleFunctions;
public:
    static auto doNothing() -> void {}
    static auto toSysMId(const StrOperatorEquals mod) -> SysModuleId {
        if (mod == "PIT") {
            return SysModuleId::PIT;
        }
        else if (mod == "Keyboard_PS2") {
            return SysModuleId::Keyboard_PS2;
        }

        return SysModuleId::Unknown;
    } 
    static auto claim(const char* mod) -> u8 {
        SysModuleId modId = toSysMId(mod);

        if (modId == SysModuleId::Unknown) {
            return 1;
        }
        SysModule sysm = modId;

        if (owners.exists(sysm)) {
            return 1;
        }
        owners[sysm] = activeProcessPid; 

        return 0;
    }
    static auto release(const char* mod) -> u8 {
        const SysModuleId modid = toSysMId(mod);

        if (!owners.exists(modid)) return 1;
        if (owners[modid] != activeProcessPid) return 1;

        owners.rmkey(modid);

        if (sysModuleFunctions.exists(modid)) sysModuleFunctions[modid] = doNothing;

        return 0;
    }
    static auto setFunc(const char* mod, void (*func)()) -> u8 {
        SysModuleId modId = toSysMId(mod);
        if (modId == SysModuleId::Unknown) return 1;

        if (!owners.exists(modId)) return 1;
        if (activeProcessPid != owners[modId]) return 1;

        sysModuleFunctions[modId] = func;

        return 0;
    }
    static auto getFunc(SysModuleId id) -> void (*)() {
        if (!sysModuleFunctions.exists(id)) {
            return doNothing;
        }
        return sysModuleFunctions[id];
    }
};

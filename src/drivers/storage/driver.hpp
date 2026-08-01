#pragma once
#include <stdint.h>
#include <stddef.h>

struct StorageDriver {
    enum class SuccessCodes {
        Sucess,
        Error,
    };

    virtual int getPriority() {
        return -1;
    }

    virtual auto readSector(uint16_t* _buf, size_t _buflen, size_t _sector) -> SuccessCodes {
        (void) _buf; (void) _buflen; (void) _sector; // Make compiler happi
        return SuccessCodes::Error;
    }
    virtual auto writeSector(uint16_t* _conts, size_t _len, size_t _sector) -> SuccessCodes {
        (void) _conts; (void) _len; (void) _sector;
        return SuccessCodes::Error;
    }

    virtual auto init() -> SuccessCodes {
        return SuccessCodes::Error;
    }
    virtual auto getDriverName() -> const char* {
        return "RivOS_DefaultStorageDriver";
    }
    virtual auto selectNextDrive() -> void {

    };
};

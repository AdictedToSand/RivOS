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

    virtual SuccessCodes readSector(uint16_t* _buf, size_t _buflen, size_t _sector) {
        (void) _buf; (void) _buflen; (void) _sector; // Make compiler happi
        return SuccessCodes::Error;
    }
    virtual SuccessCodes writeSector(uint16_t* _conts, size_t _len, size_t _sector) {
        (void) _conts; (void) _len; (void) _sector;
        return SuccessCodes::Error;
    }

    virtual SuccessCodes init() {
        return SuccessCodes::Error;
    }
    virtual const char* getDriverName() {
        return "DefaultTemplate";
    }
};

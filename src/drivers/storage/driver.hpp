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

    virtual SuccessCodes readSector(uint16_t* buf, size_t buflen, size_t sector) {
        return SuccessCodes::Error;
    }
    virtual SuccessCodes writeSector(uint16_t* conts, size_t len, size_t sector) {
        return SuccessCodes::Error;
    }

    virtual SuccessCodes init() {
        return SuccessCodes::Error;
    }
    virtual const char* getDriverName() {
        return "DefaultTemplate";
    }
};

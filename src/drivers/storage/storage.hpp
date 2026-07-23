#pragma once
#include <drivers/storage/ATA_PIO/ata_pio.hpp>
#include <drivers/storage/driver.hpp>

#include <gen/vec.hpp>

#include <stddef.h>
#include <stdint.h>

static inline StorageDriver defaultStorageDriver;

struct Storage {
private:
    static inline StorageDriver* winner;
    static inline Vector<StorageDriver*> candidates;

    static inline auto registerDriver(StorageDriver* driver) -> void {
        candidates.pushBack(driver);
    }

public:
    static constexpr size_t SECTOR_SIZE = 512;

    static auto init() -> void {
        candidates = Vector<StorageDriver*>();

        winner = &defaultStorageDriver;

        registerDriver(&atapio);

        for (const auto& driver : candidates) {
            if (driver->getPriority() > winner->getPriority()) {
                winner = driver;
            }
        }

        Terminal::printf("Storage driver: %s\n", winner->getDriverName());
    }

    static auto readSector(uint16_t* buf, size_t len, size_t sector) -> StorageDriver::SuccessCodes {
        return winner->readSector(buf, len, sector);
    }
    static auto writeSector(uint16_t* buf, size_t len, size_t sector) -> StorageDriver::SuccessCodes {
        return winner->writeSector(buf, len, sector);
    }
};

// Credits: https://wiki.osdev.org/ATA_PIO_Mode

#pragma once
#include <stdint.h>

#include <drivers/storage/driver.hpp>

#include <gen/err.hpp>
#include <gen/io.hpp>

struct AtaPioStorageDriver : StorageDriver {
    static constexpr uint16_t DATA_PORT = 0x1F0;
    static constexpr uint16_t SECTOR_COUNT_PORT = 0x1F2;
    static constexpr uint16_t LBA_LO_PORT = 0x1F3;
    static constexpr uint16_t LBA_MI_PORT = 0x1F4;
    static constexpr uint16_t LBA_HI_PORT = 0x1F5;
    static constexpr uint16_t DRIVE_SELECT_PORT = 0x1F6;
    static constexpr uint16_t COMMAND_PORT = 0x1F7;
    static constexpr uint16_t STATUS_PORT = 0x1F7;

    static constexpr uint8_t  COMMAND_IDENTIFY = 0xEC;
    static constexpr uint8_t  COMMAND_READ_SECTORS = 0x20;

    static constexpr uint16_t DRIVE_SELECT_MASTERDRIVE = 0xE0;
    static constexpr uint16_t DRIVE_SELECT_SLAVEDRIVE = 0xF0;

    static constexpr uint8_t  STATUS_BSY = 0x80;
    static constexpr uint8_t  STATUS_DRQ = 0x08;

    int getPriority() override {
        return 1; // Todo: change...
    }

    StorageDriver::SuccessCodes readSector(uint16_t* buf, size_t buflen, size_t sector) override {
        outb(DRIVE_SELECT_PORT, DRIVE_SELECT_MASTERDRIVE | ((sector >> 24) & 0x0F));

        // ~400ns delay
        // This is recommended.
        inb(STATUS_PORT);
        inb(STATUS_PORT);
        inb(STATUS_PORT);
        inb(STATUS_PORT);

        outb(SECTOR_COUNT_PORT, 1);
        outb(LBA_LO_PORT,  sector & 0xFF);
        outb(LBA_MI_PORT, (sector >> 8) & 0xFF);
        outb(LBA_HI_PORT, (sector >> 16) & 0xFF);
        outb(COMMAND_PORT, 0x20); 

        if (inb(STATUS_PORT) == 0) {
            kpanic("Drive did not exist");
        }
        while (inb(STATUS_PORT) & 0x80) ;
        while (!(inb(STATUS_PORT) & 0x08)) ;

        for (uint16_t i = 0; i < 256 && i < buflen; i++) {
            buf[i] = inw(DATA_PORT);
        }
        return StorageDriver::SuccessCodes::Sucess;
    }

    const char* getDriverName() override {
        return "RivOS_ATAPIO";
    }
};

static inline AtaPioStorageDriver atapio;


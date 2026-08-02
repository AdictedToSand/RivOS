// Credits: https://wiki.osdev.org/ATA_PIO_Mode

#pragma once
#include <stdint.h>

#include <drivers/storage/driver.hpp>

#include <gen/err.hpp>
#include <gen/io.hpp>
#include <cstring.hpp>

#include <mem/alloc.hpp>

#include <bootInfo.hpp>

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

    uint8_t driveSelect = DRIVE_SELECT_MASTERDRIVE;

    auto getPriority() -> int override {
        return 1; // Todo: change...
    }

    auto init() -> StorageDriver::SuccessCodes override {
        uint16_t* buf = (uint16_t*) KernelAllocator::alloc(512);

        readSector(buf, 256, 0);
        
        buf += 1; // The first instruction is JMP _start which will be 2 bytes, since we're using a uint16_t 1 increment is needed
        // Afterwards, if it is the boot code, a "RivBoot" signature will be there

        if (streq((char*) buf, "RivBoot")) {
            // The drive contains the RivBoot bootcode
            // It should contain the filesystem, so we ignore it
            // If you want to get access to the boot drive, use selectNextDrive()
            driveSelect = DRIVE_SELECT_SLAVEDRIVE;
        }
        if (bootinfo.bootloader == BootloaderKinds::GRUB) {
            // TODO: Add proper logic here (currently it works but not good!)
        }

        KernelAllocator::free(buf);
        return StorageDriver::SuccessCodes::Sucess;
    }

    auto readSector(uint16_t* buf, size_t buflen, size_t sector) ->  StorageDriver::SuccessCodes override {
        outb(DRIVE_SELECT_PORT, driveSelect | ((sector >> 24) & 0x0F));

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

        for (u16 i = 0; i < 256 && i < buflen; i++) {
            buf[i] = inw(DATA_PORT);
        }
        return StorageDriver::SuccessCodes::Sucess;
    }
    auto writeSector(u16* conts, size_t len, size_t sector) -> StorageDriver::SuccessCodes override {
        u16 sectorBuf[256];

        if (len < 512) {
            readSector(sectorBuf, 512, sector);

            u8* dst = (u8*) sectorBuf;
            u8* src = (u8*) conts;

            for (size_t i = 0; i < len; i++) {
                dst[i] = src[i];
            }

            conts = sectorBuf;
            len = 512;
        }

        outb(DRIVE_SELECT_PORT, driveSelect | ((sector >> 24) & 0x0F));

        inb(STATUS_PORT);
        inb(STATUS_PORT);
        inb(STATUS_PORT);
        inb(STATUS_PORT);

        outb(SECTOR_COUNT_PORT, 1);
        outb(LBA_LO_PORT, sector & 0xFF);
        outb(LBA_MI_PORT, (sector >> 8) & 0xFF);
        outb(LBA_HI_PORT, (sector >> 16) & 0xFF);

        outb(COMMAND_PORT, 0x30); // WRITE SECTORS

        while (inb(STATUS_PORT) & 0x80);
        while (!(inb(STATUS_PORT) & 0x08));

        for (u16 i = 0; i < 256; i++) {
            outw(DATA_PORT, conts[i]);
        }

        while (inb(STATUS_PORT) & 0x80);

        if (inb(STATUS_PORT) & 0x01) {
            kpanic("ATA write error.");
        }

        return StorageDriver::SuccessCodes::Sucess;
    }
    auto getDriverName() -> const char* override {
        return "RivOS_ATAPIO";
    }
    auto selectNextDrive() -> void override {
        if (driveSelect == DRIVE_SELECT_SLAVEDRIVE) {
            driveSelect = DRIVE_SELECT_MASTERDRIVE;
        }
        else driveSelect = DRIVE_SELECT_SLAVEDRIVE;
    }
};

static inline AtaPioStorageDriver atapio;


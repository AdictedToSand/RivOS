// Credits: http://wiki.osdev.org/PCI
#pragma once
#include <int.h>

#include <gen/io.hpp>
#include <gen/vec.hpp>
#include <gen/serial.hpp>

struct PCI {
    struct PciVendor {
        uint16_t id;
        const char* name;
    };

    static inline Vector<PciVendor> vendors = {
        {0x8086, "Intel"},
        {0x10DE, "NVIDIA"},
        {0x1022, "AMD"},
        {0x1AF4, "Red Hat (virtio)"},
        {0x1234, "QEMU"},
    }; // Would make this PciVendor[] but C++...
    struct PciDevice {
        u16 deviceId;
        const char* name;
    };
public:
    struct FullPciDevice {
        PciDevice dev;
        PciVendor vend;

        u8 bus, slot;
    };
private:

    static inline Vector<PciDevice> devices = {
        {0x1237, "Intel 82441FX Host Bridge"},
        {0x7000, "Intel PIIX3 ISA Bridge"},
        {0x7010, "Intel PIIX3 IDE Controller"},
        {0x1111, "QEMU VGA Controller"},
        {0x100E, "Intel PRO/1000 Ethernet Controller"},
    };

    static constexpr u16 CONFIG_ADDR = 0xCF8;

    static constexpr u16 VENDOR_NONE = 0xFFFF;
private:
    static auto configReadWord(u8 bus, u8 slot, u8 func, u8 offset) -> u16 {
        u32 lbus = (u32) bus;
        u32 lslot = (u32) slot;
        u32 lfunc = (u32) func;
        u16 tmp = 0;

        u32 address = (u32) (lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xFC) | ((u32) 0x80000000);

        outl(CONFIG_ADDR, address);

        // Read in the data
        // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
        tmp = (uint16_t) ((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
        return tmp;
    }
    static auto getVendorFrom(u8 bus, u8 slot) -> u16 {
        u16 vendor = configReadWord(bus, slot, 0, 0);
        if (vendor == VENDOR_NONE) {
            return 0; // 0 means not found for now
        }
        return vendor;
    }
    static auto getDeviceFrom(u8 bus, u8 slot) -> u16 {
        if (!getVendorFrom(bus, slot))
            return 0;

        return configReadWord(bus, slot, 0, 2);
    }
    static auto getNameFromVendorId(u16 vendor) -> const char* {
        for (auto& v : vendors) {
            if (v.id == vendor) return v.name;
        }

        return nullptr;
    }
    static auto getNameFromDeviceId(u16 device) -> const char* {
        for (auto& d : devices) {
            if (d.deviceId == device) return d.name;
        }

        return nullptr;
    }

    static auto getAllPciDevices(Vector<FullPciDevice>& pciDeviceVec) -> void {
        for (u8 bus = 0; bus < 255; bus++) {
            for (u8 slot = 0; slot < 32; slot++) {
                const u16 vendor = getVendorFrom(bus, slot);
                const u16 device = getDeviceFrom(bus, slot);

                if (vendor != 0) {
                    FullPciDevice pciDev;
                    pciDev.vend.id = vendor;
                    pciDev.dev.deviceId = device; 
                    pciDev.bus = bus;
                    pciDev.slot = slot;

                    const char*& vname = pciDev.vend.name;
                    const char*& dname = pciDev.dev.name;
                    vname = getNameFromVendorId(vendor);
                    dname = getNameFromDeviceId(device);
                    if (!vname) {
                        vname = "Vendor not found";
                    }
                    if (!dname) {
                        dname = "Device not found";
                    }
                    Serial::logf("PCI device found, vendor=%s,device=%s", vname, dname);
                    pciDeviceVec.pushBack(pciDev);
                }
            }
        }    
    }
public:
    static inline Vector<FullPciDevice> pciDevices;

    static auto init() -> void {
        getAllPciDevices(pciDevices);
    }
};

#pragma once
#include <int.h>

// Why are there so many:(
static auto baseclassToString(u8 bcl) -> const char* {
    switch (bcl) {
        case 0x00: return "Unclassified";
        case 0x01: return "MassStorageControl";
        case 0x02: return "NetworkController";
        case 0x03: return "DisplayController";
        case 0x04: return "MultimediaController";
        case 0x05: return "MemoryController";
        case 0x06: return "Bridge";
        case 0x07: return "SimpleCommunicationController";
        case 0x08: return "BaseSystemPheripheral";
        case 0x09: return "InputDeviceController";
        case 0x0A: return "DockingStation";
        case 0x0B: return "Processor";
        case 0x0C: return "SerialBusController";
        case 0x0D: return "WirelessController";
        case 0x0E: return "IntelligentController";
        case 0x0F: return "SateliteCommunicationController";
        case 0x10: return "EncryptionController";
        case 0x11: return "SignalProcessController";
        case 0x12: return "ProcessingAccelerator";
        case 0x13: return "Non-EsentialInstrumentation";
        case 0x14: return "Reserved";
        case 0x40: return "Co-Processor";
        case 0x41: return "Reserved";
        case 0xFF: return "Unasigned";
        default: return "UnknownBaseClass";
    }
}

// :(
static auto subclassToString(u8 subcl, u8 basecl) -> const char* {
    if (subcl == 0x80) return "Other";
    switch (basecl) {
        case 0x00: return "Unclassified";

        case 0x01: {
            switch (subcl) {
                case 0x0: return "SCSI_BusController";
                case 0x1: return "IDE_Controller";
                case 0x2: return "FloppyDiskController";
                case 0x3: return "IPI_BusController";
                case 0x4: return "RAID_Controller";
                case 0x5: return "ATA_Controller";
                case 0x6: return "SerialAtaController";
                case 0x7: return "SerialAtachedSCSI_Controller";
                case 0x8: return "NonVolatile-MemoryController";
                default: return "Unknown";
            }
        }
        case 0x02: {
            switch (subcl) {
                case 0x0: return "EthernetController";
                case 0x1: return "TokenRingController";
                case 0x2: return "FDDI_Controller";
                case 0x3: return "ATM_Controller";
                case 0x4: return "ISDN_Controller";
                case 0x5: return "WorldFlipController";
                case 0x6: return "PICMG 2.14 MultiComputingController";
                case 0x7: return "InfinibandController";
                case 0x8: return "FabricController";
                default: return "Unknown";
            }
        }
        case 0x03: {
            switch (subcl) {
                case 0x0: return "VgaCompatibleController";
                case 0x1: return "XGA_Controller";
                case 0x2: return "3D_Controller";
                default: return "Unknown";
            }
        }
        case 0x04: {
            switch (subcl) {
                case 0x0: return "MultimediaVideoController";
                case 0x1: return "MultimediaAudioController";
                case 0x2: return "ComputerTelephonyDevice";
                case 0x3: return "AudioDevice";
                default: return "Unknown";
            }
        }
        case 0x05: {
            switch (subcl) {
                case 0x0: return "MemoryController";
                case 0x1: return "FlashController";
                default: return "Unknown";
            }
        }
        case 0x06: {
            switch (subcl) {
                case 0x0: return "HostBridge";
                case 0x1: return "ISA_Bridge";
                case 0x2: return "EISA_Bridge";
                case 0x3: return "MCA_Bridge";
                case 0x4: case 0x9: return "PCI-to-PCI_Bridge";
                case 0x5: return "PCMCIA_Bridge";
                case 0x6: return "NuBusBridge";
                case 0x7: return "CardBusBridge";
                case 0x8: return "RACEwayBridge";
                case 0xA: return "Infiniband-to-PCI_HostBridge";
                default: return "Unknown";
            }  
        }
        case 0x7: {
            switch (subcl) {
                case 0x0: return "SerialController";
                case 0x1: return "ParallelController";
                case 0x2: return "MultiportSerialController";
                case 0x3: return "Modem";
                case 0x4: return "IEEE 488.1/2 (GPIB) Controller";
                case 0x5: return "SmartCardController";
                default: return "Unknown";
            }
        }
        case 0x8: {
            switch (subcl) {
                case 0x0: return "PIC";
                case 0x1: return "DMA_Controller";
                case 0x2: return "Timer";
                case 0x3: return "RTC_Controller";
                case 0x4: return "PCI_Hot-PlugController";
                case 0x5: return "SD_HostController";
                case 0x6: return "IOMMU";
                default: return "Unknown";
            }
        }
        case 0x9: {
            switch (subcl) {
                case 0x0: return "KeyboardController";
                case 0x1: return "DigitizerPen";
                case 0x2: return "MouseController";
                case 0x3: return "ScannerController";
                case 0x4: return "GamePortController";
                default: return "Unknown";
            }
        }
        case 0x0A: {
            switch (subcl) {
                case 0x0: return "Generic";
                default: return "Unknown";
            }
        }

        default: return "UnknownBaseClass";
    }
}

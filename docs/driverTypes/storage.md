# Storage drivers

Allow communication between storage media via a few generic functions.

## Custom

In the .cfg of the driver, inside the [custom] section, should be specified:
~~~ini
[custom]
# Kind of media (HDD, SATA or NVMe)
# mediaKind="HDD|SATA|NVMe"
mediaKind="HDD"
~~~

Functions: 

## Functions required

~~~Rust
// Returns a pointer to a Drives structure defined as
/*
struct Drives __packed__ {
    u32 driveam;
    enum class DriveKind : u8 {
        HDD,
        SATA,
        NVMe,
    } drivekind[driveam];
}
*/
getdrives( )

// A driveid is defined as n where n < getdrives().driveam
drivesel(u32 driveid )
// Reads a sector with max size len into buf. If len > sectorsize extend to the next sector
readSector(u32 sector ptr buf u32 len )
// Writes buf to a sector, if len > sectorSize then extend to the next sector
writeSector(u32 sector ptr buf u32 len)
~~~
import struct

with open("out.txt", "rb") as f:
    data = f.read()

# RivFsHeader:
# char[5], u32, u32, u8[2], u32
magic, fph_size, fph_pos, major, minor, flags = struct.unpack_from("<5sII2BI", data, 0)

print("RivFsHeader:")
print(f"  magic:   {magic.decode()}")
print(f"  fphSize: 0x{fph_size:X} ({fph_size})")
print(f"  fphPos:  0x{fph_pos:X} ({fph_pos})")
print(f"  version: {major}.{minor}")
print(f"  flags:   0x{flags:08X}")

# FilePositionHeader:
# u16, u16, u32, u32, u32
dir_hdr_am, f_am, dirstart, fstart, fph_flags = struct.unpack_from(
    "<HHIII", data, fph_pos
)

print("\nFilePositionHeader:")
print(f"  dirHdrAm: {dir_hdr_am}")
print(f"  fAm:      {f_am}")
print(f"  dirstart: 0x{dirstart:X} ({dirstart})")
print(f"  fstart:   0x{fstart:X} ({fstart})")
print(f"  flags:    0x{fph_flags:08X}")

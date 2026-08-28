#!/usr/bin/env python3
"""
rivfs_parser.py -- parser for the RivFs filesystem image format.

Usage:
    python rivfs_parser.py <input.img>
    python rivfs_parser.py <input.img> --extract /path/in/fs [output_file]

See the accompanying notes for a list of ambiguities in the spec and the
interpretation this parser assumes for each of them.
"""

import struct
import sys
import os
from dataclasses import dataclass, field
from typing import List, Optional

MAGIC = b"rivfs"

FLGS_NONE = 0
FLGS_RONLY = 1 << 0

# Fixed size (in bytes) of the non-name portion of a File Position Header /
# directory header: dirhdrAm(u16) + fAm(u16) + dirstart(u32) + fstart(u32) + flgs(u32)
FPH_FIXED_SIZE = 2 + 2 + 4 + 4 + 4  # = 16


class RivFsError(Exception):
    pass


class RivFsCorrupt(RivFsError):
    pass


@dataclass
class RivFsHeader:
    magic: bytes
    fph_size: int
    fph_pos: int
    vers_major: int
    vers_minor: int
    flags: int


@dataclass
class FileEntry:
    name: str
    offset: int            # offset of this entry's fpSv.len field (start of entry)
    filesize: int
    content_offset: int    # offset of the raw content bytes


@dataclass
class DirEntry:
    name: str               # "/" for the implicit root
    offset: int              # offset of this entry's dnSv.len field ("/" has none -> == header.fph_pos)
    dirhdr_am: int
    f_am: int
    dirstart: int
    fstart: int
    flags: int
    dirs: List["DirEntry"] = field(default_factory=list)
    files: List[FileEntry] = field(default_factory=list)


class RivFsImage:
    def __init__(self, path: str):
        with open(path, "rb") as f:
            self.data = f.read()
        self.header = self._parse_header()
        self.root = self._parse_root()

    # ---- little-endian primitive readers -------------------------------

    def _u8(self, off: int) -> int:
        self._check_bounds(off, 1)
        return self.data[off]

    def _u16(self, off: int) -> int:
        self._check_bounds(off, 2)
        return struct.unpack_from("<H", self.data, off)[0]

    def _u32(self, off: int) -> int:
        self._check_bounds(off, 4)
        return struct.unpack_from("<I", self.data, off)[0]

    def _check_bounds(self, off: int, size: int):
        if off < 0 or off + size > len(self.data):
            raise RivFsCorrupt(
                f"read of {size} byte(s) at offset {off} is out of bounds "
                f"(image size {len(self.data)})"
            )

    # ---- header ----------------------------------------------------------

    def _parse_header(self) -> RivFsHeader:
        # magic(5) + fphSize(4) + fphPos(4) + vers(2) + flgs(4) = 19 bytes
        if len(self.data) < 19:
            raise RivFsCorrupt("file too small to contain a rivfs header")

        magic = self.data[0:5]
        if magic != MAGIC:
            raise RivFsCorrupt(f"bad magic {magic!r}, expected {MAGIC!r}")

        fph_size = self._u32(5)
        fph_pos = self._u32(9)
        vers_major = self._u8(13)
        vers_minor = self._u8(14)
        flags = self._u32(15)

        if (vers_major, vers_minor) != (0, 0):
            sys.stderr.write(
                f"warning: image reports version {vers_major}.{vers_minor}; "
                f"this parser only implements the version 0.0 layout and may "
                f"misparse newer structures\n"
            )

        # Per spec: "If FLGS_RONLY is off, assume the filesystem is corrupt."
        if not (flags & FLGS_RONLY):
            raise RivFsCorrupt(
                "FLGS_RONLY is not set on the header -- per spec this image "
                "must be treated as corrupt"
            )

        return RivFsHeader(magic, fph_size, fph_pos, vers_major, vers_minor, flags)

    # ---- FPH / directory walking ------------------------------------------

    def _read_name(self, off: int):
        """
        Read a length-prefixed name (the fpSv / dnSv structure):
            len: u32
            conts: char[len]
        Returns (name_bytes, offset_after_name).
        If len == 0 this is an EOF marker: returns (None, off + 4), and per
        spec the entry is *only* the 4-byte len field -- no further data
        ("Conts thus does not exist!").
        """
        length = self._u32(off)
        if length == 0:
            return None, off + 4
        self._check_bounds(off + 4, length)
        name = self.data[off + 4: off + 4 + length]
        return name, off + 4 + length

    def _parse_root(self) -> DirEntry:
        pos = self.header.fph_pos
        if self.header.fph_size < FPH_FIXED_SIZE:
            sys.stderr.write(
                f"warning: fphSize ({self.header.fph_size}) is smaller than the "
                f"known v0.0 FPH layout ({FPH_FIXED_SIZE} bytes); reading the "
                f"fields that are declared present regardless\n"
            )

        dirhdr_am = self._u16(pos)
        f_am = self._u16(pos + 2)
        dirstart = self._u32(pos + 4)
        fstart = self._u32(pos + 8)
        flags = self._u32(pos + 12)

        root = DirEntry(
            name="/", offset=pos, dirhdr_am=dirhdr_am, f_am=f_am,
            dirstart=dirstart, fstart=fstart, flags=flags,
        )
        self._walk(root)
        return root

    def _walk(self, d: DirEntry):
        # -- files --
        off = d.fstart
        while True:
            name_bytes, after_name = self._read_name(off)
            if name_bytes is None:
                break  # EOF marker
            filesize = self._u32(after_name)
            content_off = after_name + 4
            self._check_bounds(content_off, filesize)
            d.files.append(FileEntry(
                name=name_bytes.decode("utf-8", errors="replace"),
                offset=off,
                filesize=filesize,
                content_offset=content_off,
            ))
            # Advance past: len field(4) + name(len) + filesize field(4) + contents(filesize)
            # (the "next = f.len + f.filesize" formula in the spec's pseudocode
            # omits the two u32 field sizes themselves; this is the corrected form)
            off = content_off + filesize

        if len(d.files) != d.f_am:
            sys.stderr.write(
                f"warning: dir '{d.name}' declares fAm={d.f_am} but "
                f"{len(d.files)} file entries were found before EOF\n"
            )

        # -- subdirectories --
        off = d.dirstart
        while True:
            name_bytes, after_name = self._read_name(off)
            if name_bytes is None:
                break  # EOF marker
            dirhdr_am = self._u16(after_name)
            f_am = self._u16(after_name + 2)
            dirstart = self._u32(after_name + 4)
            fstart = self._u32(after_name + 8)
            flags = self._u32(after_name + 12)
            sub = DirEntry(
                name=name_bytes.decode("utf-8", errors="replace"),
                offset=off,
                dirhdr_am=dirhdr_am, f_am=f_am,
                dirstart=dirstart, fstart=fstart, flags=flags,
            )
            d.dirs.append(sub)
            self._walk(sub)
            off = after_name + FPH_FIXED_SIZE

        if len(d.dirs) != d.dirhdr_am:
            sys.stderr.write(
                f"warning: dir '{d.name}' declares dirhdrAm={d.dirhdr_am} but "
                f"{len(d.dirs)} subdirectory entries were found before EOF\n"
            )

    # ---- public API --------------------------------------------------------

    def read_file_content(self, entry: FileEntry) -> bytes:
        return self.data[entry.content_offset: entry.content_offset + entry.filesize]

    def find(self, path: str) -> Optional[FileEntry]:
        """
        Resolve a '/'-separated path to a FileEntry, walking one directory
        component at a time the way findFInDir() does per-directory.
        """
        parts = [p for p in path.split("/") if p]
        if not parts:
            return None
        cur = self.root
        for part in parts[:-1]:
            nxt = next((sd for sd in cur.dirs if sd.name == part), None)
            if nxt is None:
                return None
            cur = nxt
        target = parts[-1]
        return next((f for f in cur.files if f.name == target), None)

    def print_tree(self, d: Optional[DirEntry] = None, indent: int = 0):
        if d is None:
            d = self.root
            print("/")
        for sd in d.dirs:
            print("  " * (indent + 1) + sd.name + "/")
            self.print_tree(sd, indent + 1)
        for f in d.files:
            print("  " * (indent + 1) + f"{f.name}  ({f.filesize} bytes)")


def main():
    if len(sys.argv) < 2:
        print("usage: rivfs_parser.py <input.img> [--extract /path/in/fs [output_file]]")
        sys.exit(1)

    img_path = sys.argv[1]
    try:
        fs = RivFsImage(img_path)
    except RivFsError as e:
        print(f"error: {e}")
        sys.exit(1)

    print(f"rivfs v{fs.header.vers_major}.{fs.header.vers_minor}  "
          f"flags=0x{fs.header.flags:08x}  fphPos={fs.header.fph_pos}  "
          f"fphSize={fs.header.fph_size}")
    fs.print_tree()

    if len(sys.argv) >= 3 and sys.argv[2] == "--extract":
        if len(sys.argv) < 4:
            print("--extract requires a path argument")
            sys.exit(1)
        target_path = sys.argv[3]
        out_path = sys.argv[4] if len(sys.argv) > 4 else os.path.basename(target_path.rstrip("/")) or "out"
        entry = fs.find(target_path)
        if entry is None:
            print(f"not found: {target_path}")
            sys.exit(1)
        content = fs.read_file_content(entry)
        with open(out_path, "wb") as f:
            f.write(content)
        print(f"extracted {target_path} -> {out_path} ({len(content)} bytes)")


if __name__ == "__main__":
    main()

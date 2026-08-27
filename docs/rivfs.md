# RivFs

The rivfs is a filesystem designed to be simple. It is mainly used for the initramfs, to minimize RAM usage. 

NOTE: This document will present stuff in a pseudo language. It is best to assume any struct not label __packed__ (__ packed __ but markdown) or just no struct label at all.
NOTE: All signed/unsigned integers are in little endian.
NOTE: Any enum type will simply just continue forwards, E.G. 

~~~C++
enum Enum : u32 {
    f1,
    f2
}
~~~
is equal to
~~~C++
enum Enum : u32 {
    f1 = 0,
    f2 = 1,
}
~~~

## How does it work?

A rivfs starts with a header. It basically contains:

~~~Rust
magic: char[5] // Magic to verify it's a rivfs. Should be "rivfs"
fphSize: u32 // File position header size. The file position header will be discussed later.
// Mainly usefull when newer versions since the size may be unknown and ya can just skip past it.
// NOTE: Whenever a position is used, it's a *RELATIVE* position. To the start of the filesystem (E.G. magic[0])
fphPos: u32
vers: u8[2] // Version of rivfs, for now should both should be 0. The version format is vers[0] is major vers[1] is minor
enum Flags : u32 {
    FLGS_NONE
    FLGS_RONLY, // Should be on. Rivfs is not meant for modification.
    // Reserved for later. (Should be a power of 2)
} flgs;
~~~

## File position header

The file position header is used to locate a dirhdr as well as files in / (root). A dirhdr simply describes a directory and it's contents. Format:

~~~Rust
dirhdrAm: u16 // Amount of dir entries in root (/)
fAm: u16 // File amount in root /
dirstart: u32 // Start of the first dir.
fstart: u32 // Start of the first file.
enum Flags : u32 {
    // Reserved.
} flgs
~~~

## File

A file is <u>dynamic</u>. Knowing fstart you can't guess where file f is located. We will mainly discuss how to find the contents of a file f with just fp.

### Layout

The layout looks like:
~~~Rust
    struct __packed__ {
        len: u32
        conts: char*
    } fpSv;
    u32 filesize;
    conts: u8[filesize]
~~~

As you can see, a lot of this is dynamic. Mainly:

- Filename
- Contents.

You look at the fpSv field and filesize field to determine

* a. Is this the file we need?
* b. Where's the next file?

So to look up a file, you do:

~~~Rust
fn findFInDir(fp: String) {
    Look up the FPH
    Look at FPH.fstart to find the first file

    loop {
        f = currentFile
        if !f.fpSv.len && !f.fpSv.conts return NOT_FOUND
        if svEq(fnameSv, f.fpSv) return f;
        else currentFile = f.len + f.filesize
    }
}
~~~

### EOF

~~~C
fpSv = {
    .len = 0,
    .conts = 0,
}
~~~
Is a EOF and signifies the end of the directory.

## Directory

/*
dirhdrAm: u16 // Amount of dir entries in root (/)
fAm: u16 // File amount in root /
dirstart: u32 // Start of the first dir.
fstart: u32 // Start of the first file.
enum Flags : u32 {
    // Reserved.
}
*/
A directory is laid out similar to a FPH, with some minor differences. The struct is:

~~~Rust
struct __packed__ {
    len: u32
    conts: char // (first item of array.) A char is 8bit
} dnSv
dirhdrAm: u16
fAm: u16
dirstart: u32
fstart: u32
enum Flags {
    // Currently unused
} flgs
~~~
NOTE: EOF is the same as in a file.

## Generating a rivfs

RivOS will include a rust binary inside initramfs/genfs/target/(debug|release)/genfs which will allow for
exec <<unused>srcdir> <<unused>destDir>
NOTE: Currently incomplete.

## Caveats

This filesystem is not meant to be modified, it should be read only. If FLGS_RONLY is off, assume the filesystem is corrupt.
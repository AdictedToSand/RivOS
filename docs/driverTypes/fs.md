# Fs drivers

These are filesystem drivers that allow for communication via on-disk files.

## Custom

In the .cfg of the driver, inside the [custom] section, should be specified:
~~~ini
[custom]
fstype="fat32"
~~~
This basically advertises "I can do FAT32"
Whats done with this is decided in /etc/fs.cfg

## Functions required

~~~Rust
// Returns a void* to a internal structure that allows the driver to reconstruct what the file is supposed to represent
open(str fp )
// Writes conts into file fileptr with len as the length of the write. 
write(ptr fileptr ptr conts u32 len )
// Reads a file fileptr into buf with max length being buf.
read(ptr fileptr ptr buf u32 len )
// Closes the file and performs cleanup
close(ptr f)
~~~
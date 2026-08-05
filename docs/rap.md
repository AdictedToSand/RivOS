# Rap file

## What is it?

A .rap file (shorthand for rivapi) is a file that describes where functions live in memory.

## How to use it?

A .rap file, raw may look like this:

~~~ Rust
idtSetDescriptor(u8 vector ptr isr u8 flags)=1065293                                                                                                           
putPixel(u32 argb u32 x u32 y)=1063056                                                                                                                                       
getScreenWidth()=1063196                                                                                                                                                     
getFbPhysAddr()=1063106                                                                                                                                                      
getScreenHeight()=1063166                                                                                                                                                    
getFbSizeBytes()=1063136    
~~~

## Specification

A rap file will always look like this:

functionName(params)=addrOfFunctionToJumpToAsAnUnsignedInt\n

### Allowed parameter types

Allowed parameter types are:

- ptr. This is a raw pointer, equivalent to a void*
- str. This is a const char*, not the Str type you might see in the source code.
- u8, u16, u32 and u64. These represent the unsigned integer types
- i8, i16, i32, i64. These represent the signed integer types
- char. This represents a character. (Use the C definition here (8bit), not the rust 32bit.)
- ..., this represent a indefinite index, and is representative to the C:
void func(...);

## Why use a rap file?

A rap file will describe a lot of required functions, which can not be found in syscalls.

## How do I use it?

There are two .rap files in the kernel:

- /krn/virt/func.rap. This is for kernel functions, and not accesible to userspace programs.
- /krn/virt/userfunc.rap. This is for userspace and exposes user functions. Note that authorited processes can write to this.

## Why does it exist?

.rap exists because it reduces the overhead of a syscall, and instead just calls some code.
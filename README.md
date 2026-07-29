# RivOS

RivOS is an operating system designed for the x86 CPU architecture. It features a multitasking kernel alongside a custom bootloader

## Bootloader

RivOS features a custom bootloader (RivBoot), which looks like this 

![ Image of bootloader ][def]

[def]: bootloader.png

The bootloader only supports one kernel entry.

### RivBoot Protocol

For further information into the RivBoot protocol look at the documentation [here](docs/RivBoot.md)

## Kernel

The RivOS kernel is designed to be extremely small. It's main job is to get the /sbin/sched started up. The scheduler is not integrated in the kernel: /sbin/sched is the one setting up the APIC and such. There is also a /bin/init but /sbin/sched actually starts that up.

### Language choice

Most of the kernel is written in C++. There is also some assembly present, and for the bootloader specifically C is used.

## Download

To download RivOS, you can either clone the repo via:

~~~Bash
git clone https://github.com/AdictedToSand/RivOS.git 
~~~

Or just click the download button the github page.

### Dependecies

The dependecies required for RivOS:

* Linux is generally needed for this to work. A lot of tools (like for example dd) are mostly on linux. If you're on windows, try looking into the [Windows subsystem for linux](https://learn.microsoft.com/en-us/windows/wsl/)
* Make. This can be circumvented by running the commands directly or using a shell script, but that is not recommended.
* [i686-elf-g++ and i686-elf-gcc](https://gcc.gnu.org/). It is recommended to use a build helper like yay. To install, run:

~~~Bash
yay -S i686-elf-gcc-bin
~~~

This will install the specific g++ and gcc cross compilers required on an arch based system. If you use other any other distro, it is worth looking at this [page](https://wiki.osdev.org/GCC_Cross-Compiler)

* [Python3](https://www.python.org/). This is required to run the build script [build.py](build.py). To install python3, use:

~~~Bash
sudo pacman -S python
~~~
(Or whatever the package manager for your distro is)

* [Qemu](https://www.qemu.org/). This is the software used as a virtual machine. It is possible to use other VMs, but  that means having to modify the "run" target in the Makefile or lose a lot of convenience. To install it:

~~~Bash
sudo pacman -S qemu
~~~

* [NASM](https://www.nasm.us/). NASM stands for Netwide ASeMbler, and is used for all .asm files. Install with:

~~~Bash
sudo pacman -S nasm
~~~

Again, you should use the package manager of your distro.

* Others. There is a high change these are already installed on your system, but please check them:

    * dosfstools and mtools: these are used for formatting the disk. Install with: 
    ~~~Bash
        sudo pacman -S dosfstools mtools
    ~~~
    * coreutils: These are also used for formatting the disk. Install if not already with:
    ~~~Bash
        sudo pacman -S coreutils
    ~~~

## Running the operating system

In here I will describe all the targets of the Makefile:

### all

This target will be ran if you just type

~~~Bash
make
~~~

. This target will simply execute the build_dbg target.

### prepare_disk

This target will use all the build files generated to make a bootable disk.

This target should the last one you run before the run target itself.

### build_dbg

This target will build the kernel, however in debug mode. This target will have no optimizations applied and may be more stable.

### build_release

This target will also build the kernel, however with more optimizations and thus may be more unstable.

### run

This will run the OS. You should have run build_dbg or build_release before this.

### debug

This will set up the OS such that it is available to GDB at localhost:1234 and can be run step by step. A debug build will be done before.

#### GDB 

Gdb should be run via 

~~~Bash
gdb build/RivOS
~~~

It is recommended to set a breakpoint in kernelMain, as the bootloader will be an unknown symbol.

If you want to also have the bootloader code visible in GDB run
~~~Bash
add-symbol-file build/stage2.elf 0x8000
~~~
and then

~~~Bash
b startBoot
c
~~~

The initial [bootcode](src/boot/loader.asm) will be a binary glob in assembly so nothing you can see there.


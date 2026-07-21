CPPFLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -Isrc -Isrc/lib


all:
	mkdir -p isodir/boot/grub
	mkdir build

build_dbg:
	i686-elf-as src/boot.s -o build/boot.o
	i686-elf-g++ -c src/kernel.cpp -o build/kernel.o -O0  $(CPPFLAGS) -DDEBUG
	i686-elf-gcc -T linker.ld -o build/RivOS -ffreestanding -O0 -nostdlib build/boot.o build/kernel.o -lgcc
	cp build/RivOS isodir/boot/RivOS
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o build/RivOS.iso isodir


build_release:
	echo 'TODO'

run:
	clear
	qemu-system-i386 -cdrom build/RivOS.iso \
		-serial stdio

mr: build_dbg run


.PHONY: build_dbg build_release run

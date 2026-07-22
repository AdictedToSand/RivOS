CPPFLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -Isrc -Isrc/lib

all: build_dbg

build_dbg:
	genext2fs -d rootFs -b 65536 build/rootfs.img
	python3 build.py

build_release:
	genext2fs -d rootFs -b 65536 build/rootfs.img 
	python3 build.py release

run:
	clear
	qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-serial stdio \
		-drive file=build/rootfs.img,format=raw,if=ide

debug: build_dbg
	qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-serial stdio \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-s -S

mr: build_dbg run

clean:
	rm -rf build
	rm -rf isodir

.PHONY: all build_dbg build_release run clean mr

CPPFLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -Isrc -Isrc/lib

all: build_dbg

build_dbg:
	python3 build.py

build_release:
	python3 build.py release

run:
	clear
	qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-serial stdio

mr: build_dbg run

clean:
	rm -rf build
	rm -rf isodir

.PHONY: all build_dbg build_release run clean mr

all: build_dbg

prepare_disk:
	dd if=/dev/zero of=build/rootfs.img bs=1024 count=65536
	mkfs.fat -F 32 build/rootfs.img
	mcopy -i build/rootfs.img -s rootFs/* ::

build_dbg: prepare_disk
	python3 build.py

build_release: prepare_disk
	python3 build.py release

run:
	clear
	qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-serial stdio \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-boot d

debug: build_dbg
	qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-serial stdio \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-s -S \
		-boot d

mr: build_dbg run

clean:
	rm -rf build
	rm -rf isodir

.PHONY: all build_dbg build_release run clean mr

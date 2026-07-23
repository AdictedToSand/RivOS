all: build_dbg

prepare_disk:
	dd if=/dev/zero of=build/disk.img bs=1M count=64
	dd if=build/loader of=build/disk.img conv=notrunc
	dd if=build/stage2 of=build/disk.img bs=512 seek=1 conv=notrunc
	mkfs.fat -F 32 build/rootfs.img
	mcopy -i build/rootfs.img -s rootFs/* ::

build_dbg:
	python3 build.py
	make prepare_disk

build_release: 
	python3 build.py release
	make prepare_disk

run:
	clear
	qemu-system-i386 \
		-drive file=build/disk.img,format=raw \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-serial stdio

debug: build_dbg
	qemu-system-i386 \
		-drive file=build/disk.img,format=raw \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-serial stdio \
		-s -S

mr: build_dbg run

clean:
	rm -rf build
	rm -rf isodir

.PHONY: all build_dbg build_release run clean mr

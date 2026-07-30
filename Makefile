all: build_dbg

prepare_disk:
	dd if=/dev/zero of=build/disk.img bs=1M count=64
	dd if=build/init of=build/disk.img conv=notrunc

	dd if=build/stage2 of=build/disk.img bs=512 seek=1 conv=notrunc

	$(eval STAGE2_SIZE := $(shell stat -c%s build/stage2))
	$(eval STAGE2_SECTORS := $(shell echo $$(( ($(STAGE2_SIZE) + 511) / 512 )) ))

	dd if=build/kernel.bin of=build/disk.img bs=512 seek=$$(($(STAGE2_SECTORS) + 1)) conv=notrunc

	dd if=/dev/zero of=build/rootfs.img bs=1M count=64
	mkfs.fat -F 32 build/rootfs.img
	mcopy -i build/rootfs.img -s rootFs/* ::


build_init:
	$(eval STAGE2_SIZE := $(shell stat -c%s build/stage2))
	$(eval STAGE2_SECTORS := $(shell echo $$(( ($(STAGE2_SIZE) + 511) / 512 )) ))

	echo "%define STAGE2_SECTORS $(STAGE2_SECTORS)" > build/generated_init.asm
	cat src/boot/initAssembly/init.asm >> build/generated_init.asm
	nasm -f bin build/generated_init.asm -o build/init


build_dbg:
	python3 build.py
	i686-elf-objcopy -O binary build/RivOS build/kernel.bin
	make build_init
	make prepare_disk


build_release:
	python3 build.py release
	i686-elf-objcopy -O binary build/RivOS build/kernel.bin
	make build_init
	make prepare_disk


run:
	clear
	qemu-system-i386 \
		-drive file=build/disk.img,format=raw \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-serial stdio \
		-boot c


debug: build_dbg
	clear
	qemu-system-i386 \
		-drive file=build/disk.img,format=raw \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-serial stdio \
		-s -S \
		-boot c


mr: build_dbg run


clean:
	rm -rf build
	rm -rf isodir


gdb:
	gdb build/RivOS -ex "target remote :1234" -ex "c" -ex "tui enable"


.PHONY: all build_dbg build_release build_init run debug clean mr

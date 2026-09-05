INITRAMFS_DIR := initramfs
INITRAMFS_GENFS_RDIR := $(INITRAMFS_DIR)/genfs
INITRAMFS_GEN_DIR := $(INITRAMFS_GENFS_RDIR)/genfs
INITRAMFS_IN_DIR := $(INITRAMFS_DIR)/rootFs/
INITRAMFS_IMG := isodir/boot/initramfs.img

build_drv:
	make -C initramfs/src build

prepare_initramfs:
	 @echo "Hello, world!" > $(INITRAMFS_IMG)
	 @make -C $(INITRAMFS_GENFS_RDIR) gen_exec_in_genfs
	 @echo ''
	 ./$(INITRAMFS_GEN_DIR) $(INITRAMFS_IN_DIR) $(INITRAMFS_IMG)

all: 
	@mkdir -p initramfs/rootFs/fs initramfs/rootFs/storage initramfs/build
	@mkdir -p isodir/boot
	@mkdir -p build
	@clear
	@echo '---NASM---'
	nasm --version
	@# make is already installed (obviously)
	@echo '---GCC/G++--'
	@i686-elf-g++ --version
	@i686-elf-gcc --version
	@echo '---PYTHON---'
	@python3 --version
	@echo '---QEMU---'
	@qemu-system-i386 --version
	@make rivboot

initramfs_tree:
	python3 initramfs/genfs/rivfsdata.py isodir/boot/initramfs.img

prepare_disk:
	@dd if=/dev/zero of=build/disk.img bs=1M count=64
	@dd if=build/init of=build/disk.img conv=notrunc

	@dd if=build/stage2 of=build/disk.img bs=512 seek=1 conv=notrunc

	@$(eval STAGE2_SIZE := $(shell stat -c%s build/stage2))
	@$(eval STAGE2_SECTORS := $(shell echo $$(( ($(STAGE2_SIZE) + 511) / 512 )) ))

	@dd if=build/kernel.bin of=build/disk.img bs=512 seek=$$(($(STAGE2_SECTORS) + 1)) conv=notrunc

	@dd if=/dev/zero of=build/rootfs.img bs=1M count=64
	@mkfs.fat -F 32 build/rootfs.img
	@mcopy -i build/rootfs.img -s rootFs/* ::

build_init:
	@$(eval STAGE2_SIZE := $(shell stat -c%s build/stage2))
	@$(eval STAGE2_SECTORS := $(shell echo $$(( ($(STAGE2_SIZE) + 511) / 512 )) ))

	@echo "%define STAGE2_SECTORS $(STAGE2_SECTORS)" > build/generated_init.asm
	@cat src/boot/initAssembly/init.asm >> build/generated_init.asm
	@nasm -f bin build/generated_init.asm -o build/init


build_dbg:
	@python3 build.py
	@make -C sbinSrc debug

rivboot: build_dbg
	@i686-elf-objcopy -O binary build/RivOS build/kernel.bin
	@make build_init
	@make prepare_disk
	@make prepare_initramfs

build_release:
	@python3 build.py release
	@make -C sbinSrc release


# GRUB ISO build
grub: build_dbg
	@rm -rf isodir
	@mkdir -p isodir/boot/grub
	@cp build/RivOS isodir/boot/RivOS
	@cp grub.cfg isodir/boot/grub/
	@make prepare_initramfs
	@grub-mkrescue -o build/RivOS.iso isodir


run_rivboot:
	@clear
	@qemu-system-i386 \
		-drive file=build/disk.img,format=raw,index=0 \
		-drive file=build/rootfs.img,format=raw,if=ide,index=1 \
		-serial stdio \
		-boot c


run:
	@clear
	@qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-boot d \
		-serial stdio \
		-d int \
		-D qemu.log


debug: grub 
	@clear
	@qemu-system-i386 \
		-cdrom build/RivOS.iso \
		-drive file=build/rootfs.img,format=raw,if=ide \
		-serial stdio \
		-s -S \
		-boot d

debug_rivboot: rivboot
	@clear
	@qemu-system-i386 \
		-drive file=build/disk.img,format=raw,index=0 \
		-drive file=build/rootfs.img,format=raw,if=ide,index=1 \
		-serial stdio \
		-s -S \
		-boot c

mr: build_dbg grub prepare_disk run

mr_rivboot: rivboot prepare_disk run_rivboot
mr_rb: mr_rivboot

clean:
	rm -rf build
	rm -rf isodir
	mkdir build
	mkdir -p isodir/boot
	rm -rf initramfs/build
	rm -rf initramfs/rootFs

gdb:
	@gdb build/RivOS -ex "target remote :1234" -ex "tui enable" -ex "display/i $pc"

.PHONY: all build_dbg build_release build_init prepare_disk grub run run_grub debug clean mr debug_rivboot

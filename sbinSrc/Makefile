CROSS = i686-elf

CC = $(CROSS)-gcc
LD = $(CROSS)-ld
AS = nasm

CFLAGS = -m32 -ffreestanding -nostdlib -static -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -static

OUT = ../rootFs/krn/bin

DEBUG = build_dbg
RELEASE = build_release

EXCLUDE_DIRS := build/ build_dbg/ build_release/
DIRS := $(filter-out $(EXCLUDE_DIRS),$(wildcard */))
PROGRAMS := $(patsubst %/,%,$(DIRS))

.PHONY: all clean debug release build_programs

all: debug release

debug:
	rm -rf $(OUT)
	$(MAKE) build_programs

release:
	rm -rf $(OUT)
	$(MAKE) build_programs

build_programs:
	$(foreach p,$(PROGRAMS),$(MAKE) $(OUT)/$(p);)

$(OUT)/%:
	mkdir -p $(OUT)
	mkdir -p $(DEBUG)/$*
	mkdir -p $(RELEASE)/$*

	$(foreach c,$(wildcard $*/*.c),$(CC) $(CFLAGS) -g -c $(c) -o $(DEBUG)/$*/$(notdir $(c:.c=.o));)
	$(foreach a,$(wildcard $*/*.asm),$(AS) -f elf32 $(a) -o $(DEBUG)/$*/$(notdir $(a:.asm=.o));)

	$(LD) $(LDFLAGS) -T $*/linker.ld $(DEBUG)/$*/*.o -o $@

clean:
	rm -rf $(DEBUG) $(RELEASE)
	rm -rf $(OUT)

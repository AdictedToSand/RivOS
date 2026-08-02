CROSS = i686-elf

CC = $(CROSS)-gcc
LD = $(CROSS)-ld
AS = nasm

CFLAGS = -m32 -ffreestanding -nostdlib -static -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -static

OUT = ../rootFs/krn/bin

EXCLUDE_DIRS := build/ build_dbg/ build_release/
DIRS := $(filter-out $(EXCLUDE_DIRS),$(wildcard */))
PROGRAMS := $(patsubst %/,%,$(DIRS))

DEBUG = build_dbg
RELEASE = build_release

.PHONY: all clean debug release

all: debug release

debug: $(PROGRAMS:%=$(OUT)/%)

release: $(PROGRAMS:%=$(OUT)/%)

$(OUT)/%:
	mkdir -p $(DEBUG)/$*
	mkdir -p $(RELEASE)/$*

	$(foreach c,$(wildcard $*/*.c),$(CC) $(CFLAGS) -g -c $(c) -o $(DEBUG)/$*/$(notdir $(c:.c=.o));)
	$(foreach a,$(wildcard $*/*.asm),$(AS) -f elf32 $(a) -o $(DEBUG)/$*/$(notdir $(a:.asm=.o));)

	$(LD) $(LDFLAGS) -T $*/linker.ld $(DEBUG)/$*/*.o -o $@

clean:
	rm -rf $(DEBUG) $(RELEASE)
	rm -rf $(OUT)

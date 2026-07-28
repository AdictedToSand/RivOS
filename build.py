from pathlib import Path
import subprocess
import sys

CPPFLAGS = [
    "-ffreestanding",
    "-Wno-pedantic",
    "-Wall",
    "-Wextra",
    "-fno-exceptions",
    "-fno-rtti",
    "-Isrc",
    "-Isrc/lib",
    "-fno-use-cxa-atexit",
]

BUILD_DIR = Path("build")
OBJ_DIR = BUILD_DIR / "obj"


def run(cmd):
    print(" ".join(cmd))
    subprocess.check_call(cmd)


def compile_cpp(file: Path, debug: bool) -> Path:
    out = OBJ_DIR / Path(str(file.relative_to("src")) + ".o")
    out.parent.mkdir(parents=True, exist_ok=True)

    flags = CPPFLAGS.copy()

    if debug:
        flags.extend(["-O0", "-g", "-DDEBUG"])
    else:
        flags.extend(["-O2"])

    run([
        "i686-elf-g++",
        "-c",
        str(file),
        "-o",
        str(out),
        *flags,
    ])

    return out

def compile_c(file: Path, debug: bool) -> Path:
    out = OBJ_DIR / Path(str(file.relative_to("src")) + ".o")
    out.parent.mkdir(parents=True, exist_ok=True)

    flags = [
        "-m32",
        "-ffreestanding",
        "-Wall",
        "-Wextra",
        "-Isrc",
        "-Isrc/lib",
    ]

    if debug:
        flags.extend(["-O0", "-g", "-DDEBUG"])
    else:
        flags.extend(["-O2"])

    run([
        "i686-elf-gcc",
        "-c",
        str(file),
        "-o",
        str(out),
        *flags,
    ])

    return out

def compile_gas(file: Path) -> Path:
    out = OBJ_DIR / Path(str(file.relative_to("src")) + ".o")
    out.parent.mkdir(parents=True, exist_ok=True)

    run([
        "i686-elf-as",
        str(file),
        "-o",
        str(out),
    ])

    return out


def compile_nasm(file: Path) -> Path:
    out = OBJ_DIR / Path(str(file.relative_to("src")) + ".o")
    out.parent.mkdir(parents=True, exist_ok=True)

    run([
        "nasm",
        "-f",
        "elf32",
        str(file),
        "-o",
        str(out),
    ])

    return out


def compile_bootloader(file: Path) -> Path:
    out = BUILD_DIR / file.stem

    run([
        "nasm",
        "-f",
        "bin",
        str(file),
        "-o",
        str(out),
    ])

    return out


def build(debug: bool = True):
    BUILD_DIR.mkdir(exist_ok=True)
    OBJ_DIR.mkdir(exist_ok=True)

    objects = []

    src = Path("src")
    boot_dir = src / "boot"

    #
    # Stage 1 bootloader
    #

    loader = boot_dir / "loader.asm"

    if loader.exists():
        compile_bootloader(loader)

    #
    # Stage 2
    #

    stage2_objects = []

    stage2_asm = boot_dir / "stage2.asm"
    stage2_c = boot_dir / "boot.c"

    if stage2_asm.exists():
        stage2_objects.append(compile_nasm(stage2_asm))

    if stage2_c.exists():
        stage2_objects.append(compile_c(stage2_c, debug))

    if stage2_objects:
        stage2_elf = BUILD_DIR / "stage2.elf"

        run([
            "i686-elf-gcc",
            "-T",
            "src/boot/linker.ld",
            "-nostdlib",
            "-ffreestanding",
            "-o",
            str(stage2_elf),
            *map(str, stage2_objects),
            "-lgcc",
        ])

        run([
            "i686-elf-objcopy",
            "-O",
            "binary",
            str(stage2_elf),
            str(BUILD_DIR / "stage2"),
        ])

    #
    # Kernel
    #

    for file in src.rglob("*.cpp"):
        if "boot" not in file.parts:
            objects.append(compile_cpp(file, debug))

    for file in src.rglob("*.s"):
        if "boot" not in file.parts:
            objects.append(compile_gas(file))

    for file in src.rglob("*.asm"):
        if "boot" not in file.parts:
            objects.append(compile_nasm(file))

    kernel = BUILD_DIR / "RivOS"

    link_cmd = [
        "i686-elf-gcc",
        "-T",
        "linker.ld",
        "-o",
        str(kernel),
        "-ffreestanding",
        "-nostdlib",
        *map(str, objects),
        "-lgcc",
    ]

    if debug:
        link_cmd.insert(-1, "-O0")
        link_cmd.insert(-1, "-g")
    else:
        link_cmd.insert(-1, "-O2")

    run(link_cmd)

    run([
        "cp",
        str(kernel),
        "isodir/boot/RivOS",
    ])


if __name__ == "__main__":
    build(debug=(len(sys.argv) < 2 or sys.argv[1] != "release"))

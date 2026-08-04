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
    "-Istd",
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


def compile_bootloader() -> Path:
    out = BUILD_DIR / "init"

    run([
        "nasm",
        "-f",
        "bin",
        str(BUILD_DIR / "generated_init.asm"),
        "-o",
        str(out),
    ])

    return out


def generate_bootloader_header():
    stage2_size = (BUILD_DIR / "stage2").stat().st_size
    stage2_sectors = (stage2_size + 511) // 512

    with open(BUILD_DIR / "generated_init.asm", "w") as f:
        f.write(f"%define STAGE2_SECTORS {stage2_sectors}\n")
        f.write((Path("src/boot/initAssembly/init.asm")).read_text())


def build(debug: bool = True):
    BUILD_DIR.mkdir(exist_ok=True)
    OBJ_DIR.mkdir(exist_ok=True)

    src = Path("src")
    boot_dir = src / "boot"

    #
    # Stage 2 boot program
    #

    stage2_objects = []

    for file in sorted(boot_dir.rglob("*.asm")):
        if file.name != "init.asm":
            stage2_objects.append(compile_nasm(file))

    for file in sorted(boot_dir.rglob("*.c")):
        stage2_objects.append(compile_c(file, debug))

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
    # Stage 1 bootloader
    #

    generate_bootloader_header()
    compile_bootloader()

    #
    # Kernel
    #

    objects = []

    init = src / "init.asm"

    if init.exists():
        objects.append(compile_nasm(init))

    for file in sorted(src.rglob("*.cpp")):
        if "boot" not in file.parts:
            objects.append(compile_cpp(file, debug))

    for file in sorted(src.rglob("*.s")):
        if "boot" not in file.parts:
            objects.append(compile_gas(file))

    for file in sorted(src.rglob("*.asm")):
        if "boot" not in file.parts and file != init:
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

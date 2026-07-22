from pathlib import Path
import subprocess
import sys

CPPFLAGS = [
    "-ffreestanding",
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


def build(debug: bool = True):
    BUILD_DIR.mkdir(exist_ok=True)
    OBJ_DIR.mkdir(exist_ok=True)

    objects = []

    src = Path("src")

    for file in src.rglob("*.cpp"):
        objects.append(compile_cpp(file, debug))

    for file in src.rglob("*.s"):
        objects.append(compile_gas(file))

    for file in src.rglob("*.asm"):
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

    iso_dir = Path("isodir")
    (iso_dir / "boot" / "grub").mkdir(parents=True, exist_ok=True)

    run(["cp", str(kernel), "isodir/boot/RivOS"])
    run(["cp", "grub.cfg", "isodir/boot/grub/grub.cfg"])

    run([
        "grub-mkrescue",
        "-o",
        str(BUILD_DIR / "RivOS.iso"),
        "isodir",
    ])


if __name__ == "__main__":
    build(debug=(len(sys.argv) < 2 or sys.argv[1] != "release"))

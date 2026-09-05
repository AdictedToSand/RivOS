from pathlib import Path
import subprocess
import configparser

supported_drv_types = ["fs", "storage"]

ROOTFS = Path("../rootFs")
DRV_DIR = ROOTFS / "drv"

CPP_FLAGS = [
    "-O0",
    "-ffreestanding",
    "-Wno-pedantic",
    "-Wall",
    "-Wextra",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-use-cxa-atexit",
    "-fomit-frame-pointer",
    "-nostdlib",
    "-I",
    "../include",
]
CPP_COMPILER = "i686-elf-g++"
BUILD_DIR = Path("../build")

LD_BASE_FMT = """
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{{
    . = {};

    .text ALIGN(4K) :
    {{
        *(.text)
        *(.text.*)
    }}

    .data ALIGN(4K) :
    {{
        *(.data)
        *(.data.*)
    }}
    .rodata ALIGN(4K) :
    {{
        *(.rodata)
        *(.rodata.*)
    }}

    .bss ALIGN(4K) :
    {{
        *(.bss)
        *(COMMON)
    }}
}}
"""
LD_FLGS = [
    "-e", # The entry is not used.
    "0",
    "-static", 
    "-m",
    "elf_i386",
]
LD = "ld"

current_elf_pos = 0xC0000000

src_fp = Path(".")
for directory in  src_fp.rglob("*"):
    if directory.is_dir():
        conf_path = directory / "conf.cfg"

        if conf_path.exists():
            config = configparser.ConfigParser()
            config.read(conf_path)

            drv_type = config["gen"]["drvType"].strip('"')
            print(f"drv_type: '{drv_type}'")
            if not drv_type in supported_drv_types:
                print(f"Invalid driver type '{drv_type}'")
                break
            drv_name = config["gen"]["name"].strip('"') 
            print(f"drv_name {drv_name}")
            drv_type_dir = DRV_DIR / drv_type
            dest_fp = drv_type_dir / drv_name
            if "destFp" in config["gen"]:
                dest_fp = drv_type_dir / (config["gen"]["destFp"]).strip('"')
                print(f"Override: {dest_fp}")
            print(f"dest_dir: {str(dest_fp)}")
            abso_dest_file = dest_fp.resolve()
            print(f"abso_dest_dir: {str(abso_dest_file)}")
            dest_path = Path(abso_dest_file.with_suffix(".drv"))
            dest_path.mkdir(exist_ok=True)

            source_file_type = config["build.sourceFiles"]["type"].strip('"')
            if source_file_type != "cpp":
                print(f"unsupported file type: '{source_file_type}'")
                break 
            source_files = config["build.sourceFiles"]["list"].split(", ")
            print(f"sourcefiles: {source_files}")
            src_dir = config["cfg"]["srcdir"].strip('"')
            print(f"src_dir: '{src_dir}'")

            total_build_files = []

            for file in source_files:
                file_as_path = directory / src_dir / file
                file_as_path = file_as_path.resolve()
                print(f"File compiling: '{str(file_as_path)}'")
                mangled_name = f"{str(directory).strip('/')}_{file.strip('/')}.o"
                print(f"inp_file: {file}, o_file: {mangled_name}")
                cmd = [CPP_COMPILER, *CPP_FLAGS, "-c", file_as_path, "-o", str(BUILD_DIR / mangled_name)]

                print(cmd)

                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True
                )
                print(result.stdout)
                print(result.stderr)

                if result.returncode != 0:
                    print(f"Compilation failed with code {result.returncode}")
                    break 
                total_build_files.append(str((BUILD_DIR / mangled_name).resolve()))

                                        # 0xC... not 0xc... or 0XC...
            ld_code = LD_BASE_FMT.format(f"0x{current_elf_pos:08X}" )
            if not total_build_files:
                print("ERROR: total_build_files.is_empty() == true")
                break 
            mangled_linker_filename = f"linker_file.{str(directory).strip('/')}.ld"
            linkerfile_full = (BUILD_DIR / mangled_linker_filename).resolve()

            print(f"linker_file: {str(linkerfile_full)}")
            linkerfile_full.write_text(ld_code)
            linker_output_file = dest_path / f"drv_{drv_name}"
            cmd = [LD, *LD_FLGS, *total_build_files,
               "-o", str(linker_output_file), "-T", str(linkerfile_full)]
            print(f"cmd={" ".join(cmd_arg for cmd_arg in cmd)}")
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True
            )
            print(result.stdout)
            print(result.stderr)

            if result.returncode != 0:
                print(f"Compilation failed with code {result.returncode}")
                break

            # Now we can generate /conf.cfg.

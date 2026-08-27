use std::{io::Write, path::{Path, PathBuf}, process::exit, str};

#[repr(u32)]
enum MainHeaderFlags {
    None = 0,
    Ronly = 1,
}
#[repr(u32)]
enum FilePositionHeaderFlags {
    None = 0,
}

#[repr(C, packed)]
struct FilePositionHeader {
    dir_hdr_am: u16,
    f_am: u16,
    dirstart: u32,
    fstart: u32,
    flgs: u32 
}
impl FilePositionHeader {
    pub fn new(dir_hdr_am: u16, f_am: u16, dirstart: u32, fstart: u32, flgs: u32) -> FilePositionHeader {
        FilePositionHeader { dir_hdr_am, f_am, dirstart, fstart, flgs}
    }
    pub fn to_raw_bytes(&self) -> Vec<u8> {
        unsafe {
            std::slice::from_raw_parts(
                self as *const Self as *const u8,
                std::mem::size_of::<Self>(),
            ).to_vec()
        }
    }
}

fn u32_to_bytes(value: u32) -> [u8; 4] {
    value.to_le_bytes()
}

fn gen_fs_into(src_fp: String, gen_fp: String) -> std::io::Result<()> {
    if !std::fs::metadata(&src_fp)?.is_dir() {
        println!("initramfs_genfs: file is not a directory: {}.", gen_fp);
        exit(0);
    }
    let mut root_file_count: u16 = 0;
    let mut root_dir_count: u16 = 0;

    for entry in std::fs::read_dir(&src_fp)? {
        let entry = entry?;

        if entry.file_type()?.is_dir() {
            root_dir_count += 1;
        }
        else {
            root_file_count += 1;
        }
    }
    let hdr_size = 19;

    let mut initial_header: Vec<u8> = Vec::new();
    let mut outfile = std::fs::OpenOptions::new()
        .read(true)
        .write(true)
        .create(true)
        .truncate(true)
        .open(gen_fp)?;

    let glob_dir_hdr_am: u16 = root_dir_count;
    let glob_f_am: u16 = root_file_count;
    let glob_dirstart: u32 = 0;
    let glob_fstart: u32 = 0;
    let glob_flgs: u32 = MainHeaderFlags::Ronly as u32;
    let glob_fph_flgs: u32 = FilePositionHeaderFlags::None as u32;

    let glob_fph_size: u32 = size_of::<FilePositionHeader>() as u32;
    let glob_fph_start: u32 = hdr_size;
    // Magic
    let magic: &'static [u8; 5] = b"rivfs";
    // Next: fphSize
    let fph: FilePositionHeader = FilePositionHeader::new(
        glob_dir_hdr_am,
        glob_f_am,
        glob_dirstart,
        glob_fstart,
        glob_fph_flgs,
    );
    let raw_fph_bytes_ref: &[u8] = &fph.to_raw_bytes();

    println!("Fph size: {:#x}", glob_fph_size);
    
    initial_header.extend_from_slice(magic);
    initial_header.extend(u32_to_bytes(glob_fph_size));
    initial_header.extend(u32_to_bytes(glob_fph_start));
    let vers: [u8; 2] = [/* Major: */ 0, /* Minor */ 0];
    initial_header.extend(vers);
    initial_header.extend(u32_to_bytes(glob_flgs));
    // The fph is next.
    initial_header.extend(raw_fph_bytes_ref);
    outfile.write_all(&initial_header)?;

    Ok(())
}

fn main() -> std::io::Result<()> {
    let mut args = std::env::args();
    let cur_dir = std::env::current_dir()?;

    if args.len() < 3 {
        println!("Usage: <path_to_exec> inp_dir o_fp.img");
        return Ok(());
    }

    let arg_1 = args.nth(1).unwrap();
    let arg_2 = args.next().unwrap();

    let src_fp = if Path::new(&arg_1).is_absolute() {
        PathBuf::from(arg_1)
    } 
    else {
        cur_dir.join(arg_1)
    };

    let gen_fp = if Path::new(&arg_2).is_absolute() {
        PathBuf::from(arg_2)
    } 
    else {
        cur_dir.join(arg_2)
    };

    gen_fs_into(
        src_fp.to_string_lossy().into_owned(),
        gen_fp.to_string_lossy().into_owned(),
    )?;

    Ok(())
}

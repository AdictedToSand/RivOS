
fn gen_fs_into(src_fp: String, gen_fp: String) -> std::io::Result<()> {
    println!("src: {}, gen: {}", src_fp, gen_fp);

    Ok(())
}

fn path_is_abso(p: String) -> bool {
    if p.is_empty() { return false; }
    p.starts_with("/")
}

fn main() -> std::io::Result<()> {
    let mut args = std::env::args();
    if args.len() < 3 {
        println!("Usage: <path_to_exec> inp_dir o_fp.img");
        return Ok(());
    }
    let mut src_fp: String = String::new();
    if !path_is_abso(args.nth(0).unwrap()) {
        src_fp;
    }

    Ok(())
}

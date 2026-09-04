from pathlib import Path
import configparser

supported_drv_types = ["fs", "storage"]

ROOTFS = Path("../rootFs")
DRV_DIR = ROOTFS / "drv"

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
                continue
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
            Path(abso_dest_file.with_suffix(".drv")).mkdir()

"""Repackage authentic members to test package ownership, never synthesize payloads.

Temporary archives are offline test fixtures, not runtime extraction/cache.
No original file is changed or uploaded. Missing media is an explicit skip.
"""
import argparse
import copy
import hashlib
import os
from pathlib import Path
import subprocess
import struct
import tempfile
import zipfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", required=True)
    parser.add_argument("--archive", required=True)
    parser.add_argument("--scratch", required=True)
    args = parser.parse_args()
    archive = Path(args.archive)
    if not archive.is_file():
        print("skip: original EN PC3.4 archive unavailable", flush=True)
        return 77
    scratch = Path(args.scratch).resolve()
    scratch.mkdir(parents=True, exist_ok=True)
    expected = {
        "DATA/GRAPHICS.DAT": "fa6b1aa29e191418713bf2cda93d962e",
        "DM.EXE": "a7d61d6127cca1b5068110f531b988b8",
        "VGA": "218895d977eaa86b25b803bf21c49f62",
    }
    with zipfile.ZipFile(archive) as source:
        if sum(info.file_size for info in source.infolist()) > 8 * 1024 * 1024:
            raise ValueError("unexpected oversized original archive")
        members = {}
        for info in source.infolist():
            if info.is_dir():
                continue
            # Authentic DOS archive has backslashes in local headers but
            # slashes in its central directory. Accept only that spelling
            # difference; zipfile still verifies compression and payload CRC.
            source.fp.seek(info.header_offset)
            header = source.fp.read(30)
            if len(header) != 30 or header[:4] != b"PK\x03\x04":
                raise ValueError("invalid original local ZIP header")
            local_name = source.fp.read(struct.unpack_from("<H", header, 26)[0])
            local_name = local_name.decode("utf-8" if info.flag_bits & 0x800 else "cp437")
            if local_name.replace("\\", "/") != info.filename:
                raise ValueError("unrelated local/central ZIP member names")
            local_info = copy.copy(info)
            local_info.orig_filename = local_name
            members[info.filename] = source.read(local_info)
    for name, digest in expected.items():
        if hashlib.md5(members[name]).hexdigest() != digest:
            raise ValueError(f"unexpected original {name} identity")
    if "EGA" not in members or members["EGA"] == members["VGA"]:
        raise ValueError("authentic alternative driver unavailable")
    # Each variant changes only package placement/selection. The changed-driver
    # case substitutes the original EGA driver, not invented executable bytes.
    variants = {
        "original": (dict(members), "bound"),
        "missing-vga": ({k: v for k, v in members.items() if k != "VGA"}, "unbound"),
        "changed-vga": ({**members, "VGA": members["EGA"]}, "unbound"),
        "sibling-installation": (
            {("sibling/" + k if k in ("DM.EXE", "VGA") else k): v
             for k, v in members.items()}, "unbound"),
    }
    env = os.environ.copy()
    # Device output is irrelevant to package admission; no emulator is needed.
    env["FIRESTAFF_AUDIO_ENABLE_SDL"] = "0"
    env["TMPDIR"] = str(scratch)
    with tempfile.TemporaryDirectory(prefix="food-package-", dir=scratch) as directory:
        for name, (payloads, result) in variants.items():
            target = Path(directory) / (name + ".zip")
            with zipfile.ZipFile(target, "w", compression=zipfile.ZIP_DEFLATED) as output:
                for member, payload in payloads.items():
                    output.writestr(member, payload)
            print(f"checking {name}: {result}", flush=True)
            subprocess.run([args.probe, str(target), result], env=env,
                           check=True, timeout=90)
    print("ok: four original-byte package ownership cases", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

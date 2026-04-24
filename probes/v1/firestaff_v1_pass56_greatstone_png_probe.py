#!/usr/bin/env python3
"""Pass 56 optional Greatstone PNG reference probe.

This does not make V2 assets part of V1.  It only checks the already-downloaded
Greatstone source PNG folder when present, proving that Greatstone exposes PNGs
for the same TITLE image-bearing item numbers parsed by title_dat_loader_v1.
"""

from __future__ import annotations

import hashlib
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_DIR = ROOT / "v2-assets" / "title-animation" / "source-greatstone"
EXPECTED_INDICES = [2, 3] + list(range(4, 41)) + [41] + list(range(42, 58))


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as f:
        sig = f.read(8)
        if sig != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"{path} is not a PNG")
        length = struct.unpack(">I", f.read(4))[0]
        ctype = f.read(4)
        if length != 13 or ctype != b"IHDR":
            raise ValueError(f"{path} does not start with IHDR")
        data = f.read(13)
    return struct.unpack(">II", data[:8])


def main() -> int:
    ref_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_DIR
    if not ref_dir.is_dir():
        print(f"SKIP P56_TITLE_PNG_00 Greatstone source PNG dir not present: {ref_dir}")
        return 0

    files = sorted(ref_dir.glob("*.png"))
    found = sorted(int(p.name[:4]) for p in files)
    passed = total = 0

    def record(name: str, ok: bool, msg: str) -> None:
        nonlocal passed, total
        total += 1
        if ok:
            passed += 1
            print(f"PASS {name} {msg}")
        else:
            print(f"FAIL {name} {msg}")

    record("P56_TITLE_PNG_01", found == EXPECTED_INDICES,
           "Greatstone PNG item numbers are P8/PL plus EN/DL image records only")

    sizes = {p.name: png_size(p) for p in files}
    frame_sizes = {name: size for name, size in sizes.items() if name[5:7] in {"EN", "DL"}}
    record("P56_TITLE_PNG_02", len(frame_sizes) == 53 and all(s == (320, 200) for s in frame_sizes.values()),
           "all Greatstone EN/DL PNGs are original-resolution 320x200 frames")

    digest = hashlib.sha256()
    for p in files:
        digest.update(p.name.encode("ascii"))
        digest.update(hashlib.sha256(p.read_bytes()).digest())
    sha = digest.hexdigest()
    record("P56_TITLE_PNG_03", sha == "f6003bfab3da12f1cfad790eff59867811162102474a69329423b529205c69bc",
           "Greatstone source PNG filename/content fingerprint matches local pass-56 evidence")
    print(f"# pass56 greatstone png tree sha256: {sha}")
    print(f"# summary: {passed}/{total} invariants passed")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())

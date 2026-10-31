#!/usr/bin/env python3
"""Verify VDP1 texture-source evidence in the authentic Japanese menu frame."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_CAPTURE_HASHES = {
    "vdp1-capture-bundle.txt":
        "10e9dd1ef6bff3ca95ffb7a57265819d92ab46621ab157d19b32478af360b874",
    "vdp1-command-trace.csv":
        "f15eb2038f2195c7d7cc64947e5d50b679b8280073e2aadbcec4d72b9a5ccf29",
    "vdp1-vram.bin":
        "807d5f5eb7314e58c4c1207695e0bb5752aedeac6f58999aeb21848600cda84a",
}
EXPECTED_RETAIL_HASHES = {
    "TITLE.BIN": "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3",
    "LEV00.DGN": "24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a",
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    capture_root = os.environ.get("FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR")
    data_root = os.environ.get(
        "FIRESTAFF_NEXUS_DATA_DIR",
        str(Path.home() / ".firestaff" / "data" / "nexus"),
    )
    if not capture_root or not Path(capture_root).is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR to authentic Ymir dumps")
        return 77
    if not Path(data_root).is_dir():
        print("SKIP: authentic Nexus retail data directory is unavailable")
        return 77

    retail = Path(data_root)
    for name, expected in EXPECTED_RETAIL_HASHES.items():
        path = retail / name
        if not path.is_file() or sha256(path) != expected:
            print("SKIP: source join requires the captured Japanese TITLE.BIN and LEV00.DGN revisions")
            return 77

    capture = Path(capture_root)
    for name, expected in EXPECTED_CAPTURE_HASHES.items():
        path = capture / name
        if not path.is_file() or sha256(path) != expected:
            print(f"FAIL: authentic frame-8037 capture hash mismatch: {name}")
            return 1

    command = [
        sys.executable,
        str(ROOT / "scripts" / "analyze_nexus_vdp1_source_join.py"),
        str(capture),
        "--data-dir",
        data_root,
        "--ymir-bundle",
    ]
    result = subprocess.run(command, check=False, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        return result.returncode

    required_receipts = (
        "frame=8037 draw_commands=137 ",
        "source_joined_draws=126/137",
        "dgn_structure2_joined_draws=121/137",
        "source_uninformative_draws=11/137",
        "source_join=partial",
        "semantic_admission=blocked",
    )
    missing = [receipt for receipt in required_receipts
               if receipt not in result.stdout]
    if missing:
        print("FAIL: frame-8037 source join did not match the authentic receipt")
        print("missing=" + "|".join(missing))
        sys.stderr.write(result.stdout)
        return 1

    print("PASS: frame 8037 has 126/137 informative retail source joins, including 121 DGN matches")
    print("PASS: the remaining 11 texture spans are uninformative zero data; semantic admission remains blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

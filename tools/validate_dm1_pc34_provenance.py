#!/usr/bin/env python3
"""Validate DM1 PC 3.4 source-data provenance for Firestaff.

Checks the worker-VM local canonical archive and extracted PC34 data set against
the locked checksums used as Firestaff V1 source-data truth. This helper is
intentionally local-only; worker subagents must not depend on DANNESBURK/SSH.
"""
from __future__ import annotations

import argparse
import hashlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

EXPECTED = {
    "DUNGEON.DAT": ("d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85", 33357),
    "GRAPHICS.DAT": ("2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e", 363417),
    "SONG.DAT": ("71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177", 162482),
}

DEFAULT_DM_ROOT = Path.home() / ".openclaw/data/firestaff-original-games/DM"
DEFAULT_ARCHIVE = DEFAULT_DM_ROOT / "Game,Dungeon_Master,DOS,Software.7z"
DEFAULT_EXTRACTED_DATA = DEFAULT_DM_ROOT / "_extracted/dm-pc34/DungeonMasterPC34/DATA"


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def check_file(label: str, path: Path) -> bool:
    expected_hash, expected_size = EXPECTED[path.name]
    actual_hash = sha256(path)
    actual_size = path.stat().st_size
    ok = actual_hash == expected_hash and actual_size == expected_size
    status = "OK" if ok else "FAIL"
    print(f"{status} {label}/{path.name} sha256={actual_hash} bytes={actual_size}")
    if not ok:
        print(f"  expected sha256={expected_hash} bytes={expected_size}", file=sys.stderr)
    return ok


def validate_data_dir(label: str, data_dir: Path) -> bool:
    if not data_dir.is_dir():
        print(f"FAIL {label}: data directory not found: {data_dir}", file=sys.stderr)
        return False
    ok = True
    for name in EXPECTED:
        path = data_dir / name
        if not path.exists():
            print(f"FAIL {label}: missing {path}", file=sys.stderr)
            ok = False
            continue
        ok = check_file(label, path) and ok
    return ok


def validate_local_archive(archive: Path) -> bool:
    sevenzip = shutil.which("7zz") or shutil.which("7z")
    if not sevenzip:
        print("FAIL local-archive: missing 7zz/7z required to extract .7z archive", file=sys.stderr)
        return False
    if not archive.exists():
        print(f"FAIL local-archive: archive not found: {archive}", file=sys.stderr)
        return False
    with tempfile.TemporaryDirectory(prefix="firestaff-pc34-provenance-") as tmp:
        subprocess.run([sevenzip, "x", "-y", f"-o{tmp}", str(archive)], check=True, stdout=subprocess.DEVNULL)
        data_dir = Path(tmp) / "DungeonMasterPC34/DATA"
        return validate_data_dir("local-archive", data_dir)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, default=DEFAULT_ARCHIVE)
    parser.add_argument("--extracted-data", type=Path, default=DEFAULT_EXTRACTED_DATA)
    parser.add_argument("--skip-archive", action="store_true")
    parser.add_argument("--check-extracted", action="store_true", help="Require the extracted PC34 data directory to exist and match checksums")
    args = parser.parse_args()

    ok = True
    if not args.skip_archive:
        ok = validate_local_archive(args.archive) and ok
    if args.check_extracted:
        ok = validate_data_dir("local-extracted", args.extracted_data) and ok
    elif args.extracted_data.is_dir():
        ok = validate_data_dir("local-extracted", args.extracted_data) and ok
    else:
        print(f"SKIP local-extracted: data directory not present: {args.extracted_data}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

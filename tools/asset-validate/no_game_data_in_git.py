#!/usr/bin/env python3
"""Fail if original game-data payloads are tracked by git.

Firestaff may track hashes, manifests, source references, synthetic fixtures,
and user instructions for where to put local data. It must never publish the
commercial game files themselves.
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path


ALLOWED_FIXTURES = {
    "scripts/fixtures/nexus_v1_save_synthetic.dat",
    "tests/fixtures/minimal.DAT",
}

FORBIDDEN_BASENAMES = {
    "graphics.dat",
    "dungeon.dat",
    "dungeonb.dat",
    "dungeonf.dat",
    "dungeong.dat",
    "song.dat",
    "hcsb.dat",
    "hcsb.htc",
    "mini.dat",
    "naked.amg",
    "dm.bin",
    "font256.s2d",
    "item.ibs",
    "menu.bpk",
}

FORBIDDEN_SUFFIXES = {
    ".adf",
    ".avi",
    ".bin",
    ".cue",
    ".dsk",
    ".fdi",
    ".hdm",
    ".hdi",
    ".img",
    ".iso",
    ".lha",
    ".lzh",
    ".moov",
    ".mve",
    ".raw",
    ".rar",
    ".sit",
    ".st",
    ".zip",
    ".7z",
}


def tracked_files() -> list[str]:
    out = subprocess.check_output(["git", "ls-files", "-z"])
    return [p.decode("utf-8", "replace") for p in out.split(b"\0") if p]


def is_forbidden(path: str) -> bool:
    if path in ALLOWED_FIXTURES:
        return False
    name = Path(path).name.lower()
    if name in FORBIDDEN_BASENAMES:
        return True
    return Path(path).suffix.lower() in FORBIDDEN_SUFFIXES


def main() -> int:
    offenders = [p for p in tracked_files() if is_forbidden(p)]
    if offenders:
        print("FAIL: original game-data-like files are tracked by git:", file=sys.stderr)
        for path in offenders:
            print(f"  {path}", file=sys.stderr)
        print(
            "\nKeep commercial game data only in a local data directory, "
            "for example ~/.firestaff/data/.",
            file=sys.stderr,
        )
        return 1
    print("PASS: no original game-data payloads are tracked by git")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

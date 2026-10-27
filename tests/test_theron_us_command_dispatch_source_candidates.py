#!/usr/bin/env python3
"""Bind captured USA mouse-dispatch code windows to retail Track 02 copies."""

from __future__ import annotations

import hashlib
import pathlib
import sys


EXPECTED_SIZE = 8_104_992
EXPECTED_MD5 = "f23601102138f87c33025877767ebf76"
EXPECTED_SHA256 = "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565"
CANDIDATE_BASES = (
    671_579,
    972_635,
    1_273_691,
    1_574_747,
    1_875_803,
    2_176_859,
    2_477_915,
)
WINDOWS = {
    0xCCDB: (
        "fbfe13d7288780ce8191ae58151dd20a9d52fe255aa5541ad248dd8b76d333e3",
        0,
    ),
    0xD34D: (
        "abf492a47b244a2d3a2185b765e1bf27a2583b4cc40466d15b3c86baf9ff941a",
        1_954,
    ),
    0xD395: (
        "1df987fd3f39cab8af44dea067ac0cf89168e1fc41ae98e85b1bde7355c33634",
        2_026,
    ),
    0xD3A0: (
        "70f0f8beed7cefd609a0dc0db53a69f83a4046104f4efe0c6bdc3b13f5c8a760",
        2_037,
    ),
}
WINDOW_BYTES = 32


def find_all(data: bytes, needle: bytes) -> list[int]:
    hits: list[int] = []
    offset = 0
    while True:
        offset = data.find(needle, offset)
        if offset < 0:
            return hits
        hits.append(offset)
        offset += 1


def verify(path: pathlib.Path) -> None:
    raw = path.read_bytes()
    if len(raw) != EXPECTED_SIZE:
        raise ValueError(f"unexpected USA Track 02 size: {len(raw)}")
    if hashlib.md5(raw).hexdigest() != EXPECTED_MD5:
        raise ValueError("USA Track 02 MD5 mismatch")
    if hashlib.sha256(raw).hexdigest() != EXPECTED_SHA256:
        raise ValueError("USA Track 02 SHA-256 mismatch")

    windows: dict[int, bytes] = {}
    for pc, (expected_hash, displacement) in WINDOWS.items():
        base_offset = CANDIDATE_BASES[0] + displacement
        window = raw[base_offset : base_offset + WINDOW_BYTES]
        if len(window) != WINDOW_BYTES:
            raise ValueError(f"candidate window at ${pc:04X} is truncated")
        if hashlib.sha256(window).hexdigest() != expected_hash:
            raise ValueError(f"candidate bytes at ${pc:04X} changed")
        expected_hits = [base + displacement for base in CANDIDATE_BASES]
        actual_hits = find_all(raw, window)
        if actual_hits != expected_hits:
            raise ValueError(
                f"USA Track 02 candidates at ${pc:04X} differ: {actual_hits}"
            )
        windows[pc] = window

    for pc, (_, displacement) in WINDOWS.items():
        if windows[pc] != raw[CANDIDATE_BASES[0] + displacement :
                               CANDIDATE_BASES[0] + displacement + WINDOW_BYTES]:
            raise ValueError(f"repeated candidate mismatch at ${pc:04X}")
    if raw.find(windows[0xCCDB], CANDIDATE_BASES[0] + 1) != CANDIDATE_BASES[1]:
        raise ValueError("first command-code window has an unexpected later candidate")


def main() -> int:
    root = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else (
        pathlib.Path.home() / ".firestaff" / "data"
    )
    path = root / "theron" / "TQUS02.bin"
    if not path.is_file():
        print(f"SKIP: authentic USA Track 02 unavailable: {path}")
        return 77
    try:
        verify(path)
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS: four observed USA dispatch code windows match exactly seven retail Track 02 candidates")
    print("LIMIT: static duplicate spans do not identify the loaded runtime source copy")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

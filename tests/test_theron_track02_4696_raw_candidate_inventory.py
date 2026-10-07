#!/usr/bin/env python3
"""Inventory raw `$4696` JSR/JMP byte patterns in authentic Track 02 data."""

from __future__ import annotations

import hashlib
import pathlib
import sys


RAW_SECTOR_BYTES = 2352
USER_SECTOR_BYTES = 2048
STAGE2_RECORD = 0x3E7
STAGE2_SECTOR_COUNT = 17
STAGE2_LOAD_ADDRESS = 0x4000

EDITIONS = {
    "US": {
        "size": 8104992,
        "index01_sector": 225,
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
    },
    "JP": {
        "size": 8102640,
        "index01_sector": 224,
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
    },
}

# Sector offsets are relative to INDEX 01; each location is a MODE1 user-data
# byte offset. The first candidate is 68 sectors before the Stage 2 record.
EXPECTED_JSR_LOCATIONS = (
    (0x3E7 - 68, 0x689),
    (0x3E7 + 8, 0x06A),
    (0x3E7 + 8, 0x5BA),
    (0x3E7 + 8, 0x5CB),
)
EXPECTED_STAGE2_JSR_LOCATIONS = ((8, 0x06A), (8, 0x5BA), (8, 0x5CB))


def extract_track_user_data(raw: bytes, index01_sector: int) -> bytes:
    if len(raw) % RAW_SECTOR_BYTES:
        raise ValueError("Track 02 size is not a whole number of raw sectors")
    user_data = bytearray()
    for sector in range(index01_sector, len(raw) // RAW_SECTOR_BYTES):
        sector_start = sector * RAW_SECTOR_BYTES
        user_start = sector_start + 16
        user_data.extend(raw[user_start:user_start + USER_SECTOR_BYTES])
    return bytes(user_data)


def raw_locations(user_data: bytes, pattern: bytes) -> tuple[tuple[int, int], ...]:
    locations = []
    offset = 0
    while True:
        offset = user_data.find(pattern, offset)
        if offset < 0:
            return tuple(locations)
        sector_offset, user_offset = divmod(offset, USER_SECTOR_BYTES)
        locations.append((sector_offset, user_offset))
        offset += 1


def verify_edition(edition: str, path: pathlib.Path) -> bool:
    expected = EDITIONS[edition]
    if not path.is_file():
        print(f"SKIP: authentic {edition} Track 02 is unavailable: {path}")
        return False

    observed_hashes = []
    for pass_number in range(1, 4):
        if path.stat().st_size != expected["size"]:
            raise ValueError(f"{edition} Track 02 size mismatch")
        with path.open("rb") as media_file:
            raw = media_file.read(expected["size"] + 1)
        if len(raw) != expected["size"]:
            raise ValueError(f"{edition} Track 02 changed during bounded read")
        media_hash = hashlib.sha256(raw).hexdigest()
        if media_hash != expected["sha256"]:
            raise ValueError(f"{edition} Track 02 SHA-256 mismatch: {media_hash}")

        user_data = extract_track_user_data(raw, expected["index01_sector"])
        observed_jsr = raw_locations(user_data, bytes.fromhex("20 96 46"))
        observed_jmp = raw_locations(user_data, bytes.fromhex("4c 96 46"))
        stage2_jsr = tuple(
            (sector_offset - STAGE2_RECORD, user_offset)
            for sector_offset, user_offset in observed_jsr
            if STAGE2_RECORD <= sector_offset < STAGE2_RECORD + STAGE2_SECTOR_COUNT
        )
        if (observed_jsr != EXPECTED_JSR_LOCATIONS or
                stage2_jsr != EXPECTED_STAGE2_JSR_LOCATIONS or observed_jmp):
            raise ValueError(
                f"{edition} raw pattern mismatch: JSR={observed_jsr}, "
                f"Stage2 JSR={stage2_jsr}, JMP={observed_jmp}"
            )

        observed_hashes.append(media_hash)
        print(f"PASS: {edition} authenticated Track 02 scan {pass_number}")

    if len(set(observed_hashes)) != 1:
        raise ValueError(f"{edition} Track 02 changed between verification passes")
    return True


def main() -> int:
    data_root = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else (
        pathlib.Path.home() / ".firestaff" / "data"
    )
    theron_root = data_root / "theron"
    us_present = verify_edition("US", theron_root / "TQUS02.bin")
    jp_present = verify_edition("JP", theron_root / "TQJP02.bin")
    if not us_present or not jp_present:
        return 77
    print("PASS: exact raw $4696 JSR/JMP byte-candidate inventory")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

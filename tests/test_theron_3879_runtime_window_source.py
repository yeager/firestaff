#!/usr/bin/env python3
"""Bind the captured below-window $3879 bytes to authentic Track 02 media."""

from __future__ import annotations

import hashlib
import pathlib
import re
import sys


SNAPSHOT_SIZE = 8192
SNAPSHOT_SHA256 = "58a31b22a409ac10f49e95b73b10603985ec29254504ef61f0932fa66a1c9cfa"
SNAPSHOT_OFFSET = 0x1879
WINDOW_SIZE = 0xA0
RAW_SECTOR_SIZE = 2352
USER_DATA_OFFSET = 16
LISTING = pathlib.Path(__file__).resolve().parents[1] / (
    "docs/source-lock/theron-disassembly/theron-us-captured-3879-linear-huc6280.asm"
)

EDITIONS = {
    "US": {
        "name": "TQUS02.bin",
        "size": 8104992,
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "index01_sector": 225,
        "track_file_offset": 534041,
    },
    "JP": {
        "name": "TQJP02.bin",
        "size": 8102640,
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "index01_sector": 224,
        "track_file_offset": 531689,
    },
}


def read_bounded(path: pathlib.Path, expected_size: int) -> bytes:
    if path.stat().st_size != expected_size:
        raise ValueError(f"unexpected size for {path}")
    with path.open("rb") as source:
        contents = source.read(expected_size + 1)
    if len(contents) != expected_size:
        raise ValueError(f"bounded read size changed for {path}")
    return contents


def read_listing_bytes() -> bytes:
    expected_pc = 0x3879
    listing_bytes = bytearray()
    line_pattern = re.compile(r"^([0-9a-f]{4}): ((?:[0-9a-f]{2} ?)+)")
    for line in LISTING.read_text(encoding="utf-8").splitlines():
        match = line_pattern.match(line)
        if not match:
            continue
        address = int(match.group(1), 16)
        encoded = bytes.fromhex(match.group(2))
        if address != expected_pc:
            raise ValueError(
                f"listing address discontinuity: expected ${expected_pc:04x}, "
                f"got ${address:04x}"
            )
        listing_bytes.extend(encoded)
        expected_pc += len(encoded)
    if len(listing_bytes) != WINDOW_SIZE or expected_pc != 0x3919:
        raise ValueError("linear listing does not cover the exact 160-byte window")
    return bytes(listing_bytes)


def verify_edition(
    edition: str, root: pathlib.Path, expected_window: bytes
) -> bool:
    expected = EDITIONS[edition]
    media_path = root / expected["name"]
    if not media_path.is_file():
        print(f"SKIP: authentic {edition} Track 02 unavailable: {media_path}")
        return False

    observed_hashes = []
    for pass_number in range(1, 4):
        raw = read_bounded(media_path, expected["size"])
        digest = hashlib.sha256(raw).hexdigest()
        if digest != expected["sha256"]:
            raise ValueError(f"{edition} Track 02 SHA-256 mismatch: {digest}")
        hits = []
        offset = 0
        while True:
            offset = raw.find(expected_window, offset)
            if offset < 0:
                break
            hits.append(offset)
            offset += 1
        if hits != [expected["track_file_offset"]]:
            raise ValueError(f"{edition} $3879 window locations differ: {hits}")

        sector, raw_byte = divmod(hits[0], RAW_SECTOR_SIZE)
        if (sector != expected["index01_sector"] + 2 or
                raw_byte != 137 or raw_byte < USER_DATA_OFFSET):
            raise ValueError(
                f"{edition} source coordinate mismatch: sector={sector}, "
                f"raw-byte={raw_byte}"
            )
        user_byte = raw_byte - USER_DATA_OFFSET
        if user_byte != 121:
            raise ValueError(f"{edition} MODE1 user-data offset mismatch")

        observed_hashes.append(digest)
        print(f"PASS: {edition} authentic $3879 window scan {pass_number}")

    if len(set(observed_hashes)) != 1:
        raise ValueError(f"{edition} Track 02 changed across verification passes")
    return True


def main() -> int:
    data_root = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else (
        pathlib.Path.home() / ".firestaff" / "data"
    )
    theron_root = data_root / "theron"
    capture_path = theron_root / "capture" / (
        "theron-us-akutuba-complete-main-ram.bin"
    )
    if not capture_path.is_file():
        print(f"SKIP: authentic Theron BaseRAM snapshot unavailable: {capture_path}")
        return 77
    snapshot = read_bounded(capture_path, SNAPSHOT_SIZE)
    snapshot_hash = hashlib.sha256(snapshot).hexdigest()
    if snapshot_hash != SNAPSHOT_SHA256:
        raise ValueError(f"BaseRAM snapshot SHA-256 mismatch: {snapshot_hash}")
    if SNAPSHOT_OFFSET + WINDOW_SIZE > len(snapshot):
        raise ValueError("$3879 window exceeds the captured BaseRAM image")
    window = snapshot[SNAPSHOT_OFFSET:SNAPSHOT_OFFSET + WINDOW_SIZE]
    if read_listing_bytes() != window:
        raise ValueError("HuC6280 listing byte columns differ from the captured window")

    us_present = verify_edition("US", theron_root, window)
    jp_present = verify_edition("JP", theron_root, window)
    if not us_present or not jp_present:
        return 77
    print("PASS: authentic captured BaseRAM $3879 window is uniquely bound to US/JP Track 02")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

#!/usr/bin/env python3
"""Bind the captured below-window $3879 bytes to authentic Track 02 media."""

from __future__ import annotations

import hashlib
import pathlib
import re
import sys


SNAPSHOT_SIZE = 8192
SNAPSHOT_SHA256 = "58a31b22a409ac10f49e95b73b10603985ec29254504ef61f0932fa66a1c9cfa"
RAW_SECTOR_SIZE = 2352
USER_SECTOR_SIZE = 2048
USER_DATA_OFFSET = 16
LISTING_DIR = pathlib.Path(__file__).resolve().parents[1] / (
    "docs/source-lock/theron-disassembly"
)

WINDOWS = (
    {
        "name": "$3879",
        "snapshot_offset": 0x1879,
        "size": 0xA0,
        "track_user_offset": 2 * USER_SECTOR_SIZE + 121,
        "us_file_offset": 534041,
        "jp_file_offset": 531689,
        "listing": "theron-us-captured-3879-linear-huc6280.asm",
        "listing_pc": 0x3879,
    },
    {
        "name": "$39c0",
        "snapshot_offset": 0x19C0,
        "size": 0x50,
        "track_user_offset": 2 * USER_SECTOR_SIZE + 448,
        "us_file_offset": 534368,
        "jp_file_offset": 532016,
        "listing": "theron-us-captured-39c0-linear-huc6280.asm",
        "listing_pc": 0x39C0,
    },
    {
        "name": "$39e0",
        "snapshot_offset": 0x19E0,
        "size": 0x620,
        "track_user_offset": 2 * USER_SECTOR_SIZE + 480,
        "us_file_offset": 534400,
        "jp_file_offset": 532048,
        "listing": None,
        "listing_pc": 0x39E0,
    },
)

EDITIONS = {
    "US": {
        "name": "TQUS02.bin",
        "size": 8104992,
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "index01_sector": 225,
    },
    "JP": {
        "name": "TQJP02.bin",
        "size": 8102640,
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "index01_sector": 224,
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


def read_listing_bytes(path: pathlib.Path, start_pc: int, expected_size: int) -> bytes:
    expected_pc = start_pc
    listing_bytes = bytearray()
    line_pattern = re.compile(r"^([0-9a-f]{4}): ((?:[0-9a-f]{2} ?)+)")
    for line in path.read_text(encoding="utf-8").splitlines():
        match = line_pattern.match(line)
        if not match:
            continue
        address = int(match.group(1), 16)
        encoded = bytes.fromhex(match.group(2))
        if address < start_pc:
            continue
        if address >= start_pc + expected_size:
            break
        if address != expected_pc:
            raise ValueError(
                f"listing address discontinuity: expected ${expected_pc:04x}, "
                f"got ${address:04x}"
            )
        listing_bytes.extend(encoded)
        expected_pc += len(encoded)
    if len(listing_bytes) != expected_size or expected_pc != start_pc + expected_size:
        raise ValueError(f"linear listing does not cover ${start_pc:04x} window")
    return bytes(listing_bytes)


def verify_edition(
    edition: str, root: pathlib.Path, snapshot: bytes
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
        track_user_data = b"".join(
            raw[sector * RAW_SECTOR_SIZE + USER_DATA_OFFSET:
                sector * RAW_SECTOR_SIZE + USER_DATA_OFFSET + USER_SECTOR_SIZE]
            for sector in range(
                expected["index01_sector"], len(raw) // RAW_SECTOR_SIZE
            )
        )
        for window in WINDOWS:
            start = window["snapshot_offset"]
            expected_window = snapshot[start:start + window["size"]]
            expected_user_offset = window["track_user_offset"]
            user_hits = []
            offset = 0
            while True:
                offset = track_user_data.find(expected_window, offset)
                if offset < 0:
                    break
                user_hits.append(offset)
                offset += 1
            if user_hits != [expected_user_offset]:
                raise ValueError(
                    f"{edition} {window['name']} user-data locations differ: "
                    f"{user_hits}"
                )

            sector_delta, user_byte = divmod(expected_user_offset, USER_SECTOR_SIZE)
            raw_offset = (
                (expected["index01_sector"] + sector_delta) * RAW_SECTOR_SIZE
                + USER_DATA_OFFSET + user_byte
            )
            expected_file_offset = window[f"{edition.lower()}_file_offset"]
            raw_hits = []
            offset = 0
            while True:
                offset = raw.find(expected_window, offset)
                if offset < 0:
                    break
                raw_hits.append(offset)
                offset += 1
            if raw_offset != expected_file_offset or raw_hits != [raw_offset]:
                raise ValueError(
                    f"{edition} {window['name']} raw-source coordinates differ: "
                    f"expected={expected_file_offset}, derived={raw_offset}, "
                    f"matches={raw_hits}"
                )

        observed_hashes.append(digest)
        print(f"PASS: {edition} authentic BaseRAM source windows scan {pass_number}")

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
    for window in WINDOWS:
        start = window["snapshot_offset"]
        end = start + window["size"]
        if end > len(snapshot):
            raise ValueError(f"{window['name']} window exceeds captured BaseRAM")
        if window["listing"]:
            listed = read_listing_bytes(
                LISTING_DIR / window["listing"],
                window["listing_pc"], window["size"]
            )
            if listed != snapshot[start:end]:
                raise ValueError(
                    f"{window['name']} listing bytes differ from captured BaseRAM"
                )

    us_present = verify_edition("US", theron_root, snapshot)
    jp_present = verify_edition("JP", theron_root, snapshot)
    if not us_present or not jp_present:
        return 77
    print("PASS: authentic captured BaseRAM $3879/$39c0/$39e0 windows are uniquely bound to US/JP Track 02")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

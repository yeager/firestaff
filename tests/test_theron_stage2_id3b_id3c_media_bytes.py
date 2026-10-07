#!/usr/bin/env python3
"""Source-lock selected Theron Stage 2 bytes against authentic Track 02 media."""

from __future__ import annotations

import hashlib
import pathlib
import sys


RAW_SECTOR_BYTES = 2352
USER_SECTOR_BYTES = 2048
STAGE2_RECORD = 0x3E7
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

EXPECTED_SPANS = (
    (0x4183, bytes.fromhex("7f44")),
    (0x4185, bytes.fromhex("6248")),
    (0x447F, bytes.fromhex("440280bc")),
    (0x443F, bytes.fromhex("6220b73a4cf540")),
    (0x4862, bytes.fromhex(
        "2063e0ad2822eaead012a99585faa94885fb201ee0ad9548f0e680"
        "0da90c202de0a214202d4bcad0fa2018e062202de04cf140"
    )),
    (0x4895, bytes(10)),
    (0x489F, bytes.fromhex("2063e0ad2822f0eb8d802780e6")),
)


def stage2_bytes_at(
    raw: bytes, stage2_sector: int, cpu_address: int, length: int
) -> bytes:
    if cpu_address < STAGE2_LOAD_ADDRESS or length < 0:
        raise ValueError("invalid Stage 2 CPU address or length")

    payload_offset = cpu_address - STAGE2_LOAD_ADDRESS
    result = bytearray()
    for byte_offset in range(length):
        offset = payload_offset + byte_offset
        sector = stage2_sector + offset // USER_SECTOR_BYTES
        raw_offset = sector * RAW_SECTOR_BYTES + 16 + offset % USER_SECTOR_BYTES
        if raw_offset >= len(raw):
            raise ValueError("Stage 2 byte range exceeds the authentic image")
        result.append(raw[raw_offset])
    return bytes(result)


def verify_edition(edition: str, path: pathlib.Path) -> bool:
    expected = EDITIONS[edition]
    if not path.is_file():
        print(f"SKIP: authentic {edition} Track 02 is unavailable: {path}")
        return False

    stage2_sector = expected["index01_sector"] + STAGE2_RECORD
    observed_hashes = []
    for pass_number in range(1, 4):
        media_size = path.stat().st_size
        if media_size != expected["size"]:
            raise ValueError(
                f"{edition} Track 02 size mismatch: expected "
                f"{expected['size']} bytes, got {media_size}"
            )
        with path.open("rb") as media_file:
            raw = media_file.read(expected["size"] + 1)
        if len(raw) != expected["size"]:
            raise ValueError(f"{edition} Track 02 changed during bounded read")
        media_hash = hashlib.sha256(raw).hexdigest()
        if media_hash != expected["sha256"]:
            raise ValueError(
                f"{edition} Track 02 SHA-256 mismatch: {media_hash}"
            )

        for address, wanted in EXPECTED_SPANS:
            actual = stage2_bytes_at(raw, stage2_sector, address, len(wanted))
            if actual != wanted:
                raise ValueError(
                    f"{edition} Stage 2 bytes mismatch at ${address:04x}: "
                    f"{actual.hex()}"
                )

        observed_hashes.append(media_hash)
        print(f"PASS: {edition} authentic Track 02 source-lock pass {pass_number}")

    if len(set(observed_hashes)) != 1:
        raise ValueError(f"{edition} Track 02 changed between verification passes")
    return True


def main() -> int:
    if len(sys.argv) > 1:
        data_root = pathlib.Path(sys.argv[1])
    else:
        data_root = pathlib.Path.home() / ".firestaff" / "data"
    theron_root = data_root / "theron"
    us_present = verify_edition("US", theron_root / "TQUS02.bin")
    jp_present = verify_edition("JP", theron_root / "TQJP02.bin")
    if not us_present or not jp_present:
        return 77
    print("PASS: Theron ID $3b/$3c authentic Track 02 byte locks")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

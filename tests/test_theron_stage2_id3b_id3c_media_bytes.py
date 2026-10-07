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

EXPECTED_RECORD_USER_SPANS = (
    (
        0x5E2B,
        0x56,
        "02f993418bd7a54dc2679854de4f98bc61f43b4fbc676978305c37bd9f3e4a9e",
    ),
    (
        0x5CE4,
        0x38,
        "f9732c087fd6f61fcc5449282154b83b4f7ac75595375f1a0c5df1a6a886a8cf",
    ),
    (
        0x5D1C,
        0x77,
        "08a58b9b934100232bfd381845cead9b11bd88904c76bf3128f9f8467bdfaee0",
    ),
    (
        0x5D93,
        0x48,
        "e6bd85ceb98a37737b5d8e0002aeaff2f5029978ff998ded34678e1240413339",
    ),
    (
        0x5DDB,
        0x1A,
        "c1becb66780c655428f31f2b1bbd8bf92f62ced603949614cd35b17597b05453",
    ),
    (
        0x5DF5,
        0x21,
        "aa37228d4a9401afabc88c37ee57a7b18dc29a7c0cba3b35a9f44d635ef098dd",
    ),
    (
        0x5E16,
        0x15,
        "3c0d8e66d4c9dfb7ee5c8d3fb385c5e90c0dc527ff02be79041ecc7b2f873c24",
    ),
)

EXPECTED_RECORD_USER_BYTES = (
    (0x5E81, bytes.fromhex("395e615e805e")),
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


def stage2_record_user_bytes_at(
    raw: bytes, stage2_sector: int, user_offset: int, length: int
) -> bytes:
    if user_offset < 0 or length < 0:
        raise ValueError("invalid Stage 2 record user offset or length")

    result = bytearray()
    for byte_offset in range(length):
        offset = user_offset + byte_offset
        sector = stage2_sector + offset // USER_SECTOR_BYTES
        raw_offset = (
            sector * RAW_SECTOR_BYTES + 16 + offset % USER_SECTOR_BYTES
        )
        if raw_offset >= len(raw):
            raise ValueError(
                "Stage 2 record user range exceeds the authentic image"
            )
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

        for user_offset, length, wanted_hash in EXPECTED_RECORD_USER_SPANS:
            actual = stage2_record_user_bytes_at(
                raw, stage2_sector, user_offset, length
            )
            actual_hash = hashlib.sha256(actual).hexdigest()
            if actual_hash != wanted_hash:
                raise ValueError(
                    f"{edition} Stage 2 record user span mismatch at "
                    f"${user_offset:04x}: {actual_hash}"
                )

        for user_offset, wanted in EXPECTED_RECORD_USER_BYTES:
            actual = stage2_record_user_bytes_at(
                raw, stage2_sector, user_offset, len(wanted)
            )
            if actual != wanted:
                raise ValueError(
                    f"{edition} Stage 2 record user bytes mismatch at "
                    f"${user_offset:04x}: {actual.hex()}"
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
    print("PASS: Theron authentic Track 02 byte locks")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

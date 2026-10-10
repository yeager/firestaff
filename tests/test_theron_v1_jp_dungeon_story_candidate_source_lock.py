#!/usr/bin/env python3
"""Lock JP Track 02 story-byte candidates without claiming runtime selection."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import sys


MEDIA_FILENAME = "TQJP02.bin"
MEDIA_MD5 = "b7afb338ad31be1025b53f9aff12d73a"
MEDIA_SHA256 = "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb"
MEDIA_SIZE = 8_102_640
RAW_SECTOR_BYTES = 2352
USER_DATA_OFFSET = 16
USER_DATA_BYTES = 2048
CANDIDATES = (
    ("Akutuba", 0x27596D, 0x275A97,
     "b30c9c76ab673802941823ea418be67545b068d9a7f94a9c6a7b106a1b6633aa"),
    ("Drator", 0x275A97, 0x275BBF,
     "259cb43b304665a859dca1cfdd3eae323413f8112727d7b8f2ec837b154f3399"),
    ("Formic", 0x275BBF, 0x275D2F,
     "9e7827bdfba08e04bdb1fb49935a4887804a80fb436b04fd50f0b509be4ac1c2"),
    ("Sarmon", 0x275D2F, 0x275E59,
     "51d6c052811d34977cc0e92e02c4180e9289564ce5064fc30ac8e7dd65e83cb2"),
    ("Shado", 0x275E59, 0x275FCF,
     "efc328485426b7ee8f8f3e075affd554c048a0c2424ce02e879de0c106c0704b"),
    ("Thief", 0x275FCF, 0x276187,
     "ad4e873b0b2c2f5aa2335b778fbb32aa7b0845e6d9c37bcfe07b05f1bf64b9a3"),
    ("Demon", 0x276187, 0x2762A3,
     "928fd95dd5d8e23cc46eb4f59e4b7987580eda42873364a4c16d8a7bdbf385f6"),
)


def find_all(data: bytes, needle: bytes) -> tuple[int, ...]:
    offsets = []
    cursor = 0
    while True:
        cursor = data.find(needle, cursor)
        if cursor < 0:
            return tuple(offsets)
        offsets.append(cursor)
        cursor += 1


def main() -> int:
    media_root = Path(
        os.environ.get(
            "FIRESTAFF_THERON_TEST_DATA_DIR",
            Path.home() / ".firestaff/data/theron",
        )
    )
    media_path = media_root / MEDIA_FILENAME
    if not media_path.is_file():
        print("SKIP: authentic JP Theron Track 02 media is unavailable")
        return 77

    if media_path.stat().st_size != MEDIA_SIZE:
        raise AssertionError(
            f"expected {MEDIA_SIZE} raw bytes, got {media_path.stat().st_size}"
        )
    with media_path.open("rb") as media_file:
        image = media_file.read(MEDIA_SIZE + 1)
    if len(image) != MEDIA_SIZE:
        raise AssertionError(f"bounded media read returned {len(image)} bytes")
    actual_md5 = hashlib.md5(image).hexdigest()
    actual_sha256 = hashlib.sha256(image).hexdigest()
    if actual_md5 != MEDIA_MD5 or actual_sha256 != MEDIA_SHA256:
        raise AssertionError(
            "JP Track 02 identity mismatch: "
            f"MD5/SHA-256 {actual_md5}/{actual_sha256}"
        )
    if MEDIA_SIZE % RAW_SECTOR_BYTES:
        raise AssertionError("JP Track 02 is not a complete MODE1/2352 stream")

    user_data = b"".join(
        image[offset + USER_DATA_OFFSET:offset + USER_DATA_OFFSET + USER_DATA_BYTES]
        for offset in range(0, MEDIA_SIZE, RAW_SECTOR_BYTES)
    )
    previous_end = None
    for name, start, end, expected_sha256 in CANDIDATES:
        if previous_end is not None and start != previous_end:
            raise AssertionError(f"{name}: candidate spans are not contiguous")
        if start < 0 or end <= start or end > len(user_data):
            raise AssertionError(f"{name}: candidate span is out of bounds")
        candidate = user_data[start:end]
        if candidate[:2] != b"\x81\x50" or candidate[-2:] != b"\x81\x97":
            raise AssertionError(f"{name}: candidate framing bytes changed")
        if hashlib.sha256(candidate).hexdigest() != expected_sha256:
            raise AssertionError(f"{name}: authentic candidate bytes changed")
        if find_all(user_data, candidate) != (start,):
            raise AssertionError(f"{name}: candidate span is not unique in Track 02")
        candidate.decode("cp932", errors="strict")
        print(f"PASS: authentic JP {name} story-byte candidate at UD {start:#x}")
        previous_end = end

    if CANDIDATES[-1][2] != previous_end:
        raise AssertionError("final candidate end does not match the observed bank end")
    print(
        "LIMIT: static byte candidates only; selector binding, control semantics, "
        "runtime consumption, and presentation remain unproven"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

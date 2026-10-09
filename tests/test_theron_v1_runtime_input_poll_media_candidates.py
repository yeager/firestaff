#!/usr/bin/env python3
"""Lock authentic Track 02 candidates matching the JP runtime poll trace.

This test proves exact bytes and raw offsets in authenticated media only. It
does not identify which duplicate was loaded into CD RAM or assign game
semantics to the observed state fields.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import sys


MEDIA = {
    "jp": (
        "TQJP02.bin",
        "b7afb338ad31be1025b53f9aff12d73a",
        {
            "poll": (
                "ad 00 10 29 0f 0d b8 28 49 ff 8d b8 28 c9 0f d0 03 4c 00 e0 60",
                (0x95072, 0xDE872, 0x128072, 0x171872, 0x1BB072, 0x204872, 0x24E075),
            ),
            "consumer": (
                "ad b8 28 29 f0 cd 12 29 d0 0d",
                (0x9CF6F, 0xE676F, 0x12FF6F, 0x17976F, 0x1C2F6F, 0x20C76F, 0x255F6F),
            ),
            "consumer_controlflow": (
                "ad b8 28 29 f0 cd 12 29 d0 0d ee 20 29 ae 20 29 e0 0c b0 03 "
                "4c bc d3 9c 20 29 8d 12 29 73 0d 29 0f 29 02 00 ad b8 28 29 "
                "40 f0 04 a5 ca 80 09 ad b8 28 29 10 f0 0d a5 c9 c9 ff f0 51 "
                "c9 00 d0 3c 4c 1a d4 ad b8 28 29 80 f0 05 ce 0e 29 80 0a ad "
                "b8 28 29 20 f0 37 ee 0e 29 ad 0e 29 85 cb ad 0d 29 c9 00 f0 "
                "0d 20 ec d4 30 d9 d0",
                (0x9CF6F, 0xE676F, 0x12FF6F, 0x17976F, 0x1C2F6F, 0x20C76F, 0x255F6F),
            ),
            "consumer_helper": (
                "bc 16 d6 30 07 be 71 d4 44 18 d0 36 a2 03 73 0f 29 0d 29 02 00 "
                "20 ec d4 44 08 d0 26 ca 10 ef 4c 1a d4 bc 6d d4 b9 c7 20 c9 ff "
                "f0 14 c9 00 f0 10 da 8d 0d 29 20 79 d4 d0 03 fa 80 e6 fa a9 01 "
                "60 a9 00 60 a5 db f0 22 ad ba 2e 29 01 d0",
                (0x9D017, 0xE6817, 0x130017, 0x179817, 0x1C3017, 0x20C817, 0x256017),
            ),
            "consumer_store": (
                "9c 20 29 8d 12 29",
                (0x9CF6F + 0x17, 0xE676F + 0x17, 0x12FF6F + 0x17,
                 0x17976F + 0x17, 0x1C2F6F + 0x17, 0x20C76F + 0x17,
                 0x255F6F + 0x17),
            ),
            "caller_branch": (
                "ad 0d 29 f0 0d c9 0d 90 20",
                (0x9D12C, 0xE692C, 0x13012C, 0x17992C, 0x1C312C, 0x20C92C, 0x25612C),
            ),
            "indexed_table_prefix": (
                "0a aa bd ce 77 85 c5 bd cf 77 85 c6 a0 01 b1 c5 85 c7 c8 b1 c5 c9 fe d0 06",
                (0x9D155, 0xE6955, 0x130155, 0x179955, 0x1C3155, 0x20C955, 0x256155),
            ),
        },
    ),
    "us": (
        "TQUS02.bin",
        "f23601102138f87c33025877767ebf76",
        {
            "poll": (
                "ad 00 10 29 0f 0d b8 28 49 ff 8d b8 28 c9 0f d0 03 4c 00 e0 60",
                (0x959A8, 0xDF1A8, 0x1289A8, 0x1721A8, 0x1BB9A8, 0x2051A8, 0x24E9A8),
            ),
            "consumer": (
                "ad b8 28 29 f0 cd 12 29 d0 0d",
                (0x9D8AD, 0xE70AD, 0x1308AD, 0x17A0AD, 0x1C38AD, 0x20D0AD, 0x2568AD),
            ),
            "consumer_controlflow": (
                "ad b8 28 29 f0 cd 12 29 d0 0d ee 20 29 ae 20 29 e0 0c b0 03 "
                "4c ca d3 9c 20 29 8d 12 29 73 0d 29 0f 29 02 00 ad b8 28 29 "
                "40 f0 04 a5 ca 80 09 ad b8 28 29 10 f0 0d a5 c9 c9 ff f0 51 "
                "c9 00 d0 3c 4c 28 d4 ad b8 28 29 80 f0 05 ce 0e 29 80 0a ad "
                "b8 28 29 20 f0 37 ee 0e 29 ad 0e 29 85 cb ad 0d 29 c9 00 f0 "
                "0d 20 fa d4 30 d9 d0",
                (0x9D8AD, 0xE70AD, 0x1308AD, 0x17A0AD, 0x1C38AD, 0x20D0AD, 0x2568AD),
            ),
            "consumer_helper": (
                "bc 24 d6 30 07 be 7f d4 44 18 d0 36 a2 03 73 0f 29 0d 29 02 00 "
                "20 fa d4 44 08 d0 26 ca 10 ef 4c 28 d4 bc 7b d4 b9 c7 20 c9 ff "
                "f0 14 c9 00 f0 10 da 8d 0d 29 20 87 d4 d0 03 fa 80 e6 fa a9 01 "
                "60 a9 00 60 a5 db f0 22 ad bb 2e 29 01 d0",
                (0x9D955, 0xE7155, 0x130955, 0x17A155, 0x1C3955, 0x20D155, 0x256955),
            ),
            "consumer_store": (
                "9c 20 29 8d 12 29",
                (0x9D8AD + 0x17, 0xE70AD + 0x17, 0x1308AD + 0x17,
                 0x17A0AD + 0x17, 0x1C38AD + 0x17, 0x20D0AD + 0x17,
                 0x2568AD + 0x17),
            ),
            "caller_branch": (
                "ad 0d 29 f0 0d c9 0d 90 20",
                (0x9DA6A, 0xE726A, 0x130A6A, 0x17A26A, 0x1C3A6A, 0x20D26A, 0x256A6A),
            ),
            "indexed_table_prefix": (
                "0a aa bd dc 77 85 c5 bd dd 77 85 c6 a0 01 b1 c5 85 c7 c8 b1 c5 c9 fe d0 06",
                (0x9DA93, 0xE7293, 0x130A93, 0x17A293, 0x1C3A93, 0x20D293, 0x256A93),
            ),
        },
    ),
}

POLL_CONSUMER_WINDOWS = {
    "jp": (
        "2d187b0eb974b353c2c384f858d337e04068b952c188d85ba326f98cafa89353",
        (0x9CF6F, 0xE676F, 0x12FF6F, 0x17976F, 0x1C2F6F, 0x20C76F, 0x255F6F),
    ),
    "us": (
        "8baba7511f4bdb3a12073c925777d84030b82dd65ffbc12df855941079229091",
        (0x9D8AD, 0xE70AD, 0x1308AD, 0x17A0AD, 0x1C38AD, 0x20D0AD, 0x2568AD),
    ),
}
POLL_CONSUMER_WINDOW_BYTES = 0x100


def find_all(data: bytes | bytearray, needle: bytes) -> tuple[int, ...]:
    offsets: list[int] = []
    position = 0
    while True:
        position = data.find(needle, position)
        if position < 0:
            return tuple(offsets)
        offsets.append(position)
        position += 1


def require_candidates(data: bytes, label: str, hex_bytes: str, expected: tuple[int, ...]) -> None:
    needle = bytes.fromhex(hex_bytes)
    actual = find_all(data, needle)
    if actual != expected:
        raise AssertionError(
            f"{label}: expected {[hex(value) for value in expected]}, "
            f"got {[hex(value) for value in actual]}"
        )

    # Negative mutations only modify a temporary copy. Each real candidate
    # must disappear independently while the other exact copies remain.
    for offset in expected:
        mutated = bytearray(data)
        mutated[offset] ^= 0xFF
        remaining = find_all(mutated, needle)
        wanted = tuple(candidate for candidate in expected if candidate != offset)
        if remaining != wanted:
            raise AssertionError(f"{label}: mutation at {offset:#x} did not reject only that candidate")


def require_poll_consumer_windows(data: bytes, region: str) -> None:
    expected_sha256, offsets = POLL_CONSUMER_WINDOWS[region]
    for offset in offsets:
        window = data[offset : offset + POLL_CONSUMER_WINDOW_BYTES]
        actual_sha256 = hashlib.sha256(window).hexdigest()
        if len(window) != POLL_CONSUMER_WINDOW_BYTES or actual_sha256 != expected_sha256:
            raise AssertionError(
                f"{region} direction window at {offset:#x}: expected {expected_sha256}, "
                f"got {actual_sha256}"
            )
        mutated = bytearray(window)
        mutated[0] ^= 0xFF
        if hashlib.sha256(mutated).hexdigest() == expected_sha256:
            raise AssertionError(f"{region} direction window at {offset:#x} survived mutation")


def main() -> int:
    root = Path(os.environ.get("FIRESTAFF_THERON_TEST_DATA_DIR", Path.home() / ".firestaff/data/theron"))
    paths = {region: root / filename for region, (filename, _, _) in MEDIA.items()}
    missing = [path for path in paths.values() if not path.is_file()]
    if missing:
        print("SKIP: authentic Theron Track 02 media is unavailable")
        return 77

    for region, (filename, expected_md5, signatures) in MEDIA.items():
        path = paths[region]
        data = path.read_bytes()
        actual_md5 = hashlib.md5(data).hexdigest()
        if actual_md5 != expected_md5:
            raise AssertionError(f"{filename}: expected authentic MD5 {expected_md5}, got {actual_md5}")
        for label, (hex_bytes, offsets) in signatures.items():
            require_candidates(data, f"{region} {label}", hex_bytes, offsets)
        require_poll_consumer_windows(data, region)
        print(f"PASS: authentic {region.upper()} Track 02 caller candidates ({expected_md5})")

    print("PASS: poll, consumer/helper code, 256-byte window hashes, caller-branch, and indexed-table signatures with per-candidate negative mutations")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

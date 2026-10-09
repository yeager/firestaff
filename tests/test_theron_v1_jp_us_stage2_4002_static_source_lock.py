#!/usr/bin/env python3
"""Lock the static JP/US Track 02 candidate for sampled cold-boot PC $4002."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
LISTING_PATH = (
    ROOT
    / "docs/source-lock/theron-disassembly/"
    "theron-jp-us-cold-boot-stage2-4002-candidate-20261010.asm"
)
WINDOW_LENGTH = 127
LISTING_START_PC = 0x4002
EXPECTED_WINDOW = bytes.fromhex("73002001200f007300200027800062")
REGIONS = {
    "JP": {
        "filename": "TQJP02.bin",
        "md5": "b7afb338ad31be1025b53f9aff12d73a",
        "sha256": "d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb",
        "size": 8_102_640,
        "offset": 0x2973A2,
        "span_sha256": "9e0c7e6926c8ad3c02284e4027f37bcbd6eaa6cb4507f321cce814e4f2eb2aec",
    },
    "US": {
        "filename": "TQUS02.bin",
        "md5": "f23601102138f87c33025877767ebf76",
        "sha256": "f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565",
        "size": 8_104_992,
        "offset": 0x297CD2,
        "span_sha256": "9e0c7e6926c8ad3c02284e4027f37bcbd6eaa6cb4507f321cce814e4f2eb2aec",
    },
}
LISTING_ROW = re.compile(r"^([0-9a-f]{6}):\s*(.*?)\s{2,}[^\s].*$")


def all_offsets(data: bytes, needle: bytes) -> tuple[int, ...]:
    found: list[int] = []
    cursor = 0
    while True:
        cursor = data.find(needle, cursor)
        if cursor < 0:
            return tuple(found)
        found.append(cursor)
        cursor += 1


def listing_bytes() -> tuple[int, bytes]:
    lines = LISTING_PATH.read_text(encoding="ascii").splitlines()
    decoded = bytearray()
    pc = LISTING_START_PC
    rows = 0
    for line_number, line in enumerate(lines, start=1):
        if not line or line.startswith(";"):
            continue
        match = LISTING_ROW.fullmatch(line)
        if match is None:
            raise AssertionError(f"malformed MAME listing row at line {line_number}")
        address = int(match.group(1), 16)
        byte_tokens = match.group(2).split()
        if not byte_tokens or any(not re.fullmatch(r"[0-9a-f]{2}", token) for token in byte_tokens):
            raise AssertionError(f"invalid MAME byte column at line {line_number}")
        if address != pc:
            raise AssertionError(
                f"listing address gap at line {line_number}: "
                f"expected {pc:#06x}, got {address:#06x}"
            )
        decoded.extend(bytes.fromhex("".join(byte_tokens)))
        pc += len(byte_tokens)
        rows += 1
    if not rows or len(decoded) != WINDOW_LENGTH:
        raise AssertionError(f"expected a {WINDOW_LENGTH}-byte listing, found {len(decoded)} bytes")
    return pc, bytes(decoded)


def main() -> int:
    media_root = Path(
        os.environ.get("FIRESTAFF_THERON_TEST_DATA_DIR", Path.home() / ".firestaff/data/theron")
    )
    media_paths = {
        region: media_root / metadata["filename"]
        for region, metadata in REGIONS.items()
    }
    if any(not path.is_file() for path in media_paths.values()):
        print("SKIP: authentic JP and US Theron Track 02 media is unavailable")
        return 77

    end_pc, decoded = listing_bytes()
    if end_pc != LISTING_START_PC + WINDOW_LENGTH:
        raise AssertionError("MAME listing does not end at the expected 127-byte boundary")

    spans = {}
    for region, metadata in REGIONS.items():
        media_path = media_paths[region]
        actual_size = media_path.stat().st_size
        if actual_size != metadata["size"]:
            raise AssertionError(
                f"{metadata['filename']}: expected {metadata['size']} bytes, "
                f"got {actual_size}"
            )
        with media_path.open("rb") as media_file:
            image = media_file.read(metadata["size"] + 1)
        if len(image) != metadata["size"]:
            raise AssertionError(
                f"{metadata['filename']}: expected bounded read of "
                f"{metadata['size']} bytes, got {len(image)}"
            )
        actual_md5 = hashlib.md5(image).hexdigest()
        actual_sha256 = hashlib.sha256(image).hexdigest()
        if actual_md5 != metadata["md5"] or actual_sha256 != metadata["sha256"]:
            raise AssertionError(
                f"{metadata['filename']}: expected authentic MD5/SHA-256 "
                f"{metadata['md5']}/{metadata['sha256']}, got {actual_md5}/{actual_sha256}"
            )
        offset = metadata["offset"]
        span = image[offset : offset + WINDOW_LENGTH]
        if len(span) != WINDOW_LENGTH:
            raise AssertionError(f"{region} candidate is truncated at {offset:#x}")
        if span[: len(EXPECTED_WINDOW)] != EXPECTED_WINDOW:
            raise AssertionError(f"{region} candidate no longer begins with sampled PC windows")
        if hashlib.sha256(span).hexdigest() != metadata["span_sha256"]:
            raise AssertionError(f"{region} candidate span hash changed at {offset:#x}")
        if all_offsets(image, span) != (offset,):
            raise AssertionError(f"{region} 127-byte candidate is not unique at {offset:#x}")
        spans[region] = span
        print(f"PASS: authentic {region} Track 02 candidate at raw offset {offset:#x}")

    if spans["JP"] != spans["US"]:
        raise AssertionError("authentic JP and US candidate bytes differ")
    if decoded != spans["JP"]:
        raise AssertionError("MAME listing bytes differ from the authentic candidate span")

    print("PASS: unique 127-byte JP/US candidate and MAME HuC6280 listing match exactly")
    print(
        "LIMIT: static media/listing correspondence only; runtime source binding "
        "and semantics remain unproven"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

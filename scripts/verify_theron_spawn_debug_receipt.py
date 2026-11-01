#!/usr/bin/env python3
"""Verify native Theron spawn-source debug receipts against authentic Track 02."""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from pathlib import Path


TRACK02_MEDIA = {
    "b7afb338ad31be1025b53f9aff12d73a": ("jp", "mode1-2352", 0x273818),
    "f23601102138f87c33025877767ebf76": ("us", "mode1-2352", 0x274018),
    "62a39bbf43415c9739c41c2481080a49": ("jp", "mode1-2048", 0x203818),
    "ceb02343868f80cec899e9b239aff2da": ("us", "mode1-2048", 0x203818),
}
POINTER_COUNT = 8
ZONE_OFFSETS_FROM_BASE = (0x58, 0xD7, 0x102, 0x129, 0x150)
HEADER_RE = re.compile(
    r"^firestaff: theron-source kind=spawn-tables status=authenticated "
    r"region=(jp|us) track02_md5=([0-9a-f]{32}) layout=(mode1-2352|mode1-2048) "
    r"pointer_ud=0x([0-9a-f]{6}) pointer_count=8 zone_count=5 "
    r"semantics=opaque$"
)
POINTER_RE = re.compile(
    r"^firestaff: theron-source kind=spawn-pointer index=(\d+) "
    r"source_ud=0x([0-9a-f]{6}) raw_words="
    r"([0-9a-f]{4}),([0-9a-f]{4}),([0-9a-f]{4}),([0-9a-f]{4}) "
    r"static_zone_link=(-?\d+)$"
)
ZONE_RE = re.compile(
    r"^firestaff: theron-source kind=spawn-zone index=(\d+) "
    r"source_ud=0x([0-9a-f]{6}) raw_words=([0-9a-f]{4}),([0-9a-f]{4}) "
    r"raw_bytes=([0-9a-f]{2}),([0-9a-f]{2}),([0-9a-f]{2}),([0-9a-f]{2})$"
)


class VerificationError(Exception):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def project_mode1_2352(raw: bytes) -> bytes:
    require(len(raw) > 0 and len(raw) % 2352 == 0,
            "Track 02 is not a whole number of 2352-byte sectors")
    return b"".join(raw[offset + 16 : offset + 2064]
                    for offset in range(0, len(raw), 2352))


def le16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little")


def check_log(path: Path, projected: bytes, md5: str, region: str,
              layout: str, expected_pointer_base: int) -> None:
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except (OSError, UnicodeError) as exc:
        raise VerificationError(f"{path}: cannot read UTF-8 log: {exc}") from exc

    headers = [match for line in lines if (match := HEADER_RE.fullmatch(line))]
    require(len(headers) == 1, f"{path}: expected exactly one authenticated table header")
    header = headers[0]
    actual_region, actual_md5, actual_layout, pointer_text = header.groups()
    pointer_base = int(pointer_text, 16)
    require(actual_region == region, f"{path}: region is {actual_region}, expected {region}")
    require(actual_md5 == md5, f"{path}: receipt media MD5 does not match input BIN")
    require(actual_layout == layout, f"{path}: layout is {actual_layout}, expected {layout}")
    require(pointer_base == expected_pointer_base,
            f"{path}: pointer-table offset is 0x{pointer_base:x}, "
            f"expected 0x{expected_pointer_base:x}")

    runtime_rows = [line for line in lines
                    if line.startswith("firestaff: startup-frame game=theron ")
                    and "phase=theron-runtime" in line]
    require(any("level-loaded=1" in line for line in runtime_rows),
            f"{path}: no loaded Theron runtime summary")

    pointer_rows: dict[int, re.Match[str]] = {}
    zone_rows: dict[int, re.Match[str]] = {}
    for line in lines:
        if line.startswith("firestaff: theron-source kind=spawn-pointer "):
            match = POINTER_RE.fullmatch(line)
            require(match is not None, f"{path}: malformed pointer receipt row")
            assert match is not None
            index = int(match.group(1))
            require(index not in pointer_rows, f"{path}: duplicate pointer index {index}")
            pointer_rows[index] = match
        elif line.startswith("firestaff: theron-source kind=spawn-zone "):
            match = ZONE_RE.fullmatch(line)
            require(match is not None, f"{path}: malformed zone receipt row")
            assert match is not None
            index = int(match.group(1))
            require(index not in zone_rows, f"{path}: duplicate zone index {index}")
            zone_rows[index] = match

    require(set(pointer_rows) == set(range(POINTER_COUNT)),
            f"{path}: expected pointer indices 0..7")
    require(set(zone_rows) == set(range(len(ZONE_OFFSETS_FROM_BASE))),
            f"{path}: expected zone indices 0..4")

    table_base = pointer_base - 0x18
    source_zone_offsets = tuple(table_base + offset for offset in ZONE_OFFSETS_FROM_BASE)
    for index, match in pointer_rows.items():
        source_offset = int(match.group(2), 16)
        require(source_offset == pointer_base + index * 8,
                f"{path}: pointer {index} source offset is wrong")
        expected_words = tuple(le16(projected, source_offset + i)
                               for i in (0, 2, 4, 6))
        actual_words = tuple(int(match.group(i), 16) for i in range(3, 7))
        require(actual_words == expected_words,
                f"{path}: pointer {index} fields differ from authenticated source")
        expected_zone = -1
        spawn_offset = expected_words[2]
        for zone_index, zone_offset in enumerate(source_zone_offsets):
            if table_base + spawn_offset == zone_offset:
                expected_zone = zone_index
                break
        require(int(match.group(7)) == expected_zone,
                f"{path}: pointer {index} static zone link differs from source offsets")

    for index, match in zone_rows.items():
        source_offset = int(match.group(2), 16)
        require(source_offset == source_zone_offsets[index],
                f"{path}: zone {index} source offset is wrong")
        expected_words = (le16(projected, source_offset),
                          le16(projected, source_offset + 2))
        actual_words = (int(match.group(3), 16), int(match.group(4), 16))
        require(actual_words == expected_words,
                f"{path}: zone {index} words differ from authenticated source")
        expected_bytes = tuple(projected[source_offset + i] for i in range(4, 8))
        actual_bytes = tuple(int(match.group(i), 16) for i in range(5, 9))
        require(actual_bytes == expected_bytes,
                f"{path}: zone {index} bytes differ from authenticated source")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("track02_media", type=Path,
                        help="authentic JP/US raw BIN or split MODE1/2048 Track 02 ISO")
    parser.add_argument("receipt_logs", nargs="+", type=Path,
                        help="native --debug log(s) containing one receipt each")
    args = parser.parse_args()
    try:
        raw = args.track02_media.read_bytes()
    except OSError as exc:
        print(f"FAIL: cannot read Track 02 media: {exc}", file=sys.stderr)
        return 2

    md5 = hashlib.md5(raw).hexdigest()
    identity = TRACK02_MEDIA.get(md5)
    if identity is None:
        print(f"FAIL: unauthenticated Track 02 media MD5 {md5}", file=sys.stderr)
        return 2
    region, layout, pointer_base = identity
    try:
        projected = project_mode1_2352(raw) if layout == "mode1-2352" else raw
        for log_path in args.receipt_logs:
            check_log(log_path, projected, md5, region, layout, pointer_base)
    except (OSError, VerificationError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 2

    print(f"PASS: region={region} track02_md5={md5} "
          f"verified_receipt_logs={len(args.receipt_logs)} "
          f"layout={layout} pointer_rows=8 zone_rows=5 source=authenticated-Track-02")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

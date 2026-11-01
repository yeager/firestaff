#!/usr/bin/env python3
"""List authentic Track 02 candidates for the observed two-byte TII transfer.

This is a static byte census, not proof of runtime bank selection, a caller,
the active map, or gameplay semantics. It prints offsets and hashes only; it
does not publish game-code bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


PATTERN = bytes.fromhex("73 b4 20 40 20 02 00")
SECTOR_BYTES = 2352
REPEAT_SECTOR_STRIDE = 128
REPEAT_BYTE_STRIDE = SECTOR_BYTES * REPEAT_SECTOR_STRIDE
REPEATED_BLOCKS = 7
CONTEXT_BYTES = 16


@dataclass(frozen=True)
class Edition:
    name: str
    size: int
    md5: str
    site_bases: tuple[int, int, int]


EDITIONS = {
    "US": Edition(
        "US",
        8_104_992,
        "f23601102138f87c33025877767ebf76",
        (0x0A334C, 0x0A33FF, 0x0A34B0),
    ),
    "JP": Edition(
        "JP",
        8_102_640,
        "b7afb338ad31be1025b53f9aff12d73a",
        (0x0A2A1C, 0x0A2ACF, 0x0A2B80),
    ),
}


def candidate_offsets(data: bytes) -> list[int]:
    result: list[int] = []
    cursor = 0
    while (offset := data.find(PATTERN, cursor)) >= 0:
        result.append(offset)
        cursor = offset + 1
    return result


def expected_offsets(edition: Edition) -> list[int]:
    return sorted(
        base + block * REPEAT_BYTE_STRIDE
        for block in range(REPEATED_BLOCKS)
        for base in edition.site_bases
    )


def analyze(path: Path, edition: Edition) -> tuple[list[int], Counter[str]]:
    data = path.read_bytes()
    if len(data) != edition.size:
        raise ValueError(
            f"{edition.name}: expected {edition.size} bytes, received {len(data)}"
        )
    digest = hashlib.md5(data).hexdigest()
    if digest != edition.md5:
        raise ValueError(
            f"{edition.name}: unauthenticated Track 02 MD5 {digest}"
        )

    offsets = candidate_offsets(data)
    expected = expected_offsets(edition)
    if offsets != expected:
        actual_text = ",".join(f"0x{value:x}" for value in offsets)
        expected_text = ",".join(f"0x{value:x}" for value in expected)
        raise ValueError(
            f"{edition.name}: exact TII candidate census changed; "
            f"expected [{expected_text}], received [{actual_text}]"
        )

    context_hashes: Counter[str] = Counter()
    for offset in offsets:
        sector_offset = offset % SECTOR_BYTES
        if sector_offset < 16 or sector_offset + len(PATTERN) > 2352 - 4:
            raise ValueError(
                f"{edition.name}: candidate crosses a raw-sector boundary "
                f"at 0x{offset:x}"
            )
        start = offset - CONTEXT_BYTES
        end = offset + len(PATTERN) + CONTEXT_BYTES
        context_hashes[hashlib.sha256(data[start:end]).hexdigest()] += 1
    return offsets, context_hashes


def print_report(edition: Edition, offsets: list[int], contexts: Counter[str]) -> None:
    print(
        f"{edition.name}: authenticated Track 02 size={edition.size} "
        f"md5={edition.md5}"
    )
    print(
        f"  exact TII candidates={len(offsets)}; "
        f"repeated 128-sector groups={REPEATED_BLOCKS}; "
        f"context signatures={len(contexts)}"
    )
    for offset in offsets:
        sector, sector_byte = divmod(offset, SECTOR_BYTES)
        print(
            f"  raw_offset=0x{offset:06x} raw_sector={sector} "
            f"sector_byte=0x{sector_byte:03x}"
        )
    print("  context_sha256_counts=" + ",".join(
        f"{digest}:{count}" for digest, count in sorted(contexts.items())
    ))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("us_track02", type=Path)
    parser.add_argument("jp_track02", type=Path)
    args = parser.parse_args()

    missing = [path for path in (args.us_track02, args.jp_track02)
               if not path.is_file()]
    if missing:
        print("SKIP: authentic US and JP Track 02 files are required", file=sys.stderr)
        return 77

    try:
        for region, path in (("US", args.us_track02), ("JP", args.jp_track02)):
            edition = EDITIONS[region]
            offsets, contexts = analyze(path, edition)
            print_report(edition, offsets, contexts)
    except (OSError, ValueError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print(
        "boundary=static candidate offsets only; no runtime PC, bank/MPR, "
        "caller, active-map, or gameplay binding"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

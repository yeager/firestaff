#!/usr/bin/env python3
"""Verify static slot-detail callback anchors in authentic Japanese Nexus DM.BIN.

This checks SH-2 instruction and data bytes only. It does not prove that the
callback runs on the retail Load panel, that its text is displayed, or that it
reads Saturn BKR comment bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

EXPECTED_SHA256 = "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
EXPECTED_SIZE = 555_144
SH2_BASE = 0x06010000

INSTRUCTIONS = {
    0x060259A6: 0xE06C,  # callback object field +0x6c
    0x060259AA: 0x0E26,  # install companion callback
}
DATA = {
    0x06025A68: bytes.fromhex("06027132"),  # callback pointer
    0x060271DC: bytes.fromhex("0607952c"),  # selected slot table pointer
    0x060271E4: bytes.fromhex("060232b4"),  # slot-detail helper pointer
    0x060272F4: bytes.fromhex("060472000604265c0604720806039ab8"),
    0x060471EC: b" save comment %s\n\x00\xff\xff EASY\x00\xff\xff NORMAL\x00",
}
CALLBACK_SPAN = (0x06027132, 0x156,
                 "560936b4c01c639bef6f5f08973b1cfb0c782ba1e791792cad61581fc24d9712")


def read_at(image: bytes, address: int, size: int) -> bytes:
    offset = address - SH2_BASE
    if offset < 0 or offset + size > len(image):
        raise ValueError(f"address {address:#010x} is outside DM.BIN")
    return image[offset:offset + size]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dm-bin", type=Path, default=Path(".firestaff/data/nexus/DM.BIN"),
        help="authentic Japanese Nexus DM.BIN (default: installed Nexus data)",
    )
    args = parser.parse_args()
    try:
        image = args.dm_bin.read_bytes()
        digest = hashlib.sha256(image).hexdigest()
        if len(image) != EXPECTED_SIZE or digest != EXPECTED_SHA256:
            raise ValueError(
                f"DM.BIN identity mismatch: size={len(image)} sha256={digest}"
            )
        for address, expected in INSTRUCTIONS.items():
            actual = struct.unpack(">H", read_at(image, address, 2))[0]
            if actual != expected:
                raise ValueError(
                    f"SH-2 word mismatch at {address:#010x}: "
                    f"expected {expected:04x}, got {actual:04x}"
                )
        for address, expected in DATA.items():
            actual = read_at(image, address, len(expected))
            if actual != expected:
                raise ValueError(
                    f"data mismatch at {address:#010x}: "
                    f"expected {expected.hex()}, got {actual.hex()}"
                )
        address, size, expected_hash = CALLBACK_SPAN
        actual_hash = hashlib.sha256(read_at(image, address, size)).hexdigest()
        if actual_hash != expected_hash:
            raise ValueError(
                f"callback hash mismatch at {address:#010x}: "
                f"expected {expected_hash}, got {actual_hash}"
            )
    except (OSError, ValueError) as exc:
        print(f"BLOCKED: {exc}", file=sys.stderr)
        return 1

    print(f"dm_bin_sha256={digest}")
    print(f"instruction_anchors={len(INSTRUCTIONS)} verified")
    print(f"data_spans={len(DATA)} verified")
    print("slot_detail_callback=0x06027132 (0x156-byte hash verified)")
    print("runtime_load_panel_or_bkr_comment_binding=not_proven")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

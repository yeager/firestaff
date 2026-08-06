#!/usr/bin/env python3
"""Validate the raw VDP1/VDP2 witness emitted by the Saturn capture patch.

This validates transport/layout only.  It intentionally does not admit PRS3,
SLEV/SAL, HUD, viewport, tilemap, CLUT, or mesh semantics.
"""

from __future__ import annotations

import argparse
from pathlib import Path


RUNTIME_MAGIC = b"FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n"
VDP1_MAGIC = b"FIRESTAFF_NEXUS_SATURN_VDP1_RAW_V1\n"
VDP2_MAGIC = b"VDP2_RAW\n"
VDP1_PAYLOAD_BYTES = 0x40000 * 2 + 0x20000 * 2 + 0x20000 * 2 + 1
VDP2_PAYLOAD_BYTES = 0x100 * 2 + 0x40000 * 2 + 0x800 * 2


def validate(blob: bytes, required_frames: int) -> tuple[int, list[int]]:
    if not blob.startswith(RUNTIME_MAGIC):
        raise ValueError("missing runtime capture magic")

    offset = len(RUNTIME_MAGIC)
    frames: list[int] = []
    while offset < len(blob):
        marker = f"frame={len(frames)}\n".encode("ascii")
        if not blob.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        frames.append(offset)
        offset += len(marker)
        if not blob.startswith(VDP1_MAGIC, offset):
            raise ValueError(f"missing VDP1 marker for frame {len(frames) - 1}")
        offset += len(VDP1_MAGIC) + VDP1_PAYLOAD_BYTES
        if not blob.startswith(VDP2_MAGIC, offset):
            raise ValueError(f"missing VDP2 marker for frame {len(frames) - 1}")
        offset += len(VDP2_MAGIC) + VDP2_PAYLOAD_BYTES

    if offset != len(blob):
        raise ValueError("trailing bytes after final frame")
    if len(frames) != required_frames:
        raise ValueError(f"expected {required_frames} frames, found {len(frames)}")
    return len(frames), frames


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--require-frames", type=int, default=2)
    args = parser.parse_args()
    try:
        blob = args.capture.read_bytes()
        count, offsets = validate(blob, args.require_frames)
    except (OSError, ValueError) as error:
        print(f"NEXUS_SATURN_RUNTIME_CAPTURE_INVALID: {error}")
        return 1
    print(
        "NEXUS_SATURN_RUNTIME_CAPTURE_RAW_V1: "
        f"frames={count} bytes={len(blob)} offsets={','.join(map(str, offsets))}"
    )
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

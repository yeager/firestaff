#!/usr/bin/env python3
"""Report region hashes and real frame changes in a Saturn raw witness.

This is deliberately a transport/observation tool.  A changed framebuffer or
register region proves that the instrumented Saturn producer observed changing
runtime state; it does not identify a menu, HUD, viewport, CLUT, or consumer.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from validate_nexus_saturn_runtime_capture import (
    RUNTIME_MAGIC,
    VDP1_MAGIC,
    VDP1_PAYLOAD_BYTES,
    VDP2_MAGIC,
    VDP2_PAYLOAD_BYTES,
    validate,
)


VDP1_REGIONS = (
    ("vdp1-vram", 0x40000 * 2),
    ("vdp1-fb0", 0x20000 * 2),
    ("vdp1-fb1", 0x20000 * 2),
    ("vdp1-fb-draw-which", 1),
)
VDP2_REGIONS = (
    ("vdp2-regs", 0x100 * 2),
    ("vdp2-vram", 0x40000 * 2),
    ("vdp2-cram", 0x800 * 2),
)


def frame_regions(blob: bytes, required_frames: int) -> list[dict[str, bytes]]:
    """Extract named raw regions after the shared validator has checked layout."""
    validate(blob, required_frames)
    offset = len(RUNTIME_MAGIC)
    frames: list[dict[str, bytes]] = []
    for frame_index in range(required_frames):
        marker = f"frame={frame_index}\n".encode("ascii")
        if not blob.startswith(marker, offset):
            raise ValueError(f"invalid frame marker at offset {offset}")
        offset += len(marker)
        if not blob.startswith(VDP1_MAGIC, offset):
            raise ValueError(f"missing VDP1 marker for frame {frame_index}")
        offset += len(VDP1_MAGIC)
        vdp1 = blob[offset : offset + VDP1_PAYLOAD_BYTES]
        offset += VDP1_PAYLOAD_BYTES
        if not blob.startswith(VDP2_MAGIC, offset):
            raise ValueError(f"missing VDP2 marker for frame {frame_index}")
        offset += len(VDP2_MAGIC)
        vdp2 = blob[offset : offset + VDP2_PAYLOAD_BYTES]
        offset += VDP2_PAYLOAD_BYTES

        regions: dict[str, bytes] = {}
        cursor = 0
        for name, size in VDP1_REGIONS:
            regions[name] = vdp1[cursor : cursor + size]
            cursor += size
        cursor = 0
        for name, size in VDP2_REGIONS:
            regions[name] = vdp2[cursor : cursor + size]
            cursor += size
        frames.append(regions)
    return frames


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--require-frames", type=int, default=2)
    parser.add_argument(
        "--require-changing-region",
        action="append",
        choices=[name for name, _ in VDP1_REGIONS + VDP2_REGIONS],
        default=[],
        help="require this region to differ in at least one adjacent frame pair",
    )
    args = parser.parse_args()
    try:
        frames = frame_regions(args.capture.read_bytes(), args.require_frames)
    except (OSError, ValueError) as error:
        print(f"NEXUS_SATURN_RUNTIME_CAPTURE_ANALYSIS_INVALID: {error}")
        return 1

    changed: set[str] = set()
    for index, frame in enumerate(frames):
        print(
            f"frame={index} "
            + " ".join(f"{name}_sha256={digest(frame[name])}" for name, _ in VDP1_REGIONS + VDP2_REGIONS)
        )
        if index:
            for name in frame:
                if frame[name] != frames[index - 1][name]:
                    changed.add(name)

    missing = sorted(set(args.require_changing_region) - changed)
    print("changed_regions=" + ",".join(sorted(changed)))
    print("semantic_admission=blocked")
    if missing:
        print("required_unchanged_regions=" + ",".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

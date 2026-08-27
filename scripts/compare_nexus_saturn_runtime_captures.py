#!/usr/bin/env python3
"""Compare two validated Saturn raw witnesses frame by frame.

This is an observation tool: identical regions establish a controlled runtime
comparison, while differing regions identify an observed state difference.
Neither result assigns input, asset, or display-consumer semantics.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file


REGIONS = (
    "vdp1-state",
    "vdp1-vram",
    "vdp1-fb0",
    "vdp1-fb1",
    "vdp1-fb-draw-which",
    "vdp2-regs",
    "vdp2-vram",
    "vdp2-cram",
)


def byte_offset(left: bytes, right: bytes) -> int:
    """Return the first unequal byte, requiring equal-sized byte regions."""
    if len(left) != len(right):
        return -1
    for offset, (a_byte, b_byte) in enumerate(zip(left, right)):
        if a_byte != b_byte:
            return offset
    return -1


def as_bytes(region: object) -> bytes:
    return region.encode("ascii") if isinstance(region, str) else region


def digest(region: object) -> str:
    return hashlib.sha256(as_bytes(region)).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("control", type=Path,
                        help="authenticated no-input raw witness")
    parser.add_argument("candidate", type=Path,
                        help="same-frame-range candidate raw witness")
    parser.add_argument("--frames", required=True, type=int)
    parser.add_argument("--require-identical", action="store_true",
                        help="fail when any observed region differs")
    args = parser.parse_args()
    if args.frames <= 0:
        raise SystemExit("NEXUS_RUNTIME_COMPARE_INVALID: --frames must be positive")
    changed: dict[str, tuple[int, int, str, str]] = {}
    try:
        control = iter_frame_regions_file(args.control, args.frames)
        candidate = iter_frame_regions_file(args.candidate, args.frames)
        for expected in range(args.frames):
            control_index, control_regions = next(control)
            candidate_index, candidate_regions = next(candidate)
            if control_index != expected or candidate_index != expected:
                raise ValueError("frame indexes are not contiguous")
            for name in REGIONS:
                left = as_bytes(control_regions[name])
                right = as_bytes(candidate_regions[name])
                if left == right or name in changed:
                    continue
                changed[name] = (expected, byte_offset(left, right),
                                 digest(left), digest(right))
    except (OSError, StopIteration, UnicodeError, ValueError) as error:
        print(f"NEXUS_RUNTIME_COMPARE_INVALID: {error}")
        return 1

    print(f"frames_compared={args.frames}")
    print(f"identical_regions={len(REGIONS) - len(changed)}")
    print(f"changed_regions={len(changed)}")
    for name in REGIONS:
        if name not in changed:
            print(f"{name}=identical")
            continue
        frame, offset, control_hash, candidate_hash = changed[name]
        print(f"{name}=changed frame={frame} first_byte={offset} "
              f"control_sha256={control_hash} candidate_sha256={candidate_hash}")
    print("semantic_admission=blocked")
    if args.require_identical and changed:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

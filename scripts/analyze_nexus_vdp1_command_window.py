#!/usr/bin/env python3
"""Inspect the bounded VDP1 command window in an authenticated raw witness.

The command words are read from the VDP1 VRAM snapshot emitted by the current
Mednafen producer. This identifies only the observed command records and their
hardware framing. It does not infer a MENU.BPK/DGN owner, CLUT, placement, or
production draw permission.
"""

from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions


STATE_RE = re.compile(r"copr:([0-9a-fA-F]+)")
COMMAND_BYTES = 32


def command_window(vram: bytes, state: str) -> list[tuple[int, tuple[int, ...]]]:
    match = STATE_RE.search(state)
    if not match:
        raise ValueError("VDP1 state has no COPR")
    copr = int(match.group(1), 16)
    # Mednafen's CurCommandAddr indexes uint16 VDP1 words and COPR is
    # CurCommandAddr >> 2, so the observed COPR maps to a byte offset * 8.
    end_offset = copr * 8
    if end_offset > len(vram) or end_offset % COMMAND_BYTES:
        raise ValueError(f"COPR does not identify a bounded command boundary: {copr}")
    first = max(0, end_offset - (COMMAND_BYTES * 4))
    return [
        (offset, struct.unpack_from("<16H", vram, offset))
        for offset in range(first, end_offset + COMMAND_BYTES, COMMAND_BYTES)
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=2)
    parser.add_argument("--require-end", action="store_true")
    args = parser.parse_args()
    try:
        frames, states = frame_regions(args.capture.read_bytes(), args.capture_frames)
        if args.frame >= len(frames):
            raise ValueError("frame index outside capture")
        records = command_window(frames[args.frame]["vdp1-vram"], states[args.frame])
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_COMMAND_WINDOW_INVALID: {error}")
        return 1

    end_seen = False
    for offset, words in records:
        control = words[0]
        command_type = control & 0x000F
        end = bool(control & 0x8000)
        end_seen |= end
        print(
            f"offset=0x{offset:05x} control=0x{control:04x} "
            f"type=0x{command_type:x} end={int(end)} "
            f"link=0x{words[1]:04x} pmod=0x{words[2]:04x} "
            f"colr=0x{words[3]:04x} srca=0x{words[4]:04x} "
            f"size=0x{words[5]:04x}"
        )
    print("semantic_admission=blocked")
    if args.require_end and not end_seen:
        print("required_end_record=missing")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

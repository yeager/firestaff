#!/usr/bin/env python3
"""Verify one captured VDP1 command chain consumes one captured texture span.

The command records are hardware framing only.  This tool does not assign a
MENU.BPK/DGN owner, palette meaning, face, camera transform, or production
renderer permission.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--capture-frames", type=int, required=True)
    parser.add_argument("--command-offset", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--command-count", type=int, required=True)
    parser.add_argument("--require-source", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--require-colour-mode", type=int, default=5)
    args = parser.parse_args()
    try:
        frames, states = frame_regions(args.capture.read_bytes(), args.capture_frames)
        if args.frame >= len(frames):
            raise ValueError("frame index outside capture")
        records = command_window(
            frames[args.frame]["vdp1-vram"], states[args.frame],
            args.command_offset, args.command_count)
        draws = []
        end_seen = False
        for offset, words in records:
            control = words[0]
            if control & 0x8000:
                end_seen = True
                break
            command_type = control & 0x000f
            if command_type > 2:
                continue
            colour_mode = (words[2] >> 3) & 0x7
            width = (words[5] & 0x003f) * 8
            height = (words[5] >> 8) & 0x00ff
            source = words[4] * 8
            bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
            source_size = width * height * bits_per_pixel // 8
            if colour_mode != args.require_colour_mode or source != args.require_source:
                raise ValueError(
                    f"command 0x{offset:05x} does not consume required source/mode")
            draws.append((offset, width, height, source, source_size))
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP1_COMMAND_SOURCE_JOIN_INVALID: {error}")
        return 1
    if not end_seen or not draws:
        print("NEXUS_VDP1_COMMAND_SOURCE_JOIN_INVALID: incomplete draw chain")
        return 1
    print(f"frame={args.frame} command_start=0x{args.command_offset:05x} "
          f"draws={len(draws)} source=0x{args.require_source:05x} "
          f"colour_mode={args.require_colour_mode} end=1")
    print("command_to_texture_span=verified")
    print("palette_command_owner=blocked")
    print("production_renderer_permission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

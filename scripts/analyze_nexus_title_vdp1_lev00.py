#!/usr/bin/env python3
"""Bind an authenticated Nexus title VDP1 chain to the real LEV00 bytes.

This is a narrow byte-provenance receipt.  It proves observed VDP1 texture
windows only; it does not infer Structure2 material semantics, CLUT ownership,
geometry, priority, timing, or native rendering permission.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory, wordswapped
from analyze_nexus_vdp1_command_sequence import find_chain, parse_copr


def texture_spans(vram: bytes, state: str) -> list[tuple[int, int, bytes]]:
    spans = []
    for record in find_chain(vram, parse_copr(state)):
        if record.end or record.command_type > 2:
            continue
        words = record.words
        colour_mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = words[5] >> 8
        bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = width * height * bits_per_pixel // 8
        if not source_size:
            continue
        if source_offset + source_size > len(vram):
            raise ValueError("VDP1 texture span lies outside VRAM")
        spans.append((record.offset, source_offset,
                      vram[source_offset:source_offset + source_size]))
    return spans


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--cue", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    args = parser.parse_args()
    try:
        if args.frame < 0:
            raise ValueError("negative frame")
        frame = None
        for index, candidate in iter_frame_regions_file(args.capture, args.frame + 1):
            if index == args.frame:
                frame = candidate
                break
        if frame is None:
            raise ValueError("selected frame is absent")
        lev00 = iso_members_in_memory(cue_track1(args.cue), {"LEV00.DGN"})["LEV00.DGN"]
        swapped = wordswapped(lev00)
        spans = texture_spans(frame["vdp1-vram"], frame["vdp1-state"])
    except (OSError, ValueError) as error:
        print(f"NEXUS_TITLE_VDP1_LEV00_INVALID: {error}")
        return 1

    unmatched = []
    for command, source_offset, source in spans:
        offset = swapped.find(source)
        if offset < 0:
            unmatched.append(command)
            continue
        print(
            f"command=0x{command:05x} vdp1_source=0x{source_offset:05x} "
            f"bytes={len(source)} lev00_word_swapped_offset=0x{offset:x} "
            f"sha256={hashlib.sha256(source).hexdigest()}"
        )
    print(f"texture_spans={len(spans)}")
    print(f"lev00_sha256={hashlib.sha256(lev00).hexdigest()}")
    if unmatched:
        print("unmatched_commands=" + ",".join(f"0x{value:05x}" for value in unmatched))
        return 1
    print("all_texture_spans_word_swapped_lev00=verified")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

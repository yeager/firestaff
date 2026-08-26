#!/usr/bin/env python3
"""Join title-frame VDP1 source/CLUT pairs to real LEV00 Structure2 records."""

from __future__ import annotations

import argparse
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory, wordswapped
from analyze_nexus_vdp1_command_sequence import find_chain, parse_copr
from analyze_nexus_vdp1_source_join import be16, be32


def materials(data: bytes) -> dict[tuple[bytes, bytes], tuple[int, int, int]]:
    base = be16(data, 0x14) * 0x800
    useful = be32(data, 0x18)
    if base > len(data) or useful > len(data) - base:
        raise ValueError("LEV00 Structure2 envelope is invalid")
    result = {}
    for cursor in range(0, useful, 20):
        descriptor = data[base + cursor:base + cursor + 20]
        if len(descriptor) < 20:
            raise ValueError("LEV00 Structure2 descriptor is truncated")
        image_id = be16(descriptor, 0)
        if image_id == 0xFFFF:
            return result
        encoding, width, height = be16(descriptor, 2), be16(descriptor, 6), be16(descriptor, 8)
        image_offset, palette_offset = be32(descriptor, 12), be32(descriptor, 16)
        if encoding == 0x0008 and width and height:
            image_size = (width * height + 1) // 2
            if image_offset + image_size > useful or palette_offset + 32 > useful:
                raise ValueError("LEV00 Structure2 material lies outside its envelope")
            result[(wordswapped(data[base + image_offset:base + image_offset + image_size]),
                    wordswapped(data[base + palette_offset:base + palette_offset + 32]))] = (image_id, width, height)
    raise ValueError("LEV00 Structure2 terminator is missing")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--cue", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--require-complete", action="store_true")
    args = parser.parse_args()
    try:
        frame = next(candidate for index, candidate in
                     iter_frame_regions_file(args.capture, args.frame + 1)
                     if index == args.frame)
        lev00 = iso_members_in_memory(cue_track1(args.cue), {"LEV00.DGN"})["LEV00.DGN"]
        index = materials(lev00)
        chain = find_chain(frame["vdp1-vram"], parse_copr(frame["vdp1-state"]))
    except (OSError, StopIteration, ValueError) as error:
        print(f"NEXUS_TITLE_VDP1_LEV00_MATERIAL_INVALID: {error}")
        return 1

    vram = frame["vdp1-vram"]
    textured = matched = 0
    unmatched = []
    for record in chain:
        if record.end or record.command_type > 2:
            continue
        words = record.words
        mode = (words[2] >> 3) & 7
        width, height = (words[5] & 0x3F) * 8, words[5] >> 8
        bits = 4 if mode <= 1 else 8 if mode <= 4 else 16
        size, source = width * height * bits // 8, words[4] * 8
        if not size:
            continue
        textured += 1
        clut = (words[3] & ~3) << 2
        if source + size > len(vram) or clut * 2 + 32 > len(vram):
            unmatched.append(record.offset)
            continue
        material = index.get((vram[source:source + size], vram[clut * 2:clut * 2 + 32]))
        if material is None:
            unmatched.append(record.offset)
            continue
        matched += 1
        print(f"command=0x{record.offset:05x} structure2={material[0]} size={material[1]}x{material[2]} clut_word=0x{clut:05x}")
    print(f"textured_spans={textured} source_clut_matches={matched}")
    print("unmatched_commands=" + (",".join(f"0x{x:05x}" for x in unmatched) or "none"))
    print("source_clut_join=verified" if matched == textured and textured else "source_clut_join=partial")
    print("semantic_admission=blocked")
    return 0 if not args.require_complete or matched == textured else 1


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Verify one retail ITEM.IBS floor texture and CLUT against a VDP1 capture."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import iter_frame_regions_file
from analyze_nexus_vdp1_source_join import swapped_words

ITEM_SHA256 = "fc32ca5875906e6e0dc69e0b5edfa5d00cb1f4401b7d497397c699be7c4530c1"


def be(data: bytes, offset: int, length: int) -> int:
    return int.from_bytes(data[offset:offset + length], "big")


def verify(item: bytes, vram: bytes, command_offset: int, floor_index: int) -> None:
    if hashlib.sha256(item).hexdigest() != ITEM_SHA256:
        raise ValueError("ITEM.IBS is not the authenticated Japanese retail file")
    if len(vram) != 0x80000 or command_offset < 0 or command_offset + 32 > len(vram):
        raise ValueError("invalid VDP1 VRAM or command offset")
    align = lambda n: (n + 2047) & ~2047
    count, palettes, associations, images, floors = (be(item, i, 2) for i in (2, 4, 6, 8, 14))
    if floor_index < 0 or floor_index >= floors:
        raise ValueError("floor descriptor out of range")
    base = 2048 + align(count * 40) + align(palettes * 32 + associations * 2 + images * 128)
    desc = base + floor_index * 20
    if desc + 20 > len(item):
        raise ValueError("truncated floor descriptor")
    image_id, encoding, palette_id, width, height = (be(item, desc + i, 2) for i in (0, 2, 4, 6, 8))
    image_start = base + be(item, desc + 12, 4)
    size = width * height // 2
    if image_id != floor_index or encoding != 8 or not size or image_start + size > len(item):
        raise ValueError("invalid floor image descriptor")
    palette_start = None
    for index in range(floor_index, -1, -1):
        candidate = base + index * 20
        offset = be(item, candidate + 16, 4)
        if be(item, candidate + 4, 2) == palette_id and offset:
            palette_start = base + offset
            break
    if palette_start is None or palette_start + 32 > len(item):
        raise ValueError("floor palette missing or truncated")
    words = [int.from_bytes(vram[command_offset + i:command_offset + i + 2], "little")
             for i in range(0, 32, 2)]
    if words[0] & 0xC000 or (words[0] & 15) > 2 or ((words[2] >> 3) & 7) != 1:
        raise ValueError("command is not an executed mode-1 texture")
    if (((words[5] >> 8) & 63) * 8, words[5] & 255) != (width, height):
        raise ValueError("command dimensions do not match descriptor")
    source = words[4] * 8
    clut = (words[3] & ~3) * 8
    if source + size > len(vram) or clut + 32 > len(vram):
        raise ValueError("command source or CLUT outside VRAM")
    if vram[source:source + size] != swapped_words(item[image_start:image_start + size]):
        raise ValueError("texture bytes differ")
    if vram[clut:clut + 32] != swapped_words(item[palette_start:palette_start + 32]):
        raise ValueError("CLUT bytes differ")
    print(f"floor_descriptor={floor_index} image_offset=0x{image_start:x} size={width}x{height}")
    print(f"palette_id={palette_id} palette_offset=0x{palette_start:x} clut_offset=0x{clut:x}")
    print("image_and_clut_join=verified semantic_admission=blocked")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("item", type=Path)
    parser.add_argument("--frames", type=int, required=True)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--command-offset", type=lambda x: int(x, 0), required=True)
    parser.add_argument("--floor-index", type=int, required=True)
    args = parser.parse_args()
    try:
        if args.frames <= 0 or args.frame < 0 or args.frame >= args.frames:
            raise ValueError("invalid frame selection")
        frame = next(frame for index, frame in iter_frame_regions_file(args.capture, args.frames)
                     if index == args.frame)
        verify(args.item.read_bytes(), frame["vdp1-vram"], args.command_offset, args.floor_index)
    except (OSError, ValueError, StopIteration) as error:
        print(f"NEXUS_ITEM_FLOOR_CAPTURE_INVALID: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

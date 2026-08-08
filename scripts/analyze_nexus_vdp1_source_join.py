#!/usr/bin/env python3
"""Check a real VDP1 type-2 source span against retail texture bytes.

The command/source join is an observation boundary only.  A byte match would
still require a CLUT, placement, command-order and source-owner receipt before
the production renderer could use it; an absent match is useful negative
evidence and never authorizes a fallback texture.  The DGN candidates are
only the bounded raw image spans of hash-verified retail Structure2 records;
their pixel and palette semantics remain unproven.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 4], "big")


def mns_surfaces(data_dir: Path) -> list[tuple[str, bytes]]:
    surfaces: list[tuple[str, bytes]] = []
    for path in sorted(data_dir.glob("*.MNS")):
        data = path.read_bytes()
        if len(data) < 0x2C or data[:4] != b"DMDF" or be32(data, 4) != len(data):
            continue
        text_offset = be32(data, 0x24)
        if (text_offset + 12 > len(data) or
                data[text_offset:text_offset + 4] != b"TEXT"):
            continue
        text_size = be32(data, text_offset + 4)
        count = be32(data, text_offset + 8)
        if text_size < 0x24 or text_offset + text_size > len(data) or count > 64:
            continue
        for index in range(count):
            descriptor = text_offset + 0x24 + index * 20
            if descriptor + 20 > text_offset + text_size:
                break
            image_id = be16(data, descriptor)
            if image_id == 0xFFFF:
                break
            width = be16(data, descriptor + 6)
            height = be16(data, descriptor + 8)
            relative = be32(data, descriptor + 12)
            size = width * height * 2
            start = text_offset + relative
            if width == 0 or height == 0 or start < text_offset:
                continue
            if relative + size > text_size or start + size > len(data):
                continue
            raw = data[start:start + size]
            surfaces.append((f"{path.name}[{index}] {width}x{height}", raw))
    return surfaces


def dgn_structure2_surfaces(data_dir: Path) -> list[tuple[str, bytes]]:
    """Return bounded raw image spans from the canonical LEV00..LEV15 set."""
    surfaces: list[tuple[str, bytes]] = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        path = data_dir / name
        data = path.read_bytes()
        expected_hash = DISC_HASH.get(name)
        if expected_hash is None or hashlib.sha256(data).hexdigest() != expected_hash:
            raise ValueError(f"{name} is not the canonical retail DGN")
        if len(data) < 0x1C:
            raise ValueError(f"{name} DGN header is truncated")
        block = be16(data, 0x14)
        blocks = be16(data, 0x16)
        useful = be32(data, 0x18)
        payload_start = block * 0x800
        payload_size = blocks * 0x800
        if (block == 0 or blocks == 0 or payload_start > len(data) or
                payload_size > len(data) - payload_start or useful > payload_size):
            raise ValueError(f"{name} Structure2 envelope is invalid")
        cursor = 0
        descriptor_index = 0
        while cursor + 20 <= useful:
            descriptor = data[payload_start + cursor:payload_start + cursor + 20]
            image_id = be16(descriptor, 0)
            if image_id == 0xFFFF:
                break
            if image_id != descriptor_index:
                raise ValueError(f"{name} Structure2 image ids are not sequential")
            encoding = be16(descriptor, 2)
            width = be16(descriptor, 6)
            height = be16(descriptor, 8)
            relative = be32(descriptor, 12)
            if width == 0 or height == 0 or encoding not in (0x0008, 0x0028):
                raise ValueError(f"{name} Structure2 descriptor is invalid")
            image_bytes = ((width * height + 1) // 2
                           if encoding == 0x0008 else width * height * 2)
            if relative > useful or image_bytes > useful - relative:
                raise ValueError(f"{name} Structure2 image span is invalid")
            start = payload_start + relative
            surfaces.append((
                f"{name}[Structure2={image_id} encoding=0x{encoding:04x} "
                f"{width}x{height}]", data[start:start + image_bytes]))
            descriptor_index += 1
            cursor += 20
        else:
            raise ValueError(f"{name} Structure2 terminator is missing")
    return surfaces


def swapped_words(data: bytes) -> bytes:
    return b"".join(data[offset:offset + 2][::-1]
                    for offset in range(0, len(data), 2))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP1_SOURCE_JOIN_INVALID: negative frame")
        return 1
    try:
        blob = args.capture.read_bytes()
        required = args.capture_frames or args.frame + 1
        frames, states = frame_regions(blob, required)
        commands = command_window(frames[args.frame]["vdp1-vram"], states[args.frame])
        mns = mns_surfaces(args.data_dir)
        dgn = dgn_structure2_surfaces(args.data_dir)
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_SOURCE_JOIN_INVALID: {error}")
        return 1

    draws: list[tuple[int, int, int, bytes]] = []
    vram = frames[args.frame]["vdp1-vram"]
    for offset, words in commands:
        control = words[0]
        command_type = control & 0x000F
        if command_type > 2 or control & 0x8000:
            continue
        colour_mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = (words[5] >> 8) & 0x00FF
        bits_per_pixel = 4 if colour_mode <= 1 else 8 if colour_mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = (width * height * bits_per_pixel) // 8
        source_end = source_offset + source_size
        if source_size <= 0 or source_end > len(vram):
            print("NEXUS_VDP1_SOURCE_JOIN_INVALID: source span outside VDP1 VRAM")
            return 1
        draws.append((offset, colour_mode, source_offset, vram[source_offset:source_end]))

    print(
        f"frame={args.frame} draw_commands={len(draws)} "
        f"mns_surfaces={len(mns)} dgn_structure2_surfaces={len(dgn)}"
    )
    for offset, colour_mode, source_offset, source in draws:
        source_hash = hashlib.sha256(source).hexdigest()
        exact: list[str] = []
        swapped_exact: list[str] = []
        dgn_exact: list[str] = []
        dgn_swapped_exact: list[str] = []
        for name, surface in mns:
            if surface == source:
                exact.append(name)
            if swapped_words(surface) == source:
                swapped_exact.append(name)
        for name, surface in dgn:
            if surface == source:
                dgn_exact.append(name)
            if swapped_words(surface) == source:
                dgn_swapped_exact.append(name)
        print(
            f"command_offset=0x{offset:05x} colour_mode={colour_mode} "
            f"source_offset=0x{source_offset:05x} source_bytes={len(source)} "
            f"source_sha256={source_hash}"
        )
        print("mns_exact=" + ("|".join(exact) if exact else "none"))
        print("mns_word_swap_exact=" +
              ("|".join(swapped_exact) if swapped_exact else "none"))
        print("dgn_structure2_exact=" +
              ("|".join(dgn_exact) if dgn_exact else "none"))
        print("dgn_structure2_word_swap_exact=" +
              ("|".join(dgn_swapped_exact) if dgn_swapped_exact else "none"))
    print("source_join=verified" if any(
        surface == source or swapped_words(surface) == source
        for _, _, _, source in draws for _, surface in (mns + dgn)
    ) else "source_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Join one authenticated VDP1 draw to a DGN image and its VDP1 CLUT.

This is an evidence probe, not a renderer admission gate.  The captured
VDP1 snapshot is little-endian because it is a raw Saturn VRAM word image;
DMWeb DGN fields and Structure2 BGR555 palette words are big-endian.  For
VDP1 colour mode 1, Mednafen's VDP1 consumer addresses the 16-word CLUT at
``((COLR & ~3) << 2)`` VDP1 words.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from analyze_nexus_vdp1_command_window import command_window
from analyze_nexus_vdp1_source_join import (
    be16,
    be32,
    swapped_words,
)
from fixtures.nexus_v1_disc_file_hashes import DISC_HASH


def dgn_materials(data_dir: Path) -> list[tuple[str, bytes, bytes]]:
    materials: list[tuple[str, bytes, bytes]] = []
    for level in range(16):
        name = f"LEV{level:02d}.DGN"
        data = (data_dir / name).read_bytes()
        if hashlib.sha256(data).hexdigest() != DISC_HASH[name]:
            raise ValueError(f"{name} is not the canonical retail DGN")
        block = be16(data, 0x14)
        useful = be32(data, 0x18)
        base = block * 0x800
        if base + useful > len(data):
            raise ValueError(f"{name} Structure2 envelope is invalid")
        cursor = 0
        descriptor_index = 0
        while cursor + 20 <= useful:
            descriptor = data[base + cursor:base + cursor + 20]
            image_id = be16(descriptor, 0)
            if image_id == 0xFFFF:
                break
            if image_id != descriptor_index:
                raise ValueError(f"{name} Structure2 image ids are not sequential")
            encoding = be16(descriptor, 2)
            width = be16(descriptor, 6)
            height = be16(descriptor, 8)
            image_offset = be32(descriptor, 12)
            palette_offset = be32(descriptor, 16)
            if encoding != 0x0008 or width == 0 or height == 0:
                cursor += 20
                descriptor_index += 1
                continue
            image_size = (width * height + 1) // 2
            palette_size = 16 * 2
            if (image_offset + image_size > useful or
                    palette_offset + palette_size > useful):
                raise ValueError(f"{name} Structure2 material is unbounded")
            materials.append((
                f"{name}[Structure2={image_id} encoding=0x{encoding:04x} "
                f"{width}x{height}]",
                data[base + image_offset:base + image_offset + image_size],
                data[base + palette_offset:base + palette_offset + palette_size],
            ))
            cursor += 20
            descriptor_index += 1
        else:
            raise ValueError(f"{name} Structure2 terminator is missing")
    return materials


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    parser.add_argument("--command-offset", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        frames, states = frame_regions(
            args.capture.read_bytes(), args.capture_frames or args.frame + 1
        )
        records = command_window(
            frames[args.frame]["vdp1-vram"], states[args.frame],
            args.command_offset,
        )
        materials = dgn_materials(args.data_dir)
    except (OSError, ValueError, struct.error) as error:
        print(f"NEXUS_VDP1_DGN_MATERIAL_JOIN_INVALID: {error}")
        return 1

    vram = frames[args.frame]["vdp1-vram"]
    for offset, words in records:
        control = words[0]
        if control & 0x8000 or (control & 0x000F) > 2:
            continue
        mode = (words[2] >> 3) & 0x7
        width = (words[5] & 0x003F) * 8
        height = (words[5] >> 8) & 0x00FF
        bits = 4 if mode <= 1 else 8 if mode <= 4 else 16
        source_offset = words[4] * 8
        source_size = width * height * bits // 8
        if source_size <= 0:
            continue
        source = vram[source_offset:source_offset + source_size]
        matches = [m for m in materials if swapped_words(m[1]) == source]
        clut_word = (words[3] & ~0x3) << 2
        captured_palette = vram[clut_word * 2:clut_word * 2 + 32]
        print(
            f"command_offset=0x{offset:05x} colour_mode={mode} "
            f"colr=0x{words[3]:04x} clut_word=0x{clut_word:05x} "
            f"source_sha256={hashlib.sha256(source).hexdigest()}"
        )
        if not matches:
            print("dgn_material=none")
            continue
        for name, _, palette in matches:
            palette_match = captured_palette == swapped_words(palette)
            print(f"dgn_material={name}")
            print(f"dgn_palette_match={'verified' if palette_match else 'none'}")
        print("source_clut_join=verified" if any(
            captured_palette == swapped_words(palette) for _, _, palette in matches
        ) else "source_clut_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

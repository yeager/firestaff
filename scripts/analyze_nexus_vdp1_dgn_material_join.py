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
import re
import struct
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions_vdp1
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


def dgn_structure3_face_owners(
    data_dir: Path, level_name: str, image_id: int
) -> list[str]:
    """Return one canonical level's faces whose raw selector is image_id."""
    owners: list[str] = []
    name = level_name
    data = (data_dir / name).read_bytes()
    if hashlib.sha256(data).hexdigest() != DISC_HASH[name]:
        raise ValueError(f"{name} is not the canonical retail DGN")
    structure3_block = be16(data, 0x1C)
    structure3_blocks = be16(data, 0x1E)
    base = structure3_block * 0x800
    size = structure3_blocks * 0x800
    if (structure3_block == 0 or structure3_blocks == 0 or
            base > len(data) or size > len(data) - base or size < 4):
        return owners
    entry_count = be32(data, base)
    directory_end = 4 + entry_count * 4
    if entry_count > 4096 or directory_end > size:
        raise ValueError(f"{name} Structure3 directory is invalid")
    for entry in range(entry_count):
        entry_offset = be32(data, base + 4 + entry * 4)
        entry_end = (be32(data, base + 4 + (entry + 1) * 4)
                     if entry + 1 < entry_count else size)
        if (entry_offset + 24 > entry_end or entry_end > size):
            raise ValueError(f"{name} Structure3 entry is invalid")
        face_count = be16(data, base + entry_offset + 6)
        face_offset = be32(data, base + entry_offset + 16)
        if face_offset > size or face_count > (size - face_offset) // 12:
            raise ValueError(f"{name} Structure3 faces are invalid")
        for face in range(face_count):
            row = base + face_offset + face * 12
            if be16(data, row + 10) == image_id:
                owners.append(f"{name}[Structure3-entry={entry},face={face}]")
    return owners


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    parser.add_argument("--command-offset", type=lambda value: int(value, 0))
    args = parser.parse_args()
    try:
        frames, states = frame_regions_vdp1(
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
        coordinates = tuple(
            struct.unpack("<h", struct.pack("<H", words[index]))[0]
            for index in range(6, 14)
        )
        print(
            f"command_offset=0x{offset:05x} colour_mode={mode} "
            f"colr=0x{words[3]:04x} clut_word=0x{clut_word:05x} "
            f"source_sha256={hashlib.sha256(source).hexdigest()} "
            f"xy={coordinates[0]},{coordinates[1]};"
            f"{coordinates[2]},{coordinates[3]};"
            f"{coordinates[4]},{coordinates[5]};"
            f"{coordinates[6]},{coordinates[7]}"
        )
        if not matches:
            print("dgn_material=none")
            continue
        for name, _, palette in matches:
            palette_match = captured_palette == swapped_words(palette)
            print(f"dgn_material={name}")
            print(f"dgn_palette_match={'verified' if palette_match else 'none'}")
            print("vdp1_coordinates_observed=verified")
            image_match = re.search(r"Structure2=(\d+)", name)
            image_id = int(image_match.group(1)) if image_match else -1
            level_match = re.match(r"(LEV\d\d\.DGN)", name)
            level_name = level_match.group(1) if level_match else ""
            owners = dgn_structure3_face_owners(
                args.data_dir, level_name, image_id
            ) if level_name else []
            print("dgn_structure3_face_owners=" +
                  ("|".join(owners) if owners else "none"))
            print("dgn_face_selector_join=" +
                  ("verified" if owners else "unbound"))
        print("source_clut_join=verified" if any(
            captured_palette == swapped_words(palette) for _, _, palette in matches
        ) else "source_clut_join=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

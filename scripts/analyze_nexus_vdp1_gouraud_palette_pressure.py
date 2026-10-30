#!/usr/bin/env python3
"""Measure indexed-palette pressure for an authenticated VDP1 capture window.

This analyzer consumes local Ymir capture evidence. It does not generate game
pixels or admit a renderer; it estimates whether authentic Gouraud-adjusted
mode-1 colors fit the existing 256-entry indexed framebuffer.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def read_manifest(root: Path) -> dict[str, str]:
    rows: dict[str, str] = {}
    for line in (root / "SHA256SUMS.txt").read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        digest, relative = line.split("  ", 1)
        rows[relative] = digest
    return rows


def verified_file(root: Path, relative: str, manifest: dict[str, str]) -> Path:
    path = root / relative
    expected = manifest.get(f"./{relative}")
    if expected is None or sha256(path) != expected:
        raise ValueError(f"capture file is absent from or disagrees with the manifest: {relative}")
    return path


def read_le16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def read_be16(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def shade_rgb555(word: int, gouraud: int) -> tuple[int, int, int]:
    return tuple(
        max(0, min(31, ((word >> shift) & 31) + ((gouraud >> shift) & 31) - 16))
        for shift in (0, 5, 10)
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path, help="authenticated Ymir capture root")
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--first", type=int, required=True)
    parser.add_argument("--last", type=int, required=True)
    parser.add_argument("--output", type=Path, help="optional JSON report path")
    args = parser.parse_args()
    if args.frame < 0 or args.first < 0 or args.last < args.first:
        parser.error("frame and command range must be non-negative and ordered")

    try:
        manifest = read_manifest(args.capture)
        trace_path = verified_file(args.capture, "dumps/vdp1-command-trace.csv", manifest)
        vram_path = verified_file(args.capture, "vdp1-vram-command-trace-reconstructed-le.bin", manifest)
        vdp2_path = verified_file(args.capture, "dumps/vdp2-vram.bin", manifest)
        cram_path = verified_file(args.capture, "dumps/vdp2-cram.bin", manifest)
        fbs_path = verified_file(args.capture, "dumps/vdp1-fbs.bin", manifest)
        regs_path = verified_file(args.capture, "dumps/vdp2-registers.bin", manifest)
        trace_hash, vram_hash, vdp2_hash = sha256(trace_path), sha256(vram_path), sha256(vdp2_path)
        cram_hash, fbs_hash, regs_hash = sha256(cram_path), sha256(fbs_path), sha256(regs_path)
        vram = vram_path.read_bytes()
        vdp2_vram = vdp2_path.read_bytes()
        cram = cram_path.read_bytes()
        fbs = fbs_path.read_bytes()
        regs = regs_path.read_bytes()
    except (OSError, ValueError) as error:
        parser.error(str(error))

    if (len(vram) != 0x80000 or len(vdp2_vram) != 0x80000 or
            len(cram) != 4096 or len(fbs) != 0x80000 or len(regs) != 512):
        parser.error("capture has unexpected VDP/CRAM plane size")
    observed_registers = {
        "TVMD": read_be16(regs, 0), "BGON": read_be16(regs, 0x20),
        "CHCTLA": read_be16(regs, 0x28), "BMPNA": read_be16(regs, 0x2C),
        "CRAOFA": read_be16(regs, 0xE4), "CRAOFB": read_be16(regs, 0xE6),
    }
    if observed_registers != {
        "TVMD": 0x8000, "BGON": 1, "CHCTLA": 0x0013,
        "BMPNA": 0, "CRAOFA": 0, "CRAOFB": 0,
    }:
        parser.error("capture does not establish the expected NBG0 8bpp bitmap/CRAM lane")

    background_indices = {
        color
        for y in range(224)
        for color in vdp2_vram[y * 512 : y * 512 + 320]
    }
    active_cram_indices = background_indices - {0}
    background_rgb555: set[int] = set()
    fbs_pixels = 0
    for y in range(224):
        for x in range(320):
            palette_index = vdp2_vram[y * 512 + x]
            if palette_index == 0:
                color_word = int.from_bytes(fbs[(y * 512 + x) * 2 : (y * 512 + x) * 2 + 2], "big")
                fbs_pixels += 1
            else:
                color_word = int.from_bytes(cram[palette_index * 2 : palette_index * 2 + 2], "big")
            background_rgb555.add(color_word & 0x7FFF)
    variants: set[tuple[tuple[int, ...], tuple[int, int, int]]] = set()
    all_colors: set[tuple[int, int, int]] = set()
    visible_colors: set[tuple[int, int, int]] = set()
    mode1_indices: list[int] = []
    table_words: dict[int, int] = {}

    with trace_path.open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            frame = int(row["frame"], 10)
            index = int(row["index"], 10)
            if frame != args.frame or not args.first <= index <= args.last:
                continue
            words = [int(row[f"word{i}"], 0) for i in range(16)]
            address = int(row["address"], 0)
            if address % 32 or address + 32 > len(vram):
                parser.error(f"command {index} has invalid VRAM address {address:#x}")
            if struct.unpack_from("<16H", vram, address) != tuple(words):
                parser.error(f"command {index} trace bytes disagree with reconstructed VDP1 VRAM")
            draw_mode = words[2]
            if ((draw_mode >> 3) & 7) != 1:
                continue
            width = ((words[5] >> 8) & 0x3F) * 8
            height = words[5] & 0xFF
            texture_offset = words[4] * 8
            palette_offset = words[3] * 8
            gouraud_offset = words[14] * 8
            texture_size = width * height // 2
            if not width or not height or texture_offset + texture_size > len(vram):
                parser.error(f"command {index} has invalid texture span")
            if palette_offset + 32 > len(vram) or gouraud_offset + 8 > len(vram):
                parser.error(f"command {index} has invalid palette or Gouraud table")
            gouraud = tuple(read_le16(vram, gouraud_offset + 2 * i) for i in range(4))
            if len(set(gouraud)) != 1:
                parser.error(f"command {index} has varying vertex Gouraud values; this analyzer only counts uniform tables")
            correction = gouraud[0]
            if correction == 0x4210:
                parser.error(f"command {index} unexpectedly has neutral Gouraud values")
            table_words[index] = correction
            palette = tuple(read_le16(vram, palette_offset + 2 * i) for i in range(16))
            shaded = tuple(shade_rgb555(color, correction) for color in palette)
            variants.add((palette, tuple((correction >> shift) & 31 for shift in (0, 5, 10))))
            all_colors.update(shaded)
            texture = vram[texture_offset : texture_offset + texture_size]
            for pixel in range(width * height):
                packed = texture[pixel >> 1]
                texel = (packed >> 4) if pixel % 2 == 0 else (packed & 0x0F)
                if texel == 0 and not (draw_mode & 0x0040):
                    continue
                if texel == 0x0F and not (draw_mode & 0x0080):
                    continue
                visible_colors.add(shaded[texel])
            mode1_indices.append(index)

    if not mode1_indices:
        parser.error("no mode-1 commands found in the selected frame window")

    report = {
        "format": "firestaff-nexus-vdp1-gouraud-palette-pressure-v1",
        "frame_index": args.frame,
        "command_range": [args.first, args.last],
        "capture_hashes": {
            "vdp1_command_trace_sha256": trace_hash,
            "reconstructed_vdp1_vram_sha256": vram_hash,
            "vdp2_vram_sha256": vdp2_hash,
            "vdp2_cram_sha256": cram_hash,
            "vdp1_fbs_sha256": fbs_hash,
            "vdp2_registers_sha256": regs_hash,
        },
        "mode1_commands": mode1_indices,
        "gouraud_table_words": {str(index): f"0x{word:04x}" for index, word in table_words.items()},
        "background_unique_8bpp_indices_in_320x224": len(background_indices),
        "background_active_cram_indices_in_320x224": len(active_cram_indices),
        "background_vdp1_fbs_pixels_in_320x224": fbs_pixels,
        "fbs_plus_nbg0_unique_rgb555_colors_in_320x224": len(background_rgb555),
        "mode1_unique_palette_and_gouraud_variants": len(variants),
        "naive_palette_slots_required_at_16_per_variant": 16 * len(variants),
        "unique_shaded_palette_rgb555_colors": len(all_colors),
        "unique_shaded_colors_in_nontransparent_source_texels": len(visible_colors),
        "unique_colors_in_background_plus_mode1_source_palette_union": len(background_rgb555 | visible_colors),
        "indexed_framebuffer_capacity": 256,
        "renderer_permitted": False,
        "limitations": [
            "The visible background is reconstructed from the captured VDP1 FBS, NBG0 8bpp bitmap, CRAM, and checked VDP2 registers.",
            "Palette variant and unique-color counts are capacity diagnostics, not a pixel parity result.",
            "Only commands with uniform four-corner Gouraud tables are supported by this analyzer.",
        ],
    }
    encoded = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

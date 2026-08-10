#!/usr/bin/env python3
"""Compare an authenticated Saturn character-mode frame with FONT256.S2D.

This is an observation boundary for the NBG1/NBG0 character path.  It joins
only exact byte spans (including the palette/attribute regions) from the
authenticated FONT256 source to captured VDP2 VRAM/CRAM.  A match does not
identify the active menu, text consumer, page placement, or glyph semantics;
the command therefore never grants production rendering admission.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_saturn_runtime_capture import frame_regions
from nexus_vdp2_registers import detect_byte_order, read_u16


FONT256_SHA256 = "764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb"


def wordswapped(data: bytes) -> bytes:
    if len(data) % 2:
        return b""
    return b"".join(data[index:index + 2][::-1]
                    for index in range(0, len(data), 2))


def font_regions(data: bytes) -> dict[str, bytes]:
    """Read the four populated FONT256 SCR spans from its descriptor table."""
    if len(data) < 0x60 or data[:16] != b"SEGA SATURN SCR\0":
        raise ValueError("FONT256.S2D SCR header is invalid")
    regions: dict[str, bytes] = {}
    names = ("page", "character_generator", "palette", "attributes")
    # The populated SCR descriptors are the even descriptor pairs beginning
    # at 0x20 in the verified European extraction.  The intervening records
    # are empty descriptors and are deliberately not collapsed into a fake
    # glyph map.
    for index, name in enumerate(names):
        descriptor = 0x20 + index * 16
        offset = int.from_bytes(data[descriptor:descriptor + 4], "big")
        size = int.from_bytes(data[descriptor + 4:descriptor + 8], "big")
        if offset < 0x60 or size == 0 or offset + size > len(data):
            raise ValueError(f"FONT256 {name} span is invalid")
        regions[name] = data[offset:offset + size]
    return regions


def find_span(haystack: bytes, source: bytes) -> tuple[int, int]:
    exact = haystack.find(source)
    swapped = haystack.find(wordswapped(source))
    return exact, swapped


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--frame", type=int, default=0)
    parser.add_argument("--capture-frames", type=int, default=None)
    args = parser.parse_args()
    if args.frame < 0:
        print("NEXUS_VDP2_CHAR_SOURCE_INVALID: negative frame")
        return 1
    required = args.capture_frames or args.frame + 1
    if required <= args.frame:
        print("NEXUS_VDP2_CHAR_SOURCE_INVALID: frame is outside capture")
        return 1
    try:
        capture = args.capture.read_bytes()
        frames, _ = frame_regions(capture, required)
        font = (args.data_dir / "FONT256.S2D").read_bytes()
        if hashlib.sha256(font).hexdigest() != FONT256_SHA256:
            raise ValueError("FONT256.S2D hash mismatch")
        regions = font_regions(font)
    except (OSError, ValueError) as error:
        print(f"NEXUS_VDP2_CHAR_SOURCE_INVALID: {error}")
        return 1

    frame = frames[args.frame]
    registers = frame["vdp2-regs"]
    byte_order = detect_byte_order(registers)
    bgon = read_u16(registers, 0x20, byte_order)
    chctla = read_u16(registers, 0x28, byte_order)
    pncn1 = read_u16(registers, 0x32, byte_order)
    vram = frame["vdp2-vram"]
    cram = frame["vdp2-cram"]
    print(f"frame={args.frame} register_byte_order={byte_order} "
          f"bgon=0x{bgon:04x} chctla=0x{chctla:04x} pncn1=0x{pncn1:04x}")

    vram_matches = 0
    cram_matches = 0
    for name, source in regions.items():
        exact, swapped = find_span(vram, source)
        if exact >= 0 or swapped >= 0:
            vram_matches += 1
        print(f"vram_{name}_exact=0x{exact:x} vram_{name}_word_swap=0x{swapped:x} "
              f"bytes={len(source)}")
        if name == "palette":
            exact, swapped = find_span(cram, source)
            if exact >= 0 or swapped >= 0:
                cram_matches += 1
            print(f"cram_palette_exact=0x{exact:x} "
                  f"cram_palette_word_swap=0x{swapped:x} bytes={len(source)}")

    print(f"font256_vram_span_matches={vram_matches}/4")
    print(f"font256_cram_palette_matches={cram_matches}/1")
    print("source_join=verified" if vram_matches == 4 and cram_matches == 1
          else "source_join=unbound")
    print("text_consumer_identity=unbound")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Compare real MENU.BPK PRS3 surfaces with one VDP1 VRAM snapshot.

This is a narrow source-byte observation tool.  It accepts only the
hash-verified Japanese retail MENU.BPK, decodes its declared indexed PRS3
surfaces with the documented DMWeb grammar, and reports exact whole-surface
matches in a supplied VDP1 VRAM dump.  A match does not establish a Saturn
upload route, CLUT, placement, command ordering, or renderer permission.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path


JAPANESE_MENU_BPK_SHA256 = (
    "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636"
)
VDP1_TRACE_REQUIRED_COLUMNS = {"word0", "word4"}


def be32(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 4], "big")


def word_swap(data: bytes) -> bytes:
    swapped = bytearray(data)
    for offset in range(0, len(swapped) - 1, 2):
        swapped[offset], swapped[offset + 1] = swapped[offset + 1], swapped[offset]
    return bytes(swapped)


def all_offsets(haystack: bytes, needle: bytes) -> list[int]:
    """Return every possibly overlapping occurrence of a nonempty byte string."""
    if not needle:
        raise ValueError("cannot search for an empty surface")
    offsets = []
    cursor = 0
    while True:
        offset = haystack.find(needle, cursor)
        if offset < 0:
            return offsets
        offsets.append(offset)
        cursor = offset + 1


def surface_offsets(vram: bytes, pixels: bytes) -> tuple[list[int], list[int]]:
    """Return all direct and word-swapped offsets for one decoded surface."""
    return all_offsets(vram, pixels), all_offsets(vram, word_swap(pixels))


def decode_prs3(body: bytes, compressed_size: int, pixel_count: int) -> bytes:
    """Decode exactly the documented bounded DMWeb PRS3 byte grammar."""
    if compressed_size <= 0 or compressed_size > len(body) or pixel_count <= 0:
        raise ValueError("invalid PRS3 lengths")
    stream = body[:compressed_size]
    output = bytearray()
    cursor = 0
    while len(output) < pixel_count and cursor < len(stream):
        control = stream[cursor]
        cursor += 1
        for bit_index in range(8):
            if len(output) == pixel_count:
                break
            if control & (1 << bit_index):
                if cursor >= len(stream):
                    raise ValueError("truncated PRS3 literal")
                output.append(stream[cursor])
                cursor += 1
                continue
            if cursor + 1 >= len(stream):
                raise ValueError("truncated PRS3 back-reference")
            low, high = stream[cursor], stream[cursor + 1]
            cursor += 2
            count = 3 + (high & 0x0F)
            source = (((high >> 4) << 8) | low)
            source = source - 0xFEE if source >= 0xFDC else source + 18
            while len(output) - source > 4095:
                source += 4096
            for _ in range(min(count, pixel_count - len(output))):
                if source < 0:
                    output.append(0)
                elif source >= len(output):
                    raise ValueError("invalid forward PRS3 reference")
                else:
                    output.append(output[source])
                source += 1
    if len(output) != pixel_count:
        raise ValueError("PRS3 output is short")
    return bytes(output)


def entries(menu: bytes):
    if len(menu) < 24 or menu[:4] != b"BPPK" or be32(menu, 4) != len(menu):
        raise ValueError("invalid BPPK envelope")
    if menu[12:16] != b"BMPD":
        raise ValueError("missing BMPD directory")
    count = be32(menu, 20)
    if count == 0 or 24 + count * 4 > len(menu):
        raise ValueError("invalid BPPK entry count")
    offsets = [be32(menu, 24 + index * 4) for index in range(count)]
    if offsets != sorted(offsets) or any(offset + 36 > len(menu) for offset in offsets):
        raise ValueError("invalid BPPK offsets")
    palette_start = len(menu) - 524 if menu[-524:-520] == b"PALT" else len(menu)
    for index, offset in enumerate(offsets):
        end = offsets[index + 1] if index + 1 < count else palette_start
        if end <= offset or end > len(menu):
            raise ValueError("invalid BPPK entry span")
        if menu[offset + 20:offset + 24] != b"PRS3":
            continue
        width = int.from_bytes(menu[offset + 12:offset + 14], "big")
        height = menu[offset + 15]
        pixels = be32(menu, offset + 28)
        compressed = be32(menu, offset + 32)
        if width == 0 or height == 0 or pixels != width * height:
            raise ValueError(f"invalid PRS3 dimensions at entry {index}")
        # The DMWeb PRS3 compressed-length word is authoritative for the
        # bounded decoder.  In the retail directory it can cross the next
        # entry boundary, so do not truncate it to this entry's raw span.
        # It must nevertheless remain within the authenticated package.
        body_start = offset + 36
        if body_start + compressed > len(menu):
            raise ValueError(f"PRS3 compressed stream exceeds package at entry {index}")
        yield index, width, height, decode_prs3(
            menu[body_start:body_start + compressed], compressed, pixels)


def command_trace_source_starts(trace_path: Path,
                                matched_ranges: list[tuple[int, int]]) -> tuple[int, int]:
    """Count textured VDP1 commands whose source start is in a matched range.

    The VDP1 command trace is an optional corroborating input.  It reports
    only command type 0, 1, or 2 (normal/scaled/distorted sprites), because
    those are the textured VDP1 primitives.  A source start in a range is
    still not proof of a complete source span, palette binding, or display
    ownership.
    """
    try:
        with trace_path.open(newline="", encoding="utf-8") as handle:
            reader = csv.DictReader(handle)
            if (reader.fieldnames is None
                    or not VDP1_TRACE_REQUIRED_COLUMNS.issubset(reader.fieldnames)):
                raise ValueError("missing VDP1 command-trace columns")
            rows = list(reader)
    except (OSError, csv.Error, UnicodeError) as error:
        raise ValueError(f"invalid VDP1 command trace: {error}") from error
    if not rows:
        raise ValueError("VDP1 command trace has no command rows")
    textured = 0
    source_starts_in_match = 0
    for row in rows:
        try:
            command_type = int(row["word0"], 0) & 0x000f
            # VDP1 command words are CMDCTRL, CMDLINK, CMDPMOD, CMDCOLR,
            # CMDSRCA, ...; CMDSRCA is word4 and stores the address / 8.
            source_start = int(row["word4"], 0) << 3
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError("invalid VDP1 command trace row") from error
        if command_type not in (0, 1, 2):
            continue
        textured += 1
        if any(begin <= source_start < end for begin, end in matched_ranges):
            source_starts_in_match += 1
    return textured, source_starts_in_match


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("menu_bpk", type=Path)
    parser.add_argument("vdp1_vram", type=Path)
    parser.add_argument("--expected-vdp1-sha256", default=None)
    parser.add_argument("--vdp1-command-trace", type=Path, default=None,
                        help="optional Ymir vdp1-command-trace.csv")
    args = parser.parse_args()
    try:
        menu = args.menu_bpk.read_bytes()
        vram = args.vdp1_vram.read_bytes()
    except OSError as error:
        print(f"NEXUS_MENU_BPK_VDP1_JOIN_INVALID: {error}")
        return 1
    menu_hash = hashlib.sha256(menu).hexdigest()
    vram_hash = hashlib.sha256(vram).hexdigest()
    if menu_hash != JAPANESE_MENU_BPK_SHA256:
        print("NEXUS_MENU_BPK_VDP1_JOIN_INVALID: MENU.BPK hash is not Japanese retail")
        return 1
    if len(vram) != 512 * 1024:
        print("NEXUS_MENU_BPK_VDP1_JOIN_INVALID: VDP1 snapshot is not 512 KiB")
        return 1
    if args.expected_vdp1_sha256 and vram_hash != args.expected_vdp1_sha256.lower():
        print("NEXUS_MENU_BPK_VDP1_JOIN_INVALID: VDP1 snapshot hash mismatch")
        return 1
    try:
        decoded = list(entries(menu))
    except ValueError as error:
        print(f"NEXUS_MENU_BPK_VDP1_JOIN_INVALID: {error}")
        return 1
    matches = 0
    matched_ranges: list[tuple[int, int]] = []
    for index, width, height, pixels in decoded:
        direct_offsets, swapped_offsets = surface_offsets(vram, pixels)
        direct = direct_offsets[0] if direct_offsets else -1
        swapped = swapped_offsets[0] if swapped_offsets else -1
        if direct >= 0 or swapped >= 0:
            matches += 1
            matched_ranges.extend(
                (offset, offset + len(pixels))
                for offset in sorted(set(direct_offsets + swapped_offsets))
            )
            print(
                f"entry={index} size={width}x{height} bytes={len(pixels)} "
                f"vram_exact_offsets={','.join(f'0x{offset:x}' for offset in direct_offsets) or 'none'} "
                f"vram_word_swap_offsets={','.join(f'0x{offset:x}' for offset in swapped_offsets) or 'none'}"
            )
    print(f"menu_bpk_sha256={menu_hash}")
    print(f"vdp1_vram_sha256={vram_hash}")
    print(f"decoded_prs3_surfaces={len(decoded)}")
    print(f"whole_surface_byte_matches={matches}/{len(decoded)}")
    if args.vdp1_command_trace:
        try:
            textured, source_starts_in_match = command_trace_source_starts(
                args.vdp1_command_trace, matched_ranges)
        except ValueError as error:
            print(f"NEXUS_MENU_BPK_VDP1_JOIN_INVALID: {error}")
            return 1
        print(f"vdp1_textured_commands={textured}")
        print(f"vdp1_textured_source_starts_in_matched_ranges={source_starts_in_match}")
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

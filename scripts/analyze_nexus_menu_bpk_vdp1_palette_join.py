#!/usr/bin/env python3
"""Join authentic MENU.BPK texels to same-run VDP1 and VDP2 color state.

This bounded analyzer accepts hash-verified retail MENU.BPK data and one
atomic Ymir capture. It supports only VDP1 8bpp bank mode feeding VDP2 sprite
type 6 with zero CRAOFB sprite color offset. A palette match is a color-index
source join; it does not authorize rendering or establish symbol semantics.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from collections import defaultdict
from pathlib import Path

from analyze_nexus_menu_bpk_vdp1_snapshot_join import (
    RETAIL_MENU_BPK_SHA256,
    all_offsets,
    entries,
    verified_menu_region,
)


def read_word(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def palt_words(menu: bytes) -> list[int]:
    if len(menu) < 524 or menu[-524:-520] != b"PALT":
        raise ValueError("retail MENU.BPK does not end in a PALT chunk")
    chunk = menu[-524:]
    if int.from_bytes(chunk[4:8], "big") != 524:
        raise ValueError("invalid PALT chunk length")
    if int.from_bytes(chunk[8:12], "big") != 256:
        raise ValueError("PALT chunk does not declare 256 colors")
    return [read_word(chunk, 12 + index * 2) for index in range(256)]


def selected_surfaces(vram: bytes, surfaces: list[tuple[int, int, int, bytes]]):
    """Return direct byte ranges where complete decoded PRS3 surfaces reside."""
    result = []
    for entry, width, height, pixels in surfaces:
        if not pixels:
            continue
        for offset in all_offsets(vram, pixels):
            result.append((offset, offset + len(pixels), entry, width, height, pixels))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("menu_bpk", type=Path)
    parser.add_argument("vdp1_vram", type=Path)
    parser.add_argument("vdp1_registers", type=Path)
    parser.add_argument("vdp1_command_trace", type=Path)
    parser.add_argument("vdp2_registers", type=Path)
    parser.add_argument("vdp2_cram", type=Path)
    parser.add_argument("--frame", type=int, required=True)
    parser.add_argument("--expected-vdp1-sha256")
    parser.add_argument("--expected-vdp2-cram-sha256")
    args = parser.parse_args()

    try:
        menu, vram, vdp1_regs, vdp2_regs, cram = (
            path.read_bytes() for path in (
                args.menu_bpk, args.vdp1_vram, args.vdp1_registers,
                args.vdp2_registers, args.vdp2_cram,
            )
        )
    except OSError as error:
        print(f"NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: {error}")
        return 1

    region = verified_menu_region(menu)
    if region is None:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: MENU.BPK hash is not an authenticated retail revision")
        return 1
    if len(vram) != 512 * 1024 or len(vdp1_regs) != 32:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: invalid VDP1 capture dimensions")
        return 1
    if len(vdp2_regs) != 512 or len(cram) != 4096:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: invalid VDP2 capture dimensions")
        return 1
    if args.expected_vdp1_sha256 and digest(vram) != args.expected_vdp1_sha256.lower():
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: VDP1 VRAM hash mismatch")
        return 1
    if args.expected_vdp2_cram_sha256 and digest(cram) != args.expected_vdp2_cram_sha256.lower():
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: VDP2 CRAM hash mismatch")
        return 1

    tvmr = read_word(vdp1_regs, 0)
    spctl = read_word(vdp2_regs, 0xE0)
    craofb = read_word(vdp2_regs, 0xE6)
    sprite_type = spctl & 0x0007
    color_offset = craofb & 0x0007
    if tvmr != 0:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: VDP1 framebuffer is not 16-bit mode")
        return 1
    if sprite_type != 6 or color_offset != 0:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: capture is outside SP type 6 / zero CRAOFB scope")
        return 1

    try:
        palette = palt_words(menu)
        decoded = list(entries(menu))
        surfaces = selected_surfaces(vram, decoded)
        with args.vdp1_command_trace.open(newline="", encoding="utf-8") as handle:
            reader = csv.DictReader(handle)
            required = {"frame", "index", "word0", "word2", "word3", "word4"}
            if reader.fieldnames is None or not required.issubset(reader.fieldnames):
                raise ValueError("VDP1 trace is missing required command fields")
            trace_rows = list(reader)
    except (OSError, UnicodeError, csv.Error, ValueError) as error:
        print(f"NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: {error}")
        return 1

    matches: dict[int, set[int]] = defaultdict(set)
    commands = []
    frame_rows = 0
    for row in trace_rows:
        try:
            if int(row["frame"], 0) != args.frame:
                continue
            frame_rows += 1
            control = int(row["word0"], 0)
            if control & 0xC000 or (control & 0x000F) not in (0, 1, 2):
                continue
            pmod = int(row["word2"], 0)
            mode = (pmod >> 3) & 0x7
            if mode != 4:  # 8bpp/256-color bank mode
                continue
            source_start = int(row["word4"], 0) << 3
            candidates = [item for item in surfaces if item[0] == source_start]
            if len(candidates) != 1:
                continue
            begin, end, entry, width, height, pixels = candidates[0]
            if end - begin != width * height or len(pixels) != width * height:
                raise ValueError(f"invalid joined surface size for MENU.BPK entry {entry}")
            bank = int(row["word3"], 0) & 0xFF00
            source_indices = sorted(set(pixels) - {0})
            if any(((bank | color) & 0x03FF) != color for color in source_indices):
                raise ValueError("source bank affects SP type 6 palette index")
            matches[entry].update(source_indices)
            commands.append((int(row["index"], 0), entry, width, height,
                             len(source_indices), bank, source_start))
        except (KeyError, TypeError, ValueError) as error:
            print(f"NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: invalid command row: {error}")
            return 1

    if frame_rows == 0:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_INVALID: no VDP1 command rows for requested frame")
        return 1
    active_indices = sorted({index for indices in matches.values() for index in indices})
    if not commands or not active_indices:
        print("NEXUS_MENU_BPK_VDP1_PALETTE_JOIN_UNBOUND: no exact retail 8bpp surface command join")
        return 1

    cram_words = [read_word(cram, index * 2) for index in range(2048)]
    mismatches = [index for index in active_indices if palette[index] != cram_words[index]]
    print(f"menu_region={region}")
    print(f"menu_sha256={digest(menu)}")
    print(f"vdp1_vram_sha256={digest(vram)}")
    print(f"vdp2_cram_sha256={digest(cram)}")
    print(f"frame={args.frame} sprite_type={sprite_type} color_data_offset={color_offset}")
    print(f"vdp1_8bpp_bank_commands={len(commands)} unique_surfaces={len(matches)}")
    print("commands=" + ",".join(
        f"{command}:{entry}:{width}x{height}:bank=0x{bank:04x}:vram=0x{source:05x}"
        for command, entry, width, height, _, bank, source in commands
    ))
    print(f"active_nonzero_indices={len(active_indices)}")
    print("index_words=" + ",".join(
        f"{index:02x}:palt={palette[index]:04x}:cram={cram_words[index]:04x}"
        for index in active_indices
    ))
    print(f"active_index_mismatches={len(mismatches)}")
    print("palette_index_join=" + ("verified" if not mismatches else "mismatch"))
    print("semantic_admission=blocked")
    return 0 if not mismatches else 1


if __name__ == "__main__":
    raise SystemExit(main())

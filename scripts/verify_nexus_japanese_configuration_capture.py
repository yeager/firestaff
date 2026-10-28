#!/usr/bin/env python3
"""Verify the Japanese retail Configuration selection against a Ymir capture.

This checks command records written immediately after Ymir's traced command
cursor, their exact TITLE.BIN texels and palettes, and every visible sprite
pixel in the paired screenshot. It does not claim full-screen menu parity.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_title_vdp2_source import read_title_assets
from analyze_nexus_vdp1_source_join import accepted_retail_hashes

JAPANESE_TITLE_SHA256 = (
    "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
)
TEXT_PALETTE_OFFSETS = (0x02F6, 0x0426, 0x0556, 0x0686, 0x07B6, 0x0986, 0x1F06)
SPRITES = (
    # command slot, source offset, dimensions, position, palette offsets,
    # expected colour-bank word, horizontal flip, visible texels
    (3, 0x0316, 64, 8, 129, 153, TEXT_PALETTE_OFFSETS, 0x5210, False, 313),
    (4, 0x0576, 64, 8, 128, 165, TEXT_PALETTE_OFFSETS, 0x5230, False, 293),
    (5, 0x1746, 24, 24, 85, 168, (0x1726,), 0x5300, False, 192),
    (6, 0x1746, 24, 24, 211, 168, (0x1726,), 0x5300, True, 192),
    (7, 0x09A6, 104, 8, 109, 177, TEXT_PALETTE_OFFSETS, 0x5260, False, 463),
)


def read_receipt(bundle: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in (bundle / "vdp1-capture-bundle.txt").read_text(
            encoding="ascii").splitlines():
        key, value = line.split("=", 1)
        if key in result:
            raise ValueError(f"duplicate receipt field: {key}")
        result[key] = value
    if (result.get("format") != "ymir-vdp1-capture-bundle-v1" or
            result.get("trace_file_complete") != "1" or
            result.get("trace_dropped_total") != "0"):
        raise ValueError("Ymir VDP1 trace is incomplete or has dropped rows")
    return result


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def verify(bundle: Path, screenshot_path: Path, title_path: Path) -> None:
    from PIL import Image

    title = (read_title_assets(None, title_path)[0]
             if title_path.suffix.lower() == ".cue"
             else title_path.read_bytes())
    digest = hashlib.sha256(title).hexdigest()
    if digest != JAPANESE_TITLE_SHA256 or digest not in accepted_retail_hashes("TITLE.BIN"):
        raise ValueError("TITLE.BIN is not the authenticated Japanese retail resource")

    receipt = read_receipt(bundle)
    cursor = int(receipt["capture_next_command_index"])
    traced_rows = int(receipt["trace_rows_for_capture_frame"])
    if cursor != traced_rows:
        raise ValueError("capture command cursor does not follow the complete frame trace")
    vdp1 = (bundle / "vdp1-vram.bin").read_bytes()
    vdp2 = (bundle / "vdp2-vram.bin").read_bytes()
    cram = (bundle / "vdp2-cram.bin").read_bytes()
    regs = (bundle / "vdp2-registers.bin").read_bytes()
    if (len(vdp1) != 512 * 1024 or len(vdp2) != 512 * 1024 or
            len(cram) != 4096 or len(regs) != 512):
        raise ValueError("incomplete Saturn video memory snapshot")
    if int.from_bytes(regs[0xE0:0xE2], "big") != 0x1536:
        raise ValueError("unexpected VDP2 sprite mode")
    if (int.from_bytes(regs[0xEC:0xEE], "big") & 0x740) != 0x40:
        raise ValueError("unexpected VDP2 sprite colour-calculation mode")
    reg = lambda offset: int.from_bytes(regs[offset:offset + 2], "big")
    if (reg(0x20), reg(0x28), reg(0x2C), reg(0x3C), reg(0xE4)) != (
            1, 0x13, 0, 0, 0):
        raise ValueError("unexpected captured NBG0 configuration")
    sprite_priority = (int.from_bytes(regs[0xE0:0xE2], "big") >> 8) & 7
    back_upper, back_lower = reg(0xAC), reg(0xAE)
    back_address = ((back_upper & 7) << 17) | (back_lower << 1)

    image = Image.open(screenshot_path).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected a 640x448 Ymir screenshot")

    list_start = cursor * 32
    if list_start + 8 * 32 > len(vdp1):
        raise ValueError("post-cursor command list is outside VDP1 VRAM")
    for (slot, title_offset, width, height, x, y, allowed_palettes,
         expected_colour, flip_x, visible_expected) in SPRITES:
        address = list_start + slot * 32
        words = [be16(vdp1, address + index * 2) for index in range(16)]
        control, mode, colour, source_address, size = (
            words[0], words[2], words[3], words[4], words[5])
        actual_width = ((size >> 8) & 0x3F) * 8
        actual_height = size & 0xFF
        source_offset = source_address * 8
        texture_size = width * height // 2
        texture = vdp1[source_offset:source_offset + texture_size]
        if (control != (0x1010 if flip_x else 0x1000) or mode != 0x0080 or
                colour != expected_colour or actual_width != width or
                actual_height != height or (words[6], words[7]) != (x, y) or
                title.find(texture) != title_offset):
            raise ValueError(f"retail command at {address:#x} does not match its source role")
        priority_index = (colour >> 12) & 7
        priority_word = int.from_bytes(
            regs[0xF0 + (priority_index // 2) * 2:
                 0xF2 + (priority_index // 2) * 2], "big")
        priority = (priority_word >> (8 if priority_index & 1 else 0)) & 7
        if priority != sprite_priority or priority != 5:
            raise ValueError(f"command {address:#x} is not at the captured sprite priority")
        ratio_index = (colour >> 10) & 3
        ratio_word = int.from_bytes(
            regs[0x100 + (ratio_index // 2) * 2:
                 0x102 + (ratio_index // 2) * 2], "big")
        ratio = 31 - ((ratio_word >> (8 if ratio_index & 1 else 0)) & 31)
        if ratio != 31:
            raise ValueError(f"command {address:#x} does not use the captured 31/32 ratio")

        palette_start = (colour & 0x03F0) * 2
        palette = cram[palette_start:palette_start + 32]
        palette_matches = []
        search = 0
        while True:
            offset = title.find(palette, search)
            if offset < 0:
                break
            palette_matches.append(offset)
            search = offset + 1
        if (len(palette) != 32 or not palette_matches or
                any(offset not in allowed_palettes for offset in palette_matches)):
            raise ValueError(f"CRAM palette at command {address:#x} is not source-bound")

        visible = matched = 0
        for py in range(height):
            for px in range(width):
                tx = width - 1 - px if flip_x else px
                texel_index = py * width + tx
                packed = texture[texel_index >> 1]
                texel = packed & 15 if texel_index & 1 else packed >> 4
                if texel == 0:
                    continue
                screen_x, screen_y = x + px, y + py
                bg_x, bg_y = screen_x, screen_y
                if vdp2[bg_y * 512 + bg_x] != 0:
                    raise ValueError(f"NBG0 is not transparent under command {address:#x}")
                back_offset = back_address + (2 * bg_y if back_upper & 0x8000 else 0)
                if back_offset + 2 > len(vdp2):
                    raise ValueError("captured back-screen table is outside VDP2 VRAM")
                if be16(vdp2, back_offset) != 0:
                    raise ValueError("title sprite back-screen is not black")
                color_index = (colour & 0x03F0) | texel
                word = be16(cram, color_index * 2)
                rgb = tuple(((((word >> shift) & 31) << 3) * 31) // 32
                            for shift in (0, 5, 10))
                visible += 1
                matched += image.getpixel(((x + px) * 2, (y + py) * 2)) == rgb
        if visible != visible_expected or matched != visible:
            raise ValueError(
                f"command {address:#x}: screenshot matches {matched}/{visible}, "
                f"expected {visible_expected}")
        print(
            f"command={address:#x} TITLE.BIN={title_offset:#x} "
            f"palette={','.join(hex(offset) for offset in palette_matches)} "
            f"screenshot_pixels={matched}/{visible}")

    print(f"capture_frame={receipt['capture_frame_index']} post_cursor={list_start:#x}")
    print("menu_state=japanese_configuration_selected")
    print("semantic_admission=source_bound_configuration_foreground")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir memory dump directory")
    parser.add_argument("screenshot", type=Path)
    parser.add_argument("title_source", type=Path, help="Japanese TITLE.BIN or CUE")
    args = parser.parse_args()
    try:
        verify(args.bundle, args.screenshot, args.title_source)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

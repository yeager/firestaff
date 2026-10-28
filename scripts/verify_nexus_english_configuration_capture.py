#!/usr/bin/env python3
"""Verify English retail Configuration sprites against a Ymir capture.

The title text is pixel-compared to the paired screenshot. Configuration
markers are source-, palette-, command-, placement-, and flip-bound; their
final screenshot RGB also depends on the independently unbound title
background and is not treated as a standalone sprite color sample.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from analyze_nexus_title_vdp2_source import read_title_assets
from analyze_nexus_vdp1_source_join import accepted_retail_hashes

ENGLISH_TITLE_SHA256 = (
    "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"
)
TEXT_PALETTE_OFFSETS = (0x02F6, 0x0426, 0x0556, 0x0686, 0x07B6, 0x0986, 0x1F06)
SPRITES = (
    # trace index, command, TITLE.BIN source, dimensions, position, palette,
    # colour word, horizontal flip, visible texels, screenshot pixel check
    (171, 0x1560, 0x0316, 64, 8, 129, 153, TEXT_PALETTE_OFFSETS,
     0x5210, False, 313, True),
    (172, 0x1580, 0x0576, 64, 8, 128, 165, TEXT_PALETTE_OFFSETS,
     0x5230, False, 293, True),
    (173, 0x15A0, 0x1DD6, 24, 24, 85, 168, (0x1DB6,),
     0x5350, False, 195, False),
    (174, 0x15C0, 0x1DD6, 24, 24, 211, 168, (0x1DB6,),
     0x5350, True, 195, False),
    (175, 0x15E0, 0x09A6, 104, 8, 109, 177, TEXT_PALETTE_OFFSETS,
     0x5260, False, 463, True),
)


def be16(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset:offset + 2], "big")


def palette_matches(title: bytes, palette: bytes) -> list[int]:
    matches = []
    offset = 0
    while True:
        offset = title.find(palette, offset)
        if offset < 0:
            return matches
        matches.append(offset)
        offset += 1


def visible_texels(texture: bytes, width: int, height: int, flip_x: bool) -> list[tuple[int, int, int]]:
    result = []
    for y in range(height):
        for x in range(width):
            source_x = width - 1 - x if flip_x else x
            index = y * width + source_x
            packed = texture[index >> 1]
            texel = packed & 15 if index & 1 else packed >> 4
            if texel:
                result.append((x, y, texel))
    return result


def verify(bundle: Path, screenshot_path: Path, title_path: Path) -> None:
    from PIL import Image

    title = (read_title_assets(None, title_path)[0]
             if title_path.suffix.lower() == ".cue"
             else title_path.read_bytes())
    digest = hashlib.sha256(title).hexdigest()
    if digest != ENGLISH_TITLE_SHA256 or digest not in accepted_retail_hashes("TITLE.BIN"):
        raise ValueError("TITLE.BIN is not the authenticated English retail resource")

    receipt = dict(
        line.split("=", 1)
        for line in (bundle / "vdp1-capture-bundle.txt").read_text(encoding="ascii").splitlines()
    )
    if (receipt.get("format") != "ymir-vdp1-capture-bundle-v1" or
            receipt.get("trace_file_complete") != "1" or
            receipt.get("trace_dropped_total") != "0"):
        raise ValueError("Ymir VDP1 trace is incomplete or has dropped rows")
    frame = int(receipt["capture_frame_index"])
    vdp1 = (bundle / "vdp1-vram.bin").read_bytes()
    cram = (bundle / "vdp2-cram.bin").read_bytes()
    regs = (bundle / "vdp2-registers.bin").read_bytes()
    if len(vdp1) != 512 * 1024 or len(cram) != 4096 or len(regs) != 512:
        raise ValueError("incomplete Saturn VDP1/VDP2 capture")
    reg = lambda offset: be16(regs, offset)
    if reg(0xE0) != 0x1536 or reg(0xEC) & 0x740 != 0x40:
        raise ValueError("unexpected captured VDP2 sprite configuration")
    sprite_priority = (reg(0xE0) >> 8) & 7
    image = Image.open(screenshot_path).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected a 640x448 Ymir screenshot")

    trace_path = bundle / receipt["trace_file"]
    rows = [row for row in csv.DictReader(trace_path.open(encoding="ascii", newline=""))
            if int(row["frame"]) == frame]
    if (len(rows) != int(receipt["trace_rows_for_capture_frame"]) or
            len(rows) != int(receipt["capture_next_command_index"])):
        raise ValueError("capture frame does not contain its complete VDP1 command list")
    by_index = {int(row["index"]): row for row in rows}

    for (trace_index, command_address, title_offset, width, height, x, y,
         allowed_palettes, expected_colour, flip_x, visible_expected,
         screenshot_check) in SPRITES:
        row = by_index.get(trace_index)
        if row is None or int(row["address"], 16) != command_address:
            raise ValueError(f"missing expected VDP1 command at {command_address:#x}")
        words = [int(row[f"word{index}"], 16) for index in range(16)]
        control, mode, colour, source_address, size = words[0], words[2], words[3], words[4], words[5]
        actual_width = ((size >> 8) & 0x3F) * 8
        actual_height = size & 0xFF
        source_offset = source_address * 8
        texture = vdp1[source_offset:source_offset + width * height // 2]
        if (control != (0x1010 if flip_x else 0x1000) or mode != 0x0080 or
                colour != expected_colour or actual_width != width or
                actual_height != height or (words[6], words[7]) != (x, y) or
                title.find(texture) != title_offset):
            raise ValueError(f"retail command at {command_address:#x} does not match its source role")

        priority_index = (colour >> 12) & 7
        priority_word = reg(0xF0 + (priority_index // 2) * 2)
        priority = (priority_word >> (8 if priority_index & 1 else 0)) & 7
        if priority != sprite_priority or priority != 5:
            raise ValueError(f"command {command_address:#x} is not at captured sprite priority")
        ratio_index = (colour >> 10) & 3
        ratio_word = reg(0x100 + (ratio_index // 2) * 2)
        ratio = 31 - ((ratio_word >> (8 if ratio_index & 1 else 0)) & 31)
        if ratio != 31:
            raise ValueError(f"command {command_address:#x} does not use captured 31/32 ratio")

        palette_start = (colour & 0x03F0) * 2
        palette = cram[palette_start:palette_start + 32]
        matches = palette_matches(title, palette)
        if (len(palette) != 32 or not matches or
                any(offset not in allowed_palettes for offset in matches)):
            raise ValueError(f"CRAM palette at command {command_address:#x} is not source-bound")

        visible = visible_texels(texture, width, height, flip_x)
        if len(visible) != visible_expected:
            raise ValueError(f"command {command_address:#x} has unexpected visible texel count")
        pixel_matches = 0
        if screenshot_check:
            for px, py, texel in visible:
                color_index = (colour & 0x03F0) | texel
                word = be16(cram, color_index * 2)
                rgb = tuple(((((word >> shift) & 31) << 3) * 31) // 32
                            for shift in (0, 5, 10))
                pixel_matches += image.getpixel(((x + px) * 2, (y + py) * 2)) == rgb
            if pixel_matches != visible_expected:
                raise ValueError(
                    f"command {command_address:#x}: screenshot matches "
                    f"{pixel_matches}/{visible_expected} text pixels")

        screenshot_result = (
            f"{pixel_matches}/{visible_expected}"
            if screenshot_check else "background-dependent"
        )
        print(
            f"command={command_address:#x} TITLE.BIN={title_offset:#x} "
            f"palette={','.join(hex(offset) for offset in matches)} "
            f"visible_texels={visible_expected} "
            f"screenshot_pixels={screenshot_result}")

    print(f"capture_frame={frame} traced_commands={len(rows)} dropped=0")
    print("menu_state=english_configuration_selected")
    print("semantic_admission=source_bound_configuration_foreground")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="Ymir VDP1 dump directory")
    parser.add_argument("screenshot", type=Path)
    parser.add_argument("title_source", type=Path, help="English TITLE.BIN or CUE")
    args = parser.parse_args()
    try:
        verify(args.bundle, args.screenshot, args.title_source)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

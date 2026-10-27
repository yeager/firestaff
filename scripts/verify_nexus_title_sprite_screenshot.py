#!/usr/bin/env python3
"""Compare retained Nexus title sprites with a Ymir screenshot and VRAM dump.

The screenshot follows the capture bundle; matches do not prove frame identity.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from analyze_nexus_vdp1_source_join import accepted_retail_hashes, ymir_commands

TITLE_MENU_DRAW_COUNT = 5


def title_menu_draws(commands, vram: bytes, source: bytes):
    """Find the final five informative 4bpp title-source draws in a frame.

    Ymir can relocate the VDP1 command table between otherwise equivalent
    cold boots. Identify the menu draws by their exact retail source bytes and
    ordering, never by emulator-specific command addresses.
    """
    matches = []
    for command_index, (address, words) in enumerate(commands):
        control, mode, colour, source_address, size = (
            words[0], words[2], words[3], words[4], words[5])
        if control & 0xC00F or mode != 0x80:
            continue
        width, height = ((size >> 8) & 63) * 8, size & 255
        if width <= 0 or height <= 0:
            continue
        source_offset = source_address * 8
        texture_size = width * height // 2
        texture = vram[source_offset:source_offset + texture_size]
        if (len(texture) != texture_size or not any(texture)):
            continue
        retail_offset = source.find(texture)
        if retail_offset >= 0:
            matches.append((command_index, address, words, retail_offset, texture))
    if len(matches) < TITLE_MENU_DRAW_COUNT:
        raise ValueError("fewer than five informative TITLE.BIN draws")
    return matches[-TITLE_MENU_DRAW_COUNT:]


def verify(bundle: Path, screenshot: Path, title: Path) -> None:
    from PIL import Image

    if hashlib.sha256(title.read_bytes()).hexdigest() not in accepted_retail_hashes("TITLE.BIN"):
        raise ValueError("TITLE.BIN is not an authenticated retail resource")
    vram, frame, commands = ymir_commands(bundle)
    vdp2_vram = (bundle / "vdp2-vram.bin").read_bytes()
    cram = (bundle / "vdp2-cram.bin").read_bytes()
    regs = (bundle / "vdp2-registers.bin").read_bytes()
    if len(vdp2_vram) != 512 * 1024 or len(cram) != 4096 or len(regs) < 0x108:
        raise ValueError("incomplete VDP2 snapshot")
    if int.from_bytes(regs[0xE0:0xE2], "big") != 0x1536 or \
       int.from_bytes(regs[0x0E:0x10], "big") != 0x0300 or \
       int.from_bytes(regs[0xE6:0xE8], "big") != 0:
        raise ValueError("unexpected sprite or colour RAM configuration")
    ccctl = int.from_bytes(regs[0xEC:0xEE], "big")
    spctl = int.from_bytes(regs[0xE0:0xE2], "big")
    if (ccctl & 0x740) != 0x40 or ((spctl >> 12) & 3) != 1:
        raise ValueError("unexpected sprite colour calculation mode")
    reg = lambda offset: int.from_bytes(regs[offset:offset + 2], "big")
    if (reg(0x20), reg(0x28), reg(0x2C), reg(0x3C), reg(0xE4)) != (1, 0x13, 0, 0, 0):
        raise ValueError("unexpected NBG0 configuration")
    if reg(0xE8) != 0 or (reg(0xF8) & 7) != 4:
        raise ValueError("unexpected line-screen or NBG0 priority")
    if any(reg(offset) for offset in (0x70, 0x72, 0x74, 0x76)) or \
       (reg(0x78), reg(0x7A), reg(0x7C), reg(0x7E)) != (1, 0, 1, 0):
        raise ValueError("unexpected NBG0 scroll or scale")
    back_upper, back_lower = reg(0xAC), reg(0xAE)
    back_address = ((back_upper & 7) << 17) | (back_lower << 1)
    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected the retained 2x screenshot")
    source = title.read_bytes()
    selected_draws = title_menu_draws(commands, vram, source)
    checked = set()
    for command_index, address, words, retail_offset, texture in selected_draws:
        ctrl, mode, colr, src, size = words[0], words[2], words[3], words[4], words[5]
        priority_index = (colr >> 12) & 7
        priority_reg = int.from_bytes(regs[0xF0 + (priority_index // 2) * 2:
                                           0xF2 + (priority_index // 2) * 2], "big")
        priority = (priority_reg >> (8 if priority_index & 1 else 0)) & 7
        if priority != ((spctl >> 8) & 7):
            raise ValueError("sprite priority does not enable colour calculation")
        ratio_index = (colr >> 10) & 3
        ratio_reg = int.from_bytes(regs[0x100 + (ratio_index // 2) * 2:
                                        0x102 + (ratio_index // 2) * 2], "big")
        ratio = 31 - ((ratio_reg >> (8 if ratio_index & 1 else 0)) & 31)
        if ratio != 31:
            raise ValueError("unexpected title sprite colour ratio")
        width, height = ((size >> 8) & 63) * 8, size & 255
        count = matches = 0
        for y in range(height):
            for x in range(width):
                u = width - 1 - x if ctrl & 0x10 else x
                v = height - 1 - y if ctrl & 0x20 else y
                index = v * width + u
                packed = texture[index // 2]
                texel = packed & 15 if index & 1 else packed >> 4
                if texel == 0:
                    continue
                colour_index = (colr & 0x3F0) | texel
                word = int.from_bytes(cram[colour_index * 2:colour_index * 2 + 2], "big")
                screen_x, screen_y = (words[6] + x) * 2, (words[7] + y) * 2
                if not (0 <= screen_x < 640 and 0 <= screen_y < 448):
                    raise ValueError("sprite is outside the retained screenshot")
                bg_x, bg_y = screen_x // 2, screen_y // 2
                if vdp2_vram[bg_y * 512 + bg_x] != 0:
                    raise ValueError("NBG0 is not transparent under the title sprite")
                back_offset = back_address + (2 * bg_y if back_upper & 0x8000 else 0)
                if back_offset + 2 > len(vdp2_vram):
                    raise ValueError("back-screen table is outside VDP2 VRAM")
                back = int.from_bytes(vdp2_vram[back_offset:back_offset + 2], "big")
                rgb = tuple((((((word >> shift) & 31) << 3) * ratio) +
                             ((((back >> shift) & 31) << 3) * (32 - ratio))) // 32
                            for shift in (0, 5, 10))
                count += 1
                matches += image.getpixel((screen_x, screen_y)) == rgb
        if count <= 0 or matches != count:
            raise ValueError(
                f"command {command_index} ({address:#x}): "
                f"{matches}/{count} screenshot matches")
        checked.add(command_index)
        print(
            f"frame={frame} command_index={command_index} command={address:#x} "
            f"title_bin_offset={retail_offset:#x} "
            f"screenshot_matches={matches}/{count}")
    if len(checked) != TITLE_MENU_DRAW_COUNT:
        raise ValueError("one or more source-bound title-menu sprites are missing")
    print("sprite_priority=5 ratio=31/32 lower_layer=VDP2_back_screen timing=unproven")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("screenshot", type=Path)
    parser.add_argument("title_bin", type=Path)
    args = parser.parse_args()
    verify(args.bundle, args.screenshot, args.title_bin)

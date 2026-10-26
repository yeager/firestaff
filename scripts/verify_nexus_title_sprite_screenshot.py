#!/usr/bin/env python3
"""Compare retained Nexus title sprites with a Ymir screenshot and VRAM dump.

The screenshot follows the capture bundle; matches do not prove frame identity
or the actual lower-layer pixels used by the observed colour calculation.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from PIL import Image

from analyze_nexus_vdp1_source_join import accepted_retail_hashes, ymir_commands

EXPECTED = {0xD8E0: 213, 0xD900: 213, 0xD920: 313, 0xD940: 293, 0xD960: 463}


def verify(bundle: Path, screenshot: Path, title: Path) -> None:
    if hashlib.sha256(title.read_bytes()).hexdigest() not in accepted_retail_hashes("TITLE.BIN"):
        raise ValueError("TITLE.BIN is not an authenticated retail resource")
    vram, frame, commands = ymir_commands(bundle)
    cram = (bundle / "vdp2-cram.bin").read_bytes()
    regs = (bundle / "vdp2-registers.bin").read_bytes()
    if len(cram) != 4096 or len(regs) < 0x108:
        raise ValueError("incomplete VDP2 snapshot")
    if int.from_bytes(regs[0xE0:0xE2], "big") != 0x1536 or \
       int.from_bytes(regs[0x0E:0x10], "big") != 0x0300 or \
       int.from_bytes(regs[0xE6:0xE8], "big") != 0:
        raise ValueError("unexpected sprite or colour RAM configuration")
    ccctl = int.from_bytes(regs[0xEC:0xEE], "big")
    spctl = int.from_bytes(regs[0xE0:0xE2], "big")
    if (ccctl & 0x740) != 0x40 or ((spctl >> 12) & 3) != 1:
        raise ValueError("unexpected sprite colour calculation mode")
    image = Image.open(screenshot).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected the retained 2x screenshot")
    source = title.read_bytes()
    checked = set()
    for address, words in commands:
        if address not in EXPECTED:
            continue
        ctrl, mode, colr, src, size = words[0], words[2], words[3], words[4], words[5]
        if ctrl & 0xC00F or mode != 0x80:
            raise ValueError("unexpected title sprite format")
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
        texture = vram[src * 8:src * 8 + width * height // 2]
        if not texture or source.find(texture) < 0:
            raise ValueError("title texture does not match retail bytes")
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
                # The observed result matches a black lower layer; its actual
                # pixels and their contribution remain independently unproven.
                rgb = tuple(((((word >> shift) & 31) << 3) * ratio) // 32
                            for shift in (0, 5, 10))
                screen_x, screen_y = (words[6] + x) * 2, (words[7] + y) * 2
                if not (0 <= screen_x < 640 and 0 <= screen_y < 448):
                    raise ValueError("sprite is outside the retained screenshot")
                count += 1
                matches += image.getpixel((screen_x, screen_y)) == rgb
        if count != EXPECTED[address] or matches != count:
            raise ValueError(f"sprite {address:#x}: {matches}/{count} screenshot matches")
        checked.add(address)
        print(f"frame={frame} command={address:#x} screenshot_matches={matches}/{count}")
    if checked != set(EXPECTED):
        raise ValueError("one or more title sprites are missing")
    print("sprite_priority=5 ratio=31/32; lower_layer_and_timing=unproven")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path)
    parser.add_argument("screenshot", type=Path)
    parser.add_argument("title_bin", type=Path)
    args = parser.parse_args()
    verify(args.bundle, args.screenshot, args.title_bin)

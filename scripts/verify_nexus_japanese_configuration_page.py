#!/usr/bin/env python3
"""Verify the Japanese retail Configuration foreground from real capture data.

The command compiles Firestaff's production renderer, feeds it the hash-pinned
Japanese retail TITLE.BIN, and compares seven UI states against a pixel map
decoded from that same file. It also checks every visible text pixel against
the paired Ymir screenshot. The scene behind the page and marker animation
are outside this check.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import subprocess
from pathlib import Path

from analyze_nexus_vdp1_source_join import accepted_retail_hashes
from verify_nexus_english_configuration_page import (
    ACTIVE_PALETTE_OFFSETS,
    CASES,
    INACTIVE_PALETTE_OFFSETS,
    MARKER_FRAMES,
    TEXT_SPRITES,
    pixel_rgb,
    source_palette_for_role,
    verify_production_renderer,
)


JAPANESE_TITLE_SHA256 = (
    "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
)
JAPANESE_SCREENSHOT_SHA256 = (
    "e3edfa97155f7c1684bf95f6f11cf600fd8ee367d12fecc556f8c50f0009087f"
)
ENGLISH_TITLE_SHA256 = (
    "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"
)


def verify_screenshot(title: bytes, screenshot_path: Path) -> None:
    from PIL import Image

    if hashlib.sha256(screenshot_path.read_bytes()).hexdigest() != (
            JAPANESE_SCREENSHOT_SHA256):
        raise ValueError("screenshot is not the retained authentic Ymir page capture")
    image = Image.open(screenshot_path).convert("RGB")
    if image.size != (640, 448):
        raise ValueError("expected the authentic 640x448 Ymir screenshot")

    matched = visible = 0
    case = CASES["initial"]
    for offset, width, height, x, y, name in TEXT_SPRITES:
        texture = title[offset:offset + width * height // 2]
        palette_offset = source_palette_for_role(name, case)
        for py in range(height):
            for px in range(width):
                texel_index = py * width + px
                packed = texture[texel_index >> 1]
                texel = packed & 15 if texel_index & 1 else packed >> 4
                if texel == 0:
                    continue
                word = int.from_bytes(
                    title[palette_offset + texel * 2:
                          palette_offset + texel * 2 + 2], "big")
                expected_rgb = pixel_rgb(word)
                visible += 1
                matched += image.getpixel(((x + px) * 2, (y + py) * 2)) == expected_rgb

    if visible != 12518 or matched != visible:
        raise ValueError(
            f"Japanese retail text matches {matched}/{visible} screenshot pixels")
    print(f"ymir_text_pixels={matched}/{visible}")


def verify(title_path: Path, screenshot_path: Path, cc: str) -> None:
    title = title_path.read_bytes()
    digest = hashlib.sha256(title).hexdigest()
    if (len(title) != 112216 or digest != JAPANESE_TITLE_SHA256 or
            digest not in accepted_retail_hashes("TITLE.BIN")):
        raise ValueError("TITLE.BIN is not the authenticated Japanese retail resource")

    root = Path(__file__).resolve().parents[1]
    verify_production_renderer(
        root, title, title_path, screenshot_path.parent, cc,
        source_region="japanese")
    verify_screenshot(title, screenshot_path)
    print("renderer_source=hash-verified Japanese retail TITLE.BIN")
    print("background_and_marker_animation=not_verified")


def verify_cross_region_spans(japanese_path: Path, english_path: Path) -> None:
    japanese = japanese_path.read_bytes()
    english = english_path.read_bytes()
    if (hashlib.sha256(english).hexdigest() != ENGLISH_TITLE_SHA256 or
            len(english) != 112216 or
            hashlib.sha256(japanese).hexdigest() != JAPANESE_TITLE_SHA256):
        raise ValueError("cross-region comparison requires the two admitted retail TITLE.BIN files")

    spans: set[tuple[int, int]] = set()
    for offset, width, height, _x, _y, _name in TEXT_SPRITES:
        spans.add((offset, width * height // 2))
    for offset in ACTIVE_PALETTE_OFFSETS + INACTIVE_PALETTE_OFFSETS:
        spans.add((offset, 32))
    for sprite_offset, palette_offset in MARKER_FRAMES:
        spans.add((sprite_offset, 24 * 24 // 2))
        spans.add((palette_offset, 32))

    for offset, size in sorted(spans):
        if (japanese[offset:offset + size] != english[offset:offset + size] or
                len(japanese[offset:offset + size]) != size):
            raise ValueError(f"regional TITLE.BIN span differs at {offset:#x}")
    if len(spans) != 71:
        raise ValueError(f"unexpected source span count: {len(spans)}")
    print(f"cross_region_identical_spans={len(spans)}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("title_bin", type=Path,
                        help="authenticated Japanese retail TITLE.BIN")
    parser.add_argument("screenshot", type=Path,
                        help="retained authentic Ymir Configuration screenshot")
    parser.add_argument("--cc", default=os.environ.get("CC", "cc"),
                        help="C compiler for the production renderer probe")
    parser.add_argument("--english-title", type=Path,
                        help="optional authenticated English TITLE.BIN for span comparison")
    args = parser.parse_args()
    try:
        verify(args.title_bin, args.screenshot, args.cc)
        if args.english_title:
            verify_cross_region_spans(args.title_bin, args.english_title)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

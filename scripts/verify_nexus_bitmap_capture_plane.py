#!/usr/bin/env python3
"""Compare a raw Nexus VDP2 capture plane with its authenticated Ymir screenshot."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from PIL import Image


WIDTH = 320
HEIGHT = 224
SCREENSHOT_SHA256 = (
    "ffd622503bafd437e4f20513e13b7ea3983f1240c930b755ae4b24986e5c90bf"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("plane", type=Path,
                        help="capture plane: index,R,G,B bytes per pixel")
    parser.add_argument("screenshot", type=Path,
                        help="authentic 640x448 Ymir gameplay screenshot")
    parser.add_argument("--tolerance", type=int, default=8)
    parser.add_argument("--layer", choices=("nbg0", "nbg1", "nbg2"), default="nbg2")
    args = parser.parse_args()
    if not 0 <= args.tolerance <= 255:
        parser.error("tolerance must be 0..255")

    try:
        raw = args.plane.read_bytes()
        if len(raw) != WIDTH * HEIGHT * 4:
            raise ValueError("capture plane has an unexpected byte count")
        screenshot_bytes = args.screenshot.read_bytes()
        actual_hash = hashlib.sha256(screenshot_bytes).hexdigest()
        if actual_hash != SCREENSHOT_SHA256:
            raise ValueError("screenshot does not match the retained Ymir capture")
        with Image.open(args.screenshot) as image:
            image = image.convert("RGB")
            if image.size != (WIDTH * 2, HEIGHT * 2):
                raise ValueError("Ymir screenshot is not 640x448 pixels")
            visible = 0
            matched = 0
            max_error = 0
            for y in range(HEIGHT):
                for x in range(WIDTH):
                    offset = (y * WIDTH + x) * 4
                    index, red, green, blue = raw[offset:offset + 4]
                    if index == 0:
                        continue
                    visible += 1
                    reference = image.getpixel((x * 2, y * 2))
                    error = max(abs(red - reference[0]),
                                abs(green - reference[1]),
                                abs(blue - reference[2]))
                    max_error = max(max_error, error)
                    matched += error <= args.tolerance
        expected_visible, expected_matched = {
            "nbg0": (3968, 3954),
            "nbg1": (148, 148),
            "nbg2": (11232, 9258),
        }[args.layer]
        if visible != expected_visible or matched != expected_matched:
            raise ValueError(
                f"{args.layer.upper()} screenshot evidence changed: "
                f"{matched}/{visible} nontransparent samples within "
                f"{args.tolerance}; expected "
                f"{expected_matched}/{expected_visible} for this capture"
            )
        print(
            f"PASS: {matched}/{visible} {args.layer.upper()} samples within "
            f"{args.tolerance} RGB levels; maximum error {max_error}"
        )
    except (OSError, ValueError) as error:
        parser.exit(1, f"FAIL: {error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

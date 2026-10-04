#!/usr/bin/env python3
"""Embed the exact project-owned Firestaff rail RGB pixels at build time."""

import argparse
import hashlib
from pathlib import Path


WIDTH = 320
HEIGHT = 778
EXPECTED_SHA256 = "f9e5da21319ef238f523938fa5dbcf58c95d888708cade4eb89635067776b5a3"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    pixels = args.input.read_bytes()
    if len(pixels) != WIDTH * HEIGHT * 3:
        raise ValueError(f"Expected {WIDTH * HEIGHT * 3} RGB bytes, got {len(pixels)}")
    digest = hashlib.sha256(pixels).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(f"Firestaff rail RGB checksum mismatch: {digest}")

    lines = [
        '#include "branding_firestaff_rail_m12.h"',
        "",
        "const unsigned char g_m12FirestaffRailRgb[",
        "    M12_FIRESTAFF_RAIL_WIDTH * M12_FIRESTAFF_RAIL_HEIGHT * 3] = {",
    ]
    for offset in range(0, len(pixels), 24):
        lines.append("    " + ", ".join(str(value) for value in pixels[offset:offset + 24]) + ",")
    lines.extend(["};", ""])
    output = "\n".join(lines).encode("ascii")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_bytes() != output:
        args.output.write_bytes(output)


if __name__ == "__main__":
    main()

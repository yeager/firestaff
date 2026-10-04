#!/usr/bin/env python3
"""Embed exact project-owned Firestaff RGB pixels at build time."""

import argparse
import hashlib
from pathlib import Path


ASSETS = {
    "rail": (
        320, 778,
        "f9e5da21319ef238f523938fa5dbcf58c95d888708cade4eb89635067776b5a3",
        "branding_firestaff_rail_m12.h", "g_m12FirestaffRailRgb",
        "M12_FIRESTAFF_RAIL_WIDTH", "M12_FIRESTAFF_RAIL_HEIGHT",
    ),
    "logo": (
        320, 320,
        "d8b16134f3917dee186ef726e0e13062491cd88eb4bf6edd342be28cbbc7ad34",
        "branding_logo_readme_m12.h", "g_m12ReadmeLogoRgb",
        "M12_README_LOGO_WIDTH", "M12_README_LOGO_HEIGHT",
    ),
}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--asset", choices=ASSETS, default="rail")
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    width, height, expected_sha256, header, symbol, width_macro, height_macro = ASSETS[args.asset]

    pixels = args.input.read_bytes()
    if len(pixels) != width * height * 3:
        raise ValueError(f"Expected {width * height * 3} RGB bytes, got {len(pixels)}")
    digest = hashlib.sha256(pixels).hexdigest()
    if digest != expected_sha256:
        raise ValueError(f"Firestaff {args.asset} RGB checksum mismatch: {digest}")

    lines = [
        f'#include "{header}"',
        "",
        f"const unsigned char {symbol}[",
        f"    {width_macro} * {height_macro} * 3] = {{",
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

#!/usr/bin/env python3
"""Embed the exact project-owned DM1, CSB and DM2 launcher card pixels."""

import argparse
import hashlib
from pathlib import Path


CARDS = {
    "dm1": "7874064a272439e98c6857dfedf6c7a97d50b38ce78d237567cb9a67ad82ae62",
    "csb": "f949fcbf8c462d49bcbdeed2dc1f93bc157e5e58311f4436ef5ce03f3a9a8e4f",
    "dm2": "e91816eb245e6322f3ca5ab1fd313d1f2dd4ff0980058a8364e47f281ed33862",
}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    lines = ['#include "card_art_generated_m12.h"', ""]
    for game, expected_digest in CARDS.items():
        path = args.input_dir / f"card_art_{game}_m12.rgb"
        pixels = path.read_bytes()
        if len(pixels) != 180 * 240 * 3:
            raise ValueError(f"Expected 129600 RGB bytes for {game}, got {len(pixels)}")
        digest = hashlib.sha256(pixels).hexdigest()
        if digest != expected_digest:
            raise ValueError(f"Firestaff {game} card RGB checksum mismatch: {digest}")
        lines.append(f"const unsigned char g_{game}_card_art_rgb[129600] = {{")
        for offset in range(0, len(pixels), 24):
            lines.append("    " + ", ".join(str(value) for value in pixels[offset:offset + 24]) + ",")
        lines.extend(["};", ""])

    output = "\n".join(lines).encode("ascii")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_bytes() != output:
        args.output.write_bytes(output)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Convert legacy M11 probe PGM screenshots to RGB PNG with the DM PC VGA palette.

Older m11 probe screenshots wrote each encoded framebuffer byte as
`gray = raw * 17` into an 8-bit PGM.  Since the framebuffer now stores
`raw = colorIndex | (paletteLevel << 4)`, that multiplication wraps and
cannot be interpreted as a simple 0..15 false-color index.

Because 17 is invertible modulo 256, recover raw with gray * 241 mod 256,
then render like render_sdl_m11.c:
  index = raw & 0x0f
  level = raw >> 4 (0 means global/light0 fallback for these probes)
"""
from __future__ import annotations

import sys
from pathlib import Path
from PIL import Image

PALETTE = [
    [(0,0,0),(109,109,109),(146,146,146),(109,36,0),(0,219,219),(146,73,0),(0,146,0),(0,219,0),(255,0,0),(255,182,0),(219,146,109),(255,255,0),(73,73,73),(182,182,182),(0,0,255),(255,255,255)],
    [(0,0,0),(73,73,73),(109,109,109),(109,36,0),(0,219,219),(146,36,0),(0,109,0),(0,182,0),(219,0,0),(219,146,0),(182,109,73),(255,219,0),(36,36,36),(146,146,146),(0,0,219),(219,219,219)],
    [(0,0,0),(36,36,36),(73,73,73),(73,36,0),(0,219,219),(109,36,0),(0,73,0),(0,146,0),(182,0,0),(182,109,0),(146,73,36),(255,182,0),(0,0,0),(109,109,109),(0,0,182),(182,182,182)],
    [(0,0,0),(0,0,0),(36,36,36),(36,0,0),(0,219,219),(73,36,0),(0,36,0),(0,109,0),(146,0,0),(146,73,0),(109,36,0),(219,146,0),(0,0,0),(73,73,73),(0,0,146),(146,146,146)],
    [(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,219,219),(36,0,0),(0,0,0),(0,73,0),(109,0,0),(109,36,0),(73,0,0),(182,109,0),(0,0,0),(36,36,36),(0,0,109),(109,109,109)],
    [(0,0,0),(0,0,0),(0,0,0),(0,0,0),(0,219,219),(0,0,0),(0,0,0),(0,36,0),(73,0,0),(73,0,0),(36,0,0),(109,73,0),(0,0,0),(0,0,0),(0,0,73),(73,73,73)],
]


def convert(src: Path, dst: Path, scale: int = 3, global_level: int = 0) -> None:
    im = Image.open(src).convert("L")
    out = Image.new("RGB", im.size)
    ip = im.load(); op = out.load()
    for y in range(im.height):
        for x in range(im.width):
            raw = (ip[x, y] * 241) & 0xff
            idx = raw & 0x0f
            level = raw >> 4
            if level == 0:
                level = global_level
            if level > 5:
                level = 5
            op[x, y] = PALETTE[level][idx]
    if scale != 1:
        out = out.resize((im.width * scale, im.height * scale), Image.Resampling.NEAREST)
    dst.parent.mkdir(parents=True, exist_ok=True)
    out.save(dst)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} input.pgm [output.png]", file=sys.stderr)
        sys.exit(2)
    src = Path(sys.argv[1])
    dst = Path(sys.argv[2]) if len(sys.argv) > 2 else src.with_suffix(".vga.png")
    convert(src, dst)

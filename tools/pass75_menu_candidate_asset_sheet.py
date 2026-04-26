#!/usr/bin/env python3
"""Build a source-asset sheet for the DM1 startup-menu candidate windows.

Pass 75 turns the pass-68 metric window into a visible, reproducible artifact
from extracted GRAPHICS.DAT PGMs. It still does not place the menu on-screen.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
DEFAULT_PGM_DIR = REPO / "extracted-graphics-v1" / "pgm"
DEFAULT_OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass75"
WINDOWS = {
    "candidate_A_304_319": range(304, 320),
    "candidate_B_left_360_367": range(360, 368),
    "candidate_B_core_368_383": range(368, 384),
    "candidate_B_right_384_391": range(384, 392),
    "candidate_B_wide_360_391": range(360, 392),
}


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def read_pgm(path: Path):
    from PIL import Image
    img = Image.open(path)
    return img.convert("L")


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--pgm-dir", type=Path, default=DEFAULT_PGM_DIR)
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    ap.add_argument("--stats-json", type=Path, default=DEFAULT_OUT_DIR / "pass75_menu_candidate_asset_sheet.json")
    args = ap.parse_args(argv)
    from PIL import Image, ImageDraw

    args.out_dir.mkdir(parents=True, exist_ok=True)
    rows = []
    problems = []
    y = 0
    sheets = []
    for name, indices in WINDOWS.items():
        imgs = []
        serial_w = 0
        max_w = 0
        max_h = 0
        entries = []
        for idx in indices:
            path = args.pgm_dir / f"graphic_{idx:04d}.pgm"
            if not path.exists():
                problems.append(f"missing {path.relative_to(REPO)}")
                continue
            img = read_pgm(path)
            imgs.append((idx, img, path))
            serial_w += img.width
            max_w = max(max_w, img.width)
            max_h = max(max_h, img.height)
            entries.append({"graphic": idx, "width": img.width, "height": img.height, "sha256": sha256_of(path)})
        if not imgs:
            continue
        pad = 2
        label_h = 12
        sheet_w = max(serial_w + pad * (len(imgs) + 1), 320)
        sheet_h = max_h + label_h + pad * 3
        sheet = Image.new("RGB", (sheet_w, sheet_h), (0, 0, 0))
        draw = ImageDraw.Draw(sheet)
        draw.text((pad, pad), name, fill=(255, 255, 255))
        x = pad
        for idx, img, _path in imgs:
            rgb = Image.merge("RGB", (img, img, img))
            sheet.paste(rgb, (x, label_h + pad * 2))
            draw.text((x, sheet_h - 10), str(idx), fill=(255, 255, 0))
            x += img.width + pad
        out_png = args.out_dir / f"{name}.png"
        sheet.save(out_png, optimize=True)
        sheets.append(sheet)
        rows.append({
            "window": name,
            "first": min(i for i, _, _ in imgs),
            "last": max(i for i, _, _ in imgs),
            "count": len(imgs),
            "serial_width": serial_w,
            "max_width": max_w,
            "max_height": max_h,
            "sheet_png": str(out_png.relative_to(REPO)),
            "entries": entries,
        })
        y += sheet_h + 4

    result = {
        "schema": "pass75_menu_candidate_asset_sheet.v1",
        "source": "extracted-graphics-v1/pgm from locked original GRAPHICS.DAT",
        "honesty": "Asset-window sheet only; no 320x200 original menu placement claim.",
        "windows": rows,
        "problems": problems,
        "pass": not problems,
    }
    args.stats_json.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"windows": len(rows), "problems": problems, "stats": str(args.stats_json.relative_to(REPO))}, indent=2))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())

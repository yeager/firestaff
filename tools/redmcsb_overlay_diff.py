#!/usr/bin/env python3
"""
Pass 47 — overlay-diff helper.

Given a Firestaff-captured 320x200 frame (PNG, PPM, or PGM) and a
reference image (typically either the 320x200 composite from
``redmcsb_reference_compose.py`` or an individual anchor graphic
from ``reference-artifacts/anchors/``), produce:

  - a pixel-diff mask PNG (white where identical, red where different)
  - a per-region diff stats JSON (rect + delta pixel count + percent)
  - an optional side-by-side composite PNG with boundary overlay

Usage:

    python3 tools/redmcsb_overlay_diff.py \\
        --firestaff verification-screens/01_ingame_start_latest.png \\
        --reference reference-artifacts/redmcsb_reference_320x200.png \\
        --region viewport \\
        --out parity-evidence/overlays/01_ingame_viewport

The tool does NOT claim parity.  It only produces measurable diff
artifacts.  Interpretation is up to the parity pass that calls it.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Tuple

from PIL import Image

REPO = Path(__file__).resolve().parent.parent

# Regions known from DEFS.H / COORD.C.
REGIONS = {
    "full":            (0,   0, 320, 200),
    "viewport":        (0,  33, 224, 136),
    "side_column":     (224, 0,  96, 200),
    "message_area":    (0, 169, 320,  31),
    "top_strip":       (0,   0, 320,  33),
    # Component anchors whose size comes from DEFS.H (placement is
    # BLOCKED_ON_REFERENCE until ZONES.H is recovered; caller supplies
    # x/y via --region-xywh if running a component overlay).
    "panel_empty_144x73": (0, 0, 144, 73),
    "champion_portraits_256x87": (0, 0, 256, 87),
    "action_area_87x45": (0, 0, 87, 45),
    "spell_area_87x25": (0, 0, 87, 25),
    "spell_area_lines_14x39": (0, 0, 14, 39),
    "slot_box_18x18": (0, 0, 18, 18),
    "status_box_67x29": (0, 0, 67, 29),
}


def load_as_rgb(path: Path) -> Image.Image:
    img = Image.open(path)
    if img.mode != "RGB":
        img = img.convert("RGB")
    return img


def crop(img: Image.Image, rect: Tuple[int, int, int, int]) -> Image.Image:
    x, y, w, h = rect
    return img.crop((x, y, x + w, y + h))


def diff(a: Image.Image, b: Image.Image, tolerance: int = 0) -> Tuple[Image.Image, int, int]:
    """Return (mask_image, differing_pixels, total_pixels).

    Mask: white where identical (within tolerance), red where different.
    """
    if a.size != b.size:
        raise SystemExit(f"size mismatch: firestaff={a.size} reference={b.size}")
    ap = a.load()
    bp = b.load()
    w, h = a.size
    total = w * h
    out = Image.new("RGB", (w, h), (255, 255, 255))
    op = out.load()
    diff_count = 0
    for yy in range(h):
        for xx in range(w):
            ar, ag, ab = ap[xx, yy]
            br, bg, bb = bp[xx, yy]
            if (abs(ar - br) > tolerance or
                abs(ag - bg) > tolerance or
                abs(ab - bb) > tolerance):
                op[xx, yy] = (255, 0, 0)
                diff_count += 1
            else:
                op[xx, yy] = (255, 255, 255)
    return out, diff_count, total


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_region(args) -> Tuple[int, int, int, int]:
    if args.region_xywh:
        parts = [int(p) for p in args.region_xywh.split(",")]
        if len(parts) != 4:
            raise SystemExit("--region-xywh must be x,y,w,h")
        return tuple(parts)  # type: ignore[return-value]
    if args.region not in REGIONS:
        raise SystemExit(f"unknown --region '{args.region}' (known: {sorted(REGIONS)})")
    return REGIONS[args.region]


def main() -> int:
    ap = argparse.ArgumentParser(description="Pass 47 overlay diff (Firestaff vs ReDMCSB reference).")
    ap.add_argument("--firestaff", required=True, type=Path)
    ap.add_argument("--reference", required=True, type=Path)
    ap.add_argument("--region", default="full", choices=sorted(REGIONS))
    ap.add_argument("--region-xywh", default=None,
                    help="override region as x,y,w,h (applied to BOTH frames identically)")
    ap.add_argument("--tolerance", type=int, default=0,
                    help="per-channel delta tolerance before a pixel counts as different")
    ap.add_argument("--out", required=True, type=Path,
                    help="output base path (will get .mask.png and .stats.json)")
    args = ap.parse_args()

    fs = load_as_rgb(args.firestaff)
    rf = load_as_rgb(args.reference)

    rect = parse_region(args)
    fs_crop = crop(fs, rect)
    rf_crop = crop(rf, rect)

    mask, dpx, total = diff(fs_crop, rf_crop, tolerance=args.tolerance)
    out_base = args.out
    out_base.parent.mkdir(parents=True, exist_ok=True)
    mask_path = out_base.with_suffix(".mask.png")
    mask.save(mask_path, format="PNG", optimize=True)

    stats = {
        "pass": 47,
        "tool": "tools/redmcsb_overlay_diff.py",
        "inputs": {
            "firestaff": str(args.firestaff),
            "firestaff_sha256": sha256_of(args.firestaff),
            "reference": str(args.reference),
            "reference_sha256": sha256_of(args.reference),
        },
        "region_xywh": list(rect),
        "tolerance_per_channel": args.tolerance,
        "differing_pixels": dpx,
        "total_pixels": total,
        "delta_percent": round(100.0 * dpx / total, 4) if total else 0.0,
        "mask_path": str(mask_path),
        "honesty": "A non-zero delta does not automatically mean Firestaff is wrong; interpretation must consider whether the reference region has source-anchored content (see reference-artifacts/provenance.json).",
    }
    stats_path = out_base.with_suffix(".stats.json")
    stats_path.write_text(json.dumps(stats, indent=2) + "\n")

    print(f"[pass-47] diff {args.region} ({rect}): {dpx}/{total} pixels differ ({stats['delta_percent']}%)")
    print(f"[pass-47] wrote {mask_path}")
    print(f"[pass-47] wrote {stats_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

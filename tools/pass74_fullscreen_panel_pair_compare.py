#!/usr/bin/env python3
"""Compare original raw 320x200 DM1 screenshots with Firestaff full-frame captures.

Pass 74 is intentionally measurement-only: it validates that both sides are
real 320x200 frames, then records per-region pixel deltas for source-relevant
right-column/action/spell/inventory surfaces. It does not claim parity.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
DEFAULT_FIRESTAFF_DIR = REPO / "verification-screens"
DEFAULT_ORIGINAL_DIR = REPO / "verification-screens" / "pass70-original-dm1-viewports"
DEFAULT_OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass74"
WIDTH = 320
HEIGHT = 200


@dataclass(frozen=True)
class Pairing:
    index: str
    scene: str
    firestaff_png: str
    original_png: str


PAIRINGS = (
    Pairing("01", "ingame_start", "01_ingame_start_latest.ppm", "image0001-raw.png"),
    Pairing("02", "ingame_turn_right", "02_ingame_turn_right_latest.ppm", "image0002-raw.png"),
    Pairing("03", "ingame_move_forward", "03_ingame_move_forward_latest.ppm", "image0003-raw.png"),
    Pairing("04", "ingame_spell_panel", "04_ingame_spell_panel_latest.ppm", "image0004-raw.png"),
    Pairing("05", "ingame_after_cast", "05_ingame_after_cast_latest.ppm", "image0005-raw.png"),
    Pairing("06", "ingame_inventory_panel", "06_ingame_inventory_panel_latest.ppm", "image0006-raw.png"),
)

# Source-relevant regions.  These are not arbitrary screenshots crops: they are
# the current V1 parity surfaces tracked in layout-696 and the parity matrix.
REGIONS = {
    "full_frame": (0, 0, 320, 200),
    "viewport": (0, 33, 224, 136),
    "action_area_C011": (224, 45, 87, 45),
    "spell_area_C013": (224, 90, 87, 25),
    "right_column_action_spell": (224, 45, 87, 70),
    "message_area": (0, 169, 224, 31),
    "inventory_panel_C101_extent": (80, 53, 144, 73),
}


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def diff_region(fs, ref, xywh: tuple[int, int, int, int], tolerance: int) -> dict[str, object]:
    x0, y0, w, h = xywh
    ap = fs.load()
    bp = ref.load()
    differing = 0
    max_delta = 0
    total_abs = 0
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            ar, ag, ab = ap[x, y]
            br, bg, bb = bp[x, y]
            dr, dg, db = abs(ar - br), abs(ag - bg), abs(ab - bb)
            delta = max(dr, dg, db)
            max_delta = max(max_delta, delta)
            total_abs += dr + dg + db
            if dr > tolerance or dg > tolerance or db > tolerance:
                differing += 1
    pixels = w * h
    return {
        "xywh": [x0, y0, w, h],
        "differing_pixels": differing,
        "total_pixels": pixels,
        "delta_percent": round(100.0 * differing / pixels, 4),
        "max_channel_delta": max_delta,
        "mean_abs_delta_rgb": round(total_abs / (pixels * 3), 4),
    }


def make_mask(fs, ref, out_path: Path, tolerance: int) -> None:
    from PIL import Image
    mask = Image.new("RGB", (WIDTH, HEIGHT), (255, 255, 255))
    ap = fs.load(); bp = ref.load(); op = mask.load()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            ar, ag, ab = ap[x, y]
            br, bg, bb = bp[x, y]
            if abs(ar - br) > tolerance or abs(ag - bg) > tolerance or abs(ab - bb) > tolerance:
                op[x, y] = (255, 0, 0)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    mask.save(out_path, format="PNG", optimize=True)


def load_rgb(path: Path):
    from PIL import Image
    img = Image.open(path)
    if img.size != (WIDTH, HEIGHT):
        raise SystemExit(f"{path} is {img.size}, expected {(WIDTH, HEIGHT)}")
    return img.convert("RGB")


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--firestaff-dir", type=Path, default=DEFAULT_FIRESTAFF_DIR)
    ap.add_argument("--original-dir", type=Path, default=DEFAULT_ORIGINAL_DIR)
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    ap.add_argument("--stats-json", type=Path, default=DEFAULT_OUT_DIR / "pass74_fullscreen_panel_compare_stats.json")
    ap.add_argument("--tolerance", type=int, default=0)
    args = ap.parse_args(argv)

    rows: list[dict[str, object]] = []
    problems: list[str] = []
    for pair in PAIRINGS:
        fs_path = args.firestaff_dir / pair.firestaff_png
        ref_path = args.original_dir / pair.original_png
        if not fs_path.exists():
            problems.append(f"missing Firestaff frame: {fs_path.relative_to(REPO)}")
            continue
        if not ref_path.exists():
            problems.append(f"missing original frame: {ref_path.relative_to(REPO)}")
            continue
        fs = load_rgb(fs_path)
        ref = load_rgb(ref_path)
        mask_path = args.out_dir / f"{pair.index}_{pair.scene}_full_frame_mask.png"
        make_mask(fs, ref, mask_path, args.tolerance)
        rows.append({
            "index": pair.index,
            "scene": pair.scene,
            "firestaff": str(fs_path.relative_to(REPO)),
            "firestaff_sha256": sha256_of(fs_path),
            "original": str(ref_path.relative_to(REPO)),
            "original_sha256": sha256_of(ref_path),
            "mask": str(mask_path.relative_to(REPO)),
            "regions": {name: diff_region(fs, ref, xywh, args.tolerance) for name, xywh in REGIONS.items()},
        })

    result = {
        "schema": "pass74_fullscreen_panel_pair_compare.v1",
        "honesty": "Measurement only. Original route and Firestaff route are not claimed semantically identical.",
        "frame_size": [WIDTH, HEIGHT],
        "tolerance_per_channel": args.tolerance,
        "regions": {name: list(xywh) for name, xywh in REGIONS.items()},
        "pairings": rows,
        "problems": problems,
        "pass": not problems,
    }
    args.stats_json.parent.mkdir(parents=True, exist_ok=True)
    args.stats_json.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"pairs": len(rows), "problems": problems, "stats": str(args.stats_json.relative_to(REPO))}, indent=2))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())

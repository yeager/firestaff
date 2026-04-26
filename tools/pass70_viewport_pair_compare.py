#!/usr/bin/env python3
"""Pass 70 viewport crop pairing/compare helper.

This helper is intentionally conservative: it defines the exact six Firestaff
runtime viewport crops and matching original-DM1 crop names, validates manifest
schema and 224x136 geometry, and only runs pixel diffs when both sides exist.
It does not claim parity by itself.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
DEFAULT_FIRESTAFF_DIR = REPO / "verification-screens"
DEFAULT_ORIGINAL_DIR = REPO / "verification-screens" / "pass70-original-dm1-viewports" / "viewport_224x136"
DEFAULT_OUT_DIR = REPO / "parity-evidence" / "overlays" / "pass70"
FIRESTAFF_MANIFEST = REPO / "verification-screens" / "capture_manifest_sha256.tsv"
ORIGINAL_MANIFEST = REPO / "verification-screens" / "pass70-original-dm1-viewports" / "original_viewport_224x136_manifest.tsv"

EXPECTED_COLUMNS = ["kind", "filename", "width", "height", "bytes", "sha256"]
WIDTH = 224
HEIGHT = 136


@dataclass(frozen=True)
class Pairing:
    index: str
    scene: str
    firestaff_ppm: str
    firestaff_png: str
    original_ppm: str
    original_png: str
    overlay_base: str


PAIRINGS: tuple[Pairing, ...] = (
    Pairing("01", "ingame_start", "01_ingame_start_latest_viewport_224x136.ppm", "01_ingame_start_latest_viewport_224x136.png", "01_ingame_start_original_viewport_224x136.ppm", "01_ingame_start_original_viewport_224x136.png", "01_ingame_start_viewport_original_vs_firestaff"),
    Pairing("02", "ingame_turn_right", "02_ingame_turn_right_latest_viewport_224x136.ppm", "02_ingame_turn_right_latest_viewport_224x136.png", "02_ingame_turn_right_original_viewport_224x136.ppm", "02_ingame_turn_right_original_viewport_224x136.png", "02_ingame_turn_right_viewport_original_vs_firestaff"),
    Pairing("03", "ingame_move_forward", "03_ingame_move_forward_latest_viewport_224x136.ppm", "03_ingame_move_forward_latest_viewport_224x136.png", "03_ingame_move_forward_original_viewport_224x136.ppm", "03_ingame_move_forward_original_viewport_224x136.png", "03_ingame_move_forward_viewport_original_vs_firestaff"),
    Pairing("04", "ingame_spell_panel", "04_ingame_spell_panel_latest_viewport_224x136.ppm", "04_ingame_spell_panel_latest_viewport_224x136.png", "04_ingame_spell_panel_original_viewport_224x136.ppm", "04_ingame_spell_panel_original_viewport_224x136.png", "04_ingame_spell_panel_viewport_original_vs_firestaff"),
    Pairing("05", "ingame_after_cast", "05_ingame_after_cast_latest_viewport_224x136.ppm", "05_ingame_after_cast_latest_viewport_224x136.png", "05_ingame_after_cast_original_viewport_224x136.ppm", "05_ingame_after_cast_original_viewport_224x136.png", "05_ingame_after_cast_viewport_original_vs_firestaff"),
    Pairing("06", "ingame_inventory_panel", "06_ingame_inventory_panel_latest_viewport_224x136.ppm", "06_ingame_inventory_panel_latest_viewport_224x136.png", "06_ingame_inventory_panel_original_viewport_224x136.ppm", "06_ingame_inventory_panel_original_viewport_224x136.png", "06_ingame_inventory_panel_viewport_original_vs_firestaff"),
)


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def read_manifest(path: Path) -> dict[str, dict[str, str]]:
    if not path.exists():
        return {}
    rows: dict[str, dict[str, str]] = {}
    data_lines: list[str] = []
    explicit_header = False
    for line in path.read_text().splitlines():
        if not line.strip():
            continue
        if line.startswith("#"):
            if "columns:" in line and "kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256" in line:
                explicit_header = True
            continue
        data_lines.append(line)
    if not data_lines:
        return rows
    first_fields = data_lines[0].split("\t")
    if first_fields == EXPECTED_COLUMNS:
        reader = csv.DictReader(data_lines, delimiter="\t")
        for row in reader:
            rows[row["filename"]] = row
        return rows
    if len(first_fields) == len(EXPECTED_COLUMNS) and explicit_header:
        for fields in csv.reader(data_lines, delimiter="\t"):
            if len(fields) != len(EXPECTED_COLUMNS):
                raise SystemExit(f"manifest row width mismatch for {path}: {fields}")
            row = dict(zip(EXPECTED_COLUMNS, fields))
            rows[row["filename"]] = row
        return rows
    raise SystemExit(f"manifest schema mismatch for {path}: expected {EXPECTED_COLUMNS} header or matching '# columns:' comment, got first row {first_fields}")


def validate_manifest_row(row: dict[str, str] | None, filename: str, kind: str, manifest_path: Path, problems: list[str]) -> None:
    if row is None:
        problems.append(f"missing manifest row in {display_path(manifest_path)}: {filename}")
        return
    if row["kind"] != kind:
        problems.append(f"wrong kind for {filename} in {display_path(manifest_path)}: {row['kind']} != {kind}")
    if row["width"] != str(WIDTH) or row["height"] != str(HEIGHT):
        problems.append(f"wrong geometry for {filename} in {display_path(manifest_path)}: {row['width']}x{row['height']} != {WIDTH}x{HEIGHT}")
    if len(row["sha256"]) != 64 or any(c not in "0123456789abcdefABCDEF" for c in row["sha256"]):
        problems.append(f"invalid sha256 for {filename} in {display_path(manifest_path)}: {row['sha256']}")


def diff_images(firestaff: Path, original: Path, out_base: Path, tolerance: int) -> dict[str, object]:
    from PIL import Image

    def load(path: Path) -> Image.Image:
        img = Image.open(path)
        return img.convert("RGB") if img.mode != "RGB" else img

    fs = load(firestaff)
    ref = load(original)
    if fs.size != (WIDTH, HEIGHT) or ref.size != (WIDTH, HEIGHT):
        raise SystemExit(f"size mismatch for {firestaff.name}/{original.name}: {fs.size} vs {ref.size}; expected {(WIDTH, HEIGHT)}")
    mask = Image.new("RGB", (WIDTH, HEIGHT), (255, 255, 255))
    ap = fs.load(); bp = ref.load(); op = mask.load()
    differing = 0
    for y in range(HEIGHT):
        for x in range(WIDTH):
            ar, ag, ab = ap[x, y]
            br, bg, bb = bp[x, y]
            if abs(ar - br) > tolerance or abs(ag - bg) > tolerance or abs(ab - bb) > tolerance:
                op[x, y] = (255, 0, 0)
                differing += 1
    out_base.parent.mkdir(parents=True, exist_ok=True)
    mask_path = out_base.with_suffix(".mask.png")
    mask.save(mask_path, format="PNG", optimize=True)
    stats = {
        "pass": 70,
        "tool": "tools/pass70_viewport_pair_compare.py",
        "firestaff": str(firestaff),
        "firestaff_sha256": sha256_of(firestaff),
        "original": str(original),
        "original_sha256": sha256_of(original),
        "region_xywh": [0, 0, WIDTH, HEIGHT],
        "tolerance_per_channel": tolerance,
        "differing_pixels": differing,
        "total_pixels": WIDTH * HEIGHT,
        "delta_percent": round(100.0 * differing / (WIDTH * HEIGHT), 4),
        "mask_path": str(mask_path),
        "honesty": "Diff is measurement only; it is not a parity claim without source-backed interpretation of the mismatch.",
    }
    stats_path = out_base.with_suffix(".stats.json")
    stats_path.write_text(json.dumps(stats, indent=2) + "\n")
    return stats


def display_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def command_for(pair: Pairing, firestaff_dir: Path, original_dir: Path, out_dir: Path) -> str:
    return (
        "python3 tools/redmcsb_overlay_diff.py "
        f"--firestaff {display_path(firestaff_dir / pair.firestaff_png)} "
        f"--reference {display_path(original_dir / pair.original_png)} "
        "--region-xywh 0,0,224,136 "
        f"--out {display_path(out_dir / pair.overlay_base)}"
    )


def build_plan(args: argparse.Namespace) -> tuple[list[dict[str, object]], list[str]]:
    firestaff_dir = args.firestaff_dir
    original_dir = args.original_dir
    fs_manifest = read_manifest(args.firestaff_manifest)
    orig_manifest = read_manifest(args.original_manifest)
    problems: list[str] = []
    plan: list[dict[str, object]] = []
    for pair in PAIRINGS:
        fs_ppm = firestaff_dir / pair.firestaff_ppm
        fs_png = firestaff_dir / pair.firestaff_png
        orig_ppm = original_dir / pair.original_ppm
        orig_png = original_dir / pair.original_png
        if fs_manifest:
            validate_manifest_row(fs_manifest.get(pair.firestaff_ppm), pair.firestaff_ppm, "viewport_224x136", args.firestaff_manifest, problems)
        else:
            problems.append(f"missing Firestaff manifest: {display_path(args.firestaff_manifest)}")
        if orig_manifest:
            validate_manifest_row(orig_manifest.get(pair.original_ppm), pair.original_ppm, "original_viewport_224x136", args.original_manifest, problems)
        else:
            problems.append(f"missing original manifest: {display_path(args.original_manifest)}")
        plan.append({
            "index": pair.index,
            "scene": pair.scene,
            "firestaff_ppm": display_path(fs_ppm),
            "firestaff_png": display_path(fs_png),
            "firestaff_exists": fs_ppm.exists() or fs_png.exists(),
            "original_ppm": display_path(orig_ppm),
            "original_png": display_path(orig_png),
            "original_exists": orig_ppm.exists() or orig_png.exists(),
            "overlay_base": display_path(args.out_dir / pair.overlay_base),
            "redmcsb_overlay_diff_command": command_for(pair, firestaff_dir, original_dir, args.out_dir),
        })
    return plan, sorted(set(problems))


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Define, validate, and optionally diff Pass 70 viewport crop pairings.")
    ap.add_argument("--firestaff-dir", type=Path, default=DEFAULT_FIRESTAFF_DIR)
    ap.add_argument("--original-dir", type=Path, default=DEFAULT_ORIGINAL_DIR)
    ap.add_argument("--firestaff-manifest", type=Path, default=FIRESTAFF_MANIFEST)
    ap.add_argument("--original-manifest", type=Path, default=ORIGINAL_MANIFEST)
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    ap.add_argument("--plan-json", type=Path, default=None, help="write inventory/pairing plan JSON")
    ap.add_argument("--run-diff", action="store_true", help="run diffs; requires all six crop pairs to exist")
    ap.add_argument("--tolerance", type=int, default=0)
    args = ap.parse_args(argv)

    plan, problems = build_plan(args)
    all_pairs_exist = all(bool(row["firestaff_exists"] and row["original_exists"]) for row in plan)
    result: dict[str, object] = {
        "schema": "pass70_viewport_pair_compare.v1",
        "viewport_xywh": [0, 33, WIDTH, HEIGHT],
        "crop_xywh_for_224x136_inputs": [0, 0, WIDTH, HEIGHT],
        "firestaff_manifest_schema": EXPECTED_COLUMNS,
        "original_manifest_schema": EXPECTED_COLUMNS,
        "pairings": plan,
        "blockers": problems + ([] if all_pairs_exist else ["not all six Firestaff/original viewport crop pairs are present"]),
        "parity_claimed": False,
    }

    if args.plan_json:
        args.plan_json.parent.mkdir(parents=True, exist_ok=True)
        args.plan_json.write_text(json.dumps(result, indent=2) + "\n")
        print(f"[pass-70] wrote plan: {args.plan_json}")

    if args.run_diff:
        if result["blockers"]:
            for blocker in result["blockers"]: print(f"[blocked] {blocker}", file=sys.stderr)
            return 2
        stats = []
        for row, pair in zip(plan, PAIRINGS):
            fs = Path(row["firestaff_png"] if Path(str(row["firestaff_png"])).exists() else row["firestaff_ppm"])
            orig = Path(row["original_png"] if Path(str(row["original_png"])).exists() else row["original_ppm"])
            stats.append(diff_images(fs, orig, args.out_dir / pair.overlay_base, args.tolerance))
        worst = max(stats, key=lambda s: int(s["differing_pixels"]))
        print(json.dumps({"diffs": stats, "worst": worst}, indent=2))
        return 0

    for row in plan:
        print(f"{row['index']} {row['scene']}: firestaff={'yes' if row['firestaff_exists'] else 'no'} original={'yes' if row['original_exists'] else 'no'}")
    for blocker in result["blockers"]:
        print(f"[blocked] {blocker}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

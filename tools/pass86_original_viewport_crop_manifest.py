#!/usr/bin/env python3
"""Pass 86 original-runtime viewport crop/manifest builder.

Given a directory of raw DOSBox Staging original DM1 screenshots
(`imageNNNN-raw.png` or `imageNNNN.png`), classify the frames with the pass-80
layout heuristic, require the intended six-scene route by default, crop the
DM1 viewport (`0,33,224,136`), and emit deterministic pass-70-compatible
original viewport artifacts plus a checksum manifest.

Evidence tooling only: this script prepares original-reference inputs for
DM1/V1 overlay comparisons. It does not capture gameplay, alter gameplay, or
claim pixel parity.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
import tempfile
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
PASS80_PATH = REPO / "tools" / "pass80_original_frame_classifier.py"
DEFAULT_OUT_DIR = REPO / "verification-screens" / "pass86-original-dm1-viewports"
VIEWPORT_XYWH = (0, 33, 224, 136)
SCENES: tuple[tuple[str, str, str], ...] = (
    ("01", "ingame_start", "dungeon_gameplay"),
    ("02", "ingame_turn_right", "dungeon_gameplay"),
    ("03", "ingame_move_forward", "dungeon_gameplay"),
    ("04", "ingame_spell_panel", "spell_panel"),
    ("05", "ingame_after_cast", "dungeon_gameplay"),
    ("06", "ingame_inventory_panel", "inventory"),
)
MANIFEST_COLUMNS = ("kind", "filename", "width", "height", "bytes", "sha256")


def _load_pass80():
    spec = importlib.util.spec_from_file_location("pass80_classifier", PASS80_PATH)
    if spec is None or spec.loader is None:
        raise SystemExit(f"cannot import {PASS80_PATH.relative_to(REPO)}")
    mod = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = mod
    spec.loader.exec_module(mod)
    return mod


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def raw_paths(raw_dir: Path) -> list[Path]:
    paths = sorted(raw_dir.glob("image*.png"))
    # Prefer raw DOSBox exports when both imageNNNN.png and imageNNNN-raw.png exist.
    raw_named = [p for p in paths if "-raw" in p.stem]
    return raw_named or paths


def classify_path(pass80, path: Path) -> tuple[str, str, tuple[int, int]]:
    dims = pass80.png_dims(path)
    if dims != (pass80.WIDTH, pass80.HEIGHT):
        return "non_graphics_blocker", f"raw dimensions are {dims[0]}x{dims[1]}, not 320x200", dims
    img = pass80.load_rgb(path)
    regions = {name: pass80.stats_for(img, xywh) for name, xywh in pass80.REGIONS.items()}
    label, reason = pass80.classify(regions, dims)
    return label, reason, dims


def manifest_row(kind: str, path: Path, filename: str, width: int, height: int) -> str:
    return "\t".join([kind, filename, str(width), str(height), str(path.stat().st_size), sha256(path)])


def write_manifest(path: Path, rows: list[str]) -> None:
    path.write_text(
        "# Pass 86 original DM1 viewport crop manifest\n"
        "# columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256\n"
        + "\n".join(rows)
        + ("\n" if rows else "")
    )


def write_markdown(path: Path, result: dict[str, object]) -> None:
    lines = [
        "# Pass 86 — original DM1 viewport crop manifest",
        "",
        "This pass converts a semantically checked original-runtime screenshot route into pass-70-compatible viewport crops.",
        "",
        f"- raw dir: `{result['raw_dir']}`",
        f"- output dir: `{result['out_dir']}`",
        f"- manifest: `{result['manifest']}`",
        f"- viewport crop: `{result['viewport_xywh']}` (x, y, width, height)",
        f"- pass: `{result['pass']}`",
        "- honesty: original-reference input preparation only; no pixel parity is claimed.",
        "",
    ]
    if result.get("problems"):
        lines.extend(["## Problems", ""])
        for problem in result["problems"]:  # type: ignore[index]
            lines.append(f"- {problem}")
        lines.append("")
    lines.extend([
        "## Frames",
        "",
        "| scene | raw capture | classified | expected | output PNG | output PPM | sha256 |",
        "|-------|-------------|------------|----------|------------|------------|--------|",
    ])
    for row in result["frames"]:  # type: ignore[index]
        lines.append(
            f"| `{row['scene']}` | `{row['raw']}` | `{row['classification']}` | `{row['expected_class']}` | "
            f"`{row.get('png', '')}` | `{row.get('ppm', '')}` | `{str(row.get('ppm_sha256', ''))[:12]}` |"
        )
    path.write_text("\n".join(lines).rstrip() + "\n")


def build(raw_dir: Path, out_dir: Path, allow_mismatch: bool, dry_run: bool) -> dict[str, object]:
    pass80 = _load_pass80()
    paths = raw_paths(raw_dir)
    problems: list[str] = []
    frames: list[dict[str, object]] = []
    if len(paths) != len(SCENES):
        problems.append(f"expected {len(SCENES)} raw captures for the canonical route, found {len(paths)}")

    manifest_rows: list[str] = []
    viewport_dir = out_dir / "viewport_224x136"
    if not dry_run:
        viewport_dir.mkdir(parents=True, exist_ok=True)

    for i, (idx, scene, expected_class) in enumerate(SCENES):
        if i >= len(paths):
            frames.append({"index": idx, "scene": scene, "expected_class": expected_class, "problem": "missing raw capture"})
            continue
        raw = paths[i]
        label, reason, dims = classify_path(pass80, raw)
        ok = label == expected_class
        if not ok:
            problems.append(f"{raw.name}: classified {label}, expected {expected_class}")
        png_name = f"{idx}_{scene}_original_viewport_224x136.png"
        ppm_name = f"{idx}_{scene}_original_viewport_224x136.ppm"
        row: dict[str, object] = {
            "index": idx,
            "scene": scene,
            "raw": display(raw),
            "raw_sha256": sha256(raw),
            "width": dims[0],
            "height": dims[1],
            "classification": label,
            "classification_reason": reason,
            "expected_class": expected_class,
            "expected_match": ok,
        }
        if ok or allow_mismatch:
            row["png"] = display(viewport_dir / png_name)
            row["ppm"] = display(viewport_dir / ppm_name)
            if not dry_run:
                img = pass80.load_rgb(raw)
                x, y, w, h = VIEWPORT_XYWH
                crop = img.crop((x, y, x + w, y + h))
                png_path = viewport_dir / png_name
                ppm_path = viewport_dir / ppm_name
                crop.save(png_path, format="PNG", optimize=True)
                crop.save(ppm_path, format="PPM")
                row["png_sha256"] = sha256(png_path)
                row["ppm_sha256"] = sha256(ppm_path)
                manifest_rows.append(manifest_row("original_viewport_224x136", ppm_path, ppm_name, 224, 136))
        frames.append(row)

    manifest = out_dir / "original_viewport_224x136_manifest.tsv"
    md = out_dir / "pass86_original_viewport_crop_manifest.md"
    stats = out_dir / "pass86_original_viewport_crop_manifest.json"
    result: dict[str, object] = {
        "schema": "pass86_original_viewport_crop_manifest.v1",
        "honesty": "Original-reference input preparation only; no pixel parity is claimed.",
        "raw_dir": display(raw_dir),
        "out_dir": display(out_dir),
        "viewport_xywh": list(VIEWPORT_XYWH),
        "expected_sequence": [{"index": i, "scene": s, "class": c} for i, s, c in SCENES],
        "frames": frames,
        "manifest": display(manifest),
        "markdown": display(md),
        "stats": display(stats),
        "dry_run": dry_run,
        "problems": problems,
        "pass": bool(frames) and not problems,
    }
    if not dry_run:
        out_dir.mkdir(parents=True, exist_ok=True)
        write_manifest(manifest, manifest_rows)
        stats.write_text(json.dumps(result, indent=2) + "\n")
        write_markdown(md, result)
    return result


def run_self_test() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        out = tmp_path / "out"
        rows = []
        sample = out / "viewport_224x136" / "01_ingame_start_original_viewport_224x136.ppm"
        sample.parent.mkdir(parents=True)
        sample.write_bytes(b"P6\n1 1\n255\n\x00\x00\x00")
        rows.append(manifest_row("original_viewport_224x136", sample, sample.name, 1, 1))
        manifest = out / "original_viewport_224x136_manifest.tsv"
        write_manifest(manifest, rows)
        text = manifest.read_text()
        checks = [
            "columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256" in text,
            sample.name in text,
            len(text.splitlines()[-1].split("\t")[-1]) == 64,
        ]
    print(json.dumps({"pass": all(checks), "checks": checks}, indent=2))
    return 0 if all(checks) else 1


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Build pass-70-compatible viewport crops from original DM1 raw screenshots.")
    ap.add_argument("raw_dir", type=Path, nargs="?", help="directory containing imageNNNN-raw.png original screenshots")
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    ap.add_argument("--allow-mismatch", action="store_true", help="write crops even when pass-80 semantic labels do not match the canonical route")
    ap.add_argument("--dry-run", action="store_true", help="classify and plan only; do not write artifacts")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        return run_self_test()
    if args.raw_dir is None:
        ap.error("raw_dir is required unless --self-test is used")
    result = build(args.raw_dir, args.out_dir, args.allow_mismatch, args.dry_run)
    print(json.dumps({
        "frames": len(result["frames"]),
        "problems": result["problems"],
        "manifest": result["manifest"],
        "stats": result["stats"],
        "dry_run": result["dry_run"],
    }, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

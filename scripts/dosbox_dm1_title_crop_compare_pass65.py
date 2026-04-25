#!/usr/bin/env python3
"""Pass 65 bridge: crop DOSBox window TITLE captures to 320x200 and compare.

This intentionally consumes the clean targeted window captures produced by
scripts/dosbox_dm1_title_clean_capture_pass64.sh.  It does not claim a raw
framebuffer capture; it derives a reproducible 320x200 image from the DOSBox
window capture and records the remaining limits.
"""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path
from typing import Iterable

from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
PASS64_DIR = ROOT / "verification-screens" / "pass64-clean-title-capture"
OUT_DIR = ROOT / "verification-screens" / "pass65-title-crop-compare"
REF_DIR = ROOT / "verification" / "pass58-title-render-dump"


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_scaled_size(log_path: Path) -> tuple[int, int]:
    text = log_path.read_text(errors="replace")
    matches = re.findall(r"graphics mode 13h.*?scaled to (\d+)x(\d+) pixels", text)
    if not matches:
        matches = re.findall(r"scaled to (\d+)x(\d+) pixels", text)
    if not matches:
        raise SystemExit(f"could not find DOSBox scaled size in {log_path}")
    w, h = matches[-1]
    return int(w), int(h)


def load_refs(ref_dir: Path) -> list[tuple[Path, np.ndarray]]:
    refs: list[tuple[Path, np.ndarray]] = []
    for path in sorted(ref_dir.glob("frame_*.ppm")):
        refs.append((path, np.asarray(Image.open(path).convert("RGB"), dtype=np.int16)))
    if not refs:
        raise SystemExit(f"no rendered TITLE reference frames found in {ref_dir}")
    return refs


def normalized(arr: np.ndarray) -> np.ndarray:
    out = arr.copy()
    # DOSBox's window renderer stores the title-card black as near-black (12,12,12)
    # in this capture path.  Normalize only the near-black background before metrics.
    out[np.max(out, axis=2) < 16] = 0
    return out


def metrics(a: np.ndarray, b: np.ndarray) -> tuple[float, int, int]:
    diff = np.abs(a.astype(np.int16) - b.astype(np.int16))
    mae = float(np.mean(diff))
    differing_pixels = int(np.count_nonzero(np.any(diff != 0, axis=2)))
    max_delta = int(np.max(diff))
    return mae, differing_pixels, max_delta


def best_y_for_capture(win: Image.Image, x: int, scaled_w: int, scaled_h: int, refs: list[tuple[Path, np.ndarray]]) -> tuple[int, Path, float, int, int]:
    best: tuple[float, int, int, int, Path] | None = None
    max_y = win.height - scaled_h
    for y in range(0, max_y + 1):
        crop = win.crop((x, y, x + scaled_w, y + scaled_h)).resize((320, 200), Image.Resampling.NEAREST)
        arr = normalized(np.asarray(crop.convert("RGB"), dtype=np.int16))
        for ref_path, ref in refs:
            mae, differing, max_delta = metrics(arr, ref)
            candidate = (mae, differing, max_delta, y, ref_path)
            if best is None or candidate < best:
                best = candidate
    assert best is not None
    mae, differing, max_delta, y, ref_path = best
    return y, ref_path, mae, differing, max_delta


def write_ppm(path: Path, arr: np.ndarray) -> None:
    Image.fromarray(arr.astype(np.uint8)).save(path)


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--capture-dir", type=Path, default=PASS64_DIR)
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR)
    ap.add_argument("--ref-dir", type=Path, default=REF_DIR)
    args = ap.parse_args(argv)

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    scaled_w, scaled_h = parse_scaled_size(args.capture_dir / "dosbox-title-clean.log")
    refs = load_refs(args.ref_dir)
    captures = sorted(args.capture_dir.glob("title-clean-window-*.png"))
    if not captures:
        raise SystemExit(f"no title-clean-window PNG captures found in {args.capture_dir}")

    report_lines: list[str] = []
    report_lines.append("# Pass 65 TITLE crop/compare run")
    report_lines.append("")
    report_lines.append(f"Input directory: `{args.capture_dir}`")
    report_lines.append(f"Output directory: `{out_dir}`")
    report_lines.append(f"DOSBox log scaled surface: `{scaled_w}x{scaled_h}`")
    report_lines.append(f"Rendered reference frames: `{len(refs)}` from `{args.ref_dir}`")
    report_lines.append("")
    report_lines.append("## Crops")
    report_lines.append("")
    report_lines.append("| Capture | Window size | Crop x,y,w,h | Best reference | MAE | Diff pixels | Max delta | SHA-256 cropped PPM |")
    report_lines.append("|---|---:|---:|---|---:|---:|---:|---|")

    normalized_arrays: list[np.ndarray] = []
    crop_hashes: list[str] = []
    best_refs: list[str] = []

    for capture in captures:
        win = Image.open(capture).convert("RGB")
        x = (win.width - scaled_w) // 2
        if x < 0:
            raise SystemExit(f"scaled width {scaled_w} exceeds {capture} width {win.width}")
        y, ref_path, mae, differing, max_delta = best_y_for_capture(win, x, scaled_w, scaled_h, refs)
        crop = win.crop((x, y, x + scaled_w, y + scaled_h)).resize((320, 200), Image.Resampling.NEAREST)
        arr = normalized(np.asarray(crop.convert("RGB"), dtype=np.int16)).astype(np.uint8)
        out_ppm = out_dir / f"{capture.stem}-cropped-320x200.ppm"
        out_png = out_dir / f"{capture.stem}-cropped-320x200.png"
        write_ppm(out_ppm, arr)
        Image.fromarray(arr).save(out_png)
        digest = sha256(out_ppm)
        crop_hashes.append(digest)
        best_refs.append(ref_path.name)
        normalized_arrays.append(arr)
        report_lines.append(
            f"| `{capture.name}` | `{win.width}x{win.height}` | `{x},{y},{scaled_w},{scaled_h}` | `{ref_path.name}` | {mae:.6f} | {differing} | {max_delta} | `{digest}` |"
        )

    report_lines.append("")
    report_lines.append("## Sequence check")
    report_lines.append("")
    if len(normalized_arrays) >= 2:
        for i in range(len(normalized_arrays) - 1):
            mae, differing, max_delta = metrics(normalized_arrays[i], normalized_arrays[i + 1])
            report_lines.append(
                f"- Cropped frame {i:02d} vs {i+1:02d}: MAE {mae:.6f}, differing pixels {differing}, max delta {max_delta}."
            )
    else:
        report_lines.append("- Only one cropped frame was available.")

    if len(set(crop_hashes)) == 1:
        report_lines.append("- Cropped frames are byte-identical after deterministic crop/downscale/near-black normalization.")
    else:
        report_lines.append("- Cropped frames are not byte-identical; cadence measurement can proceed only with capture timestamps.")

    report_lines.append("")
    report_lines.append("## Cadence status")
    report_lines.append("")
    report_lines.append(
        "Cadence measurement is not started from this run: pass-64 captured two identical frames and did not record per-frame capture timestamps. The next bounded step is a timestamped targeted-window sequence, or native/raw DOSBox screenshots if that path can be made to emit files in the detached run."
    )
    report_lines.append("")

    report_path = out_dir / "pass65_crop_compare_report.md"
    report_path.write_text("\n".join(report_lines) + "\n")
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

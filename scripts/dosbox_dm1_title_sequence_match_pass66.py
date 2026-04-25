#!/usr/bin/env python3
"""Pass 66 bridge: crop timestamped DOSBox TITLE window sequence and match frames.

Consumes the manifest/stills produced by
scripts/dosbox_dm1_title_sequence_capture_pass66.sh, reuses the pass-65 crop
alignment approach, and records timestamped best-match evidence against the
source-backed Firestaff/Greatstone TITLE render dump.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
CAPTURE_DIR = ROOT / "verification-screens" / "pass66-title-timestamp-sequence"
OUT_DIR = ROOT / "verification-screens" / "pass66-title-timestamp-sequence-match"
REF_DIR = ROOT / "verification" / "pass58-title-render-dump"


@dataclass(frozen=True)
class ManifestRow:
    index: str
    path: Path
    before_epoch_ns: int
    after_epoch_ns: int
    before_iso: str
    after_iso: str
    capture_sha: str
    size_bytes: int


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


def load_manifest(path: Path) -> list[ManifestRow]:
    rows: list[ManifestRow] = []
    with path.open(newline="") as f:
        reader = csv.DictReader(f, delimiter="\t")
        for row in reader:
            image_path = Path(row["path"])
            if not image_path.is_absolute():
                image_path = path.parent / image_path
            rows.append(
                ManifestRow(
                    index=row["index"],
                    path=image_path,
                    before_epoch_ns=int(row["before_epoch_ns"]),
                    after_epoch_ns=int(row["after_epoch_ns"]),
                    before_iso=row["before_iso"],
                    after_iso=row["after_iso"],
                    capture_sha=row["sha256"],
                    size_bytes=int(row["size_bytes"]),
                )
            )
    if not rows:
        raise SystemExit(f"empty manifest: {path}")
    return rows


def normalized(arr: np.ndarray) -> np.ndarray:
    out = arr.copy()
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


def format_ms(delta_ns: int) -> str:
    return f"{delta_ns / 1_000_000:.3f}"


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--capture-dir", type=Path, default=CAPTURE_DIR)
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR)
    ap.add_argument("--ref-dir", type=Path, default=REF_DIR)
    ap.add_argument("--keep-crops", action="store_true", help="also keep PNG crops next to compact PPM evidence")
    args = ap.parse_args(argv)

    manifest_path = args.capture_dir / "manifest.tsv"
    log_path = args.capture_dir / "dosbox-title-sequence.log"
    scaled_w, scaled_h = parse_scaled_size(log_path)
    refs = load_refs(args.ref_dir)
    rows = load_manifest(manifest_path)

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    report_lines: list[str] = []
    report_lines.append("# Pass 66 TITLE timestamp sequence match run")
    report_lines.append("")
    report_lines.append(f"Input directory: `{args.capture_dir}`")
    report_lines.append(f"Output directory: `{out_dir}`")
    report_lines.append(f"Manifest: `{manifest_path}`")
    report_lines.append(f"DOSBox log scaled surface: `{scaled_w}x{scaled_h}`")
    report_lines.append(f"Rendered reference frames: `{len(refs)}` from `{args.ref_dir}`")
    report_lines.append("")
    report_lines.append("## Timestamped crops/matches")
    report_lines.append("")
    report_lines.append("| # | Before UTC | After UTC | Since first start ms | Capture duration ms | Window SHA-256 | Crop SHA-256 | Crop x,y,w,h | Best reference | MAE | Diff pixels | Max delta | TITLE-quality |")
    report_lines.append("|---:|---|---|---:|---:|---|---|---:|---|---:|---:|---:|---|")

    normalized_arrays: list[np.ndarray] = []
    crop_hashes: list[str] = []
    best_refs: list[str] = []
    title_quality: list[bool] = []
    first_ns = rows[0].before_epoch_ns

    for row in rows:
        if not row.path.exists():
            raise SystemExit(f"manifest image missing: {row.path}")
        actual_capture_sha = sha256(row.path)
        if actual_capture_sha != row.capture_sha:
            raise SystemExit(f"capture sha mismatch for {row.path}: manifest {row.capture_sha}, actual {actual_capture_sha}")
        win = Image.open(row.path).convert("RGB")
        x = (win.width - scaled_w) // 2
        if x < 0:
            raise SystemExit(f"scaled width {scaled_w} exceeds {row.path} width {win.width}")
        y, ref_path, mae, differing, max_delta = best_y_for_capture(win, x, scaled_w, scaled_h, refs)
        crop = win.crop((x, y, x + scaled_w, y + scaled_h)).resize((320, 200), Image.Resampling.NEAREST)
        arr = normalized(np.asarray(crop.convert("RGB"), dtype=np.int16)).astype(np.uint8)
        out_ppm = out_dir / f"{row.path.stem}-cropped-320x200.ppm"
        write_ppm(out_ppm, arr)
        if args.keep_crops:
            Image.fromarray(arr).save(out_dir / f"{row.path.stem}-cropped-320x200.png")
        digest = sha256(out_ppm)
        crop_hashes.append(digest)
        best_refs.append(ref_path.name)
        normalized_arrays.append(arr)
        # Pass 65 established that clean TITLE-window evidence matches the
        # source-backed frame dump at about MAE 1.69 / max delta 12.  Larger
        # residuals are still logged, but should be treated as post-TITLE/menu
        # or capture-transition frames rather than TITLE cadence evidence.
        good_title_match = mae <= 2.0 and max_delta <= 12
        title_quality.append(good_title_match)
        quality = "TITLE-like" if good_title_match else "transition/non-TITLE"
        report_lines.append(
            f"| {row.index} | `{row.before_iso}` | `{row.after_iso}` | {format_ms(row.before_epoch_ns - first_ns)} | {format_ms(row.after_epoch_ns - row.before_epoch_ns)} | `{actual_capture_sha}` | `{digest}` | `{x},{y},{scaled_w},{scaled_h}` | `{ref_path.name}` | {mae:.6f} | {differing} | {max_delta} | {quality} |"
        )

    report_lines.append("")
    report_lines.append("## Adjacent-frame deltas")
    report_lines.append("")
    if len(normalized_arrays) >= 2:
        for i in range(len(normalized_arrays) - 1):
            mae, differing, max_delta = metrics(normalized_arrays[i], normalized_arrays[i + 1])
            delta_ns = rows[i + 1].before_epoch_ns - rows[i].before_epoch_ns
            report_lines.append(
                f"- {rows[i].index} -> {rows[i+1].index}: start delta {format_ms(delta_ns)} ms; crop MAE {mae:.6f}, differing pixels {differing}, max delta {max_delta}; refs `{best_refs[i]}` -> `{best_refs[i+1]}`."
            )
    else:
        report_lines.append("- Only one cropped frame was available.")

    report_lines.append("")
    report_lines.append("## Sequence status")
    report_lines.append("")
    unique_windows = len({r.capture_sha for r in rows})
    unique_crops = len(set(crop_hashes))
    unique_refs = len(set(best_refs))
    good_refs = [ref for ref, good in zip(best_refs, title_quality) if good]
    good_crop_hashes = [digest for digest, good in zip(crop_hashes, title_quality) if good]
    report_lines.append(f"- Window captures: {len(rows)} total, {unique_windows} unique SHA-256 values.")
    report_lines.append(f"- Cropped normalized frames: {unique_crops} unique SHA-256 values.")
    report_lines.append(f"- Best source-backed reference frames: {unique_refs} unique (`{', '.join(sorted(set(best_refs)))}`).")
    report_lines.append(f"- TITLE-like matches at pass-65 tolerance: {len(good_refs)} frames, {len(set(good_crop_hashes))} unique crops, {len(set(good_refs))} unique best references (`{', '.join(sorted(set(good_refs))) if good_refs else 'none'}`).")
    if not good_refs:
        report_lines.append("- Cadence remains unmeasured: no captured frame matched the source-backed TITLE dump within the pass-65 tolerance envelope.")
    elif len(set(good_crop_hashes)) == 1:
        report_lines.append("- The TITLE-like portion of the timestamped targeted-window sequence is still a still: every normalized TITLE crop is byte-identical.")
        report_lines.append("- Cadence remains unmeasured because this host-window path is sampling after the original TITLE animation has already reached a stable visible frame. The next route needs earlier/continuous native DOSBox image capture or a launch-time sequence with shorter post-selector delay and verified changing frames.")
    else:
        report_lines.append("- The TITLE-like portion contains distinct low-residual captures, followed by high-residual transition/non-TITLE frames. This supports only a conservative observation that the targeted-window path can see changes; it is not a final cadence claim because host screencapture is sparse/slow and the later frames no longer match the TITLE dump within tolerance.")
    report_lines.append("")

    report_path = out_dir / "pass66_sequence_match_report.md"
    report_path.write_text("\n".join(report_lines) + "\n")
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

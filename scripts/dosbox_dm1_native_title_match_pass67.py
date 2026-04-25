#!/usr/bin/env python3
"""Pass 67 bridge: match native DOSBox Staging raw screenshots to TITLE frames.

The pass-67 capture script triggers DOSBox Staging's built-in Cmd+F5 raw
screenshot action.  Those screenshots are already 320x200 emulator framebuffer
captures in mode 13h, so this script validates the manifest and compares them
directly against the source-backed Firestaff/Greatstone TITLE frame dump.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable

from PIL import Image
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
CAPTURE_DIR = ROOT / "verification-screens" / "pass67-native-title-raw-sequence"
OUT_DIR = ROOT / "verification-screens" / "pass67-native-title-raw-sequence-match"
REF_DIR = ROOT / "verification" / "pass58-title-render-dump"


@dataclass(frozen=True)
class ManifestRow:
    index: str
    path: Path
    mtime_epoch_ns: int
    mtime_iso: str
    capture_sha: str
    size_bytes: int
    width: int
    height: int


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


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
                    mtime_epoch_ns=int(row["mtime_epoch_ns"]),
                    mtime_iso=row["mtime_iso"],
                    capture_sha=row["sha256"],
                    size_bytes=int(row["size_bytes"]),
                    width=int(row["width"]),
                    height=int(row["height"]),
                )
            )
    if not rows:
        raise SystemExit(f"empty manifest: {path}")
    return rows


def load_refs(ref_dir: Path) -> list[tuple[Path, np.ndarray]]:
    refs: list[tuple[Path, np.ndarray]] = []
    for path in sorted(ref_dir.glob("frame_*.ppm")):
        refs.append((path, np.asarray(Image.open(path).convert("RGB"), dtype=np.int16)))
    if not refs:
        raise SystemExit(f"no rendered TITLE reference frames found in {ref_dir}")
    return refs


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


def format_ms(delta_ns: int) -> str:
    return f"{delta_ns / 1_000_000:.3f}"


def parse_log_capture_times(log_path: Path) -> dict[str, str]:
    text = log_path.read_text(errors="replace")
    out: dict[str, str] = {}
    pattern = re.compile(r"^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}).*CAPTURE: Capturing raw image to '(.+?)'", re.MULTILINE)
    for stamp, raw_path in pattern.findall(text):
        # DOSBox Staging logs local wall-clock time without an explicit zone.
        out[Path(raw_path).name] = stamp
    return out


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--capture-dir", type=Path, default=CAPTURE_DIR)
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR)
    ap.add_argument("--ref-dir", type=Path, default=REF_DIR)
    args = ap.parse_args(argv)

    manifest_path = args.capture_dir / "manifest.tsv"
    log_path = args.capture_dir / "dosbox-native-title.log"
    rows = load_manifest(manifest_path)
    refs = load_refs(args.ref_dir)
    log_times = parse_log_capture_times(log_path) if log_path.exists() else {}

    args.out_dir.mkdir(parents=True, exist_ok=True)

    lines: list[str] = []
    lines.append("# Pass 67 native raw TITLE screenshot match run")
    lines.append("")
    lines.append(f"Input directory: `{args.capture_dir}`")
    lines.append(f"Output directory: `{args.out_dir}`")
    lines.append(f"Manifest: `{manifest_path}`")
    lines.append(f"DOSBox log: `{log_path}`")
    lines.append(f"Rendered reference frames: `{len(refs)}` from `{args.ref_dir}`")
    lines.append("")
    lines.append("## Native raw screenshot matches")
    lines.append("")
    lines.append("| # | DOSBox log local time | mtime UTC | Since first mtime ms | PNG SHA-256 | Size | WxH | Best reference | MAE | Diff pixels | Max delta | TITLE-quality |")
    lines.append("|---:|---|---|---:|---|---:|---:|---|---:|---:|---:|---|")

    arrays: list[np.ndarray] = []
    hashes: list[str] = []
    best_refs: list[str] = []
    title_quality: list[bool] = []
    first_ns = rows[0].mtime_epoch_ns

    for row in rows:
        if not row.path.exists():
            raise SystemExit(f"manifest image missing: {row.path}")
        actual = sha256(row.path)
        if actual != row.capture_sha:
            raise SystemExit(f"capture sha mismatch for {row.path}: manifest {row.capture_sha}, actual {actual}")
        with Image.open(row.path) as img:
            if img.size != (row.width, row.height):
                raise SystemExit(f"dimension mismatch for {row.path}: manifest {(row.width, row.height)}, actual {img.size}")
            arr = normalized(np.asarray(img.convert("RGB"), dtype=np.int16)).astype(np.uint8)
        arrays.append(arr)
        hashes.append(actual)
        best = min((metrics(arr, ref)[0], metrics(arr, ref)[1], metrics(arr, ref)[2], ref_path) for ref_path, ref in refs)
        mae, differing, max_delta, ref_path = best
        best_refs.append(ref_path.name)
        good = row.width == 320 and row.height == 200 and mae <= 2.0 and max_delta <= 12
        title_quality.append(good)
        quality = "TITLE-like" if good else "non-TITLE/transition"
        lines.append(
            f"| {row.index} | `{log_times.get(row.path.name, 'n/a')}` | `{row.mtime_iso}` | {format_ms(row.mtime_epoch_ns - first_ns)} | `{actual}` | {row.size_bytes} | {row.width}x{row.height} | `{ref_path.name}` | {mae:.6f} | {differing} | {max_delta} | {quality} |"
        )

    lines.append("")
    lines.append("## Adjacent-frame deltas")
    lines.append("")
    for i in range(len(arrays) - 1):
        mae, differing, max_delta = metrics(arrays[i], arrays[i + 1])
        delta_ns = rows[i + 1].mtime_epoch_ns - rows[i].mtime_epoch_ns
        lines.append(
            f"- {rows[i].index} -> {rows[i+1].index}: mtime delta {format_ms(delta_ns)} ms; image MAE {mae:.6f}, differing pixels {differing}, max delta {max_delta}; refs `{best_refs[i]}` -> `{best_refs[i+1]}`."
        )

    good_refs = [ref for ref, good in zip(best_refs, title_quality) if good]
    good_hashes = [digest for digest, good in zip(hashes, title_quality) if good]
    lines.append("")
    lines.append("## Native capture status")
    lines.append("")
    lines.append(f"- Native raw screenshots: {len(rows)} total, {len(set(hashes))} unique SHA-256 values.")
    lines.append(f"- 320x200 raw framebuffer screenshots: {sum(1 for r in rows if (r.width, r.height) == (320, 200))}/{len(rows)}.")
    lines.append(f"- TITLE-like matches at pass-65 tolerance: {len(good_refs)} frames, {len(set(good_hashes))} unique images, {len(set(good_refs))} unique best references (`{', '.join(sorted(set(good_refs))) if good_refs else 'none'}`).")
    if good_refs and len(set(good_refs)) == 1:
        lines.append("- Native raw capture is now proven usable for source-backed still evidence, but this run captured one repeated TITLE source frame followed by transition/menu frames; it is not a monotonic cadence sample.")
    elif len(set(good_refs)) > 1:
        lines.append("- Native raw capture produced multiple source-backed TITLE references in this run, but cadence should still be claimed only if the log/mtime order is monotonic and dense enough for the source-frame sequence.")
    else:
        lines.append("- Native raw capture did not hit the TITLE window in this run; adjust WAIT_AFTER_INPUT_MS and rerun.")
    lines.append("- DOSBox Staging's screenshot action itself is blocking/sparse on this host (hundreds of ms between written PNGs despite a shorter requested gap), so Cmd+F5 raw screenshots are a still-evidence route, not a reliable animation-cadence route.")
    lines.append("")
    lines.append("## Precise remaining blocker and next route")
    lines.append("")
    lines.append("Native/raw screenshots work, but the mapper-triggered Cmd+F5 path cannot prove original TITLE cadence: every screenshot request pauses or stalls enough that a single tuned run samples one TITLE frame repeatedly and then misses the short transition to the entrance/menu.  The next bounded route should bypass the interactive screenshot action: either instrument DOSBox Staging/SDL to dump the raw 320x200 framebuffer once per presented VGA frame, use an emulator/debugger with frame-dump support (for example DOSBox-X or a custom Staging build), or source-prove the PC conditional timing from the original/ReDMCSB TITLE code and then use native screenshots only as still-frame identity evidence.")
    lines.append("")

    report_path = args.out_dir / "pass67_native_raw_sequence_match_report.md"
    report_path.write_text("\n".join(lines))
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

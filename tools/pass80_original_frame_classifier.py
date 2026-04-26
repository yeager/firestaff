#!/usr/bin/env python3
"""Pass 80 raw original-frame classifier/audit.

This is deliberately non-invasive: it reads DOSBox Staging raw screenshot PNGs
from an original-route attempt directory and writes JSON/Markdown evidence.  It
answers the pass-79 blocker before pixel comparisons are trusted: are the raw
320x200 captures actually title/menu, dungeon gameplay, spell panel, inventory,
or something else?

The classifier is heuristic but transparent.  It uses stable DM1 320x200 layout
regions and records the measured ratios that drove each label.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
WIDTH = 320
HEIGHT = 200

REGIONS = {
    # DM1 full-frame layout regions used by the existing pass70/pass74 tooling.
    "viewport": (0, 33, 224, 136),
    "right_action": (224, 45, 87, 45),
    "spell_area": (224, 90, 87, 25),
    "right_column": (224, 0, 96, 200),
    "inventory_extent": (80, 53, 144, 73),
    "title_top": (0, 0, 224, 33),
}

PASS77_EXPECTED = [
    "dungeon_gameplay",
    "dungeon_gameplay",
    "dungeon_gameplay",
    "spell_panel",
    "dungeon_gameplay",
    "inventory",
]


@dataclass(frozen=True)
class RegionStats:
    nonblack_ratio: float
    color_ratio: float
    unique_colors: int
    mean_rgb: tuple[float, float, float]
    luma_stddev: float

    def as_json(self) -> dict[str, object]:
        return {
            "nonblack_ratio": round(self.nonblack_ratio, 4),
            "color_ratio": round(self.color_ratio, 4),
            "unique_colors": self.unique_colors,
            "mean_rgb": [round(x, 2) for x in self.mean_rgb],
            "luma_stddev": round(self.luma_stddev, 2),
        }


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


def png_dims(path: Path) -> tuple[int, int]:
    data = path.read_bytes()[:32]
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise SystemExit(f"not PNG/IHDR: {path}")
    return struct.unpack(">II", data[16:24])


def load_rgb(path: Path):
    from PIL import Image

    return Image.open(path).convert("RGB")


def stats_for(img, xywh: tuple[int, int, int, int]) -> RegionStats:
    x0, y0, w, h = xywh
    pix = list(img.crop((x0, y0, x0 + w, y0 + h)).getdata())
    n = len(pix)
    nonblack = 0
    color = 0
    sr = sg = sb = 0
    lumas: list[float] = []
    for r, g, b in pix:
        sr += r
        sg += g
        sb += b
        if (r, g, b) != (0, 0, 0):
            nonblack += 1
        if max(r, g, b) - min(r, g, b) > 8:
            color += 1
        lumas.append(0.299 * r + 0.587 * g + 0.114 * b)
    mean_luma = sum(lumas) / n
    var = sum((x - mean_luma) ** 2 for x in lumas) / n
    return RegionStats(
        nonblack_ratio=nonblack / n,
        color_ratio=color / n,
        unique_colors=len(set(pix)),
        mean_rgb=(sr / n, sg / n, sb / n),
        luma_stddev=math.sqrt(var),
    )


def classify(regions: dict[str, RegionStats], dims: tuple[int, int]) -> tuple[str, str]:
    if dims != (WIDTH, HEIGHT):
        return "non_graphics_blocker", f"raw dimensions are {dims[0]}x{dims[1]}, not 320x200"

    viewport = regions["viewport"]
    right_col = regions["right_column"]
    right_action = regions["right_action"]
    spell = regions["spell_area"]
    inventory = regions["inventory_extent"]

    # TITLE/menu frames fill the right-side game UI coordinates with colorful
    # title/menu art while the DM viewport area is comparatively sparse/dark.
    if viewport.nonblack_ratio < 0.40 and right_col.nonblack_ratio > 0.70 and right_col.color_ratio > 0.20:
        return "title_or_menu", "sparse viewport plus colorful/right-column title-menu art"

    # A flat close wall can fill the historical inventory extent with dense,
    # low-color pixels.  Classify that as its own unsafe state before inventory
    # or generic dungeon gameplay so route audits do not treat a wall close-up as
    # proof that the inventory/spell transition reached the intended state.
    if (
        viewport.nonblack_ratio > 0.85
        and viewport.color_ratio < 0.05
        and viewport.unique_colors <= 8
        and right_col.nonblack_ratio < 0.15
        and inventory.nonblack_ratio > 0.70
        and inventory.color_ratio < 0.10
    ):
        return "wall_closeup", "flat low-color viewport fills inventory extent; unsafe for inventory/spell evidence"

    # Inventory panel overlays the central viewport area with a dense, low-color
    # panel.  It must be distinguishable from the flat close-wall guard above.
    if inventory.nonblack_ratio > 0.95 and inventory.color_ratio < 0.10 and viewport.nonblack_ratio > 0.70:
        return "inventory", "dense low-color inventory extent over viewport"

    # Spell panel has the right-column spell region lit with colored rune/text
    # content.  Title/menu was already filtered above because it also occupies
    # this coordinate range.
    if spell.nonblack_ratio > 0.50 and spell.color_ratio > 0.45 and right_action.nonblack_ratio < 0.20:
        return "spell_panel", "colored spell-area content in in-game right panel"

    # Normal dungeon views have substantial viewport content but a mostly dark
    # right panel/action area in the current deterministic route.
    if viewport.nonblack_ratio > 0.50 and right_col.nonblack_ratio < 0.35:
        return "dungeon_gameplay", "viewport content with mostly dark in-game right column"

    return "graphics_320x200_unclassified", "320x200 graphics frame, but layout heuristics did not match a known class"


def parse_expected(value: str | None) -> list[str] | None:
    if value is None:
        return None
    if value == "pass77":
        return PASS77_EXPECTED
    parsed = [x.strip() for x in value.split(",") if x.strip()]
    return parsed or None


def write_markdown(path: Path, result: dict[str, object]) -> None:
    lines = [
        "# Pass 80 — original raw-frame classifier audit",
        "",
        "This audit classifies raw DOSBox `320x200` screenshots before they are used as original-route parity evidence.",
        "It is measurement-only and records the layout features behind each label.",
        "",
        f"- attempt dir: `{result['attempt_dir']}`",
        f"- capture count: {result['capture_count']}",
        f"- pass: `{result['pass']}`",
        f"- honesty: {result['honesty']}",
        "",
    ]
    if result.get("warnings"):
        lines.extend(["## Warnings", ""])
        for warning in result["warnings"]:
            lines.append(f"- {warning}")
        lines.append("")
    if result.get("problems"):
        lines.extend(["## Problems", ""])
        for problem in result["problems"]:
            lines.append(f"- {problem}")
        lines.append("")
    lines.extend([
        "| # | file | classification | expected | ok | reason | sha256 |",
        "|---|------|----------------|----------|----|--------|--------|",
    ])
    for i, row in enumerate(result["captures"], 1):
        expected = row.get("expected_class") or ""
        ok = row.get("expected_match")
        ok_s = "" if ok is None else ("yes" if ok else "NO")
        lines.append(
            f"| {i} | `{row['file']}` | `{row['classification']}` | `{expected}` | {ok_s} | {row['reason']} | `{str(row['sha256'])[:12]}` |"
        )
    lines.extend([
        "",
        "## Region metrics",
        "",
        "Ratios are per region: nonblack pixels, visibly colored pixels, unique RGB colors, and luma standard deviation.",
        "",
    ])
    for row in result["captures"]:
        lines.append(f"### `{row['file']}`")
        lines.append("")
        lines.append("| region | nonblack | color | unique | luma stddev |")
        lines.append("|--------|----------|-------|--------|-------------|")
        for name, stats in row["regions"].items():
            lines.append(
                f"| {name} | {stats['nonblack_ratio']:.4f} | {stats['color_ratio']:.4f} | {stats['unique_colors']} | {stats['luma_stddev']:.2f} |"
            )
        lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines).rstrip() + "\n")


def _stats(nonblack: float, color: float, unique: int) -> RegionStats:
    return RegionStats(nonblack, color, unique, (0.0, 0.0, 0.0), 0.0)


def run_self_test() -> int:
    base = {name: _stats(0.0, 0.0, 1) for name in REGIONS}
    cases = []

    wall = dict(base)
    wall.update({
        "viewport": _stats(0.925, 0.0, 6),
        "right_column": _stats(0.063, 0.060, 3),
        "right_action": _stats(0.014, 0.0, 2),
        "spell_area": _stats(0.0, 0.0, 1),
        "inventory_extent": _stats(0.784, 0.0, 6),
    })
    cases.append(("flat close-wall route frame", wall, "wall_closeup"))

    inventory = dict(base)
    inventory.update({
        "viewport": _stats(0.82, 0.16, 24),
        "right_column": _stats(0.10, 0.03, 5),
        "right_action": _stats(0.03, 0.0, 2),
        "spell_area": _stats(0.02, 0.0, 2),
        "inventory_extent": _stats(0.98, 0.04, 12),
    })
    cases.append(("dense inventory panel", inventory, "inventory"))

    spell = dict(base)
    spell.update({
        "viewport": _stats(0.74, 0.18, 30),
        "right_column": _stats(0.24, 0.18, 16),
        "right_action": _stats(0.08, 0.03, 5),
        "spell_area": _stats(0.64, 0.52, 15),
        "inventory_extent": _stats(0.42, 0.11, 10),
    })
    cases.append(("spell panel", spell, "spell_panel"))

    failures = []
    for name, regions, expected in cases:
        got, reason = classify(regions, (WIDTH, HEIGHT))
        if got != expected:
            failures.append(f"{name}: got {got}, expected {expected} ({reason})")
    if failures:
        print(json.dumps({"pass": False, "failures": failures}, indent=2))
        return 1
    print(json.dumps({"pass": True, "cases": len(cases)}, indent=2))
    return 0


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("attempt_dir", type=Path, nargs="?", help="directory containing DOSBox imageNNNN-raw.png captures")
    ap.add_argument("--out-json", type=Path, default=None)
    ap.add_argument("--out-md", type=Path, default=None)
    ap.add_argument("--expected", default=None, help="comma-separated expected classes, or 'pass77'")
    ap.add_argument("--fail-on-duplicates", action="store_true", help="treat repeated raw frame sha256 values as a failing problem instead of a warning")
    ap.add_argument("--self-test", action="store_true", help="run classifier guard tests without reading PNG files")
    args = ap.parse_args(argv)

    if args.self_test:
        return run_self_test()
    if args.attempt_dir is None:
        ap.error("attempt_dir is required unless --self-test is used")

    if args.attempt_dir is None:
        ap.error("attempt_dir is required unless --self-test is used")

    paths = sorted(args.attempt_dir.glob("image*.png"))
    expected = parse_expected(args.expected)
    rows: list[dict[str, object]] = []
    dims_seen: dict[str, int] = {}
    duplicate_sha256: dict[str, int] = {}
    problems: list[str] = []
    warnings: list[str] = []

    if expected is not None and len(expected) != len(paths):
        problems.append(f"expected {len(expected)} frame classes but found {len(paths)} raw captures")

    for idx, path in enumerate(paths):
        dims = png_dims(path)
        dims_seen[f"{dims[0]}x{dims[1]}"] = dims_seen.get(f"{dims[0]}x{dims[1]}", 0) + 1
        digest = sha256(path)
        duplicate_sha256[digest] = duplicate_sha256.get(digest, 0) + 1
        regions: dict[str, RegionStats] = {}
        if dims == (WIDTH, HEIGHT):
            img = load_rgb(path)
            regions = {name: stats_for(img, xywh) for name, xywh in REGIONS.items()}
        label, reason = classify(regions, dims) if regions else ("non_graphics_blocker", f"raw dimensions are {dims[0]}x{dims[1]}, not 320x200")
        want = expected[idx] if expected is not None and idx < len(expected) else None
        match = None if want is None else (label == want)
        if match is False:
            problems.append(f"{path.name}: classified {label}, expected {want}")
        rows.append({
            "file": display(path),
            "width": dims[0],
            "height": dims[1],
            "bytes": path.stat().st_size,
            "sha256": digest,
            "classification": label,
            "reason": reason,
            "expected_class": want,
            "expected_match": match,
            "regions": {name: s.as_json() for name, s in regions.items()},
        })

    repeated_hashes = {h: n for h, n in duplicate_sha256.items() if n > 1}
    if repeated_hashes:
        duplicate_msg = f"duplicate raw frames detected: {len(repeated_hashes)} unique sha256 value(s) repeat"
        if args.fail_on_duplicates:
            problems.append(duplicate_msg)
        else:
            warnings.append(duplicate_msg)

    result = {
        "schema": "pass80_original_frame_classifier.v1",
        "attempt_dir": display(args.attempt_dir),
        "capture_count": len(rows),
        "dimensions_seen": dims_seen,
        "expected_sequence": expected,
        "class_counts": {label: sum(1 for row in rows if row["classification"] == label) for label in sorted({str(row["classification"]) for row in rows})},
        "duplicate_sha256_counts": repeated_hashes,
        "honesty": "Classifier/audit only. A 320x200 frame is not parity evidence until its semantic class matches the intended route state.",
        "captures": rows,
        "warnings": warnings,
        "problems": problems,
        "pass": bool(rows) and not problems,
    }

    out_json = args.out_json or (args.attempt_dir / "pass80_original_frame_classifier.json")
    out_md = args.out_md or out_json.with_suffix(".md")
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(result, indent=2) + "\n")
    write_markdown(out_md, result)

    print(json.dumps({
        "captures": len(rows),
        "class_counts": result["class_counts"],
        "warnings": warnings,
        "problems": problems,
        "json": display(out_json),
        "markdown": display(out_md),
    }, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

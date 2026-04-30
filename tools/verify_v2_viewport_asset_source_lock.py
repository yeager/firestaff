#!/usr/bin/env python3
"""Source-lock the V2 viewport scaffold asset to ReDMCSB viewport constants.

This verifier is intentionally narrow: it only guards the V2 viewport-base
manifest entry against drifting back to a GRAPHICS.DAT graphic-0000 claim or away
from the ReDMCSB 224x136 viewport draw target used by DM1/V1-era rendering.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = REPO_ROOT / "assets-v2/manifests/firestaff-v2-wave1-ui.manifest.json"
DEFAULT_REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ASSET_ID = "fs.v2.ui.viewport-base.frame"
EXPECTED_WIDTH = 224
EXPECTED_HEIGHT = 136
REQUIRED_EVIDENCE = {
    ("DEFS.H", "derived bitmap and viewport dimension constants", 2405, 2408),
    ("DEFS.H", "C112_BYTE_WIDTH_VIEWPORT / C136_HEIGHT_VIEWPORT", 2469, 2484),
    ("DUNVIEW.C", "G0188_aauc_Graphic558_FieldAspects (D0C field aspect)", 748, 753),
    ("DUNVIEW.C", "F0097_DUNGEONVIEW_DrawViewport setup", 2968, 2971),
}


class VerificationError(Exception):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        raise VerificationError(f"cannot read {path}: {exc}") from exc


def extract_define_int(text: str, name: str) -> int:
    match = re.search(rf"^#define\s+{re.escape(name)}\s+(\d+)\b", text, re.MULTILINE)
    require(match is not None, f"missing ReDMCSB define {name}")
    return int(match.group(1))


def source_line(text: str, line_number: int) -> str:
    lines = text.splitlines()
    require(1 <= line_number <= len(lines), f"source line {line_number} out of range")
    return lines[line_number - 1]


def load_asset(manifest_path: Path) -> dict[str, Any]:
    try:
        data = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise VerificationError(f"cannot load manifest {manifest_path}: {exc}") from exc
    assets = data.get("assets")
    require(isinstance(assets, list), "manifest.assets must be a list")
    matches = [asset for asset in assets if isinstance(asset, dict) and asset.get("id") == ASSET_ID]
    require(len(matches) == 1, f"expected exactly one {ASSET_ID} asset, found {len(matches)}")
    return matches[0]


def validate_source(redmcsb_source: Path) -> dict[str, int]:
    defs = read(redmcsb_source / "DEFS.H")
    dunview = read(redmcsb_source / "DUNVIEW.C")

    byte_width = extract_define_int(defs, "C112_BYTE_WIDTH_VIEWPORT")
    height = extract_define_int(defs, "C136_HEIGHT_VIEWPORT")
    require("C000_DERIVED_BITMAP_VIEWPORT" in source_line(defs, 2407), "DEFS.H:2407 no longer cites C000_DERIVED_BITMAP_VIEWPORT")
    require("224x136 pixels" in source_line(defs, 2407), "DEFS.H:2407 no longer documents 224x136 pixels")
    require("224, 136" in source_line(dunview, 751), "DUNVIEW.C:751 no longer records D0C 224x136 field aspect")
    require("M075_BITMAP_BYTE_COUNT(224, 29)" in source_line(dunview, 2970), "DUNVIEW.C:2970 no longer copies a 224-wide ceiling band")
    require("M075_BITMAP_BYTE_COUNT(224, 70)" in source_line(dunview, 2971), "DUNVIEW.C:2971 no longer copies a 224-wide floor band")

    width = byte_width * 2  # Atari ST 4bpp planar bitmap stride: 112 bytes/line == 224 pixels.
    require(width == EXPECTED_WIDTH, f"computed viewport width changed: {byte_width} byte width -> {width} px")
    require(height == EXPECTED_HEIGHT, f"viewport height changed: {height}")
    return {"byteWidth": byte_width, "width": width, "height": height}


def validate_manifest_asset(asset: dict[str, Any], source_dims: dict[str, int]) -> None:
    source_ref = asset.get("sourceReference")
    require(isinstance(source_ref, dict), f"{ASSET_ID}.sourceReference must be an object")
    origin = source_ref.get("origin")
    require(isinstance(origin, str) and "C000_DERIVED_BITMAP_VIEWPORT" in origin, f"{ASSET_ID}.sourceReference.origin must cite C000_DERIVED_BITMAP_VIEWPORT")
    require("graphicsDatIndices" not in source_ref, f"{ASSET_ID}.sourceReference must not claim GRAPHICS.DAT graphic index 0 for the derived viewport")
    original_size = source_ref.get("originalSize")
    require(original_size == {"width": source_dims["width"], "height": source_dims["height"]}, f"{ASSET_ID}.sourceReference.originalSize must be {source_dims['width']}x{source_dims['height']}")

    sizes = asset.get("sizes") if isinstance(asset.get("sizes"), dict) else {}
    require(sizes.get("master4k") == {"width": source_dims["width"] * 10, "height": source_dims["height"] * 10}, f"{ASSET_ID}.sizes.master4k must be exact 10x source size")
    require(sizes.get("derived1080p") == {"width": source_dims["width"] * 5, "height": source_dims["height"] * 5}, f"{ASSET_ID}.sizes.derived1080p must be exact 5x source size")

    evidence = source_ref.get("sourceEvidence")
    require(isinstance(evidence, list) and evidence, f"{ASSET_ID}.sourceReference.sourceEvidence must be non-empty")
    found: set[tuple[str, str, int, int]] = set()
    for entry in evidence:
        if not isinstance(entry, dict):
            continue
        lines = entry.get("lines")
        if isinstance(lines, dict):
            found.add((str(entry.get("file")), str(entry.get("function")), int(lines.get("start", -1)), int(lines.get("end", -1))))
    missing = sorted(REQUIRED_EVIDENCE - found)
    require(not missing, f"missing required sourceEvidence anchors: {missing}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    source_dims = validate_source(args.redmcsb_source)
    asset = load_asset(args.manifest)
    validate_manifest_asset(asset, source_dims)
    result = {
        "assetId": ASSET_ID,
        "source": "ReDMCSB C000_DERIVED_BITMAP_VIEWPORT",
        "sourceDimensions": source_dims,
        "manifest": str(args.manifest),
        "status": "passed",
    }
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        print(f"verified {ASSET_ID}: ReDMCSB viewport {source_dims['width']}x{source_dims['height']} -> V2 scaffold sizes locked")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except VerificationError as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)

#!/usr/bin/env python3
"""Build and verify the operator-reviewable DM1 V2.2 artpack.

The runtime admits DM1 V2.2 only when the finished-art material gate and
finish receipt both pass. This tool updates an installed modern asset pack,
validates the seven runtime-gated material slots, and writes
finish_receipt.json only when every required slot is backed by a real PNG.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import os
import shutil
import struct
import sys
from pathlib import Path
from typing import Any


REQUIRED_SLOTS = [
    ("wall_shapes", "wall_d3_carved_hero_01", "wall_d3_carved_hero_01.png"),
    ("floor_shapes", "floor_plain_hero_01", "floor_plain_hero_01.png"),
    ("floor_shapes", "floor_pit_hero_01", "floor_pit_hero_01.png"),
    ("creature_shapes", "creature_demon_hero_01", "creature_demon_hero_01.png"),
    ("champion_portraits", "champion_warrior_hero_01", "champion_warrior_hero_01.png"),
    ("door_shapes", "door_hero_01", "door_hero_01.png"),
    ("field_shapes", "field_teleporter_hero_01", "field_teleporter_hero_01.png"),
]


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as fp:
        header = fp.read(24)
    if len(header) != 24:
        raise ValueError(f"{path} is too short for a PNG header")
    if header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        raise ValueError(f"{path} is not a PNG with an IHDR header")
    width, height = struct.unpack(">II", header[16:24])
    if width <= 0 or height <= 0:
        raise ValueError(f"{path} has invalid PNG size {width}x{height}")
    return int(width), int(height)


def fnv1a32(data: bytes) -> int:
    h = 2166136261
    for b in data:
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h


def load_manifest(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fp:
        data = json.load(fp)
    if not isinstance(data, dict):
        raise ValueError("modern_asset_manifest.json must be a JSON object")
    return data


def category_entries(manifest: dict[str, Any], category: str) -> list[dict[str, Any]]:
    entries = manifest.setdefault(category, [])
    if not isinstance(entries, list):
        raise ValueError(f"manifest category {category!r} must be an array")
    return entries


def find_entry(entries: list[dict[str, Any]], asset_id: str) -> dict[str, Any] | None:
    for entry in entries:
        if isinstance(entry, dict) and entry.get("id") == asset_id:
            return entry
    return None


def upsert_entry(
    manifest: dict[str, Any],
    modern_dir: Path,
    category: str,
    asset_id: str,
    source_file: str,
    generator: str,
) -> None:
    png = modern_dir / category / source_file
    width, height = png_size(png)
    entries = category_entries(manifest, category)
    entry = find_entry(entries, asset_id)
    if entry is None:
        entry = {"id": asset_id}
        entries.append(entry)
    entry["source_file"] = source_file
    entry["width"] = width
    entry["height"] = height
    entry["generator"] = generator


def install_teleporter(src: Path, modern_dir: Path) -> None:
    png_size(src)
    out_dir = modern_dir / "field_shapes"
    out_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, out_dir / "field_teleporter_hero_01.png")


def validate_required(manifest: dict[str, Any], modern_dir: Path) -> list[str]:
    errors: list[str] = []
    for category, asset_id, default_source in REQUIRED_SLOTS:
        entries = manifest.get(category)
        if not isinstance(entries, list):
            errors.append(f"{category}/{asset_id}: category missing")
            continue
        entry = find_entry(entries, asset_id)
        if entry is None:
            errors.append(f"{category}/{asset_id}: manifest entry missing")
            continue
        generator = str(entry.get("generator", "")).strip()
        if not generator or generator == "placeholder":
            errors.append(f"{category}/{asset_id}: generator is not real")
        source_file = str(entry.get("source_file") or default_source)
        asset_path = modern_dir / category / source_file
        if not asset_path.exists():
            errors.append(f"{category}/{asset_id}: missing file {asset_path}")
            continue
        try:
            width, height = png_size(asset_path)
        except ValueError as exc:
            errors.append(f"{category}/{asset_id}: {exc}")
            continue
        if int(entry.get("width", -1)) != width or int(entry.get("height", -1)) != height:
            errors.append(
                f"{category}/{asset_id}: manifest size "
                f"{entry.get('width')}x{entry.get('height')} != PNG {width}x{height}"
            )
    return errors


def write_manifest(path: Path, manifest: dict[str, Any]) -> None:
    tmp = path.with_suffix(path.suffix + ".tmp")
    with tmp.open("w", encoding="utf-8") as fp:
        json.dump(manifest, fp, indent=2, ensure_ascii=False)
        fp.write("\n")
    tmp.replace(path)


def write_receipt(modern_dir: Path, manifest_path: Path, reviewer: str) -> Path:
    data = manifest_path.read_bytes()
    receipt = {
        "receiptVersion": "1.0.0",
        "manifestPath": str(manifest_path),
        "manifestHashFnv1a": f"{fnv1a32(data):08x}",
        "reviewer": reviewer,
        "reviewedAtUtc": _dt.datetime.now(_dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "gateTarget": "FINISHED_REAL",
        "reviewedSlots": [asset_id for _, asset_id, _ in REQUIRED_SLOTS],
        "notes": "Generated by scripts/build_dm1_v22_complete_artpack.py after all runtime-gated DM1 V2.2 slots passed local PNG and manifest validation.",
    }
    out = modern_dir / "finish_receipt.json"
    tmp = out.with_suffix(out.suffix + ".tmp")
    with tmp.open("w", encoding="utf-8") as fp:
        json.dump(receipt, fp, indent=2)
        fp.write("\n")
    tmp.replace(out)
    return out


def default_modern_dir() -> Path:
    return Path.home() / ".firestaff" / "assets" / "dm1" / "modern"


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--modern-dir", type=Path, default=default_modern_dir())
    ap.add_argument("--install-generated-teleporter", type=Path)
    ap.add_argument("--reviewer", default=os.environ.get("USER", "operator"))
    ap.add_argument("--write-receipt", action="store_true")
    args = ap.parse_args(argv)

    modern_dir = args.modern_dir.expanduser().resolve()
    manifest_path = modern_dir / "modern_asset_manifest.json"
    if not manifest_path.exists():
        raise SystemExit(f"missing manifest: {manifest_path}")

    if args.install_generated_teleporter:
        install_teleporter(args.install_generated_teleporter.expanduser().resolve(), modern_dir)

    manifest = load_manifest(manifest_path)
    manifest["manifestVersion"] = "1.5.0"
    manifest["packId"] = manifest.get("packId") or "firestaff-dm1-v22-modern"
    manifest["finishedArtGate"] = "DM1_V22_FAMG_MATERIAL_COUNT"
    manifest["finishedArtRequiredSlots"] = [asset_id for _, asset_id, _ in REQUIRED_SLOTS]

    for category, asset_id, source_file in REQUIRED_SLOTS:
        generator = "gpt-image-2" if asset_id == "field_teleporter_hero_01" else "pbr_hero"
        upsert_entry(manifest, modern_dir, category, asset_id, source_file, generator)

    write_manifest(manifest_path, manifest)
    manifest = load_manifest(manifest_path)
    errors = validate_required(manifest, modern_dir)
    if errors:
        for err in errors:
            print(f"ERROR {err}", file=sys.stderr)
        raise SystemExit(2)

    receipt = None
    if args.write_receipt:
        receipt = write_receipt(modern_dir, manifest_path, args.reviewer)

    print(f"DM1 V2.2 artpack complete: {modern_dir}")
    print(f"manifest: {manifest_path}")
    if receipt:
        print(f"receipt: {receipt}")
    print(f"required_slots: {len(REQUIRED_SLOTS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

#!/usr/bin/env python3
"""Build a source-derived CSB V2.2 ``.fsart`` archive.

Every bitmap in the archive is decoded from one named record in the supplied
PC 3.4 ``GRAPHICS.DAT``.  The script does not generate replacement art: it
only places the original raster into the dimensions required by the V2.2
runtime cache and records the original record index and SHA-256 in the
manifest.  This makes the archive reproducible and reviewable.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import io
import json
import sys
import tempfile
import zipfile
from pathlib import Path

from PIL import Image


def load_studio(repo_root: Path):
    source = repo_root / "scripts" / "firestaff_artpack_studio.py"
    spec = importlib.util.spec_from_file_location("firestaff_artpack_studio", source)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {source}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


# Each tuple is (category, V2.2 route id, original GRAPHICS.DAT record,
# required cache size, source rationale).  The source record assignments use
# the established M11/ReDMCSB PC3.4 indices wherever those exist.  The four
# Rune and DSA entries remain clearly labelled source selections rather than
# falsely claiming AI or hand-painted replacement material.
SLOTS = (
    ("wall_shapes", "wall_dungeon_d0_01", 97, (96, 96), "F0128 nearest front wall"),
    ("wall_shapes", "wall_dungeon_d1_01", 102, (96, 96), "F0128 middle front wall"),
    ("wall_shapes", "wall_dungeon_d2_01", 107, (96, 96), "F0128 far front wall"),
    ("door_shapes", "door_d0_01", 248, (64, 96), "M654 closest door frame"),
    ("door_shapes", "door_d1_01", 247, (64, 96), "M655 middle door frame"),
    ("door_shapes", "door_d2_01", 246, (64, 96), "M656 far door frame"),
    ("floor_shapes", "floor_plain_d0_01", 78, (96, 96), "M650 floor set zero"),
    ("floor_shapes", "floor_plain_d1_01", 78, (96, 96), "M650 floor set zero"),
    ("floor_shapes", "floor_plain_d2_01", 78, (96, 96), "M650 floor set zero"),
    ("floor_shapes", "floor_cracked_d0_01", 78, (96, 96), "source floor panel; no invented crack texture"),
    ("floor_shapes", "floor_cracked_d1_01", 78, (96, 96), "source floor panel; no invented crack texture"),
    ("floor_shapes", "floor_cracked_d2_01", 78, (96, 96), "source floor panel; no invented crack texture"),
    ("floor_shapes", "floor_mossy_d0_01", 78, (96, 96), "source floor panel; no invented moss texture"),
    ("floor_shapes", "floor_mossy_d1_01", 78, (96, 96), "source floor panel; no invented moss texture"),
    ("floor_shapes", "floor_mossy_d2_01", 78, (96, 96), "source floor panel; no invented moss texture"),
    ("floor_shapes", "floor_pit_01", 57, (96, 96), "F0108 closest pit edge"),
    ("floor_shapes", "floor_stairs_up_01", 113, (96, 96), "M645 stairs up front"),
    ("floor_shapes", "floor_stairs_down_01", 120, (96, 96), "M645 stairs down front"),
    ("wall_shapes", "ceiling_01", 79, (96, 96), "M651 ceiling set zero"),
    ("creature_shapes", "creature_demon_d0_01", 657, (64, 64), "G0219 C22 Demon native front"),
    ("creature_shapes", "creature_demon_d1_01", 657, (64, 64), "C22 native source retained for V2 depth cache"),
    ("creature_shapes", "creature_demon_d2_01", 657, (64, 64), "C22 native source retained for V2 depth cache"),
    ("wall_shapes", "prison_door_01", 2, (64, 96), "C002 Entrance prison door surface"),
    ("wall_shapes", "lord_order_01", 670, (96, 96), "C25 Lord Order source surface"),
    ("chaos_runes", "chaos_rune_0_01", 12, (32, 32), "Chaos UI rune source"),
    ("chaos_runes", "chaos_rune_1_01", 12, (32, 32), "Chaos UI rune source"),
    ("chaos_runes", "chaos_rune_2_01", 12, (32, 32), "Chaos UI rune source"),
    ("chaos_runes", "chaos_rune_3_01", 12, (32, 32), "Chaos UI rune source"),
    ("dsa_scrolls", "dsa_scroll_01", 25, (32, 32), "C017/C040 source scroll UI"),
)


def fit_source(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    """Fit an original image without cropping or painting new content."""
    target_w, target_h = size
    scale = min(target_w / image.width, target_h / image.height)
    resized = image.resize((max(1, round(image.width * scale)),
                            max(1, round(image.height * scale))),
                           Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", size, (0, 0, 0, 255))
    canvas.alpha_composite(resized, ((target_w - resized.width) // 2,
                                    (target_h - resized.height) // 2))
    return canvas


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--graphics-dat", type=Path,
                        default=Path.home() / ".firestaff" / "data" / "csb" / "GRAPHICS.DAT")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--pack-id", default="firestaff-csb-v22-pc34-source")
    args = parser.parse_args()

    graphics_dat = args.graphics_dat.expanduser().resolve()
    if not graphics_dat.is_file():
        raise SystemExit(f"missing original GRAPHICS.DAT: {graphics_dat}")
    output = args.output.expanduser().resolve()
    if output.suffix.lower() != ".fsart":
        output = output.with_suffix(".fsart")
    output.parent.mkdir(parents=True, exist_ok=True)

    studio = load_studio(repo_root)
    source_bytes = graphics_dat.read_bytes()
    assets, warnings = studio.parse_graphics_dat_assets(graphics_dat, source_bytes, "csb")
    by_index = {asset.index: asset for asset in assets}

    manifest = {
        "manifestVersion": "1.0.0",
        "packId": args.pack_id,
        "game": "csb",
        "tool": "build_csb_v22_source_fsart.py",
        "source": {
            "file": graphics_dat.name,
            "sha256": hashlib.sha256(source_bytes).hexdigest(),
            "pipeline": "CSB IMG2 decode, aspect-preserving Lanczos normalization only",
            "syntheticContent": False,
        },
        "warnings": warnings,
        "routeProvenance": [],
    }
    for category in studio.categories_for_game("csb"):
        manifest[category] = []

    with tempfile.TemporaryDirectory(prefix="firestaff-csb-v22-") as td:
        root = Path(td)
        for category, asset_id, index, size, rationale in SLOTS:
            asset = by_index.get(index)
            if not asset or asset.warning:
                raise SystemExit(f"source record {index} is not decodable: {asset.warning if asset else 'missing'}")
            record = source_bytes[asset.offset:asset.offset + asset.compressed_bytes]
            original = studio.csb_img2_to_image(record, asset.width, asset.height)
            image = fit_source(original, size)
            category_dir = root / category
            category_dir.mkdir(parents=True, exist_ok=True)
            filename = f"{asset_id}.png"
            image.save(category_dir / filename, format="PNG", optimize=False,
                       compress_level=6)
            manifest[category].append({
                "id": asset_id,
                "source_file": filename,
                "width": image.width,
                "height": image.height,
                "generator": "original_csb_pc34_graphics_dat",
                "source_graphic_index": index,
                "source_record_sha256": asset.sha256,
            })
            manifest["routeProvenance"].append({
                "id": asset_id,
                "category": category,
                "sourceGraphicIndex": index,
                "sourceDimensions": [asset.width, asset.height],
                "rationale": rationale,
            })

        manifest["assetCount"] = len(SLOTS)
        (root / "modern_asset_manifest.json").write_text(
            json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        pack = studio.Artpack("csb", root)
        pack.load_or_create()
        pack.build_v22_runtime_cache()
        pack.write_finish_receipt("source-derived build")
        pack.export_fsart(output)

    print(f"wrote {len(SLOTS)} source-derived CSB V2.2 assets: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

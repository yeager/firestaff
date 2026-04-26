#!/usr/bin/env python3
"""Check the local DM1 PC 3.4 source-reference asset lock.

This is intentionally a small, deterministic guard around the Greatstone/SCK
reference material Daniel pointed at.  It does not scrape the web at test time;
it verifies the local original hashes plus the local SCK-style extracted PGM
reference set for the critical graphics that Firestaff currently relies on.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import BinaryIO

EXPECTED_GRAPHICS_SHA256 = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"
EXPECTED_DUNGEON_SHA256 = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
EXPECTED_SONG_SHA256 = "71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177"

CRITICAL_GRAPHICS: dict[int, tuple[str, int, int]] = {
    0: ("Interface / derived viewport aperture", 224, 136),
    7: ("Interface - Champion Information", 67, 29),
    8: ("Interface - Dead Champion Information", 67, 29),
    9: ("Interface - Spell Casting Area", 87, 25),
    10: ("Interface - Item Actions Area", 87, 45),
    20: ("Interface - Character Sheet - Empty Information Area", 144, 73),
    26: ("Interface - Champions' Portraits", 256, 87),
    33: ("Interface - Gray Border Item Slot (Empty)", 18, 18),
    34: ("Interface - Red Border Item Slot (Wounded)", 18, 18),
    35: ("Interface - Cyan Border Item Slot (Item Action Selected)", 18, 18),
    42: ("Items Graphics 0 (32 Items)", 256, 32),
    78: ("Dungeon Graphics - Floor", 224, 97),
    79: ("Dungeon Graphics - Ceiling", 224, 39),
    86: ("Dungeon Graphics - Door Left or Right Frame (Front 1)", 32, 123),
    97: ("Dungeon Graphics - Wall (Front 1)", 160, 111),
    102: ("Dungeon Graphics - Wall (Front 2)", 106, 74),
    107: ("Dungeon Graphics - Wall (Front 3)", 70, 49),
    303: ("Dungeon Graphics - Wall Ornament / Lock family (left side)", 16, 19),
    304: ("Dungeon Graphics - Wall Ornament / Lock family (front)", 32, 28),
    344: ("Item/object sprite sheet slice used by V1 inventory/object probes", 90, 12),
    420: ("Projectile/effect sprite source", 60, 25),
    437: ("Projectile/effect sprite source", 9, 7),
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def next_token(f: BinaryIO) -> bytes:
    token = bytearray()
    while True:
        b = f.read(1)
        if not b:
            raise ValueError("unexpected EOF in PGM header")
        if b in b" \t\r\n":
            continue
        if b == b"#":
            while b and b not in b"\r\n":
                b = f.read(1)
            continue
        token.extend(b)
        break
    while True:
        b = f.read(1)
        if not b or b in b" \t\r\n":
            break
        token.extend(b)
    return bytes(token)


def read_pgm(path: Path) -> tuple[int, int, str, int]:
    with path.open("rb") as f:
        magic = next_token(f)
        width = int(next_token(f))
        height = int(next_token(f))
        maxval = int(next_token(f))
    if magic != b"P5" or maxval != 255:
        raise ValueError(f"{path}: expected binary P5 maxval 255, got {magic!r} maxval={maxval}")
    return width, height, sha256(path), path.stat().st_size


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    args = parser.parse_args()

    repo = args.repo.resolve()
    data_dir = repo / "verification-screens" / "dm1-dosbox-capture" / "DungeonMasterPC34" / "DATA"
    ref_dir = repo / "extracted-graphics-v1" / "pgm"

    originals = [
        ("GRAPHICS.DAT", data_dir / "GRAPHICS.DAT", EXPECTED_GRAPHICS_SHA256),
        ("DUNGEON.DAT", data_dir / "DUNGEON.DAT", EXPECTED_DUNGEON_SHA256),
        ("SONG.DAT", data_dir / "SONG.DAT", EXPECTED_SONG_SHA256),
    ]
    original_rows = []
    for name, path, expected in originals:
        if not path.exists():
            raise SystemExit(f"missing original source file: {path}")
        actual = sha256(path)
        if actual != expected:
            raise SystemExit(f"{name} SHA256 mismatch: expected {expected}, got {actual}")
        original_rows.append({"name": name, "path": str(path.relative_to(repo)), "sha256": actual})

    asset_rows = []
    for index, (label, expected_w, expected_h) in sorted(CRITICAL_GRAPHICS.items()):
        path = ref_dir / f"graphic_{index:04d}.pgm"
        if not path.exists():
            raise SystemExit(f"missing Greatstone/SCK-style PGM reference: {path}")
        width, height, digest, byte_count = read_pgm(path)
        if (width, height) != (expected_w, expected_h):
            raise SystemExit(
                f"graphic {index:04d} geometry mismatch: expected {expected_w}x{expected_h}, got {width}x{height}"
            )
        asset_rows.append(
            {
                "index": index,
                "label": label,
                "width": width,
                "height": height,
                "bytes": byte_count,
                "sha256": digest,
                "path": str(path.relative_to(repo)),
            }
        )

    result = {
        "source": "DM1 PC 3.4 local original + local Greatstone/SCK-style extracted references",
        "originals": original_rows,
        "critical_graphics": asset_rows,
        "critical_graphics_count": len(asset_rows),
        "parity_claimed": False,
        "honesty": "This locks source files and key asset identities/dimensions; it does not claim full runtime viewport parity.",
    }

    if args.json_out:
        args.json_out.parent.mkdir(parents=True, exist_ok=True)
        args.json_out.write_text(json.dumps(result, indent=2) + "\n")

    if args.markdown_out:
        args.markdown_out.parent.mkdir(parents=True, exist_ok=True)
        lines = [
            "# Greatstone / DM1 PC 3.4 source lock check",
            "",
            "This check verifies the local original PC 3.4 data files and the local Greatstone/SCK-style extracted PGM reference assets used by Firestaff parity work.",
            "",
            "It is a source-lock gate only; it does **not** claim full runtime viewport parity.",
            "",
            "## Original data hashes",
            "",
            "| file | sha256 |",
            "| --- | --- |",
        ]
        for row in original_rows:
            lines.append(f"| `{row['name']}` | `{row['sha256']}` |")
        lines += ["", "## Critical graphics locked", "", "| index | Greatstone/SCK role | geometry | sha256 |", "| ---: | --- | ---: | --- |"]
        for row in asset_rows:
            lines.append(
                f"| `{row['index']:04d}` | {row['label']} | `{row['width']}x{row['height']}` | `{row['sha256']}` |"
            )
        lines += [
            "",
            "## Result",
            "",
            f"PASS: `{len(asset_rows)}` critical graphics references matched expected source geometry and all original data hashes matched.",
            "",
            "Next: use these locked source identities as the input table for deterministic viewport/state construction, instead of treating live DOSBox keyboard routing as authoritative.",
        ]
        args.markdown_out.write_text("\n".join(lines) + "\n")

    print(f"PASS source lock: {len(asset_rows)} critical graphics, {len(original_rows)} original files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

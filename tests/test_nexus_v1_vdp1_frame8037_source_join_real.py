#!/usr/bin/env python3
"""Verify VDP1 texture-source evidence in the authentic Japanese menu frame."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
EXPECTED_CAPTURE_HASHES = {
    "vdp1-capture-bundle.txt":
        "10e9dd1ef6bff3ca95ffb7a57265819d92ab46621ab157d19b32478af360b874",
    "vdp1-command-trace.csv":
        "f15eb2038f2195c7d7cc64947e5d50b679b8280073e2aadbcec4d72b9a5ccf29",
    "vdp1-vram.bin":
        "807d5f5eb7314e58c4c1207695e0bb5752aedeac6f58999aeb21848600cda84a",
}
EXPECTED_RETAIL_HASHES = {
    "TITLE.BIN": "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3",
    "LEV00.DGN": "24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a",
}
EXPECTED_SCREENSHOT_SHA256 = (
    "8721cce3354bc20b1cd367fd44be3d4ecc60ca65fa89c7d00654e83f5ad7db23"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    capture_root = os.environ.get("FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR")
    data_root = os.environ.get(
        "FIRESTAFF_NEXUS_DATA_DIR",
        str(Path.home() / ".firestaff" / "data" / "nexus"),
    )
    screenshot_path = os.environ.get("FIRESTAFF_NEXUS_YMIR_FRAME8037_SCREENSHOT")
    if not capture_root or not Path(capture_root).is_dir():
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME8037_CAPTURE_DIR to authentic Ymir dumps")
        return 77
    if not Path(data_root).is_dir():
        print("SKIP: authentic Nexus retail data directory is unavailable")
        return 77
    if not screenshot_path or not Path(screenshot_path).is_file():
        print("SKIP: set FIRESTAFF_NEXUS_YMIR_FRAME8037_SCREENSHOT to the authentic title-menu PNG")
        return 77

    retail = Path(data_root)
    for name, expected in EXPECTED_RETAIL_HASHES.items():
        path = retail / name
        if not path.is_file() or sha256(path) != expected:
            print("SKIP: source join requires the captured Japanese TITLE.BIN and LEV00.DGN revisions")
            return 77

    capture = Path(capture_root)
    for name, expected in EXPECTED_CAPTURE_HASHES.items():
        path = capture / name
        if not path.is_file() or sha256(path) != expected:
            print(f"FAIL: authentic frame-8037 capture hash mismatch: {name}")
            return 1
    screenshot = Path(screenshot_path)
    if sha256(screenshot) != EXPECTED_SCREENSHOT_SHA256:
        print("FAIL: authentic frame-8037 screenshot hash mismatch")
        return 1

    command = [
        sys.executable,
        str(ROOT / "scripts" / "analyze_nexus_vdp1_source_join.py"),
        str(capture),
        "--data-dir",
        data_root,
        "--ymir-bundle",
    ]
    result = subprocess.run(command, check=False, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        return result.returncode

    required_receipts = (
        "frame=8037 draw_commands=137 ",
        "source_joined_draws=126/137",
        "dgn_structure2_joined_draws=121/137",
        "source_uninformative_draws=11/137",
        "source_join=partial",
        "semantic_admission=blocked",
    )
    missing = [receipt for receipt in required_receipts
               if receipt not in result.stdout]
    if missing:
        print("FAIL: frame-8037 source join did not match the authentic receipt")
        print("missing=" + "|".join(missing))
        sys.stderr.write(result.stdout)
        return 1

    sequence_command = [
        sys.executable,
        str(ROOT / "scripts" / "analyze_nexus_vdp1_dgn_command_sequence_join.py"),
        str(capture),
        "--data-dir",
        data_root,
        "--frame",
        "8037",
        "--ymir-bundle",
        "--require-complete",
    ]
    sequence = subprocess.run(
        sequence_command, check=False, capture_output=True, text=True
    )
    if sequence.returncode != 0:
        sys.stderr.write(sequence.stdout)
        sys.stderr.write(sequence.stderr)
        return sequence.returncode

    required_sequence_receipts = (
        "frame=8037 chain_records=178",
        "textured_draws=132 source_matches=121 palette_matches=113 direct_color_matches=8 face_owner_matches=105",
        "informative_textured_draws=121 transparent_zero_filled_draws=11",
        "bounded_failures=0 unmatched_offsets=none",
        "sequence_dgn_material_join=verified",
        "semantic_admission=blocked",
    )
    missing = [receipt for receipt in required_sequence_receipts
               if receipt not in sequence.stdout]
    if missing:
        print("FAIL: frame-8037 DGN material sequence did not match the authentic receipt")
        print("missing=" + "|".join(missing))
        sys.stderr.write(sequence.stdout)
        return 1

    title_sprite_command = [
        sys.executable,
        str(ROOT / "scripts" / "verify_nexus_title_sprite_screenshot.py"),
        str(capture),
        str(screenshot),
        str(retail / "TITLE.BIN"),
    ]
    title_sprites = subprocess.run(
        title_sprite_command, check=False, capture_output=True, text=True
    )
    if title_sprites.returncode != 0:
        sys.stderr.write(title_sprites.stdout)
        sys.stderr.write(title_sprites.stderr)
        return title_sprites.returncode

    required_title_sprite_receipts = (
        "command_index=171 command=0x1560 title_bin_offset=0x15f6",
        "command_index=172 command=0x1580 title_bin_offset=0x15f6",
        "command_index=173 command=0x15a0 title_bin_offset=0x446",
        "command_index=174 command=0x15c0 title_bin_offset=0x576",
        "command_index=175 command=0x15e0 title_bin_offset=0x7d6",
        "menu_state=japanese-new-game",
        "sprite_priority=5 ratio=31/32 lower_layer=VDP2_back_screen timing=unproven",
    )
    missing = [receipt for receipt in required_title_sprite_receipts
               if receipt not in title_sprites.stdout]
    if missing:
        print("FAIL: frame-8037 TITLE.BIN sprite receipts did not match")
        print("missing=" + "|".join(missing))
        sys.stderr.write(title_sprites.stdout)
        return 1
    expected_sprite_samples = (213, 213, 313, 293, 463)
    for command_index, samples in zip(range(171, 176), expected_sprite_samples):
        if not any(
            f"command_index={command_index}" in line and
            f"screenshot_matches={samples}/{samples}" in line
            for line in title_sprites.stdout.splitlines()
        ):
            print(f"FAIL: frame-8037 menu sprite {command_index} did not match every visible sample")
            sys.stderr.write(title_sprites.stdout)
            return 1

    print("PASS: frame 8037 matches 126/137 informative retail spans; all 121 indexed/direct DGN draws bind source data")
    print("PASS: DGN palette/direct-color checks cover all 121 informative draws; 11 zero spans stay transparent and unowned")
    print("PASS: 105 draws have a canonical Structure3 face owner; selected face, transform, culling and renderer admission remain blocked")
    print("PASS: all five TITLE.BIN title-menu sprites match their CRAM palettes and 1,495 visible screenshot samples")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Source-first guard for the DM1 V1 original-faithful evidence lane.

This verifier deliberately starts from ReDMCSB source anchors plus the local
DM1 PC 3.4 data hashes.  It does not run the emulator and it does not assert
new screenshot parity; it keeps the lane honest about the runtime contract it
may use as reference evidence.
"""
from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM_DATA = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA"

SOURCE_CHECKS: list[tuple[str, str, str]] = [
    (
        "DEFS.H",
        r"#define C9_PLATFORM_PC\s+9[\s\S]*#define C10_DUNGEON_DM\s+10",
        "DM dungeon id and PC platform constants are source-defined",
    ),
    (
        "CEDTINCD.C",
        r"Choice: Dungeon Master[\s\S]*M741_FILE_ID_LOAD_DMSAVE_DAT[\s\S]*F7054_DetermineGameFormats\(L3924_ps_\)[\s\S]*F7272_IsDungeonValid\(L3924_ps_, 1\)",
        "load routing determines formats then validates saved-game dungeon/platform metadata",
    ),
    (
        "CEDTINCU.C",
        r"case 1:[\s\S]*L4109_i_ == C10_DUNGEON_DM[\s\S]*L4110_B_ = C1_TRUE",
        "validation criterion 1 admits source DM dungeon id through the common validator",
    ),
    (
        "COORD.C",
        r"G2071_C320_ScreenPixelWidth = 320;[\s\S]*G2072_C200_ScreenPixelHeight = 200;[\s\S]*G2070_ViewportBitmapByteCount = 15232;[\s\S]*G2073_C224_ViewportPixelWidth = 224;[\s\S]*G2074_C136_ViewportHeight = 136;",
        "runtime render contract keeps 320x200 screen and 224x136 viewport bitmap",
    ),
    (
        "STARTUP2.C",
        r"G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;[\s\S]*G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;[\s\S]*G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;[\s\S]*G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
        "startup routes primary/secondary mouse and keyboard tables before map processing",
    ),
]

DATA_HASHES: dict[str, tuple[int, str]] = {
    "DUNGEON.DAT": (33357, "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"),
    "GRAPHICS.DAT": (363417, "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"),
    "SONG.DAT": (162482, "71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177"),
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def check_source() -> bool:
    ok = True
    for rel, pattern, desc in SOURCE_CHECKS:
        path = REDMCSB / rel
        if not path.is_file():
            print(f"FAIL source {rel}: missing {path}")
            ok = False
            continue
        text = path.read_text(errors="replace")
        if re.search(pattern, text, re.MULTILINE):
            print(f"PASS source {rel}: {desc}")
        else:
            print(f"FAIL source {rel}: {desc}")
            ok = False
    return ok


def check_data() -> bool:
    ok = True
    for name, (expected_size, expected_hash) in DATA_HASHES.items():
        path = DM_DATA / name
        if not path.is_file():
            print(f"FAIL data {name}: missing {path}")
            ok = False
            continue
        actual_size = path.stat().st_size
        actual_hash = sha256(path)
        if actual_size == expected_size and actual_hash == expected_hash:
            print(f"PASS data {name}: sha256={actual_hash} bytes={actual_size}")
        else:
            print(f"FAIL data {name}: sha256={actual_hash} bytes={actual_size}")
            print(f"  expected sha256={expected_hash} bytes={expected_size}")
            ok = False
    return ok


def main() -> int:
    ok = check_source()
    ok = check_data() and ok
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

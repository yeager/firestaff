#!/usr/bin/env python3
"""Guard the V1 floor-ornament/stair parity boundary.

ReDMCSB DUNGEON.C F0172 treats the stairs square 0x08 bit as orientation and
routes stairs through T0172046_Stairs without assigning
M558_FLOOR_ORNAMENT_ORDINAL.  Firestaff must not interpret stair 0x08 as random
floor-ornament permission.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
text = SRC.read_text()

errors: list[str] = []

compute = re.search(
    r"static int m11_compute_floor_ornament_ordinal\([^{}]*\) \{(?P<body>.*?)\n\}",
    text,
    re.S,
)
if not compute:
    errors.append("missing m11_compute_floor_ornament_ordinal")
else:
    body = compute.group("body")
    eligibility = re.search(
        r"if \(elementType != DUNGEON_ELEMENT_CORRIDOR &&(?P<expr>.*?)\) \{\n\s*return 0;",
        body,
        re.S,
    )
    if not eligibility:
        errors.append("missing floor-ornament element eligibility guard")
    elif "DUNGEON_ELEMENT_STAIRS" in eligibility.group("expr"):
        errors.append("stairs must not be eligible for computed floor ornaments")
    if "stairs 0x08" not in body or "T0172046_Stairs" not in body:
        errors.append("missing ReDMCSB stair-orientation source comment")

helper = re.search(
    r"static int m11_viewport_cell_is_wall_free\([^{}]*\) \{(?P<body>.*?)\n\}",
    text,
    re.S,
)
if not helper:
    errors.append("missing m11_viewport_cell_is_wall_free")
elif "DUNGEON_ELEMENT_STAIRS" in helper.group("body"):
    errors.append("wall-free floor ornament helper must exclude stairs")

if errors:
    for err in errors:
        print(f"[FAIL] {err}")
    sys.exit(1)
print("[OK] V1 floor ornament stair gate matches ReDMCSB source boundary")

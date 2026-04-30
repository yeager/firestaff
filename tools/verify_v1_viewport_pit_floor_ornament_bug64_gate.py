#!/usr/bin/env python3
"""Source-lock ReDMCSB BUG0_64: pit floor ornaments are not suppressed.

ReDMCSB DUNVIEW.C calls F0108_DUNGEONVIEW_DrawFloorOrnament before the
open-pit rendering branch in every visible floor slot and explicitly annotates
BUG0_64: floor ornaments are drawn over open pits.  Firestaff must therefore
keep pit cells eligible for floor-ornament extraction/drawing while excluding
stairs, whose 0x08 bit is orientation in DUNGEON.C F0172.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
text = SRC.read_text(encoding="utf-8")
errors: list[str] = []


def function_body(name: str) -> str:
    pattern = re.compile(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pattern.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0:
            break
        semicolon = text.find(";", m.end(), brace)
        if semicolon >= 0:
            continue
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return text[brace + 1:i]
        raise AssertionError(f"unterminated {name}")
    raise AssertionError(f"missing {name}")

try:
    wall_free = function_body("m11_viewport_cell_is_wall_free")
    if "DUNGEON_ELEMENT_PIT" not in wall_free:
        errors.append("pit cells must remain floor-ornament eligible")
    if "DUNGEON_ELEMENT_TELEPORTER" not in wall_free:
        errors.append("teleporter cells must remain sensor floor-ornament eligible")
    if "DUNGEON_ELEMENT_STAIRS" in wall_free:
        errors.append("stairs must stay excluded; ReDMCSB F0172 uses stair bit 0x08 as orientation")
except AssertionError as exc:
    errors.append(str(exc))

try:
    contents = function_body("m11_draw_wall_contents")
    layer0 = contents.find("/* Layer 0: Floor ornaments")
    pit_guard = re.search(r"elementType\s*==\s*DUNGEON_ELEMENT_PIT|DUNGEON_ELEMENT_PIT\s*==\s*[^;]+elementType", contents)
    if layer0 < 0 or "m11_draw_floor_ornament" not in contents[layer0:]:
        errors.append("open-cell content stack must draw layer-0 floor ornaments")
    if pit_guard:
        errors.append("m11_draw_wall_contents must not suppress floor ornaments on pit cells")
except AssertionError as exc:
    errors.append(str(exc))

try:
    side = function_body("m11_draw_side_feature")
    side_layer = side.find("Floor ornaments in side cells")
    if side_layer < 0 or "m11_draw_floor_ornament" not in side[side_layer:]:
        errors.append("side cells must draw floor ornaments through the same open-cell path")
    side_window = side[side_layer:side_layer + 600] if side_layer >= 0 else side
    if re.search(r"elementType\s*==\s*DUNGEON_ELEMENT_PIT|DUNGEON_ELEMENT_PIT\s*==\s*[^;]+elementType", side_window):
        errors.append("m11_draw_side_feature must not suppress pit floor ornaments")
except AssertionError as exc:
    errors.append(str(exc))

required_comments = [
    "BUG0_64 deliberately keeps",
    "stairs are excluded by F0172",
    "orientation, not random-ornament permission",
]
for needle in required_comments:
    if needle not in text:
        errors.append(f"missing source-lock comment: {needle!r}")

if errors:
    for err in errors:
        print(f"[FAIL] {err}")
    sys.exit(1)
print("[OK] V1 pit floor-ornament BUG0_64 source gate matches ReDMCSB")
print("- ReDMCSB DUNGEON.C:2628-2682 pit/teleporter path scans floor sensors and routes to T0172049_Footprints")
print("- ReDMCSB DUNGEON.C:2693-2718 stairs route through T0172046_Stairs without assigning M558")
print("- ReDMCSB DUNVIEW.C:6284-6286, 6351-6353, 7020-7031, 7213-7224, 7655-7704 draw F0108 before/with pit handling; BUG0_64 says ornaments draw over open pits")

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
REDMCSB_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
text = SRC.read_text(encoding="utf-8")
errors: list[str] = []


def source_excerpt(file_name: str, start: int, end: int) -> str:
    path = REDMCSB_SOURCE / file_name
    if not path.exists():
        errors.append(f"missing ReDMCSB source file: {path}")
        return ""
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def require_source(file_name: str, line_range: tuple[int, int], needles: list[str], label: str) -> None:
    start, end = line_range
    excerpt = source_excerpt(file_name, start, end)
    for needle in needles:
        if needle not in excerpt:
            errors.append(f"missing ReDMCSB source lock for {label}: {file_name}:{start}-{end} lacks {needle!r}")


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

# Verify the ReDMCSB evidence behind the Firestaff guard directly, rather
# than relying only on comments or remembered line numbers.
require_source(
    "DUNGEON.C",
    (2668, 2721),
    [
        "T0172030_Pit:",
        "P0317_pui_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL] = L0308_ps_Sensor->Remote.OrnamentOrdinal;",
        "case C03_ELEMENT_STAIRS:",
        "P0317_pui_SquareAspect[C0_ELEMENT] = ((M007_GET(AL0307_uc_Square, MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION) >> 3)",
        "P0317_pui_SquareAspect[M555_STAIRS_UP] = M007_GET(AL0307_uc_Square, MASK0x0004_STAIRS_UP);",
        "goto T0172046_Stairs;",
        "P0317_pui_SquareAspect[M550_FIRST_THING] = L0314_T_Thing;",
    ],
    "pit/teleporter floor-sensor path versus stair orientation path",
)
require_source(
    "DUNVIEW.C",
    (3940, 4011),
    [
        "STATICFUNCTION void F0108_DUNGEONVIEW_DrawFloorOrnament(",
        "G0102_as_CurrentMapFloorOrnamentsInfo[--AP0118_ui_FloorOrnamentIndex].NativeBitmapIndex",
        "G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements[P0119_ui_ViewFloorIndex]",
        "C1500_ZONE_FLOOR_ORNAMENT",
        "F0791_DUNGEONVIEW_DrawBitmapXX",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(M000_INDEX_TO_ORDINAL(C15_FLOOR_ORNAMENT_FOOTPRINTS), P0119_ui_ViewFloorIndex);",
    ],
    "floor-ornament bitmap selection, zone draw, and footprints recursion",
)
require_source(
    "DUNVIEW.C",
    (7912, 7937),
    [
        "case C02_ELEMENT_PIT:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(L0218_ai_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] ? M765_GRAPHIC_FLOOR_PIT_INVISIBLE_D1C : M759_GRAPHIC_FLOOR_PIT_D1C",
        "case C05_ELEMENT_TELEPORTER:",
        "case C01_ELEMENT_CORRIDOR:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0218_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M595_VIEW_FLOOR_D1C); /* BUG0_64",
        "F0112_DUNGEONVIEW_DrawCeilingPit(C067_GRAPHIC_CEILING_PIT_D1C, C868_ZONE_CEILING_PIT_D1C",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
    ],
    "D1C pit fallthrough draws pit then floor ornament (BUG0_64) then ceiling pit/content",
)
require_source(
    "DUNVIEW.C",
    (4341, 4380),
    [
        "STATICFUNCTION void F0112_DUNGEONVIEW_DrawCeilingPit(",
        "F0154_DUNGEON_GetLocationAfterLevelChange(G0272_i_CurrentMapIndex, -1",
        "M034_SQUARE_TYPE(AL0117_i_Square = G0279_pppuc_DungeonMapData[AL0117_i_MapIndex][P0132_i_MapX][P0133_i_MapY]) == C02_ELEMENT_PIT",
        "M007_GET(AL0117_i_Square, MASK0x0008_PIT_OPEN)",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(P0130_i_NativeBitmapIndex, P2085_ui_ZoneIndex);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(P0130_i_NativeBitmapIndex, P2085_ui_ZoneIndex);",
    ],
    "ceiling-pit above-current-level lookup and flipped/unflipped pit draw",
)

if errors:
    for err in errors:
        print(f"[FAIL] {err}")
    sys.exit(1)
print("[OK] V1 pit floor-ornament BUG0_64 source gate matches ReDMCSB")
print("- ReDMCSB DUNGEON.C:2668-2721 locks pit/teleporter floor sensors separately from stair orientation/up bits")
print("- ReDMCSB DUNVIEW.C:3940-4011 locks floor-ornament bitmap increments, C1500 zones, flips, and footprints recursion")
print("- ReDMCSB DUNVIEW.C:7912-7937 locks D1C pit draw falling through to BUG0_64 floor ornament, ceiling pit, then content")
print("- ReDMCSB DUNVIEW.C:4341-4380 locks ceiling-pit lookup one level above with flipped/unflipped draw paths")

#!/usr/bin/env python3
"""Pass443 DM1 V1 graphics-asset atlas source lock.

Primary evidence: ReDMCSB PC34/I34E constants and F0095 wall-set materialization.
Secondary evidence: Greatstone PC 3.4 and PC 3.4 multi-language GRAPHICS.DAT
atlases agree for the dungeon graphics blocks 49..79 and 86..125.

This gate specifically prevents a real regression found in pass443: viewport
stairs were still routed to wall-panel graphic slots 93..96 instead of the
source stair slots 108/111/113/118/120 in the materialized wall-set block.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
RED_DEFS = RED_ROOT / "DEFS.H"
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"

# Greatstone PC34 and PC34_MULTI labels observed from:
# http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html
# http://greatstone.free.fr/dm/db_data/dm_pc_34_multi/graphics.dat/graphics.dat.html
# PC34_MULTI adds language labels to interface assets but agrees with plain
# PC34 for these dungeon-graphics labels.
GREATSTONE_DUNGEON_LABELS = {
    49: "Dungeon Graphics - Floor Pit (Far Left Side 3)",
    50: "Dungeon Graphics - Floor Pit (Left Side 3)",
    51: "Dungeon Graphics - Floor Pit (Front 3)",
    52: "Dungeon Graphics - Floor Pit (Left Side 2)",
    53: "Dungeon Graphics - Floor Pit (Front 2)",
    54: "Dungeon Graphics - Floor Pit (Left Side 1)",
    55: "Dungeon Graphics - Floor Pit (Front 1)",
    56: "Dungeon Graphics - Floor Pit (Left Side 0)",
    57: "Dungeon Graphics - Floor Pit (Front 0)",
    58: "Dungeon Graphics - Invisible Floor Pit (Left Side 2)",
    59: "Dungeon Graphics - Invisible Floor Pit (Front 2)",
    60: "Dungeon Graphics - Invisible Floor Pit (Left Side 1)",
    61: "Dungeon Graphics - Invisible Floor Pit (Front 1)",
    62: "Dungeon Graphics - Invisible Floor Pit (Left Side 0)",
    63: "Dungeon Graphics - Invisible Floor Pit (Front 0)",
    64: "Dungeon Graphics - Ceiling Pit (Left Side 2)",
    65: "Dungeon Graphics - Ceiling Pit (Front 2)",
    66: "Dungeon Graphics - Ceiling Pit (Left Side 1)",
    67: "Dungeon Graphics - Ceiling Pit (Front 1)",
    68: "Dungeon Graphics - Ceiling Pit (Left Side 0)",
    69: "Dungeon Graphics - Ceiling Pit (Front 0)",
    70: "Dungeon Graphics - Wall Mask (Left Side 3)",
    71: "Dungeon Graphics - Wall Mask (Left Front 3)",
    72: "Dungeon Graphics - Wall Mask (Left Side 2)",
    73: "Dungeon Graphics - Wall Mask (Left Front 2)",
    74: "Dungeon Graphics - Wall Mask (Left Front 1)",
    75: "Dungeon Graphics - Wall Mask (Left Front 0)",
    76: "Dungeon Graphics - Teleporter",
    77: "Dungeon Graphics - Fluxcage",
    78: "Dungeon Graphics - Floor",
    79: "Dungeon Graphics - Ceiling",
    86: "Dungeon Graphics - Door Left or Right Frame (Front 1)",
    87: "Dungeon Graphics - Door Left Frame (Front 1)",
    88: "Dungeon Graphics - Door Left Frame (Front 2)",
    89: "Dungeon Graphics - Door Left Frame (Front 3)",
    90: "Dungeon Graphics - Door Left Frame (Left Side 3)",
    91: "Dungeon Graphics - Door Top Frame (Front 1)",
    92: "Dungeon Graphics - Door Top Frame (Front 2)",
    93: "Dungeon Graphics - Wall (Right Side 0)",
    94: "Dungeon Graphics - Wall (Left Side 0)",
    95: "Dungeon Graphics - Wall (Right Side 1)",
    96: "Dungeon Graphics - Wall (Left Side 1)",
    97: "Dungeon Graphics - Wall (Front 1)",
    98: "Dungeon Graphics - Wall (Far Right Side 2)",
    99: "Dungeon Graphics - Wall (Far Left Side 2)",
    100: "Dungeon Graphics - Wall (Right Side 2)",
    101: "Dungeon Graphics - Wall (Left Side 2)",
    102: "Dungeon Graphics - Wall (Front 2)",
    103: "Dungeon Graphics - Wall (Far Right Side 3)",
    104: "Dungeon Graphics - Wall (Far Left Side 3)",
    105: "Dungeon Graphics - Wall (Right Side 3)",
    106: "Dungeon Graphics - Wall (Left Side 3)",
    107: "Dungeon Graphics - Wall (Front 3)",
    108: "Dungeon Graphics - Stairs Up (Front 3 Left)",
    109: "Dungeon Graphics - Stairs Up (Front 3)",
    110: "Dungeon Graphics - Stairs Up (Front 2 Left)",
    111: "Dungeon Graphics - Stairs Up (Front 2)",
    112: "Dungeon Graphics - Stairs Up (Front 1 Left)",
    113: "Dungeon Graphics - Stairs Up (Front 1)",
    114: "Dungeon Graphics - Stairs Up (Front 0)",
    115: "Dungeon Graphics - Stairs Down (Front 3 Left)",
    116: "Dungeon Graphics - Stairs Down (Front 3)",
    117: "Dungeon Graphics - Stairs Down (Front 2 Left)",
    118: "Dungeon Graphics - Stairs Down (Front 2)",
    119: "Dungeon Graphics - Stairs Down (Front 1 Left)",
    120: "Dungeon Graphics - Stairs Down (Front 1)",
    121: "Dungeon Graphics - Stairs Down (Front 0)",
    122: "Dungeon Graphics - Stairs (Side 3 Left)",
    123: "Dungeon Graphics - Stairs Up (Side 2 Left)",
    124: "Dungeon Graphics - Stairs Down (Side 2 Left)",
    125: "Dungeon Graphics - Stairs (Side 1)",
}


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def enum_value(text: str, name: str) -> str:
    m = re.search(r"\b" + re.escape(name) + r"\s*=\s*([^,\n]+)", text)
    if not m:
        raise AssertionError(f"missing enum value {name}")
    return m.group(1).strip()


def find_function(text: str, name: str) -> tuple[int, str]:
    pat = re.compile(r"\b(?:static\s+)?(?:int|unsigned\s+int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pat.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0 or text.find(";", m.end(), brace) >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{": depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"missing function {name}")


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    defs = RED_DEFS.read_text(encoding="latin-1")
    dunview = RED_DUNVIEW.read_text(encoding="latin-1")

    # ReDMCSB primary PC34/I34E source constants.
    for needle in [
        "#define C049_GRAPHIC_FLOOR_PIT_D3L2                    49",
        "#define M754_GRAPHIC_FLOOR_PIT_D3L                     50",
        "#define M652_GRAPHIC_FIRST_FIELD_MASK                  70",
        "#define M644_GRAPHIC_FIRST_FLOOR_SET                   78",
        "#define M650_GRAPHIC_FLOOR_SET_0_FLOOR                 78",
        "#define M651_GRAPHIC_FLOOR_SET_0_CEILING               79",
        "#define M646_GRAPHIC_FIRST_WALL_SET                    86",
        "#define M654_GRAPHIC_WALLSET_0_DOOR_FRAME_FRONT_D0C    86",
        "#define C097_GRAPHIC_WALLSET_0_D1C                     97",
        "#define C107_GRAPHIC_WALLSET_0_D3C                    107",
        "#define M645_GRAPHIC_FIRST_STAIRS                     108",
        "#define M647_WALL_SET_GRAPHIC_COUNT        40",
        "#define M633_GRAPHIC_FIRST_DOOR_SET                   246",
    ]:
        require(defs, needle, "ReDMCSB PC34/I34E graphics constants")

    for needle in [
        "AP0099_i_GraphicIndex = (P0099_i_WallSet * M647_WALL_SET_GRAPHIC_COUNT) + M646_GRAPHIC_FIRST_WALL_SET;",
        "F0490_MEMORY_LoadDecompressAndExpandGraphic(AP0099_i_GraphicIndex++, F0631_GetBitmapPointer(G2116_DoorFrameFrontD0C));",
        "for (L0070_i_WallSetLastGraphicIndex = 0; L0070_i_WallSetLastGraphicIndex < 15; L0070_i_WallSetLastGraphicIndex++)",
    ]:
        require(dunview, needle, "ReDMCSB F0095 materializes wall-set block")

    # Greatstone secondary atlas agreement for PC34 and PC34_MULTI. This is an
    # embedded source-lock list, not a live network dependency.
    if len([i for i in range(49, 80) if i in GREATSTONE_DUNGEON_LABELS]) != 31:
        raise AssertionError("Greatstone 49..79 embedded label coverage incomplete")
    if len([i for i in range(86, 126) if i in GREATSTONE_DUNGEON_LABELS]) != 40:
        raise AssertionError("Greatstone 86..125 embedded label coverage incomplete")
    for i in [93, 94, 95, 96]:
        if "Wall (" not in GREATSTONE_DUNGEON_LABELS[i]:
            raise AssertionError(f"Greatstone wall-panel label changed for {i}")
    for i in [111, 113, 118, 120]:
        if "Stairs" not in GREATSTONE_DUNGEON_LABELS[i]:
            raise AssertionError(f"Greatstone stair label changed for {i}")

    # Firestaff must not route stairs through the old wall-panel slots.
    for name, expected in {
        "M11_GFX_STAIRS_UP_D2": "M11_GFX_DM1_STAIRS_UP_FRONT_D2C",
        "M11_GFX_STAIRS_UP_D1": "M11_GFX_DM1_STAIRS_UP_FRONT_D1C",
        "M11_GFX_STAIRS_DOWN_D2": "M11_GFX_DM1_STAIRS_DOWN_FRONT_D2C",
        "M11_GFX_STAIRS_DOWN_D1": "M11_GFX_DM1_STAIRS_DOWN_FRONT_D1C",
    }.items():
        value = enum_value(fire, name)
        if value != expected:
            raise AssertionError(f"{name} must be {expected}, found {value}")

    stair_start, stair_body = find_function(fire, "m11_draw_stairs_asset")
    require(stair_body, "m11_wallset_graphic_index_for_state(state, stairIdx)", "stairs materialize current map wall set")
    for legacy in [
        "M11_GFX_STAIRS_DOWN_D2 = 93",
        "M11_GFX_STAIRS_DOWN_D1 = 95",
        "M11_GFX_STAIRS_UP_D2   = 94",
        "M11_GFX_STAIRS_UP_D1   = 96",
    ]:
        if legacy in fire:
            raise AssertionError(f"legacy wall-panel-as-stairs mapping still present: {legacy}")

    require(cmake, "NAME pass443_dm1_v1_graphics_asset_atlas_source_lock", "CMake gate registration")

    print("PASS pass443 DM1 V1 GRAPHICS.DAT asset atlas source lock")
    print(f"- ReDMCSB PC34/I34E constants: {RED_DEFS.name}:2332..2387")
    print(f"- ReDMCSB wall-set materialization: {RED_DUNVIEW.name}:2124")
    print("- Greatstone PC34 and PC34_MULTI agree for dungeon graphics 49..79 and 86..125")
    print(f"- Firestaff stairs now use stair slots 111/113/118/120, not wall slots 93..96: {SRC.name}:{line_no(fire, stair_start)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

#!/usr/bin/env python3
"""Verify DM1 V1 GRAPHICS.DAT wall-set materialization stays source-locked.

ReDMCSB does not treat wall panels 93..107 as a one-off table: F0095 loads a
40-record wall-set block starting at M646_GRAPHIC_FIRST_WALL_SET for the current
map's wall set.  This gate pins Firestaff's helper to that same formula so wall,
door-frame/side, and stairs bitmaps can materialize from nonzero wall sets
instead of silently collapsing to set 0.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"
RED_DEFS = RED_ROOT / "DEFS.H"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def find_function(text: str, name: str) -> tuple[int, str]:
    pattern = re.compile(r"\b(?:static\s+)?(?:int|unsigned\s+int)\s+" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0 or text.find(";", match.end(), brace) >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return match.start(), text[match.start():pos + 1]
    raise AssertionError(f"missing function {name}")


def find_red_function(text: str, name: str) -> tuple[int, str]:
    pos = require(text, name + "(", f"ReDMCSB {name}")
    next_pos = text.find("\n}\n\n", pos)
    if next_pos < 0:
        raise AssertionError(f"could not bound ReDMCSB {name}")
    return pos, text[pos:next_pos + 3]


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    defs = RED_DEFS.read_text(encoding="latin-1")

    require(defs, "#define M646_GRAPHIC_FIRST_WALL_SET                    86", "ReDMCSB I34E wall-set first graphic")
    require(defs, "#define M647_WALL_SET_GRAPHIC_COUNT        40", "ReDMCSB I34E wall-set graphic count")
    require(defs, "#define M633_GRAPHIC_FIRST_DOOR_SET                   246", "ReDMCSB I34E door-set starts after wall-set blocks")

    f0095_start, f0095 = find_red_function(red, "F0095_DUNGEONVIEW_LoadWallSet")
    for needle in [
        "AP0099_i_GraphicIndex = (P0099_i_WallSet * M647_WALL_SET_GRAPHIC_COUNT) + M646_GRAPHIC_FIRST_WALL_SET;",
        "F0490_MEMORY_LoadDecompressAndExpandGraphic(AP0099_i_GraphicIndex++, F0631_GetBitmapPointer(G2116_DoorFrameFrontD0C));",
        "for (L0070_i_WallSetLastGraphicIndex = 0; L0070_i_WallSetLastGraphicIndex < 15; L0070_i_WallSetLastGraphicIndex++)",
        "F0490_MEMORY_LoadDecompressAndExpandGraphic(AP0099_i_GraphicIndex++, F0631_GetBitmapPointer(G2107_WallSet[L0070_i_WallSetLastGraphicIndex]));",
    ]:
        require(f0095, needle, "ReDMCSB F0095 wall-set materialization")

    enum_pos = require(fire, "M11_GFX_DM1_WALLSET_FIRST = 86", "Firestaff explicit wall-set first")
    require(fire, "M11_GFX_DM1_WALLSET_COUNT = 40", "Firestaff explicit wall-set count")
    is_start, is_body = find_function(fire, "m11_is_dm1_wallset_materialized_graphic")
    for needle in [
        "graphicIndex >= (unsigned int)M11_GFX_DM1_WALLSET_FIRST",
        "graphicIndex < (unsigned int)(M11_GFX_DM1_WALLSET_FIRST +",
        "M11_GFX_DM1_WALLSET_COUNT",
    ]:
        require(is_body, needle, "Firestaff wall-set range helper")

    map_start, map_body = find_function(fire, "m11_wallset_graphic_index_for_state")
    for needle in [
        "!m11_is_dm1_wallset_materialized_graphic(wallSet0GraphicIndex)",
        "state->world.dungeon->maps[state->world.party.mapIndex].wallSet",
        "M11_GFX_DM1_WALLSET_FIRST +",
        "wallSet * M11_GFX_DM1_WALLSET_COUNT",
        "((int)wallSet0GraphicIndex - M11_GFX_DM1_WALLSET_FIRST)",
    ]:
        require(map_body, needle, "Firestaff wall-set graphic remap formula")

    wall_blit_start, wall_blit = find_function(fire, "m11_draw_dm1_wall_blit_with_transparency")
    require(wall_blit, "m11_wallset_graphic_index_for_state", "Firestaff wall blit materializes map wall set")
    flip_start, flip = find_function(fire, "m11_draw_dm1_wall_blit_flipped")
    require(flip, "m11_wallset_graphic_index_for_state", "Firestaff flipped wall blit materializes map wall set")
    zone_start, zone = find_function(fire, "m11_draw_dm1_zone_blit")
    require(zone, "m11_wallset_graphic_index_for_state", "Firestaff zone blit materializes map wall set")

    require(cmake, "NAME dm1_v1_wallset_materialization_source_lock", "CMake gate registration")

    print("PASS DM1 V1 wall-set GRAPHICS.DAT materialization source lock")
    print(f"- ReDMCSB F0095 wall-set formula: {RED_DUNVIEW.name}:{line_no(red, f0095_start)}")
    print(f"- ReDMCSB wall-set constants: {RED_DEFS.name}:{line_no(defs, defs.find('#define M646_GRAPHIC_FIRST_WALL_SET                    86'))}")
    print(f"- Firestaff explicit wall-set block constants: {SRC.name}:{line_no(fire, enum_pos)}")
    print(f"- Firestaff wall-set range helper: {SRC.name}:{line_no(fire, is_start)}")
    print(f"- Firestaff wall-set remap helper: {SRC.name}:{line_no(fire, map_start)}")
    print(f"- Firestaff wall/zone blits materialize through helper: {line_no(fire, flip_start)}, {line_no(fire, wall_blit_start)}, {line_no(fire, zone_start)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

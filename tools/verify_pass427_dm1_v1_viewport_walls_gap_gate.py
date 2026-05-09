#!/usr/bin/env python3
"""Source-lock pass427 DM1 V1 far-side wall gap geometry.

D3L2/D3R2 and D2L2/D2R2 are the easiest places to regress viewport
wall parity: they are clipped edge zones, not generic side panes. This gate
pins the ReDMCSB F0676/F0677/F0678/F0679 source branches to Firestaff's
opaque wall blits, parity partner swap, side-door clipping, and same-lane
occlusion guard so later batching work cannot reopen the pass427 wall gap.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"
RED_DEFS = RED_ROOT / "DEFS.H"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    pattern = re.compile(r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pattern.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0:
            continue
        if text.find(";", m.end(), brace) >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"missing function body {name}")


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} out of order")
        last = pos


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    defs = RED_DEFS.read_text(encoding="latin-1")

    for needle in [
        "#define C702_ZONE_WALL_D3L2",
        "#define C703_ZONE_WALL_D3R2",
        "#define C707_ZONE_WALL_D2L2",
        "#define C708_ZONE_WALL_D2R2",
    ]:
        require(defs, needle, "ReDMCSB DEFS far-side wall zones")

    _, f0676 = find_function(red, "F0676_DrawD3L2")
    _, f0677 = find_function(red, "F0677_DrawD3R2")
    _, f0678 = find_function(red, "F0678_DrawD2L2")
    _, f0679 = find_function(red, "F0679_DrawD2R2")

    require_in_order(f0676, [
        "void F0676_DrawD3L2(",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C10_WALL_D3R2], C702_ZONE_WALL_D3L2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "return;",
    ], "ReDMCSB F0676 D3L2 wall branch")
    require_in_order(f0677, [
        "void F0677_DrawD3R2(",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C11_WALL_D3L2], C703_ZONE_WALL_D3R2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C10_WALL_D3R2], C703_ZONE_WALL_D3R2);",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "return;",
    ], "ReDMCSB F0677 D3R2 wall branch")
    require_in_order(f0678, [
        "void F0678_DrawD2L2(",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
        "#endif\n, C707_ZONE_WALL_D2L2);",
        "return;",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2);",
    ], "ReDMCSB F0678 D2L2 wall/field branch")
    require_in_order(f0679, [
        "void F0679_DrawD2R2(",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]",
        "#endif\n, C708_ZONE_WALL_D2R2);",
        "return;",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2);",
    ], "ReDMCSB F0679 D2R2 wall/field branch")
    require_in_order(red, [
        "F0676_DrawD3L2(P0183_i_Direction",
        "F0677_DrawD3R2(P0183_i_Direction",
        "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction",
        "F0117_DUNGEONVIEW_DrawSquareD3R(P0183_i_Direction",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction",
        "F0678_DrawD2L2(P0183_i_Direction",
        "F0679_DrawD2R2(P0183_i_Direction",
        "F0119_DUNGEONVIEW_DrawSquareD2L(P0183_i_Direction",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF(P0183_i_Direction",
        "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction",
    ], "ReDMCSB F0128 edge-wall draw order")

    side_start, side = find_function(fire, "m11_draw_dm1_side_walls")
    require_in_order(side, [
        "{3, 3, -2, M11_GFX_WALLSET0_D3L2, 0,   25, 44, 49}",
        "{3, 3,  2, M11_GFX_WALLSET0_D3R2, 180, 25, 44, 49}",
        "{3, 3, -1, M11_GFX_WALLSET0_D3L,  7,   25, 83, 49}",
        "{3, 3,  1, M11_GFX_WALLSET0_D3R,  134, 25, 83, 49}",
        "{2, 2, -2, M11_GFX_WALLSET0_D2L2, 0,   24, 8,  52}",
        "{2, 2,  2, M11_GFX_WALLSET0_D2R2, 216, 24, 8,  52}",
    ], "Firestaff far-edge side wall zones")
    require(side, "!m11_dm1_side_lane_clear_for_rel(cells", "Firestaff side wall same-lane occlusion guard")
    require(side, "partner = i ^ 1", "Firestaff parity partner swap")
    require(side, "swapped.graphicIndex = kSideBlits[partner].graphicIndex", "Firestaff parity graphic swap")
    require(side, "m11_draw_dm1_wall_blit_flipped", "Firestaff F0105 parity flip path")
    require(side, "m11_draw_dm1_wall_blit_with_transparency", "Firestaff F0104 C10-keyed path")
    # ReDMCSB DUNVIEW.C:3128/3144 (MEDIA463 includes I34E/PC34) routes side walls
    # through F0104/F0105, which call F0132_VIDEO_Blit with C10_COLOR_FLESH as the
    # transparent color.  Side panels must keep that key; only center walls
    # (F0792/F0765) draw with CM1_COLOR_NO_TRANSPARENCY.
    require(side, "10);", "Firestaff side wall path keeps C10_COLOR_FLESH transparent")

    door_start, doors = find_function(fire, "m11_draw_dm1_side_doors")
    require_in_order(doors, [
        "{3, -2, 2, {M11_GFX_DOOR_SET0_D3, 35, 0, 0,   28, 9,  38}",
        "{3,  2, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 210, 28, 14, 38}",
    ], "Firestaff D3L2/D3R2 clipped side-door zones")
    if "{2, -2" in doors or "{2,  2" in doors:
        raise AssertionError("Firestaff side doors unexpectedly added D2L2/D2R2 door specs; ReDMCSB F0678/F0679 only handle wall/teleporter")
    require(doors, "!m11_dm1_side_lane_clear_for_rel(cells", "Firestaff side doors same-lane occlusion guard")

    require(cmake, "NAME pass427_dm1_v1_viewport_walls_gap_gate", "CMake pass427 gate registration")

    print("PASS pass427 DM1 V1 viewport far-edge wall gap source lock")
    print(f"- Firestaff side wall zones: {SRC}:{line_no(fire, side_start)}")
    print(f"- Firestaff side door clipping: {SRC}:{line_no(fire, door_start)}")
    for needle in [
        "void F0676_DrawD3L2(",
        "void F0677_DrawD3R2(",
        "void F0678_DrawD2L2(",
        "void F0679_DrawD2R2(",
        "F0676_DrawD3L2(P0183_i_Direction",
    ]:
        pos = red.find(needle)
        print(f"- ReDMCSB {RED_DUNVIEW.name}:{line_no(red, pos)} {needle}")
    print(f"- ReDMCSB far-edge zones: {RED_DEFS}:{line_no(defs, defs.find('#define C702_ZONE_WALL_D3L2'))}-{line_no(defs, defs.find('#define C708_ZONE_WALL_D2R2'))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

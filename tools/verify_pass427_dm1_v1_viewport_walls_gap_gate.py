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
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
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

    # 2026-07-20 round 16 re-anchor (same-drift-family): the far-edge side
    # wall zone table moved from an m11-local kSideBlits[] literal into the
    # PC34 contract module's s_wall_draw_specs[] (DM1_VIEW_SQUARE_* /
    # DM1_WALL_* / runtime dst rows, parity partner column included), and the
    # D3L2/D3R2 clipped side-door specs moved into the side-door render plan
    # module.  Lock the same geometry at its new home.
    contract = (ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c").read_text(encoding="utf-8")
    doorplan = (ROOT / "src/dm1/dm1_v1_side_door_render_pc34_compat.c").read_text(encoding="utf-8")

    require_in_order(contract, [
        "{ DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, true,  false, DM1_PC34_ZONE_WALL_D3L2, true,  false, 3, -2, 0,   25, 44,  49",
        "{ DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, true,  false, DM1_PC34_ZONE_WALL_D3R2, true,  false, 3,  2, 180, 25, 44,  49",
        "{ DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  true,  false, DM1_PC34_ZONE_WALL_D3L,  true,  true,  3, -1, 7,   25, 83,  49",
        "{ DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  true,  false, DM1_PC34_ZONE_WALL_D3R,  true,  true,  3,  1, 134, 25, 83,  49",
        "{ DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, true,  false, DM1_PC34_ZONE_WALL_D2L2, true,  false, 2, -2, 0,   24, 8,   52",
        "{ DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, true,  false, DM1_PC34_ZONE_WALL_D2R2, true,  false, 2,  2, 216, 24, 8,   52",
    ], "Firestaff far-edge side wall zones")

    side_start, side = find_function(fire, "m11_draw_dm1_side_walls")
    # The wall pass honors only the nearest center blocker; the round-14
    # source review established that F0128 draws side walls far-to-near
    # without a nearer side-lane-open mask (that mask gates floor/content
    # passes only), so the lock now pins the center-blocker guard plus the
    # explicit source comment instead of the removed side-lane test.
    require(side, "spec->runtime_rel_forward > maxVisibleForward", "Firestaff side wall center-blocker guard")
    require(side, "without testing nearer side-lane occupancy", "Firestaff side wall same-lane occlusion source note")
    # Parity partner swap now lives in the spec table's native/parity wall
    # columns plus the shared flip decision.
    require(side, "flipWalls = m11_dm1_use_flipped_walls(state);", "Firestaff parity partner swap")
    require(contract, "dm1_viewport_3d_select_wall_bitmap(\n        spec, parity_flip, &handoff.flip_horizontally)", "Firestaff parity graphic swap")
    _, host_draw = find_function(fire, "m11_draw_dm1_side_wall_host_receipt")
    require(host_draw, "receipt->material.flip_horizontally", "Firestaff F0105 parity flip path")
    require(host_draw, "receipt->material.transparent_color >= 0", "Firestaff F0104 C10-keyed path")
    # ReDMCSB DUNVIEW.C:3128/3144 (MEDIA463 includes I34E/PC34) routes side walls
    # through F0104/F0105, which call F0132_VIDEO_Blit with C10_COLOR_FLESH as the
    # transparent color.  Side panels must keep that key; only center walls
    # (F0792/F0765) draw with CM1_COLOR_NO_TRANSPARENCY.
    require(contract, "handoff.transparent_color = 10;", "Firestaff side wall path keeps C10_COLOR_FLESH transparent")

    door_start, doors = find_function(fire, "m11_draw_dm1_side_doors")
    require_in_order(doorplan, [
        "{3, -2, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 35, 0, 0,   28, 9,  38}",
        "{3,  2, 2, {DM1_GFX_DOOR_SET0_D3_PC34, 0,  0, 210, 28, 14, 38}",
    ], "Firestaff D3L2/D3R2 clipped side-door zones")
    if "{2, -2" in doorplan or "{2,  2" in doorplan:
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

#!/usr/bin/env python3
"""Pass581: source-lock D3/D2 wall ornament draw order against ReDMCSB."""
from __future__ import annotations

import re
import sys
import os
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source",
))
FIRE = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def source_slice(path: Path, start: int, end: int) -> str:
    lines = read(path).splitlines()
    return "\n".join(lines[start - 1:end])


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def require_order(text: str, markers: list[tuple[str, str]], label: str) -> None:
    last = -1
    last_name = ""
    for name, needle in markers:
        pos = require(text, needle, f"{label} {name}")
        if pos <= last:
            raise AssertionError(f"{label}: {name} appears before/at {last_name}")
        last = pos
        last_name = name


def c_function(text: str, name: str) -> str:
    match = re.search(r"(?m)^static\s+[^\n]*\b" + re.escape(name) + r"\s*\(", text)
    if not match:
        raise AssertionError(f"missing Firestaff function {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for idx in range(brace, len(text)):
        if text[idx] == "{":
            depth += 1
        elif text[idx] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():idx + 1]
    raise AssertionError(f"unterminated Firestaff function {name}")


def main() -> int:
    dunview_path = RED / "DUNVIEW.C"
    if not dunview_path.is_file():
        print(f"SKIP ReDMCSB source unavailable: {RED}")
        return 77
    fire = read(FIRE)
    cmake = read(CMAKE)

    require_order(source_slice(dunview_path, 8488, 8521), [
        ("D3L side before D3R side", "F0116_DUNGEONVIEW_DrawSquareD3L"),
        ("D3R side before D3C center", "F0117_DUNGEONVIEW_DrawSquareD3R"),
        ("D3C center before D2L side", "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF"),
        ("D2L side before D2R side", "F0119_DUNGEONVIEW_DrawSquareD2L"),
        ("D2R side before D2C center", "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF"),
        ("D2C center closes D2 band", "F0121_DUNGEONVIEW_DrawSquareD2C"),
    ], "ReDMCSB F0128 D3/D2 far-to-near square order")

    for label, start, end, markers in [
        ("D3L", 6406, 6437, [
            ("wall panel", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);"),
            ("side ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0201_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M575_VIEW_WALL_D3L_RIGHT);"),
            ("front ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0201_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M577_VIEW_WALL_D3L_FRONT)"),
            ("alcove order", "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;"),
            ("solid wall return", "return;"),
        ]),
        ("D3R", 6545, 6573, [
            ("wall panel", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C12_WALL_D3R], C706_ZONE_WALL_D3R);"),
            ("side ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0203_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M576_VIEW_WALL_D3R_LEFT);"),
            ("front ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0203_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M579_VIEW_WALL_D3R_FRONT)"),
            ("alcove order", "L0202_i_Order = C0x0000_CELL_ORDER_ALCOVE;"),
            ("solid wall return", "return;"),
        ]),
        ("D2L", 6945, 6973, [
            ("wall panel", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C08_WALL_D2L], C710_ZONE_WALL_D2L);"),
            ("side ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0208_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M580_VIEW_WALL_D2L_RIGHT);"),
            ("front ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0208_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M582_VIEW_WALL_D2L_FRONT)"),
            ("alcove order", "L0207_i_Order = C0x0000_CELL_ORDER_ALCOVE;"),
            ("solid wall return", "return;"),
        ]),
        ("D2R", 7096, 7123, [
            ("wall panel", "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C07_WALL_D2R], C711_ZONE_WALL_D2R);"),
            ("side ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0210_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M581_VIEW_WALL_D2R_LEFT);"),
            ("front ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0210_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M584_VIEW_WALL_D2R_FRONT)"),
            ("alcove order", "L0209_i_Order = C0x0000_CELL_ORDER_ALCOVE;"),
            ("alcove draw handoff", "goto T0120029;"),
        ]),
        ("D2C", 7289, 7312, [
            ("center wall panel", "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C, G0076_B_UseFlippedWallAndFootprintsBitmaps);"),
            ("front ornament", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0212_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M583_VIEW_WALL_D2C_FRONT)"),
            ("alcove order", "L0211_i_Order = C0x0000_CELL_ORDER_ALCOVE;"),
            ("solid wall return", "return;"),
        ]),
    ]:
        require_order(source_slice(dunview_path, start, end), markers, f"ReDMCSB {label} wall ornament order")

    wall_fn = c_function(fire, "m11_draw_dm1_wall_ornaments")
    # 2026-07-20 round 15 re-anchor (same-drift-family as round 14): the
    # per-view wall ornament draw specs moved from an m11-local literal
    # table with baked coordinates into the dm1_v1_wall_ornament contract
    # module (4-field specs; coordinates come from the G0205 contract), and
    # the round-14 architecture reconciliation keeps the side-lane-open gate
    # out of wall material (nearer panels supply the source overpaint).
    wall_orn = (ROOT / "src/dm1/dm1_v1_wall_ornament_pc34_compat.c").read_text(encoding="utf-8")
    # The historical spec table opened with two extra "outer column"
    # rows -- {3, -2, 0, 1} and {3, 2, 1, 1} -- that PC34 does not carry.
    # ReDMCSB DUNVIEW.C:1061 (G0205) has exactly 13 rows on PC34, and
    # D3L2/D3R2 go through separate F0676/F0677 square passes that do
    # not consume the wall-ornament table (see the source comment at
    # the top of s_wallOrnamentViewSpecs). The gate was locking a
    # non-PC34 shape. Lock the actual PC34 13-row layout: the D3/D2 half
    # covered by this gate is the first ten entries.
    spec_block = "\n".join([
        "    {3, -1,  0, 0}, /* D3L right */",
        "    {3,  1,  1, 0}, /* D3R left */",
        "    {3, -1,  2, 0}, /* D3L front */",
        "    {3,  0,  3, 0}, /* D3C front */",
        "    {3,  1,  4, 0}, /* D3R front */",
        "    {2, -1,  5, 0}, /* D2L right */",
        "    {2,  1,  6, 0}, /* D2R left */",
        "    {2, -1,  7, 0}, /* D2L front */",
        "    {2,  0,  8, 0}, /* D2C front */",
        "    {2,  1,  9, 0}, /* D2R front */",
    ])
    require(wall_orn, spec_block, "Firestaff D3/D2 wall ornament spec order (contract module view-spec table)")
    require(wall_orn, """static const unsigned char s_wallOrnamentDrawOrderPc34[] = {
    0, 2, 1, 4, 3,  /* D3L right/front, D3R left/front, D3C front */
    5, 7, 6, 9, 8,  /* D2L right/front, D2R left/front, D2C front */""",
            "Firestaff F0128 D3/D2 completed-square ornament traversal")
    require_order(wall_fn, [
        ("center blocker limits far ornaments", "m11_viewport_cell_is_wall_like(&cells[depth][1])"),
        ("explicit maxVisibleForwardLimit replay bound", "maxVisibleForwardLimit > 0 && maxVisibleForwardLimit < maxVisibleForward"),
        ("spec table count via contract module", "dm1_v1_wall_ornament_view_spec_count_pc34()"),
        ("source draw order separated from G0205 table", "dm1_v1_wall_ornament_view_draw_order_at_pc34(i)"),
        ("do not draw beyond current visible band", "spec.relForward > maxVisibleForward"),
        ("wall-like cell guard before ornament draw", "!m11_viewport_cell_is_wall_like(&cell)"),
        ("wall panel before alcove item handoff", "if (plan->isAlcove)"),
    ], "Firestaff wall ornament occlusion envelope")

    viewport_fn = c_function(fire, "m11_draw_viewport")
    require_order(viewport_fn, [
        ("side walls first", "m11_draw_dm1_side_walls(state, framebuffer, framebufferWidth, framebufferHeight,"),
        ("front walls second", "m11_draw_dm1_front_walls(state, framebuffer, framebufferWidth, framebufferHeight, cells);"),
        ("wall ornaments after wall panels", "m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,"),
        ("blocking center detected", "int blockingCenterDepth = visibility.nearest_blocking_center_depth_index;"),
    ], "Firestaff viewport primary D3/D2 wall ornament order")
    replay = viewport_fn[require(viewport_fn, "int blockingCenterDepth = visibility.nearest_blocking_center_depth_index;", "blocking center replay block"):]
    require_order(replay, [
        ("blocking center guard", "if (blockingCenterDepth > 0)"),
        ("near-side wall replay", "m11_draw_dm1_side_walls(state, framebuffer, framebufferWidth, framebufferHeight,"),
        ("near-side ornament replay", "m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,"),
    ], "Firestaff viewport near-side wall ornament replay order")
    require(cmake, "NAME pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock", "CMake pass581 registration")

    print("PASS pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock")
    print("ReDMCSB anchors:")
    print("- DUNVIEW.C:6406-6437 F0116 D3L wall: panel -> side ornament -> front ornament/alcove -> return")
    print("- DUNVIEW.C:6545-6573 F0117 D3R wall: panel -> side ornament -> front ornament/alcove -> return")
    print("- DUNVIEW.C:6945-6973 F0119 D2L wall: panel -> side ornament -> front ornament/alcove -> return")
    print("- DUNVIEW.C:7096-7123 F0120 D2R wall: panel -> side ornament -> front ornament/alcove -> F0115 handoff")
    print("- DUNVIEW.C:7289-7312 F0121 D2C wall: center panel -> front ornament/alcove -> return")
    print("- DUNVIEW.C:8488-8521 F0128 D3L,D3R,D3C,D2L,D2R,D2C traversal")
    print("Firestaff anchors:")
    print("- m11_game_view.c:m11_draw_dm1_wall_ornaments D3/D2 spec order and visibility guards")
    print("- m11_game_view.c:m11_draw_viewport wall panels before wall ornaments, then near-side replay after D2/D3 center blockers")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL pass581_dm1_v1_d3_d2_wall_ornament_order_source_lock: {exc}", file=sys.stderr)
        raise SystemExit(1)

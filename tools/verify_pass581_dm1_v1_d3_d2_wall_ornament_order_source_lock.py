#!/usr/bin/env python3
"""Pass581: source-lock D3/D2 wall ornament draw order against ReDMCSB."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
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
    require_order(wall_fn, [
        ("D3L2 right side", "{3,-2,0,1,{0,0,0,26,  21,10,42}}"),
        ("D3R2 left side", "{3, 2,1,1,{0,0,0,187, 22,10,42}}"),
        ("D3L right side", "{3,-1,2,0,{0,0,0,80,  22,10,42}}"),
        ("D3R left side", "{3, 1,4,1,{0,0,0,134, 22,10,42}}"),
        ("D3L front", "{3,-1,2,0,{0,0,0,0,   16,90,56}}"),
        ("D3C front", "{3, 0,3,0,{0,0,0,67,  16,90,56}}"),
        ("D3R front", "{3, 1,4,0,{0,0,0,135, 16,89,56}}"),
        ("D2L right side", "{2,-1,5,0,{0,0,0,66,  24,10,42}}"),
        ("D2R left side", "{2, 1,6,1,{0,0,0,149, 24,10,42}}"),
        ("D2L front", "{2,-1,7,0,{0,35,0,0,  19,55,56}}"),
        ("D2C front", "{2, 0,8,0,{0,0,0,67,  19,90,56}}"),
        ("D2R front", "{2, 1,9,0,{0,0,0,169, 19,55,56}}"),
    ], "Firestaff D3/D2 wall ornament spec order")
    require_order(wall_fn, [
        ("center blocker limits far ornaments", "m11_viewport_cell_is_wall_like(&cells[depth][1])"),
        ("explicit maxVisibleForwardLimit replay bound", "maxVisibleForwardLimit > 0 && maxVisibleForwardLimit < maxVisibleForward"),
        ("do not draw beyond current visible band", "kWallOrnaments[i].relForward > maxVisibleForward"),
        ("side lane blocker guard", "m11_dm1_side_lane_clear_for_rel(cells,"),
        ("only wall square ornaments", "cell.elementType != DUNGEON_ELEMENT_WALL"),
        ("wall panel before alcove item handoff", "m11_dm1_wall_ornament_is_alcove_global(ornGlobalIdx)"),
    ], "Firestaff wall ornament occlusion envelope")

    viewport_fn = c_function(fire, "m11_draw_viewport")
    require_order(viewport_fn, [
        ("side walls first", "m11_draw_dm1_side_walls(state, framebuffer, framebufferWidth, framebufferHeight,"),
        ("front walls second", "m11_draw_dm1_front_walls(state, framebuffer, framebufferWidth, framebufferHeight, cells);"),
        ("wall ornaments after wall panels", "m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,"),
        ("blocking center detected", "int blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);"),
    ], "Firestaff viewport primary D3/D2 wall ornament order")
    replay = viewport_fn[require(viewport_fn, "int blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);", "blocking center replay block"):]
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

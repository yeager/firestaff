#!/usr/bin/env python3
"""Verify center-door adornments cannot bleed through nearer center blockers.

ReDMCSB draws each center square as a complete D3/D2/D1 square; once a nearer
non-open center square exists, deeper center-door ornaments, destroyed masks, and
buttons are not independently eligible to paint over it. Firestaff previously
searched those adornment passes independently after drawing the nearest center
door panel, which allowed D2/D3 door details to overlay a nearer D1 blocker.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source/DUNVIEW.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated {name}")


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
            raise AssertionError(f"{label}: marker out of order {marker!r}")
        last = pos


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    red = RED.read_text(encoding="latin-1")
    cmake = CMAKE.read_text(encoding="utf-8")

    red_d1_start = red.rfind("STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(")
    red_d2_start = red.find("STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(")
    red_d3_start = red.find("STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(")
    if min(red_d1_start, red_d2_start, red_d3_start) < 0:
        raise AssertionError("missing ReDMCSB center-square definition")
    red_d1 = red[red_d1_start:red.find("STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L", red_d1_start)]
    red_d2 = red[red_d2_start:red.find("STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L", red_d2_start)]
    red_d3 = red[red_d3_start:red.find("STATICFUNCTION void F0119_DUNGEONVIEW_DrawSquareD2L", red_d3_start)]
    require_in_order(red_d1, [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING]",
        "F0110_DUNGEONVIEW_DrawDoorButton",
        "F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]",
    ], "ReDMCSB D1C center-door square body")
    require_in_order(red_d2, [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING]",
        "F0110_DUNGEONVIEW_DrawDoorButton",
        "F0111_DUNGEONVIEW_DrawDoor(L0212_ai_SquareAspect[M557_DOOR_THING_INDEX]",
    ], "ReDMCSB D2C center-door square body")
    require_in_order(red_d3, [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0206_ai_SquareAspect[M550_FIRST_THING]",
        "F0110_DUNGEONVIEW_DrawDoorButton",
        "F0111_DUNGEONVIEW_DrawDoor(L0206_ai_SquareAspect[M557_DOOR_THING_INDEX]",
    ], "ReDMCSB D3C center-door square body")

    # 2026-07-20 round 16 re-anchor (same-drift-family): the m11-local
    # nearest-blocking-center-door helper was superseded by the F0111
    # far-to-near composition (commit a5612d142, gated by
    # verify_dm1_v1_f0111_center_door_order_source_gate.py): every center
    # door material route draws D3C -> D2C -> D1C and the closer source
    # panel overpaints the farther one.  The nearest-blocking-door depth
    # itself now lives in the shared PC34 visibility receipt.
    contract = (ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c").read_text(encoding="utf-8")
    helper_start, helper = find_function(contract, "dm1_viewport_3d_nearest_blocking_center_door_depth_pc34")
    require_in_order(helper, [
        "unsigned int blocking_door_depth_mask)",
        "int nearest = dm1_viewport_3d_nearest_blocking_center_depth_index_pc34(",
        "return (blocking_door_depth_mask & (1u << (unsigned int)nearest)) != 0u",
    ], "Firestaff nearest blocking center-door helper")
    require(contract, "receipt.nearest_blocking_center_door_depth =", "Firestaff visibility receipt carries nearest blocking center-door depth")

    for fn, markers in [
        ("m11_draw_dm1_center_door_ornaments", [
            "F0111 owns the ornament in each D3C/D2C/D1C door panel.",
            "for (depth = 2; depth >= 0; --depth)",
            "cell->elementType != DUNGEON_ELEMENT_DOOR",
            "m11_viewport_cell_is_open(cell)",
        ]),
        ("m11_draw_dm1_center_destroyed_door_masks", [
            "F0111 applies destroyed masks to each source door panel",
            "for (depth = 2; depth >= 0; --depth)",
            "cell->elementType != DUNGEON_ELEMENT_DOOR",
            "cell->doorState != 5",
        ]),
        ("m11_draw_dm1_center_door_buttons", [
            "F0110 is reached from each F0118/F0121/F0124 door-front route.",
            "for (depth = 2; depth >= 0; --depth)",
            "cell->elementType != DUNGEON_ELEMENT_DOOR",
            "!cell->hasDoorThing",
        ]),
    ]:
        _start, body = find_function(fire, fn)
        require_in_order(body, markers, f"Firestaff {fn} far-to-near overpaint gate")
        if "for (depth = 0; depth < 3; ++depth)" in body:
            raise AssertionError(f"{fn}: reverted to independent near-first depth scan")

    d3r_start, d3r = find_function(fire, "m11_draw_dm1_d3r_door_button")
    require_in_order(d3r, [
        "int maxVisibleForward,",
        "const M11_ViewportCell cells[3][3]",
        "if (3 > maxVisibleForward ||",
        "!m11_dm1_side_lane_clear_for_rel(cells, 3, 1)",
        "m11_sample_viewport_cell(state, 3, 1, &cell)",
    ], "Firestaff D3R door-button side-lane occlusion")

    callback_start, callback = find_function(
        fire, "m11_dm1_f0128_execute_source_step")
    require_in_order(callback, [
        "M11_DM1_F0128_EXECUTE_DOOR_FRAME",
        "M11_DM1_F0128_EXECUTE_DOOR_BUTTON",
        "m11_draw_dm1_d3r_door_button(",
        "m11_draw_dm1_center_door_buttons(",
        "M11_DM1_F0128_EXECUTE_DOOR_MATERIAL",
    ], "Firestaff callback-owned F0110 order")
    require(callback,
            "m11_draw_dm1_d3r_door_button(\n"
            "                dispatch->state, dispatch->framebuffer,\n"
            "                dispatch->framebufferWidth, dispatch->framebufferHeight,\n"
            "                relForward, dispatch->cells);",
            "Firestaff callback-owned D3R button call")
    draw_start, draw = find_function(fire, "m11_draw_viewport")
    if "m11_draw_dm1_d3r_door_button(" in draw:
        raise AssertionError("viewport revives direct D3R F0110 replay")
    require(cmake, "NAME v1_viewport_center_door_occlusion_gate", "CMake test registration")

    print("V1 viewport center-door occlusion gate passed")
    print(f"- Firestaff nearest center-door helper: src/dm1/dm1_v1_viewport_3d_pc34_compat.c:{line_no(contract, helper_start)}")
    print(f"- Firestaff D3R side-button guard: {SRC}:{line_no(fire, d3r_start)}")
    print(f"- Firestaff callback call-site: {SRC}:{line_no(fire, callback_start)}")
    for pos, needle in [
        (red_d1_start, "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C("),
        (red_d2_start, "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C("),
        (red_d3_start, "STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF("),
    ]:
        print(f"- ReDMCSB {RED.name}:{line_no(red, pos)} {needle}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

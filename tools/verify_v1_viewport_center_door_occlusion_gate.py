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
SRC = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C")


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

    helper_start, helper = find_function(fire, "m11_dm1_nearest_blocking_center_door_depth")
    require_in_order(helper, [
        "int depth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "if (depth < 0)",
        "cells[depth][1].elementType != DUNGEON_ELEMENT_DOOR",
        "m11_viewport_cell_is_open(&cells[depth][1])",
        "return depth;",
    ], "Firestaff nearest blocking center-door helper")

    for fn in [
        "m11_draw_dm1_center_door_ornaments",
        "m11_draw_dm1_center_destroyed_door_masks",
        "m11_draw_dm1_center_door_buttons",
    ]:
        _start, body = find_function(fire, fn)
        require_in_order(body, [
            "depth = m11_dm1_nearest_blocking_center_door_depth(cells);",
            "if (depth < 0)",
            "&cells[depth][1]",
        ], f"Firestaff {fn} nearest-door gate")
        if "for (depth = 0; depth < 3; ++depth)" in body:
            raise AssertionError(f"{fn}: reverted to independent depth scan")

    d3r_start, d3r = find_function(fire, "m11_draw_dm1_d3r_door_button")
    require_in_order(d3r, [
        "int maxVisibleForward,",
        "const M11_ViewportCell cells[3][3]",
        "if (3 > maxVisibleForward ||",
        "!m11_dm1_side_lane_clear_for_rel(cells, 3, 1)",
        "m11_sample_viewport_cell(state, 3, 1, &cell)",
    ], "Firestaff D3R door-button side-lane occlusion")

    draw_start, draw = find_function(fire, "m11_draw_viewport")
    require(draw, "m11_draw_dm1_d3r_door_button(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  maxVisibleForward, cells);", "Firestaff D3R button call")
    require(cmake, "NAME v1_viewport_center_door_occlusion_gate", "CMake test registration")

    print("V1 viewport center-door occlusion gate passed")
    print(f"- Firestaff nearest center-door helper: {SRC}:{line_no(fire, helper_start)}")
    print(f"- Firestaff D3R side-button guard: {SRC}:{line_no(fire, d3r_start)}")
    print(f"- Firestaff viewport call-site: {SRC}:{line_no(fire, draw_start)}")
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

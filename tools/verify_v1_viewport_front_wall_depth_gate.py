#!/usr/bin/env python3
"""Verify DM1 V1 front-wall depth/occlusion stays source-locked.

ReDMCSB draws center wall squares through complete D3C, D2C, then D1C
square routines.  Each center wall branch draws the source wall bitmap/zone and
returns from that square routine.  Because those front-wall panels are opaque,
Firestaff may collapse the final visible front-wall result to the nearest
wall-like center cell, but it must not draw farther D2C/D3C walls over a nearer
D1C/D2C wall and it must keep the source D1C/D2C/D3C graphics/zones.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
RED_DUNGEON = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNGEON.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0:
            continue
        if text.find(";", match.end(), brace) >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    return match.start(), pos + 1, text[match.start():pos + 1]
    raise AssertionError(f"missing Firestaff function {name}")


def find_red_region(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r"(?m)^[ \t]*(?:STATICFUNCTION\s+void|void)\s+" + re.escape(name) + r"\s*\(")
    next_pattern = re.compile(r"(?m)^[ \t]*(?:STATICFUNCTION\s+void|void)\s+F\d{4}_")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0 or text.find(";", match.end(), brace) >= 0:
            continue
        line_start = text.rfind("\n", 0, match.start()) + 1
        next_match = next_pattern.search(text, brace + 1)
        end = next_match.start() if next_match else len(text)
        return line_start, end, text[line_start:end]
    raise AssertionError(f"missing ReDMCSB region {name}")


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_in_order(text: str, markers: list[tuple[str, str]], label: str) -> None:
    last = -1
    last_name = ""
    for name, needle in markers:
        pos = require(text, needle, label)
        if pos <= last:
            raise AssertionError(f"{label}: {name} appears before/at {last_name}")
        last = pos
        last_name = name


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    dungeon = RED_DUNGEON.read_text(encoding="latin-1")

    f0128_start, _f0128_end, f0128 = find_red_region(red, "F0128_DUNGEONVIEW_Draw_CPSF")
    require_in_order(
        f0128,
        [
            ("D3C center square", "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF"),
            ("D2C center square", "F0121_DUNGEONVIEW_DrawSquareD2C"),
            ("D1C center square", "F0124_DUNGEONVIEW_DrawSquareD1C"),
        ],
        "ReDMCSB F0128 center draw order",
    )

    red_specs = [
        ("D3C", "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "case C00_ELEMENT_WALL:", "G2107_WallSet[C14_WALL_D3C]", "C704_ZONE_WALL_D3C", "return;"),
        ("D2C", "F0121_DUNGEONVIEW_DrawSquareD2C", "case C00_ELEMENT_WALL:", "G2107_WallSet[C09_WALL_D2C]", "C709_ZONE_WALL_D2C", "return;"),
        ("D1C", "F0124_DUNGEONVIEW_DrawSquareD1C", "case C00_ELEMENT_WALL:", "G2107_WallSet[C04_WALL_D1C]", "C712_ZONE_WALL_D1C", "return;"),
    ]
    red_lines: list[str] = []
    for label, fn, wall_case, graphic, zone, ret in red_specs:
        start, _end, body = find_red_region(red, fn)
        wall_pos = require(body, wall_case, f"ReDMCSB {label} wall branch")
        graphic_pos = require(body, graphic, f"ReDMCSB {label} wall branch")
        zone_pos = require(body, zone, f"ReDMCSB {label} wall branch")
        ret_pos = require(body, ret, f"ReDMCSB {label} wall branch")
        if not (wall_pos < graphic_pos < ret_pos and wall_pos < zone_pos < ret_pos):
            raise AssertionError(f"ReDMCSB {label} wall graphic/zone are not inside wall-return branch")
        red_lines.append(f"ReDMCSB {fn}: {RED_DUNVIEW.name}:{line_no(red, start)}")

    fake_case = require(dungeon, "case C06_ELEMENT_FAKEWALL:", "ReDMCSB F0172 fakewall aspect")
    fake_region = dungeon[fake_case:dungeon.find("case C05_ELEMENT_TELEPORTER:", fake_case)]
    require(fake_region, "P0317_pui_SquareAspect[C0_ELEMENT] = C00_ELEMENT_WALL;", "ReDMCSB F0172 closed fakewall aspect")
    require(fake_region, "P0317_pui_SquareAspect[C0_ELEMENT] = C01_ELEMENT_CORRIDOR;", "ReDMCSB F0172 open fakewall aspect")

    wall_like_start, _wall_like_end, wall_like = find_function(fire, "m11_viewport_cell_is_wall_like")
    for token in ["DUNGEON_ELEMENT_WALL", "DUNGEON_ELEMENT_FAKEWALL"]:
        require(wall_like, token, "Firestaff wall-like helper")

    start, _end, body = find_function(fire, "m11_draw_dm1_front_walls")
    require_in_order(
        body,
        [
            ("D1C wall blit", "{0, 1, 0, M11_GFX_WALLSET0_D1C, 32, 9, 160, 111}"),
            ("D2C wall blit", "{1, 2, 0, M11_GFX_WALLSET0_D2C, 59, 19, 106, 74}"),
            ("D3C wall blit", "{2, 3, 0, M11_GFX_WALLSET0_D3C, 77, 25, 70, 49}"),
            ("near-to-far scan", "for (depth = 0; depth < 3; ++depth)"),
            ("occlusion break before sampling", "if (occluded)"),
            ("wall-like center test", "m11_viewport_cell_is_wall_like(&cells[depth][1])"),
            ("source wall blit", "m11_draw_dm1_front_wall_blit"),
            ("nearest wall stops farther front walls", "occluded = 1;"),
        ],
        "Firestaff front-wall nearest-depth collapse",
    )

    draw_start, _draw_end, draw = find_function(fire, "m11_draw_viewport")
    require_in_order(
        draw,
        [
            ("sample cells", "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])"),
            ("derive center max", "maxVisibleForward = m11_dm1_max_visible_forward_from_center(cells);"),
            ("draw side walls", "m11_draw_dm1_side_walls"),
            ("draw front walls", "m11_draw_dm1_front_walls(state, framebuffer, framebufferWidth, framebufferHeight, cells);"),
            ("draw wall ornaments", "m11_draw_dm1_wall_ornaments"),
        ],
        "Firestaff viewport front-wall call site",
    )
    require(cmake, "NAME v1_viewport_front_wall_depth_gate", "CMake test registration")

    print("V1 viewport front-wall depth gate passed")
    print(f"- ReDMCSB F0128 center order: {RED_DUNVIEW.name}:{line_no(red, f0128_start)}")
    for line in red_lines:
        print(f"- {line}")
    print(f"- ReDMCSB fakewall-as-wall aspect evidence: {RED_DUNGEON.name}:2651")
    print(f"- Firestaff wall-like helper: {SRC.name}:{line_no(fire, wall_like_start)}")
    print(f"- Firestaff nearest front-wall depth collapse: {SRC.name}:{line_no(fire, start)}")
    print(f"- Firestaff viewport call site: {SRC.name}:{line_no(fire, draw_start)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

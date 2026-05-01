#!/usr/bin/env python3
"""Verify DM1 V1 side wall-ornament source selection stays ReDMCSB-locked.

The side-wall occlusion gate proves farther side work is clipped by nearer
non-open side squares.  This gate covers the adjacent ornament-selection detail:
ReDMCSB F0107 draws D2R_LEFT by reusing the base side ornament bitmap with a
horizontal flip, not by advancing to the front-facing ornament bitmap variant.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_c_function(text: str, name: str) -> tuple[int, str]:
    pat = re.compile(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    m = pat.search(text)
    if not m:
        raise AssertionError(f"missing Firestaff function {name}")
    brace = text.find("{", m.end())
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():i + 1]
    raise AssertionError(f"unterminated Firestaff function {name}")


def find_red_region(text: str, name: str) -> tuple[int, str]:
    pat = re.compile(r"(?m)^STATICFUNCTION\s+BOOLEAN\s+" + re.escape(name) + r"\s*\(")
    m = pat.search(text)
    if not m:
        raise AssertionError(f"missing ReDMCSB region {name}")
    next_pat = re.compile(r"(?m)^STATICFUNCTION\s+(?:void|BOOLEAN)\s+F\d{4}_")
    n = next_pat.search(text, m.end())
    end = n.start() if n else len(text)
    return m.start(), text[m.start():end]


def require(body: str, needle: str, label: str) -> int:
    pos = body.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def require_order(body: str, markers: list[tuple[str, str]], label: str) -> None:
    last = -1
    last_name = ""
    for name, needle in markers:
        pos = require(body, needle, f"{label} {name}")
        if pos <= last:
            raise AssertionError(f"{label}: {name} appears before/at {last_name}")
        last = pos
        last_name = name


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
    red = DUNVIEW.read_text(encoding="utf-8", errors="replace")
    cmake = CMAKE.read_text(encoding="utf-8")

    red_start, f0107 = find_red_region(red, "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF")
    require_order(
        f0107,
        [
            ("base native bitmap selected", "AL0088_i_NativeBitmapIndex = G0101_as_CurrentMapWallOrnamentsInfo"),
            ("native D1/front increment branch", "AL0088_i_NativeBitmapIndex++;"),
            ("D1R side is flipped", "if (P0117_i_ViewWallIndex == M586_VIEW_WALL_D1R_LEFT)"),
            ("D2R/D3R side flip test", "(P0117_i_ViewWallIndex == M581_VIEW_WALL_D2R_LEFT) || (P0117_i_ViewWallIndex == M576_VIEW_WALL_D3R_LEFT)"),
            ("front-only side-depth native increment", "if ((P0117_i_ViewWallIndex >= M577_VIEW_WALL_D3L_FRONT) && (P0117_i_ViewWallIndex != M580_VIEW_WALL_D2L_RIGHT))"),
        ],
        "ReDMCSB F0107 side ornament bitmap selection",
    )

    fire_start, wall = find_c_function(fire, "m11_draw_dm1_wall_ornaments")
    require(wall, "{2,-1,5,0,{0,0,0,66,  24,10,42}}", "D2L_RIGHT side ornament spec")
    require(wall, "{2, 1,6,1,{0,0,0,149, 24,10,42}}", "D2R_LEFT flipped side ornament spec")
    require(wall, "{2,-1,7,0,{0,35,0,0,  19,55,56}}", "D2L_FRONT front-variant spec")
    require_order(
        wall,
        [
            ("ReDMCSB native-offset comment", "D2R_LEFT reuses the base"),
            ("D2L_RIGHT excluded", "kWallOrnaments[i].viewWallIndex != 5"),
            ("D2R_LEFT excluded", "kWallOrnaments[i].viewWallIndex != 6"),
            ("graphic index uses native offset", "M11_GFX_WALL_ORNAMENT_BASE + ornGlobalIdx * 2 + nativeOffset"),
            ("flip passed to blitter", "kWallOrnaments[i].flipHorizontal"),
        ],
        "Firestaff side ornament native-offset/flip mapping",
    )
    require(cmake, "NAME v1_viewport_side_wall_ornament_source_gate", "CMake test registration")

    print("V1 viewport side wall-ornament source gate passed")
    print(f"- ReDMCSB {DUNVIEW.name}:{line_no(red, red_start)} F0107 side ornament native/flip branch")
    print(f"- Firestaff {FIRE.name}:{line_no(fire, fire_start)} D2R_LEFT uses base native bitmap plus horizontal flip")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

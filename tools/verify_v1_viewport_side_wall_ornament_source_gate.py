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
FIRE = ROOT / "src/engine/m11_game_view.c"
DM1_WALL_ORN = ROOT / "src/dm1/dm1_v1_wall_ornament_pc34_compat.c"
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


def find_c_array(text: str, name: str) -> tuple[int, str]:
    m = re.search(re.escape(name) + r"\[\]\s*=\s*\{", text)
    if not m:
        raise AssertionError(f"missing Firestaff array {name}")
    depth = 0
    for i in range(m.end() - 1, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():i + 1]
    raise AssertionError(f"unterminated Firestaff array {name}")


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
    # M11 only consumes the DM1-owned specs and host material receipts.
    require(wall, "dm1_v1_wall_ornament_view_spec_pc34(i, &spec)", "DM1 view-spec consumption")
    require(wall, "dm1_v1_wall_ornament_host_material_receipt_pc34(", "DM1 host material receipt consumption")

    dm1 = DM1_WALL_ORN.read_text(encoding="utf-8")
    spec_pos, spec_table = find_c_array(dm1, "s_wallOrnamentViewSpecs")
    require(spec_table, "{2, -1,  5, 0},", "D2L_RIGHT side ornament spec")
    require(spec_table, "{2,  1,  6, 0},", "D2R_LEFT side ornament spec (flip via F0107 rule)")
    require(spec_table, "{2, -1,  7, 0},", "D2L_FRONT front-variant spec")

    _, flip_fn = find_c_function(dm1, "dm1_v1_wall_ornament_flip_horizontal_pc34")
    require(flip_fn, "flipped only for right-side left-wall projections", "F0107 flip provenance")
    require(flip_fn, "return viewWallIndex == 1 || viewWallIndex == 6 || viewWallIndex == 11;", "F0107 D3R/D2R/D1R_LEFT flip rule")

    _, plan_fn = find_c_function(dm1, "dm1_v1_wall_ornament_render_plan_pc34")
    require_order(
        plan_fn,
        [
            ("ReDMCSB native-offset comment", "increments the native wall-ornament bitmap"),
            ("D2L_RIGHT excluded", "viewWallIndex != 5 &&"),
            ("D2R_LEFT excluded", "viewWallIndex != 6) ? 1 : 0;"),
            ("graphic index uses native offset", "DM1_GFX_WALL_ORNAMENT_BASE_PC34 + globalIndex * 2 + nativeOffset"),
            ("flip passed to plan", "dm1_v1_wall_ornament_flip_horizontal_pc34(viewWallIndex)"),
        ],
        "Firestaff side ornament native-offset/flip mapping",
    )
    require(cmake, "NAME v1_viewport_side_wall_ornament_source_gate", "CMake test registration")

    print("V1 viewport side wall-ornament source gate passed")
    print(f"- ReDMCSB {DUNVIEW.name}:{line_no(red, red_start)} F0107 side ornament native/flip branch")
    print(f"- Firestaff {DM1_WALL_ORN.name}:{line_no(dm1, spec_pos)} D2R_LEFT uses base native bitmap plus F0107 horizontal flip")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

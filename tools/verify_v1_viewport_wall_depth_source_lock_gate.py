#!/usr/bin/env python3
"""Source-lock DM1 V1 viewport wall depth/occlusion/draw-order evidence.

This is a narrow source-shape gate.  It ties Firestaff's split M11 wall passes
back to ReDMCSB's relative-square sampling, far-to-near DUNVIEW draw sequence,
and viewport blit path without touching the live renderer.
"""
from __future__ import annotations

from pathlib import Path
import os
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIRESTAFF = ROOT / "src/engine/m11_game_view.c"
DEFAULT_REDMCSB_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REDMCSB_ROOT = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", DEFAULT_REDMCSB_ROOT))
DUNVIEW = REDMCSB_ROOT / "DUNVIEW.C"
DRAWVIEW = REDMCSB_ROOT / "DRAWVIEW.C"
DUNGEON = REDMCSB_ROOT / "DUNGEON.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def find_function(text: str, name: str, *, allow_kr: bool = False) -> tuple[int, int, str]:
    pattern = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0:
            continue
        if not allow_kr:
            semi = text.find(";", match.end(), brace)
            if semi >= 0:
                continue
        depth = 0
        for pos in range(brace, len(text)):
            if text[pos] == "{":
                depth += 1
            elif text[pos] == "}":
                depth -= 1
                if depth == 0:
                    start = text.rfind("\n", 0, match.start()) + 1
                    return start, pos + 1, text[start:pos + 1]
    raise AssertionError(f"missing function body for {name}")


def find_red_function(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r"(?m)^(?:STATICFUNCTION\s+void|void|int16_t|unsigned char)\s+" + re.escape(name) + r"\s*\(")
    matches = list(pattern.finditer(text))
    if not matches:
        raise AssertionError(f"missing ReDMCSB function region for {name}")
    match = matches[-1]
    next_pattern = re.compile(r"(?m)^(?:STATICFUNCTION\s+void|void|int16_t|unsigned char)\s+F\d{4}_")
    next_match = next_pattern.search(text, match.end())
    end = next_match.start() if next_match else len(text)
    return match.start(), end, text[match.start():end]

def require_in_order(body: str, markers: list[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    out: list[tuple[str, int]] = []
    last = -1
    last_name = ""
    for name, needle in markers:
        pos = body.find(needle)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name}: {needle!r}")
        if pos <= last:
            raise AssertionError(f"{label}: {name} appears before/at {last_name}")
        out.append((name, pos))
        last = pos
        last_name = name
    return out


def require_tokens(body: str, tokens: list[str], label: str) -> None:
    for token in tokens:
        if token not in body:
            raise AssertionError(f"{label}: missing {token!r}")


def main() -> int:
    red_dunview = read(DUNVIEW)
    red_drawview = read(DRAWVIEW)
    red_dungeon = read(DUNGEON)
    fire = read(FIRESTAFF)
    ok: list[str] = []

    # DUNGEON.C: source relative-square contract.  Out-of-bounds squares are
    # walls; relative samples first transform from party direction/forward/side.
    d151_start, _d151_end, d151 = find_red_function(red_dungeon, "F0151_DUNGEON_GetSquare")
    require_tokens(d151, [
        "returned square type will be C00_ELEMENT_WALL",
        "return M035_SQUARE(C00_ELEMENT_WALL",
        "G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY]",
    ], "ReDMCSB absolute square wall fallback")
    d152_start, _d152_end, d152 = find_red_function(red_dungeon, "F0152_DUNGEON_GetRelativeSquare")
    require_in_order(d152, [
        ("relative movement transform", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement"),
        ("absolute square sample", "return F0151_DUNGEON_GetSquare"),
    ], "ReDMCSB relative square sampling")
    d153_start, _d153_end, d153 = find_red_function(red_dungeon, "F0153_DUNGEON_GetRelativeSquareType")
    require_tokens(d153, ["M034_SQUARE_TYPE", "F0152_DUNGEON_GetRelativeSquare"], "ReDMCSB relative square type")
    ok.append(
        f"DUNGEON relative samples/wall fallback: DUNGEON.C:{line_no(red_dungeon, d151_start)}-1505 "
        f"(F0151:{line_no(red_dungeon, d151_start)}, F0152:{line_no(red_dungeon, d152_start)}, F0153:{line_no(red_dungeon, d153_start)})"
    )

    # DUNVIEW.C: the original viewport draws far-to-near by complete square.
    f128_start, _f128_end, f128 = find_red_function(red_dunview, "F0128_DUNGEONVIEW_Draw_CPSF")
    require_in_order(f128, [
        ("D3 left", "F0116_DUNGEONVIEW_DrawSquareD3L"),
        ("D3 right", "F0117_DUNGEONVIEW_DrawSquareD3R"),
        ("D3 center", "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF"),
        ("D2 left", "F0119_DUNGEONVIEW_DrawSquareD2L"),
        ("D2 right", "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF"),
        ("D2 center", "F0121_DUNGEONVIEW_DrawSquareD2C"),
        ("D1 left", "F0122_DUNGEONVIEW_DrawSquareD1L"),
        ("D1 right", "F0123_DUNGEONVIEW_DrawSquareD1R"),
        ("D1 center", "F0124_DUNGEONVIEW_DrawSquareD1C"),
        ("D0 left", "F0125_DUNGEONVIEW_DrawSquareD0L"),
        ("D0 right", "F0126_DUNGEONVIEW_DrawSquareD0R"),
        ("D0 center", "F0127_DUNGEONVIEW_DrawSquareD0C"),
    ], "ReDMCSB F0128 far-to-near square draw order")
    ok.append(f"DUNVIEW far-to-near draw order: DUNVIEW.C:{line_no(red_dunview, f128_start)}-8542")

    for fn, bitmap in [
        ("F0124_DUNGEONVIEW_DrawSquareD1C", "G0700_puc_Bitmap_WallSet_Wall_D1LCR"),
        ("F0121_DUNGEONVIEW_DrawSquareD2C", "G0699_puc_Bitmap_WallSet_Wall_D2LCR"),
        ("F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "G0698_puc_Bitmap_WallSet_Wall_D3LCR"),
    ]:
        start, _end, body = find_red_function(red_dunview, fn)
        require_in_order(body, [
            ("wall element case", "case C00_ELEMENT_WALL:"),
            ("wall bitmap", bitmap),
            ("wall branch returns", "return;"),
        ], f"ReDMCSB {fn} center wall occlusion")
        ok.append(f"DUNVIEW {fn} wall branch returns after opaque wall: DUNVIEW.C:{line_no(red_dunview, start)}")

    # DRAWVIEW.C: the viewport buffer drawn by DUNVIEW is what reaches the screen.
    draw_start, _draw_end, draw = find_red_function(red_drawview, "F0097_DUNGEONVIEW_DrawViewport")
    require_in_order(draw, [
        ("viewport request flag", "G0324_B_DrawViewportRequested"),
        ("wait for vertical blank", "M526_WaitVerticalBlank"),
        ("blit viewport bitmap", "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport"),
    ], "ReDMCSB viewport presentation path")
    ok.append(f"DRAWVIEW viewport presentation path: DRAWVIEW.C:{line_no(red_drawview, draw_start)}-861")

    # Firestaff: split renderer must preserve the source depth/occlusion shape.
    max_start, _max_end, max_body = find_function(fire, "m11_dm1_max_visible_forward_from_center")
    require_in_order(max_body, [
        ("scan far depths", "for (depth = 0; depth < 3; ++depth)"),
        ("non-open center blocker", "!m11_viewport_cell_is_open(&cells[depth][1])"),
        ("nearest visible forward bound", "return depth + 1;"),
    ], "Firestaff center-line max-visible helper")
    front_blit_start, _front_blit_end, front_blit = find_function(fire, "m11_draw_dm1_front_wall_blit")
    require_tokens(front_blit, [
        "m11_draw_dm1_wall_blit_with_transparency",
        "fbW, fbH, blit, -1",
    ], "Firestaff front wall opaque blit wrapper")
    front_start, _front_end, front = find_function(fire, "m11_draw_dm1_front_walls")
    require_in_order(front, [
        ("D1C near front wall spec", "{0, 1, 0, M11_GFX_WALLSET0_D1C, 32, 9, 160, 111}"),
        ("D2C mid front wall spec", "{1, 2, 0, M11_GFX_WALLSET0_D2C, 59, 19, 106, 74}"),
        ("D3C far front wall spec", "{2, 3, 0, M11_GFX_WALLSET0_D3C, 77, 25, 70, 49}"),
        ("front wall wrapper blit", "m11_draw_dm1_front_wall_blit"),
        ("nearer center occludes farther", "occluded = 1;"),
    ], "Firestaff front wall depth/occlusion")
    side_start, _side_end, side = find_function(fire, "m11_draw_dm1_side_walls")
    require_in_order(side, [
        ("D3 side wall first", "{3, 3, -2, M11_GFX_WALLSET0_D3L2"),
        ("D2 side wall after D3", "{2, 2, -2, M11_GFX_WALLSET0_D2L2"),
        ("D1 side wall after D2", "{1, 1, -1, M11_GFX_WALLSET0_D1L"),
        ("D0 side wall nearest", "{0, 0, -1, M11_GFX_WALLSET0_D0L"),
        ("same-lane guard", "m11_dm1_side_lane_clear_for_rel(cells,"),
        ("wall blit", "m11_draw_dm1_wall_blit_with_transparency"),
    ], "Firestaff side wall depth/occlusion")
    view_start, _view_end, view = find_function(fire, "m11_draw_viewport")
    require_in_order(view, [
        ("sample viewport cells", "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])"),
        ("derive max visible", "maxVisibleForward = m11_dm1_max_visible_forward_from_center(cells);"),
        ("draw side walls", "m11_draw_dm1_side_walls"),
        ("draw front walls", "m11_draw_dm1_front_walls"),
        ("center blocker replay", "m11_dm1_nearest_blocking_center_depth_index(cells)"),
    ], "Firestaff viewport draw wiring")
    ok.append(
        "Firestaff wall depth/occlusion wiring: "
        f"m11_game_view.c:{line_no(fire, max_start)}, {line_no(fire, front_blit_start)}, {line_no(fire, front_start)}, {line_no(fire, side_start)}, {line_no(fire, view_start)}"
    )

    print("V1 viewport wall depth/source-lock gate passed")
    for line in ok:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

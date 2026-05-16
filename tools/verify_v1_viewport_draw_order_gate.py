#!/usr/bin/env python3
"""Verify the narrow V1 viewport world draw-order contract.

This is a source-shape/evidence gate. It does not inspect pixels; it keeps the
DM1 V1 open-cell content stack wired in the original-faithful order so future
wall/item/creature/projectile work cannot silently reorder layers.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
REDMCSB_DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    for m in pattern.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0:
            break
        semicolon = text.find(";", m.end(), brace)
        if semicolon >= 0:
            continue  # prototype/declaration, not the function body
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), i + 1, text[m.start() : i + 1]
        raise AssertionError(f"unterminated function body for {name}")
    raise AssertionError(f"missing function body for {name}")



def require_regex(text: str, pattern: str, label: str) -> re.Match[str]:
    match = re.search(pattern, text, re.S)
    if not match:
        raise AssertionError(f"missing {label}: {pattern}")
    return match

def require_in_order(body: str, markers: list[tuple[str, str]], label: str) -> list[str]:
    positions: list[tuple[str, int]] = []
    for name, needle in markers:
        pos = body.find(needle)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        positions.append((name, pos))
    for (prev_name, prev_pos), (next_name, next_pos) in zip(positions, positions[1:]):
        if prev_pos >= next_pos:
            raise AssertionError(f"{label}: {prev_name} appears after {next_name}")
    return [name for name, _pos in positions]



def find_redmcsb_function_region(text: str, name: str) -> tuple[int, int, str]:
    start = text.find(name)
    if start < 0:
        raise AssertionError(f"missing ReDMCSB function region for {name}")
    line_start = text.rfind("\n", 0, start) + 1
    end = text.find("\nSTATICFUNCTION ", start + len(name))
    if end < 0:
        end = text.find("\nvoid F", start + len(name))
    if end < 0:
        end = len(text)
    return line_start, end, text[line_start:end]

def main() -> int:
    text = SRC.read_text(encoding="utf-8")
    redmcsb_text = REDMCSB_DUNVIEW.read_text(encoding="utf-8")
    ok: list[str] = []

    f0115_start, _f0115_end, f0115_body = find_redmcsb_function_region(
        redmcsb_text, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF")
    require_in_order(
        f0115_body,
        [
            ("source summary defers then draws objects", "draw each object found"),
            ("source summary draws creatures after objects", "Draw one creature at the cell being processed"),
            ("source summary restarts for projectiles", "Draw only projectiles at specified cell"),
            ("source summary handles explosions last", "Draw only explosions at specified cell"),
            ("code object section", "/* Draw objects */"),
            ("code creature section", "/* Draw creatures */"),
            ("code projectile section", "/* Draw projectiles */"),
        ],
        "ReDMCSB F0115 source draw-order evidence",
    )
    ok.append(f"ReDMCSB F0115 evidence: {REDMCSB_DUNVIEW.name}:{line_no(redmcsb_text, f0115_start)}")

    wall_start, _wall_end, wall_body = find_function(text, "m11_draw_wall_face")
    require_in_order(
        wall_body,
        [
            ("front wall/door/stair face", "switch (cell->elementType)"),
            ("wall ornament", "m11_draw_wall_ornament"),
            ("door ornament", "m11_draw_door_ornament"),
            ("open-cell contents", "m11_draw_wall_contents"),
        ],
        "wall face world stack",
    )
    if "if (m11_viewport_cell_is_open(cell))" not in wall_body:
        raise AssertionError("wall face does not guard open-cell contents with m11_viewport_cell_is_open")
    ok.append(f"wall/door ornaments before open-cell contents: m11_game_view.c:{line_no(text, wall_start)}")

    frame_table = require_regex(
        redmcsb_text,
        r"G0163_aauc_Graphic558_Frame_Walls\[12\]\[8\].*?\{\s*32,\s*191,\s*9,\s*119,\s*128,\s*111,\s*48,\s*0\s*\},\s*/\* D1C \*/",
        "ReDMCSB D1C wall frame row",
    )
    f0100_start, _f0100_end, f0100_body = find_redmcsb_function_region(
        redmcsb_text, "F0100_DUNGEONVIEW_DrawWallSetBitmap")
    for token in [
        "P0106_puc_Frame[C6_X]",
        "P0106_puc_Frame[C7_Y]",
        "P0106_puc_Frame[C4_BYTE_WIDTH]",
        "P0106_puc_Frame[C5_HEIGHT]",
    ]:
        if token not in f0100_body:
            raise AssertionError(f"ReDMCSB F0100 missing source-row/frame token {token}")
    f0101_start, _f0101_end, f0101_body = find_redmcsb_function_region(
        redmcsb_text, "F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency")
    for token in [
        "P0108_puc_Frame[C6_X]",
        "P0108_puc_Frame[C7_Y]",
        "P0108_puc_Frame[C4_BYTE_WIDTH]",
        "P0108_puc_Frame[C5_HEIGHT]",
        "CM1_COLOR_NO_TRANSPARENCY",
    ]:
        if token not in f0101_body:
            raise AssertionError(f"ReDMCSB F0101 missing source-row/frame token {token}")
    d1c_function_start = redmcsb_text.rfind("STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C")
    d1c_start = redmcsb_text.find("case C00_ELEMENT_WALL:", d1c_function_start)
    d1c_end = redmcsb_text.find("return;", d1c_start)
    d1c_wall_case = redmcsb_text[d1c_start:d1c_end]
    require_in_order(
        d1c_wall_case,
        [
            ("D1C source wall case", "case C00_ELEMENT_WALL:"),
            ("D1C wall bitmap draw", "G0163_aauc_Graphic558_Frame_Walls[M606_VIEW_SQUARE_D1C]"),
            ("D1C alcove item gate", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF"),
            ("D1C alcove contents", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
        ],
        "ReDMCSB D1C front-wall draw path",
    )
    d1c_frame_row = frame_table.start() + frame_table.group(0).rfind("/* D1C */")
    ok.append(
        f"ReDMCSB D1C front-wall frame/source-row evidence: "
        f"{REDMCSB_DUNVIEW.name}:{line_no(redmcsb_text, d1c_frame_row)}, "
        f"F0100:{line_no(redmcsb_text, f0100_start)}, F0101:{line_no(redmcsb_text, f0101_start)}, "
        f"F0124:{line_no(redmcsb_text, d1c_start)}"
    )

    front_start, _front_end, front_body = find_function(text, "m11_draw_dm1_front_walls")
    require_in_order(
        front_body,
        [
            ("D1C perspective front wall", "{0, 1, 0, M11_GFX_WALLSET0_D1C, 32, 9, 160, 111}"),
            ("D2C perspective front wall", "{1, 2, 0, M11_GFX_WALLSET0_D2C, 59, 19, 106, 74}"),
            ("D3C perspective front wall", "{2, 3, 0, M11_GFX_WALLSET0_D3C, 77, 25, 70, 49}"),
            ("nearer-center wall occlusion", "occluded = 1"),
        ],
        "Firestaff DM1 front-wall perspective/occlusion stack",
    )
    wall_blit_start, _wall_blit_end, wall_blit_body = find_function(text, "m11_draw_dm1_wall_blit_with_transparency")
    for token in [
        "slot->width != blit->width",
        "slot->height != blit->height",
        "M11_AssetLoader_BlitRegion(slot",
        "0, 0, blit->width, blit->height",
        "M11_VIEWPORT_X + blit->dstX",
        "M11_VIEWPORT_Y + blit->dstY",
    ]:
        if token not in wall_blit_body:
            raise AssertionError(f"Firestaff wall blit missing clipped-front-wall token {token}")
    ok.append(
        f"DM1 front-wall D1C/D2C/D3C perspective and clipped asset blit: "
        f"m11_game_view.c:{line_no(text, front_start)}, blit:{line_no(text, wall_blit_start)}"
    )

    contents_start, _contents_end, contents_body = find_function(text, "m11_draw_wall_contents")
    for citation in [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "DUNVIEW.C:4567-4582",
        "DUNVIEW.C:4820",
        "DUNVIEW.C:5201",
        "DUNVIEW.C:5645",
    ]:
        if citation not in contents_body:
            raise AssertionError(f"open-cell content stack missing ReDMCSB citation {citation}")
    layer_names = require_in_order(
        contents_body,
        [
            ("layer 0 floor ornaments", "/* Layer 0: Floor ornaments"),
            ("floor ornament draw", "m11_draw_floor_ornament"),
            ("layer 1 floor items", "/* Layer 1: Floor items"),
            ("item sprite draw", "m11_draw_item_sprite"),
            ("layer 2 creatures", "/* Layer 2: Creatures"),
            ("creature sprite draw", "m11_draw_creature_sprite"),
            ("layer 3 projectiles/effects", "/* Layer 3: Projectiles and explosions"),
            ("effect/projectile draw", "m11_draw_effect_cue"),
        ],
        "open-cell content draw order",
    )
    guard_prefix = contents_body.split("/* Layer 0: Floor ornaments", 1)[0]
    if "!m11_viewport_cell_is_open(cell)" not in guard_prefix:
        raise AssertionError("open-cell content stack lacks early non-open-cell return before layer 0")
    ok.append(f"open-cell layer order: {' -> '.join(layer_names)} at m11_game_view.c:{line_no(text, contents_start)}")

    print("V1 viewport draw-order source-shape verification passed")
    for line in ok:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

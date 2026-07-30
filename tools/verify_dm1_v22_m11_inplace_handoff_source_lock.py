#!/usr/bin/env python3
"""Verify the DM1 V2.2 M11 in-place modern-art handoff.

This is a source/flow gate, not an original-DOS pixel-parity claim. It pins the
current M11 path that samples the V1 source viewport cells, fills the V22 shape
cache, and admits the in-place bitmap pass only after the DM1 V2.2 reviewed-art
gate has accepted the installed pack.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v22_m11_inplace_handoff_source_lock.json"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def display(path: Path) -> str:
    text = str(path)
    home = str(Path.home())
    if text.startswith(home + "/"):
        return "~/" + text[len(home) + 1 :]
    return text


def require(errors: list[str], condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def ordered(text: str, needles: list[str]) -> tuple[list[dict[str, int | str]], list[str]]:
    positions: list[dict[str, int | str]] = []
    errors: list[str] = []
    last = -1
    for needle in needles:
        pos = text.find(needle)
        if pos < 0:
            errors.append(f"missing ordered marker: {needle}")
        elif pos <= last:
            errors.append(f"out-of-order marker: {needle}")
        positions.append({"marker": needle, "offset": pos})
        last = pos
    return positions, errors


def main() -> int:
    errors: list[str] = []
    game_view_path = ROOT / "src/engine/m11_game_view.c"
    boot_path = ROOT / "src/dm1v2/dm1_v2_boot_pc34.c"
    inplace_path = ROOT / "src/dm1v2/m11_v22_inplace_draw_pc34.c"
    cell_rects_path = ROOT / "src/dm1v2/m11_v22_cell_rects_pc34.c"
    cache_path = ROOT / "src/dm1v2/m11_v22_shape_cache_pc34.c"
    overlay_path = ROOT / "src/dm1v2/m11_v22_render_overlay_pc34.c"
    dunview_path = REDMCSB / "DUNVIEW.C"

    game_view = read(game_view_path)
    boot = read(boot_path)
    inplace = read(inplace_path)
    cell_rects = read(cell_rects_path)
    cache = read(cache_path)
    overlay = read(overlay_path)
    dunview = read(dunview_path)

    required_game_view_markers = [
        '#include "m11_v22_shape_cache_pc34.h"',
        '#include "m11_v22_inplace_draw_pc34.h"',
        "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])",
        "raw_squares[d][s] = cells[d][s].square;",
        "m11_v22_shape_cache_update((int)state->world.party.direction,\n"
        "                                       raw_squares);",
        "m11_apply_dungeon_palette_level(framebuffer, framebufferWidth, framebufferHeight,",
        "if (state->presentationMode == M12_PRESENTATION_V22_MODERN)",
        "(void)m11_v22_inplace_render_pass(framebuffer,",
        "m11_apply_viewport_turn_pan(framebuffer, framebufferWidth, framebufferHeight,",
    ]
    for marker in required_game_view_markers:
        require(errors, marker in game_view, f"m11_game_view.c missing marker: {marker}")

    required_boot_markers = [
        "if (!game_id || strcmp(game_id, \"dm1\") != 0)",
        "if (dm1_v2_shape_runtime_v22_active())",
        "int cache_ready = m11_v22_inplace_draw_init();",
        "out_receipt->v22_inplace_cache_active =",
        "cache_ready && m11_v22_inplace_draw_active();",
    ]
    for marker in required_boot_markers:
        require(errors, marker in boot,
                f"dm1_v2_boot_pc34.c missing marker: {marker}")

    draw_start = game_view.find("m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])")
    draw_end = game_view.find("m11_apply_viewport_turn_pan(framebuffer, framebufferWidth, framebufferHeight,")
    draw_window = game_view[draw_start:draw_end]
    draw_positions, draw_errors = ordered(
        draw_window,
        [
            "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])",
            "raw_squares[d][s] = cells[d][s].square;",
            "m11_v22_shape_cache_update((int)state->world.party.direction,\n"
            "                                       raw_squares);",
            "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,",
            "m11_apply_dungeon_palette_level(framebuffer, framebufferWidth, framebufferHeight,",
            "if (state->presentationMode == M12_PRESENTATION_V22_MODERN)",
            "(void)m11_v22_inplace_render_pass(framebuffer,",
        ],
    )
    errors.extend(draw_errors)
    require(errors, "m11_v22_render_overlay_with_palette" not in game_view,
            "DM1 V2.2 must not fall back to synthetic overlay art")

    inplace_required = [
        "static const char* v22_wall_asset_id  = \"wall_d3_carved_hero_01\";",
        "static const char* v22_floor_plain_id = \"floor_plain_hero_01\";",
        "static const char* v22_floor_pit_id = \"floor_pit_hero_01\";",
        "static const char* v22_field_teleporter_id = \"field_teleporter_hero_01\";",
        "static const char* v22_creature_asset_id = \"creature_demon_hero_01\";",
        "case M11_V22_SHAPE_FLOOR_STAIRS_UP:\n"
        "        case M11_V22_SHAPE_FLOOR_STAIRS_DOWN:\n"
        "            return NULL;",
        "case M11_V22_SHAPE_FLOOR_PIT:",
        "case M11_V22_SHAPE_FIELD_TELEPORTER:",
        "int m11_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH)",
        "if (!m11_v22_inplace_draw_active()) return 0;",
        "if (!m11_v22_shape_cache_populated()) return 0;",
        "m11_v22_cell_rect(depth + 1, lateral);",
        "cells_painted++;",
        "pit/stairs/teleporter-field material routing and no wrong-wall fallback;",
    ]
    for marker in inplace_required:
        require(errors, marker in inplace, f"m11_v22_inplace_draw_pc34.c missing marker: {marker}")

    cache_required = [
        "m11_v22_shape_cache_update(int direction,",
        "if (!dm1_v2_shape_runtime_v22_active())",
        "r->active = 0;  /* V1 path */",
        "dm1_v2_shape_runtime_for_cell(",
        "d + 1,",
        "s - 1);",
    ]
    for marker in cache_required:
        require(errors, marker in cache, f"m11_v22_shape_cache_pc34.c missing marker: {marker}")

    overlay_required = [
        "int m11_v22_render_overlay(unsigned char* framebuffer, int fbW, int fbH)",
        "m11_v22_shape_cache_get(depth + 1, lateral);",
        "if (!r || !r->active) continue;",
        "m11_v22_cell_rect(depth + 1, lateral);",
        "M11_V22_OVERLAY_PLACEHOLDER_INDEX",
    ]
    for marker in overlay_required:
        require(errors, marker in overlay, f"m11_v22_render_overlay_pc34.c missing marker: {marker}")

    cell_rect_required = [
        "m11_v22_cell_rect(int depth, int lateral)",
        "depth < 1 || depth > 3",
        "lateral < -1 || lateral > 1",
        "return &kV22CellRects[depth - 1][lateral + 1];",
    ]
    for marker in cell_rect_required:
        require(errors, marker in cell_rects, f"m11_v22_cell_rects_pc34.c missing marker: {marker}")

    redmcsb_required = [
        "void F0128_DUNGEONVIEW_Draw_CPSF",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
    ]
    for marker in redmcsb_required:
        require(errors, marker in dunview, f"ReDMCSB DUNVIEW.C missing marker: {marker}")

    result = {
        "status": "failed" if errors else "passed",
        "scope": "DM1 V2.2 M11 in-place modern-art handoff source-lock",
        "firestaffAnchors": {
            "gameView": str(game_view_path.relative_to(ROOT)),
            "boot": str(boot_path.relative_to(ROOT)),
            "inplaceDraw": str(inplace_path.relative_to(ROOT)),
            "cellRects": str(cell_rects_path.relative_to(ROOT)),
            "shapeCache": str(cache_path.relative_to(ROOT)),
            "overlayFallback": str(overlay_path.relative_to(ROOT)),
        },
        "redmcsbAnchor": display(dunview_path),
        "drawOrder": draw_positions,
        "claims": [
            "M11 samples the source V1 viewport cells before V22 shape-cache update.",
            "DM1 V2.2 boot initializes the optional in-place bitmap cache only on the DM1 V22-active branch.",
            "The viewport draw path invokes m11_v22_inplace_render_pass only in the admitted V2.2 presentation mode.",
            "Pit and stairs have distinct asset ids, and teleporter/field shapes do not fall back to wall art.",
            "The in-place and placeholder overlay fallback passes share m11_v22_cell_rect() for D1/D2/D3 x L/C/R geometry.",
        ],
        "nonClaims": [
            "No original DOS screenshot or pixel-parity claim.",
            "No finished PBR-art quality claim beyond the reviewed-art gate.",
            "No CSB V2 material gate.",
        ],
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    print(f"dm1_v22_m11_inplace_handoff_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

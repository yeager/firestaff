#!/usr/bin/env python3
"""Verify the DM1 V2.2 M11 in-place modern-art handoff.

This is a source/flow gate, not an original-DOS pixel-parity claim. It pins the
current M11 path that samples the V1 source viewport cells, fills the V22 shape
cache, and permits only the verified in-place bitmap pass. Missing/partial art
must resolve at boot to the source-data V2.1 path, never a colored overlay.
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
    inplace_path = ROOT / "src/dm1v2/m11_v22_inplace_draw_pc34.c"
    cell_rects_path = ROOT / "src/dm1v2/m11_v22_cell_rects_pc34.c"
    cache_path = ROOT / "src/dm1v2/m11_v22_shape_cache_pc34.c"
    dunview_path = REDMCSB / "DUNVIEW.C"

    game_view = read(game_view_path)
    inplace = read(inplace_path)
    cell_rects = read(cell_rects_path)
    cache = read(cache_path)
    dunview = read(dunview_path)

    required_game_view_markers = [
        '#include "m11_v22_shape_cache_pc34.h"',
        '#include "m11_v22_inplace_draw_pc34.h"',
        "m11_v22_inplace_draw_shutdown();",
        "dm1_v2_boot_startup_prepare_pc34(spec->gameId,",
        "switch (receipt.resolved_mode)",
        "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])",
        "raw_squares[d][s] = cells[d][s].square;",
        "m11_v22_shape_cache_update((int)state->world.party.direction, raw_squares);",
        "m11_apply_dungeon_palette_level(framebuffer, framebufferWidth, framebufferHeight,",
        "(void)m11_v22_inplace_render_pass(framebuffer,",
        "m11_apply_viewport_turn_pan(framebuffer, framebufferWidth, framebufferHeight,",
    ]
    for marker in required_game_view_markers:
        require(errors, marker in game_view, f"m11_game_view.c missing marker: {marker}")

    require(errors, "m11_v22_render_overlay_with_palette" not in game_view,
            "V22 placeholder overlay remains reachable from M11")

    draw_start = game_view.find("m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])")
    draw_end = game_view.find("m11_apply_viewport_turn_pan(framebuffer, framebufferWidth, framebufferHeight,")
    draw_window = game_view[draw_start:draw_end]
    draw_positions, draw_errors = ordered(
        draw_window,
        [
            "m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side])",
            "raw_squares[d][s] = cells[d][s].square;",
            "m11_v22_shape_cache_update((int)state->world.party.direction, raw_squares);",
            "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,",
            "m11_apply_dungeon_palette_level(framebuffer, framebufferWidth, framebufferHeight,",
            "(void)m11_v22_inplace_render_pass(framebuffer,",
        ],
    )
    errors.extend(draw_errors)

    inplace_required = [
        "static const char* v22_floor_pit_id = \"floor_pit_01\";",
        "static const char* v22_floor_stairs_down_id = \"floor_stairs_down_01\";",
        "case M11_V22_SHAPE_FLOOR_PIT:",
        "case M11_V22_SHAPE_FIELD_TELEPORTER:",
        "return NULL;",
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
            "inplaceDraw": str(inplace_path.relative_to(ROOT)),
            "cellRects": str(cell_rects_path.relative_to(ROOT)),
            "shapeCache": str(cache_path.relative_to(ROOT)),
        },
        "redmcsbAnchor": display(dunview_path),
        "drawOrder": draw_positions,
        "claims": [
            "M11 samples the source V1 viewport cells before V22 shape-cache update.",
            "DM1 V2.2 boot resolves incomplete art to V2.1 before M11 presentation.",
            "The viewport draw path uses m11_v22_inplace_render_pass only; no colored placeholder overlay is reachable.",
            "Pit and stairs have distinct asset ids, and teleporter/field shapes do not fall back to wall art.",
            "The in-place pass uses m11_v22_cell_rect() for D1/D2/D3 x L/C/R geometry.",
        ],
        "nonClaims": [
            "No original DOS screenshot or pixel-parity claim.",
            "No finished PBR-art quality claim.",
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

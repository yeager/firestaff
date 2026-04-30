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
SRC = ROOT / "m11_game_view.c"
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

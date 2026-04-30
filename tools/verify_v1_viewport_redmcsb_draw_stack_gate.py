#!/usr/bin/env python3
"""Source-lock the V1 viewport open-cell draw stack to ReDMCSB.

This gate is intentionally source-shape based.  It verifies the ReDMCSB F0115
thing-pass contract (objects first, creatures next, projectiles later, explosions
last) and the Firestaff M11 open-cell stack that mirrors the visible subset used
by DM1 V1 parity work.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIRESTAFF_SRC = ROOT / "m11_game_view.c"
DEFAULT_REDMCSB = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
)
REDMCSB_SRC = Path(os.environ.get("FIRESTAFF_REDMCSB_DUNVIEW", DEFAULT_REDMCSB))


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
    # ReDMCSB uses STATICFUNCTION + K&R-style parameter declarations; Firestaff
    # uses normal static prototypes.  Match the function header, then balance the
    # first body brace after it, skipping declarations/prototypes if present.
    pattern = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    for match in pattern.finditer(text):
        brace = text.find("{", match.end())
        if brace < 0:
            continue
        semi = text.find(";", match.end(), brace)
        if semi >= 0:
            continue
        depth = 0
        for pos in range(brace, len(text)):
            ch = text[pos]
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return match.start(), pos + 1, text[match.start() : pos + 1]
        raise AssertionError(f"unterminated function body for {name}")
    raise AssertionError(f"missing function body for {name}")


def require_in_order(body: str, markers: list[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    found: list[tuple[str, int]] = []
    for name, needle in markers:
        pos = body.find(needle)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        found.append((name, pos))
    for (prev_name, prev_pos), (next_name, next_pos) in zip(found, found[1:]):
        if prev_pos >= next_pos:
            raise AssertionError(f"{label}: {prev_name} appears after {next_name}")
    return found


def verify_redmcsb() -> list[str]:
    text = REDMCSB_SRC.read_text(encoding="latin-1")
    name = "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"
    start = text.find(name)
    if start < 0:
        raise AssertionError(f"missing function body for {name}")
    # F0115's introductory source comment contains pseudo-code braces before the
    # K&R-style body brace, so use a bounded source window instead of generic
    # brace matching for this translated C function.
    body = text[start : start + 180_000]
    rel = require_in_order(
        body,
        [
            ("function structure: note projectile/creature before drawing", "if there is a projectile, explosion or creature, take note of it, but do not draw them yet"),
            ("function structure: draw each object", "draw each object found"),
            ("function structure: draw creature after objects", "Draw one creature at the cell being processed"),
            ("function structure: draw projectiles after creatures", "Draw only projectiles at specified cell"),
            ("function structure: draw explosions last", "Draw only explosions at specified cell"),
            ("object pass starts", "/* Draw objects */"),
            ("groups deferred", "L0151_T_GroupThing = P0141_T_Thing"),
            ("projectiles deferred", "L0186_B_SquareHasProjectile = C1_TRUE"),
            ("object native graphic", "M612_GRAPHIC_FIRST_OBJECT"),
            ("creature native graphic", "M618_GRAPHIC_FIRST_CREATURE"),
            ("projectile native graphic", "M613_GRAPHIC_FIRST_PROJECTILE"),
        ],
        "ReDMCSB F0115 draw stack",
    )
    return [
        f"ReDMCSB F0115 starts at {REDMCSB_SRC}:{line_no(text, start)}",
        *(f"ReDMCSB {name}: line {line_no(text, start + pos)}" for name, pos in rel),
    ]


def verify_firestaff() -> list[str]:
    text = FIRESTAFF_SRC.read_text(encoding="utf-8")
    wall_start, _wall_end, wall_body = find_function(text, "m11_draw_wall_face")
    wall = require_in_order(
        wall_body,
        [
            ("front wall/door/stair face", "switch (cell->elementType)"),
            ("wall ornament before contents", "m11_draw_wall_ornament"),
            ("door ornament before contents", "m11_draw_door_ornament"),
            ("open-cell contents", "m11_draw_wall_contents"),
        ],
        "Firestaff wall face stack",
    )
    if "if (m11_viewport_cell_is_open(cell))" not in wall_body:
        raise AssertionError("Firestaff wall face no longer gates contents with m11_viewport_cell_is_open")

    contents_start, _contents_end, contents_body = find_function(text, "m11_draw_wall_contents")
    contents = require_in_order(
        contents_body,
        [
            ("early open-cell guard", "!m11_viewport_cell_is_open(cell)"),
            ("layer 0 floor ornaments", "/* Layer 0: Floor ornaments"),
            ("floor ornament draw", "m11_draw_floor_ornament"),
            ("layer 1 floor items", "/* Layer 1: Floor items"),
            ("item sprite draw", "m11_draw_item_sprite"),
            ("layer 2 creatures", "/* Layer 2: Creatures"),
            ("creature sprite draw", "m11_draw_creature_sprite"),
            ("layer 3 projectiles/effects", "/* Layer 3: Projectiles and explosions"),
            ("effect/projectile draw", "m11_draw_effect_cue"),
        ],
        "Firestaff open-cell content stack",
    )
    return [
        f"Firestaff m11_draw_wall_face starts at {FIRESTAFF_SRC}:{line_no(text, wall_start)}",
        *(f"Firestaff wall {name}: line {line_no(text, wall_start + pos)}" for name, pos in wall),
        f"Firestaff m11_draw_wall_contents starts at {FIRESTAFF_SRC}:{line_no(text, contents_start)}",
        *(f"Firestaff contents {name}: line {line_no(text, contents_start + pos)}" for name, pos in contents),
    ]


def main() -> int:
    lines = verify_redmcsb() + verify_firestaff()
    print("V1 viewport ReDMCSB draw-stack source gate passed")
    for line in lines:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

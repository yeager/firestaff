#!/usr/bin/env python3
"""Source-lock the high-risk D1C door-pass ordering in ReDMCSB DUNVIEW.C.

This is intentionally narrow: it protects the exact D1 center front-door stack
from DUNVIEW.C F0124 before anyone claims full DM1 V1 viewport parity.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIRESTAFF_SRC = ROOT / "src/engine/m11_game_view.c"
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
REDMCSB_SRC = Path(os.environ.get("FIRESTAFF_REDMCSB_DUNVIEW", DEFAULT_REDMCSB))


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
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


def ordered(body: str, markers: list[tuple[str, str]], label: str) -> list[tuple[str, int]]:
    found: list[tuple[str, int]] = []
    cursor = 0
    for name, needle in markers:
        pos = body.find(needle, cursor)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r} after offset {cursor}")
        found.append((name, pos))
        cursor = pos + len(needle)
    return found


def verify_redmcsb() -> list[str]:
    text = REDMCSB_SRC.read_text(encoding="latin-1")
    starts = [m.start() for m in re.finditer(r"STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C\s*\(", text)]
    start = -1
    body = ""
    for candidate in starts:
        next_start = text.find("STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L", candidate)
        if next_start < 0:
            continue
        window = text[candidate:next_start]
        if "case C17_ELEMENT_DOOR_FRONT:" in window:
            start = candidate
            body = window
            break
    if start < 0:
        raise AssertionError("missing F0124_DUNGEONVIEW_DrawSquareD1C body before F0125")
    door_start = body.find("case C17_ELEMENT_DOOR_FRONT:")
    if door_start < 0:
        raise AssertionError("ReDMCSB F0124 has no C17_ELEMENT_DOOR_FRONT case")
    case = body[door_start:]
    markers = ordered(
        case,
        [
            ("D1C door-front case", "case C17_ELEMENT_DOOR_FRONT:"),
            ("floor ornament first", "F0108_DUNGEONVIEW_DrawFloorOrnament"),
            ("door pass 1 before frame/door", "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT"),
            ("door-frame top", "G2112_DoorFrameTopD1LCR"),
            ("door-frame left", "G2117_DoorFrameLeftD1C"),
            ("door button before door slab", "F0110_DUNGEONVIEW_DrawDoorButton"),
            ("door slab after button", "F0111_DUNGEONVIEW_DrawDoor"),
            ("door pass 2 selected", "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
            ("door pass 2 jumps to shared draw", "goto T0124018"),
            ("shared draw label", "T0124018:"),
            ("door pass 2 thing draw", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
            ("door pass 2 order variable", "L0217_i_Order"),
        ],
        "ReDMCSB D1C front-door stack",
    )
    corridor_start = body.find("case C05_ELEMENT_TELEPORTER:")
    if corridor_start < 0:
        raise AssertionError("ReDMCSB F0124 has no open-cell corridor/teleporter path")
    corridor = body[corridor_start:]
    corridor_markers = ordered(
        corridor,
        [
            ("open-cell order constant", "C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT"),
            ("open-cell floor ornament", "F0108_DUNGEONVIEW_DrawFloorOrnament"),
            ("open-cell ceiling pit", "F0112_DUNGEONVIEW_DrawCeilingPit"),
            ("shared draw label", "T0124018:"),
            ("open-cell thing draw", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
        ],
        "ReDMCSB D1C open-cell stack",
    )
    return [
        f"ReDMCSB F0124_DUNGEONVIEW_DrawSquareD1C starts at {REDMCSB_SRC}:{line_no(text, start)}",
        *(f"ReDMCSB D1C door {name}: line {line_no(text, start + door_start + pos)}" for name, pos in markers),
        *(f"ReDMCSB D1C open {name}: line {line_no(text, start + corridor_start + pos)}" for name, pos in corridor_markers),
    ]


def verify_firestaff_gap_guard() -> list[str]:
    text = FIRESTAFF_SRC.read_text(encoding="utf-8")
    start, _end, body = find_function(text, "m11_draw_viewport")
    markers = ordered(
        body,
        [
            ("floor pits batch", "m11_draw_dm1_floor_pits"),
            ("floor ornaments batch", "m11_draw_dm1_floor_ornaments"),
            ("side walls batch", "m11_draw_dm1_side_walls"),
            ("front walls batch", "m11_draw_dm1_front_walls"),
            ("side doors batch", "m11_draw_dm1_side_doors"),
            ("center doors batch", "m11_draw_dm1_center_doors"),
            ("center door ornaments batch", "m11_draw_dm1_center_door_ornaments"),
            ("center door buttons batch", "m11_draw_dm1_center_door_buttons"),
            ("source gap warning", "Firestaff current V1 renderer still batches by primitive class"),
            ("near-side replay guard", "m11_dm1_nearest_blocking_center_depth_index"),
        ],
        "Firestaff m11_draw_viewport current batched renderer guard",
    )
    return [
        f"Firestaff m11_draw_viewport starts at {FIRESTAFF_SRC}:{line_no(text, start)}",
        *(f"Firestaff viewport {name}: line {line_no(text, start + pos)}" for name, pos in markers),
    ]


def main() -> int:
    lines = verify_redmcsb() + verify_firestaff_gap_guard()
    print("V1 viewport D1C door-pass source-lock gate passed")
    for line in lines:
        print(f"- {line}")
    print("- invariant: D1C front door draws floor ornament + doorpass1 contents before frame/button/slab, then doorpass2 contents via shared F0115 label")
    print("- gap_guard: Firestaff is still protected as batched-with-replay, not full per-square DUNVIEW parity")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

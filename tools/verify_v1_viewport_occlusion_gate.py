#!/usr/bin/env python3
"""Verify the narrow V1 viewport source-occlusion gate wiring.

This is intentionally a source-shape guard, not a visual capture: it keeps the
normal V1 source-backed pit/floor-ornament/stair/field passes bound to the
nearest non-open center-lane blocker before they sample or draw farther cells.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"

TARGETS = {
    "pits": "m11_draw_dm1_floor_pits",
    "floor ornaments": "m11_draw_dm1_floor_ornaments",
    "stairs": "m11_draw_dm1_stairs",
    "teleporter fields": "m11_draw_dm1_teleporter_fields",
    "side walls": "m11_draw_dm1_side_walls",
    "side doors": "m11_draw_dm1_side_doors",
    "side door ornaments": "m11_draw_dm1_side_door_ornaments",
    "side destroyed-door masks": "m11_draw_dm1_side_destroyed_door_masks",
}


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing function body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), i + 1, text[m.start() : i + 1]
    raise AssertionError(f"unterminated function body for {name}")


def require_before(body: str, first: str, second: str, label: str) -> None:
    a = body.find(first)
    b = body.find(second)
    if a < 0:
        raise AssertionError(f"{label}: missing {first!r}")
    if b < 0:
        raise AssertionError(f"{label}: missing {second!r}")
    if a > b:
        raise AssertionError(f"{label}: {first!r} appears after {second!r}")


def main() -> int:
    text = SRC.read_text(encoding="utf-8")
    ok: list[str] = []

    start, _end, body = find_function(text, "m11_dm1_max_visible_forward_from_center")
    if "!m11_viewport_cell_is_open(&cells[depth][1])" not in body or "return depth + 1;" not in body:
        raise AssertionError("m11_dm1_max_visible_forward_from_center no longer gates at nearest non-open center cell")
    ok.append(f"maxVisibleForward source: m11_game_view.c:{line_no(text, start)}")

    for label, fn in TARGETS.items():
        start, _end, body = find_function(text, fn)
        if "int maxVisibleForward" not in body.split("{")[0]:
            raise AssertionError(f"{label}: {fn} does not accept maxVisibleForward")
        require_before(body, "relForward > maxVisibleForward", "m11_sample_viewport_cell", label)
        ok.append(f"{label} gate before sampling: m11_game_view.c:{line_no(text, start)}")

    start, _end, body = find_function(text, "m11_draw_viewport")
    if "maxVisibleForward = m11_dm1_max_visible_forward_from_center(cells);" not in body:
        raise AssertionError("m11_draw_viewport does not derive maxVisibleForward from sampled center cells")
    for label, fn in TARGETS.items():
        call = re.search(re.escape(fn) + r"\s*\([^;]*maxVisibleForward", body, flags=re.S)
        if not call:
            raise AssertionError(f"m11_draw_viewport does not pass maxVisibleForward to {label}")
    ok.append(f"viewport call-site wiring: m11_game_view.c:{line_no(text, start)}")

    print("V1 viewport occlusion gate source-shape verification passed")
    for line in ok:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

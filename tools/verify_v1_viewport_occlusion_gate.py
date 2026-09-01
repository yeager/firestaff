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
SRC = ROOT / "src/engine/m11_game_view.c"

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

    # 2026-07-20 round 15 re-anchor (same-drift-family): the nearest
    # non-open center scan moved into the dm1_viewport_3d contract module
    # (lane-visibility receipt); the m11 helper delegates to it, and the
    # side-walls pass gates via the draw spec's runtime_rel_forward.
    start, _end, body = find_function(text, "m11_dm1_max_visible_forward_from_center")
    if "m11_dm1_lane_visibility(cells).max_visible_forward" not in body:
        raise AssertionError("m11_dm1_max_visible_forward_from_center no longer derives from the lane-visibility receipt")
    lane_start = text.find("DM1_ViewportLaneVisibilityReceiptPc34 m11_dm1_lane_visibility(")
    if lane_start < 0:
        raise AssertionError("missing m11_dm1_lane_visibility")
    lane_body = text[lane_start:text.find("\n}", lane_start)]
    if "const M11_ViewportCell* cell = &cells[depth][1];" not in lane_body or \
            "m11_viewport_cell_is_open(cell)" not in lane_body:
        raise AssertionError("m11_dm1_lane_visibility no longer feeds center openness from sampled cells")
    contract_3d = (ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c").read_text(encoding="utf-8")
    cstart, _cend, cbody = find_function(contract_3d, "dm1_viewport_3d_max_visible_forward_from_center_pc34")
    if "nearest >= 0 ? nearest + 1 : 3" not in cbody:
        raise AssertionError("contract max_visible_forward no longer gates at nearest non-open center cell")
    ok.append(f"maxVisibleForward source: m11_game_view.c:{line_no(text, start)}")

    for label, fn in TARGETS.items():
        start, _end, body = find_function(text, fn)
        if "int maxVisibleForward" not in body.split("{")[0]:
            raise AssertionError(f"{label}: {fn} does not accept maxVisibleForward")
        gate_token = ("runtime_rel_forward > maxVisibleForward"
                      if "runtime_rel_forward > maxVisibleForward" in body
                      else "relForward > maxVisibleForward")
        require_before(body, gate_token, "m11_sample_viewport_cell", label)
        ok.append(f"{label} gate before sampling: m11_game_view.c:{line_no(text, start)}")

    start, _end, body = find_function(text, "m11_draw_viewport")
    # 2026-07-20 round 15 re-anchor (same-drift-family): maxVisibleForward
    # now comes from the lane-visibility receipt, and the round-14
    # architecture reconciliation deliberately gives the primary pit and
    # field passes the full D3..D1 range ("geometry is hidden by later
    # source panels, not pre-culled by a host visibility shortcut").  The
    # final stair replay is different: it follows the late center-wall
    # envelope and must retain the nearest-center visibility bound, or a
    # D2/D3 stair panel would reappear through a nearer closed center cell.
    if "maxVisibleForward = visibility.max_visible_forward;" not in body:
        raise AssertionError("m11_draw_viewport does not derive maxVisibleForward from the lane-visibility receipt")
    FULL_RANGE_TARGETS = {"pits", "teleporter fields"}
    if "not pre-culled by a host visibility shortcut" not in body:
        raise AssertionError("primary floor passes lost the no-pre-cull rationale")
    for label, fn in TARGETS.items():
        if label in FULL_RANGE_TARGETS:
            call = re.search(re.escape(fn) + r"\s*\([^;]*\b1, 3, cells\)", body, flags=re.S)
            if not call:
                raise AssertionError(f"m11_draw_viewport does not pass the full D3..D1 range to {label}")
            continue
        if label == "floor ornaments":
            call = re.search(re.escape(fn) + r"\s*\([^;]*\b3, cells\)", body, flags=re.S)
            if not call:
                raise AssertionError("m11_draw_viewport does not pass the full D3..D1 range to floor ornaments")
            continue
        if label == "stairs":
            call = re.search(re.escape(fn) + r"\s*\([^;]*\b1, maxVisibleForward, cells\)", body, flags=re.S)
            if not call:
                raise AssertionError("m11_draw_viewport does not bind the late stair replay to maxVisibleForward")
            continue
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

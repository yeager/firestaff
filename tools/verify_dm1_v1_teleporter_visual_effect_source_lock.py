#!/usr/bin/env python3
"""Source-lock DM1 V1 teleporter visual effect rendering.

This gate retires the TODO wording "shimmer/sparkle" by proving the backed
effect is the original field bitmap overlay, not invented procedural art.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIRE_VIEW = ROOT / "src/engine/m11_game_view.c"
DM1_FIELD = ROOT / "src/dm1/dm1_v1_field_teleporter_effect_pc34_compat.c"
DOC = ROOT / "docs/graphics/DM1_V1_TELEPORTER_FIELD_SOURCE_LOCK.md"
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise AssertionError(f"{label}: forbidden {needle!r}")


def line_no(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated function {name}")


def require_ordered(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: out of order {marker!r}")
        last = pos


def main() -> int:
    fire = FIRE_VIEW.read_text(encoding="utf-8")
    dm1_field = DM1_FIELD.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    doc = DOC.read_text(encoding="utf-8")

    red_needles = [
        "char G2035_ac_ViewSquareIndexToFieldAspectIndex[23] = { 13, 14, 15, 10, 11, 12, 7, 8, 9, 5, 6, 2, 3, 4, 0, 1, -1, -1, -1, -1, -1, -1, -1 };",
        "unsigned char G0188_aauc_Graphic558_FieldAspects[16][8] = {",
        "C076_GRAPHIC_FIRST_FIELD + M728_NATIVE_BITMAP_RELATIVE_INDEX(P0135_puc_FieldAspect)",
        "M005_RANDOM(2) + M729_BASE_START_UNIT_INDEX(P0135_puc_FieldAspect), M003_RANDOM(32)",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M609_VIEW_SQUARE_D0C]], C715_ZONE_WALL_D0C);",
    ]
    for needle in red_needles:
        require(red, needle, "ReDMCSB teleporter field evidence")

    forbid(fire, "static void m11_draw_teleporter_effect", "Firestaff invented teleporter cue")
    forbid(fire, "Outer shimmer frame", "Firestaff invented teleporter cue")
    forbid(fire, "Inner cross pattern", "Firestaff invented teleporter cue")
    forbid(fire, "Corner dots for shimmer", "Firestaff invented teleporter cue")

    _, cue_body = find_function(fire, "m11_draw_effect_cue")
    forbid(cue_body, "summary.teleporters", "m11_draw_effect_cue teleporter procedural overlay")
    require(cue_body, "Teleporter fields are source bitmap overlays", "m11_draw_effect_cue source note")

    field_zone_pos, field_zone = find_function(fire, "m11_draw_dm1_field_zone")
    require_ordered(field_zone, [
        "dm1_v1_field_asset_binding_pc34(plan, &binding)",
        "binding.fieldGraphicIndex",
        "binding.maskGraphicIndex",
        "dm1_v1_field_bitmap_pixel_pc34(",
    ], "Firestaff source field bitmap shimmer")

    _, field_bitmap = find_function(dm1_field, "dm1_v1_field_bitmap_sample_pc34")
    require_ordered(field_bitmap, [
        "fieldStartUnit = plan->baseStartUnit + (int)((animTick >> 1) & 1u);",
        "fieldYPhase = (int)((animTick * 7u) & 31u);",
        "outSample->fieldX = (localX + (fieldStartUnit * 16)) % fieldWidth;",
        "outSample->fieldY = (localY + fieldYPhase) % fieldHeight;",
    ], "DM1-owned source field bitmap shimmer")

    fields_pos, fields = find_function(fire, "m11_draw_dm1_teleporter_fields")
    require_ordered(fields, [
        "cell.elementType != DUNGEON_ELEMENT_TELEPORTER",
        "dm1_v1_field_square_is_visible_open_pc34(cell.square)",
        "m11_draw_dm1_field_zone(state, framebuffer, fbW, fbH,",
    ], "Firestaff teleporter open/visible field gate")

    _, visible_open = find_function(dm1_field, "dm1_v1_field_square_is_visible_open_pc34")
    require_ordered(visible_open, [
        "DM1_FIELD_TELEPORTER_VISIBLE_MASK_PC34",
        "DM1_FIELD_TELEPORTER_OPEN_MASK_PC34",
    ], "DM1-owned teleporter open/visible field gate")

    require(doc, "Status: source-backed implementation present", "source-lock document")
    require(doc, "procedural sparkle, crosshair, or particle effect", "source-lock document")

    print("DM1 V1 teleporter visual effect source-lock passed")
    print(f"- Firestaff field blit: {FIRE_VIEW}:{line_no(fire, field_zone_pos)}")
    print(f"- DM1 field sampling: {DM1_FIELD}")
    print(f"- Firestaff teleporter field rows: {FIRE_VIEW}:{line_no(fire, fields_pos)}")
    for needle in red_needles:
        pos = red.find(needle)
        print(f"- ReDMCSB {RED_DUNVIEW.name}:{line_no(red, pos)} {needle}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

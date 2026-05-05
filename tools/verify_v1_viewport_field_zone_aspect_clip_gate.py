#!/usr/bin/env python3
"""Verify DM1 V1 teleporter field rows stay source-locked to ReDMCSB.

This is a narrow source-first gate for the non-C2500/C2900 half of viewport
row clipping: teleporter fields select a G2035 field aspect for the viewed
square, draw into the matching C702..C717 wall zone, and must be suppressed
behind nearer center blockers by Firestaff's maxVisibleForward guard.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"
RED_DEFS = RED_ROOT / "DEFS.H"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated {name}")


def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} is out of order")
        last = pos


def parse_field_specs(body: str) -> list[tuple[int, int, int, int, int, int, int, int, int]]:
    m = re.search(r"static\s+const\s+M11_DM1FieldSpec\s+kFields\[\]\s*=\s*\{(?P<body>.*?)\n\s*\};", body, re.S)
    if not m:
        raise AssertionError("Firestaff fields: missing kFields table")
    table = m.group("body")
    out: list[tuple[int, int, int, int, int, int, int, int, int]] = []
    for match in re.finditer(r"\{\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\}", table):
        out.append(tuple(int(x, 0) for x in match.groups()))
    return out


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    defs = RED_DEFS.read_text(encoding="latin-1")
    cmake = CMAKE.read_text(encoding="utf-8")

    # Source evidence: MEDIA720 field aspect mapping and wall-zone family.
    source_needles = [
        "char G2035_ac_ViewSquareIndexToFieldAspectIndex[23] = { 13, 14, 15, 10, 11, 12, 7, 8, 9, 5, 6, 2, 3, 4, 0, 1, -1, -1, -1, -1, -1, -1, -1 };",
        "unsigned char G0188_aauc_Graphic558_FieldAspects[12][8];",
        "F0113_DUNGEONVIEW_DrawField(L0198_auc_FieldAspect, C702_ZONE_WALL_D3L2 + L2482_i_FieldAspectIndex);",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C14_VIEW_SQUARE_D3L2]], C702_ZONE_WALL_D3L2);",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M600_VIEW_SQUARE_D3C]], C704_ZONE_WALL_D3C);",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M607_VIEW_SQUARE_D1L]], C713_ZONE_WALL_D1L);",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);",
    ]
    for needle in source_needles:
        require(red, needle, "ReDMCSB DUNVIEW field row/aspect source")

    for needle in [
        "#define C702_ZONE_WALL_D3L2                                     702",
        "#define C704_ZONE_WALL_D3C                                      704",
        "#define C712_ZONE_WALL_D1C                                      712",
        "#define C715_ZONE_WALL_D0C                                      715",
        "#define C717_ZONE_WALL_D0R                                      717",
    ]:
        require(defs, needle, "ReDMCSB DEFS wall zones")

    # Firestaff table is expected to be the G2035 aspect order, converted to
    # (relative forward/side, layout-696 destination rect).  The final field in
    # each tuple is the mask-aspect index/flip; it is how Firestaff records the
    # source field aspect row selected by G2035 for side cells.
    _, field_body = find_function(fire, "m11_draw_dm1_teleporter_fields")
    expected = [
        (3, -2, 0,   25, 36,  49, 0x3f, 0x0a, 0x00), # C14_VIEW_SQUARE_D3L2 -> aspect 0 -> C702
        (3,  2, 188, 25, 36,  49, 0x3f, 0x0a, 0x80), # C15_VIEW_SQUARE_D3R2 -> aspect 1 -> C703 flipped
        (3, -1, 7,   25, 83,  49, 0x3f, 0x0a, 0x01), # M601_VIEW_SQUARE_D3L  -> aspect 3 -> C705
        (3,  0, 77,  25, 70,  49, 0x3f, 0x8a, 0xff), # M600_VIEW_SQUARE_D3C  -> aspect 2 -> C704
        (3,  1, 134, 25, 83,  49, 0x3f, 0x0a, 0x81), # M602_VIEW_SQUARE_D3R  -> aspect 4 -> C706 flipped
        (2, -2, 0,   24, 8,   52, 0x3f, 0x0a, 0x02), # M604_VIEW_SQUARE_D2L2 -> aspect 5 -> C707
        (2,  2, 216, 24, 8,   52, 0x3f, 0x0a, 0x82), # M605_VIEW_SQUARE_D2R2 -> aspect 6 -> C708 flipped
        (2, -1, 0,   19, 78,  74, 0x3f, 0x0a, 0x03), # M604? side D2L -> aspect 8 -> C710
        (2,  0, 59,  19, 106, 74, 0x3c, 0x8a, 0xff), # M603_VIEW_SQUARE_D2C -> aspect 7 -> C709
        (2,  1, 146, 19, 78,  74, 0x3f, 0x0a, 0x83), # D2R -> aspect 9 -> C711 flipped
        (1, -1, 0,   9,  60, 111, 0x3f, 0x0a, 0x04), # M607_VIEW_SQUARE_D1L -> aspect 11 -> C713
        (1,  0, 32,  9, 160, 111, 0x3d, 0x8a, 0xff), # M606_VIEW_SQUARE_D1C -> aspect 10 -> C712
        (1,  1, 164, 9,  60, 111, 0x3f, 0x0a, 0x84), # M608_VIEW_SQUARE_D1R -> aspect 12 -> C714 flipped
        (0, -1, 0,   0,  33, 136, 0x3f, 0x0a, 0x05), # D0L -> aspect 14 -> C716
        (0,  0, 0,   0, 224, 136, 0x3b, 0x8a, 0xff), # D0C -> aspect 13 -> C715
        (0,  1, 191, 0,  33, 136, 0x3f, 0x0a, 0x85), # M611_VIEW_SQUARE_D0R -> aspect 15 -> C717 flipped
    ]
    got = parse_field_specs(field_body)
    if got != expected:
        raise AssertionError(f"Firestaff field rows/zones differ from source lock\nexpected={expected}\nactual={got}")

    require_in_order(field_body, [
        "if (kFields[i].relForward > maxVisibleForward)",
        "if (!m11_sample_viewport_cell(state, kFields[i].relForward, kFields[i].relSide, &cell))",
        "if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_TELEPORTER)",
        "if ((cell.square & 0x04) == 0 || (cell.square & 0x08) == 0)",
        "m11_draw_dm1_field_zone(state, framebuffer, fbW, fbH,",
    ], "Firestaff field clip/sample/draw gate")

    order_pos, order_body = find_function(fire, "m11_draw_viewport")
    require_in_order(order_body, [
        "maxVisibleForward = m11_dm1_max_visible_forward_from_center(cells);",
        "m11_draw_dm1_side_walls(state, framebuffer, framebufferWidth, framebufferHeight,",
        "m11_draw_dm1_front_walls(state, framebuffer, framebufferWidth, framebufferHeight, cells);",
        "m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight,",
        "m11_draw_dm1_side_doors(state, framebuffer, framebufferWidth, framebufferHeight,",
        "m11_draw_dm1_center_doors(state, framebuffer, framebufferWidth, framebufferHeight, cells);",
    ], "Firestaff field/wall/door source draw order")

    require(cmake, "NAME v1_viewport_field_zone_aspect_clip_gate", "CMake test registration")

    print("V1 viewport field zone/aspect clip gate passed")
    print(f"- Firestaff field rows/zones: {SRC}:{line_no(fire, fire.find('static void m11_draw_dm1_teleporter_fields'))}")
    print(f"- Firestaff source viewport draw order: {SRC}:{line_no(fire, order_pos)}")
    for needle in source_needles:
        pos = red.find(needle)
        print(f"- ReDMCSB {RED_DUNVIEW.name}:{line_no(red, pos)} {needle}")
    print(f"- ReDMCSB DEFS wall zones: {RED_DEFS}:{line_no(defs, defs.find('#define C702_ZONE_WALL_D3L2'))}-{line_no(defs, defs.find('#define C717_ZONE_WALL_D0R'))}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

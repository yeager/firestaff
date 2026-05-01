#!/usr/bin/env python3
"""Verify V1 viewport distance rows stay source-locked to ReDMCSB clipping.

This gate is intentionally narrow and source-first.  ReDMCSB MEDIA720 F0115
selects C2500/C2900 rows through G2028, rejects near/far cells before drawing,
and clips final blits through F0635_/G0296_puc_Bitmap_Viewport.  Walls, doors,
and fields use their own distance-specific wall/door/field zones and source draw
order; they must not be collapsed into synthetic Firestaff panes.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
RED_DUNVIEW = RED_ROOT / "DUNVIEW.C"
RED_DEFS = RED_ROOT / "DEFS.H"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


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


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} is out of order")
        last = pos


def require_absent(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise AssertionError(f"{label}: forbidden {needle!r}")


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    red = RED_DUNVIEW.read_text(encoding="latin-1")
    defs = RED_DEFS.read_text(encoding="latin-1")
    cmake = CMAKE.read_text(encoding="utf-8")

    # ReDMCSB source citations: row maps and zone families.
    red_needles = [
        "char G2027_ac_ViewSquareIndexToViewDepth[23] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4 };",
        "char G2028_ac_ViewSquareIndexTo[23] = { 11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1 };",
        "char G2035_ac_ViewSquareIndexToFieldAspectIndex[23] = { 13, 14, 15, 10, 11, 12, 7, 8, 9, 5, 6, 2, 3, 4, 0, 1, -1, -1, -1, -1, -1, -1, -1 };",
        "if ((AL0127_i_ThingType >= C05_THING_TYPE_WEAPON) && (AL0127_i_ThingType <= C10_THING_TYPE_JUNK) && (L2476_i_ >= 0) && (M011_CELL(P0141_T_Thing) == L0139_i_Cell) && ((L2475_i_ViewDepth != 3) || (AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT)) && ((L2475_i_ViewDepth != 0) || (AL0126_i_ViewCell < C02_VIEW_CELL_BACK_RIGHT)))",
        "L2474_i_ZoneIndex = (C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES) + (L2476_i_ * 4) + AL0126_i_ViewCell;",
        "if ((L2479_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)",
        "if ((L2475_i_ViewDepth == 3) && (AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT))",
        "if ((L2475_i_ViewDepth == 0) && (AL0126_i_ViewCell >= C02_VIEW_CELL_BACK_RIGHT))",
        "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
        "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex",
        "F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination, G2032_ai_XYZ",
        "F0635_(NULL, L2472_ai_XYZ, P2086_i_ZoneIndex",
        "F0133_VIDEO_BlitBoxFilledWithMaskedBitmap(L0119_puc_Bitmap, G0296_puc_Bitmap_Viewport",
    ]
    for n in red_needles:
        require(red, n, "ReDMCSB DUNVIEW source")

    for n in [
        "#define C2500_ZONE_                                            2500",
        "#define C2548_ZONE_                                            2548",
        "#define C2900_ZONE_                                            2900",
        "#define M624_ZONE_DOOR_D3L                                     3700",
        "#define M631_ZONE_DOOR_D1C                                     3770",
    ]:
        require(defs, n, "ReDMCSB DEFS zone family")

    # ReDMCSB draw-order/source-zone contract for D3 walls/doors/fields.
    require_in_order(red, [
        "void F0676_DrawD3L2(",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L2484_ai_SquareAspect[M550_FIRST_THING]",
        "F0111_DUNGEONVIEW_DrawDoor(L2484_ai_SquareAspect[M557_DOOR_THING_INDEX]",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[C14_VIEW_SQUARE_D3L2]], C702_ZONE_WALL_D3L2);",
    ], "ReDMCSB D3L2 wall/door/field order")
    require_in_order(red, [
        "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0201_ai_SquareAspect[M550_FIRST_THING]",
        "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX]",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M601_VIEW_SQUARE_D3L]], C705_ZONE_WALL_D3L);",
    ], "ReDMCSB D3L wall/door/field order")

    # Firestaff: row map must mirror G2028 for D1/D2/D3 center+side squares.
    _, row_body = find_function(fire, "m11_dm1_f0115_c2500_c2900_row")
    require(row_body, "static const signed char kG2028[23] = {", "Firestaff F0115 row map")
    require(row_body, "11, -1, -1,  8,  9, 10,  5,  6,  7, -1, -1,", "Firestaff F0115 row map")
    require(row_body, "0,  1,  2,  3,  4", "Firestaff F0115 row map")
    require_in_order(row_body, [
        "int viewSquare = m11_dm1_f0115_view_square_index(relForward, relSide);",
        "if (viewSquare < 0 || viewSquare >= 23) return -1;",
        "return (int)kG2028[viewSquare];",
    ], "Firestaff F0115 row map")

    _, square_body = find_function(fire, "m11_dm1_f0115_view_square_index")
    for n in ["{ 4,  3,  5}", "{ 7,  6,  8}", "{12, 11, 13}"]:
        require(square_body, n, "Firestaff view-square map")

    # Firestaff: raw C2500/C2900 helpers expose side/deep rows and reject invalid cells.
    for func, rows, tail_marker in [
        ("m11_c2500_object_raw_zone_point", "kC2500Raw[17][4][2]", "if (rowIndex < 0 || rowIndex >= 17) return 0;"),
        ("m11_c2900_projectile_raw_zone_point", "kC2900Raw[12][4][2]", "if (rowIndex < 0 || rowIndex >= 12) return 0;"),
    ]:
        _, body = find_function(fire, func)
        require(body, rows, f"Firestaff {func}")
        require(body, tail_marker, f"Firestaff {func}")
        require(body, "if (relativeCell < 0 || relativeCell > 3) return 0;", f"Firestaff {func}")
        require(body, "if (zx == 0 && zy == 0) return 0;", f"Firestaff {func}")

    # Firestaff: projectile source row path clips only to the viewport, not pane bounds.
    _, proj_body = find_function(fire, "m11_draw_projectile_sprite")
    require_in_order(proj_body, [
        "m11_c2900_projectile_raw_zone_point(sourceZoneRow",
        "drawX = M11_VIEWPORT_X + zoneX - drawW / 2;",
        "if (sourceZoneRow >= 0)",
        "int minX = M11_VIEWPORT_X - drawW + 1;",
        "int maxX = M11_VIEWPORT_X + M11_VIEWPORT_W - 1;",
        "if (drawX > maxX) drawX = maxX;",
    ], "Firestaff C2900 viewport clip")
    source_branch = proj_body.split("if (sourceZoneRow >= 0)", 1)[1].split("        } else {", 1)[0]
    require_absent(source_branch, "drawX < x", "Firestaff C2900 source-row branch")
    require_absent(source_branch, "drawX + drawW > x + w", "Firestaff C2900 source-row branch")

    require(cmake, "NAME v1_viewport_distance_row_clip_gate", "CMake test registration")

    print("V1 viewport distance row clip gate passed")
    print(f"- Firestaff row map: {SRC}:{line_no(fire, fire.find("static int m11_dm1_f0115_c2500_c2900_row"))}")
    print(f"- Firestaff C2500 raw rows: {SRC}:{line_no(fire, fire.find("static int m11_c2500_object_raw_zone_point"))}")
    print(f"- Firestaff C2900 viewport clip: {SRC}:{line_no(fire, fire.find("static int m11_draw_projectile_sprite"))}")
    for needle in [
        "char G2028_ac_ViewSquareIndexTo[23]",
        "L2474_i_ZoneIndex = (C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES)",
        "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
        "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M601_VIEW_SQUARE_D3L]], C705_ZONE_WALL_D3L);",
    ]:
        pos = red.find(needle)
        print(f"- ReDMCSB {RED_DUNVIEW.name}:{line_no(red, pos)} {needle}")
    print(f"- ReDMCSB DEFS zones: {RED_DEFS}:{line_no(defs, defs.find("#define C2500_ZONE_"))}-{line_no(defs, defs.find("#define M632_ZONE_DOOR_D1R"))}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

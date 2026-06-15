#!/usr/bin/env python3
"""Pass764: source-lock CSB V1 F0115 wall text/ornament D1C route."""
from __future__ import annotations

from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
CSB_LINEAGE = Path("/Users/bosse/.openclaw/data/firestaff-csb-source/CSB/src/Viewport.cpp")
CSBWIN = Path("/Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp")

CHECKS = [
    ("redmcsb-f0115-thing-pass", RED / "DUNVIEW.C", "4547-4581", [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.",
        "The remaining nibbles contain ordinals of square view cells to draw",
    ]),
    ("redmcsb-f0107-wall-text-ornament", RED / "DUNVIEW.C", "3502-3938", [
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "P0116_i_WallOrnamentOrdinal--;",
        "AL0090_puc_CoordinateSet",
        "L0096_B_IsAlcove = F0149_DUNGEON_IsWallOrnamentAnAlcove",
        "F0168_DUNGEON_DecodeText",
        "C10_COLOR_FLESH",
        "return L0096_B_IsAlcove;",
    ]),
    ("redmcsb-f0124-d1c-wall-route", RED / "DUNVIEW.C", "7825-7843", [
        "F0100_DUNGEONVIEW_DrawWallSetBitmap",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT)",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
    ]),
    ("redmcsb-f0128-frame-setup", RED / "DUNVIEW.C", "8318-8486", [
        "F0128_DUNGEONVIEW_Draw_CPSF",
        "G0578_B_UseByteBoxCoordinates = C1_TRUE;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0676_DrawD3L2",
        "F0677_DrawD3R2",
    ]),
    ("redmcsb-defs-c10-c5-zones-g0208", RED / "DEFS.H", "2088-5576", [
        "#define C10_COLOR_FLESH           10",
        "#define C5_HEIGHT        5",
        "#define M606_VIEW_SQUARE_D1C  3",
        "#define M587_VIEW_WALL_D1C_FRONT 14",
        "#define C800_ZONE_STAIRS_UP_FRONT_D3L2",
        "#define C814_ZONE_STAIRS_DOWN_FRONT_D3R2",
        "extern unsigned char G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
    ]),
    ("csb-lineage-viewport-open-f0", CSB_LINEAGE, "1192-1209", [
        "StdDrawF0L1Open",
        "F0Contents,  F0xy, F0 , DrawOrder21,  StdDrawRoomObjects",
        "StdDrawF0R1Open",
    ]),
    ("csb-lineage-viewport-f1-door", CSB_LINEAGE, "1903-1915", [
        "StdDrawF1DoorFacing",
        "F1Contents,  F1xy, F1, DrawOrder218,  StdDrawRoomObjects",
        "StdDrawDoor",
        "F1Contents,  F1xy, F1, DrawOrder349,  StdDrawRoomObjects",
    ]),
    ("csb-lineage-viewport-f0-door", CSB_LINEAGE, "1930-1944", [
        "StdDrawF0L1DoorFacing",
        "StdDrawF0DoorFacing",
        "F0Contents,  F0xy, F0, DrawOrder21,  StdDrawRoomObjects",
        "StdDrawF0R1DoorFacing",
    ]),
    ("csbwin-viewport-f1-door", CSBWIN, "1903-1915", [
        "StdDrawF1DoorFacing",
        "F1Contents,  F1xy, F1, DrawOrder218,  StdDrawRoomObjects",
        "StdDrawDoor",
        "F1Contents,  F1xy, F1, DrawOrder349,  StdDrawRoomObjects",
    ]),
    ("firestaff-header", ROOT / "include/csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.h", "1-120", [
        "CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 320",
        "CSB_V1_F0115_WALL_TEXT_ORNAMENT_VIEWPORT_HEIGHT_PC34 136",
        "CSB_V1_ViewportF0115WallTextOrnamentPc34Spec",
        "csb_v1_viewport_f0115_wall_text_ornament_render_pc34",
    ]),
    ("firestaff-source", ROOT / "src/csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.c", "1-280", [
        "DUNVIEW.C:7825-7843 F0124_DUNGEONVIEW_DrawSquareD1C",
        "DUNVIEW.C:3502-3938 F0107",
        "DUNVIEW.C:4547-4581 F0115",
        "DUNVIEW.C:8318-8486 F0128",
        "DEFS.H:2088 anchors C10",
        "CSB-lineage Viewport.cpp:1192-1209",
        "csb_v1_viewport_f0115_wall_text_ornament_render_pc34",
    ]),
    ("firestaff-test", ROOT / "tests/test_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.c", "1-420", [
        "test_spec_contract",
        "test_route_gate",
        "test_render_trace_and_hash",
        "trace.text_non_zero",
        "trace.hash",
        "assertion_count_between_80_and_120",
    ]),
    ("firestaff-evidence", ROOT / "parity-evidence/pass764_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.md", "1-120", [
        "Pass764 CSB V1 viewport F0115 wall text ornament source-lock gate",
        "DUNVIEW.C F0115:4547-4581",
        "DUNVIEW.C F0107:3502-3938",
        "DUNVIEW.C F0128:8318-8486",
        "DEFS.H:2088",
        "CSB-lineage Viewport.cpp:1192-1209,1903-1915,1930-1944",
    ]),
]


def read_span(path: Path, span: str) -> tuple[int, str]:
    first, last = [int(part) for part in span.split("-", 1)]
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    lines = path.read_text(encoding=encoding, errors="replace").splitlines()
    return first, "\n".join(lines[first - 1:last])


def check_one(name: str, path: Path, span: str, needles: list[str]) -> bool:
    first, text = read_span(path, span)
    cursor = 0
    ok = True
    print(f"[{name}] {path}:{span}")
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            print(f"  FAIL missing: {needle}")
            ok = False
        else:
            line = first + text.count("\n", 0, pos)
            print(f"  PASS line {line}: {needle}")
            cursor = pos + len(needle)
    return ok


def main() -> int:
    ok = True
    for check in CHECKS:
        ok &= check_one(*check)
    if ok:
        print("PASS pass764_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat")
        return 0
    print("FAIL pass764_csb_v1_viewport_f0115_wall_text_ornament_pc34_compat")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())

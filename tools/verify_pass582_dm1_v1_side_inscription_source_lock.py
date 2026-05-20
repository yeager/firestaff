#!/usr/bin/env python3
"""Pass582: source-lock DM1 V1 side-wall inscription rendering."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
FIRE = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def source_slice(path: Path, start: int, end: int) -> str:
    return "\n".join(read(path).splitlines()[start - 1:end])


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def require_order(text: str, markers: list[tuple[str, str]], label: str) -> None:
    last = -1
    last_name = ""
    for name, needle in markers:
        pos = require(text, needle, f"{label} {name}")
        if pos <= last:
            raise AssertionError(f"{label}: {name} appears before/at {last_name}")
        last = pos
        last_name = name


def c_function(text: str, name: str) -> str:
    match = re.search(r"(?m)^static\s+[^\n]*\b" + re.escape(name) + r"\s*\(", text)
    if not match:
        raise AssertionError(f"missing Firestaff function {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for idx in range(brace, len(text)):
        if text[idx] == "{":
            depth += 1
        elif text[idx] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():idx + 1]
    raise AssertionError(f"unterminated Firestaff function {name}")


def main() -> int:
    dunview = RED / "DUNVIEW.C"
    dungeon = RED / "DUNGEON.C"
    fire = read(FIRE)
    cmake = read(CMAKE)

    require_order(source_slice(dungeon, 2568, 2594), [
        ("visible text-string branch", "if (L0312_i_ThingType == C02_THING_TYPE_TEXTSTRING)"),
        ("visible text gates inscription ornament", "if (((TEXTSTRING*)L0308_ps_Sensor)->Visible)"),
        ("PC34 square aspect gets inscription ordinal", "P0317_pui_SquareAspect[AL0310_i_SideIndex] = G0265_i_CurrentMapInscriptionWallOrnamentIndex + 1;"),
        ("inscription thing captured", "G0290_T_DungeonView_InscriptionThing = L0314_T_Thing;"),
    ], "ReDMCSB DUNGEON.C F0172 inscription square-aspect route")

    require_order(source_slice(dunview, 3589, 3593), [
        ("current-map inscription test", "L0095_B_IsInscription = (AP0116_i_WallOrnamentIndex == G0265_i_CurrentMapInscriptionWallOrnamentIndex)"),
        ("decode captured text thing", "F0168_DUNGEON_DecodeText(L0099_auc_InscriptionString, G0290_T_DungeonView_InscriptionThing, C0_TEXT_TYPE_INSCRIPTION);"),
    ], "ReDMCSB DUNVIEW.C F0107 inscription decode")

    require_order(source_slice(dunview, 3864, 3902), [
        ("line counting starts for inscription", "if (L0095_B_IsInscription)"),
        ("count line terminators", "while (*AL0091_puc_Character < 128)"),
        ("only unreadable plaque for under four lines", "if (AL0097_i_TextLineIndex < 4)"),
        ("shift unreadable/open-door zone", "MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR"),
        ("source bitmap width shift", "G2154_i_ZoneShiftX = M100_PIXEL_WIDTH(AL0091_puc_Bitmap);"),
        ("source height by view increment", "G2155_i_ZoneShiftY = G0204_auc_Graphic558_UnreadableInscriptionBoxY2"),
    ], "ReDMCSB DUNVIEW.C F0107 unreadable inscription height route")

    g0204 = source_slice(dunview, 1327, 1332)
    for expected in [
        "5,   8, 13",
        "7,  13, 20",
        "5,  12, 19",
        "10, 17, 27",
        "11, 22, 33",
    ]:
        require(g0204, expected, f"PC34 G0204 unreadable inscription height row {expected}")

    height_fn = c_function(fire, "m11_dm1_unreadable_inscription_box_height")
    require_order(height_fn, [
        ("D3 side row", "{5, 8, 13}"),
        ("D3 front row", "{7, 13, 20}"),
        ("D2 side row", "{5, 12, 19}"),
        ("D2 front row", "{10, 17, 27}"),
        ("D1 side row", "{11, 22, 33}"),
        ("four-line plaque keeps full height", "lineCount >= 4"),
        ("D3 side projection selector", "row = (relSide != 0 && sideProjection) ? 0 : 1;"),
        ("D2 side projection selector", "row = (relSide != 0 && sideProjection) ? 2 : 3;"),
    ], "Firestaff unreadable inscription height helper")

    wall_fn = c_function(fire, "m11_draw_dm1_wall_ornaments")
    require_order(wall_fn, [
        ("inscription global index gate", "ornGlobalIdx == 0 && kWallOrnaments[i].viewWallIndex != 12"),
        ("uses side/source projection flag", "kWallOrnaments[i].blit.width <= 16"),
        ("counts decoded text lines", "m11_dm1_visible_wall_text_line_count(state, &cell)"),
        ("clips unreadable side plaque", "blit.height = unreadableHeight;"),
        ("then loads ornament bitmap", "M11_AssetLoader_Load"),
        ("then draws clipped bitmap", "m11_blit_scaled_palette_map_maybe_flip"),
    ], "Firestaff wall ornament side-inscription render path")

    require(cmake, "NAME pass582_dm1_v1_side_inscription_source_lock", "CMake pass582 registration")

    print("PASS pass582_dm1_v1_side_inscription_source_lock")
    print("ReDMCSB anchors:")
    print("- DUNGEON.C:2568-2594 F0172: visible TextString maps the wall side to the current map inscription ornament and captures G0290.")
    print("- DUNVIEW.C:3589-3593 F0107: inscription ornament decodes G0290 with C0_TEXT_TYPE_INSCRIPTION.")
    print("- DUNVIEW.C:3864-3902 F0107: non-D1C inscription plaques use line count and G0204/G2155 before F0791.")
    print("- DUNVIEW.C:1327-1332 G0204: PC34 unreadable inscription heights for D3 side/front, D2 side/front, D1 side.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL pass582_dm1_v1_side_inscription_source_lock: {exc}", file=sys.stderr)
        raise SystemExit(1)

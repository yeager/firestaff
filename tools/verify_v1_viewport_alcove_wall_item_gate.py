#!/usr/bin/env python3
"""Source-lock DM1 V1 wall-alcove item visibility to ReDMCSB.

This is a narrow source-first gate for the wall-cell exception: normal wall
squares block open-cell contents, but wall ornaments that are alcoves draw the
ornament and then run F0115 with CELL_ORDER_ALCOVE so items placed on that wall
cell remain visible in the alcove.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / "src/engine/m11_game_view.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DUNVIEW = RED_ROOT / "DUNVIEW.C"
DUNGEON = RED_ROOT / "DUNGEON.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def find_function(text: str, name: str) -> tuple[int, str]:
    # Match the definition (has { after parentheses), not just a declaration
    m = re.search(r"\b" + re.escape(name) + r"\s*\([^)]*\)\s*\{", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = m.end() - 1  # the { position
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated function {name}")


def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} is out of order")
        last = pos


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
    red = DUNVIEW.read_text(encoding="latin-1")
    dungeon = DUNGEON.read_text(encoding="latin-1")
    cmake = CMAKE.read_text(encoding="utf-8")

    source_needles = [
        "unsigned char G0192_auc_Graphic558_AlcoveOrnamentIndices[C003_ALCOVE_ORNAMENT_COUNT] = {\n        1,   /* Square Alcove */\n        2,   /* Vi Altar */\n        3 }; /* Arched Alcove */",
        "BOOLEAN F0149_DUNGEON_IsWallOrnamentAnAlcove(",
        "if (G0267_ai_CurrentMapAlcoveOrnamentIndices[L0247_i_Counter] == P0252_i_WallOrnamentIndex)",
        "If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.",
        "L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals);",
        "AL0126_i_ViewCell = C04_VIEW_CELL_ALCOVE; /* Index of coordinates to draw objects in alcoves */",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0212_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M583_VIEW_WALL_D2C_FRONT))",
        "L0211_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
        "T0121016:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING], P0162_i_Direction, P0163_i_MapX, P0164_i_MapY, M603_VIEW_SQUARE_D2C, L0211_i_Order);",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);",
    ]
    for needle in source_needles:
        src = dungeon if needle.startswith("BOOLEAN F0149") or needle.startswith("if (G0267") else red
        require(src, needle, "ReDMCSB alcove wall-item source")

    _, alcove_body = find_function(fire, "m11_dm1_wall_ornament_is_alcove_global")
    for needle in ["globalIndex == 1", "globalIndex == 2", "globalIndex == 3"]:
        require(alcove_body, needle, "Firestaff alcove global index guard")

    _, items_body = find_function(fire, "m11_draw_dm1_alcove_wall_items")
    require_in_order(items_body, [
        "cell->floorItemCount <= 0",
        "C0x0000_CELL_ORDER_ALCOVE",
        "C04_VIEW_CELL_ALCOVE",
        "M018_OPPOSITE(direction)",
        "for (ii = 0; ii < cell->floorItemCount; ++ii)",
        "cell->floorItemCells[ii] != alcoveCellRelativeToParty",
        "m11_draw_item_sprite(state, framebuffer, fbW, fbH,",
    ], "Firestaff alcove wall-item pass")

    sample_start = fire.find("static int m11_sample_viewport_cell")
    if sample_start < 0:
        raise AssertionError("missing m11_sample_viewport_cell")
    sample_body = fire[sample_start:fire.find("/* Extract door ornament ordinal */", sample_start)]
    require_in_order(sample_body, [
        "Do not filter WALL squares here",
        "if (cell.summary.items > 0 && state->world.things)",
        "cell.floorItemCells[cell.floorItemCount]",
    ], "Firestaff wall-item extraction for alcove pass")
    if "cell.summary.items > 0 && state->world.things &&\n        cell.elementType != DUNGEON_ELEMENT_WALL" in sample_body:
        raise AssertionError("wall items are still filtered before alcove rendering")

    _, wall_body = find_function(fire, "m11_draw_dm1_wall_ornaments")
    require_in_order(wall_body, [
        "m11_dm1_wall_ornament_zone(m11_dm1_wall_ornament_coord_set_index(ornGlobalIdx)",
        "m11_blit_scaled_palette_map_maybe_flip(slot, framebuffer, fbW, fbH,",
        "if (m11_dm1_wall_ornament_is_alcove_global(ornGlobalIdx))",
        "m11_draw_dm1_alcove_wall_items(state, framebuffer, fbW, fbH,",
        "m11_dm1_f0115_c2500_c2900_row(",
    ], "Firestaff wall ornament then alcove item order")

    require(cmake, "NAME v1_viewport_alcove_wall_item_gate", "CMake registration")

    print("V1 viewport alcove wall-item source gate passed")
    for needle in source_needles:
        src_path = DUNGEON if needle.startswith("BOOLEAN F0149") or needle.startswith("if (G0267") else DUNVIEW
        src_text = dungeon if src_path == DUNGEON else red
        pos = src_text.find(needle)
        print(f"- ReDMCSB {src_path.name}:{line_no(src_text, pos)} {needle.splitlines()[0]}")
    print(f"- Firestaff {FIRE.name}:{line_no(fire, fire.find('m11_dm1_wall_ornament_is_alcove_global'))} alcove global-index guard")
    print(f"- Firestaff {FIRE.name}:{line_no(fire, fire.find('m11_draw_dm1_alcove_wall_items'))} alcove wall-item draw pass")
    print(f"- Firestaff {FIRE.name}:{line_no(fire, fire.find('m11_draw_dm1_wall_ornaments'))} ornament draw then alcove item pass")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

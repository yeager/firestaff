#!/usr/bin/env python3
"""Source-lock DM1 V1 viewport door/wall ornament parity against ReDMCSB.

This is intentionally a narrow evidence/probe gate: it verifies the source
shape that matters for door panel composition, wall ornament alcove metadata,
and Firestaff's current local ordering/occlusion guards.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()


def read_slice(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def ordered_missing(text: str, needles: list[str]) -> list[str]:
    pos = -1
    missing: list[str] = []
    for needle in needles:
        idx = text.find(needle, pos + 1)
        if idx < 0:
            missing.append(needle)
        else:
            pos = idx
    return missing


def unordered_missing(text: str, needles: list[str]) -> list[str]:
    return [needle for needle in needles if needle not in text]


def local_function(text: str, name: str) -> str:
    match = re.search(rf"^static [^\n]*\b{re.escape(name)}\s*\(", text, re.M)
    if not match:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing function body {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():i + 1]
    raise AssertionError(f"unterminated function {name}")


def add_check(checks: list[dict[str, Any]], check_id: str, passed: bool,
              source: str, why: str, missing: list[str]) -> bool:
    checks.append({
        "id": check_id,
        "passed": passed,
        "source": source,
        "why": why,
        "missing": missing,
    })
    return passed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    checks: list[dict[str, Any]] = []
    ok = True

    text = read_slice(args.source / "DUNVIEW.C", "2672-2761")
    missing = unordered_missing(text, [
        "F0010_MAIN_WriteSpacedWords(G0267_ai_CurrentMapAlcoveOrnamentIndices",
        "G0267_ai_CurrentMapAlcoveOrnamentIndices[L0085_i_MapAlcoveGraphicCount++] = AL0074_ui_OrnamentIndex;",
        "G0101_as_CurrentMapWallOrnamentsInfo[AL0074_ui_OrnamentIndex].NativeBitmapIndex",
        "G0101_as_CurrentMapWallOrnamentsInfo[AL0074_ui_OrnamentIndex].CoordinateSet = G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices",
        "G0103_as_CurrentMapDoorOrnamentsInfo[AL0074_ui_OrnamentIndex].NativeBitmapIndex = AL0073_i_GraphicIndex;",
        "G0103_as_CurrentMapDoorOrnamentsInfo[AL0074_ui_OrnamentIndex].CoordinateSet = G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices",
        "G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets",
    ])
    ok &= add_check(checks, "redmcsb-current-map-ornament-metadata", not missing,
                    "DUNVIEW.C:2672-2761",
                    "Current-map wall/door ornament ordinals are mapped to native bitmaps/coordinate sets, and alcove ordinals are cached before drawing.", missing)

    text = read_slice(args.source / "DUNGEON.C", "1330-1346")
    missing = ordered_missing(text, [
        "BOOLEAN F0149_DUNGEON_IsWallOrnamentAnAlcove(",
        "for (L0247_i_Counter = 0; L0247_i_Counter < C003_ALCOVE_ORNAMENT_COUNT; L0247_i_Counter++)",
        "if (G0267_ai_CurrentMapAlcoveOrnamentIndices[L0247_i_Counter] == P0252_i_WallOrnamentIndex)",
        "return C1_TRUE;",
        "return C0_FALSE;",
    ])
    ok &= add_check(checks, "redmcsb-alcove-cache-drives-wall-ornament-reveal", not missing,
                    "DUNGEON.C:1330-1346",
                    "Alcove reveal/blocker behavior is driven by current-map wall ornament indexes, not by global ornament ids alone.", missing)

    text = read_slice(args.source / "DUNVIEW.C", "3567-3933")
    missing = ordered_missing(text, [
        "P0116_i_WallOrnamentOrdinal--",
        "G0101_as_CurrentMapWallOrnamentsInfo[AP0116_i_WallOrnamentIndex].NativeBitmapIndex",
        "G0101_as_CurrentMapWallOrnamentsInfo[AP0116_i_WallOrnamentIndex].CoordinateSet",
        "L0096_B_IsAlcove = F0149_DUNGEON_IsWallOrnamentAnAlcove(AP0116_i_WallOrnamentIndex);",
        "G0286_B_FacingAlcove = L0096_B_IsAlcove;",
        "F0791_DUNGEONVIEW_DrawBitmapXX(AL0091_puc_Bitmap, G0296_puc_Bitmap_Viewport",
        "return L0096_B_IsAlcove;",
    ])
    ok &= add_check(checks, "redmcsb-wall-ornament-draws-and-reports-alcove", not missing,
                    "DUNVIEW.C:3567-3933",
                    "F0107 draws the wall ornament to G0296 and returns whether it is an alcove, allowing wall squares to reveal or block contained content.", missing)

    text = read_slice(args.source / "DUNVIEW.C", "4013-4314")
    missing = ordered_missing(text, [
        "STATICFUNCTION void F0109_DUNGEONVIEW_DrawDoorOrnament(",
        "L0104_i_NativeBitmapIndex = G0103_as_CurrentMapDoorOrnamentsInfo[P0120_i_DoorOrnamentOrdinal].NativeBitmapIndex;",
        "G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets",
        "F0791_DUNGEONVIEW_DrawBitmapXX(AL0107_puc_Bitmap, G0074_puc_Bitmap_Temporary",
        "STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor(",
        "F0616_CopyBitmap(F0489_MEMORY_GetNativeBitmapOrGraphic(P0126_pi_DoorNativeBitmapIndices",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex);",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK)",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), P0128_i_ViewDoorOrnamentIndex);",
        "F0102_DUNGEONVIEW_DrawDoorBitmap(P0129_ps_DoorFrames->ClosedOrDestroyed);",
    ])
    ok &= add_check(checks, "redmcsb-door-panel-temp-composition-order", not missing,
                    "DUNVIEW.C:4013-4314",
                    "F0111 copies a native door bitmap to G0074, draws normal/thieves/destroyed ornaments into that temporary panel, then blits the panel to the viewport.", missing)

    text = read_slice(args.source / "DUNVIEW.C", "6428-6459")
    missing = ordered_missing(text, [
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
        "return;",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0705_puc_Bitmap_WallSet_DoorFrameLeft_D3L",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0200_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
    ])
    ok &= add_check(checks, "redmcsb-square-order-wall-alcove-doorframe-door", not missing,
                    "DUNVIEW.C:6428-6459",
                    "A D3 side square first lets wall ornaments decide alcove reveal/return; door-front squares draw rear contents, frame, door panel, then front contents.", missing)

    local_text = (ROOT / "src/engine/m11_game_view.c").read_text(errors="replace")

    local = local_function(local_text, "m11_draw_dm1_door_ornament_on_panel")
    missing = unordered_missing(local, [
        "kOrnD3Palette",
        "kOrnD2Palette",
        "kDoorOrnCoordSets[4][3][6]",
        "viewIndex = (depthIndex == 0) ? 2 : ((depthIndex == 1) ? 1 : 0);",
        "M11_VIEWPORT_X + panel->dstX + relX",
        "depthIndex == 2 ? kOrnD3Palette :",
    ])
    ok &= add_check(checks, "firestaff-door-ornament-uses-redmcsb-panel-relative-coordinates", not missing,
                    "m11_game_view.c:m11_draw_dm1_door_ornament_on_panel",
                    "Firestaff applies G0207-style door ornament coordinates relative to the resolved door panel and uses D3/D2 palette remaps.", missing)

    local = local_function(local_text, "m11_dm1_nearest_blocking_center_door_depth")
    missing = ordered_missing(local, [
        "m11_dm1_nearest_blocking_center_depth_index(cells)",
        "cells[depth][1].elementType != DUNGEON_ELEMENT_DOOR",
        "m11_viewport_cell_is_open(&cells[depth][1])",
        "return depth;",
    ])
    ok &= add_check(checks, "firestaff-center-door-ornaments-bound-to-nearest-blocker", not missing,
                    "m11_game_view.c:m11_dm1_nearest_blocking_center_door_depth",
                    "Center door ornaments/masks/buttons are attached only to the nearest non-open center door, matching source occlusion by complete square replay.", missing)

    local = local_function(local_text, "m11_draw_dm1_side_door_ornaments")
    missing = ordered_missing(local, [
        "if (kSpecs[i].relForward > maxVisibleForward)",
        "m11_dm1_side_lane_clear_for_rel(cells,",
        "cell.elementType != DUNGEON_ELEMENT_DOOR",
        "m11_viewport_cell_is_open(&cell) || cell.doorOrnamentOrdinal <= 0",
        "panelGraphic = m11_dm1_door_panel_graphic(state, &cell, kSpecs[i].depthIndex);",
        "m11_draw_dm1_door_ornament_on_panel(state, framebuffer, fbW, fbH,",
    ])
    ok &= add_check(checks, "firestaff-side-door-ornaments-guarded-by-visibility-and-panel-state", not missing,
                    "m11_game_view.c:m11_draw_dm1_side_door_ornaments",
                    "Side door ornaments obey max-visible/side-lane blockers and compose against the same panel state as the door bitmap.", missing)

    local = local_function(local_text, "m11_draw_viewport")
    missing = ordered_missing(local, [
        "m11_draw_dm1_side_walls",
        "m11_draw_dm1_front_walls",
        "m11_draw_dm1_wall_ornaments",
        "m11_draw_dm1_side_doors",
        "m11_draw_dm1_side_door_ornaments",
        "m11_draw_dm1_side_destroyed_door_masks",
        "m11_draw_dm1_center_doors",
        "m11_draw_dm1_center_door_ornaments",
        "m11_draw_dm1_center_destroyed_door_masks",
        "int blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "m11_draw_dm1_side_walls",
        "m11_draw_dm1_wall_ornaments",
        "m11_draw_dm1_side_doors",
        "m11_draw_dm1_side_door_ornaments",
        "m11_draw_dm1_side_destroyed_door_masks",
    ])
    ok &= add_check(checks, "firestaff-viewport-replays-near-side-wall-door-ornaments-after-center-blockers", not missing,
                    "m11_game_view.c:m11_draw_viewport",
                    "The batched renderer preserves the source-critical occlusion correction by replaying nearer side wall/door ornament layers after a blocking D2/D3 center square.", missing)

    payload = {
        "gate": "dm1_v1_viewport_door_wall_ornament_source_lock",
        "passed": ok,
        "source_root": str(args.source),
        "checks": checks,
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for check in checks:
            print(("PASS" if check["passed"] else "FAIL"), check["id"], check["source"])
            print(" ", check["why"])
            for missing_needle in check["missing"]:
                print("  missing/order:", missing_needle)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

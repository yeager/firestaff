#!/usr/bin/env python3
"""Source-lock DM1 V1 stairs, pits, level-change coordinates, and viewport refresh."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB_SOURCE = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
DEFAULT_OUT = ROOT / "parity-evidence/verification/dm1_v1_stairs_pits_viewport_source_lock.json"

def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")

def block(rel: str, start: int, end: int) -> str:
    lines = read(REDMCSB_SOURCE / rel).splitlines()
    return "\n".join(lines[start - 1:end])

def compact(text: str) -> str:
    return " ".join(text.split())

def require(citation: str, text: str, needles: list[str], why: str) -> dict[str, Any]:
    c = compact(text)
    for needle in needles:
        if compact(needle) not in c:
            raise SystemExit(f"{citation} missing expected text: {needle}")
    return {"citation": citation, "verified": True, "why": why, "needles": needles}

def verify_redmcsb() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    checks.append(require("CLIKMENU.C:124-145", block("CLIKMENU.C", 124, 145), [
        "STATICFUNCTION void F0364_COMMAND_TakeStairs",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, CM1_MAPX_NOT_ON_A_SQUARE, 0);",
        "F0154_DUNGEON_GetLocationAfterLevelChange(G0309_i_PartyMapIndex, P0733_B_StairsGoDown ? -1 : 1, &G0306_i_PartyMapX, &G0307_i_PartyMapY)",
        "F0284_CHAMPION_SetPartyDirection(F0155_DUNGEON_GetStairsExitDirection(G0306_i_PartyMapX, G0307_i_PartyMapY));",
    ], "Taking stairs removes party from source, maps through F0154, then applies F0155 exit direction."))
    checks.append(require("CLIKMENU.C:256-276", block("CLIKMENU.C", 256, 276), [
        "L1123_B_StairsSquare",
        "AL1118_ui_MovementArrowIndex == 2",
        "F0364_COMMAND_TakeStairs(M007_GET(AL1115_ui_Square, MASK0x0004_STAIRS_UP));",
        "L1116_i_SquareType == C03_ELEMENT_STAIRS",
        "G0306_i_PartyMapX = L1121_i_MapX;",
        "G0307_i_PartyMapY = L1122_i_MapY;",
    ], "Backward movement on stairs and stepping onto stairs immediately consume the stairs consequence."))
    checks.append(require("DUNGEON.C:1508-1554", block("DUNGEON.C", 1508, 1554), [
        "F0154_DUNGEON_GetLocationAfterLevelChange",
        "L0250_i_NewMapX = L0254_ps_Map->OffsetMapX + *P0272_pi_MapX;",
        "L0251_i_NewMapY = L0254_ps_Map->OffsetMapY + *P0273_pi_MapY;",
        "L0252_i_NewLevel = L0254_ps_Map->A.Level + P0271_i_LevelDelta;",
        "*P0273_pi_MapY = L0251_i_NewMapY - L0253_i_Offset;",
        "*P0272_pi_MapX = L0250_i_NewMapX - L0254_ps_Map->OffsetMapX;",
    ], "Level changes preserve global coordinates across maps using map offsets and Level delta."))
    checks.append(require("DUNGEON.C:1560-1585", block("DUNGEON.C", 1560, 1585), [
        "F0155_DUNGEON_GetStairsExitDirection",
        "MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION",
        "C1_DIRECTION_EAST",
        "C0_DIRECTION_NORTH",
        "C00_ELEMENT_WALL",
        "C03_ELEMENT_STAIRS",
    ], "Stairs exit facing is derived from orientation and the adjacent wall/stairs test."))
    checks.append(require("MOVESENS.C:469-472", block("MOVESENS.C", 469, 472), [
        "for (L0728_i_ChainedMoveCount = 100; --L0728_i_ChainedMoveCount; )",
    ], "PC builds bound chained pit/teleporter consequences at 100 iterations."))
    checks.append(require("MOVESENS.C:538-607", block("MOVESENS.C", 538, 607), [
        "AL0709_i_DestinationSquareType == C02_ELEMENT_PIT",
        "MASK0x0008_PIT_OPEN",
        "!M007_GET(AL0708_i_DestinationSquare, MASK0x0001_PIT_IMAGINARY)",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        "F0154_DUNGEON_GetLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1",
        "F0324_CHAMPION_DamageAll_GetDamagedChampionCount(20, MASK0x0010_WOUND_LEGS | MASK0x0020_WOUND_FEET, C2_ATTACK_SELF)",
    ], "Open non-imaginary pits fall one level via F0154, draw the falling viewport, and apply 20 damage."))
    checks.append(require("DEFS.H:1025-1032", block("DEFS.H", 1025, 1032), [
        "MASK0x0001_PIT_IMAGINARY",
        "MASK0x0008_PIT_OPEN",
        "MASK0x0004_STAIRS_UP",
        "MASK0x0004_TELEPORTER_VISIBLE",
        "MASK0x0008_TELEPORTER_OPEN",
    ], "DEFS.H defines the canonical bit positions for pit/stairs/teleporter state bits."))
    checks.append(require("DUNGEON.C:2628-2695", block("DUNGEON.C", 2628, 2695), [
        "C02_ELEMENT_PIT",
        "MASK0x0008_PIT_OPEN",
        "C01_ELEMENT_CORRIDOR",
        "C05_ELEMENT_TELEPORTER",
        "MASK0x0008_TELEPORTER_OPEN",
        "MASK0x0004_TELEPORTER_VISIBLE",
        "C03_ELEMENT_STAIRS",
        "MASK0x0004_STAIRS_UP",
        "MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION",
    ], "F0172 SetSquareAspect: closed pit->corridor, teleporter requires OPEN+VISIBLE, stairs extracts UP bit."))
    checks.append(require("DUNVIEW.C:8318-8354", block("DUNVIEW.C", 8318, 8354), [
        "void F0128_DUNGEONVIEW_Draw_CPSF",
        "G0297_B_DrawFloorAndCeilingRequested",
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        "G0291_aauc_DungeonViewClickableBoxes",
    ], "The dungeon-view redraw entry refreshes floor/ceiling and clickable field state before field sampling."))
    return checks

def verify_firestaff() -> list[dict[str, Any]]:
    compat_c = read(ROOT / "memory_movement_pc34_compat.c")
    compat_h = read(ROOT / "memory_movement_pc34_compat.h")
    m11 = read(ROOT / "m11_game_view.c")
    cmake = read(ROOT / "CMakeLists.txt")
    checks: list[dict[str, Any]] = []
    checks.append(require("memory_movement_pc34_compat.c:movement_get_location_after_level_change", compat_c, [
        "movement_get_location_after_level_change",
        "globalX = (int)sourceMap->offsetMapX + *mapX;",
        "globalY = (int)sourceMap->offsetMapY + *mapY;",
        "targetLevel = (int)sourceMap->level + levelDelta;",
        "*mapX = globalX - (int)targetMap->offsetMapX;",
        "*mapY = globalY - (int)targetMap->offsetMapY;",
    ], "Firestaff implements F0154 offset/level semantics instead of map-index +/- shortcuts."))
    checks.append(require("memory_movement_pc34_compat.c:movement_get_stairs_exit_direction/F0705", compat_c, [
        "movement_get_stairs_exit_direction",
        "northSouth = (squareByte & 0x08) ? 0 : 1;",
        "blocked = (checkType == DUNGEON_ELEMENT_WALL || checkType == DUNGEON_ELEMENT_STAIRS);",
        "return (blocked << 1) + northSouth;",
        "stairUp = (squareByte & 0x04) ? 1 : 0;",
        "outResult->newDirection = movement_get_stairs_exit_direction(",
    ], "Firestaff consumes MASK0x0004_STAIRS_UP and F0155-style exit facing."))
    checks.append(require("memory_movement_pc34_compat.c:F0704 pit resolution", compat_c, [
        "int remaining = MOVEMENT_POST_MOVE_CHAIN_LIMIT;",
        "if (elementType == DUNGEON_ELEMENT_PIT &&",
        "(squareByte & 0x08) && !(squareByte & 0x01)",
        "movement_get_location_after_level_change(",
        "dungeon, cursor.mapIndex, 1, &cursor.mapX, &cursor.mapY",
        "outResolution->championFallDamage[i] += 20;",
    ], "Firestaff pit falls match PC chain limit, open/non-imaginary gating, F0154 mapping, and 20 damage."))
    checks.append(require("memory_movement_pc34_compat.h source contract", compat_h, [
        "MOVEMENT_POST_MOVE_CHAIN_LIMIT 100",
        "MASK0x0004_STAIRS_UP",
        "F0154_DUNGEON_GetLocationAfterLevelChange semantics",
        "F0155_DUNGEON_GetStairsExitDirection",
    ], "The public compat contract documents the source-owned level-transition semantics."))
    checks.append(require("m11_game_view.c redraw hooks", m11, [
        "if (m11_try_stairs_transition(state)) {",
        "return M11_GAME_INPUT_REDRAW;",
        "M11_GameView_CheckPostMoveTransitions",
        "m11_apply_post_move_environment_from_compat",
        "m11_refresh_hash(state);",
    ], "M11 calls the compat transition layer and requests redraw/hash refresh after transitions."))
    checks.append(require("m11_game_view.c:stairs_up_bit", m11, [
        "cell->square & 0x04) ? 1 : 0; /* ReDMCSB DEFS.H MASK0x0004_STAIRS_UP",
        "cell.square & 0x04) ? 1 : 0; /* ReDMCSB DEFS.H MASK0x0004_STAIRS_UP",
    ], "Stairs up/down rendering uses MASK0x0004_STAIRS_UP (not 0x01)."))
    checks.append(require("m11_game_view.c:pit_open_check", m11, [
        "!(cell.square & 0x08)) { /* not PIT_OPEN",
        "cell->square & 0x08) { /* PIT_OPEN",
    ], "Pit rendering checks MASK0x0008_PIT_OPEN before drawing the hole graphic."))
    checks.append(require("m11_game_view.c:teleporter_visible_check", m11, [
        "MASK0x0004_TELEPORTER_VISIBLE, MASK0x0008_TELEPORTER_OPEN",
        "(cell.square & 0x04) == 0 || (cell.square & 0x08) == 0",
    ], "Teleporter field rendering checks both VISIBLE and OPEN bits."))
    checks.append(require("CMakeLists.txt:dm1_v1_stairs_pits_viewport_source_lock", cmake, [
        "NAME dm1_v1_stairs_pits_viewport_source_lock",
        "verify_dm1_v1_stairs_pits_viewport_source_lock.py",
    ], "CTest runs this source-lock gate."))
    return checks

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()
    result = {"status": "PASS", "redmcsb_source_root": str(REDMCSB_SOURCE), "redmcsb_checks": verify_redmcsb(), "firestaff_checks": verify_firestaff()}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS dm1_v1_stairs_pits_viewport_source_lock wrote {args.out}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())

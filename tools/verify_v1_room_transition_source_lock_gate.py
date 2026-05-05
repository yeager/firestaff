#!/usr/bin/env python3
"""Static source-lock gate for DM1 V1 room/party-square transitions."""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
RED_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
GAMELOOP = RED_ROOT / "GAMELOOP.C"
MOVESENS = RED_ROOT / "MOVESENS.C"
DUNGEON = RED_ROOT / "DUNGEON.C"
ENTRANCE = RED_ROOT / "ENTRANCE.C"
FIRE_H = ROOT / "dm1_v1_room_transition_pc34_compat.h"
FIRE_C = ROOT / "dm1_v1_room_transition_pc34_compat.c"

def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1

def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos

def require_re(text: str, pattern: str, label: str) -> re.Match[str]:
    m = re.search(pattern, text, re.S)
    if not m:
        raise AssertionError(f"{label}: missing pattern {pattern!r}")
    return m

def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: marker out of order: {marker!r}")
        last = pos

def find_function(text: str, name: str) -> str:
    m = require_re(text, r"\b(?:static\s+)?(?:int|void|const char\*)\s+" + re.escape(name) + r"\s*\(", f"function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"function {name}: missing body")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[m.start():pos + 1]
    raise AssertionError(f"function {name}: unterminated body")

def main() -> int:
    gameloop = GAMELOOP.read_text(encoding="latin-1")
    movesens = MOVESENS.read_text(encoding="latin-1")
    dungeon = DUNGEON.read_text(encoding="latin-1")
    entrance = ENTRANCE.read_text(encoding="latin-1")
    fire_h = FIRE_H.read_text(encoding="utf-8")
    fire_c = FIRE_C.read_text(encoding="utf-8")
    citations: list[str] = []

    for needle, label in [
        ("if (G0327_i_NewPartyMapIndex != CM1_MAP_INDEX_NONE)", "GAMELOOP deferred new party map"),
        ("F0003_MAIN_ProcessNewPartyMap_CPSE(G0327_i_NewPartyMapIndex);", "GAMELOOP process new map"),
        ("F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, CM1_MAPX_NOT_ON_A_SQUARE, 0, G0306_i_PartyMapX, G0307_i_PartyMapY);", "GAMELOOP re-enter party square"),
        ("F0357_COMMAND_DiscardAllInput();", "GAMELOOP discard input after map switch"),
        ("F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "GAMELOOP redraw current view"),
    ]:
        citations.append(f"{label}: {GAMELOOP.name}:{line_no(gameloop, require(gameloop, needle, label))}")

    for needle, label in [
        ("G0306_i_PartyMapX = P0560_i_DestinationMapX;", "MOVESENS party X update"),
        ("G0307_i_PartyMapY = P0561_i_DestinationMapY;", "MOVESENS party Y update"),
        ("L0716_ui_Direction = G0308_i_PartyDirection;", "MOVESENS carries party direction"),
        ("F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination = L0712_ps_Teleporter->TargetMapIndex);", "MOVESENS teleporter current-map switch"),
        ("F0284_CHAMPION_SetPartyDirection(L0712_ps_Teleporter->Rotation);", "MOVESENS absolute teleporter rotation"),
        ("F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + L0712_ps_Teleporter->Rotation));", "MOVESENS relative teleporter rotation"),
        ("if ((AL0709_i_DestinationSquareType == C02_ELEMENT_PIT) && !L0713_B_ThingLevitates", "MOVESENS open pit fall branch"),
        ("L0715_ui_MapIndexDestination = F0154_DUNGEON_GetLocationAfterLevelChange", "MOVESENS pit level-change lookup"),
        ("F0324_CHAMPION_DamageAll_GetDamagedChampionCount(20", "MOVESENS pit fall damage"),
        ("G0402_B_UseRopeToClimbDownPit = C0_FALSE;", "MOVESENS rope reset"),
        ("G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination;", "MOVESENS deferred new party map"),
    ]:
        citations.append(f"{label}: {MOVESENS.name}:{line_no(movesens, require(movesens, needle, label))}")

    for needle, label in [
        ("int16_t F0154_DUNGEON_GetLocationAfterLevelChange", "DUNGEON level-change function"),
        ("L0250_i_NewMapX = L0254_ps_Map->OffsetMapX + *P0272_pi_MapX;", "DUNGEON global X lookup"),
        ("return L0255_i_TargetMapIndex;", "DUNGEON target map index"),
        ("void F0173_DUNGEON_SetCurrentMap", "DUNGEON set current map"),
        ("G0271_ppuc_CurrentMapData = G0279_pppuc_DungeonMapData[P0321_ui_MapIndex];", "DUNGEON map data pointer"),
        ("G0275_as_CurrentMapDoorInfo[0] = G0254_as_Graphic559_DoorInfo", "DUNGEON door info set"),
        ("void F0174_DUNGEON_SetCurrentMapAndPartyMap", "DUNGEON set party map"),
        ("F0007_MAIN_CopyBytes(M772_CAST_PC(L0316_puc_MapMetaData", "DUNGEON map metadata copies"),
    ]:
        citations.append(f"{label}: {DUNGEON.name}:{line_no(dungeon, require(dungeon, needle, label))}")

    require_in_order(entrance, [
        "while ((M708_ZONE_WIDTH(L3368_ai_XYZ) > 0) || (M708_ZONE_WIDTH(L3369_ai_XYZ) > 0))",
        "if ((L3364_i_ += 2) >= G2226_i_EntranceGraphicCount)",
        "F0616_CopyBitmap(G2225_puc_Bitmap_InterfaceEntranceScreen, G2219_puc_EntranceAnimationStep);",
        "F0654_Call_F0132_VIDEO_Blit(M772_CAST_PC(G0296_puc_Bitmap_Viewport)",
        "while (G0317_i_WaitForInputVerticalBlankCount < L3367_l_);",
        "L3367_l_ = G0317_i_WaitForInputVerticalBlankCount + 3;",
        "F0807_ENTRANCE_DrawAnimationStep();",
    ], "ENTRANCE door reveal loop")
    entrance_loop = require(entrance, "while ((M708_ZONE_WIDTH(L3368_ai_XYZ) > 0)", "ENTRANCE loop")
    citations.append(f"ENTRANCE door reveal loop: {ENTRANCE.name}:{line_no(entrance, entrance_loop)}")

    for needle in [
        "DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL",
        "DM1_V1_ROOM_TRANSITION_ENTRANCE_FRAMES 32",
        "DM1_V1_ROOM_TRANSITION_ENTRANCE_VBLANKS_PC34 3",
        "DM1_V1_ROOM_TRANSITION_CHAIN_LIMIT_PC34 100",
        "fadeFrames",
        "wipeFrames",
        "walkOutFrames",
        "doorTransitionFrames",
        "preserveChampionInventories",
        "preserveLeaderHandObject",
    ]:
        require(fire_h, needle, "Firestaff room transition header contract")

    build = find_function(fire_c, "DM1_V1_RoomTransition_BuildPlanPc34Compat")
    require_in_order(build, [
        "if (input->presentationMode != DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL)",
        "if (input->trigger == DM1_V1_ROOM_TRANSITION_TRIGGER_ENTRANCE)",
        "DM1_V1_ROOM_TRANSITION_VISUAL_ENTRANCE_DOORS",
        "DM1_V1_ROOM_TRANSITION_ENTRANCE_FRAMES",
        "changed = dm1_v1_pose_changed",
        "DM1_V1_ROOM_TRANSITION_VISUAL_VIEWPORT_REDRAW",
        "outPlan->fadeFrames = 0;",
        "outPlan->wipeFrames = 0;",
        "outPlan->walkOutFrames = 0;",
        "outPlan->doorTransitionFrames = 0;",
        "if (outPlan->mapChanged)",
        "outPlan->requestSetCurrentMap = 1;",
        "outPlan->requestSetCurrentMapAndPartyMap = 1;",
        "outPlan->requestInputDiscard = 1;",
        "outPlan->preserveChampionInventories = 1;",
        "outPlan->preserveChampionStats = 1;",
        "outPlan->preserveLeaderHandObject = 1;",
    ], "Firestaff room transition source-shape")

    evidence = find_function(fire_c, "DM1_V1_RoomTransition_SourceEvidencePc34Compat")
    for needle in [
        "GAMELOOP.C:58-64,90",
        "MOVESENS.C:441-451,492-517,538-606,817-822",
        "DUNGEON.C:1508-1558,2724-2740,2742-2762",
        "ENTRANCE.C:323-360,367",
    ]:
        require(evidence, needle, "Firestaff evidence string")

    print("DM1 V1 room transition source-lock gate passed")
    for citation in citations:
        print(f"- {citation}")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)

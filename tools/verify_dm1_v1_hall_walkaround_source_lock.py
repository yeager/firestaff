#!/usr/bin/env python3
"""ReDMCSB source-lock for DM1 V1 Hall of Champions walkaround.

Scope: walking/turning around the Hall, not recruiting.  The gate records exact
source citations for entry position, movement command routing, blockers around
walls/doors/fakewalls/groups, viewport refresh, mirror/alcove render metadata,
and separation from recruit/resurrect/reincarnate/cancel modal routes.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
OUT = ROOT / 'parity-evidence/verification/dm1_v1_hall_walkaround_source_lock.json'


def txt(file: str, start: int, end: int) -> str:
    return '\n'.join((SRC_ROOT / file).read_text(encoding='latin-1').splitlines()[start - 1:end])


def flat(s: str) -> str:
    return ' '.join(s.split())


def check(file: str, start: int, end: int, needles: list[str], point: str) -> dict:
    body = txt(file, start, end)
    fbody = flat(body)
    missing = [n for n in needles if flat(n) not in fbody]
    if missing:
        raise SystemExit(f'{file}:{start}-{end} missing {missing[0]!r}')
    return {'citation': f'{file}:{start}-{end}', 'line_range': [start, end], 'point': point, 'needles': needles, 'verified': True}


def absent(file: str, start: int, end: int, tokens: list[str], point: str) -> dict:
    body = txt(file, start, end)
    hit = [t for t in tokens if t in body]
    if hit:
        raise SystemExit(f'{file}:{start}-{end} unexpectedly contains {hit[0]!r}')
    return {'citation': f'{file}:{start}-{end}', 'line_range': [start, end], 'point': point, 'forbidden': tokens, 'verified': True}


def main() -> int:
    for file in ['LOADSAVE.C','COMMAND.C','CLIKMENU.C','DUNGEON.C','MOVESENS.C','GAMELOOP.C','DUNVIEW.C','DRAWVIEW.C','CLIKVIEW.C','CHAMPION.C']:
        if not (SRC_ROOT / file).exists():
            raise SystemExit(f'missing ReDMCSB source file: {SRC_ROOT / file}')

    checks = [
        check('LOADSAVE.C', 1940, 1944,
              ['if (G0298_B_NewGame)', 'G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F', 'G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F', 'G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003', 'G0309_i_PartyMapIndex = 0'],
              'New-game Hall entry comes from dungeon header initial party X/Y/direction on map 0.'),
        check('COMMAND.C', 106, 114,
              ['G0448_as_Graphic561_SecondaryMouseInput_Movement', 'C001_COMMAND_TURN_LEFT', 'C003_COMMAND_MOVE_FORWARD', 'C002_COMMAND_TURN_RIGHT', 'C006_COMMAND_MOVE_LEFT', 'C005_COMMAND_MOVE_BACKWARD', 'C004_COMMAND_MOVE_RIGHT', 'C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168'],
              'Movement mouse table separates arrows from viewport click walking/front-wall touch.'),
        check('COMMAND.C', 272, 305,
              ['G0459_as_Graphic561_SecondaryKeyboardInput_Movement', 'C003_COMMAND_MOVE_FORWARD', 'C006_COMMAND_MOVE_LEFT', 'C004_COMMAND_MOVE_RIGHT', 'C005_COMMAND_MOVE_BACKWARD', 'C001_COMMAND_TURN_LEFT', 'C002_COMMAND_TURN_RIGHT'],
              'Keyboard walkaround maps only to turn/step commands.'),
        check('COMMAND.C', 2150, 2156,
              ['if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))', 'F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command)', 'if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))', 'F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command)'],
              'Dispatcher sends turns/steps to walkaround handlers before later status/inventory/modal command branches.'),
        check('CLIKMENU.C', 142, 173,
              ['F0365_COMMAND_ProcessTypes1To2_TurnParty', 'F0151_DUNGEON_GetSquare(G0306_i_PartyMapX, G0307_i_PartyMapY)', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE)', 'F0284_CHAMPION_SetPartyDirection', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C1_TRUE)'],
              'Turning stays on the current Hall square and only processes local sensor departure/arrival plus party direction.'),
        check('CHAMPION.C', 117, 130,
              ['if (P0600_i_Direction == G0308_i_PartyDirection)', 'L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection', 'L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta)', 'L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta)', 'G0308_i_PartyDirection = P0600_i_Direction', 'F0296_CHAMPION_DrawChangedObjectIcons()'],
              'Party turn rotates champion cells/directions and redraws icons; no recruit/modal side effect.'),
        check('CLIKMENU.C', 224, 233,
              ['G0465_ai_Graphic561_MovementArrowToStepForwardCount', '1,   /* Forward */', '0,   /* Right */', '-1,  /* Backward */', '0 }; /* Left */', 'G0466_ai_Graphic561_MovementArrowToStepRightCount', '0,    /* Forward */', '1,    /* Right */', '0,    /* Backward */', '-1 }; /* Left */'],
              'Forward/right/back/left use original relative step vectors.'),
        check('CLIKMENU.C', 264, 328,
              ['F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 'L1116_i_SquareType == C00_ELEMENT_WALL', 'L1116_i_SquareType == C04_ELEMENT_DOOR', 'L1116_i_SquareType == C06_ELEMENT_FAKEWALL', 'F0175_GROUP_GetThing(L1121_i_MapX, L1122_i_MapY)', 'if (L1117_B_MovementBlocked)', 'F0357_COMMAND_DiscardAllInput()', 'F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY)'],
              'Stepping computes target coordinates and blocks wall/door/fakewall/group targets before moving.'),
        check('DUNGEON.C', 1371, 1391,
              ['F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 'G0233_ai_Graphic559_DirectionToStepEastCount', 'G0234_ai_Graphic559_DirectionToStepNorthCount', 'P0253_i_Direction += 1, P0253_i_Direction &= 3', 'P0255_i_StepsRightCount'],
              'Relative walk vectors use facing-direction deltas and a simulated right turn for strafe deltas.'),
        check('DUNGEON.C', 1423, 1475,
              ['F0151_DUNGEON_GetSquare', 'P0258_i_MapX >= 0', 'P0258_i_MapX < G0273_i_CurrentMapWidth', 'P0259_i_MapY >= 0', 'P0259_i_MapY < G0274_i_CurrentMapHeight', 'return G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY]', 'return M035_SQUARE(C00_ELEMENT_WALL, 0)'],
              'Out-of-bounds walk targets become wall squares.'),
        check('MOVESENS.C', 799, 821,
              ['if (P0557_T_Thing == C0xFFFF_THING_PARTY)', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY', 'F0173_DUNGEON_SetCurrentMap(L0715_ui_MapIndexDestination)', 'F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)', 'F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY', 'G0327_i_NewPartyMapIndex = L0715_ui_MapIndexDestination'],
              'Successful party movement runs source/destination sensors and same-square group consequence logic.'),
        check('GAMELOOP.C', 80, 90,
              ['if (!G0300_B_PartyIsResting)', 'if (!G0299_ui_CandidateChampionOrdinal)', 'F0457_START_DrawEnabledMenus_CPSF()', 'if (!G0423_i_InventoryChampionOrdinal)', 'F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)'],
              'Normal Hall walkaround redraws viewport from current party direction/position when no candidate/inventory modal is active.'),
        check('DRAWVIEW.C', 709, 722,
              ['F0097_DUNGEONVIEW_DrawViewport', 'G0324_B_DrawViewportRequested = C1_TRUE', 'M526_WaitVerticalBlank'],
              'Viewport refresh is requested and synchronized to vertical blank.'),
        check('DUNVIEW.C', 371, 377,
              ['G2026_ac_ViewSquareIndexToViewLane', 'G2027_ac_ViewSquareIndexToViewDepth', 'G2035_ac_ViewSquareIndexToFieldAspectIndex'],
              'View lane/depth/aspect tables define what Hall walls/mirrors appear while walking/turning.'),
        check('DUNVIEW.C', 2458, 2464,
              ['if (!G0269_ps_CurrentMap->C.CreatureTypeCount', 'These graphics are loaded only if there are no creature types allowed on the map (for the Hall of Champions / Prison)', 'champion mirrors'],
              'Hall/prison maps preload champion mirror graphics because no creature types are allowed.'),
        check('DUNVIEW.C', 2672, 2684,
              ['G0267_ai_CurrentMapAlcoveOrnamentIndices', 'G0268_ai_CurrentMapFountainOrnamentIndices', 'G0269_ps_CurrentMap->B.WallOrnamentCount', 'G0101_as_CurrentMapWallOrnamentsInfo', 'G0192_auc_Graphic558_AlcoveOrnamentIndices', 'G0266_i_CurrentMapViAltarWallOrnamentIndex'],
              'Map wall-ornament scan classifies alcoves independently of movement.'),
        check('DUNGEON.C', 2608, 2612,
              ['M039_TYPE(L0308_ps_Sensor) == C127_SENSOR_WALL_CHAMPION_PORTRAIT', 'G0289_i_DungeonView_ChampionPortraitOrdinal = M000_INDEX_TO_ORDINAL(M040_DATA(L0308_ps_Sensor))'],
              'Champion mirror detection is a front-wall render marker, not movement collision.'),
        check('DUNVIEW.C', 3912, 3919,
              ['P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT', 'G0289_i_DungeonView_ChampionPortraitOrdinal--', 'C026_GRAPHIC_CHAMPION_PORTRAITS', 'G0109_auc_Graphic558_Box_ChampionPortraitOnWall', 'A portrait is 32x29 pixels'],
              'Only the rendered front-wall portrait box gets champion portrait art.'),
        check('CLIKVIEW.C', 406, 432,
              ['if (G0415_ui_LeaderEmptyHanded)', 'C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT', 'if (!G0286_B_FacingAlcove)', 'F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor()'],
              'Click-walking/front-wall touch reaches portrait sensors only through the wall-ornament route; alcove-facing clicks skip it.'),
        check('MOVESENS.C', 1501, 1503,
              ['case C127_SENSOR_WALL_CHAMPION_PORTRAIT', 'F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)', 'goto T0275058_ProceedToNextThing'],
              'Recruit/candidate modal is isolated to explicitly touching a C127 champion portrait sensor.'),
        check('COMMAND.C', 1985, 1991,
              ['case M568_PANEL_RESURRECT_REINCARNATE', 'if (!G0415_ui_LeaderEmptyHanded)', 'G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel', 'F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel'],
              'The /reincarnate/cancel routes are panel-modal routes, separate from walkaround.'),
        absent('CLIKMENU.C', 142, 347,
               ['F0280_CHAMPION_AddCandidateChampionToParty', 'F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel', 'M568_PANEL_RESURRECT_REINCARNATE', 'C160_COMMAND', 'C161_COMMAND', 'C162_COMMAND'],
               'Turn/step handlers never directly open candidate or resurrect/reincarnate/cancel modal routes.'),
        absent('COMMAND.C', 106, 114,
               ['C160_COMMAND', 'C161_COMMAND', 'C162_COMMAND', 'M568_PANEL_RESURRECT_REINCARNATE'],
               'Mouse movement table has no resurrect/reincarnate/cancel commands.'),
        absent('COMMAND.C', 272, 305,
               ['C160_COMMAND', 'C161_COMMAND', 'C162_COMMAND', 'M568_PANEL_RESURRECT_REINCARNATE'],
               'Keyboard movement table has no resurrect/reincarnate/cancel commands.'),
    ]
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps({'schema': 'dm1_v1_hall_walkaround_source_lock.v1', 'redmcsb_root': str(SRC_ROOT), 'source_checks': checks, 'status': 'PASS'}, indent=2) + '\n')
    print(f'PASS wrote {OUT}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

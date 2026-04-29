#!/usr/bin/env python3
"""Verify that the current DM1/V1 work is grounded in ReDMCSB source facts.

This is intentionally a source-lock gate, not a screenshot/pixel gate.  It
checks the exact ReDMCSB control points Daniel called out and the Firestaff V1
bridges that must remain faithful to them.
"""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
REDMCSB = pathlib.Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

checks: list[tuple[pathlib.Path, str, str]] = [
    (REDMCSB / "TITLE.C", r"M526_WaitVerticalBlank\(\).*BUG0_71|BUG0_71.*title screen", "TITLE.C locks title timing to VBlank/BUG0_71"),
    (REDMCSB / "TITLE.C", r"Delay\(25L\)", "TITLE.C keeps the source 25 tick title delay"),
    (REDMCSB / "ENTRANCE.C", r"F0438_STARTEND_OpenEntranceDoors", "ENTRANCE.C has the source entrance-door animation routine"),
    (REDMCSB / "ENTRANCE.C", r"M526_WaitVerticalBlank\(\).*BUG0_71|BUG0_71.*doors open", "ENTRANCE.C locks entrance door timing to VBlank/BUG0_71"),
    (REDMCSB / "COMMAND.C", r"C080_COMMAND_CLICK_IN_DUNGEON_VIEW.*C007_ZONE_VIEWPORT", "COMMAND.C maps C007 viewport left-click to C080"),
    (REDMCSB / "COMMAND.C", r"F0377_COMMAND_ProcessType80_ClickInDungeonView", "COMMAND.C dispatches C080 to CLIKVIEW F0377"),
    (REDMCSB / "CLIKVIEW.C", r"F0377_COMMAND_ProcessType80_ClickInDungeonView", "CLIKVIEW.C defines C080 click-in-dungeon-view behavior"),
    (REDMCSB / "CLIKVIEW.C", r"F0275_SENSOR_IsTriggeredByClickOnWall", "CLIKVIEW.C triggers wall sensors from viewport clicks"),
    (REDMCSB / "CLIKVIEW.C", r"G4055_s_LeaderHandObject\.Thing", "CLIKVIEW.C uses G4055 leader-hand object in viewport interactions"),
    (REDMCSB / "MOVESENS.C", r"LEADER_HAND_OBJECT\s+G4055_s_LeaderHandObject", "MOVESENS.C owns the source G4055 leader-hand object"),
    (REDMCSB / "MOVESENS.C", r"BOOLEAN\s+F0267_MOVE_GetMoveResult_CPSCE", "MOVESENS.C source-locks object/party move result behavior"),
    (REDMCSB / "MOVESENS.C", r"BOOLEAN\s+F0275_SENSOR_IsTriggeredByClickOnWall", "MOVESENS.C source-locks wall-click sensor trigger behavior"),
    (REDMCSB / "DRAWVIEW.C", r"F0097_DUNGEONVIEW_DrawViewport", "DRAWVIEW.C defines the viewport redraw/request path"),
    (REDMCSB / "DRAWVIEW.C", r"C007_ZONE_VIEWPORT", "DRAWVIEW.C blits the viewport to C007_ZONE_VIEWPORT"),
    (REDMCSB / "DRAWVIEW.C", r"M526_WaitVerticalBlank\(\).*viewport", "DRAWVIEW.C waits VBlank after requesting viewport draw"),
    (ROOT / "m11_game_view.c", r"M11_GameView_GetV1ViewportZoneId[\s\S]*return 7;", "Firestaff exposes source C007 viewport zone id"),
    (ROOT / "m11_game_view.c", r"Source runtime bridge[\s\S]*G4055_s_LeaderHandObject", "Firestaff documents G4055-equivalent leader-hand bridge"),
    (ROOT / "m11_game_view.c", r"M11_GameView_SetV1LeaderHandObject[\s\S]*leaderHandObjectPresent\s*=\s*1", "Firestaff sets dedicated leader-hand object state"),
    (ROOT / "m11_game_view.c", r"M11_GameView_ClearV1LeaderHandObject[\s\S]*leaderHandObjectPresent\s*=\s*0", "Firestaff clears dedicated leader-hand object state"),
    (ROOT / "m11_game_view.h", r"G4055_s_LeaderHandObject[\s\S]*distinct from any champion inventory", "Firestaff header preserves G4055 as separate from champion slots"),
    (ROOT / "m11_game_view.h", r"M11_GameView_GetV1MouseCommandForPoint", "Firestaff exposes bounded source-backed mouse command resolver"),
]

failed = False
for path, pattern, desc in checks:
    if not path.exists():
        print(f"FAIL {desc}: missing {path}")
        failed = True
        continue
    text = path.read_text(errors="replace")
    if re.search(pattern, text, re.MULTILINE):
        print(f"PASS {desc}")
    else:
        print(f"FAIL {desc}: pattern not found in {path}")
        failed = True

sys.exit(1 if failed else 0)

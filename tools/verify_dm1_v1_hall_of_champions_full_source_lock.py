#!/usr/bin/env python3
"""Source-lock the DM1 V1 Hall of Champions route against ReDMCSB.

This is intentionally a provenance/regression gate: it does not guess at UI
semantics. It verifies the original-source chain for entering the game,
reaching/looking at champion mirrors, rendering portrait sensors, clicking the
portrait, opening candidate inventory, routing the resurrect/reincarnate/cancel
panel, and suppressing normal party actions while a candidate is open.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()
OUT = ROOT / "parity-evidence/verification/dm1_v1_hall_of_champions_full_source_lock.json"

SRC = {name: REDMCSB / name for name in [
    "ENTRANCE.C", "CHAMPION.C", "COMMAND.C", "CLIKMENU.C", "CLIKVIEW.C",
    "MOVESENS.C", "PANEL.C", "CHAMDRAW.C", "DUNGEON.C", "DUNVIEW.C",
    "COORD.C", "REVIVE.C",
]}

EXISTING_GATES = [
    ROOT / "tools/verify_v1_champion_portrait_click_source_path.py",
    ROOT / "tools/verify_v1_champion_portrait_click_geometry.py",
]


def lines(path: Path) -> list[str]:
    return path.read_text(encoding="latin-1").splitlines()


def block(path: Path, start: int, end: int) -> str:
    ls = lines(path)
    return "\n".join(ls[start - 1:end])


def require(citation: str, path: Path, start: int, end: int, needles: list[str], point: str) -> dict[str, Any]:
    text = block(path, start, end)
    compact = " ".join(text.split())
    missing = [n for n in needles if " ".join(n.split()) not in compact]
    if missing:
        raise SystemExit(f"{citation} missing expected text: {missing[0]}")
    return {"citation": citation, "source": str(path), "line_range": [start, end], "point": point, "needles": needles, "verified": True}


def verify_existing_gates() -> list[dict[str, Any]]:
    out = []
    for gate in EXISTING_GATES:
        if not gate.exists():
            raise SystemExit(f"missing existing gate: {gate}")
        text = gate.read_text()
        if gate.name.endswith("source_path.py"):
            needles = ["C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty"]
        else:
            needles = ["G0109_auc_Graphic558_Box_ChampionPortraitOnWall", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"]
        for needle in needles:
            if needle not in text:
                raise SystemExit(f"{gate} missing {needle}")
        out.append({"path": str(gate), "verified": True})
    return out


def verify_source() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    checks.append(require("ENTRANCE.C:739-747", SRC["ENTRANCE.C"], 739, 747,
        ["G0441_ps_PrimaryMouseInput = G0445_as_Graphic561_PrimaryMouseInput_Entrance", "G0442_ps_SecondaryMouseInput = NULL", "G0444_ps_SecondaryKeyboardInput = NULL"],
        "Entrance screen owns primary input until the door/start path transfers to gameplay."))
    checks.append(require("ENTRANCE.C:775-791", SRC["ENTRANCE.C"], 775, 791,
        ["C003_GRAPHIC_ENTRANCE_RIGHT_DOOR", "C002_GRAPHIC_ENTRANCE_LEFT_DOOR", "C004_GRAPHIC_ENTRANCE", "F0478_MEMORY_CloseGraphicsDat_CPSDF"],
        "Entrance loads/decompresses the door and entrance graphics before gameplay."))
    checks.append(require("CLIKMENU.C:180-347", SRC["CLIKMENU.C"], 180, 347,
        ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "L1116_i_SquareType == C00_ELEMENT_WALL", "F0357_COMMAND_DiscardAllInput"],
        "Hall traversal uses the original menu movement command path and legality gates."))
    checks.append(require("DUNGEON.C:2608-2612", SRC["DUNGEON.C"], 2608, 2612,
        ["M039_TYPE(L0308_ps_Sensor) == C127_SENSOR_WALL_CHAMPION_PORTRAIT", "G0289_i_DungeonView_ChampionPortraitOrdinal = M000_INDEX_TO_ORDINAL(M040_DATA(L0308_ps_Sensor))"],
        "The rendered wall portrait ordinal comes directly from the C127 portrait sensor data."))
    checks.append(require("DUNVIEW.C:525-525", SRC["DUNVIEW.C"], 525, 525,
        ["G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4] = { 96, 127, 35, 63 }"],
        "The front-wall champion portrait clickable box is x=96..127 y=35..63 viewport-relative."))
    checks.append(require("DUNVIEW.C:3916-3919", SRC["DUNVIEW.C"], 3916, 3919,
        ["C026_GRAPHIC_CHAMPION_PORTRAITS", "G0109_auc_Graphic558_Box_ChampionPortraitOnWall", "A portrait is 32x29 pixels"],
        "Portrait rendering blits the champion portrait atlas through the same front-wall portrait box."))
    checks.append(require("COORD.C:1693-1749", SRC["COORD.C"], 1693, 1749,
        ["G2067_i_ViewportScreenX = 0", "G2068_i_ViewportScreenY = 33", "G2078_C32_PortraitWidth = 32", "G2079_C29_PortraitHeight = 29"],
        "PC viewport origin and portrait dimensions match the recommended portrait click geometry."))
    checks.append(require("COMMAND.C:1985-1991", SRC["COMMAND.C"], 1985, 1991,
        ["case M568_PANEL_RESURRECT_REINCARNATE", "if (!G0415_ui_LeaderEmptyHanded)", "G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"],
        "The resurrect/reincarnate/cancel panel only dispatches when the leader hand is empty."))
    checks.append(require("COMMAND.C:228-240", SRC["COMMAND.C"], 228, 240,
        ["C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "C162_COMMAND_CLICK_IN_PANEL_CANCEL"],
        "Panel mouse-input table defines the three Hall decisions: resurrect, reincarnate, cancel."))
    checks.append(require("CLIKVIEW.C:347-349", SRC["CLIKVIEW.C"], 347, 349,
        ["P0752_i_X -= G2067_i_ViewportScreenX", "P0753_i_Y -= G2068_i_ViewportScreenY"],
        "Dungeon-view clicks normalize screen coordinates to viewport-relative coordinates."))
    checks.append(require("CLIKVIEW.C:406-432", SRC["CLIKVIEW.C"], 406, 432,
        ["G0415_ui_LeaderEmptyHanded", "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "if (!G0286_B_FacingAlcove)", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"],
        "Empty-hand portrait/front-wall click routes through C05 to the wall-sensor touch handler."))
    checks.append(require("MOVESENS.C:1501-1503", SRC["MOVESENS.C"], 1501, 1503,
        ["case C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)"],
        "The wall portrait sensor opens the candidate champion flow with sensor data as champion index."))
    checks.append(require("REVIVE.C:272-286", SRC["REVIVE.C"], 272, 286,
        ["G0299_ui_CandidateChampionOrdinal = L0799_ui_PreviousPartyChampionCount + 1", "if (++G0305_ui_PartyChampionCount == 1)", "F0368_COMMAND_SetLeader", "F0388_MENUS_ClearActingChampion"],
        "Opening a mirror creates a candidate party slot, sets candidate ordinal, and adjusts leader/acting champion UI."))
    checks.append(require("REVIVE.C:295-303", SRC["REVIVE.C"], 295, 303,
        ["L0802_i_MapX = G0306_i_PartyMapX", "L0804_ui_ChampionObjectsCell = M018_OPPOSITE(G0308_i_PartyDirection)", "F0161_DUNGEON_GetSquareFirstThing", "M011_CELL(L0793_T_Thing) == L0804_ui_ChampionObjectsCell"],
        "Candidate possessions are collected from the mirror square/opposite wall cell."))
    checks.append(require("PANEL.C:1619-1636", SRC["PANEL.C"], 1619, 1636,
        ["F0346_INVENTORY_DrawPanel_ResurrectReincarnate", "G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE", "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE"],
        "Candidate inventory draws and labels the resurrect/reincarnate panel."))
    checks.append(require("PANEL.C:1654-1656", SRC["PANEL.C"], 1654, 1656,
        ["if (G0299_ui_CandidateChampionOrdinal)", "F0346_INVENTORY_DrawPanel_ResurrectReincarnate", "return"],
        "Any candidate inventory panel redraw is forced to the Hall decision panel."))
    checks.append(require("PANEL.C:2377-2384", SRC["PANEL.C"], 2377, 2384,
        ["if (G0299_ui_CandidateChampionOrdinal)", "C562_ZONE_SAVE_GAME_ICON", "C564_ZONE_REST_ICON", "C566_ZONE_CLOSE_INVENTORY_ICON"],
        "Candidate inventory hides save/rest/close affordances that would escape the Hall modal state."))
    checks.append(require("CHAMDRAW.C:536-545", SRC["CHAMDRAW.C"], 536, 545,
        ["If drawing a slot for a champion other than the champion whose inventory is open", "G0299_ui_CandidateChampionOrdinal == M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex)", "return"],
        "Slot rendering is candidate-aware and does not draw unrelated champion hand slots over candidate UI."))
    checks.append(require("CHAMDRAW.C:1210-1212", SRC["CHAMDRAW.C"], 1210, 1212,
        ["L0883_ui_InventoryChampionOrdinal = G0423_i_InventoryChampionOrdinal", "if (G0299_ui_CandidateChampionOrdinal && !L0883_ui_InventoryChampionOrdinal)", "return"],
        "Changed leader-hand object drawing is suppressed if candidate mode has no inventory champion."))
    checks.append(require("CHAMPION.C:678-679", SRC["CHAMPION.C"], 678, 679,
        ["if (G0299_ui_CandidateChampionOrdinal)", "return"],
        "Status-box slot clicks are ignored while a candidate champion is open."))
    checks.append(require("CHAMPION.C:1946-1947", SRC["CHAMPION.C"], 1946, 1947,
        ["M000_INDEX_TO_ORDINAL(P0666_i_ChampionIndex) == G0299_ui_CandidateChampionOrdinal", "return"],
        "Poison/damage effects skip the candidate champion ordinal."))
    checks.append(require("COMMAND.C:2159-2184", SRC["COMMAND.C"], 2159, 2184,
        ["C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX", "!G0299_ui_CandidateChampionOrdinal", "C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0", "C04_CHAMPION_CLOSE_INVENTORY", "!G0299_ui_CandidateChampionOrdinal"],
        "Champion status/inventory toggles and close-inventory commands are blocked during candidate mode."))
    checks.append(require("COMMAND.C:2336-2370", SRC["COMMAND.C"], 2336, 2370,
        ["if (!G0299_ui_CandidateChampionOrdinal)", "G0300_B_PartyIsResting = C1_TRUE", "C146_COMMAND_WAKE_UP", "G0305_ui_PartyChampionCount > 0", "!G0299_ui_CandidateChampionOrdinal"],
        "Rest/wake/save paths cannot be entered from the Hall candidate modal."))
    checks.append(require("REVIVE.C:744-783", SRC["REVIVE.C"], 744, 783,
        ["P0598_i_Command == C162_COMMAND_CLICK_IN_PANEL_CANCEL", "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)", "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "G0305_ui_PartyChampionCount--", "F0457_START_DrawEnabledMenus_CPSF"],
        "Cancel closes candidate inventory, clears candidate ordinal, removes the temporary party slot, and redraws menus."))
    checks.append(require("REVIVE.C:785-807", SRC["REVIVE.C"], 785, 807,
        ["G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "F0164_DUNGEON_UnlinkThingFromList", "M044_SET_TYPE_DISABLED", "P0598_i_Command == C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "F0281_CHAMPION_Rename"],
        "Resurrect/reincarnate acceptance clears candidate mode, unlinks possessions, disables the mirror sensor, and branches to reincarnation rename/stat reset when requested."))
    return checks


def main() -> int:
    for path in SRC.values():
        if not path.exists():
            raise SystemExit(f"missing ReDMCSB source file: {path}")
    result = {
        "schema": "dm1_v1_hall_of_champions_full_source_lock.v1",
        "redmcsb_root": str(REDMCSB),
        "scope": "DM1 V1 Hall of Champions: entrance, movement to mirrors, portrait render/click, candidate inventory, resurrect/reincarnate/cancel, candidate modal blockers",
        "existing_portrait_gates": verify_existing_gates(),
        "source_checks": verify_source(),
        "status": "PASS",
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n")
    print(f"PASS wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

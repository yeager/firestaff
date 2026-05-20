#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_ui_overlay_affordance_routes_source_lock.json"


def redmcsb_source_root() -> Path:
    candidates = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.extend([
        Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser(),
    ])
    for candidate in candidates:
        if (candidate / "COMMAND.C").exists() and (candidate / "CLIKMENU.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()

SOURCE_CHECKS = [
    {
        "file": "COMMAND.C",
        "start": 375,
        "end": 405,
        "function": "G0447/G0448 mouse route tables",
        "needles": [
            "G0447_as_Graphic561_PrimaryMouseInput_Interface",
            "C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0",
            "C100_COMMAND_CLICK_IN_SPELL_AREA",
            "C111_COMMAND_CLICK_IN_ACTION_AREA",
            "G0448_as_Graphic561_SecondaryMouseInput_Movement",
            "C001_COMMAND_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD",
            "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
        ],
        "claim": "primary interface and secondary movement overlay areas are source-owned command/zone records",
    },
    {
        "file": "COMMAND.C",
        "start": 407,
        "end": 451,
        "function": "G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory",
        "needles": [
            "C011_COMMAND_CLOSE_INVENTORY",
            "C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND",
            "C070_COMMAND_CLICK_ON_MOUTH",
            "C071_COMMAND_CLICK_ON_EYE",
            "C081_COMMAND_CLICK_IN_PANEL",
        ],
        "claim": "inventory overlay affordances route to the V1 inventory table instead of V2-local inventory mutations",
    },
    {
        "file": "COMMAND.C",
        "start": 461,
        "end": 497,
        "function": "G0452/G0453/G0454/G0455 subroute tables",
        "needles": [
            "G0452_as_Graphic561_MouseInput_ActionAreaNames",
            "C113_COMMAND_CLICK_IN_ACTION_AREA_ACTION_0",
            "G0453_as_Graphic561_MouseInput_ActionAreaIcons",
            "C116_COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION",
            "G0454_as_Graphic561_MouseInput_SpellArea",
            "C101_COMMAND_CLICK_IN_SPELL_AREA_SYMBOL_1",
            "C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL",
            "G0455_as_Graphic561_MouseInput_ChampionNamesHands",
            "C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND",
        ],
        "claim": "action, rune/spell, and champion-hand/name affordances stay on the V1 subroute tables",
    },
    {
        "file": "COMMAND.C",
        "start": 1394,
        "end": 1439,
        "function": "F0358_COMMAND_GetCommandFromMouseInput_CPSC",
        "needles": [
            "while (L1107_Command = P0721_ps_MouseInput->Command)",
            "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button",
            "P0722_i_X <= P0721_ps_MouseInput->Box.X2",
        ],
        "claim": "mouse dispatch is a command/zone/button hit test, not direct UI action execution",
    },
    {
        "file": "COMMAND.C",
        "start": 1641,
        "end": 1660,
        "function": "F0359_COMMAND_ProcessClick_CPSC queue write",
        "needles": [
            "G0441_ps_PrimaryMouseInput",
            "G0442_ps_SecondaryMouseInput",
            "G0432_as_CommandQueue",
            ".Command = L1109_i_Command",
            ".X = P0725_i_X",
            ".Y = P0726_i_Y",
        ],
        "claim": "click hits are enqueued with coordinates for the source command processor",
    },
    {
        "file": "COMMAND.C",
        "start": 2045,
        "end": 2184,
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox",
            "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
            "F0355_INVENTORY_Toggle_CPSE",
        ],
        "claim": "source queue dispatch owns movement, champion, slot/inventory, and inventory-toggle transactions",
    },
    {
        "file": "COMMAND.C",
        "start": 352,
        "end": 518,
        "function": "F0369/F0370 spell-area processing and route tables",
        "needles": [
            "F0399_MENUS_AddChampionSymbol",
            "F0400_MENUS_DeleteChampionSymbol",
            "F0408_MENUS_GetClickOnSpellCastResult",
            "F0394_MENUS_SetMagicCasterAndDrawSpellArea",
        ],
        "claim": "rune/cast/recant affordances remain owned by source spell transactions",
    },
    {
        "file": "CLIKCHAM.C",
        "start": 24,
        "end": 35,
        "function": "F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox",
        "needles": [
            "F0368_COMMAND_SetLeader",
            "G0455_as_Graphic561_MouseInput_ChampionNamesHands",
            "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        ],
        "claim": "champion-name and hand clicks route through V1 leader/slot transactions",
    },
    {
        "file": "CLIKMENU.C",
        "start": 519,
        "end": 587,
        "function": "F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE",
        "needles": [
            "G0452_as_Graphic561_MouseInput_ActionAreaNames",
            "F0391_MENUS_DidClickTriggerAction",
            "G0453_as_Graphic561_MouseInput_ActionAreaIcons",
            "F0389_MENUS_ProcessCommands116To119_SetActingChampion",
        ],
        "claim": "action rows/icons route through V1 menu transactions",
    },
]

FIRESTAFF_CHECKS = {
    "src/shared/touch_click_zone_matrix_pc34_compat.c": [
        "movement.forward",
        "movement.backward",
        "inventory.toggle_leader",
        "inventory.close_right",
        "inventory.mouth",
        "inventory.eye",
        "inventory.panel",
        "champion0.toggle_box",
        "champion3.action_hand",
        "spell.symbol1",
        "spell.cast",
        "spell.recant",
        "action.parent",
        "action.row1",
        "action.icon1",
        "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary",
        "TOUCHCLICK_Compat_MapScaledScreenPointToDispatch",
        "TOUCHCLICK_Compat_MapViewportLocalPointToDispatch",
    ],
    "src/dm1v2/dm1_v2_hud_interaction_pc34.c": [
        "TOUCHCLICK_Compat_HitTestWithButton",
        "action_area_routes_GetTouchMatrixInvariant",
        "champion_name_hand_routes_GetInvariant",
    ],
    "src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c": [
        "return dm1_v2_affordance_result(0, affordance, command, route);",
        "dm1_v2_movement_command_route_for_presentation(1, command)",
    ],
    "tests/test_dm1_v2_hud_interaction_pc34.c": [
        "champion0.toggle_box",
        "champion3.action_hand",
        "action.icon1",
    ],
    "tests/test_dm1_v2_touch_controller_affordance_pc34.c": [
        "V1 touch/click parity guard",
        "route.accepted == 0",
        "runtime.lastCommand == 0",
    ],
    "tools/verify_dm1_v2_hud_interaction_source_lock.py": [
        "forbiddenDirectTransactions",
        "V2 HUD interaction must not call V1 transaction owner directly",
    ],
    "tools/verify_dm1_v2_touch_controller_affordance_source_lock.py": [
        "runtime.lastCommand == 0",
        "dm1_v2_touch_controller_affordance_source_lock",
    ],
}

FORBIDDEN_DIRECT_TRANSACTIONS = [
    "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty",
    "F0355_INVENTORY_Toggle_CPSE",
    "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
    "F0399_MENUS_AddChampionSymbol",
    "F0400_MENUS_DeleteChampionSymbol",
    "F0408_MENUS_GetClickOnSpellCastResult",
    "F0391_MENUS_DidClickTriggerAction",
    "F0389_MENUS_ProcessCommands116To119_SetActingChampion",
]

errors: list[str] = []
anchors = []

for check in SOURCE_CHECKS:
    path = SOURCE / check["file"]
    if not path.exists():
        errors.append(f"missing ReDMCSB source {path}")
        continue
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    start = int(check["start"])
    end = int(check["end"])
    if start < 1 or end > len(lines) or start > end:
        errors.append(f"invalid source range {check['file']}:{start}-{end}")
        continue
    excerpt = "\n".join(lines[start - 1:end])
    for needle in check["needles"]:
        if needle not in excerpt:
            errors.append(f"{check['file']}:{start}-{end}: missing {needle!r}")
    anchors.append({
        "file": check["file"],
        "function": check["function"],
        "lineRange": f"{start}-{end}",
        "claim": check["claim"],
        "needles": check["needles"],
    })

for rel, needles in FIRESTAFF_CHECKS.items():
    path = ROOT / rel
    if not path.exists():
        errors.append(f"missing Firestaff file {rel}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    for needle in needles:
        if needle not in text:
            errors.append(f"{rel}: missing {needle!r}")

for rel in [
    "src/dm1v2/dm1_v2_hud_interaction_pc34.c",
    "src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c",
]:
    text = (ROOT / rel).read_text(encoding="utf-8", errors="replace")
    for forbidden in FORBIDDEN_DIRECT_TRANSACTIONS:
        if forbidden in text:
            errors.append(f"{rel}: overlay affordance bypasses source-owned transaction {forbidden}")

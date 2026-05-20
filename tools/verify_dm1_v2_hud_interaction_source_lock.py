#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
source_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
evidence_path = root / "parity-evidence/verification/dm1_v2_hud_interaction_source_lock.json"

source_checks = [
    {
        "file": "COMMAND.C",
        "start": 375,
        "end": 395,
        "needles": [
            "G0447_as_Graphic561_PrimaryMouseInput_Interface",
            "C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0",
            "C111_COMMAND_CLICK_IN_ACTION_AREA",
        ],
        "meaning": "primary interface mouse table routes champion status boxes, spell area, and action-area parent through source command IDs",
    },
    {
        "file": "COMMAND.C",
        "start": 461,
        "end": 497,
        "needles": [
            "G0452_as_Graphic561_MouseInput_ActionAreaNames",
            "G0453_as_Graphic561_MouseInput_ActionAreaIcons",
            "G0455_as_Graphic561_MouseInput_ChampionNamesHands",
            "C020_COMMAND_CLICK_ON_SLOT_BOX_00_CHAMPION_0_STATUS_BOX_READY_HAND",
        ],
        "meaning": "action rows, action icons, champion names, and ready/action hands stay on the V1 mouse subroute tables",
    },
    {
        "file": "COMMAND.C",
        "start": 407,
        "end": 451,
        "needles": [
            "G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory",
            "C028_COMMAND_CLICK_ON_SLOT_BOX_08_INVENTORY_READY_HAND",
            "C070_COMMAND_CLICK_ON_MOUTH",
            "C071_COMMAND_CLICK_ON_EYE",
            "C081_COMMAND_CLICK_IN_PANEL",
        ],
        "meaning": "full inventory panel interactions are a separate V1 secondary table and must not be reimplemented by the V2 HUD overlay",
    },
    {
        "file": "CLIKCHAM.C",
        "start": 24,
        "end": 35,
        "needles": [
            "F0368_COMMAND_SetLeader",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0455_as_Graphic561_MouseInput_ChampionNamesHands",
            "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        ],
        "meaning": "champion status clicks dispatch through the V1 leader/slot-box transaction path",
    },
    {
        "file": "CLIKMENU.C",
        "start": 519,
        "end": 587,
        "needles": [
            "F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE",
            "F0391_MENUS_DidClickTriggerAction",
            "F0389_MENUS_ProcessCommands116To119_SetActingChampion",
        ],
        "meaning": "action-area rows/icons dispatch through V1 menu command handling",
    },
    {
        "file": "PANEL.C",
        "start": 1639,
        "end": 1693,
        "needles": [
            "F0347_INVENTORY_DrawPanel",
            "F0334_INVENTORY_CloseChest",
            "F0345_INVENTORY_DrawPanel_FoodWaterPoisoned",
            "F0342_INVENTORY_DrawPanel_Object",
        ],
        "meaning": "inventory panel draw state remains owned by V1 panel code",
    },
    {
        "file": "PANEL.C",
        "start": 1743,
        "end": 1824,
        "needles": [
            "F0349_INVENTORY_ProcessCommand70_ClickOnMouth",
            "G0415_ui_LeaderEmptyHanded",
            "G0333_B_PressingMouth",
            "AllowedSlots, MASK0x0001_MOUTH",
        ],
        "meaning": "mouth/use inventory transactions have source-owned state changes and are outside this V2 overlay gate",
    },
]

checks = {
    "src/dm1v2/dm1_v2_hud_interaction_pc34.c": [
        "COMMAND.C:375-395",
        "COMMAND.C:461-471",
        "COMMAND.C:484-497",
        "CLIKCHAM.C:24-35",
        "TOUCHCLICK_Compat_HitTestWithButton",
        "action_area_routes_GetTouchMatrixInvariant",
        "champion_name_hand_routes_GetInvariant",
    ],
    "include/dm1_v2_hud_interaction_pc34.h": [
        "M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34",
        "M11_V2_HUD_TOUCH_ACTION_ICON_PC34",
        "v2_hud_interaction_dispatch_scaled_click",
    ],
    "tests/test_dm1_v2_hud_interaction_pc34.c": [
        "champion0.toggle_box",
        "champion2.name",
        "champion3.action_hand",
        "action.icon1",
        "action.row1",
    ],
}

forbidden_transactions = [
    "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
    "F0349_INVENTORY_ProcessCommand70_ClickOnMouth",
    "F0350_INVENTORY_ProcessCommands28To74_ClickInPanel",
]

errors: list[str] = []
anchors = []
for check in source_checks:
    source_path = source_root / check["file"]
    if not source_path.exists():
        errors.append(f"missing ReDMCSB source {source_path}")
        continue
    lines = source_path.read_text(encoding="utf-8", errors="replace").splitlines()
    start = int(check["start"])
    end = int(check["end"])
    if start < 1 or end > len(lines) or start > end:
        errors.append(f"invalid line range {check['file']}:{start}-{end}")
        continue
    excerpt = "\n".join(lines[start - 1:end])
    for needle in check["needles"]:
        if needle not in excerpt:
            errors.append(f"{check['file']}:{start}-{end}: missing {needle!r}")
    anchors.append({
        "file": check["file"],
        "lineRange": f"{start}-{end}",
        "meaning": check["meaning"],
        "needles": check["needles"],
    })

for rel, needles in checks.items():
    path = root / rel
    if not path.exists():
        errors.append(f"missing Firestaff file {rel}")
        continue
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            errors.append(f"{rel}: missing {needle!r}")

hud_text = (root / "src/dm1v2/dm1_v2_hud_interaction_pc34.c").read_text(encoding="utf-8")
for forbidden in forbidden_transactions:
    if forbidden in hud_text:
        errors.append(f"V2 HUD interaction must not call V1 transaction owner directly: {forbidden}")

for rel in [
    "src/dm1/dm1_v1_click_routing_pc34_compat.c",
    "src/shared/touch_click_zone_matrix_pc34_compat.c",
    "src/shared/champion_name_hand_routes_pc34_compat.c",
    "src/shared/action_area_routes_pc34_compat.c",
]:
    if not (root / rel).exists():
        errors.append(f"missing source-lock dependency {rel}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "dm1_v2_hud_interaction champion/action overlay command mirror",
    "redmcsbSourceRoot": str(source_root),
    "anchors": anchors,
    "firestaffFiles": sorted(checks),
    "forbiddenDirectTransactions": forbidden_transactions,
    "errors": errors,
}
evidence_path.parent.mkdir(parents=True, exist_ok=True)
evidence_path.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_hud_interaction_source_lock=FAIL")
    for err in errors:
        print(err)
    sys.exit(1)

print(f"dm1_v2_hud_interaction_source_lock=OK evidence={evidence_path.relative_to(root)}")

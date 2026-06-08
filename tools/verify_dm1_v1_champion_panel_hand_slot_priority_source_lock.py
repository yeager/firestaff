#!/usr/bin/env python3
"""Verify the DM1 V1 champion-panel hand-slot priority source lock."""
from __future__ import annotations

import argparse
import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")
PASS = "dm1_v1_champion_panel_hand_slot_priority_source_lock"
STATUS = "DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LOCKED"
FAILED_STATUS = "FAILED_DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_LOCK"
MANIFEST = ROOT / "parity-evidence/verification" / PASS / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
TEST_BINARY = ROOT / "build/test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat"


def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


RED = first_existing("FIRESTAFF_REDMCSB_SOURCE", [
    DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
])


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def slice_text(path: Path, span: str) -> tuple[int, str]:
    first, last = (int(part) for part in span.split("-", 1))
    lines = read_text(path).splitlines()
    return first, "\n".join(lines[first - 1:last])


def ordered_hits(body: str, base: int, needles: list[str]) -> tuple[list[dict[str, Any]], list[str]]:
    cursor = 0
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"line": base + body.count("\n", 0, pos), "needle": needle})
        cursor = pos + len(needle)
    return hits, missing


def check_region(ident: str, path: Path, span: str, claim: str, needles: list[str]) -> dict[str, Any]:
    base, body = slice_text(path, span)
    hits, missing = ordered_hits(body, base, needles)
    return {
        "id": ident,
        "file": str(path),
        "source": f"{path.name}:{span}",
        "claim": claim,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def check_file(ident: str, rel: str, needles: list[str]) -> dict[str, Any]:
    path = ROOT / rel
    body = read_text(path)
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": 1 + body.count("\n", 0, pos), "needle": needle})
    return {
        "id": ident,
        "file": rel,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run(cmd: list[str]) -> dict[str, Any]:
    if not Path(cmd[0]).exists():
        return {
            "command": cmd,
            "returncode": 127,
            "passed": False,
            "outputTail": "missing test binary; build the target first",
        }
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=180,
    )
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-18:]),
    }


REDMCSB_CHECKS = [
    (
        "champion_f0302_status_hand_route",
        RED / "CHAMPION.C",
        "677-684",
        "F0302 routes slot-box indices 0..7 through candidate/open/dead guards and M070 hand-slot mapping before inventory slots.",
        [
            "if (P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT) {",
            "if (G0299_ui_CandidateChampionOrdinal)",
            "L0903_ui_ChampionIndex = P0633_ui_SlotBoxIndex >> 1;",
            "G0305_ui_PartyChampionCount",
            "G0423_i_InventoryChampionOrdinal",
            "M516_CHAMPIONS[L0903_ui_ChampionIndex].CurrentHealth",
            "L0904_ui_SlotIndex = M070_HAND_SLOT_INDEX(P0633_ui_SlotBoxIndex);",
        ],
    ),
    (
        "champion_f0297_f0298_leader_hand",
        RED / "CHAMPION.C",
        "243-298",
        "F0297/F0298 own leader-hand object, pointer/name, load byte, and F0292 redraw side effects.",
        [
            "void F0297_CHAMPION_PutObjectInLeaderHand",
            "G0415_ui_LeaderEmptyHanded = C0_FALSE;",
            "F0034_OBJECT_DrawLeaderHandObjectName(P0621_T_Thing);",
            "M516_CHAMPIONS[G0411_i_LeaderIndex].Load += F0140_DUNGEON_GetObjectWeight(P0621_T_Thing);",
            "F0292_CHAMPION_DrawState(G0411_i_LeaderIndex);",
            "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
            "G0415_ui_LeaderEmptyHanded = C1_TRUE;",
            "M516_CHAMPIONS[G0411_i_LeaderIndex].Load -= F0140_DUNGEON_GetObjectWeight(L0890_T_LeaderHandObject);",
            "return L0890_T_LeaderHandObject;",
        ],
    ),
    (
        "champion_f0300_c30_clear",
        RED / "CHAMPION.C",
        "511-515",
        "F0300 clears C30+ slots through G0425_aT_ChestSlots.",
        [
            "L0896_ps_Champion = &M516_CHAMPIONS[P0628_ui_ChampionIndex];",
            "if (P0629_ui_SlotIndex >= C30_SLOT_CHEST_1) {",
            "L0894_T_Thing = G0425_aT_ChestSlots[P0629_ui_SlotIndex - C30_SLOT_CHEST_1];",
            "G0425_aT_ChestSlots[P0629_ui_SlotIndex - C30_SLOT_CHEST_1] = C0xFFFF_THING_NONE;",
        ],
    ),
    (
        "champion_f0301_c30_write",
        RED / "CHAMPION.C",
        "606-614",
        "F0301 writes C30+ slots through G0425_aT_ChestSlots and then updates the champion load.",
        [
            "if (P0631_T_Thing == C0xFFFF_THING_NONE)",
            "L0900_ps_Champion = &M516_CHAMPIONS[P0630_ui_ChampionIndex];",
            "if (P0632_ui_SlotIndex >= C30_SLOT_CHEST_1) {",
            "G0425_aT_ChestSlots[P0632_ui_SlotIndex - C30_SLOT_CHEST_1] = P0631_T_Thing;",
            "L0900_ps_Champion->Load += F0140_DUNGEON_GetObjectWeight(P0631_T_Thing);",
        ],
    ),
    (
        "champion_f0302_transaction_order",
        RED / "CHAMPION.C",
        "688-712",
        "F0302 snapshots leader hand, reads storage, rejects empty/mask cases, then executes F0077/F0298/F0300/F0297/F0301/F0292/F0078 order.",
        [
            "L0905_T_LeaderHandObject = G4055_s_LeaderHandObject.Thing;",
            "L0906_T_SlotThing = G0425_aT_ChestSlots[L0904_ui_SlotIndex - C30_SLOT_CHEST_1];",
            "L0906_T_SlotThing = M516_CHAMPIONS[L0903_ui_ChampionIndex].Slots[L0904_ui_SlotIndex];",
            "AllowedSlots & G0038_ai_Graphic562_SlotMasks[L0904_ui_SlotIndex]",
            "F0077_MOUSE_EnableScreenUpdate_CPSE();",
            "F0298_CHAMPION_GetObjectRemovedFromLeaderHand();",
            "F0300_CHAMPION_GetObjectRemovedFromSlot(L0903_ui_ChampionIndex, L0904_ui_SlotIndex);",
            "F0297_CHAMPION_PutObjectInLeaderHand(L0906_T_SlotThing, C0_FALSE);",
            "F0301_CHAMPION_AddObjectInSlot(L0903_ui_ChampionIndex, L0905_T_LeaderHandObject, L0904_ui_SlotIndex);",
            "F0292_CHAMPION_DrawState(L0903_ui_ChampionIndex);",
            "F0078_MOUSE_DisableScreenUpdate();",
        ],
    ),
    (
        "defs_hand_slot_symbols",
        RED / "DEFS.H",
        "780-817",
        "DEFS.H binds hand/body/backpack/chest slot constants including C30.",
        [
            "#define C00_SLOT_READY_HAND",
            "#define C01_SLOT_ACTION_HAND",
            "#define C13_SLOT_BACKPACK_LINE1_1",
            "#define C29_SLOT_BACKPACK_LINE1_9",
            "#define C30_SLOT_CHEST_1",
            "#define C37_SLOT_CHEST_8",
        ],
    ),
    (
        "defs_m516_party_storage",
        RED / "DEFS.H",
        "873-876",
        "DEFS.H maps M516_CHAMPIONS to the party champion array.",
        [
            "#define M516_CHAMPIONS",
        ],
    ),
    (
        "defs_slotbox_and_m070",
        RED / "DEFS.H",
        "1874-1878",
        "DEFS.H binds the status/inventory slot-box boundary and M070 hand-slot macro.",
        [
            "#define C08_SLOT_BOX_INVENTORY_FIRST_SLOT   8",
            "#define C38_SLOT_BOX_CHEST_FIRST_SLOT      38",
            "#define M070_HAND_SLOT_INDEX(slotboxindex) ((slotboxindex) & 0x0001)",
        ],
    ),
    (
        "defs_slot_masks",
        RED / "DEFS.H",
        "5324-5332",
        "DEFS.H binds slot-box metadata and the slot masks used by F0302 AllowedSlots.",
        [
            "extern SLOT_BOX G0030_as_Graphic562_SlotBoxes[46];",
            "extern int16_t G0038_ai_Graphic562_SlotMasks[38];",
        ],
    ),
    (
        "defs_globals",
        RED / "DEFS.H",
        "5700-5881",
        "DEFS.H exposes party count, inventory champion ordinal, chest slots, and open chest.",
        [
            "extern unsigned int16_t G0305_ui_PartyChampionCount;",
            "extern int16_t G0423_i_InventoryChampionOrdinal;",
            "extern THING G0425_aT_ChestSlots[8];",
            "extern THING G0426_T_OpenChest;",
        ],
    ),
    (
        "chamdraw_f0291_slot_redraw",
        RED / "CHAMDRAW.C",
        "498-559",
        "F0291 redraws a requested champion slot and reads C30+ inventory from G0425_aT_ChestSlots.",
        [
            "void F0291_CHAMPION_DrawSlot",
            "L0854_ps_Champion = &M516_CHAMPIONS[P0613_ui_ChampionIndex];",
            "L0852_B_IsInventoryChampion = (G0423_i_InventoryChampionOrdinal == M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex));",
            "L0851_T_Thing = G0425_aT_ChestSlots[P0614_ui_SlotIndex - C30_SLOT_CHEST_1];",
        ],
    ),
    (
        "chamdraw_f0292_redraw_contract",
        RED / "CHAMDRAW.C",
        "703-711",
        "F0292 is the champion state redraw entry called by F0297/F0302.",
        [
            "void F0292_CHAMPION_DrawState",
            "REGISTER CHAMPION* L0865_ps_Champion;",
        ],
    ),
    (
        "chamdraw_f0292_action_hand",
        RED / "CHAMDRAW.C",
        "1060-1088",
        "F0292 redraws panel content and action hand when the relevant attributes are set.",
        [
            "if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0800_PANEL) && L0863_B_IsInventoryChampion) {",
            "F0345_INVENTORY_DrawPanel_FoodWaterPoisoned();",
            "F0347_INVENTORY_DrawPanel();",
            "if (M007_GET(L0862_ui_ChampionAttributes, MASK0x8000_ACTION_HAND)) {",
            "F0291_CHAMPION_DrawSlot(P0615_ui_ChampionIndex, C01_SLOT_ACTION_HAND);",
            "F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex);",
        ],
    ),
    (
        "panel_f0344_f0345_dependency",
        RED / "PANEL.C",
        "1493-1616",
        "PANEL.C F0344/F0345 are the food/water panel redraw dependency reached by the action-hand panel path.",
        [
            "STATICFUNCTION void F0344_INVENTORY_DrawPanel_FoodOrWaterBar",
            "if (P0712_i_Amount < -512) {",
            "void F0345_INVENTORY_DrawPanel_FoodWaterPoisoned",
            "G0424_i_PanelContent = M565_PANEL_FOOD_WATER_POISONED;",
            "L1074_ps_Champion = &M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0423_i_InventoryChampionOrdinal)];",
            "F0344_INVENTORY_DrawPanel_FoodOrWaterBar(L1074_ps_Champion->Food",
            "F0344_INVENTORY_DrawPanel_FoodOrWaterBar(L1074_ps_Champion->Water",
        ],
    ),
    (
        "dunview_non_claim_viewport_order",
        RED / "DUNVIEW.C",
        "8337-8349",
        "DUNVIEW viewport redraw/clickable setup exists as a non-claim fence for this HUD gate.",
        [
            "if (G0297_B_DrawFloorAndCeilingRequested) {",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0008_MAIN_ClearBytes(G0291_aauc_DungeonViewClickableBoxes",
            "F0008_MAIN_ClearBytes(M772_CAST_PC(G2210_aai_XYZ_DungeonViewClickable)",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "firestaff_header_contract",
        "src/dm1/dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h",
        [
            "DM1 V1 champion panel hand-slot priority source-lock contract",
            "CHAMPION.C F0302:677-684",
            "CHAMPION.C F0297:243-298",
            "CHAMPION.C F0300:511-515",
            "CHAMPION.C F0301:606-614",
            "DEFS.H anchors C30_SLOT_CHEST_1, G0425_aT_ChestSlots",
            "CHAMDRAW.C F0291/F0292",
            "PANEL.C F0344/F0345",
            "DM1_V1_CHAMPION_PANEL_HAND_SLOT_PRIORITY_SOURCE_STATUS_HAND_PC34",
            "dm1_v1_champion_panel_hand_slot_priority_resolve_pc34",
        ],
    ),
    (
        "firestaff_module_source_lock",
        "src/dm1/dm1_v1_champion_panel_hand_slot_priority_pc34_compat.c",
        [
            "status hand -> leader hand -> backpack -> belt",
            "CHAMPION.C F0302:677-684 resolves status hand slot boxes",
            "CHAMPION.C F0297:243-298 and F0298",
            "CHAMPION.C F0300:511-515 clears C30+ through G0425_aT_ChestSlots",
            "F0301:606-614 writes C30+ through G0425_aT_ChestSlots",
            "M516_CHAMPIONS[].Load",
            "CHAMDRAW.C F0291/F0292 provide the redraw contract",
            "PANEL.C F0344/F0345 are only recorded as the panel redraw dependency",
            "DUNVIEW.C is a non-claim anchor",
            "input->slot_box_index <",
            "input->slot_box_index >> 1",
            "input->slot_box_index & 1u",
        ],
    ),
    (
        "firestaff_test_status_chain",
        "tests/test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat.c",
        [
            "test_status_hand_slotbox_0_to_7_routes",
            "slot%u.status_route",
            "slot%u.champion",
            "slot%u.hand",
            "slot%u.family",
            "slot%u.accepted",
            "CHAMPION.C F0302:677-684 slot-order chain",
            "Assertions:",
        ],
    ),
    (
        "cmake_registration",
        "CMakeLists.txt",
        [
            "test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat",
            "src/dm1/dm1_v1_champion_panel_hand_slot_priority_pc34_compat.c",
            "dm1_v1_champion_panel_hand_slot_priority_source_lock",
        ],
    ),
]


def collect_checks() -> list[dict[str, Any]]:
    rows = [check_region(*item) for item in REDMCSB_CHECKS]
    rows.extend(check_file(*item) for item in LOCAL_CHECKS)
    return rows


def write_outputs(checks: list[dict[str, Any]], runtime: dict[str, Any], problems: list[str]) -> None:
    ok = not problems
    manifest: dict[str, Any] = {
        "schema": "firestaff.parity.dm1_v1_champion_panel_hand_slot_priority_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else FAILED_STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "claim": "DM1 V1 champion-panel/HUD hand-slot priority is source-locked: status hand -> leader hand -> backpack -> belt, with F0302 status hand routing and C30/G0425 chest storage evidence.",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": [row for row in checks if str(row["file"]).startswith(str(RED))],
        "firestaffChecks": [row for row in checks if not str(row["file"]).startswith(str(RED))],
        "verificationRuns": [runtime],
        "nonClaims": [
            "No real GRAPHICS.DAT, DUNGEON.DAT, or savegame load.",
            "No bitmap or original DOS pixel parity claim.",
            "No change to DUNVIEW viewport presentation order.",
            "No expansion of chest contents, backpack storage, or object type semantics beyond this contract.",
        ],
        "problems": problems,
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# DM1 V1 champion-panel hand-slot priority source lock",
        "",
        f"Status: {manifest['statusToken']}",
        "",
        str(manifest["claim"]),
        "",
        "## Primary Evidence",
    ]
    for row in manifest["redmcsbChecks"]:
        lines.append(f"- {row['status']} {row['source']}: {row['claim']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Local Gates"]
    for row in manifest["firestaffChecks"]:
        lines.append(f"- {row['status']} {row['file']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += [
        "",
        "## Verification",
        f"- {' '.join(runtime['command'])}: rc={runtime['returncode']}",
    ]
    if runtime["outputTail"]:
        lines.extend(["~~~", str(runtime["outputTail"]), "~~~"])
    lines += ["", "## Non-Claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true")
    args = parser.parse_args()

    checks = collect_checks()
    runtime = run([str(TEST_BINARY)])
    problems = [str(row["id"]) for row in checks if row["status"] != "PASS"]
    if not runtime["passed"]:
        problems.append("test_dm1_v1_champion_panel_hand_slot_priority_pc34_compat")

    if args.check_only:
        if problems:
            print("FAIL dm1_v1_champion_panel_hand_slot_priority check-only: " + ",".join(problems))
            return 1
        print("PASS dm1_v1_champion_panel_hand_slot_priority check-only")
        return 0

    write_outputs(checks, runtime, problems)
    print(STATUS if not problems else FAILED_STATUS)
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    if problems:
        print("- problems", ", ".join(problems))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())

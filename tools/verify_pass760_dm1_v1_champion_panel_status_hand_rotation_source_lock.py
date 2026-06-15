#!/usr/bin/env python3
"""Verify pass760 DM1 V1 champion-panel status-hand rotation source lock."""
from __future__ import annotations

import argparse
import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

PASS = "pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock"
STATUS = "PASS760_DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_SOURCE_LOCKED"
FAILED_STATUS = "FAILED_PASS760_DM1_V1_CHAMPION_PANEL_STATUS_HAND_ROTATION_SOURCE_LOCK"

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR", ROOT / "build"))
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")
MANIFEST = ROOT / f"parity-evidence/verification/{PASS}/manifest.json"
REPORT = ROOT / f"parity-evidence/{PASS}.md"
TEST_BINARY = BUILD / "test_dm1_v1_champion_panel_pc34_compat"


def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


RED = first_existing(
    "FIRESTAFF_REDMCSB_SOURCE",
    [
        DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    ],
)


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


def check_region(
    ident: str,
    path: Path,
    span: str,
    claim: str,
    needles: list[str],
) -> dict[str, Any]:
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


def run_test_binary() -> dict[str, Any]:
    cmd = [str(TEST_BINARY)]
    if not TEST_BINARY.exists():
        return {
            "command": cmd,
            "returncode": 127,
            "passed": False,
            "assertions": 0,
            "failures": 1,
            "outputTail": "missing test binary; build test_dm1_v1_champion_panel_pc34_compat first",
        }
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=180,
    )
    assertions = 0
    failures = 1
    for line in proc.stdout.splitlines():
        if line.startswith("Assertions:"):
            assertions = int(line.split(":", 1)[1].strip())
        if line.startswith("Failures:"):
            failures = int(line.split(":", 1)[1].strip())
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0 and assertions >= 80 and failures == 0,
        "assertions": assertions,
        "failures": failures,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-18:]),
    }


REDMCSB_CHECKS = [
    (
        "champion_f0302_status_hand_route",
        RED / "CHAMPION.C",
        "677-684",
        "F0302 routes C00..C07 status-hand slot boxes before inventory, mapping champion and hand slot from the slot-box index.",
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
        "champion_f0297_leader_hand_put",
        RED / "CHAMPION.C",
        "243-298",
        "F0297/F0298 own leader-hand put/remove state, load, pointer, and redraw side effects.",
        [
            "void F0297_CHAMPION_PutObjectInLeaderHand",
            "G0415_ui_LeaderEmptyHanded = C0_FALSE;",
            "F0034_OBJECT_DrawLeaderHandObjectName(P0621_T_Thing);",
            "M516_CHAMPIONS[G0411_i_LeaderIndex].Load += F0140_DUNGEON_GetObjectWeight(P0621_T_Thing);",
            "F0292_CHAMPION_DrawState(G0411_i_LeaderIndex);",
            "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
        ],
    ),
    (
        "champion_f0298_leader_hand_remove",
        RED / "CHAMPION.C",
        "270-298",
        "F0298 removes the leader-hand object and redraws/load-adjusts the leader before the slot write path continues.",
        [
            "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
            "G0415_ui_LeaderEmptyHanded = C1_TRUE;",
            "G4055_s_LeaderHandObject.Thing = C0xFFFF_THING_NONE;",
            "F0035_OBJECT_ClearLeaderHandObjectName();",
            "M516_CHAMPIONS[G0411_i_LeaderIndex].Load -= F0140_DUNGEON_GetObjectWeight(L0890_T_LeaderHandObject);",
            "F0292_CHAMPION_DrawState(G0411_i_LeaderIndex);",
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
        "F0301 writes C30+ slots through G0425_aT_ChestSlots and updates load.",
        [
            "if (P0631_T_Thing == C0xFFFF_THING_NONE)",
            "L0900_ps_Champion = &M516_CHAMPIONS[P0630_ui_ChampionIndex];",
            "if (P0632_ui_SlotIndex >= C30_SLOT_CHEST_1) {",
            "G0425_aT_ChestSlots[P0632_ui_SlotIndex - C30_SLOT_CHEST_1] = P0631_T_Thing;",
            "L0900_ps_Champion->Load += F0140_DUNGEON_GetObjectWeight(P0631_T_Thing);",
        ],
    ),
    (
        "champion_f0302_occupied_slot_swap",
        RED / "CHAMPION.C",
        "688-712",
        "F0302 snapshots leader hand, reads storage, applies mask/empty guards, and performs the occupied-slot swap helper order.",
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
        "chamdraw_f0291_action_hand_slot_box_rotation",
        RED / "CHAMDRAW.C",
        "621-630",
        "F0291 resolves the status-panel action-hand icon before the status-hand border rotation.",
        [
            "L0853_i_IconIndex = F0033_OBJECT_GetIconIndex(L0851_T_Thing);",
            "if (L0852_B_IsInventoryChampion && (P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND)",
            "L0853_i_IconIndex++;",
        ],
    ),
    (
        "chamdraw_f0291_c033_c034_c035_selection",
        RED / "CHAMDRAW.C",
        "634-642",
        "F0291 selects C035 for the acting champion's action hand, otherwise C034 wounded or C033 normal.",
        [
            "if ((P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND) && (M000_INDEX_TO_ORDINAL(P0613_ui_ChampionIndex) == G0506_ui_ActingChampionOrdinal)) {",
            "L0859_i_NativeBitmapIndex = C035_GRAPHIC_SLOT_BOX_ACTING_HAND;",
            "if (M007_GET(L0854_ps_Champion->Wounds, 1 << P0614_ui_SlotIndex)) {",
            "L0859_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;",
            "L0859_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;",
        ],
    ),
    (
        "chamdraw_f0292_redraw_tuple",
        RED / "CHAMDRAW.C",
        "898-935",
        "F0292 redraw tuple is recorded as context only; this pass does not duplicate the mouth/eye or food/water gates.",
        [
            "if (M007_GET(L0862_ui_ChampionAttributes, MASK0x0100_STATISTICS)) {",
            "F0287_CHAMPION_DrawBarGraphs(P0615_ui_ChampionIndex);",
            "F0290_CHAMPION_DrawHealthStaminaManaValues(L0865_ps_Champion);",
            "AL0870_i_NativeBitmapIndex = C034_GRAPHIC_SLOT_BOX_WOUNDED;",
            "AL0870_i_NativeBitmapIndex = C033_GRAPHIC_SLOT_BOX_NORMAL;",
            "M008_SET(L0862_ui_ChampionAttributes, MASK0x4000_VIEWPORT);",
        ],
    ),
    (
        "defs_slots_requested_anchor",
        RED / "DEFS.H",
        "780-817",
        "DEFS.H:780-817 anchors hand, belt/quick, backpack, and C30..C37 slots.",
        [
            "#define C00_SLOT_READY_HAND",
            "#define C01_SLOT_ACTION_HAND",
            "#define C06_SLOT_POUCH_2",
            "#define C12_SLOT_QUIVER_LINE1_1",
            "#define C13_SLOT_BACKPACK_LINE1_1",
            "#define C29_SLOT_BACKPACK_LINE1_9",
            "#define C30_SLOT_CHEST_1",
            "#define C37_SLOT_CHEST_8",
        ],
    ),
    (
        "defs_m516_actual_anchor",
        RED / "DEFS.H",
        "873-876",
        "M516_CHAMPIONS lives outside the requested 780-817 slice in this ReDMCSB snapshot.",
        ["#define M516_CHAMPIONS"],
    ),
    (
        "defs_m070_actual_anchor",
        RED / "DEFS.H",
        "1874-1878",
        "M070_HAND_SLOT_INDEX and the C08 status/inventory boundary live outside the requested 780-817 slice.",
        [
            "#define C08_SLOT_BOX_INVENTORY_FIRST_SLOT   8",
            "#define M070_HAND_SLOT_INDEX(slotboxindex) ((slotboxindex) & 0x0001)",
        ],
    ),
    (
        "defs_flesh_transparency",
        RED / "DEFS.H",
        "2088-2088",
        "DEFS.H:2088 anchors C10_COLOR_FLESH transparency/color lineage.",
        ["#define C10_COLOR_FLESH           10"],
    ),
    (
        "defs_g0305_g0423_g0425_g0426_actual_anchor",
        RED / "DEFS.H",
        "5700-5881",
        "G0305/G0423/G0425/G0426 live outside the requested 780-817 slice in this ReDMCSB snapshot.",
        [
            "extern unsigned int16_t G0305_ui_PartyChampionCount;",
            "extern int16_t G0423_i_InventoryChampionOrdinal;",
            "extern THING G0425_aT_ChestSlots[8];",
            "extern THING G0426_T_OpenChest;",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "firestaff_header_contract",
        "include/dm1_v1_champion_panel_pc34_compat.h",
        [
            "DM1 V1 champion-panel status-hand rotation source-lock contract",
            "CHAMPION.C F0302:677-684",
            "CHAMPION.C F0297:243-298",
            "CHAMPION.C F0298:270-298",
            "CHAMPION.C F0300:511-515",
            "CHAMPION.C F0301:606-614",
            "CHAMPION.C F0302:688-712",
            "CHAMDRAW.C F0291:621-630",
            "CHAMDRAW.C F0292:898-935",
            "DEFS.H:780-817",
            "DEFS.H:2088",
            "DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34",
            "DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34",
            "DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34",
            "dm1_v1_champion_panel_status_hand_rotation_plan_pc34",
        ],
    ),
    (
        "firestaff_module_source_lock",
        "src/dm1/dm1_v1_champion_panel_pc34_compat.c",
        [
            "pass760 contract-only status-hand rotation gate",
            "CHAMPION.C F0302:677-684",
            "F0291:634-642 selects C035 acting, C034 wounded, or C033 normal",
            "CHAMDRAW.C F0292:898-935 is recorded only",
            "DEFS.H:780-817",
            "DEFS.H:2088",
            "does not duplicate pass673 mouth/eye or pass683 food/water gates",
            "DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_STATUS_HAND_PC34",
            "DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_LEADER_HAND_PC34",
            "DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BACKPACK_PC34",
            "DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BELT_PC34",
            "box->slot_box_index = (uint16_t)((champion << 1) | slot);",
            "box->native_bitmap_index =",
        ],
    ),
    (
        "firestaff_test_rotation_matrix",
        "tests/test_dm1_v1_champion_panel_pc34_compat.c",
        [
            "pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock",
            "for (party_count = 1; party_count <= 4; ++party_count)",
            "for (acting_index = 0; acting_index < 4; ++acting_index)",
            "DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34",
            "DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34",
            "DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34",
            "Assertions:",
            "want>=80",
        ],
    ),
    (
        "cmake_registration",
        "CMakeLists.txt",
        [
            "test_dm1_v1_champion_panel_pc34_compat",
            "tests/test_dm1_v1_champion_panel_pc34_compat.c",
            "src/dm1/dm1_v1_champion_panel_pc34_compat.c",
            "pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock",
            "verify_pass760_dm1_v1_champion_panel_status_hand_rotation_source_lock",
        ],
    ),
]


def collect_checks() -> list[dict[str, Any]]:
    rows = [check_region(*item) for item in REDMCSB_CHECKS]
    rows.extend(check_file(*item) for item in LOCAL_CHECKS)
    return rows


def write_outputs(
    checks: list[dict[str, Any]],
    runtime: dict[str, Any],
    problems: list[str],
) -> None:
    ok = not problems
    manifest: dict[str, Any] = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else FAILED_STATUS,
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "claim": "DM1 V1 champion-panel status-hand C033/C034/C035 box rotation is source-locked across acting-champion changes and preserves the status-hand -> leader-hand -> backpack -> belt priority chain.",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": [row for row in checks if str(row["file"]).startswith(str(RED))],
        "firestaffChecks": [row for row in checks if not str(row["file"]).startswith(str(RED))],
        "verificationRuns": [runtime],
        "anchorNotes": [
            "Requested DEFS.H:780-817 is preserved as the C00/C01/backpack/belt/C30 slot anchor.",
            "G0425/G0426/G0423/G0305/M070/M516 are not located in DEFS.H:780-817 in this ReDMCSB snapshot; this verifier records their exact actual lines as supplemental anchors.",
        ],
        "nonClaims": [
            "No real GRAPHICS.DAT, DUNGEON.DAT, savegame, or bitmap load.",
            "No original-vs-Firestaff pixel parity claim.",
            "No duplicate pass673 mouth/eye status-box gate.",
            "No duplicate pass683 food/water status-box gate.",
        ],
        "problems": problems,
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# pass760 DM1 V1 champion-panel status-hand rotation source lock",
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
        f"- {' '.join(runtime['command'])}: rc={runtime['returncode']}; assertions={runtime['assertions']}; failures={runtime['failures']}",
    ]
    if runtime["outputTail"]:
        lines.extend(["~~~", str(runtime["outputTail"]), "~~~"])
    lines += ["", "## Anchor Notes"]
    lines.extend(f"- {item}" for item in manifest["anchorNotes"])
    lines += ["", "## Non-Claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true")
    args = parser.parse_args()

    checks = collect_checks()
    runtime = run_test_binary()
    problems = [str(row["id"]) for row in checks if row["status"] != "PASS"]
    if not runtime["passed"]:
        problems.append("test_dm1_v1_champion_panel_pc34_compat")

    if args.check_only:
        if problems:
            print("FAIL pass760 check-only: " + ",".join(problems))
            return 1
        print("PASS pass760 check-only")
        return 0

    write_outputs(checks, runtime, problems)
    print(STATUS if not problems else FAILED_STATUS)
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    print(f"- assertions={runtime['assertions']} failures={runtime['failures']}")
    if problems:
        print("- problems", ", ".join(problems))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())

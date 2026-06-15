#!/usr/bin/env python3
"""Pass653: DM1 V1 mirror candidate key-rotation plus inventory-click race."""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass653_dm1_v1_mirror_candidate_keyrot_combo_invclick_regression"
STATUS = "PASS653_DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REGRESSION_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.c"
HDR = ROOT / "src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.c"
CMK = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def read(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} and path.is_relative_to(RED) else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def check_needles(label: str, path: Path, needles: list[str]) -> dict[str, object]:
    text = read(path)
    hits = []
    missing = []
    for needle in needles:
        pos = text.find(needle)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": text.count("\n", 0, pos) + 1, "needle": needle})
    return {
        "id": label,
        "file": str(path.relative_to(ROOT) if path.is_relative_to(ROOT) else path),
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run(cmd: list[str]) -> dict[str, object]:
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
        "command_f0359_m568_panel_dispatch",
        RED / "COMMAND.C",
        [
            "case M568_PANEL_RESURRECT_REINCARNATE:",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel",
            "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel(L1157_ui_Command);",
        ],
    ),
    (
        "command_f0361_keyboard_queue_write",
        RED / "COMMAND.C",
        [
            "void F0361_COMMAND_ProcessKeyPress(",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
        ],
    ),
    (
        "command_f0380_queue_dispatch",
        RED / "COMMAND.C",
        [
            "void F0380_COMMAND_ProcessQueue_CPSC(",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        ],
    ),
    (
        "revive_candidate_pending_and_clear",
        RED / "REVIVE.C",
        [
            "void F0280_CHAMPION_AddCandidateChampionToParty(",
            "if (!G0415_ui_LeaderEmptyHanded)",
            "if (G0305_ui_PartyChampionCount >= 4)",
            "void F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel(",
            "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);",
        ],
    ),
    (
        "champion_slot_and_leader_hand",
        RED / "CHAMPION.C",
        [
            "void F0297_CHAMPION_PutObjectInLeaderHand(",
            "G0415_ui_LeaderEmptyHanded = C0_FALSE;",
            "void F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(",
            "if (P0633_ui_SlotBoxIndex < C08_SLOT_BOX_INVENTORY_FIRST_SLOT)",
            "F0297_CHAMPION_PutObjectInLeaderHand(L0906_T_SlotThing, C0_FALSE);",
        ],
    ),
    (
        "chamdraw_redraw_tuple",
        RED / "CHAMDRAW.C",
        [
            "void F0291_CHAMPION_DrawSlot(",
            "if (L0852_B_IsInventoryChampion && (P0614_ui_SlotIndex == C01_SLOT_ACTION_HAND)",
            "void F0292_CHAMPION_DrawState(",
            "void F0293_CHAMPION_DrawAllChampionStates(",
            "F0292_CHAMPION_DrawState(L0873_ui_ChampionIndex);",
        ],
    ),
    (
        "defs_required_chain",
        RED / "DEFS.H",
        [
            "#define C10_COLOR_FLESH",
            "#define C30_SLOT_CHEST_1",
            "#define M070_HAND_SLOT_INDEX(slotboxindex)",
            "#define C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1",
            "extern unsigned int16_t G0305_ui_PartyChampionCount;",
            "extern int16_t G0423_i_InventoryChampionOrdinal;",
            "extern THING G0425_aT_ChestSlots[8];",
            "extern THING G0426_T_OpenChest;",
            "#define M516_CHAMPIONS",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "module_runtime_helper",
        SRC,
        [
            "dm1_v1_mirror_candidate_keyrot_combo_invclick_run_pc34_compat",
            "simulate_f0361_key_queue_write",
            "process_f0380_turn_with_optional_race",
            "defer_pending_click_during_f0380",
            "redrawByteIdenticalToNoClick",
            "F0380_COMMAND_ProcessQueue_CPSC line ~2045-2156",
        ],
    ),
    (
        "header_public_contract",
        HDR,
        [
            "Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat",
            "clickDidNotClearCandidate",
            "rotationProcessedNormally",
            "redrawByteIdenticalToNoClick",
        ],
    ),
    (
        "c_test_runtime_assertions",
        TEST,
        [
            "candidate remains pending and inventory click does not clear it",
            "TURN_* key was written through the F0361 queue path",
            "F0293 redraw tuple is byte-identical to a no-click rotation",
            "panel cancel remains the modeled F0282 candidate-clear path",
        ],
    ),
    (
        "cmake_registration",
        CMK,
        [
            "test_dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat",
            "src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.c",
            "NAME dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat",
        ],
    ),
]


def collect_checks() -> list[dict[str, object]]:
    checks = []
    for label, path, needles in REDMCSB_CHECKS:
        checks.append(check_needles(label, path, needles))
    for label, path, needles in LOCAL_CHECKS:
        checks.append(check_needles(label, path, needles))
    return checks


def write_outputs(checks: list[dict[str, object]], runs: list[dict[str, object]]) -> None:
    ok = all(row["status"] == "PASS" for row in checks) and all(run["passed"] for run in runs)
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else "FAILED_PASS653_DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REGRESSION",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": "DM1 V1 mirror candidate pending, F0361 TURN_* key queue write, F0380 in-flight pending inventory click race, and byte-identical F0293 redraw against no-click rotation.",
        "redmcsbRoot": str(RED),
        "sourceChecks": checks,
        "verificationRuns": runs,
        "nonClaims": [
            "No new original DOSBox capture.",
            "No SDL input capture or framebuffer parity claim.",
            "No behavior beyond this contract-only key-dispatch inventory-click race.",
            "No changes to existing mirror-candidate modules.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass653 - DM1 V1 mirror candidate key-rotation inventory-click regression",
        "",
        f"Status: {manifest['status']}",
        "",
        manifest["scope"],
        "",
        "## Source Checks",
    ]
    for row in checks:
        lines.append(f"- {row['status']} {row['id']} ({row['file']})")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Verification"]
    for run_row in runs:
        lines.append(f"- {' '.join(run_row['command'])}: rc={run_row['returncode']}")
        if run_row["outputTail"]:
            lines.extend(["~~~", str(run_row["outputTail"]), "~~~"])
    lines += ["", "## Non-Claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true")
    args = parser.parse_args()

    checks = collect_checks()
    failures = [row["id"] for row in checks if row["status"] != "PASS"]
    if args.check_only:
        if failures:
            print("FAIL pass653 check-only: " + ",".join(str(f) for f in failures))
            return 1
        print("PASS pass653 check-only")
        return 0

    runs = [
        run([str(resolve_build_dir(ROOT, ROOT / "build") / "test_dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat")]),
        run([sys.executable, str(Path(__file__).resolve()), "--check-only"]),
    ]
    write_outputs(checks, runs)
    ok = not failures and all(row["passed"] for row in runs)
    print(STATUS if ok else "FAILED_PASS653_DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REGRESSION")
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

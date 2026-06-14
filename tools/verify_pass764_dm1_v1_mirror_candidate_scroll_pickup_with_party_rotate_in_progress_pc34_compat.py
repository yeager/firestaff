#!/usr/bin/env python3
"""Verify pass764 DM1 V1 mirror candidate scroll-pickup party-rotate gate."""
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
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
PASS = "pass764_dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat"
TARGET = "test_dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat"
TEST_RE = "dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat"
HDR = (
    ROOT
    / "include/firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat.h"
)
SRC = (
    ROOT
    / "src/dm1/dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat.c"
)
TEST = ROOT / f"tests/{TARGET}.c"
REPORT = ROOT / f"parity-evidence/{PASS}.md"
CMK = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"


def read(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} and RED in path.parents else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def check_needles(label: str, path: Path, needles: list[str]) -> dict[str, object]:
    text = read(path)
    hits: list[dict[str, object]] = []
    missing: list[str] = []
    for needle in needles:
        pos = text.find(needle)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": text.count("\n", 0, pos) + 1, "needle": needle})
    return {
        "id": label,
        "file": str(path.relative_to(ROOT) if ROOT in path.parents else path),
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run(cmd: list[str], timeout: int = 240) -> dict[str, object]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=timeout,
    )
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-24:]),
    }


REDMCSB_CHECKS = [
    (
        "panel_f0344_f0345",
        RED / "PANEL.C",
        [
            "F0344_INVENTORY_DrawPanel_FoodOrWaterBar",
            "F0345_INVENTORY_DrawPanel_FoodWaterPoisoned",
        ],
    ),
    (
        "champion_f0297_f0298_f0302",
        RED / "CHAMPION.C",
        [
            "void F0297_CHAMPION_PutObjectInLeaderHand",
            "THING F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
            "void F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
            "C30_SLOT_CHEST_1",
        ],
    ),
    (
        "command_f0359_f0361_f0380",
        RED / "COMMAND.C",
        [
            "case M568_PANEL_RESURRECT_REINCARNATE:",
            "void F0361_COMMAND_ProcessKeyPress",
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        ],
    ),
    (
        "revive_f0280_f0282",
        RED / "REVIVE.C",
        [
            "void F0280_CHAMPION_AddCandidateChampionToParty",
            "if (!G0415_ui_LeaderEmptyHanded)",
            "void F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel",
            "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE);",
        ],
    ),
    (
        "chamdraw_f0291_f0292_f0296",
        RED / "CHAMDRAW.C",
        [
            "void F0291_CHAMPION_DrawSlot",
            "void F0292_CHAMPION_DrawState",
            "void F0296_CHAMPION_DrawChangedObjectIcons",
            "G0425_aT_ChestSlots",
        ],
    ),
    (
        "defs_c040_c30_c537_c544",
        RED / "DEFS.H",
        [
            "C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1",
            "#define C30_SLOT_CHEST_1",
            "#define C537_ZONE_SLOT_BOX_38_CHEST_1",
            "#define C544_ZONE_SLOT_BOX_45_CHEST_8",
            "G0299_ui_CandidateChampionOrdinal",
            "G0305_ui_PartyChampionCount",
            "M516_CHAMPIONS",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "header_contract",
        HDR,
        [
            "Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat",
            "candidateInternalRotationCount",
            "ignoredPickupDuringPartyRotationCount",
            "rotationCompletedBeforePickup",
        ],
    ),
    (
        "source_runtime_model",
        SRC,
        [
            "PANEL.C F0344:1895-1944 + F0345:1946-1999",
            "COMMAND.C F0380:2045-2156",
            "MOUSE.C F0077:97-126 + F0078:128-168",
            "candidateInternalRotationCount == 0",
            "DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat",
        ],
    ),
    (
        "test_assertions",
        TEST,
        [
            "scroll pickup is ignored during party rotation",
            "candidate chain bytes preserved during ignore",
            "party rotation flag clears before pickup is honored",
            "final hash is deterministic across runs",
        ],
    ),
    (
        "evidence_file",
        REPORT,
        [
            "COMMAND.C F0380:2045-2156",
            "REVIVE.C F0282:744-806",
            "CHAMDRAW.C F0296:1185-1252",
            "No original assets or `GRAPHICS.DAT` are required",
        ],
    ),
    (
        "cmake_registration",
        CMK,
        [
            TARGET,
            "src/dm1/dm1_v1_mirror_candidate_scroll_pickup_with_party_rotate_in_progress_pc34_compat.c",
            f"NAME {TEST_RE}",
        ],
    ),
]


def collect_checks() -> list[dict[str, object]]:
    checks: list[dict[str, object]] = []
    for label, path, needles in REDMCSB_CHECKS + LOCAL_CHECKS:
        checks.append(check_needles(label, path, needles))
    return checks


def write_manifest(checks: list[dict[str, object]], runs: list[dict[str, object]]) -> None:
    ok = all(row["status"] == "PASS" for row in checks) and all(run["passed"] for run in runs)
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(
        json.dumps(
            {
                "schema": f"firestaff.parity.{PASS}.v1",
                "status": "PASS" if ok else "FAIL",
                "timestampUtc": datetime.now(timezone.utc).isoformat(),
                "scope": (
                    "DM1 V1 C040 mirror candidate: panel scroll-pickup ignored "
                    "while F0380 party rotation is in progress; candidate "
                    "index/chain/redraw preserved; pickup honored only after "
                    "rotation completion."
                ),
                "redmcsbRoot": str(RED),
                "sourceChecks": checks,
                "verificationRuns": runs,
            },
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true", help="also build and run the test")
    args = parser.parse_args()

    checks = collect_checks()
    runs: list[dict[str, object]] = []
    if args.run:
        runs.append(run(["cmake", "--build", "build", "--target", TARGET, "--parallel"]))
        runs.append(run([str(resolve_build_dir(ROOT, ROOT / "build") / TARGET)]))
        runs.append(
            run(
                [
                    "ctest",
                    "--test-dir",
                    "build",
                    "-R",
                    TEST_RE,
                    "--output-on-failure",
                ]
            )
        )
        runs.append(run(["git", "diff", "--check"]))

    write_manifest(checks, runs)
    failed = [row for row in checks if row["status"] != "PASS"]
    failed_runs = [row for row in runs if not row["passed"]]
    if failed or failed_runs:
        print(json.dumps({"failedChecks": failed, "failedRuns": failed_runs}, indent=2))
        return 1
    print(f"PASS {PASS}: {len(checks)} checks, {len(runs)} runs")
    return 0


if __name__ == "__main__":
    sys.exit(main())

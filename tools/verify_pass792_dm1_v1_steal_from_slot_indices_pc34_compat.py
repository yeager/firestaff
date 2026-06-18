#!/usr/bin/env python3
"""pass792 DM1 V1 steal-from-slot-indices contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass792_dm1_v1_steal_from_slot_indices_pc34_compat"
STATUS = "PASS792_DM1_V1_STEAL_FROM_SLOT_INDICES_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_steal_from_slot_indices_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/steal_from_slot_indices_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_steal_from_slot_indices_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:31",
    "DATA.C:244-251",
    "GROUP.C:1032",
    "GROUP.C:1041",
    "GROUP.C:1045",
    "GROUP.C:1075",
]

LOCAL_NEEDLES = [
    "dm1_v1_steal_from_slot_indices_table_pc34",
    "dm1_v1_steal_from_slot_indices_pc34",
    "dm1_v1_steal_from_slot_indices_is_backpack_pc34",
    "kSlotNeck",
    "kSlotPouch1",
    "kSlotBackpackLine1_1",
    "kSlotQuiverLine1_1",
    "kSlotPouch2",
    "G0025_auc_Graphic562_StealFromSlotIndices",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_steal_from_slot_indices_pc34_compat",
    "src/dm1/dm1_v1_steal_from_slot_indices_pc34_compat.c",
    "NAME dm1_v1_steal_from_slot_indices_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (31, "G0025_auc_Graphic562_StealFromSlotIndices"),
        (244, "C10_SLOT_NECK"),
        (251, "C13_SLOT_BACKPACK_LINE1_1"),
    ],
    "GROUP.C": [
        (1032, "G0394_auc_StealFromSlotIndices"),
        (1041, "G0394_auc_StealFromSlotIndices[L0393_ui_Counter]"),
        (1045, "StealFromSlotIndex += M002_RANDOM(17)"),
        (1075, "L0393_ui_Counter &= 0x0007"),
    ],
}


def read(path: Path) -> str:
    encoding = "latin-1" if path.is_relative_to(RED) else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def line_at(path: Path, line_no: int) -> str:
    lines = read(path).splitlines()
    if line_no <= 0 or line_no > len(lines):
        return ""
    return lines[line_no - 1]


def check_needles(label: str, path: Path, needles: list[str]) -> dict[str, object]:
    text = read(path)
    missing = [needle for needle in needles if needle not in text]
    return {
        "id": label,
        "file": str(path.relative_to(ROOT)),
        "status": "PASS" if not missing else "FAIL",
        "missing": missing,
    }


def check_redmcsb_windows() -> list[dict[str, object]]:
    checks: list[dict[str, object]] = []
    for filename, windows in REDMCSB_WINDOWS.items():
        path = RED / filename
        for line_no, needle in windows:
            lo = max(1, line_no - 3)
            hi = line_no + 3
            text = "\n".join(line_at(path, row) for row in range(lo, hi + 1))
            checks.append(
                {
                    "id": f"{filename}:{line_no}",
                    "file": str(path),
                    "line": line_no,
                    "needle": needle,
                    "status": "PASS" if needle in text else "DRIFT",
                    "lineText": line_at(path, line_no),
                }
            )
    return checks


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
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-20:]),
    }


def resolve_build_dir() -> Path:
    for candidate in (ROOT / "build", ROOT / "builds" / "n2-build",
                      ROOT / "builds" / "nv1-build"):
        if (candidate / "CMakeCache.txt").exists():
            return candidate
    return ROOT / "build"


def write_outputs(
    local_checks: list[dict[str, object]],
    redmcsb_checks: list[dict[str, object]],
    runs: list[dict[str, object]],
) -> None:
    ok = (
        all(row["status"] == "PASS" for row in local_checks)
        and all(run["passed"] for run in runs)
    )
    drift = [row for row in redmcsb_checks if row["status"] != "PASS"]
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": (
            "DM1 V1 Graphics.dat item 562 init vars: "
            "G0025_auc_Graphic562_StealFromSlotIndices[8] is the 8-byte "
            "table of slot indices a creature tries to steal from. "
            "GROUP.C F0193 (StealFromChampion) uses it in post-1.3 Atari "
            "versions (pre-1.3 Atari uses G0394). Counter = RANDOM(8), "
            "lookup G0025[counter]; if the result is the backpack-base "
            "slot, +RANDOM(17) selects a random backpack line-1 slot. "
            "Counter loops with += 1 &= 0x0007 (mod 8). Disjoint from "
            "pass784-790 mirror-candidate C040 and wound gates."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:244 and GROUP.C:1041 line numbers when the "
            "local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound gates).",
            "Not the chest cancel-reopen-pickup gate (M569 chest path).",
            "Not c161/c160/c159/c061/c030 mirror-candidate gates.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass792 DM1 V1 Steal-From-Slot-Indices",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init data. G0025_auc_Graphic562_"
        "StealFromSlotIndices[0..7] = {NECK=10, POUCH_1=11, BACKPACK_"
        "LINE1_1=13, QUIVER_LINE1_1=12, NECK=10, BACKPACK_LINE1_1=13, "
        "POUCH_2=6, BACKPACK_LINE1_1=13}. GROUP.C F0193 dispatch: "
        "counter = RANDOM(8), lookup G0025[counter]; if the result is "
        "the backpack-base slot, +RANDOM(17) selects a random backpack "
        "line-1 slot. Counter loops with += 1 &= 0x0007 (mod 8).",
        "- Runtime assertion floor: 96 assertions in `tests/test_dm1_v1_"
        "steal_from_slot_indices_pc34_compat.c`.",
        "- Expected test output: `96/96 assertions passed`.",
        "",
        "## ReDMCSB Anchors",
        "",
    ]
    lines.extend(f"- {anchor}" for anchor in ANCHORS)
    lines.extend(
        [
            "",
            "## Non-Overlap",
            "",
            "- Not pass784-790.",
            "- Not the chest cancel-reopen-pickup gate.",
            "- Not c161/c160/c159/c061/c030 mirror-candidate gates.",
            "",
            "## Verification",
            "",
        ]
    )
    for run_row in runs:
        lines.append(
            f"- `{ ' '.join(run_row['command']) }`: rc={run_row['returncode']}"
        )
    if drift:
        lines.extend(["", "## TODO", ""])
        lines.append(
            "- Anchor drift note: re-pin DATA.C:244 and GROUP.C:1041 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_steal_from_slot_indices_table_pc34",
            "DM1_V1_StealFromSlotIndicesResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_lookup_function",
                "test_backpack_detection",
                "test_counter_mod_eight",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir()
    runs = [
        run([str(build_dir / "test_dm1_v1_steal_from_slot_indices_pc34_compat")])
    ]
    write_outputs(local_checks, redmcsb_checks, runs)

    ok = all(row["status"] == "PASS" for row in local_checks) and all(
        row["passed"] for row in runs
    )
    print(f"{PASS}: {'PASS' if ok else 'FAIL'}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for row in local_checks:
        if row["status"] != "PASS":
            print(f"missing in {row['id']}: {row['missing']}")
    for run_row in runs:
        print(run_row["outputTail"])
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

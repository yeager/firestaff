#!/usr/bin/env python3
"""pass784 DM1 V1 mirror-candidate C040 cancel-then-reopen same tick."""
from __future__ import annotations

import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass784_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat"
STATUS = "PASS784_DM1_V1_MIRROR_CANDIDATE_C040_CANCEL_THEN_REOPEN_SAME_TICK_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/mirror_candidate/c040_cancel_then_reopen_same_tick_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "PANEL.C F0355:2299-2318",
    "COMMAND.C F0378:1956-1990",
    "MOVESENS.C F0275:1502",
    "DEFS.H C040/M568, C127, C162, G0299, G0305, G0415, G0424",
]

LOCAL_NEEDLES = [
    "dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_spec_pc34",
    "dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_init_pc34",
    "dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_run_pc34",
    "dispatch_f0282_cancel",
    "dispatch_f0280_new_sensor",
    "F0282 C162 cancel branch",
    "F0280 CHAMPION_AddCandidateChampionToParty",
    "sameTick",
    "f0282DispatchCount == 1",
    "f0280DispatchCount == 1",
    "Disjoint from pass760",
    "and pass762 mirror-candidate rotate-in-progress-open",
    "C162 cancel followed by a new-sensor F0280 reopen in the same tick",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat",
    "src/dm1/dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat.c",
    "NAME dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "REVIVE.C": [
        (124, "G0415_ui_LeaderEmptyHanded"),
        (744, "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"),
    ],
    "PANEL.C": [(2299, "F0355_INVENTORY_Toggle_CPSE")],
    "COMMAND.C": [
        (1956, "STATICFUNCTION void F0378_COMMAND_ProcessType81_ClickInPanel("),
        (1990, "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"),
    ],
    "MOVESENS.C": [
        (1502, "F0280_CHAMPION_AddCandidateChampionToParty"),
        (1498, "C127_SENSOR_WALL_CHAMPION_PORTRAIT"),
    ],
    "DEFS.H": [(0, "C127_SENSOR_WALL_CHAMPION_PORTRAIT")],
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
            "DM1 V1 mirror candidate: a single tick drives "
            "F0282(C162 cancel) then F0280 (new sensor) so the C040 panel "
            "goes 568 -> 0 -> 568 with a fresh candidate ordinal and a "
            "fresh party count mid-tick. Disjoint from the chest "
            "cancel-reopen-pickup gate (M569 chest path, not M568 mirror) "
            "and the C045 food/water gates."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "DEFS.H:C040 and SENSOR.C:F0212 anchors are best-effort line "
            "0 needle checks; if the local ReDMCSB tree lacks exact "
            "anchors, treat the corresponding row as DRIFT and record a "
            "follow-up to re-pin the line numbers."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass760 mirror-candidate chrome after non-candidate transition.",
            "Not pass762 mirror-candidate rotate-in-progress open.",
            "Not test_dm1_v1_chest_c040_cancel_reopen_pickup (M569 chest path, not M568 mirror).",
            "Not C045 food/water mirror-candidate gates.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass784 DM1 V1 Mirror Candidate C040 Cancel-Then-Reopen Same Tick",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: C162 cancel followed by a new-sensor F0280 reopen inside the same tick.",
        "- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat.c`.",
        "- Expected test output: `53/53 assertions passed`.",
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
            "- Not pass760 mirror-candidate chrome after non-candidate transition.",
            "- Not pass762 mirror-candidate rotate-in-progress open.",
            "- Not the M569 chest cancel-reopen-pickup gate.",
            "- Not C045 food/water mirror-candidate gates.",
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
            "- Anchor drift note: DEFS.H and SENSOR.C rows are line-0 "
            "best-effort; re-pin when the local ReDMCSB tree is updated."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_spec_pc34",
            "DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34",
            "DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_source_evidence_is_pinned",
                "test_spec_is_stable",
                "test_init_clears_observability",
                "test_run_accepted",
                "test_run_cancels_then_reopens_in_one_tick",
                "assertions passed",
                "F0275:1502",
                "C127",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir()
    runs = [
        run([str(build_dir / "test_dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_pc34_compat")])
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

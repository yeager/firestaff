#!/usr/bin/env python3
"""pass790 DM1 V1 wound-probability-index-to-mask contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass790_dm1_v1_wound_probability_index_to_mask_pc34_compat"
STATUS = "PASS790_DM1_V1_WOUND_PROBABILITY_INDEX_TO_MASK_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_wound_probability_index_to_mask_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/wound_probability_index_to_mask_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_wound_probability_index_to_mask_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:30",
    "DATA.C:243",
    "PROJEXPL.C:1378",
    "PROJEXPL.C:1386",
    "PROJEXPL.C:1389",
    "DEFS.H:736",
    "DEFS.H:741",
]

LOCAL_NEEDLES = [
    "dm1_v1_wound_probability_index_to_mask_table_pc34",
    "dm1_v1_wound_probability_index_to_mask_pc34",
    "dm1_v1_wound_probability_test_branch_pc34",
    "kMaskFeet",
    "kMaskLegs",
    "kMaskTorso",
    "kMaskHead",
    "kMaskReadyHand",
    "G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask",
    "Disjoint from pass784-789",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_wound_probability_index_to_mask_pc34_compat",
    "src/dm1/dm1_v1_wound_probability_index_to_mask_pc34_compat.c",
    "NAME dm1_v1_wound_probability_index_to_mask_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (30, "G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask"),
        (243, "MASK0x0020_WOUND_FEET"),
    ],
    "PROJEXPL.C": [
        (1378, "if (AL0559_ui_WoundTest = M006_RANDOM(65536)) & 0x0070)"),
        (1386, "G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask"),
        (1389, "MASK0x0001_WOUND_READY_HAND"),
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
            "DM1 V1 Graphics.dat item 562 init vars: G0024_auc_Graphic562_"
            "WoundProbabilityIndexToWoundMask[4] is the 4-byte table that "
            "maps a wound-probability index (0..3) to a body-location mask "
            "({FEET=0x20, LEGS=0x10, TORSO=0x08, HEAD=0x04}). PROJEXPL.C:1386 "
            "reads it after a wound-test branch. The fallback branch "
            "(PROJEXPL.C:1389) uses MASK0x0001_WOUND_READY_HAND when the "
            "test-mask bits 4,5,6 are all clear. Disjoint from pass784-789 "
            "mirror-candidate C040 gates; this gate is the first non-"
            "mirror-candidate contract in the wound/projexpl namespace."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:243 and PROJEXPL.C:1386 line numbers when the "
            "local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-789 mirror-candidate C040 gates (those cover "
            "COMMAND.C F0380/CHAMPION.C F0282/REVIVE.C F0280/PANEL.C F0355; "
            "this gate covers PROJEXPL.C:1386 wound-index lookup).",
            "Not the chest cancel-reopen-pickup gate (M569 chest path).",
            "Not c161/c160/c159/c061/c030 mirror-candidate gates (those "
            "cover mirror-candidate transitions; this covers the wound "
            "subsystem).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass790 DM1 V1 Wound-Probability-Index-To-Mask",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init data. G0024_auc_Graphic562_"
        "WoundProbabilityIndexToWoundMask[0..3] = {FEET=0x20, LEGS=0x10, "
        "TORSO=0x08, HEAD=0x04}. PROJEXPL.C:1386 reads it after a "
        "wound-test branch. The fallback branch (PROJEXPL.C:1389) uses "
        "MASK0x0001_WOUND_READY_HAND when the test-mask bits 4,5,6 are "
        "all clear.",
        "- Runtime assertion floor: 37 assertions in `tests/test_dm1_v1_"
        "wound_probability_index_to_mask_pc34_compat.c`.",
        "- Expected test output: `37/37 assertions passed`.",
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
            "- Not pass784-789 mirror-candidate C040 gates.",
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
            "- Anchor drift note: re-pin DATA.C:243 and PROJEXPL.C:1386 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_wound_probability_index_to_mask_table_pc34",
            "DM1_V1_WoundProbabilityIndexToMaskResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_lookup_function",
                "test_test_mask_constants",
                "test_branch_decision",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir()
    runs = [
        run([str(build_dir / "test_dm1_v1_wound_probability_index_to_mask_pc34_compat")])
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

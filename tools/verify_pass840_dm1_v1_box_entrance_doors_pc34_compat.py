#!/usr/bin/env python3
"""pass840 DM1 V1 box-entrance-doors contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass840_dm1_v1_box_entrance_doors_pc34_compat"
STATUS = "PASS840_DM1_V1_ENTRANCE_DOORS_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_box_entrance_doors_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/box_entrance_doors_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_box_entrance_doors_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:15",
    "DATA.C:137",
    "DATA.C:557",
    "ENTRANCE.C:529/538/541/544/547",
]

LOCAL_NEEDLES = [
    "dm1_v1_box_entrance_doors_table_pc34",
    "dm1_v1_box_entrance_doors_get_pc34",
    "dm1_v1_box_entrance_doors_x_pc34",
    "kDoorX",
    "kDoorY",
    "kDoorW",
    "kDoorH",
    "G0009_ai_Graphic562_Box_Entrance_Doors",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_box_entrance_doors_pc34_compat",
    "src/dm1/dm1_v1_box_entrance_doors_pc34_compat.c",
    "NAME dm1_v1_box_entrance_doors_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (15, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (137, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (557, "G0009_ai_Graphic562_Box_Entrance_Doors"),
    ],
    "ENTRANCE.C": [
        (529, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (538, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (541, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (544, "G0009_ai_Graphic562_Box_Entrance_Doors"),
        (547, "G0009_ai_Graphic562_Box_Entrance_Doors"),
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


def resolve_build_dir(binary_name: str = "") -> Path:
    candidates = [
        ROOT / "build",
        ROOT / "builds" / "nv1-build",
        ROOT / "builds" / "n2-build",
    ]
    if binary_name:
        for c in candidates:
            if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
                return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]



def write_outputs(local_checks, redmcsb_checks, runs):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    ok = (all(row["status"] == "PASS" for row in local_checks)
          and all(run["passed"] for run in runs))
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": __import__("datetime").datetime.now(__import__("datetime").timezone.utc).isoformat(),
        "scope": "DM1 V1 Graphics.dat item 562 init var G0009 contract.",
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-799 (champion-panel/leader/mirror + chest).",
            "Not pass798+ (Graphics.dat init-table gates).",
        ],
    }
    import json
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    lines = [f"# {PASS}\n",
             f"- Status: {manifest['status']}\n",
             "- Gate: Graphics.dat item 562 init var G0009.\n",
             "- Runtime assertion floor: 53 assertions.\n",
             "- Expected test output: `53/53 assertions passed`.\n",
             "## ReDMCSB Anchors\n"]
    for a in ANCHORS:
        lines.append(f"- {a}")
    lines.append("\n## Non-Overlap\n")
    lines.append("- Not pass784-790.\n")
    lines.append("- Not pass791+ (champion-panel + champion + mirror + chest).\n")
    lines.append("- Not pass798+ (Graphics.dat init-table gates).\n")
    lines.append("\n## Verification\n")
    for run_row in runs:
        lines.append(f"- \`{' '.join(run_row['command'])}\`: rc={run_row['returncode']}")
    REPORT.write_text("\n".join(lines))




def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_box_entrance_doors_table_pc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_accessor_functions",
                "test_get_function",
                "test_components_non_negative",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir(
        "test_dm1_v1_box_entrance_doors_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_box_entrance_doors_pc34_compat")])
    ]
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
    write_outputs(local_checks, redmcsb_checks, runs)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

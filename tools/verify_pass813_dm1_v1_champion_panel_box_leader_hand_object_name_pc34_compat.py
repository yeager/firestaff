#!/usr/bin/env python3
"""pass813 DM1 V1 champion-panel-box-leader-hand-object-name contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass813_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat"
STATUS = "PASS813_DM1_V1_CHAMPION_PANEL_BOX_LEADER_HAND_OBJECT_NAME_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/champion_panel_box_leader_hand_object_name_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:34",
    "DATA.C:262",
    "DATA.C:923",
    "OBJECT.C:281",
]

LOCAL_NEEDLES = [
    "dm1_v1_champion_panel_box_leader_hand_object_name_table_pc34",
    "dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34",
    "dm1_v1_champion_panel_box_leader_hand_object_name_x_pc34",
    "kLeaderX",
    "kLeaderY",
    "kLeaderW",
    "kLeaderH",
    "G0028_ai_Graphic562_Box_LeaderHandObjectName",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat",
    "src/dm1/dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat.c",
    "NAME dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (34, "G0028_ai_Graphic562_Box_LeaderHandObjectName"),
        (262, "G0028_ai_Graphic562_Box_LeaderHandObjectName"),
        (923, "G0028_ai_Graphic562_Box_LeaderHandObjectName"),
    ],
    "OBJECT.C": [
        (281, "G0028_ai_Graphic562_Box_LeaderHandObjectName"),
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
            "DM1 V1 Graphics.dat item 562 init var "
            "G0028_ai_Graphic562_Box_LeaderHandObjectName[4] = "
            "{233, 319, 33, 38}. The {X, Y, W, H} pixel-coordinate "
            "rectangle for the leader-hand object name black-fill "
            "backdrop. Read site: OBJECT.C:281 M524_FillScreenBox("
            "G0028, C00_COLOR_BLACK) draws the black backdrop for "
            "the leader-hand object name text."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:262/923 + OBJECT.C:281 line numbers when "
            "the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-799 (champion-panel/leader/mirror + chest).",
            "Not pass798-812 (Graphics.dat init-table gates batches "
            "1+2+3+4).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass813 DM1 V1 Champion-Panel-Box-Leader-Hand-Object-Name",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0028_ai_Graphic562_"
        "Box_LeaderHandObjectName[4] = {233, 319, 33, 38}. The "
        "{X, Y, W, H} pixel-coordinate rectangle for the leader-hand "
        "object name black-fill backdrop. OBJECT.C:281 "
        "M524_FillScreenBox(G0028, C00_COLOR_BLACK) draws the black "
        "backdrop before the leader-hand object name text is "
        "rendered on top.",
        "- Runtime assertion floor: 50 assertions in `tests/test_dm1_v1_"
        "champion_panel_box_leader_hand_object_name_pc34_compat.c`.",
        "- Expected test output: `50/50 assertions passed`.",
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
            "- Not pass791 (champion-panel ammo-compat).",
            "- Not pass792 (steal-from-slot-indices).",
            "- Not pass793-799 (champion-panel/leader/mirror + chest).",
            "- Not pass798-812 (Graphics.dat init-table gates batches "
            "1+2+3+4).",
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
            "- Anchor drift note: re-pin DATA.C:262/923, OBJECT.C:281 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_champion_panel_box_leader_hand_object_name_table_pc34",
            "DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34",
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
        "test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat")])
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
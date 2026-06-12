#!/usr/bin/env python3
"""pass760 DM1 V1 mirror-candidate C040 chrome after non-candidate transition."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression"
STATUS = "PASS760_DM1_V1_MIRROR_CANDIDATE_CHROME_AFTER_NON_CANDIDATE_TRANSITION_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_mirror_candidate_pc34_compat.c"
TEST = ROOT / "tests/test_dm1_v1_mirror_candidate_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "CHAMDRAW.C F0291/F0296:551-552,1249-1252",
    "CHAMPION.C F0284:93-131",
    "CHAMPION.C F0297:243-268",
    "CHAMPION.C F0298:270-298",
    "CHAMPION.C F0300:511-515",
    "CHAMPION.C F0301:606-614",
    "CHAMPION.C F0302:662-714",
    "COMMAND.C F0359:1985-1990",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:113-132",
    "PANEL.C F0344:1895-1944",
    "PANEL.C F0345:1946-1999",
    "OBJECT.C F0033:147-212",
    "BLITMASK.C F0133:30-33",
    "DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040",
]

LOCAL_NEEDLES = [
    "dm1_v1_mirror_candidate_pass760_run_pc34_compat",
    "close_candidate_panel",
    "apply_non_candidate_transition",
    "reopen_candidate_panel",
    "stalePixelsClearedBeforeReopen",
    "newChromePublishedAfterReopen",
    "distinct from pass674",
    "pass686",
    "pass710/pass711",
    "pass736",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_mirror_candidate_pc34_compat",
    "src/dm1/dm1_v1_mirror_candidate_pc34_compat.c",
    "NAME pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "CHAMDRAW.C": [(551, "G0425_aT_ChestSlots"), (1249, "G0425_aT_ChestSlots")],
    "CHAMPION.C": [
        (93, "F0284_CHAMPION_SetPartyDirection"),
        (243, "F0297_CHAMPION_PutObjectInLeaderHand"),
        (270, "F0298_CHAMPION_GetObjectRemovedFromLeaderHand"),
        (511, "M516_CHAMPIONS"),
        (606, "P0631_T_Thing"),
        (662, "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox"),
    ],
    "COMMAND.C": [(1985, "M568_PANEL_RESURRECT_REINCARNATE")],
    "REVIVE.C": [(124, "G0415_ui_LeaderEmptyHanded"), (744, "G0305_ui_PartyChampionCount")],
    "CHEST.C": [(30, "G0426_T_OpenChest"), (113, "G0426_T_OpenChest")],
    "OBJECT.C": [(147, "F0033_OBJECT_GetIconIndex")],
    "BLITMASK.C": [(30, "This function creates a bitmap")],
    "DEFS.H": [(2088, "C10_COLOR_FLESH")],
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
    defs_line = line_at(RED / "DEFS.H", 2088)
    defs_anchor_todo = "C30" not in defs_line or "G0425" not in defs_line
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": (
            "C040 mirror-candidate close, non-candidate inventory/chest/slot "
            "transition, and reopened C040 chrome freshness with no stale "
            "C040 owner/generation pixels."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Keep the requested DEFS.H:2088 anchor unchanged; in this local "
            "ReDMCSB tree that exact line is C10_COLOR_FLESH while the named "
            "C30/G0425/G0426/G0423/G0305/M070/M516/C040 symbols live on "
            "other DEFS.H lines."
        ]
        if drift or defs_anchor_todo
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "pass674 scroll-pickup-leader-rotation-inventory-click is not used",
            "pass686 keyboard-browse-occupied-slot is not used",
            "pass710/pass711 live-panel C045/C038 routes are not used",
            "pass736 close-while-resurrect-pending pickup is not used",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass760 DM1 V1 Mirror Candidate Chrome After Non-Candidate Transition",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: candidate close -> non-candidate inventory/chest/slot transition -> reopened C040 chrome.",
        "- Runtime assertion floor: >=80 assertions in `tests/test_dm1_v1_mirror_candidate_pc34_compat.c`.",
        "- Expected test output: `PASS pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression`.",
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
            "- Not pass674 scroll-pickup-leader-rotation-inventory-click.",
            "- Not pass686 keyboard-browse-occupied-slot.",
            "- Not pass710/pass711 live-panel C045/C038 drop/pickup.",
            "- Not pass736 close-while-resurrect-pending with inventory pickup.",
            "",
            "## Verification",
            "",
        ]
    )
    for run_row in runs:
        lines.append(
            f"- `{ ' '.join(run_row['command']) }`: rc={run_row['returncode']}"
        )
    if drift or defs_anchor_todo:
        lines.extend(["", "## TODO", ""])
        lines.append(
            "- Anchor drift note: keep requested `DEFS.H:2088 "
            "C30/G0425/G0426/G0423/G0305/M070/M516/C040`; local line 2088 "
            "is `C10_COLOR_FLESH`, while those symbols are elsewhere in DEFS.H."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "passXXX_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression",
                "assertionsExpectedAtLeast, 80",
                "after-close framebuffer has no C040 tag",
                "after-transition framebuffer has no C040 tag",
                "final framebuffer is C040",
            ]
            + ANCHORS,
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    runs = [run([str(ROOT / "build/test_dm1_v1_mirror_candidate_pc34_compat")])]
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

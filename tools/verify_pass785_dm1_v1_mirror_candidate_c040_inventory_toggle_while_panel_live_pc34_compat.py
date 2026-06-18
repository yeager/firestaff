#!/usr/bin/env python3
"""pass785 DM1 V1 mirror-candidate C040 inventory-toggle-while-panel-live."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass785_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat"
STATUS = "PASS785_DM1_V1_MIRROR_CANDIDATE_C040_INVENTORY_TOGGLE_WHILE_PANEL_LIVE_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/mirror_candidate/c040_inventory_toggle_while_panel_live_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "COMMAND.C F0380:2181-2183",
    "PANEL.C F0355:2299-2318",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "DEFS.H C007..C011, C040/M568, G0299, G0305, G0411",
]

LOCAL_NEEDLES = [
    "dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_spec_pc34",
    "dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_init_pc34",
    "dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_run_pc34",
    "dispatch_f0355_inventory_toggle",
    "F0380 gate",
    "f0355CallsWhileLive == 0",
    "rejectedWhileLive == 5",
    "f0355CallsAfterClear == 1",
    "Disjoint from pass784",
    "inventory-toggle-while-c040-live",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat",
    "src/dm1/dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat.c",
    "NAME dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "COMMAND.C": [
        (2181, "G0299_ui_CandidateChampionOrdinal"),
        (2176, "F0355_INVENTORY_Toggle_CPSE"),
    ],
    "PANEL.C": [(2299, "F0355_INVENTORY_Toggle_CPSE")],
    "REVIVE.C": [
        (124, "G0415_ui_LeaderEmptyHanded"),
        (744, "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"),
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
    # Prefer the candidate that has both CMakeCache.txt and the binary
    if binary_name:
        for c in candidates:
            if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
                return c
    # Fall back to the first candidate with CMakeCache.txt
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
            "DM1 V1 mirror-candidate C040: while the candidate panel is "
            "live (G0299 set), the C007..C011 inventory-toggle commands "
            "are gated by COMMAND.C F0380 and dropped without calling "
            "F0355. After F0282(C162) clears G0299 the toggle becomes live "
            "again. Disjoint from pass784 cancel-then-reopen same-tick and "
            "from the C160/C161 accept path."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin the COMMAND.C F0380:2181 line number when the local "
            "ReDMCSB tree updates; the gate line may shift in future "
            "checkouts."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784 cancel-then-reopen same-tick (that gate covers "
            "F0282(C162) and a new-sensor F0280 in the same tick; this "
            "gate covers F0380:2181-2183 inventory-toggle rejection while "
            "the panel is live).",
            "Not the C160/C161 accept path (those go through F0282 without "
            "clearing G0299 until accept; this gate only tests C162 cancel).",
            "Not the chest cancel-reopen-pickup gate (M569 chest path).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass785 DM1 V1 Mirror Candidate C040 Inventory-Toggle-While-Panel-Live",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: inventory-toggle commands C007..C011 dispatched while "
        "G0299 is set are dropped by COMMAND.C F0380:2181-2183; after "
        "F0282(C162) clears G0299 the toggle becomes live again.",
        "- Runtime assertion floor: 44 assertions in `tests/test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat.c`.",
        "- Expected test output: `44/44 assertions passed`.",
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
            "- Not pass784 cancel-then-reopen same-tick.",
            "- Not the C160/C161 accept path.",
            "- Not the chest cancel-reopen-pickup gate.",
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
            "- Anchor drift note: re-pin COMMAND.C F0380:2181 line number "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_spec_pc34",
            "DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34",
            "DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_source_evidence_is_pinned",
                "test_spec_is_stable",
                "test_init_clears_observability",
                "test_run_accepted",
                "test_run_gates_inventory_while_panel_live",
                "test_run_unlocks_after_cancel",
                "assertions passed",
                "F0380:2181-2183",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir("test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat")
    runs = [
        run([str(build_dir / "test_dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat")])
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

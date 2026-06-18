#!/usr/bin/env python3
"""pass786 DM1 V1 mirror-candidate C040 spell-area-click-while-panel-live."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass786_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat"
STATUS = "PASS786_DM1_V1_MIRROR_CANDIDATE_C040_SPELL_AREA_CLICK_WHILE_PANEL_LIVE_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/mirror_candidate/c040_spell_area_click_while_panel_live_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "COMMAND.C F0380:2303-2306",
    "COMMAND.C F0370:2482-2520",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "DEFS.H C100, C040/M568, G0299, G0305, G0411, G0514",
]

LOCAL_NEEDLES = [
    "dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_spec_pc34",
    "dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_init_pc34",
    "dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34",
    "dispatch_f0370_spell_area_click",
    "F0380 gate",
    "f0370CallsWhileLive == 0",
    "rejectedWhileLive == 3",
    "f0370CallsAfterClear == 1",
    "rejectedNoCaster == 1",
    "Disjoint from pass785",
    "spell-area-click-while-c040-live",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat",
    "src/dm1/dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat.c",
    "NAME dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "COMMAND.C": [
        (2303, "!G0299_ui_CandidateChampionOrdinal"),
        (2303, "G0514_i_MagicCasterChampionIndex"),
    ],
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
            "live (G0299 set), COMMAND.C F0380:2303-2306 drops the C100 "
            "spell-area click. After F0282(C162) clears G0299 with a valid "
            "G0514 magic-caster, the click fires F0370. If G0514 is then "
            "cleared to CM1_CHAMPION_NONE the click is dropped again. "
            "Disjoint from pass785 (inventory-toggle) and pass784 "
            "(cancel-then-reopen same-tick)."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin COMMAND.C F0380:2303 and F0370:2482 line numbers when "
            "the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass785 (inventory-toggle-while-c040-live covers "
            "F0380:2181-2183 C007..C011; this gate covers F0380:2303-2306 "
            "C100 with the additional G0514 magic-caster gate).",
            "Not pass784 cancel-then-reopen same-tick.",
            "Not the C111 action-area gate (which is a separate F0380 "
            "line; not yet covered by any pass).",
            "Not the chest cancel-reopen-pickup gate (M569 chest path).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass786 DM1 V1 Mirror Candidate C040 Spell-Area-Click-While-Panel-Live",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: COMMAND.C F0380:2303-2306 drops C100 spell-area clicks "
        "while G0299 is set; after F0282(C162) clears G0299 with a valid "
        "G0514 the click fires F0370; if G0514 is then cleared the click "
        "is dropped again.",
        "- Runtime assertion floor: 48 assertions in `tests/test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat.c`.",
        "- Expected test output: `48/48 assertions passed`.",
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
            "- Not pass785 inventory-toggle-while-c040-live.",
            "- Not pass784 cancel-then-reopen same-tick.",
            "- Not the C111 action-area gate (separate F0380 line).",
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
            "- Anchor drift note: re-pin COMMAND.C F0380:2303 and "
            "F0370:2482 line numbers in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_spec_pc34",
            "DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveStatePc34",
            "DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_source_evidence_is_pinned",
                "test_spec_is_stable",
                "test_init_clears_observability",
                "test_run_accepted",
                "test_run_gates_spell_area_while_panel_live",
                "test_run_unlocks_after_cancel_with_caster",
                "test_run_drops_after_caster_clear",
                "assertions passed",
                "F0380:2303-2306",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir("test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat")
    runs = [
        run([str(build_dir / "test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat")])
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

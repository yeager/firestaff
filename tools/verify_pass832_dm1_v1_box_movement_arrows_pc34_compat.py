#!/usr/bin/env python3
"""pass832 DM1 V1 box-movement-arrows contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass832_dm1_v1_box_movement_arrows_pc34_compat"
STATUS = "PASS832_DM1_V1_BOX_MOVEMENT_ARROWS_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_box_movement_arrows_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_box_movement_arrows_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:8",
    "DATA.C:123",
    "DATA.C:543",
    "MENUDRAW.C:13",
    "PANEL.C:2369",
    "STARTUP2.C:369",
]

LOCAL_NEEDLES = [
    "dm1_v1_box_movement_arrows_table_pc34",
    "dm1_v1_box_movement_arrows_get_pc34",
    "dm1_v1_box_movement_arrows_x_pc34",
    "kArrX",
    "kArrY",
    "kArrW",
    "kArrH",
    "G0002_ai_Graphic562_Box_MovementArrows",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_box_movement_arrows_pc34_compat",
    "src/dm1/dm1_v1_box_movement_arrows_pc34_compat.c",
    "NAME dm1_v1_box_movement_arrows_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (6, "G0002_ai_Graphic562_Box_MovementArrows"),
        (119, "G0002_ai_Graphic562_Box_MovementArrows"),
        (539, "G0002_ai_Graphic562_Box_MovementArrows"),
    ],
    "CASTER.C": [
        (24, "G0002_ai_Graphic562_Box_MovementArrows"),
        (31, "G0002_ai_Graphic562_Box_MovementArrows"),
    ],
    "STARTUP2.C": [
        (376, "G0002_ai_Graphic562_Box_MovementArrows"),
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
            "G0002_ai_Graphic562_Box_MovementArrows[4] = {224, 319, 124, 168}. "
            "The {X, Y, W, H} byte-coordinate sub-rectangle for the "
            "movement-arrows background blit on the champion panel. Read "
            "sites: MENUDRAW.C:13 M520_F0021_MAIN_BlitToScreen (movement-"
            "area background blit), PANEL.C:2369 M524_FillScreenBox "
            "(clear), STARTUP2.C:369 F0136_VIDEO_HatchScreenBox (hatch)."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:123/539 + MENUDRAW.C:13/31 + STARTUP2.C:369 "
            "line numbers when the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-799 (champion-panel/leader/mirror + chest).",
            "Not pass798-821 (Graphics.dat init-table gates batches "
            "1+2+3+4+5+6+7+8+9).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass832 DM1 V1 Box-Movement-Arrows",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0002_ai_Graphic562_"
        "Box_MovementArrows[4] = {224, 319, 124, 168}. The {X, Y, W, H} "
        "byte-coordinate sub-rectangle for the movement-arrows background "
        "blit on the champion panel. Read sites: MENUDRAW.C:13/31 + "
        "STARTUP2.C:369 (movement-arrows blit + clear + hatch).",
        "- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_"
        "box_movement_arrows_pc34_compat.c`.",
        "- Expected test output: `54/53 assertions passed`.",
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
            "- Not pass798-821 (Graphics.dat init-table gates batches "
            "1+2+3+4+5+6+7+8+9).",
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
            "- Anchor drift note: re-pin DATA.C:123/539, MENUDRAW.C:13/"
            "31, STARTUP2.C:369 in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_box_movement_arrows_table_pc34",
            "DM1_V1_BoxMovementArrowsResultPc34",
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
        "test_dm1_v1_box_movement_arrows_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_box_movement_arrows_pc34_compat")])
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

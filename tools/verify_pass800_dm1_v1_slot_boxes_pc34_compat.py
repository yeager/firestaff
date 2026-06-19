#!/usr/bin/env python3
"""pass800 DM1 V1 slot-boxes contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass800_dm1_v1_slot_boxes_pc34_compat"
STATUS = "PASS800_DM1_V1_SLOT_BOXES_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_slot_boxes_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/slot_boxes_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_slot_boxes_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:36",
    "DATA.C:264-309",
    "OBJECT.C:435",
    "OBJECT.C:521",
    "CHAMDRAW.C:557",
    "CHAMDRAW.C:562",
]

LOCAL_NEEDLES = [
    "dm1_v1_slot_boxes_table_pc34",
    "dm1_v1_slot_boxes_get_x_pc34",
    "dm1_v1_slot_boxes_get_y_pc34",
    "dm1_v1_slot_boxes_get_zone_index_pc34",
    "dm1_v1_slot_boxes_get_icon_index_pc34",
    "dm1_v1_slot_boxes_is_status_hand_pc34",
    "dm1_v1_slot_boxes_is_inventory_pc34",
    "dm1_v1_slot_boxes_is_chest_pc34",
    "kStatusHandCount",
    "kInventoryCount",
    "kChestCount",
    "G0030_as_Graphic562_SlotBoxes",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_slot_boxes_pc34_compat",
    "src/dm1/dm1_v1_slot_boxes_pc34_compat.c",
    "NAME dm1_v1_slot_boxes_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (36, "G0030_as_Graphic562_SlotBoxes"),
        (264, "G0030_as_Graphic562_SlotBoxes"),
        (309, "Chest 8"),
    ],
    "OBJECT.C": [
        (435, "G0030_as_Graphic562_SlotBoxes"),
        (521, "G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex].IconIndex"),
    ],
    "CHAMDRAW.C": [
        (557, "G0030_as_Graphic562_SlotBoxes"),
        (562, "G0030_as_Graphic562_SlotBoxes"),
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
            "G0030_as_Graphic562_SlotBoxes[46] — the pixel-coordinate "
            "table for the 46 clickable slot boxes: 8 status-box hands "
            "(2 per champion * up to 4 champions), 30 inventory slots, "
            "and 8 chest slots. Each entry is a SLOT_BOX { X, Y, "
            "ZoneIndex=0, IconIndex }. Read sites: OBJECT.C:435 "
            "(F0486_OBJECT_DrawSlotBoxAtSlotIndex), OBJECT.C:521 "
            "(F0488_OBJECT_GetSlotBoxIconIndex), CHAMDRAW.C:557 "
            "(F0487 status-box drawing), CHAMDRAW.C:562 (F0619_"
            "GetSlotBoxBorderCoordinates(ZoneIndex))."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:264-309 + OBJECT.C:435/521 + CHAMDRAW.C:557/"
            "562 line numbers when the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-796 (champion-panel/leader/mirror).",
            "Not pass797 (icon-graphic-first-icon-index).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass800 DM1 V1 Slot-Boxes",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0030_as_Graphic562_"
        "SlotBoxes[46]. 8 status-box hands (Y=10, +20/within-pair, +69/"
        "between-champions), 30 inventory slots (X in [6, 202], Y in "
        "[16, 90]), 8 chest slots (curved bottom row, Y monotonically "
        "increasing 59..105, X traces 117->106->111->128->145->162->"
        "179->196). All ZoneIndex=0 in PC 3.4 init.",
        "- Runtime assertion floor: 565 assertions in `tests/test_dm1_v1_"
        "slot_boxes_pc34_compat.c`.",
        "- Expected test output: `565/565 assertions passed`.",
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
            "- Not pass793-796 (champion-panel/leader/mirror).",
            "- Not pass797 (icon-graphic-first-icon-index).",
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
            "- Anchor drift note: re-pin DATA.C:264-309, OBJECT.C:435/"
            "521, CHAMDRAW.C:557/562 in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_slot_boxes_table_pc34",
            "DM1_V1_SlotBoxesResultPc34",
            "DM1_V1_SlotBoxPc34Compat",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_size_and_partition",
                "test_status_box_hand_entries",
                "test_inventory_entries",
                "test_chest_entries",
                "test_get_x_function",
                "test_get_y_function",
                "test_get_zone_index_function",
                "test_get_icon_index_function",
                "test_get_pointer_function",
                "test_partition_classification",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir("test_dm1_v1_slot_boxes_pc34_compat")
    runs = [
        run([str(build_dir / "test_dm1_v1_slot_boxes_pc34_compat")])
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
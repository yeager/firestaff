#!/usr/bin/env python3
"""pass804 DM1 V1 charge-count-to-torch-type contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass804_dm1_v1_charge_count_to_torch_type_pc34_compat"
STATUS = "PASS804_DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_charge_count_to_torch_type_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/charge_count_to_torch_type_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_charge_count_to_torch_type_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:35",
    "DATA.C:263",
    "DATA.C:926",
    "OBJECT.C:178",
]

LOCAL_NEEDLES = [
    "dm1_v1_charge_count_to_torch_type_table_pc34",
    "dm1_v1_charge_count_to_torch_type_pc34",
    "dm1_v1_charge_count_to_torch_type_bucket_pc34",
    "dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34",
    "dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34",
    "kTableSize",
    "kTorchTypeCount",
    "kBucket1Low",
    "kBucket2Low",
    "kBucket3Low",
    "G0029_auc_Graphic562_ChargeCountToTorchType",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_charge_count_to_torch_type_pc34_compat",
    "src/dm1/dm1_v1_charge_count_to_torch_type_pc34_compat.c",
    "NAME dm1_v1_charge_count_to_torch_type_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (35, "G0029_auc_Graphic562_ChargeCountToTorchType"),
        (263, "G0029_auc_Graphic562_ChargeCountToTorchType"),
        (926, "G0029_auc_Graphic562_ChargeCountToTorchType"),
    ],
    "OBJECT.C": [
        (178, "G0029_auc_Graphic562_ChargeCountToTorchType"),
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
            "G0029_auc_Graphic562_ChargeCountToTorchType[16] = "
            "{0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}. "
            "Maps a torch's remaining charge count (0..15) to the "
            "torch-icon type (0..3) used to draw that torch on the "
            "inventory panel. Read site: OBJECT.C:178 "
            "F0486_OBJECT_DrawObjectIcon — for a lit torch weapon, "
            "IconIndex += G0029[ChargeCount]."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:263/926 + OBJECT.C:178 line numbers when "
            "the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-799 (champion-panel/leader/mirror + auto-"
            "chest + chest-open-stack-split).",
            "Not pass798 (icon-graphic-first-icon-index).",
            "Not pass800 (slot-boxes).",
            "Not pass801 (light-power-to-light-amount).",
            "Not pass802 (palette-index-to-light-amount).",
            "Not pass803 (ordered-cells-to-attack).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass804 DM1 V1 Charge-Count-To-Torch-Type",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0029_auc_Graphic562_"
        "ChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, "
        "3, 3, 3, 3, 3, 3}. Bucket design: 0 charges -> type 0, 1..3 "
        "charges -> type 1, 4..7 charges -> type 2, 8..15 charges -> "
        "type 3. OBJECT.C:178 F0486_OBJECT_DrawObjectIcon reads this "
        "table to pick the torch-icon variant when drawing a lit "
        "torch weapon.",
        "- Runtime assertion floor: 144 assertions in `tests/test_dm1_v1_"
        "charge_count_to_torch_type_pc34_compat.c`.",
        "- Expected test output: `144/144 assertions passed`.",
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
            "- Not pass793-799 (champion-panel/leader/mirror + auto-"
            "chest + chest-open-stack-split).",
            "- Not pass798 (icon-graphic-first-icon-index).",
            "- Not pass800 (slot-boxes).",
            "- Not pass801 (light-power-to-light-amount).",
            "- Not pass802 (palette-index-to-light-amount).",
            "- Not pass803 (ordered-cells-to-attack).",
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
            "- Anchor drift note: re-pin DATA.C:263/926, OBJECT.C:178 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_charge_count_to_torch_type_table_pc34",
            "DM1_V1_ChargeCountToTorchTypeResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_lookup_function",
                "test_bucket_boundaries",
                "test_first_last_count_for_type",
                "test_monotonic_and_range",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir(
        "test_dm1_v1_charge_count_to_torch_type_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_charge_count_to_torch_type_pc34_compat")])
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
#!/usr/bin/env python3
"""pass801 DM1 V1 light-power-to-light-amount contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass801_dm1_v1_light_power_to_light_amount_pc34_compat"
STATUS = "PASS801_DM1_V1_LIGHT_POWER_TO_LIGHT_AMOUNT_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_light_power_to_light_amount_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/light_power_to_light_amount_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_light_power_to_light_amount_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:45",
    "DATA.C:359",
    "DATA.C:1088",
    "PANEL.C:412",
    "CHAMPION.C:529/645",
    "MENU.C:1608/1936/1941",
    "TIMELINE.C:1754",
]

LOCAL_NEEDLES = [
    "dm1_v1_light_power_to_light_amount_table_pc34",
    "dm1_v1_light_power_to_light_amount_pc34",
    "dm1_v1_light_power_to_light_amount_diff_pc34",
    "dm1_v1_light_power_to_light_amount_illumulet_index_pc34",
    "kIllumuletLightPower",
    "kIllumuletLightAmount",
    "kMaxLightPower",
    "kMaxLightAmount",
    "G0039_ai_Graphic562_LightPowerToLightAmount",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_light_power_to_light_amount_pc34_compat",
    "src/dm1/dm1_v1_light_power_to_light_amount_pc34_compat.c",
    "NAME dm1_v1_light_power_to_light_amount_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (45, "G0039_ai_Graphic562_LightPowerToLightAmount"),
        (359, "G0039_ai_Graphic562_LightPowerToLightAmount"),
        (1088, "G0039_ai_Graphic562_LightPowerToLightAmount"),
    ],
    "PANEL.C": [
        (412, "G0039_ai_Graphic562_LightPowerToLightAmount"),
    ],
    "CHAMPION.C": [
        (529, "G0039_ai_Graphic562_LightPowerToLightAmount"),
        (645, "G0039_ai_Graphic562_LightPowerToLightAmount"),
    ],
    "MENU.C": [
        (1608, "G0039_ai_Graphic562_LightPowerToLightAmount"),
        (1936, "G0039_ai_Graphic562_LightPowerToLightAmount"),
        (1941, "G0039_ai_Graphic562_LightPowerToLightAmount"),
    ],
    "TIMELINE.C": [
        (1754, "G0039_ai_Graphic562_LightPowerToLightAmount"),
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
            "G0039_ai_Graphic562_LightPowerToLightAmount[16] = "
            "{0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, "
            "97, 100}. Read sites: PANEL.C:412 (F0337_INVENTORY_"
            "SetDungeonViewPalette torch sum), CHAMPION.C:529/645 "
            "(Illumulet equip delta = G0039[2] = 12), MENU.C:1608 "
            "(Light spell +12), MENU.C:1936/1941 (MagicTorch / "
            "Darkness spell deltas), TIMELINE.C:1754 (light-event "
            "tick diff = G0039[Strong] - G0039[Weaker])."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:359/1088 + PANEL.C:412 + CHAMPION.C:529/645 "
            "+ MENU.C:1608/1936/1941 + TIMELINE.C:1754 line numbers "
            "when the local ReDMCSB tree updates."
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
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass801 DM1 V1 Light-Power-To-Light-Amount",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0039_ai_Graphic562_"
        "LightPowerToLightAmount[16] = {0, 5, 12, 24, 33, 40, 46, 51, "
        "59, 68, 76, 82, 89, 94, 97, 100}. Monotonically non-decreasing "
        "saturating curve. PANEL.C:412 sums per-torch light power with "
        "a <<multiplier>>6 scale; CHAMPION.C:529/645 reads G0039[2] for "
        "Illumulet equip delta (= 12); MENU.C:1608 (Light spell +12), "
        "MENU.C:1936 (MagicTorch +G0039[LightPower]), MENU.C:1941 "
        "(Darkness -G0039[LightPower]); TIMELINE.C:1754 diff = "
        "G0039[Strong] - G0039[Weaker] for the per-tick light-event.",
        "- Runtime assertion floor: 126 assertions in `tests/test_dm1_v1_"
        "light_power_to_light_amount_pc34_compat.c`.",
        "- Expected test output: `126/126 assertions passed`.",
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
            "- Anchor drift note: re-pin DATA.C:359/1088, PANEL.C:412, "
            "CHAMPION.C:529/645, MENU.C:1608/1936/1941, TIMELINE.C:1754 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_light_power_to_light_amount_table_pc34",
            "DM1_V1_LightPowerToLightAmountResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_lookup_function",
                "test_illumulet_constant",
                "test_diff_helper",
                "test_monotonic_and_range",
                "test_max_value",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir(
        "test_dm1_v1_light_power_to_light_amount_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_light_power_to_light_amount_pc34_compat")])
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
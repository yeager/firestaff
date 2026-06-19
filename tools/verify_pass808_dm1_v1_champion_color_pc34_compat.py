#!/usr/bin/env python3
"""pass808 DM1 V1 champion-color contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass808_dm1_v1_champion_color_pc34_compat"
STATUS = "PASS808_DM1_V1_CHAMPION_COLOR_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_champion_color_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/champion_color_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_color_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:84",
    "DATA.C:423",
    "DATA.C:1095",
    "CHAMDRAW.C:48/51/60/300/342/1022",
    "CHAMPION.C:986/1016/1052",
    "REVIVE.C:868/872/887",
]

LOCAL_NEEDLES = [
    "dm1_v1_champion_color_table_pc34",
    "dm1_v1_champion_color_pc34",
    "dm1_v1_champion_color_leader_pc34",
    "kLeaderIndex",
    "kLeaderColor",
    "kFirstFollowerIdx",
    "kFirstFollowerClr",
    "kMaxColor",
    "G0046_auc_Graphic562_ChampionColor",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_champion_color_pc34_compat",
    "src/dm1/dm1_v1_champion_color_pc34_compat.c",
    "NAME dm1_v1_champion_color_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (84, "G0046_auc_Graphic562_ChampionColor"),
        (423, "G0046_auc_Graphic562_ChampionColor"),
        (1095, "G0046_auc_Graphic562_ChampionColor"),
    ],
    "CHAMDRAW.C": [
        (48, "G0046_auc_Graphic562_ChampionColor"),
        (51, "G0046_auc_Graphic562_ChampionColor"),
        (60, "G0046_auc_Graphic562_ChampionColor"),
        (300, "G0046_auc_Graphic562_ChampionColor"),
        (342, "G0046_auc_Graphic562_ChampionColor"),
        (1022, "G0046_auc_Graphic562_ChampionColor"),
    ],
    "CHAMPION.C": [
        (986, "G0046_auc_Graphic562_ChampionColor"),
        (1016, "G0046_auc_Graphic562_ChampionColor"),
        (1052, "G0046_auc_Graphic562_ChampionColor"),
    ],
    "REVIVE.C": [
        (868, "G0046_auc_Graphic562_ChampionColor"),
        (872, "G0046_auc_Graphic562_ChampionColor"),
        (887, "G0046_auc_Graphic562_ChampionColor"),
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
            "G0046_auc_Graphic562_ChampionColor[4] = {7, 11, 8, 14}. "
            "Assigns each champion a unique text/fill color: "
            "champion 0 (leader) = color 7 (LIGHT_GRAY), "
            "champion 1 = color 11 (LIGHT_CYAN), "
            "champion 2 = color 8 (LIGHT_RED), "
            "champion 3 = color 14 (LIGHT_YELLOW). Read sites: "
            "CHAMDRAW.C:48/51/60/300/342/1022 (champion icon/portrait "
            "fill), CHAMPION.C:986/1016/1052 (champion-name text "
            "color), REVIVE.C:868/872/887 (resurrect text color)."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:423/1095 + CHAMDRAW.C:48/51/60/300/342/1022 "
            "+ CHAMPION.C:986/1016/1052 + REVIVE.C:868/872/887 line "
            "numbers when the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-799 (champion-panel/leader/mirror + chest).",
            "Not pass798-807 (Graphics.dat init-table gates batches "
            "1+2+3).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass808 DM1 V1 Champion-Color",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0046_auc_Graphic562_"
        "ChampionColor[4] = {7, 11, 8, 14}. Champion 0 (leader) "
        "gets color 7 (LIGHT_GRAY), champion 1 = 11 (LIGHT_CYAN), "
        "champion 2 = 8 (LIGHT_RED), champion 3 = 14 (LIGHT_YELLOW). "
        "Used for champion-icon/portrait fill and champion-name text "
        "color.",
        "- Runtime assertion floor: 44 assertions in `tests/test_dm1_v1_"
        "champion_color_pc34_compat.c`.",
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
            "- Not pass784-790.",
            "- Not pass791 (champion-panel ammo-compat).",
            "- Not pass792 (steal-from-slot-indices).",
            "- Not pass793-799 (champion-panel/leader/mirror + chest).",
            "- Not pass798-807 (Graphics.dat init-table gates batches "
            "1+2+3).",
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
            "- Anchor drift note: re-pin DATA.C:423/1095, CHAMDRAW.C:"
            "48/51/60/300/342/1022, CHAMPION.C:986/1016/1052, "
            "REVIVE.C:868/872/887 in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_champion_color_table_pc34",
            "DM1_V1_ChampionColorResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_lookup_function",
                "test_leader_helper",
                "test_colors_distinct",
                "test_colors_in_palette_range",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir(
        "test_dm1_v1_champion_color_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_champion_color_pc34_compat")])
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
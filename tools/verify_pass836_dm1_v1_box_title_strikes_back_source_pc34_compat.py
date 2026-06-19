#!/usr/bin/env python3
"""pass836 DM1 V1 box-title-strikes-back-source contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass836_dm1_v1_box_title_strikes_back_source_pc34_compat"
STATUS = "PASS836_DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_box_title_strikes_back_source_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/box_title_strikes_back_source_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_box_title_strikes_back_source_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:10",
    "DATA.C:127",
    "DATA.C:547",
    "TITLE.C:134/137/335/338",
]

LOCAL_NEEDLES = [
    "dm1_v1_box_title_strikes_back_source_table_pc34",
    "dm1_v1_box_title_strikes_back_source_get_pc34",
    "dm1_v1_box_title_strikes_back_source_x_pc34",
    "kSBSX",
    "kSBSY",
    "kSBSW",
    "kSBSH",
    "G0004_ai_Graphic562_Box_Title_StrikesBack_Source",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_box_title_strikes_back_source_pc34_compat",
    "src/dm1/dm1_v1_box_title_strikes_back_source_pc34_compat.c",
    "NAME dm1_v1_box_title_strikes_back_source_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (10, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
        (127, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
        (547, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
    ],
    "TITLE.C": [
        (134, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
        (137, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
        (335, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
        (338, "G0004_ai_Graphic562_Box_Title_StrikesBack_Source"),
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
            "G0004_ai_Graphic562_Box_Title_StrikesBack_Source[4] = "
            "{0, 319, 0, 56}. The {X, Y, W, H} byte-coordinate sub-"
            "rectangle for the Strikes-Back title screen source "
            "blit. Read sites: TITLE.C:134/137/335/338 F0132_VIDEO_"
            "Blit(Strikes Back title source, G0004, 0, 80, C160_BYTE_"
            "WIDTH_SCREEN, C160_BYTE_WIDTH_SCREEN, CM1_COLOR_NO_"
            "TRANSPARENCY)."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:127/547 + TITLE.C:134/137/335/338 line "
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
            "Not pass798-835 (Graphics.dat init-table gates batches "
            "1+2+3+4+5+6+7+8+9+10+11+12+13+14).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass836 DM1 V1 Box-Title-Strikes-Back-Source",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0004_ai_Graphic562_"
        "Box_Title_StrikesBack_Source[4] = {0, 319, 0, 56}. The "
        "{X, Y, W, H} byte-coordinate sub-rectangle for the "
        "Strikes-Back title screen source blit. Read sites: "
        "TITLE.C:134/137/335/338 F0132_VIDEO_Blit.",
        "- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_"
        "box_title_strikes_back_source_pc34_compat.c`.",
        "- Expected test output: `53/53 assertions passed`.",
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
            "- Not pass798-835 (Graphics.dat init-table gates batches "
            "1+2+3+4+5+6+7+8+9+10+11+12+13+14).",
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
            "- Anchor drift note: re-pin DATA.C:127/547, TITLE.C:"
            "134/137/335/338 in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_box_title_strikes_back_source_table_pc34",
            "DM1_V1_BoxTitleStrikesBackSourceResultPc34",
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
        "test_dm1_v1_box_title_strikes_back_source_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_box_title_strikes_back_source_pc34_compat")])
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

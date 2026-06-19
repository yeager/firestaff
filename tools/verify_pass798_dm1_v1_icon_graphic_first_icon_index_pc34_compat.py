#!/usr/bin/env python3
"""pass798 DM1 V1 icon-graphic-first-icon-index contract."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass798_dm1_v1_icon_graphic_first_icon_index_pc34_compat"
STATUS = "PASS798_DM1_V1_ICON_GRAPHIC_FIRST_ICON_INDEX_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_icon_graphic_first_icon_index_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/icon_graphic_first_icon_index_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_icon_graphic_first_icon_index_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DATA.C:32",
    "DATA.C:253-260",
    "OBJECT.C:312-319",
    "OBJECT.C:455-467",
    "OBJECT.C:521",
]

LOCAL_NEEDLES = [
    "dm1_v1_icon_graphic_first_icon_index_table_pc34",
    "dm1_v1_icon_graphic_first_icon_index_pc34",
    "dm1_v1_icon_graphic_first_icon_index_resolve_pc34",
    "dm1_v1_icon_graphic_first_icon_index_block_size_pc34",
    "kIconBlockSize",
    "kIconGraphCount",
    "kFirstBlockStart",
    "kLastBlockStart",
    "G0026_ai_Graphic562_IconGraphicFirstIconIndex",
    "Disjoint from pass784-790",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_icon_graphic_first_icon_index_pc34_compat",
    "src/dm1/dm1_v1_icon_graphic_first_icon_index_pc34_compat.c",
    "NAME dm1_v1_icon_graphic_first_icon_index_pc34_compat",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DATA.C": [
        (32, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (253, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (260, "192"),
    ],
    "OBJECT.C": [
        (315, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (319, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (458, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (467, "G0026_ai_Graphic562_IconGraphicFirstIconIndex"),
        (521, "G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex].IconIndex"),
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
            "G0026_ai_Graphic562_IconGraphicFirstIconIndex[7] = "
            "{0, 32, 64, 96, 128, 160, 192} — the 32-icon stride "
            "between graphics #42..#48. OBJECT.C F0489_MEMORY_"
            "GetNativeBitmapOrGraphicIcon walks G0026 in a 7-iteration "
            "loop (OBJECT.C:312-319 and OBJECT.C:455-467) to find the "
            "icon-graphic block + within-block offset for each icon "
            "index. Disjoint from pass784-792 + pass793-796."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DATA.C:253-260 + OBJECT.C:312-521 line numbers when "
            "the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not pass784-790 (mirror-candidate C040 + wound).",
            "Not pass791 (champion-panel ammo-compat).",
            "Not pass792 (steal-from-slot-indices).",
            "Not pass793-796 (champion-panel/leader/mirror).",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass798 DM1 V1 Icon-Graphic-First-Icon-Index",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: Graphics.dat item 562 init var G0026_ai_Graphic562_"
        "IconGraphicFirstIconIndex[7] = {0, 32, 64, 96, 128, 160, 192}. "
        "OBJECT.C F0489 walks the table in a 7-iteration loop "
        "(OBJECT.C:312-319 and OBJECT.C:455-467) to find the icon-"
        "graphic block + within-block offset for each icon index. "
        "Stride is 32 (one graphic block); 7 blocks cover 224 icons.",
        "- Runtime assertion floor: 86 assertions in `tests/test_dm1_v1_"
        "icon_graphic_first_icon_index_pc34_compat.c`.",
        "- Expected test output: `86/86 assertions passed`.",
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
            "- Anchor drift note: re-pin DATA.C:253 and OBJECT.C:315 "
            "in next source refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_icon_graphic_first_icon_index_table_pc34",
            "DM1_V1_IconGraphicFirstIconIndexResultPc34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_table_values",
                "test_block_size",
                "test_lookup_function",
                "test_resolve_function",
                "test_resolve_null_safety",
                "test_run_accepted",
                "assertions passed",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir(
        "test_dm1_v1_icon_graphic_first_icon_index_pc34_compat"
    )
    runs = [
        run([str(build_dir / "test_dm1_v1_icon_graphic_first_icon_index_pc34_compat")])
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

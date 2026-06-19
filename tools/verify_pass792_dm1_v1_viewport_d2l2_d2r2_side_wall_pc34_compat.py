#!/usr/bin/env python3
"""pass792 DM1 V1 viewport D2L2/D2R2 side-wall (F0678/F0679) gate."""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass792_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat"
STATUS = "PASS792_DM1_V1_VIEWPORT_D2L2_D2R2_SIDE_WALL_LOCKED"
SRC = ROOT / "src/dm1/dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.c"
HDR = ROOT / "include/firestaff/dm1/v1/viewport/d2l2_d2r2_side_wall_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)

ANCHORS = [
    "DUNVIEW.C:6837-6865",
    "DUNVIEW.C:6868-6896",
    "DUNVIEW.C:8503-8508",
    "DEFS.H:2088",
    "DEFS.H:2605",
    "DEFS.H:2606",
    "DEFS.H:3428",
    "DEFS.H:3429",
    "DEFS.H:4047",
    "DEFS.H:4048",
]

LOCAL_NEEDLES = [
    "dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34",
    "dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34",
    "dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34",
    "dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34",
    "DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0678_PC34",
    "DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0679_PC34",
    "DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C00_WALL_PC34",
    "DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C05_TELEPORTER_PC34",
    "F0678_DrawD2L2",
    "F0679_DrawD2R2",
    "F0128",
    "PC_FIX_CODE_SIZE",
    "MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M",
    "side-row depth=2 lateral=-2",
    "lateral=+2",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat",
    "src/dm1/dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.c",
    "NAME dm1_v1_viewport_d2l2_d2r2_side_wall_source_lock",
    f"verify_{PASS}",
]

REDMCSB_WINDOWS = {
    "DUNVIEW.C": [
        (6854, "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"),
        (6864, "F0113_DUNGEONVIEW_DrawField"),
        (6885, "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"),
        (6895, "F0113_DUNGEONVIEW_DrawField"),
        (8504, "F0678_DrawD2L2"),
        (8508, "F0679_DrawD2R2"),
    ],
    "DEFS.H": [
        (2088, "C10_COLOR_FLESH"),
        (2605, "C09_VIEW_SQUARE"),
        (2606, "C10_VIEW_SQUARE"),
        (3428, "C05_WALL_D2R2"),
        (3429, "C06_WALL_D2L2"),
        (4047, "C707_ZONE_WALL_D2L2"),
        (4048, "C708_ZONE_WALL_D2R2"),
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
            "DM1 V1 viewport D2L2/D2R2 side-row (depth=2, lateral=+-2) "
            "F0678/F0679 dispatcher contract: F0678_DrawD2L2 (DUNVIEW."
            "C:6837-6865) and F0679_DrawD2R2 (DUNVIEW.C:6868-6896) each "
            "handle exactly two element cases (C00_ELEMENT_WALL via "
            "F0104/F0105 with G2107_WallSet[C06_WALL_D2L2+2]/[C05_WALL_"
            "D2R2+2] to C707/C708 zone; C05_ELEMENT_TELEPORTER via F0113 "
            "with C09/C10 field aspect to C707/C708). The F0128 caller "
            "(DUNVIEW.C:8503-8508) dispatches F0678 at relative (2,-2) and "
            "F0679 at (2,+2) AFTER F0118 (D3C) and BEFORE F0119 (D2L). The "
            "G0076 flipped branch is excluded on PC 3.4 (MEDIA720 path + "
            "PC_FIX_CODE_SIZE). Disjoint from the D2L2/D2R2 wall-set blit "
            "(F0104/F0105), F0107 wall-ornament alcove, F0108 wall/floor "
            "ornament composition, F0111 door-front pair, F0115 thing "
            "pass, stairs/pit dispatch, and the integrated D0L2/D0R2, "
            "D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, D3C viewport gates."
        ),
        "anchors": ANCHORS,
        "redmcsbRoot": str(RED),
        "sourceChecks": local_checks,
        "redmcsbLineWindowChecks": redmcsb_checks,
        "anchorDriftTodo": [
            "Re-pin DUNVIEW.C F0678/F0679/F0128 line numbers and DEFS.H "
            "C05_WALL_D2R2/C06_WALL_D2L2/C707_ZONE_WALL_D2L2/C708_ZONE_"
            "WALL_D2R2 line numbers when the local ReDMCSB tree updates."
        ]
        if drift
        else [],
        "verificationRuns": runs,
        "nonOverlap": [
            "Not the d2l2_d2r2 wall-set blit gate (F0104/F0105 native+"
            "flipped wallset into C707/C708 with the G2107 offset; this "
            "gate covers the dispatcher + F0128 caller only).",
            "Not the d2l2_d2r2 f0107 wall-ornament alcove gate.",
            "Not the d2l2_d2r2 f0108 wall-composition guard for the "
            "D2L/D2R front pair.",
            "Not the d2l2_d2r2 f0108 floor-ornament pixel contract.",
            "Not the d2l2_d2r2 f0108 floor+ceiling ornament two-pass.",
            "Not the d2l2_d2r2 f0111 door-front pair blit.",
            "Not the d2l2_d2r2 f0115 thing pass.",
            "Not the d2l2_d2r2 stairs/pit dispatch.",
            "Not the integrated D0L2/D0R2, D1C, D1L/D1R, D2C, D2L/D2R, "
            "D3L/D3R, D3C, or CSB-lineage viewport gates.",
            "Contract-only; no GRAPHICS.DAT or DUNGEON.DAT reads, no "
            "real-asset pixel parity, no original-DOS pixel parity.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# pass792 DM1 V1 Viewport D2L2/D2R2 Side-Wall",
        "",
        f"- Status: {manifest['status']}",
        "- Gate: F0678_DrawD2L2 (DUNVIEW.C:6837-6865) and F0679_DrawD2R2 "
        "(DUNVIEW.C:6868-6896) are the side-row (depth=2, lateral=+-2) "
        "wall dispatchers. Each handles exactly two element cases: "
        "C00_ELEMENT_WALL dispatches F0104_DrawFloorPitOrStairsBitmap "
        "with G2107_WallSet[C06_WALL_D2L2+2] / [C05_WALL_D2R2+2] (PC_FIX_"
        "CODE_SIZE) into C707_ZONE_WALL_D2L2 / C708_ZONE_WALL_D2R2 and "
        "returns; C05_ELEMENT_TELEPORTER dispatches F0113_DrawField with "
        "C09_VIEW_SQUARE_D2L2 / C10_VIEW_SQUARE_D2R2 field aspect to "
        "C707 / C708. No default case, no break after either case, no "
        "F0107/F0108/F0111/F0115 in body, no stairs/pit branch, no "
        "alcove/corridor/door route, no F0124 dispatch. F0128_DUNGEON"
        "VIEW_Draw_CPSF (DUNVIEW.C:8503-8508) dispatches F0678 at "
        "relative (2,-2) and F0679 at (2,+2), AFTER F0118 (D3C, "
        "DUNVIEW.C:8499) and BEFORE F0119 (D2L, DUNVIEW.C:8513).",
        "- Runtime assertion floor: 225 assertions in "
        "`tests/test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.c`.",
        "- Expected test output: `assertions=225 failures=0 hash=0xb1fce2ad`.",
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
            "- Not the d2l2_d2r2 wall-set blit gate (F0104/F0105).",
            "- Not the d2l2_d2r2 f0107 wall-ornament alcove gate.",
            "- Not the d2l2_d2r2 f0108 wall-composition / floor-ornament / "
            "floor-ceiling-ornament gates.",
            "- Not the d2l2_d2r2 f0111 door-front pair blit.",
            "- Not the d2l2_d2r2 f0115 thing pass.",
            "- Not the d2l2_d2r2 stairs/pit dispatch.",
            "- Not the integrated D0L2/D0R2, D1C, D1L/D1R, D2C, D2L/D2R, "
            "D3L/D3R, D3C, or CSB-lineage viewport gates.",
            "- Contract-only; no real-asset or original-DOS pixel parity.",
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
            "- Anchor drift note: re-pin DUNVIEW.C F0678/F0679/F0128 and "
            "DEFS.H C05_WALL_D2R2/C06_WALL_D2L2/C707/C708 in next source "
            "refresh."
        )
    lines.extend(["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`", ""])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34",
            "DM1_V1_D2L2D2R2SideWallDispatchModelPc34",
            "DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0678_PC34",
            "DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0679_PC34",
        ]),
        check_needles(
            "test_entry_and_assertions",
            TEST,
            [
                "test_model_core",
                "test_lanes_cases_dispatch_order",
                "test_sibling_rejects_and_evidence",
                "test_render_probes",
                "test_hash_stability",
                "assertions=%d failures=%d hash=0x%08x",
            ],
        ),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    redmcsb_checks = check_redmcsb_windows()
    build_dir = resolve_build_dir("test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat")
    runs = [
        run([str(build_dir / "test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat")])
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
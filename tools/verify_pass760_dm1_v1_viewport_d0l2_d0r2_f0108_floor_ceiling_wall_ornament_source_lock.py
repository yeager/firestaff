#!/usr/bin/env python3
"""Pass760 DM1 V1 D0L2/D0R2 F0108 wall+floor+ceiling+ornament source lock."""

from __future__ import annotations

import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

PASS = "pass760_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_wall_ornament_source_lock"
ROOT = Path(__file__).resolve().parents[1]
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR", ROOT / "build"))
TEST_BINARY = BUILD / "test_dm1_v1_viewport_3d_pc34_compat"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
MANIFEST = ROOT / "parity-evidence" / "verification" / PASS / "manifest.json"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

REQUIRED_ANCHORS = [
    "DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011",
    "DUNVIEW.C F0107:3502-3938",
    "DUNVIEW.C F0098:2962-3002",
    "DUNVIEW.C F0115:4547-4581,5180-5188,5211-5214,5668-5671",
    "DUNVIEW.C:6432-6600 M575_VIEW_WALL_D3L_RIGHT",
    "DEFS.H:2088 C10_COLOR_FLESH",
    "DEFS.H:2596-2611 I34E/P31J view-square ordinals",
    "DEFS.H:2668-2677 and 2698-2702 cell orders and M575..M579 view-square ordinals",
    "DEFS.H:4045-4046 C705/C706 wall zones",
    "DRAWVIEW.C F0097:1-50 wall-side dispatch",
    "DUNVIEW.C F0104:3113-3156 native C10 blit",
    "DUNVIEW.C F0105:3185-3247 flipped C10 blit",
]

LOCAL_CHECKS = [
    ("include/dm1_v1_viewport_3d_pc34_compat.h", "DM1_ViewportD0L2D0R2F0108CompositionSpec"),
    ("include/dm1_v1_viewport_3d_pc34_compat.h", "#define DM1_VIEW_SQUARE_D0L2"),
    ("include/dm1_v1_viewport_3d_pc34_compat.h", "#define DM1_VIEW_SQUARE_D0R2"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "s_d0l2_d0r2_f0108_composition_specs"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D0L2, DM1_VIEW_SQUARE_D0L"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D0R2, DM1_VIEW_SQUARE_D0R"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:3940-4011 F0108 floor ornament MASK0x8000 keepout"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:3502-3938 F0107 wall ornament ordinal/coordinateSet"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:2962-3002 F0098 floor+ceiling base"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:4547-4581,5180-5188,5211-5214,5668-5671 F0115"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DRAWVIEW.C F0097:1-50 wall-side dispatch"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "pass760_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_wall_ornament"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "pass760.count"),
    ("CMakeLists.txt", PASS),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def slice_contains(path: Path, start: int, end: int, needles: list[str]) -> dict:
    if not path.exists():
        return {"file": str(path), "lines": f"{start}-{end}", "ok": False, "missing": needles}
    lines = read(path).splitlines()
    body = "\n".join(lines[start - 1:end])
    missing = [needle for needle in needles if needle not in body]
    return {
        "file": str(path.relative_to(RED.parent) if path.is_relative_to(RED.parent) else path),
        "lines": f"{start}-{end}",
        "ok": not missing,
        "missing": missing,
    }


def local_checks() -> list[dict]:
    rows = []
    for rel, token in LOCAL_CHECKS:
        text = read(ROOT / rel)
        rows.append({"file": rel, "token": token, "ok": token in text})
    return rows


def redmcsb_checks() -> tuple[list[dict], list[str]]:
    dun = RED / "DUNVIEW.C"
    defs = RED / "DEFS.H"
    draw = RED / "DRAWVIEW.C"
    checks = [
        slice_contains(dun, 3940, 4011, ["F0108_DUNGEONVIEW_DrawFloorOrnament", "MASK0x8000_FOOTPRINTS", "C10_COLOR_FLESH"]),
        slice_contains(dun, 3502, 3938, ["F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF", "F0791_DUNGEONVIEW_DrawBitmapXX", "C10_COLOR_FLESH"]),
        slice_contains(dun, 2962, 3002, ["F0098_DUNGEONVIEW_DrawFloorAndCeiling", "G2109_Ceiling", "G2108_Floor"]),
        slice_contains(dun, 4547, 4581, ["F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "Objects are drawn"]),
        slice_contains(dun, 5180, 5188, ["C10_COLOR_FLESH"]),
        slice_contains(dun, 5211, 5214, ["G2033_ac_ViewSquareIndexTo"]),
        slice_contains(dun, 5668, 5671, ["G2028_ac_ViewSquareIndexTo"]),
        slice_contains(dun, 6432, 6600, ["M575_VIEW_WALL_D3L_RIGHT", "M576_VIEW_WALL_D3R_LEFT", "C705_ZONE_WALL_D3L", "C706_ZONE_WALL_D3R"]),
        slice_contains(defs, 2088, 2088, ["C10_COLOR_FLESH"]),
        slice_contains(defs, 2596, 2611, ["M609_VIEW_SQUARE_D0C", "M611_VIEW_SQUARE_D0R", "C15_VIEW_SQUARE_D3R2"]),
        slice_contains(defs, 2668, 2677, ["C0x0128_CELL_ORDER", "C0x4312_CELL_ORDER"]),
        slice_contains(defs, 2698, 2702, ["M575_VIEW_WALL_D3L_RIGHT", "M579_VIEW_WALL_D3R_FRONT"]),
        slice_contains(defs, 4045, 4046, ["C705_ZONE_WALL_D3L", "C706_ZONE_WALL_D3R"]),
        slice_contains(dun, 3113, 3156, ["F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap", "C10_COLOR_FLESH"]),
        slice_contains(dun, 3185, 3247, ["F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally", "MASK0x0001_FLIP_HORIZONTAL", "C10_COLOR_FLESH"]),
    ]
    drift_todos = []
    draw_f0097_requested = slice_contains(draw, 1, 50, ["F0097_DUNGEONVIEW_DrawViewport"])
    if not draw_f0097_requested["ok"]:
        actual = "unknown"
        for n, line in enumerate(read(draw).splitlines(), start=1):
            if "F0097_DUNGEONVIEW_DrawViewport" in line:
                actual = str(n)
                break
        drift_todos.append(
            "Required anchor DRAWVIEW.C F0097:1-50 appears drifted in local ReDMCSB; "
            f"F0097_DUNGEONVIEW_DrawViewport is at line {actual}. Anchor text was not changed."
        )
    checks.append(draw_f0097_requested)
    return checks, drift_todos


def run_test() -> dict:
    if not TEST_BINARY.exists():
        return {
            "command": [str(TEST_BINARY)],
            "returncode": 127,
            "passed": False,
            "pass760Assertions": 0,
            "pass760Failures": 1,
            "outputTail": "test binary missing",
        }
    proc = subprocess.run([str(TEST_BINARY)], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = proc.stdout.splitlines()
    return {
        "command": [str(TEST_BINARY)],
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "pass760Assertions": sum(1 for line in lines if line.startswith("PASS pass760.")),
        "pass760Failures": sum(1 for line in lines if line.startswith("FAIL pass760.")),
        "outputTail": "\n".join(lines[-20:]),
    }


def write_report(payload: dict) -> None:
    status = payload["status"]
    test = payload["testRun"]
    lines = [
        f"# {PASS}",
        "",
        f"Status: `{status}`",
        "",
        "## Scope",
        "",
        "- DM1 V1 D0L2/D0R2 combined wall + floor + ceiling + ornament source-lock metadata.",
        "- Non-duplicative with pass729 D0L/D0R F0108, pass733 D0C F0108, and pass717 D1L2/D1R2 wall composition gates.",
        "- Contract-only metadata gate; no original-vs-Firestaff pixel parity and no game-data load.",
        "",
        "## Required ReDMCSB Anchors",
        "",
    ]
    lines.extend(f"- {anchor}" for anchor in REQUIRED_ANCHORS)
    lines += [
        "",
        "## Verification",
        "",
        f"- C test: `{test['command'][0]}`",
        f"- pass760 assertions: {test['pass760Assertions']} pass / {test['pass760Failures']} fail",
        f"- Local checks: {sum(1 for row in payload['localChecks'] if row['ok'])}/{len(payload['localChecks'])}",
        f"- Source checks: {sum(1 for row in payload['redmcsbChecks'] if row['ok'])}/{len(payload['redmcsbChecks'])}",
        "",
        "## Drift TODOs",
        "",
    ]
    if payload["anchorDriftTodos"]:
        lines.extend(f"- {todo}" for todo in payload["anchorDriftTodos"])
    else:
        lines.append("- None.")
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    local = local_checks()
    red, drift_todos = redmcsb_checks()
    test = run_test()
    ok = (
        test["passed"]
        and test["pass760Assertions"] >= 80
        and test["pass760Failures"] == 0
        and all(row["ok"] for row in local)
        and all(row["ok"] for row in red if row["lines"] != "1-50")
    )
    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS" if ok else "FAIL",
        "pass": 760,
        "requiredAnchors": REQUIRED_ANCHORS,
        "localChecks": local,
        "redmcsbChecks": red,
        "anchorDriftTodos": drift_todos,
        "testRun": test,
        "nonClaims": [
            "No original DOS pixel parity is claimed.",
            "No real game data is loaded by this gate.",
            "The D0L2/D0R2 rows are intentionally separate from pass729/pass733/pass717 metadata.",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(payload)
    print(
        f"{PASS}: {payload['status']} pass760_assertions="
        f"{test['pass760Assertions']} failures={test['pass760Failures']} "
        f"manifest={MANIFEST.relative_to(ROOT)}"
    )
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass517_dm1_v1_d3_d2_field_occlusion_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass517_dm1_v1_d3_d2_field_occlusion_source_lock.md"
STATUS = "PASS517_DM1_V1_D3_D2_FIELD_OCCLUSION_SOURCE_LOCKED"

CHECKS = [
    ("D3R", "6514-6638", [
        "case C19_ELEMENT_STAIRS_FRONT:",
        "goto T0117016;",
        "case C02_ELEMENT_PIT:",
        "case C05_ELEMENT_TELEPORTER:",
        "case C01_ELEMENT_CORRIDOR:",
        "L0202_i_Order = C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT;",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "if (L0203_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
        "F0113_DUNGEONVIEW_DrawField",
        "C706_ZONE_WALL_D3R",
    ]),
    ("D2L", "6914-7048", [
        "case C19_ELEMENT_STAIRS_FRONT:",
        "goto T0119018;",
        "case C02_ELEMENT_PIT:",
        "case C05_ELEMENT_TELEPORTER:",
        "case C01_ELEMENT_CORRIDOR:",
        "L0207_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0112_DUNGEONVIEW_DrawCeilingPit",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "if (L0208_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
        "F0113_DUNGEONVIEW_DrawField",
        "C710_ZONE_WALL_D2L",
    ]),
    ("D2R", "7065-7240", [
        "case C19_ELEMENT_STAIRS_FRONT:",
        "goto T0120027;",
        "case C02_ELEMENT_PIT:",
        "case C05_ELEMENT_TELEPORTER:",
        "case C01_ELEMENT_CORRIDOR:",
        "L0209_i_Order = C0x4312_CELL_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT;",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0112_DUNGEONVIEW_DrawCeilingPit",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "if (L0210_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
        "F0113_DUNGEONVIEW_DrawField",
        "C711_ZONE_WALL_D2R",
    ]),
]

LOCAL_NEEDLES = [
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D3R, 0x4312"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:6624-6638 teleporter field after F0115"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D2L, 0x3421"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:7033-7048 teleporter field after F0115"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D2R, 0x4312"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:7226-7240 teleporter field after F0115"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "floor_field_order_spec_count(), 13"),
]


def slice_lines(text: str, spec: str) -> tuple[int, str]:
    start, end = (int(x) for x in spec.split("-", 1))
    return start, "\n".join(text.splitlines()[start - 1:end])


def ordered_hits(body: str, base: int, needles: list[str]) -> tuple[list[dict[str, object]], list[str]]:
    cursor = 0
    hits = []
    missing = []
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": base + body.count("\n", 0, pos), "needle": needle})
            cursor = pos + len(needle)
    return hits, missing


def run(cmd: list[str]) -> dict[str, object]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-10:])}


def main() -> int:
    red_text = RED.read_text(encoding="latin-1", errors="replace")
    red_rows = []
    for square, lines, needles in CHECKS:
        base, body = slice_lines(red_text, lines)
        hits, missing = ordered_hits(body, base, needles)
        red_rows.append({"square": square, "source": f"DUNVIEW.C:{lines}", "status": "PASS" if not missing else "FAIL", "hits": hits, "missing": missing})
    local_rows = []
    for rel, needle in LOCAL_NEEDLES:
        text = (ROOT / rel).read_text(encoding="utf-8", errors="replace")
        local_rows.append({"file": rel, "needle": needle, "status": "PASS" if needle in text else "FAIL"})
    runtime = run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")])
    ok = all(r["status"] == "PASS" for r in red_rows + local_rows) and runtime["passed"]
    manifest = {
        "schema": "pass517_dm1_v1_d3_d2_field_occlusion_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS517_DM1_V1_D3_D2_FIELD_OCCLUSION_SOURCE_LOCK",
        "redmcsbRoot": str(RED.parent),
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [runtime],
        "nonClaims": ["no input or movement queue edits", "no original DOS pixel parity claim", "no DANNESBURK use"],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    report = ["# Pass517 DM1 V1 D3/D2 field occlusion source lock", "", f"Status: {manifest['status']}", "", "## Primary ReDMCSB Evidence"]
    for row in red_rows:
        report += ["", f"- {row['status']} {row['square']} {row['source']}"]
        report += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        report += [f"  - missing: {miss}" for miss in row["missing"]]
    report += ["", "## Verification", "", f"- {' '.join(runtime['command'])}: rc={runtime['returncode']}", "~~~", runtime["outputTail"], "~~~", "", "## Non-Claims", "", "- No input or movement queue edits.", "- No original DOS pixel parity claim.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

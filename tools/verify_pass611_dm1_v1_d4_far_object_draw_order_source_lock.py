#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR", ROOT / "build"))
TEST_BINARY = BUILD / "test_dm1_v1_viewport_3d_pc34_compat"
MANIFEST = ROOT / "parity-evidence/verification/pass611_dm1_v1_d4_far_object_draw_order_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass611_dm1_v1_d4_far_object_draw_order_source_lock.md"
STATUS = "PASS611_DM1_V1_D4_FAR_OBJECT_DRAW_ORDER_SOURCE_LOCKED"

SOURCE_CHECKS = [
    ("dunview-f0128-d4-before-d3", "DUNVIEW.C", "8466-8482", [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
        "M598_VIEW_SQUARE_D4L, C0x0001_CELL_ORDER_BACKLEFT);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 1",
        "M599_VIEW_SQUARE_D4R, C0x0001_CELL_ORDER_BACKLEFT);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 0",
        "M597_VIEW_SQUARE_D4C, C0x0001_CELL_ORDER_BACKLEFT);",
        "F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
    ]),
    ("dunview-f0115-cell-layer-contract", "DUNVIEW.C", "4547-4582", [
        "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
        "If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.",
        "The remaining nibbles contain ordinals of square view cells to draw",
        "Draw only explosions at specified cell, except for Fluxcages",
    ]),
    ("drawview-present-boundary", "DRAWVIEW.C", "709-858", [
        "void F0097_DUNGEONVIEW_DrawViewport(",
        "G0324_B_DrawViewportRequested = C1_TRUE",
        "M526_WaitVerticalBlank();",
        "F0638_GetZone(C007_ZONE_VIEWPORT, L2413_ai_Box);",
        "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
    ]),
    ("command-viewport-zone-and-queue", "COMMAND.C", "396-405", [
        "G0448_as_Graphic561_SecondaryMouseInput_Movement",
        "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
        "C007_ZONE_VIEWPORT",
    ]),
    ("command-queue-mutation-before-redraw", "COMMAND.C", "2045-2156", [
        "void F0380_COMMAND_ProcessQueue_CPSC(",
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ]),
    ("movesens-fall-redraw-caveat", "MOVESENS.C", "316-356", [
        "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE(",
        "P0558_i_SourceMapX",
        "P0560_i_DestinationMapX",
    ]),
    ("movesens-f0128-while-falling-blocker", "MOVESENS.C", "548-558", [
        "F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF();",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        "BUG0_28",
    ]),
    ("defs-pc34-d4-square-ids", "DEFS.H", "2595-2615", [
        "#define M597_VIEW_SQUARE_D4C 16",
        "#define M598_VIEW_SQUARE_D4L 17",
        "#define M599_VIEW_SQUARE_D4R 18",
    ]),
    ("defs-viewport-zone-id", "DEFS.H", "3748-3756", ["#define C007_ZONE_VIEWPORT"]),
    ("coord-zone-clip-contract", "COORD.C", "2389-2410", [
        "#ifdef MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J",
        "M708_ZONE_WIDTH(P2130_pi_XYZ)",
        "M709_ZONE_HEIGHT(P2130_pi_XYZ)",
        "return NULL;",
    ]),
]

LOCAL_CHECKS = [
    ("far-object-struct", ROOT / "include/dm1_v1_viewport_3d_pc34_compat.h", "DM1_ViewportFarObjectPassSpec"),
    ("far-object-table", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "s_far_object_pass_specs"),
    ("far-object-d4l", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D4L, 4, -1, 0x0001, 17"),
    ("far-object-d4r", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D4R, 4,  1, 0x0001, 18"),
    ("far-object-d4c", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D4C, 4,  0, 0x0001, 16"),
    ("far-object-test", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "test_f0128_d4_far_object_pass_order"),
    ("far-object-evidence", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:8466-8477 F0128 D4 far-object passes"),
]


def source_window(path: Path, span: str) -> tuple[int, str]:
    first, last = [int(part) for part in span.split("-", 1)]
    text = path.read_text(encoding="latin-1", errors="replace")
    return first, "\n".join(text.splitlines()[first - 1:last])


def ordered_hits(path: Path, span: str, needles: list[str]) -> dict:
    first, text = source_window(path, span)
    cursor = 0
    hits = []
    missing = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"line": first + text.count("\n", 0, pos), "needle": needle})
        cursor = pos + len(needle)
    return {"status": "PASS" if not missing else "FAIL", "hits": hits, "missing": missing}


def run(cmd: list[str], env: dict[str, str] | None = None) -> dict:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    return {"command": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-16:])}


def main(check_only: bool = False) -> int:
    red_rows = []
    for ident, filename, span, needles in SOURCE_CHECKS:
        row = ordered_hits(RED / filename, span, needles)
        row.update({"id": ident, "sourceFile": filename, "lines": span})
        red_rows.append(row)
    local_rows = []
    for ident, path, needle in LOCAL_CHECKS:
        text = path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
        local_rows.append({"id": ident, "file": str(path.relative_to(ROOT)), "needle": needle, "status": "PASS" if needle in text else "FAIL"})
    failures = [row["id"] for row in red_rows + local_rows if row["status"] != "PASS"]
    if check_only:
        print("PASS pass611 check-only" if not failures else "FAIL pass611 check-only: " + ",".join(failures))
        return 0 if not failures else 1
    runtime = run([str(TEST_BINARY)])
    check_run = run([sys.executable, str(Path(__file__).resolve()), "--check-only"], env={**os.environ, "FIRESTAFF_BUILD_DIR": str(BUILD)})
    ok = not failures and runtime["passed"] and check_run["passed"]
    manifest = {
        "schema": "pass611_dm1_v1_d4_far_object_draw_order_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS611_DM1_V1_D4_FAR_OBJECT_DRAW_ORDER_SOURCE_LOCK",
        "buildDir": str(BUILD),
        "redmcsbRoot": str(RED),
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [runtime, check_run],
        "nonClaims": [
            "No renderer pixel parity claim.",
            "No original PC34 same-run transcript/frame binding claim.",
            "No movement queue, viewport capture, audible C006, or item panel behavior changed.",
            "No CSB, DM2, or Nexus behavior claim.",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass611 DM1 V1 D4 far-object draw-order source lock", "", f"Status: {manifest['status']}", "",
        "Claim: ReDMCSB F0128 draws D4L, D4R, and D4C as direct far-object F0115 passes with cell order 0x0001 before any D3/D2/D1/D0 wall helper runs; this is a deterministic source-lock gate only, not a pixel-parity promotion.", "", "## Primary ReDMCSB Evidence",
    ]
    for row in red_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})")
        for hit in row["hits"]:
            lines.append(f"  - line {hit['line']}: {hit['needle']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Firestaff Evidence"]
    for row in local_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['file']})")
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += [f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims"] + [f"- {item}" for item in manifest["nonClaims"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))

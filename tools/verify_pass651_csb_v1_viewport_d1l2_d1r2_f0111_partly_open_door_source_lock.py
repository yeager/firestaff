#!/usr/bin/env python3
"""Pass651: source-lock CSB V1 D1L2/D1R2 F0111 partly-open door dispatch."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
LINEAGE = Path("/Users/bosse/.openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp")
PASS = "pass651_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_lock"
STATUS = "PASS651_CSB_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SOURCE_LOCKED"
MANIFEST = ROOT / "parity-evidence" / "verification" / PASS / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

SOURCE_CHECKS = [
    ("redmcsb-f0111-partly-open-horizontal", RED / "DUNVIEW.C", "4218-4337", [
        "STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor(",
        "if (P0125_ui_DoorState != C0_DOOR_STATE_OPEN)",
        "P0125_ui_DoorState--;",
        "P0129_ps_DoorFrames->LeftHorizontal[P0125_ui_DoorState]",
        "P0129_ps_DoorFrames->RightHorizontal[P0125_ui_DoorState]",
        "P2084_i_ZoneIndex += P0125_ui_DoorState;",
        "P2084_i_ZoneIndex + C6_UNKNOWN",
        "F0654_Call_F0132_VIDEO_Blit",
        "P2084_i_ZoneIndex += (unsigned int16_t)(3 | MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR);",
        "F0791_DUNGEONVIEW_DrawBitmapXX",
        "C10_COLOR_FLESH",
    ]),
    ("redmcsb-f0122-d1l-body-door-front", RED / "DUNVIEW.C", "7391-7557", [
        "STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L(",
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L);",
        "F0111_DUNGEONVIEW_DrawDoor(L0214_ai_SquareAspect[M557_DOOR_THING_INDEX], L0214_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M630_ZONE_DOOR_D1L);",
        "L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, L0213_i_Order);",
    ]),
    ("redmcsb-f0123-d1r-body-door-front", RED / "DUNVIEW.C", "7559-7725", [
        "STATICFUNCTION void F0123_DUNGEONVIEW_DrawSquareD1R(",
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_FRAME_TOP_D1R);",
        "F0111_DUNGEONVIEW_DrawDoor(L0216_ai_SquareAspect[M557_DOOR_THING_INDEX], L0216_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M632_ZONE_DOOR_D1R);",
        "L0215_i_Order = C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, L0215_i_Order);",
    ]),
    ("redmcsb-f0128-d1-dispatch-f0127-followup", RED / "DUNVIEW.C", "8524-8542", [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1, &L0224_i_MapX, &L0225_i_MapY);",
        "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1, &L0224_i_MapX, &L0225_i_MapY);",
        "F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
    ]),
    ("redmcsb-f0127-object-pass-boundary", RED / "DUNVIEW.C", "8288-8296", [
        "F0112_DUNGEONVIEW_DrawCeilingPit(C069_GRAPHIC_CEILING_PIT_D0C, C871_ZONE_CEILING_PIT_D0C",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);",
    ]),
    ("redmcsb-defs-door-zone-baselines", RED / "DEFS.H", "2088-4260", [
        "#define C10_COLOR_FLESH           10",
        "#define M607_VIEW_SQUARE_D1L  4",
        "#define M608_VIEW_SQUARE_D1R  5",
        "#define C09_VIEW_SQUARE_D2L2  9",
        "#define C10_VIEW_SQUARE_D2R2 10",
        "#define C6_UNKNOWN",
        "#define MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR 0x4000",
        "#define C707_ZONE_WALL_D2L2",
        "#define C708_ZONE_WALL_D2R2",
        "#define C713_ZONE_WALL_D1L",
        "#define C714_ZONE_WALL_D1R",
        "#define M630_ZONE_DOOR_D1L",
        "#define M632_ZONE_DOOR_D1R",
    ]),
    ("csb-lineage-f1-door-dispatch", LINEAGE, "1903-1915", [
        "ui16 StdDrawF1DoorFacing[]",
        "StdDoorFacingFrameLeftBitmapF1",
        "StdDoorFacingFrameRightBitmapF1",
        "F1DoorRecordIndex, F1DoorState, StdDoorGraphicsF1, StdDoorRectsF1",
        "StdDrawDoor",
        "F1Contents,  F1xy, F1, DrawOrder349,  StdDrawRoomObjects",
    ]),
]

LOCAL_CHECKS = [
    ("header-public-include", ROOT / "include/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.h", "1-210", [
        "f0128_dispatch_order",
        "f0127_followup_order",
        "left_horizontal_frame_bitmap",
        "right_horizontal_frame_bitmap",
        "CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34",
        "csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34",
    ]),
    ("module-spec-and-zone-math", ROOT / "src/csb/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c", "1-380", [
        "ReDMCSB: DUNVIEW.C F0122 lines 7391-7557.",
        "CSB_F0128_D1L_ORDER = 13",
        "CSB_F0128_D1R_ORDER = 14",
        "CSB_F0127_ORDER = 18",
        "CSB_ZONE_DOOR_D1L = 3780",
        "CSB_ZONE_DOOR_D1R = 3800",
        "DUNVIEW.C:4218-4337",
        "D1L.LeftHorizontal",
        "D1R.RightHorizontal",
        "spec->door_zone_base + door_state + spec->first_half_dest_zone_offset",
        "spec->door_zone_base + door_state +",
        "spec->second_half_zone_offset | spec->second_half_zone_mask",
    ]),
    ("test-asserts-pass651-surface", ROOT / "tests/test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c", "1-520", [
        "test_f0128_dispatch_and_followup",
        "test_f0111_state_frame_and_blit_math",
        "frame.left.d1l2",
        "d1l2.first.state2",
        "d1r2.second.state2",
        "probe.second_half",
        "assertion_count_at_least_70",
    ]),
    ("cmake-registration", ROOT / "CMakeLists.txt", "80-2140", [
        "src/csb/csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.c",
        "add_executable(test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat",
        "NAME csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat",
    ]),
]


def read_span(path: Path, span: str) -> tuple[int, str]:
    first, last = [int(part) for part in span.split("-", 1)]
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    lines = path.read_text(encoding=encoding, errors="replace").splitlines()
    return first, "\n".join(lines[first - 1:last])


def ordered_hits(path: Path, span: str, needles: list[str]) -> dict:
    first, text = read_span(path, span)
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


def audit(rows: list[tuple[str, Path, str, list[str]]]) -> list[dict]:
    audited = []
    for ident, path, span, needles in rows:
        row = ordered_hits(path, span, needles)
        row.update({
            "id": ident,
            "file": str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path),
            "lines": span,
        })
        audited.append(row)
    return audited


def write_evidence(red_rows: list[dict], local_rows: list[dict]) -> dict:
    failures = [row["id"] for row in red_rows + local_rows if row["status"] != "PASS"]
    ok = not failures
    manifest = {
        "schema": "pass651_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS651_CSB_V1_VIEWPORT_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SOURCE_LOCK",
        "claim": "CSB V1 D1L2/D1R2 partly-open horizontal F0111 door dispatch is source-locked to ReDMCSB F0122/F0123 body calls, F0111 frame/zone/blit order, F0128 ordering, and F0127 follow-up.",
        "redmcsbRoot": str(RED),
        "lineageViewport": str(LINEAGE),
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "nonClaims": [
            "No renderer output or real-asset pixel parity claim.",
            "No game-data load or archive/materialization behavior change.",
            "No changes to the main CSB viewport module.",
            "No DM1, DM2, Nexus, or Theron behavior claim.",
        ],
    }

    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass651 CSB V1 D1L2/D1R2 F0111 partly-open door source lock",
        "",
        f"Status: {manifest['status']}",
        "",
        manifest["claim"],
        "",
        "## ReDMCSB And Lineage Evidence",
    ]
    for row in red_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['file']}:{row['lines']})")
        for hit in row["hits"]:
            lines.append(f"  - line {hit['line']}: {hit['needle']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Firestaff Evidence"]
    for row in local_rows:
        lines.append(f"- {row['status']} {row['id']} ({row['file']}:{row['lines']})")
        for hit in row["hits"]:
            lines.append(f"  - line {hit['line']}: {hit['needle']}")
        for missing in row["missing"]:
            lines.append(f"  - missing: {missing}")
    lines += ["", "## Non-Claims"] + [f"- {item}" for item in manifest["nonClaims"]]
    lines += ["", f"Manifest: parity-evidence/verification/{PASS}/manifest.json"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check-only", action="store_true",
                        help="audit anchors and write pass651 evidence")
    parser.parse_args()

    red_rows = audit(SOURCE_CHECKS)
    local_rows = audit(LOCAL_CHECKS)
    manifest = write_evidence(red_rows, local_rows)
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if manifest["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())

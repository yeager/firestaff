#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
TEST_BINARY = ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat"
MANIFEST = ROOT / "parity-evidence/verification/pass608_dm1_v1_d0c_current_square_occlusion_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass608_dm1_v1_d0c_current_square_occlusion_source_lock.md"
STATUS = "PASS608_DM1_V1_D0C_CURRENT_SQUARE_OCCLUSION_SOURCE_LOCKED"


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def slice_text(path: Path, span: str) -> tuple[int, str]:
    first, last = (int(part) for part in span.split("-", 1))
    lines = read_text(path).splitlines()
    return first, "\n".join(lines[first - 1:last])


def ordered_hits(body: str, base: int, needles: list[str]) -> tuple[list[dict[str, object]], list[str]]:
    cursor = 0
    hits: list[dict[str, object]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"line": base + body.count("\n", 0, pos), "needle": needle})
        cursor = pos + len(needle)
    return hits, missing


def check_region(ident: str, path: Path, span: str, claim: str, needles: list[str]) -> dict[str, object]:
    base, body = slice_text(path, span)
    hits, missing = ordered_hits(body, base, needles)
    return {
        "id": ident,
        "file": str(path),
        "source": f"{path.name}:{span}",
        "claim": claim,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def check_file(ident: str, rel: str, needles: list[str]) -> dict[str, object]:
    path = ROOT / rel
    body = read_text(path)
    hits, missing = ordered_hits(body, 1, needles)
    return {
        "id": ident,
        "file": rel,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run(cmd: list[str]) -> dict[str, object]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-12:]),
    }


REDMCSB_CHECKS = [
    (
        "f0128_d0_side_then_d0c_current_square",
        RED / "DUNVIEW.C",
        "8534-8542",
        "F0128 draws D0L and D0R before the current-square D0C pass.",
        [
            "F0125_DUNGEONVIEW_DrawSquareD0L",
            "F0126_DUNGEONVIEW_DrawSquareD0R",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
    ),
    (
        "f0127_d0c_door_side_foreground_before_common_f0115",
        RED / "DUNVIEW.C",
        "8184-8240",
        "D0C door-side foreground frame work completes and breaks before the common F0115 current-square content pass.",
        [
            "F0172_DUNGEON_SetSquareAspect(L0222_ai_SquareAspect, P0180_i_Direction, P0181_i_MapX, P0182_i_MapY);",
            "case C16_ELEMENT_DOOR_SIDE:",
            "if (G0407_s_Party.Event73Count_ThievesEye)",
            "F0616_CopyBitmap(F0631_GetBitmapPointer(G2116_DoorFrameFrontD0C), G0074_puc_Bitmap_Temporary);",
            "F0635_(NULL, L2496_ai_XYZ, C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME",
            "F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C, C10_COLOR_FLESH);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2116_DoorFrameFrontD0C, C728_ZONE_DOOR_FRAME_D0C);",
            "break;",
        ],
    ),
    (
        "f0127_d0c_stairs_pit_then_common_f0115_field",
        RED / "DUNVIEW.C",
        "8241-8308",
        "D0C stairs-front foreground breaks; pit falls through to ceiling, F0115 with C0x0021, then teleporter field in C715.",
        [
            "case C19_ELEMENT_STAIRS_FRONT:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C06_STAIRS_BITMAP_UP_FRONT_D0C_LEFT], C811_ZONE_STAIRS_UP_FRONT_D0L);",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G0079_ai_StairsNativeBitmapIndices[C06_STAIRS_BITMAP_UP_FRONT_D0C_LEFT], C812_ZONE_STAIRS_UP_FRONT_D0R);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C13_STAIRS_BITMAP_DOWN_FRONT_D0C_LEFT], C824_ZONE_STAIRS_DOWN_FRONT_D0L);",
            "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G0079_ai_StairsNativeBitmapIndices[C13_STAIRS_BITMAP_DOWN_FRONT_D0C_LEFT], C825_ZONE_STAIRS_DOWN_FRONT_D0R);",
            "break;",
            "case C02_ELEMENT_PIT:",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(L0222_ai_SquareAspect[M554_PIT_OR_TELEPORTER_VISIBLE] ? M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C : M761_GRAPHIC_FLOOR_PIT_D0C, C862_ZONE_FLOORPIT_D0C);",
            "F0112_DUNGEONVIEW_DrawCeilingPit(C069_GRAPHIC_CEILING_PIT_D0C, C871_ZONE_CEILING_PIT_D0C",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);",
            "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M609_VIEW_SQUARE_D0C]], C715_ZONE_WALL_D0C);",
        ],
    ),
    (
        "f0115_current_square_layer_contract",
        RED / "DUNVIEW.C",
        "4561-4582",
        "F0115 draws objects, creatures, and projectiles per ordered cell, then explosions after ordered cells.",
        [
            "Contains 4 nibbles processed from the least significant to the most significant nibble",
            "draw each object found",
            "Draw one creature at the cell being processed",
            "Draw only projectiles at specified cell",
            "} while there are cells left to process",
            "Draw only explosions at specified cell",
        ],
    ),
    (
        "pc34_d0c_zone_constants",
        RED / "DEFS.H",
        "2596-2647",
        "PC34 MEDIA720 view-square and cell ordinals make D0C id 0 and define the back-left/back-right cell names used by C0x0021.",
        [
            "#define M609_VIEW_SQUARE_D0C  0",
            "#define M610_VIEW_SQUARE_D0L  1",
            "#define M611_VIEW_SQUARE_D0R  2",
            "#define C02_VIEW_CELL_BACK_RIGHT",
            "#define C03_VIEW_CELL_BACK_LEFT                    3",
        ],
    ),
    (
        "pc34_d0c_viewport_zone_constants",
        RED / "DEFS.H",
        "4055-4219",
        "PC34/I34E zone ids bind D0C field, frame, thieves-eye, stairs, floor pit, and ceiling pit draw destinations.",
        [
            "#define C715_ZONE_WALL_D0C                                      715",
            "#define C728_ZONE_DOOR_FRAME_D0C                                728",
            "#define C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME                736",
            "#define C811_ZONE_STAIRS_UP_FRONT_D0L                           811",
            "#define C812_ZONE_STAIRS_UP_FRONT_D0R                           812",
            "#define C824_ZONE_STAIRS_DOWN_FRONT_D0L                         824",
            "#define C825_ZONE_STAIRS_DOWN_FRONT_D0R                         825",
            "#define C862_ZONE_FLOORPIT_D0C                                  862",
            "#define C871_ZONE_CEILING_PIT_D0C                               871",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "firestaff_d0c_floor_field_metadata",
        "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
        [
            "{ DM1_VIEW_SQUARE_D0C, 0x0021, true, true, false, true, true, true, false,",
            "DUNVIEW.C:8241-8273 stairs front bitmap draws and breaks before F0115",
            "DUNVIEW.C:8274-8292 pit floor/ceiling bitmap before F0115",
            "DUNVIEW.C:8294 F0115 object/creature/projectile/explosion handoff with C0x0021",
            "DUNVIEW.C:8295-8308 teleporter field after F0115",
            "DUNVIEW.C:8185-8240 door-side case breaks before common F0115; no wall case in D0C",
        ],
    ),
    (
        "firestaff_d0c_foreground_tests",
        "tests/test_dm1_v1_viewport_3d_pc34_compat.c",
        [
            "test_d0c_thieves_eye_door_frame_occlusion_order",
            "test_floor_field_stairs_pit_teleporter_order",
            "DM1_VIEW_SQUARE_D0C",
            "d0c_foreground_before_things",
            "test_d0_d1_visible_square_draw_order_gate",
            "dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D0C)",
        ],
    ),
    (
        "firestaff_source_evidence_string",
        "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
        [
            "DUNVIEW.C:8185-8240,8241-8308 D0C door-side/stairs foreground blockers draw before common F0115; pit/ceiling/F0115/teleporter-field order",
            "DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion: copy front frame, composite hole, blit temporary frame before common F0115",
        ],
    ),
]


def main() -> int:
    red_rows = [check_region(*item) for item in REDMCSB_CHECKS]
    local_rows = [check_file(*item) for item in LOCAL_CHECKS]
    runtime = run([str(TEST_BINARY)]) if TEST_BINARY.exists() else {
        "command": [str(TEST_BINARY)],
        "returncode": 127,
        "passed": False,
        "outputTail": "missing test binary; configure/build first",
    }
    problems = [row["id"] for row in red_rows + local_rows if row["status"] != "PASS"]
    if not runtime["passed"]:
        problems.append("test_dm1_v1_viewport_3d_pc34_compat")
    ok = not problems
    manifest = {
        "schema": "pass608_dm1_v1_d0c_current_square_occlusion_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS608_DM1_V1_D0C_CURRENT_SQUARE_OCCLUSION_SOURCE_LOCK",
        "claim": "D0C current-square foreground blockers, common F0115 content handoff, and teleporter-field overlay order are source-locked for DM1 V1 PC34/I34E.",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [runtime],
        "nonClaims": [
            "Does not duplicate pass517 D3/D2 side-field occlusion.",
            "Does not duplicate pass518 D1 side-field occlusion.",
            "Does not duplicate pass583 D0 side-field occlusion.",
            "Does not claim original DOS pixel parity or capture-backed closure.",
            "Does not change renderer behavior.",
            "No DANNESBURK use.",
        ],
        "problems": problems,
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    report = [
        "# Pass608 DM1 V1 D0C current-square occlusion source lock",
        "",
        f"Status: {manifest['status']}",
        "",
        "Claim: D0C current-square foreground blockers, common F0115 content handoff, and teleporter-field overlay order are source-locked for DM1 V1 PC34/I34E.",
        "",
        "Primary evidence:",
    ]
    for row in red_rows:
        report.append(f"- {row['status']} {row['source']}: {row['claim']}")
    report += [
        "",
        "Verification:",
        f"- {' '.join(runtime['command'])}: rc={runtime['returncode']}",
        "",
        "Non-claims:",
    ]
    report += [f"- {claim}" for claim in manifest["nonClaims"]]
    REPORT.write_text("\n".join(report) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    if problems:
        print("- problems", ", ".join(problems))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())

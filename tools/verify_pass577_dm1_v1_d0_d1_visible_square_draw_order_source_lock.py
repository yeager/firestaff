#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
TEST_BINARY = ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat"
STATUS = "PASS577_DM1_V1_D0_D1_VISIBLE_SQUARE_DRAW_ORDER_SOURCE_LOCKED"


def read_lines(path: Path, start: int, end: int) -> tuple[int, str]:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    lines = path.read_text(encoding=encoding, errors="replace").splitlines()
    return start, "\n".join(lines[start - 1:end])


def ordered_hits(base: int, text: str, needles: list[str]) -> tuple[list[dict[str, object]], list[str]]:
    cursor = 0
    hits: list[dict[str, object]] = []
    missing: list[str] = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": base + text.count("\n", 0, pos), "needle": needle})
            cursor = pos + len(needle)
    return hits, missing


def audit_region(ident: str, path: Path, start: int, end: int, needles: list[str]) -> dict[str, object]:
    base, text = read_lines(path, start, end)
    hits, missing = ordered_hits(base, text, needles)
    return {"id": ident, "file": str(path), "lines": f"{start}-{end}", "status": "PASS" if not missing else "FAIL", "hits": hits, "missing": missing}


REDMCSB_CHECKS = [
    ("f0128_d0_d1_visible_square_sequence", RED / "DUNVIEW.C", 8522, 8542, [
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1",
        "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1",
        "F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0",
        "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, -1",
        "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, 1",
        "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
    ]),
    ("f0115_layer_return_paths", RED / "DUNVIEW.C", 4561, 4582, [
        "Contains 4 nibbles processed from the least significant",
        "draw each object found",
        "Draw one creature at the cell being processed",
        "Draw only projectiles at specified cell",
        "} while there are cells left to process",
        "Draw only explosions at specified cell",
        "If a Fluxcage is present, draw the fluxcage",
    ]),
    ("d1l_wall_open_door_pit_field_paths", RED / "DUNVIEW.C", 7436, 7555, [
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
        "return;",
        "case C16_ELEMENT_DOOR_SIDE:",
        "L0213_i_Order = C0x0032_CELL_ORDER_BACKRIGHT_FRONTRIGHT;",
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;",
        "case C02_ELEMENT_PIT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0112_DUNGEONVIEW_DrawCeilingPit",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0113_DUNGEONVIEW_DrawField",
    ]),
    ("d1r_wall_open_door_pit_field_paths", RED / "DUNVIEW.C", 7604, 7722, [
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);",
        "return;",
        "case C16_ELEMENT_DOOR_SIDE:",
        "L0215_i_Order = C0x0041_CELL_ORDER_BACKLEFT_FRONTLEFT;",
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0215_i_Order = C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT;",
        "case C02_ELEMENT_PIT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0112_DUNGEONVIEW_DrawCeilingPit",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0113_DUNGEONVIEW_DrawField",
    ]),
    ("d1c_front_wall_door_open_paths", RED / "DUNVIEW.C", 7833, 7937, [
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
        "C0x0000_CELL_ORDER_ALCOVE",
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "case C02_ELEMENT_PIT:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
    ]),
    ("d0l_d0r_side_wall_returns_and_open_cells", RED / "DUNVIEW.C", 8000, 8144, [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
        "return;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
        "return;",
    ]),
    ("d0c_foreground_then_common_f0115", RED / "DUNVIEW.C", 8185, 8295, [
        "case C16_ELEMENT_DOOR_SIDE:",
        "if (G0407_s_Party.Event73Count_ThievesEye)",
        "F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C",
        "case C02_ELEMENT_PIT:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
    ]),
    ("set_square_aspect_visible_area_inputs", RED / "DUNGEON.C", 2517, 2589, [
        "F0008_MAIN_ClearBytes",
        "L0314_T_Thing = F0161_DUNGEON_GetSquareFirstThing",
        "P0317_pui_SquareAspect[C0_ELEMENT] = M034_SQUARE_TYPE",
        "G0289_i_DungeonView_ChampionPortraitOrdinal = 0;",
        "F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals",
        "if (((TEXTSTRING*)L0308_ps_Sensor)->Visible)",
        "P0317_pui_SquareAspect[AL0310_i_SideIndex] = G0265_i_CurrentMapInscriptionWallOrnamentIndex + 1;",
    ]),
]

LOCAL_CHECKS = [
    ("firestaff_combined_c_test", ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c", 858, 1012, [
        "static void test_d0_d1_visible_square_draw_order_gate(void)",
        "DM1_VIEW_SQUARE_D1L, 1, -1",
        "DM1_VIEW_SQUARE_D1R, 1,  1",
        "DM1_VIEW_SQUARE_D1C, 1,  0",
        "DM1_VIEW_SQUARE_D0L, 0, -1",
        "DM1_VIEW_SQUARE_D0R, 0,  1",
        "DM1_VIEW_SQUARE_D0C, 0,  0",
        "dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1C)",
        "dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C)",
        "dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_VIEW_SQUARE_D1C)",
        "test_d0_d1_visible_square_draw_order_gate();",
    ]),
    ("firestaff_metadata_tables", ROOT / "dm1_v1_viewport_3d_pc34_compat.c", 80, 170, [
        "DM1_VIEW_SQUARE_D1L, 1, -1",
        "DM1_VIEW_SQUARE_D1R, 1,  1",
        "DM1_VIEW_SQUARE_D1C, 1,  0",
        "DM1_VIEW_SQUARE_D0L, 0, -1",
        "DM1_VIEW_SQUARE_D0R, 0,  1",
        "DM1_VIEW_SQUARE_D0C, 0,  0",
        "DM1_VIEW_SQUARE_D1C,   3, 1,  8",
        "DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736",
    ]),
]


def run(cmd: list[str]) -> dict[str, object]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-18:])}


def main() -> int:
    checks = [audit_region(*row) for row in REDMCSB_CHECKS + LOCAL_CHECKS]
    test_run = run([str(TEST_BINARY)]) if TEST_BINARY.exists() else {"command": [str(TEST_BINARY)], "returncode": 127, "passed": False, "outputTail": "missing test binary; configure/build first"}
    problems = [c["id"] for c in checks if c["status"] != "PASS"]
    if not test_run["passed"]:
        problems.append("test_dm1_v1_viewport_3d_pc34_compat")
    result = {
        "schema": "pass577_dm1_v1_d0_d1_visible_square_draw_order_source_lock.v1",
        "status": STATUS if not problems else "FAILED_PASS577_DM1_V1_D0_D1_VISIBLE_SQUARE_DRAW_ORDER_SOURCE_LOCK",
        "ok": not problems,
        "redmcsbRoot": str(RED),
        "checks": checks,
        "verificationRuns": [test_run],
        "nonClaims": ["no original DUNGEON.DAT/GRAPHICS.DAT bytes read", "no original runtime or pixel parity capture", "no renderer behavior change", "no DANNESBURK use"],
        "problems": problems,
    }
    print(json.dumps(result, indent=2, sort_keys=True))
    print(result["status"])
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())

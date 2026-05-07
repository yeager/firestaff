#!/usr/bin/env python3
"""Verify pass359 DM1 V1 viewport wall draw-order/occlusion sweep artifacts."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MANIFEST = ROOT / "parity-evidence/verification/pass359_dm1_v1_viewport_wall_draw_order_occlusion_sweep/manifest.json"
EVIDENCE = ROOT / "parity-evidence/pass359_dm1_v1_viewport_wall_draw_order_occlusion_sweep.md"
EXPECTED_STATUS = "PASS_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_OCCLUSION_SWEEP"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS359_VIEWPORT_WALL_DRAW_ORDER_OCCLUSION_SWEEP reason={message}")
    return 1


def text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_all(blob: str, needles: list[str], context: str) -> None:
    compact = " ".join(blob.split())
    for needle in needles:
        require(" ".join(needle.split()) in compact, f"{context} missing {needle}")


def source_block(file_name: str, start: int, end: int) -> str:
    path = SOURCE_ROOT / file_name
    require(path.exists(), f"missing ReDMCSB source {path}")
    lines = path.read_text(encoding="latin-1").splitlines()
    return "\n".join(lines[start - 1:end])


def repo_block(file_name: str, start: int, end: int) -> str:
    path = ROOT / file_name
    require(path.exists(), f"missing Firestaff file {path}")
    lines = path.read_text(encoding="utf-8").splitlines()
    return "\n".join(lines[start - 1:end])


def run_gate(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).stdout


def main() -> int:
    try:
        manifest = json.loads(text(MANIFEST))
        evidence = text(EVIDENCE)

        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require("raw source dumps" not in evidence.lower(), "evidence should cite, not dump source")
        for anchor in manifest.get("source_anchors", []):
            marker = anchor["file"] + ":" + anchor["lines"]
            require(marker in evidence, f"evidence missing anchor {marker}")

        require_all(source_block("DRAWVIEW.C", 709, 900), [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ], "DRAWVIEW.C:709-900")
        require_all(source_block("DUNVIEW.C", 2962, 3003), [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",
            "F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea);",
            "G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;",
        ], "DUNVIEW.C:2962-3003")
        require_all(source_block("DUNVIEW.C", 3048, 3082), [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "G0296_puc_Bitmap_Viewport",
        ], "DUNVIEW.C:3048-3082")
        require_all(source_block("DUNVIEW.C", 4547, 4910), [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
        ], "DUNVIEW.C:4547-4910")
        require_all(source_block("DUNVIEW.C", 6400, 6835), [
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "return;",
        ], "DUNVIEW.C:6400-6835")
        require_all(source_block("DUNVIEW.C", 7244, 7937), [
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G3013_i_WallSet_Wall_D1C",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
        ], "DUNVIEW.C:7244-7937")
        require_all(source_block("DUNVIEW.C", 8318, 8618), [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ], "DUNVIEW.C:8318-8618")
        require_all(source_block("COORD.C", 1693, 1724), [
            "G2067_i_ViewportScreenX = 0", "G2068_i_ViewportScreenY = 33",
            "G2070_ViewportBitmapByteCount = 15232", "G2073_C224_ViewportPixelWidth = 224", "G2074_C136_ViewportHeight = 136",
        ], "COORD.C:1693-1724")
        require_all(source_block("COORD.C", 174, 186), [
            "Wall D3L2", "Wall D3C", "Wall D2C", "Wall D1C", "Wall D0R",
        ], "COORD.C:174-186")
        require_all(source_block("DEFS.H", 4042, 4057), [
            "C702_ZONE_WALL_D3L2", "C704_ZONE_WALL_D3C", "C709_ZONE_WALL_D2C", "C712_ZONE_WALL_D1C", "C717_ZONE_WALL_D0R",
        ], "DEFS.H:4042-4057")

        require_all(repo_block("m11_game_view.c", 9329, 9371), [
            "m11_dm1_max_visible_forward_from_center", "m11_dm1_nearest_blocking_center_depth_index", "m11_dm1_nearest_blocking_center_door_depth",
        ], "m11_game_view.c:9329-9371")
        require_all(repo_block("m11_game_view.c", 9373, 9414), [
            "m11_draw_dm1_front_walls", "M11_GFX_WALLSET0_D1C", "M11_GFX_WALLSET0_D2C", "M11_GFX_WALLSET0_D3C", "occluded = 1",
        ], "m11_game_view.c:9373-9414")
        require_all(repo_block("m11_game_view.c", 9918, 9979), [
            "m11_draw_dm1_side_walls", "Far to near", "m11_dm1_side_lane_clear_for_rel", "m11_viewport_cell_is_wall_like",
        ], "m11_game_view.c:9918-9979")
        require_all(repo_block("m11_game_view.c", 12107, 12135), [
            "m11_draw_dm1_side_contents", "after source wall/door panels and before center", "m11_dm1_center_line_clear_before_depth",
        ], "m11_game_view.c:12107-12135")
        require_all(repo_block("m11_game_view.c", 17980, 18080), [
            "m11_draw_viewport", "m11_sample_viewport_cell", "m11_draw_dm1_side_walls", "m11_draw_dm1_front_walls",
            "replay only", "m11_draw_dm1_side_contents",
        ], "m11_game_view.c:17980-18080")

        run_gate([sys.executable, "scripts/verify_dm1_v1_viewport_wall_draw_order_source_lock.py"])
        for gate in [
            "tools/verify_v1_viewport_occlusion_gate.py",
            "tools/verify_v1_viewport_side_wall_occlusion_gate.py",
            "tools/verify_v1_viewport_draw_order_gate.py",
            "tools/verify_v1_viewport_wall_depth_source_lock_gate.py",
            "tools/verify_v1_viewport_redmcsb_draw_stack_gate.py",
            "tools/verify_v1_viewport_center_door_occlusion_gate.py",
        ]:
            run_gate([sys.executable, gate])

        print(f"status={EXPECTED_STATUS}")
        print("sourceAnchors=%u" % len(manifest.get("source_anchors", [])))
        print("viewportWallOcclusionGatesOk=1")
        return 0
    except (AssertionError, OSError, json.JSONDecodeError, subprocess.CalledProcessError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())

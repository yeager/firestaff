#!/usr/bin/env python3
"""Verify DM1 V1 door button and door ornament viewport coordinates match ReDMCSB.

Source references:
  DUNVIEW.C G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets[4][3][6]
  DUNVIEW.C G0208_aaauc_Graphic558_DoorButtonCoordinateSets[1][4][6]
  DUNVIEW.C F0109_DUNGEONVIEW_DrawDoorOrnament (lines 4013-4118)
  DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton (lines 4119-4260)
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / "src/engine/m11_game_view.c"
DM1_VIEWPORT = ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c"
DM1_DOOR_ORN = ROOT / "src/dm1/dm1_v1_door_ornament_render_pc34_compat.c"
DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
    vp = DM1_VIEWPORT.read_text(encoding="utf-8")
    orn = DM1_DOOR_ORN.read_text(encoding="utf-8")
    red = DUNVIEW.read_text(encoding="utf-8", errors="replace")

    # --- Door button coordinates from G0208 ---
    # G0208 D3R: {199,204,41,44,8,4} → pos=(199,41), w=6, h=4
    # G0208 D3C: {136,141,41,44,8,4} → pos=(136,41), w=6, h=4
    # G0208 D2C: {144,155,42,47,8,6} → pos=(144,42), w=12, h=6
    # G0208 D1C: {160,175,44,52,8,9} → pos=(160,44), w=16, h=9

    # Verify the original ReDMCSB has these values
    require(red, "{ 199, 204, 41, 44, 8, 4}",
            "ReDMCSB G0208 D3R door button coordinates")
    require(red, "{ 136, 141, 41, 44, 8, 4}",
            "ReDMCSB G0208 D3C door button coordinates")
    require(red, "{ 144, 155, 42, 47, 8, 6}",
            "ReDMCSB G0208 D2C door button coordinates")
    require(red, "{ 160, 175, 44, 52, 8, 9}",
            "ReDMCSB G0208 D1C door button coordinates")

    # The G0208 frames are owned by the DM1 viewport module; M11 only
    # consumes them through dm1_v1_viewport_get_door_button_frame_pc34.
    require(vp, "G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
            "DM1 G0208 provenance comment")
    require(vp, "/* D1C */ { 160, 175, 44, 52, 8, 9, 0, 0 },",
            "DM1 G0208 D1C door button frame")
    require(vp, "/* D2C */ { 144, 155, 42, 47, 8, 6, 0, 0 },",
            "DM1 G0208 D2C door button frame")
    require(vp, "/* D3C */ { 136, 141, 41, 44, 8, 4, 0, 0 },",
            "DM1 G0208 D3C door button frame")
    require(vp, "/* D3R */ { 199, 204, 41, 44, 8, 4, 0, 0 },",
            "DM1 G0208 D3R door button frame")

    # M11 center + D3R door buttons consume the DM1 frames and blit with C10.
    require(fire, "frame = dm1_v1_viewport_get_door_button_frame_pc34(1, viewIndex);",
            "M11 center door buttons consume DM1 G0208 frames")
    require(fire, "M11_VIEWPORT_X + frame->left_x",
            "M11 door button X from DM1 frame")
    require(fire, "M11_VIEWPORT_Y + frame->top_y",
            "M11 door button Y from DM1 frame")
    require(fire, "DM1_VIEW_DOOR_BUTTON_D3R",
            "M11 D3R door button view index")

    # --- Door ornament coordinates from G0207 ---
    # The G0207 coordinate sets are owned by the DM1 door-ornament render module.
    require(orn, "s_doorOrnamentCoordSets[4][3][6]",
            "DM1 G0207 door ornament coordinate set table (4 sets)")

    # Verify specific G0207 entries match ReDMCSB
    # Set 0 D1LCR: {32,63,13,31,16,19}
    require(orn, "{32,63,13,31,16,19}",
            "DM1 G0207 set 0 D1LCR coordinates")
    # Set 1 D1LCR: {0,95,0,87,48,88}
    require(orn, "{ 0,95, 0,87,48,88}",
            "DM1 G0207 set 1 D1LCR coordinates")
    # Set 2 D3LCR: {17,31,15,24,8,10}
    require(orn, "{17,31,15,24, 8,10}",
            "DM1 G0207 set 2 D3LCR coordinates")
    # Set 3 D1LCR: {44,75,61,79,16,19}
    require(orn, "{44,75,61,79,16,19}",
            "DM1 G0207 set 3 D1LCR coordinates")

    # Verify ReDMCSB G0207 data
    require(red, "{ 32, 63, 13, 31, 16, 19 }",
            "ReDMCSB G0207 set 0 D1LCR")
    require(red, "{  0, 95,  0, 87, 48, 88 }",
            "ReDMCSB G0207 set 1 D1LCR")

    # Verify ornament position uses coordinate set directly (DM1 render plan)
    require(orn, "ornW = coord[1] - coord[0] + 1;",
            "DM1 ornament width from G0207 X2-X1+1")
    require(orn, "ornH = coord[3] - coord[2] + 1;",
            "DM1 ornament height from G0207 Y2-Y1+1")
    require(orn, "relX = coord[0];",
            "DM1 ornament X from G0207 X1")
    require(orn, "relY = coord[2];",
            "DM1 ornament Y from G0207 Y1")

    # Verify door ornament palette changes from G0200/G0201 (DM1-owned)
    require(orn, "0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 0, 13",
            "DM1 door ornament D3 palette (G0200)")
    require(orn, "0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15",
            "DM1 door ornament D2 palette (G0201)")

    print("V1 door button and ornament coordinate gate passed")
    print("- Center door buttons: D1C=(160,44,16x9), D2C=(144,42,12x6), D3C=(136,41,6x4)")
    print("- D3R door button: (199,41,6x4)")
    print("- Door ornament positions: G0207 coordinate sets [4][3][6] source-locked")
    print("- Door ornament palettes: G0200/G0201 source-locked")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)

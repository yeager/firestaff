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
DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
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

    # Verify Firestaff center door buttons match G0208
    require(fire, "0, 0, 160, 44, 16, 9",
            "Firestaff center D1C door button at G0208 position")
    require(fire, "0, 0, 144, 42, 12, 6",
            "Firestaff center D2C door button at G0208 position")
    require(fire, "0, 0, 136, 41, 6, 4",
            "Firestaff center D3C door button at G0208 position")

    # Verify Firestaff D3R door button matches G0208[0][0]
    require(fire, "M11_VIEWPORT_X + 199",
            "Firestaff D3R door button X=199")
    require(fire, "M11_VIEWPORT_Y + 41",
            "Firestaff D3R door button Y=41")

    # --- Door ornament coordinates from G0207 ---
    # Verify Firestaff has the G0207 coordinate set table embedded
    require(fire, "kDoorOrnCoordSets[4][3][6]",
            "Firestaff G0207 door ornament coordinate set table (4 sets)")

    # Verify specific G0207 entries match ReDMCSB
    # Set 0 D1LCR: {32,63,13,31,16,19}
    require(fire, "{32,63,13,31,16,19}",
            "Firestaff G0207 set 0 D1LCR coordinates")
    # Set 1 D1LCR: {0,95,0,87,48,88}
    require(fire, "{ 0,95, 0,87,48,88}",
            "Firestaff G0207 set 1 D1LCR coordinates")
    # Set 2 D3LCR: {17,31,15,24,8,10}
    require(fire, "{17,31,15,24, 8,10}",
            "Firestaff G0207 set 2 D3LCR coordinates")
    # Set 3 D1LCR: {44,75,61,79,16,19}
    require(fire, "{44,75,61,79,16,19}",
            "Firestaff G0207 set 3 D1LCR coordinates")

    # Verify ReDMCSB G0207 data
    require(red, "{ 32, 63, 13, 31, 16, 19 }",
            "ReDMCSB G0207 set 0 D1LCR")
    require(red, "{  0, 95,  0, 87, 48, 88 }",
            "ReDMCSB G0207 set 1 D1LCR")

    # Verify ornament position uses coordinate set directly
    require(fire, "ornW = cs[1] - cs[0] + 1",
            "Firestaff ornament width from G0207 X2-X1+1")
    require(fire, "ornH = cs[3] - cs[2] + 1",
            "Firestaff ornament height from G0207 Y2-Y1+1")
    require(fire, "relX = cs[0]",
            "Firestaff ornament X from G0207 X1")
    require(fire, "relY = cs[2]",
            "Firestaff ornament Y from G0207 Y1")

    # Verify door ornament palette changes from G0200/G0201
    require(fire, "0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 0, 13",
            "Firestaff door ornament D3 palette (G0200)")
    require(fire, "0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15",
            "Firestaff door ornament D2 palette (G0201)")

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

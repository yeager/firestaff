#!/usr/bin/env python3
"""Source-lock V1 DM1 side-wall panel transparency.

ReDMCSB PC/I34E side-wall paths draw side panels through F0104/F0105.
Those helpers call F0132_VIDEO_Blit with C10_COLOR_FLESH as the transparent
key.  Center-front walls are different: F0118/F0121/F0124 use F0792/F0765
with CM1_COLOR_NO_TRANSPARENCY.
"""
from __future__ import annotations

from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SRC = REPO / "m11_game_view.c"

CITATIONS = [
    "ReDMCSB DUNVIEW.C:3111-3155 F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap blits with C10_COLOR_FLESH on MEDIA529/PC paths.",
    "ReDMCSB DUNVIEW.C:3193-3267 F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally preserves the same C10 transparency after flip.",
    "ReDMCSB DUNVIEW.C:6423-6427 / 6555-6563 route D3L/D3R side walls through F0105/F0104.",
    "ReDMCSB DUNVIEW.C:6708 / 7300 / 7834 route center walls through F0792_DUNGEONVIEW_DrawBitmapYYY with CM1_COLOR_NO_TRANSPARENCY.",
]


def line_no(text: str, needle: str) -> int:
    off = text.index(needle)
    return text.count("\n", 0, off) + 1


def main() -> int:
    text = SRC.read_text()
    side_start = text.index("static void m11_draw_dm1_side_walls(")
    side_end = text.index("static void m11_draw_dm1_center_doors(", side_start)
    side = text[side_start:side_end]
    front_start = text.index("static void m11_draw_dm1_front_walls(")
    front_end = text.index("static int m11_dm1_side_lane_clear_before_depth", front_start)
    front = text[front_start:front_end]

    required_side = [
        "F0104/F0105",
        "C10_COLOR_FLESH as the transparent color",
        "m11_draw_dm1_wall_blit_with_transparency",
        "&kSideBlits[i],\n                                                               10);",
        "&swapped,\n                                                     10);",
        "partner = i ^ 1",
    ]
    missing = [needle for needle in required_side if needle not in side]
    if missing:
        raise AssertionError(f"side-wall C10 transparency source-lock missing tokens: {missing}")

    if "&kFrontBlits[depth],\n                                                     -1);" not in front:
        raise AssertionError("center-front flipped wall path must remain opaque (-1 transparency)")

    helper = text[text.index("static int m11_draw_dm1_wall_blit_flipped("):text.index("static unsigned int m11_wallset_graphic_index_for_state", text.index("static int m11_draw_dm1_wall_blit_flipped("))]
    if "transparentColor >= 0" not in helper or "continue;" not in helper:
        raise AssertionError("flipped wall helper does not honor transparentColor")

    print("PASS v1 viewport side-wall panels use ReDMCSB C10 transparency; center walls stay opaque")
    print("Firestaff: m11_game_view.c:{}".format(line_no(text, "static void m11_draw_dm1_side_walls(")))
    for citation in CITATIONS:
        print("- " + citation)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

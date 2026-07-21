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
SRC = REPO / "src/engine/m11_game_view.c"
DM1_SRC = REPO / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c"

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
    dm1_text = DM1_SRC.read_text()

    # Side-wall C10 ownership now lives in the DM1 owner module: DM1 selects
    # the wall material and stamps C10_COLOR_FLESH (10) on the host handoff,
    # exactly as F0104/F0105 pass it to F0132_VIDEO_Blit.
    handoff_start = dm1_text.index("dm1_viewport_3d_build_d3_side_wall_host_handoff_pc34(")
    handoff_start = dm1_text.index("{", handoff_start)
    handoff_end = dm1_text.index("\n}", handoff_start)
    handoff = dm1_text[handoff_start:handoff_end]
    required_handoff = [
        "handoff.transparent_color = 10;",
        "C10-transparent wall material",
    ]
    missing = [needle for needle in required_handoff if needle not in handoff]
    if missing:
        raise AssertionError(f"DM1 side-wall C10 handoff source-lock missing tokens: {missing}")
    for citation in ("F0104", "F0105", "C10_COLOR_FLESH"):
        if citation not in dm1_text:
            raise AssertionError(f"DM1 viewport module lost its {citation} provenance")

    # M11 must honor the DM1-selected transparent color when consuming the
    # side-wall host receipt (and must not re-invent the key itself).
    receipt_start = text.index("static int m11_draw_dm1_side_wall_host_receipt(")
    receipt_end = text.index("\nstatic ", receipt_start)
    receipt = text[receipt_start:receipt_end]
    if "receipt->material.transparent_color >= 0" not in receipt or "continue;" not in receipt:
        raise AssertionError("M11 side-wall host receipt does not honor the DM1 transparent color")

    # Center-front walls stay opaque (CM1_COLOR_NO_TRANSPARENCY -> -1).
    front_start = text.index("static void m11_draw_dm1_front_walls(")
    front_end = text.index("\nstatic ", front_start) + 1
    front = text[front_start:front_end]
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

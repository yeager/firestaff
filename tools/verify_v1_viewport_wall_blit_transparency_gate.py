#!/usr/bin/env python3
"""Source-lock V1 DM1 wall-panel blits as opaque geometry.

This verifier prevents regression of the pass195 right-side viewport geometry
fix: DUNVIEW.C draws wall panels with F0104/F0105 (no C10 transparency), while
Firestaff had keyed side-wall panels on palette 10 and punched black holes in
valid tan wall texels around ingame_move_forward/D1R.
"""
from __future__ import annotations

import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SRC = REPO / "m11_game_view.c"

CITATIONS = [
    "ReDMCSB DUNVIEW.C:7604-7628 draws D1R wall via F0104(... C714_ZONE_WALL_D1R) and returns, with no C10 transparency key.",
    "ReDMCSB DUNVIEW.C:7690-7704 draws D1R corridor/teleporter ceiling pit then F0115 objects, not a wall panel.",
    "ReDMCSB DUNVIEW.C:8117-8144 draws D0R wall via F0104(... C717_ZONE_WALL_D0R) and returns, with no C10 transparency key.",
    "ReDMCSB DUNVIEW.C:2389-2408 prebuilds flipped wall bitmaps; C10 is used as a temporary fill for derived flipped masks, not as transparency for the direct wall primitives.",
]


def line_no(text: str, needle: str) -> int:
    off = text.index(needle)
    return text.count("\n", 0, off) + 1


def main() -> int:
    text = SRC.read_text()
    start = text.index("static void m11_draw_dm1_side_walls(")
    end = text.index("static void m11_draw_dm1_center_doors(", start)
    body = text[start:end]

    required = [
        "M11_GFX_WALLSET0_D1R",
        "M11_GFX_WALLSET0_D0R",
        "m11_draw_dm1_wall_blit_with_transparency",
        "-1);",
        "real tan wall texel",
    ]
    missing = [needle for needle in required if needle not in body]
    if missing:
        raise AssertionError(f"side-wall opaque source-lock missing tokens: {missing}")

    bad = re.search(
        r"m11_draw_dm1_wall_blit_with_transparency\([^;]*M11_COLOR_MAGENTA\s*\)",
        body,
        re.S,
    )
    if bad:
        raise AssertionError(
            "side-wall wall-panel blit still keys on M11_COLOR_MAGENTA; "
            "DUNVIEW.C F0104/F0105 wall primitives are opaque"
        )

    print("PASS v1 viewport wall-panel blits are opaque source-locked")
    print("Firestaff: m11_game_view.c:{}".format(line_no(text, "static void m11_draw_dm1_side_walls(")))
    for citation in CITATIONS:
        print("- " + citation)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

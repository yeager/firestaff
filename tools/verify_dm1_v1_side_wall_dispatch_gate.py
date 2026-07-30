#!/usr/bin/env python3
"""Lock the DM1 F0128 side-wall dispatch contract in the live M11 caller."""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
M11 = ROOT / "src/engine/m11_game_view.c"
VIEWPORT = ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c"


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise AssertionError(f"missing {signature}")
    brace = source.find("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated {signature}")


def main() -> int:
    m11 = M11.read_text(encoding="utf-8")
    viewport = VIEWPORT.read_text(encoding="utf-8")
    side_walls = function_body(m11, "static void m11_draw_dm1_side_walls")
    ornaments = function_body(m11, "static void m11_draw_dm1_wall_ornaments")

    assert "m11_dm1_side_lane_clear_for_rel" not in side_walls, (
        "F0128 wall dispatch must not use the side-lane content occlusion gate")
    assert "m11_dm1_side_lane_clear_for_rel" not in ornaments, (
        "F0107 wall material must not use the side-lane content occlusion gate")
    assert "F0116/F0117 (D3L/D3R), F0119/F0120 (D2L/D2R)" in side_walls
    assert "spec->runtime_rel_forward > maxVisibleForward" in side_walls
    assert "m11_viewport_cell_is_wall_like(&cell)" in side_walls

    for square, depth, lateral, function in (
        ("DM1_VIEW_SQUARE_D3L", 3, -1, "F0116_DUNGEONVIEW_DrawSquareD3L"),
        ("DM1_VIEW_SQUARE_D3R", 3, 1, "F0117_DUNGEONVIEW_DrawSquareD3R"),
        ("DM1_VIEW_SQUARE_D2L", 2, -1, "F0119_DUNGEONVIEW_DrawSquareD2L"),
        ("DM1_VIEW_SQUARE_D2R", 2, 1, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF"),
    ):
        pattern = re.compile(
            rf'\{{\s*{square},[^\n]*?\b{depth},\s*{lateral},[^\n]*?"'
            + re.escape(function))
        assert pattern.search(viewport), f"missing F0128 spec for {square}"

    assert "F0128 lines 8446-8542 draws D3/D2/D1 side" in viewport
    assert "champion portraits are owned by the D1C front-mirror route" in m11
    assert "ornGlobalIdx == 0 && spec.viewWallIndex == 12" in m11
    print("ok: DM1 F0128/F0107 D2/D3 side wall material bypasses content-only occlusion")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

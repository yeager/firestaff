#!/usr/bin/env python3
"""Lock the source-owned F0128 live DM1 viewport composition order."""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
M11 = ROOT / "src/engine/m11_game_view.c"


def function_body(source: str, signature: str) -> str:
    start = source.rfind(signature)
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
    source = M11.read_text(encoding="utf-8")
    viewport = function_body(source, "static void m11_draw_viewport")
    side_contents = function_body(source, "static void m11_draw_dm1_side_contents_at_depth")
    ornaments = function_body(source, "static void m11_draw_dm1_wall_ornaments")

    content_loop = viewport[viewport.find("kContentSquares"):]
    assert "DM1_V1_F0128_VIEW_SQUARE_D3L" in content_loop
    assert "DM1_V1_F0128_VIEW_SQUARE_D3C" in content_loop
    assert re.search(
        r"D3L.*?D3R.*?D3C.*?D2L.*?D2R.*?D2C.*?D1L.*?D1R.*?D1C",
        content_loop,
        re.S,
    ), "F0128 square scheduler must retain D3 -> D2 -> D1 order"
    assert "else if (!sidesDrawnForDepth[squareDepth])" in content_loop
    assert "m11_draw_dm1_side_contents_at_depth" in content_loop
    assert "m11_draw_wall_contents" in content_loop
    assert "int depth," in side_contents
    assert "for (sideSlot = 0; sideSlot < 2; ++sideSlot)" in side_contents

    center_contents = viewport.find("centerContentMask")
    mirror = viewport.find("m11_draw_dm1_front_mirror_route")
    assert 0 <= mirror < center_contents, (
        "D1C C127 mirror must remain in the F0107 wall pass before F0115 content")
    assert "&cells[0][1]" in viewport[mirror:mirror + 200], (
        "C127 mirror owner must consume only the D1C front cell")
    assert "m11_draw_dm1_front_mirror_route(" not in ornaments, (
        "generic F0107 ornament pass cannot duplicate C127")
    print("ok: DM1 F0128 D3->D1 content order and D1C C127 ownership are locked")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

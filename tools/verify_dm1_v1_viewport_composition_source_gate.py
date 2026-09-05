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
    source_callback = function_body(
        source, "static void m11_dm1_f0128_execute_source_step")
    ornaments = function_body(source, "static void m11_draw_dm1_wall_ornaments")

    content_loop = viewport[viewport.find("kContentSquares"):]
    assert "DM1_V1_F0128_VIEW_SQUARE_D3L" in content_loop
    assert "DM1_V1_F0128_VIEW_SQUARE_D3C" in content_loop
    assert re.search(
        r"D3L.*?D3R.*?D3C.*?D2L.*?D2R.*?D2C.*?D1L.*?D1R.*?D1C",
        content_loop,
        re.S,
    ), "F0128 square scheduler must retain D3 -> D2 -> D1 order"
    # The old generic content loop and hand-written replay were superseded by
    # scheduler phase dispatch: DnL completes before DnR, then DnC, while the
    # callback owns each material operation.
    assert "kSideReplayOrder" in viewport
    assert "m11_dm1_f0128_dispatch_foreground_square" in viewport
    assert "m11_draw_wall_contents" in source_callback
    assert "int depth," in side_contents
    assert "int side," in side_contents
    assert "sourceOrderedPass" in side_contents
    assert "m11_draw_dm1_side_contents_at_depth" in source_callback

    mirror = source_callback.find("m11_draw_dm1_front_mirror_route")
    foreground_phase = source_callback.find(
        "M11_DM1_F0128_EXECUTE_FOREGROUND", mirror)
    assert 0 <= mirror < foreground_phase, (
        "D1C C127 mirror must remain in its F0107 callback before the "
        "following F0115 foreground phase")
    assert "&dispatch->cells[0][1]" in source_callback[mirror:mirror + 300], (
        "C127 mirror owner must consume only the D1C front cell")
    assert "m11_draw_dm1_front_mirror_route(" not in viewport, (
        "viewport must not replay C127 outside its scheduler callback")
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

#!/usr/bin/env python3
"""Lock the ReDMCSB F0125..F0127 D0 material pass after D1 content."""

from pathlib import Path
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

    for signature in (
        "static void m11_draw_dm1_floor_pits",
        "static void m11_draw_dm1_stairs",
        "static void m11_draw_dm1_teleporter_fields",
    ):
        body = function_body(source, signature)
        assert "int minVisibleForward" in body
        assert "plan.relForward < minVisibleForward" in body
        assert "plan.relForward > maxVisibleForward" in body

    early_calls = (
        "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                             1, 3, cells);",
        "m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                        1, 3, cells);",
        "m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                                  1, 3, cells);",
    )
    final_calls = (
        "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                             0, 0, cells);",
        "m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                        0, 0, cells);",
        "m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight,\n"
        "                                  0, 0, cells);",
    )
    for call in early_calls + final_calls:
        assert call in viewport, f"missing material pass: {call.split('(')[0]}"

    effect = viewport.find("m11_draw_dm1_deferred_explosion_pass")
    final = viewport.find(final_calls[0])
    mirror = viewport.find("m11_draw_dm1_front_mirror_route")
    assert effect >= 0 and final > effect and mirror > final
    assert "m11_draw_dm1_floor_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  0, 0, cells);" not in viewport
    print("ok: DM1 F0128 D0 pit/stair/field material follows D1 content and effects")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

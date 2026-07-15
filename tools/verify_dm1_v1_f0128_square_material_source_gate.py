#!/usr/bin/env python3
"""Lock F0128 D3..D1 square-material dispatch outside F0115 lane culling."""

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
    for signature, source_name in (
        ("static void m11_draw_dm1_floor_pits", "F0104 pit"),
        ("static void m11_draw_dm1_floor_ornaments", "F0108 floor ornament"),
        ("static void m11_draw_dm1_stairs", "F0104 stairs"),
        ("static void m11_draw_dm1_teleporter_fields", "F0113 field"),
    ):
        body = function_body(source, signature)
        assert "m11_dm1_side_lane_clear_for_rel" not in body, (
            f"{source_name} must remain a F0128 square-material route")
        assert "plan.relForward > maxVisibleForward" in body

    for call in (
        "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n                             3, cells);",
        "m11_draw_dm1_floor_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  3, cells);",
        "m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight,\n                        3, cells);",
        "m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  3, cells);",
    ):
        assert call in viewport, "viewport must retain full D3..D1 source dispatch"
    print("ok: F0128 D3..D1 pit/ornament/stairs/field material is not side-culled")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

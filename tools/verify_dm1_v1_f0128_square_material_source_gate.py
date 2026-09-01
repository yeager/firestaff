#!/usr/bin/env python3
"""Lock F0128 material dispatch and its final D0 source order."""

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
        assert "plan.relForward > maxVisibleForward" in body or (
            "plan.relForward < minVisibleForward" in body
        )

    for signature, source_name in (
        ("static void m11_draw_dm1_floor_pits", "F0104 pit"),
        ("static void m11_draw_dm1_floor_ornaments", "F0108 floor ornament"),
        ("static void m11_draw_dm1_stairs", "F0104 stairs"),
        ("static void m11_draw_dm1_teleporter_fields", "F0113 field"),
    ):
        body = function_body(source, signature)
        assert "plan.relForward < minVisibleForward" in body, (
            f"{source_name} must support the final D0-only pass")

    for source_name, call in (
        ("F0104 pit", "m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n                             1, 3, cells);"),
        ("F0108 floor ornament", "m11_draw_dm1_floor_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  1, 3, cells);"),
        ("F0104 stairs", "m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight,\n                        1, maxVisibleForward, cells);"),
        ("F0113 field", "m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight,\n                                  1, 3, cells);"),
    ):
        assert call in viewport, (
            f"{source_name} viewport call must retain full D3..D1 source dispatch"
        )

    effect = viewport.find("m11_draw_dm1_deferred_explosion_pass")
    d0 = viewport.find("m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight,\n                             0, 0, cells);")
    mirror = viewport.find("m11_draw_dm1_front_mirror_route")
    assert mirror >= 0 and effect >= 0 and mirror < effect < d0
    print("ok: F0128 D3..D1 material is not side-culled and D0 is final")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

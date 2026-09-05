#!/usr/bin/env python3
"""Keep DM1 F0108 floor ornaments out of the later F0115 host geometry path."""

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
    legacy = function_body(source, "static int m11_draw_floor_ornament")
    source_route = function_body(source, "static void m11_draw_dm1_floor_ornaments")
    callback = function_body(source, "static void m11_dm1_f0128_execute_source_step")
    viewport = function_body(source, "static void m11_draw_viewport")

    assert "m11_is_dm1_source_kind(state->sourceKind)" in legacy
    assert "m11_draw_dm1_floor_ornaments()" in legacy
    assert "m11_draw_dm1_zone_blit_maybe_flip" in source_route
    assert "dm1_v1_floor_ornament_render_plan_at_pc34" in source_route
    assert callback.find("M11_DM1_F0128_EXECUTE_PRE_DOOR_FLOOR_ORNAMENT") < callback.find(
        "M11_DM1_F0128_EXECUTE_DOOR_PASS1"), "door-front F0108 must precede F0115 pass 1"
    assert viewport.find("m11_dm1_f0128_dispatch_pre_door_floor_ornament_square") < viewport.find(
        "m11_dm1_f0128_dispatch_door_pass1_square"), "viewport must dispatch door-front F0108 before F0115"
    assert viewport.find("M11_DM1_F0128_EXECUTE_D0C_THINGS") < viewport.find(
        "M11_DM1_F0128_EXECUTE_D0C_FIELD_AFTER_THINGS"), "D0C F0115 must precede F0113"
    assert "m11_draw_dm1_d0c_floor_item_pass(state" not in viewport
    assert "m11_draw_dm1_d0c_projectile_pass(state" not in viewport
    assert "m11_draw_dm1_d0c_deferred_explosion_pass(state" not in viewport
    print("ok: DM1 F0108 floor ornaments have one source-owned zone renderer")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

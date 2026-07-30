#!/usr/bin/env python3
"""Keep DM1 F0115 viewport things on verified GRAPHICS.DAT paths only."""

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
    center = function_body(source, "static void m11_draw_wall_contents")
    side = function_body(source, "static void m11_draw_dm1_side_contents_at_depth")
    effects = function_body(source, "static void m11_draw_effect_cue")
    explosions = function_body(source, "static int m11_draw_explosion_material")
    projectile = function_body(source, "static int m11_draw_viewport_projectile_sprite")

    assert "m11_draw_dm1_f0115_floor_item_sprite" in center
    assert "m11_draw_creature_sprite" in center
    assert "m11_draw_creature_cue" not in center
    assert "m11_draw_dm1_f0115_floor_item_sprite" in side
    assert "m11_draw_creature_sprite_source_anchored" in side
    assert "m11_draw_viewport_projectile_sprite" in side
    assert "m11_fill_rect" not in side

    assert "m11_draw_viewport_projectile_sprite" in effects
    assert "m11_draw_projectile_sprite" in projectile
    for body, name in ((effects, "effect"), (projectile, "projectile")):
        assert "m11_draw_hline" not in body, f"{name} revives synthetic cross"
        assert "m11_draw_vline" not in body, f"{name} revives synthetic cross"

    assert "m11_draw_explosion_sprite" in explosions
    for synthetic in ("m11_fill_rect", "m11_draw_rect", "m11_draw_hline", "m11_draw_vline"):
        assert synthetic not in explosions, f"explosion route revives {synthetic}"
    assert "m11_draw_pit_effect" not in source
    assert "m11_draw_creature_cue" not in source
    print("ok: DM1 F0115 objects, creatures, projectiles, and explosions fail closed to source material")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

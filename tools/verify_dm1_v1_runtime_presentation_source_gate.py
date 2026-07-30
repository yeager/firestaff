#!/usr/bin/env python3
"""Lock source-owned DM1 M648, ACTIDRAW, TITLE, and C127 presentation."""

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
    m11 = M11.read_text(encoding="utf-8")
    ornaments = function_body(m11, "static void m11_draw_dm1_wall_ornaments")
    viewport = function_body(m11, "static void m11_draw_viewport")
    font_loader = function_body(
        m11, "static const M11_AssetSlot* m11_dm1_inscription_font_slot_for_material")
    glyphs = function_body(m11, "static int m11_draw_dm1_inscription_glyph_line")
    actions = function_body(m11, "static void m11_draw_dm1_ui_text_trailing_spaces")

    assert "m11_draw_dm1_front_wall_inscription_text" not in ornaments, (
        "M648 cannot be emitted in the pre-palette ornament pass")
    assert viewport.count("m11_repaint_dm1_f0128_front_wall_inscription") == 1, (
        "D1C M648 presentation must have one post-palette owner")
    palette = viewport.find("m11_apply_dungeon_palette_level")
    inscription = viewport.find("m11_repaint_dm1_f0128_front_wall_inscription")
    assert 0 <= palette < inscription
    assert "DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34" in font_loader
    for token in (
        "DM1_V1_INSCRIPTION_GLYPH_WIDTH",
        "DM1_V1_INSCRIPTION_GLYPH_HEIGHT",
        "DM1_V1_INSCRIPTION_TRANSPARENT_COLOR",
        "M11_AssetLoader_BlitRegion",
    ):
        assert token in glyphs, f"missing M648 source token: {token}"
    assert "BlitScaled" not in glyphs, "M648 cells must remain native 8x8"

    assert "m11_draw_dm1_m653_cell_clipped_at_baseline" in actions
    assert "m11_draw_text" not in actions, "ACTIDRAW cannot use a host-font fallback"
    print("ok: DM1 M648 and ACTIDRAW presentation remain source-owned")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

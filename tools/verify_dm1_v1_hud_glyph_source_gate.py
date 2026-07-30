#!/usr/bin/env python3
"""Lock DM1 action/spell glyph rendering to ReDMCSB TEXT2 source cells."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
M11 = ROOT / "src/engine/m11_game_view.c"


def function_body(source: str, signature: str) -> str:
    # This renderer has a forward declaration for the spell-area painter;
    # the definition is the later occurrence with the body we must inspect.
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
    text_cells = function_body(source, "static void m11_draw_dm1_ui_text_trailing_spaces")
    action_menu = function_body(source, "static int m11_draw_dm_action_menu")
    spell_area = function_body(source, "static void m11_draw_v1_spell_area_overlay")

    assert "m11_dm1_pc34_hud_font_is_source_bound(state)" in text_cells
    assert "m11_draw_dm1_m653_cell_clipped_at_baseline" in text_cells
    assert "m11_draw_text" not in text_cells, "TEXT2 must not substitute the host font"
    assert "m11_fill_rect" in text_cells, "missing-font path must clear source cells"
    assert "m11_draw_dm1_ui_text_trailing_spaces" in action_menu

    assert "if (state->spellBuffer.runeCount < 4)" not in spell_area
    for token in (
        "plan.available_symbol_count",
        "plan.champion_symbol_count",
        "plan.line2[i].screen_x",
        "plan.line2[i].screen_y",
        "plan.line3[i].screen_x",
        "plan.line3[i].screen_y",
        "DM1_V1_CPSAO_COLOR_CYAN_PC34",
        "DM1_V1_CPSAO_COLOR_BLACK_PC34",
        "m11_draw_dm1_m653_cell_clipped_at_baseline",
    ):
        assert token in spell_area, f"missing source geometry token: {token}"
    assert "plan.line3[i].drawn_character" in spell_area, (
        "F0398 must paint the planner's space-padded four cells")
    assert "facts.m653_source_bound" in spell_area

    print("ok: DM1 HUD action/spell glyphs use source TEXT2 cells without host-font fallback")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

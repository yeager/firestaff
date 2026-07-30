#!/usr/bin/env python3
"""Lock DM1 F0292 status-name TEXT2 cells and normal HUD composition."""

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
    status_name = function_body(source, "static void m11_draw_dm1_status_name_text")
    party_panel = function_body(source, "static void m11_draw_party_panel")
    screen = function_body(source, "void M11_GameView_Draw")

    assert "g_activeOriginalFont" in status_name
    assert "M11_Font_IsLoaded(g_activeOriginalFont)" in status_name
    assert "M11_Font_DrawChar" in status_name
    assert "m11_draw_text" not in status_name, "F0292 status names cannot use host text"
    assert "DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34" in status_name
    assert "DM1_V1_CPNBC_GLYPH_WIDTH_PC34" in status_name
    assert "text[charCount] != '\\0'" in status_name, (
        "F0053 must stop at Name[8] terminator")

    assert "m11_draw_dm1_status_name_text" in party_panel
    assert "DM1_V1_CPNBC_NAME_BOX_PRINT_Y_PC34" in party_panel
    assert "m11_draw_text_centered_in_rect" not in party_panel, (
        "normal V1 status names must not be host-font centered")

    party = screen.find("m11_draw_party_panel")
    icons = screen.find("m11_draw_v1_champion_icons")
    spell = screen.find("m11_draw_v1_spell_area_overlay")
    action = screen.find("m11_draw_v1_action_area_overlay")
    assert 0 <= party < icons < spell < action, (
        "normal HUD order must retain status/icon before spell/action overlays")
    print("ok: DM1 F0292 status names use source TEXT2 cells in HUD order")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

#!/usr/bin/env python3
"""Lock ReDMCSB F0111 center-door D3C -> D2C -> D1C composition."""

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
    for signature, token in (
        ("static void m11_draw_dm1_center_doors", "m11_draw_dm1_center_door_material_receipt"),
        ("static void m11_draw_dm1_center_door_ornaments", "m11_draw_dm1_door_ornament_on_panel"),
        ("static void m11_draw_dm1_center_destroyed_door_masks", "m11_draw_dm1_destroyed_door_mask_on_panel"),
        ("static void m11_draw_dm1_center_door_buttons", "m11_blit_scaled_palette_map"),
    ):
        body = function_body(source, signature)
        assert "for (depth = 2; depth >= 0; --depth)" in body, (
            f"{signature} must follow F0128 far-to-near center order")
        assert token in body
    doors = function_body(source, "static void m11_draw_dm1_center_doors")
    assert "break;" not in doors, "F0111 cannot discard D2C/D3C door material"
    print("ok: F0111 center doors, ornaments, masks, and buttons render D3C->D1C")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

#!/usr/bin/env python3
"""Lock DM1 startup special palettes to the selected presentation target."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/engine/main_loop_m11.c"


def function_body(text: str, name: str) -> str:
    match = re.search(rf"\b{name}\s*\([^;]*?\)\s*\{{", text, re.S)
    if not match:
        raise ValueError(f"function missing: {name}")
    start = match.end() - 1
    depth = 0
    for offset in range(start, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start:offset + 1]
    raise ValueError(f"unterminated function: {name}")


def main() -> int:
    try:
        text = SOURCE.read_text(encoding="utf-8")
        helper = function_body(text, "m11_present_dm1_startup_special_palette")
        required = (
            "M11_Render_SetModernPresentationActive",
            "M11_GameView_PresentationTarget",
            "M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette",
            "M11_Render_PresentIndexedToResolutionWithSpecialPalette",
            "M11_Render_PresentIndexedWithSpecialPalette",
            "M12_PRESENTATION_V1_ORIGINAL",
        )
        for token in required:
            if token not in helper:
                raise ValueError(f"startup presenter lost {token}")
        owners = {
            "m11_show_redmcsb_entrance_credits": 1,
            "m11_play_redmcsb_entrance_transition": 1,
            "m11_play_redmcsb_title_graphic_intro_if_available": 2,
        }
        for name, expected_calls in owners.items():
            body = function_body(text, name)
            if body.count("m11_present_dm1_startup_special_palette(") != expected_calls:
                raise ValueError(f"{name} lost its startup presentation route")
            if "M11_Render_PresentIndexedWithSpecialPalette(" in body:
                raise ValueError(f"{name} bypasses selected resolution")
    except (OSError, UnicodeError, ValueError) as error:
        print(f"DM1_STARTUP_SPECIAL_PALETTE_PRESENTATION_INVALID: {error}")
        return 1
    print("dm1_startup_special_palette_original_route=verified")
    print("dm1_startup_special_palette_modern_resolution_route=verified")
    print("dm1_startup_special_palette_call_sites=4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

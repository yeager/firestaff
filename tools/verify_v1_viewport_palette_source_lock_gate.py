#!/usr/bin/env python3
"""Verify V1 viewport light uses ReDMCSB global dungeon palette semantics.

ReDMCSB F0337 computes one G0304_i_DungeonViewPaletteIndex from torch
ChargeCount and G0407_s_Party.MagicalLightAmount. DRAWVIEW applies that one
palette to the dungeon view. Firestaff must not add invented D0/D1/D2 depth
post-dimming over source wall/floor/field zones.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
ORCH = ROOT / "src/memory/memory_tick_orchestrator_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
RED_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
RED_PANEL = RED_ROOT / "PANEL.C"
RED_DRAWVIEW = RED_ROOT / "DRAWVIEW.C"
RED_DATA = RED_ROOT / "DATA.C"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    pattern = re.compile(
        r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(")
    m = None
    for candidate in pattern.finditer(text):
        brace = text.find("{", candidate.end())
        semicolon = text.find(";", candidate.end())
        if brace >= 0 and (semicolon < 0 or brace < semicolon):
            m = candidate
            break
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():pos + 1]
    raise AssertionError(f"unterminated {name}")


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_absent(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise AssertionError(f"{label}: forbidden {needle!r}")


def require_in_order(text: str, markers: list[str], label: str) -> None:
    last = -1
    for marker in markers:
        pos = require(text, marker, label)
        if pos <= last:
            raise AssertionError(f"{label}: {marker!r} is out of order")
        last = pos


def main() -> int:
    fire = SRC.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")
    panel = RED_PANEL.read_text(encoding="latin-1")
    draw = RED_DRAWVIEW.read_text(encoding="latin-1")
    data = RED_DATA.read_text(encoding="latin-1")

    # ReDMCSB source contract.
    for needle in [
        "void F0337_INVENTORY_SetDungeonViewPalette(",
        "G0304_i_DungeonViewPaletteIndex = 0; /* Brightest color palette index */",
        "*AL1040_pi_TorchLightPower = L1042_ps_Weapon->ChargeCount;",
        "L1036_i_TotalLightAmount += (G0039_ai_Graphic562_LightPowerToLightAmount[*AL1040_pi_TorchLightPower] << L1037_ui_TorchLightAmountMultiplier) >> 6;",
        "L1036_i_TotalLightAmount += G0407_s_Party.MagicalLightAmount;",
        "AL1040_pi_LightAmount = G0040_ai_Graphic562_PaletteIndexToLightAmount;",
        "while (*AL1040_pi_LightAmount++ > L1036_i_TotalLightAmount)",
        "G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex;",
        "G0342_B_RefreshDungeonViewPaletteRequested = C1_TRUE;",
    ]:
        require(panel, needle, "ReDMCSB PANEL F0337")
    require(data, "int16_t G0039_ai_Graphic562_LightPowerToLightAmount[16] = { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 };", "ReDMCSB DATA light power table")
    require(data, "int16_t G0040_ai_Graphic562_PaletteIndexToLightAmount[6] = { 99, 75, 50, 25, 1, 0 };", "ReDMCSB DATA palette threshold table")
    for needle in [
        "G1010_pui_DungeonViewCurrentPalette = G0021_aaui_Graphic562_Palette_DungeonView[G0304_i_DungeonViewPaletteIndex];",
        "F0694_SetMultipleColorsInPalette(G2061_DungeonViewPaletteIndices[G0304_i_DungeonViewPaletteIndex]);",
        "G3123_PaletteSourceForMiddleScreen = G0021_aaui_Graphic562_Palette_DungeonView[G0304_i_DungeonViewPaletteIndex];",
    ]:
        require(draw, needle, "ReDMCSB DRAWVIEW palette apply")

    # Firestaff F0337 ownership moved to the M10 receipt module
    # (F0890b_ORCH_ComputeDungeonViewLight_Compat); M11 only consumes the
    # receipt for presentation. Verify the algorithm at its owner.
    orch = ORCH.read_text(encoding="utf-8")
    _, light_body = find_function(orch, "F0890b_ORCH_ComputeDungeonViewLight_Compat")
    for needle in [
        "static const int s_orch_light_power_to_amount_pc34[16] = {",
        "0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100",
        "static const int s_orch_palette_index_to_light_amount_pc34[6] = {",
        "99, 75, 50, 25, 1, 0",
    ]:
        require(orch, needle, "Firestaff M10 light tables")
    for needle in [
        "world->dungeon->maps[world->party.mapIndex].difficulty == 0",
        "for (i = 0; i < 5; i++)",
        "totalLight += (s_orch_light_power_to_amount_pc34[power] << multiplier) >> 6;",
        "totalLight += world->magic.magicalLightAmount;",
        "s_orch_palette_index_to_light_amount_pc34[paletteIndex] > totalLight",
        "outLight->paletteIndex = 5;",
    ]:
        require(light_body, needle, "Firestaff M10 F0337 algorithm")

    # M11 consumes the source-owned receipt; it must not re-implement the
    # weighted torch algorithm or invent depth post-dimming.
    _, palette_body = find_function(fire, "m11_compute_dungeon_palette_index")
    for needle in [
        "m11_dm1_dungeon_view_light(state, &light)",
        "F0890b_ORCH_ComputeDungeonViewLight_Compat",
        "return 5;",
    ]:
        require(fire, needle, "Firestaff M11 F0337 receipt consumption")
    require(palette_body, "return light.paletteIndex;", "Firestaff M11 palette delegation")
    require(palette_body, "return 5;", "Firestaff M11 palette fallback")

    _, apply_body = find_function(fire, "m11_apply_dungeon_palette_level")
    require(apply_body, "M11_FB_ENCODE(idx, paletteLevel)", "Firestaff whole-viewport palette apply")
    require_absent(fire, "m11_apply_depth_dimming", "Firestaff invented depth dimming")
    render_pos = require(fire, "int paletteIndex = m11_compute_dungeon_palette_index(state);", "Firestaff render palette application")
    render_tail = fire[render_pos:render_pos + 500]
    require_in_order(render_tail, [
        "int paletteIndex = m11_compute_dungeon_palette_index(state);",
        "m11_apply_dungeon_palette_level(framebuffer, framebufferWidth, framebufferHeight,",
        "M11_VIEWPORT_X, M11_VIEWPORT_Y,",
        "M11_VIEWPORT_W, M11_VIEWPORT_H,",
    ], "Firestaff render palette application")

    require(cmake, "NAME v1_viewport_palette_source_lock_gate", "CMake test registration")

    print("V1 viewport palette source-lock gate passed")
    print(f"- Firestaff palette compute: {SRC}:{line_no(fire, fire.find('static int m11_compute_dungeon_palette_index'))}")
    print(f"- Firestaff palette apply: {SRC}:{line_no(fire, fire.find('static void m11_apply_dungeon_palette_level'))}")
    print(f"- ReDMCSB F0337: {RED_PANEL}:{line_no(panel, panel.find('void F0337_INVENTORY_SetDungeonViewPalette'))}")
    print(f"- ReDMCSB light tables: {RED_DATA}:{line_no(data, data.find('int16_t G0039_ai_Graphic562_LightPowerToLightAmount'))}")
    print(f"- ReDMCSB DRAWVIEW palette install: {RED_DRAWVIEW}:{line_no(draw, draw.find('G1010_pui_DungeonViewCurrentPalette'))}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"V1 viewport palette source-lock gate failed: {exc}", file=sys.stderr)
        raise SystemExit(1)

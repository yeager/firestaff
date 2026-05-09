#!/usr/bin/env python3
"""Gate Daniel's DM1 V1 priority: door buttons, TITLE cadence, end sequence."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
main = (root / "main_loop_m11.c").read_text()
view = (root / "m11_game_view.c").read_text()
title = (root / "title_frontend_v1.c").read_text()
end = (root / "endgame_frontend_pc34_compat.c").read_text()
errors = []

def require(haystack: str, needle: str, label: str) -> None:
    if needle not in haystack:
        errors.append(f"missing {label}")

# Door with buttons: draw source button zones only for real door-button doors,
# after the center door panel/ornament path so the button remains visible.
for needle, label in [
    ("m11_draw_dm1_center_doors(state", "center door draw call"),
    ("m11_draw_dm1_center_door_ornaments(state", "center door ornament draw call"),
    ("m11_draw_dm1_center_door_buttons(state", "center door button draw call"),
    ("m11_draw_dm1_d3r_door_button(state", "D3R door button draw call"),
    ("state->world.things->doors[doorIdx].button", "door thing button flag gate"),
    ("M11_GFX_DOOR_BUTTON_BASE", "original door button graphic"),
]:
    require(view, needle, label)
order = [view.find(x) for x in [
    "m11_draw_dm1_center_doors(state",
    "m11_draw_dm1_center_door_ornaments(state",
    "m11_draw_dm1_center_door_buttons(state",
    "m11_draw_dm1_d3r_door_button(state",
]]
if not all(i >= 0 for i in order) or order != sorted(order):
    errors.append("door/buttons draw order is not source-safe")

# TITLE: runtime must consume source timing, not bypass with an unused helper.
require(main, "timing = V1_TitleFrontend_GetSourceTimingEvidence();", "TITLE timing evidence loaded")
if "(void)timing;" in main:
    errors.append("TITLE timing evidence is still unused at runtime")
require(main, "timing.vblankBeforeEachZoomStep", "TITLE vblank cadence used in runtime delay")
require(main, "timing.postZoomVblankCount + timing.finalFadeGuardVblankCount", "TITLE final guard cadence used")
require(title, "V1_TitleFrontend_GetSourceAnimationStepCount", "TITLE source animation schedule")

# End animation: source schedule and runtime overlay must still expose THE END,
# restart/quit controls, champion summary zones, and source timing evidence.
for needle, label in [
    ("ENDGAME_Compat_GetSourceAnimationStepCount", "endgame source animation step count"),
    ("ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT", "THE END blit source event"),
    ("ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER", "restart button source event"),
    ("ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT", "restart wait source event"),
]:
    require(end, needle, label)
for needle, label in [
    ("M11_GameView_GetV1EndgameTheEndGraphicId", "runtime THE END graphic"),
    ("M11_GameView_GetV1EndgameRestartBox", "runtime restart box"),
    ("M11_GameView_GetV1EndgameQuitBox", "runtime quit box"),
    ("M11_GameView_EndgameTitleXForSourceText", "runtime champion title placement"),
]:
    require(view, needle, label)

if errors:
    for e in errors:
        print(f"FAIL: {e}")
    sys.exit(1)
print("ok: DM1 V1 priority gate passed (door buttons, TITLE cadence, end sequence)")

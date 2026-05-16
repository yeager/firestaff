#!/usr/bin/env python3
"""Gate Daniel's DM1 V1 priority: door buttons, TITLE cadence, end sequence."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
red_root = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
main = (root / "src/engine/main_loop_m11.c").read_text()
view = (root / "src/engine/m11_game_view.c").read_text()
title = (root / "src/frontend/title_frontend_v1.c").read_text()
end = (root / "src/frontend/endgame_frontend_pc34_compat.c").read_text()
red_dunview = (red_root / "DUNVIEW.C").read_text(encoding="latin-1", errors="replace")
red_title = (red_root / "TITLE.C").read_text(encoding="latin-1", errors="replace")
red_end = (red_root / "ENDGAME.C").read_text(encoding="latin-1", errors="replace")
red_data = (red_root / "DATA.C").read_text(encoding="latin-1", errors="replace")
errors = []

def require(haystack: str, needle: str, label: str) -> None:
    if needle not in haystack:
        errors.append(f"missing {label}")

def require_order(haystack: str, needles: list[str], label: str) -> None:
    pos = -1
    for needle in needles:
        found = haystack.find(needle, pos + 1)
        if found < 0:
            errors.append(f"missing ordered {label}: {needle}")
            return
        pos = found


# ReDMCSB source audit for Daniel's explicit priority areas. This keeps the
# priority gate anchored in source evidence instead of only checking Firestaff
# implementation names.
for needle, label in [
    ("G0208_aaauc_Graphic558_DoorButtonCoordinateSets", "ReDMCSB door button coordinate table"),
    ("F0110_DUNGEONVIEW_DrawDoorButton", "ReDMCSB door button draw function"),
    ("M634_GRAPHIC_FIRST_DOOR_BUTTON", "ReDMCSB door button graphic base"),
    ("M526_WaitVerticalBlank", "ReDMCSB TITLE vblank primitive"),
    ("F0437_STARTEND_DrawTitle", "ReDMCSB TITLE draw function"),
    ("F0444_STARTEND_Endgame", "ReDMCSB endgame function"),
    ("C006_GRAPHIC_THE_END", "ReDMCSB THE END graphic ordinal"),
    ("G0012_ai_Graphic562_Box_Endgame_TheEnd", "ReDMCSB THE END destination box"),
]:
    if "door" in label or "Door" in label or "C046" in needle:
        require(red_dunview, needle, label)
    elif "TITLE" in label or "M526" in needle:
        require(red_title, needle, label)
    elif "Box_TheEnd" in needle:
        require(red_data, needle, label)
    else:
        require(red_end, needle, label)
require_order(red_title, [
    "F0437_STARTEND_DrawTitle",
    "while (--L1380_i_Counter >= 0)",
    "M526_WaitVerticalBlank();",
    "F0021_MAIN_BlitToScreen",
    "F0022_MAIN_Delay(20);",
    "F0022_MAIN_Delay(2);",
], "ReDMCSB TITLE cadence path")
require_order(red_end, [
    "F0444_STARTEND_Endgame",
    "C006_GRAPHIC_THE_END",
    "F0436_STARTEND_FadeToPalette",
    "F0022_MAIN_Delay(300)",
    "AL1409_i_VerticalBlankCount = 900",
], "ReDMCSB end animation/restart path")

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
require(main, "V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing)", "TITLE vblank cadence helper used in runtime delay")
require(main, "V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing)", "TITLE final guard cadence helper used")
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

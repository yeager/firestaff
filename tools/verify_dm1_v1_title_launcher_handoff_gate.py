#!/usr/bin/env python3
"""Gate: DM1 launch must not bypass ReDMCSB TITLE/swoosh.

This is intentionally a source-shape regression gate. The user-visible bug was
that selecting DM1 from Firestaff's launcher reached the game/entrance without
playing the original TITLE animation or title audio cue. ReDMCSB's startup path
runs F0437_STARTEND_DrawTitle before F0441_STARTEND_ProcessEntrance, so the
Firestaff launcher handoff must call the TITLE frontend after DM1 GRAPHICS.DAT
is loaded and before the entrance transition. This keeps the ReDMCSB
title-before-entrance order while allowing runtime to use C001_GRAPHIC_TITLE,
the bitmap that TITLE.C actually animates.
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
main = (root / "src/engine/main_loop_m11.c").read_text()
frontend = (root / "src/frontend/title_frontend_v1.c").read_text()
title_h = (root / "include/title_dat_loader_v1.h").read_text()
startup = (root / "src/dm1/dm1_v1_startup_sequence_pc34_compat.c").read_text()
pathfinder = (root / "src/frontend/v1_title_intro_pathfinder_pc34_compat.c").read_text()

errors = []

def function_body(source, name):
    """Return a complete C function body, including nested blocks."""
    match = re.search(rf"\b{name}\s*\([^;]*?\)\s*\{{", source, re.S)
    if not match:
        return None
    start = source.find("{", match.start())
    depth = 0
    for index in range(start, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start + 1:index]
    return None

body = function_body(main, "m11_open_requested_launch")
if body is None:
    errors.append("m11_open_requested_launch() not found")
else:
    for needle in [
        "dm1HandoffCallbacks.play_title = m11_dm1_handoff_play_title",
        "dm1HandoffCallbacks.play_entrance = m11_dm1_handoff_play_entrance",
        "dm1_v1_startup_execute_selected_launch_transaction_pc34",
    ]:
        if needle not in body:
            errors.append(f"launcher does not wire DM1-owned TITLE/entrance transaction: {needle}")

for name, needle in [
    ("m11_dm1_handoff_play_title", "m11_play_redmcsb_title_intro_if_available"),
    ("m11_dm1_handoff_play_entrance", "m11_play_redmcsb_entrance_transition"),
]:
    body = function_body(main, name)
    if body is None or needle not in body:
        errors.append(f"DM1 host callback does not consume its runtime helper: {name}")

post_launch = function_body(startup, "dm1_v1_startup_execute_handoff_post_launch_pc34")
if post_launch is None:
    errors.append("DM1 post-launch transaction not found")
else:
    title_idx = post_launch.find("callbacks->play_title")
    entrance_idx = post_launch.find("callbacks->play_entrance")
    if title_idx < 0 or entrance_idx < 0 or title_idx >= entrance_idx:
        errors.append("DM1-owned transaction must call TITLE before entrance")
    if "F0437_STARTEND_DrawTitle precedes" not in post_launch:
        errors.append("DM1 transaction must retain ReDMCSB title-before-entrance evidence")

body = function_body(main, "M11_PhaseA_Run")
if body:
    initial_draw_idx = body.find("m11_draw_launcher(&menuState")
    early_title_idx = body.find("m11_play_redmcsb_title_intro_if_available(&menuState")
    if early_title_idx >= 0 and (initial_draw_idx < 0 or early_title_idx < initial_draw_idx):
        errors.append("TITLE intro still plays before the launcher is first drawn; it belongs at launcher->DM1 handoff")
else:
    errors.append("M11_PhaseA_Run() not found")

body = function_body(main, "m11_play_redmcsb_title_intro_if_available")
if body is None:
    errors.append("m11_play_redmcsb_title_intro_if_available() not found")
else:
    for needle in [
        "M11_Audio_Init(&titleAudio)",
        "M11_Audio_PlayTitleMusic(&titleAudio)",
        "M11_Audio_Shutdown(&titleAudio)",
        "V1_TitleFrontend_RenderFrameToScreen",
        "M11_Render_PresentIndexed",
        "M11_RENDER_OK",
    ]:
        if needle not in body:
            errors.append(f"TITLE intro missing required runtime step: {needle}")

body = function_body(main, "m11_play_redmcsb_title_graphic_intro_if_available")
if body is None:
    errors.append("m11_play_redmcsb_title_graphic_intro_if_available() not found")
else:
    for needle in [
        "M11_AssetLoader_Load(&gameView->assetLoader, 1U)",
        "V1_TitleFrontend_GetSourceAnimationStep",
        "V1_TitleFrontend_GetC001BlitPlanForStep",
        "dm1_v1_startup_title_presentation_command_pc34",
        "V1_TITLE_FRONTEND_C001_BLIT_REGION",
        "M11_AssetLoader_BlitRegion(titleGraphic",
        "V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION",
        "M11_AssetLoader_BlitSubRectScaled(titleGraphic",
        "VGA_PALETTE_PC34_SPECIAL_TITLE",
        "M11_Audio_PlayTitleMusic(&titleAudio)",
        "command.post_present_delay_ms",
    ]:
        if needle not in body:
            errors.append(f"GRAPHICS.DAT C001 TITLE intro missing source runtime step: {needle}")


for needle in [
    "v1_title_intro_candidate_is_valid",
    "V1_Title_IsCanonicalPc34Title(path",
]:
    if needle not in (main + startup + pathfinder):
        errors.append(f"TITLE path selection is not hash/provenance gated: {needle}")

for needle in [
    "V1_TITLE_PC34_CANONICAL_SHA256",
    "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    "V1_TITLE_PC34_CANONICAL_FNV1A32",
]:
    if needle not in title_h:
        errors.append(f"TITLE canonical identity constant missing: {needle}")

for needle in [
    "TITLE.C:340-360 builds 18 shrinked bitmaps",
    "TITLE.C:385-387 waits VBlank then blits in reverse order",
    "TITLE.C:397-402 blits Master/Strikes Back",
]:
    if needle not in frontend:
        errors.append(f"TITLE frontend source-lock evidence missing: {needle}")

if errors:
    for e in errors:
        print(f"FAIL: {e}", file=sys.stderr)
    sys.exit(1)
print("ok: DM1 V1 launcher handoff runs ReDMCSB TITLE animation/audio before game/entrance")

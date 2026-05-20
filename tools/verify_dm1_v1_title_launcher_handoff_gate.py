#!/usr/bin/env python3
"""Gate: DM1 launch must not bypass ReDMCSB TITLE/swoosh.

This is intentionally a source-shape regression gate. The user-visible bug was
that selecting DM1 from Firestaff's launcher reached the game/entrance without
playing the original TITLE animation or title audio cue. ReDMCSB's startup path
runs F0437_STARTEND_DrawTitle before F0441_STARTEND_ProcessEntrance, so the
Firestaff launcher handoff must call the TITLE frontend before opening the game
view / entrance transition.
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
main = (root / "src/engine/main_loop_m11.c").read_text()
frontend = (root / "src/frontend/title_frontend_v1.c").read_text()
title_h = (root / "include/title_dat_loader_v1.h").read_text()

errors = []

m = re.search(r"static int m11_open_requested_launch\([^)]*\) \{(?P<body>.*?)\n\}", main, re.S)
if not m:
    errors.append("m11_open_requested_launch() not found")
else:
    body = m.group("body")
    title_idx = body.find("m11_play_redmcsb_title_intro_if_available(menuState")
    open_idx = body.find("M11_GameView_OpenSelectedMenuEntry(gameView, menuState)")
    entrance_idx = body.find("m11_play_redmcsb_entrance_transition(gameView")
    if title_idx < 0:
        errors.append("launcher handoff does not call m11_play_redmcsb_title_intro_if_available")
    if open_idx < 0:
        errors.append("launcher handoff does not open selected game view")
    if title_idx >= 0 and open_idx >= 0 and not title_idx < open_idx:
        errors.append("TITLE intro must run before M11_GameView_OpenSelectedMenuEntry")
    if title_idx >= 0 and entrance_idx >= 0 and not title_idx < entrance_idx:
        errors.append("TITLE intro must run before entrance transition")
    if "m11_play_redmcsb_entrance_transition(gameView, 1200)" not in body:
        errors.append("modern launcher must auto-confirm entrance wait shortly after explicit Launch")
    guard_start = body.rfind("M12_StartupMenu_GetPresentationMode", 0, title_idx if title_idx >= 0 else 0)
    if title_idx >= 0 and guard_start >= 0:
        errors.append("TITLE intro call is still guarded by presentation mode; DM1 TITLE must run before Entrance in every DM1 presentation mode")
    if title_idx >= 0 and 'strcmp(launchEntry->gameId, "dm1") == 0' not in body:
        errors.append("TITLE intro call must remain guarded to DM1 launches only")

phase = re.search(r"int M11_PhaseA_Run\([^)]*\) \{(?P<body>.*?)\n\}", main, re.S)
if phase:
    body = phase.group("body")
    initial_draw_idx = body.find("m11_draw_launcher(&menuState")
    early_title_idx = body.find("m11_play_redmcsb_title_intro_if_available(&menuState")
    if early_title_idx >= 0 and (initial_draw_idx < 0 or early_title_idx < initial_draw_idx):
        errors.append("TITLE intro still plays before the launcher is first drawn; it belongs at launcher->DM1 handoff")
else:
    errors.append("M11_PhaseA_Run() not found")

intro = re.search(r"static void m11_play_redmcsb_title_intro_if_available\([^)]*\) \{(?P<body>.*?)\n\}", main, re.S)
if not intro:
    errors.append("m11_play_redmcsb_title_intro_if_available() not found")
else:
    body = intro.group("body")
    for needle in [
        "M11_Audio_Init(&titleAudio)",
        "M11_Audio_PlayTitleMusic(&titleAudio)",
        "M11_Audio_Shutdown(&titleAudio)",
        "V1_TitleFrontend_RenderFrameToScreen",
        "M11_Render_PresentIndexed",
    ]:
        if needle not in body:
            errors.append(f"TITLE intro missing required runtime step: {needle}")


for needle in [
    "V1_Title_IsCanonicalPc34Title(envPath",
    "V1_Title_IsCanonicalPc34Title(candidate",
]:
    if needle not in main:
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

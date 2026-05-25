# Nexus V1 Title Screen — Source-Locked Audit

## Sources
- src/frontend/title_frontend_v1.c (DM1 reference)
- docs/nexus_menus.md (menu system status)
- docs/nexus_overview.md (graphics architecture)
- src/nexus/nexus_v1_viewport.c, nexus_v1_rasterizer.c

---

## 1. Nexus Title Screen — Original Saturn

The Nexus title screen is a **3D animated scene**, not a static bitmap:

- A 3D dungeon corridor/room rendered in real-time by the VDP1 software
  rasterizer
- The word "DUNGEON MASTER NEXUS" rendered as 3D text polygons or sprite labels
- Camera slowly moves or rotates through a dungeon environment
- This is different from DM1 which uses a static 2D bitmap title
- The title screen is part of the game executable, not a separate asset

---

## 2. DM1 Title Screen (Reference)

DM1 title (source-locked from ReDMCSB):
- File: TITLE.C, function F0437_STARTEND_DrawTitle
- Static 2D bitmap logo: "DUNGEON MASTER" in stylized text
- Pre-rendered graphic, not real-time 3D
- No animation (static image until player presses key)

DM1 title is a flat 2D sprite-based render — the complete opposite of
Nexus's real-time 3D approach.

---

## 3. Firestaff Title Screen Implementation

**NOT YET IMPLEMENTED.**

The docs/nexus_menus.md states clearly:
> "Nexus title screen displays a 3D dungeon logo (different from DM1s 2D logo).
> The Saturn version has a unique animated title rendered as a 3D scene,
> not a static bitmap. No Nexus title screen implementation exists."

src/frontend/ contains title_frontend_v1.c which handles DM1 PC 3.4 title
but there is no equivalent nexus_v1_title.c.

---

## 4. Title Screen vs Intro Movie

Nexus does NOT have a separate intro movie before the title screen.
The title screen itself IS the intro — a 3D animated logo displayed
immediately after nexus_v1_init() completes. There is no DMV0.AVI
played before the title; DMV files are in-game cutscenes only.

---

## 5. Title Screen Architecture

### Original Saturn
- VDP1 renders 3D scene at 320x224 (NTSC) or 320x240 (PAL)
- Software rasterizer runs on SH-2 (no GPU — all CPU rendering)
- 3D text labels rendered with FONT256.S2D font
- Camera animation: slow dolly/rotate on a timer

### Firestaff PC
- SDL_CreateWindow for display
- nexus_v1_viewport_render() — same software rasterizer, 320x200 output
- Title state would call nexus_v1_viewport_render() in a loop with
  animated camera parameters
- No nexus_v1_title.c or equivalent exists

---

## 6. What Needs to be Built

1. nexus_v1_title_screen_state_t enum and state machine entry
2. nexus_v1_title_render() — 3D dungeon scene with animated camera
3. nexus_v1_title_input() — detect keypress/mouse to dismiss and go to
   champion selection
4. Bridge title state with nexus_v1_engine game loop
5. Japanese text rendering for title logo (FONT256.S2D already loaded)

---

## 7. Comparison Table

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Title type | Static 2D bitmap | Real-time 3D animated |
| Source | TITLE.C asset | Rendered by VDP1 rasterizer |
| Animation | None (static) | Camera dolly/rotate |
| Language | English | Japanese (Shift-JIS) |
| Impl status | Complete (ReDMCSB) | NOT IMPLEMENTED |
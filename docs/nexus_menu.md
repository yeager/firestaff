# Nexus V1 Menu System — Source-Locked Audit

## Sources
- src/nexus/nexus_v1_engine.c, nexus_v1_game.c
- docs/nexus_menus.md (full menu audit)
- docs/nexus_overview.md
- src/frontend/title_frontend_v1.c, dialog_frontend_pc34_compat.c (DM1 reference)
- docs/menu_startup.md, docs/dm2_menu.md (reference comparisons)

---

## 1. Menu System Overview

**Nexus V1 has no menu system implemented in the Firestaff codebase.**

The engine (nexus_v1_engine.c) handles initialization, file loading, level
loading, and game tick — but no menu, dialog, or front-end state machine
exists in src/nexus/. The front-end files implement DM1 V1 (PC 3.4) compatible
menus only. No Nexus-specific menu front-end has been written.

---

## 2. Expected Menu Flow (from DM1/CSB pattern)

Based on the DM1 menu flow and Nexus-specific features:

```
Title Screen (3D animated)
  -> Champion Selection (Hall of Champions, 24 JP champions)
     -> Dungeon Entrance (3D entrance scene, party formation)
        -> In-Game UI (champion panels, inventory, spell list)
           -> ESC Menu (pause, stats, save/load)
```

---

## 3. Required Menu States

### Title/Start State
- 3D animated logo (see nexus_title.md)
- New Game / Continue / Options buttons
- No FMV before title (Nexus has no boot-time intro)

### Champion Select State
- 24 Japanese champions (Syra, Leyla, Nabi, etc.)
- Shift-JIS text rendering for names
- Party formation (up to 4 champions)
- Similar to DM1's SELECTOR.C but with Japanese names and 3D portraits

### Options State
- Display mode (fullscreen / windowed)
- Audio levels (music, SFX)
- See nexus_options.md for full options audit

### In-Game ESC Menu
- Champion stats panel
- Inventory management
- Spell list / casting
- Save / Load
- Return to game

### End Game / Credits
- DMV0-2.AVI cutscenes (NOT IMPLEMENTED)
- Credits screen

---

## 4. DM1 Menu Reference (Source-Locked)

DM1 menus driven by front-end, source-locked from ReDMCSB:

| Menu | File | Entry Point |
|------|------|-------------|
| Title Screen | TITLE.C | F0437_STARTEND_DrawTitle |
| Entrance (New/Continue) | ENTRANCE.C | F0441_STARTEND_ProcessEntrance |
| In-Game ESC Menu | MENU.C | F0361_COMMAND_ProcessKeyPress |
| Champion Selection | SELECTOR.C | (in-game) |
| Save/Load | STARTUP2.C | F0435_STARTEND_LoadGame |

DM1 V1 menus are sprite-based 2D — Nexus menus must be 3D-aware
due to the Saturn VDP1/VDP2 hardware compositing model.

---

## 5. Front-End vs Engine Separation

In Firestaff, Nexus engine is decoupled from the front-end:
- Engine (src/nexus/): 3D viewport, game logic, dungeon state, combat, AI
- Front-end (src/frontend/): menus, UI, input handling, save/load

A Nexus menu layer must bridge:
- Menu state machine (front-end concern)
- Game state machine (engine concern)
- 3D viewport rendering (engine renders, front-end composites)

No nexus_v1_menu.c or equivalent exists yet.

---

## 6. Not Yet Implemented

- nexus_v1_menu_state_t enum and state machine
- Title screen renderer (nexus_v1_title_render)
- Champion selection UI (3D portraits, party formation)
- In-game ESC menu
- Save/load menu (Saturn SRAM format)
- Options/settings menu
- FMV cutscene playback for DMV0-2.AVI
- Credits screen

---

## 7. DM1 vs Nexus Menu Comparison

| Feature | DM1 | Nexus V1 |
|---------|-----|----------|
| Title screen | 2D bitmap logo | 3D animated (not impl) |
| Start menu options | New Game / Continue | TBD |
| Champion roster | Western names (24) | Japanese names (24) |
| Champion select UI | Sprite-based | Not impl |
| In-game menu | ESC key, 2D panel | Not impl |
| Save/load | Binary slot files | Saturn SRAM |
| Credits | Static bitmap | AVI cutscenes |
| Menu rendering | SDL blit | VDP1/VDP2 (not impl) |
| Font | ASCII | Japanese Shift-JIS |
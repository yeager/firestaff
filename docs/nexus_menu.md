# Nexus V1 Menu System — Source-Locked Audit

## Sources
- src/nexus/nexus_v1_engine.c, nexus_v1_game.c
- docs/nexus_menus.md (full menu audit)
- docs/nexus_overview.md
- src/frontend/title_frontend_v1.c, dialog_frontend_pc34_compat.c (DM1 reference)
- docs/menu_startup.md, docs/dm2_menu.md (reference comparisons)

---

## 1. Menu System Overview

The startup state machine is implemented in `src/nexus/nexus_v1_startup_menu.c`
and is exercised by the Nexus startup tests. It owns save-slot discovery,
new-game/continue selection, champion selection, input receipts, and the
transition into the engine. `src/nexus/nexus_v1_launcher.c` owns the real-data
asset gate.

This is a host-side state/input route, not proof of Saturn menu presentation.
The current startup layout helpers are compatibility hit regions; they are
not claimed as retail screen coordinates. `MENU.BPK`, FONT256/S2D mapping,
palette ownership, and VDP1/VDP2 placement remain capture-gated.

---

## 2. Verified menu boundary

No retail Nexus menu sequence is promoted from the DM1/CSB flow. The local
Nexus corpus proves startup assets, a DM.BIN hit-rectangle table and 20 PLRD
records, but it does not yet prove the Saturn screen order, champion-selection
consumer, or the VDP1/VDP2 composition of those assets. Those relationships
remain capture-gated.

---

## 3. Required Menu States

### Title/Start State
- Real TITLE.BIN/TITLE.CG and WARNING/GAMEOVER source loading is present.
- New Game / Continue and champion-selection state transitions are present.
- Saturn tilemap/CLUT placement and final pixels remain blocked without an
  authenticated original capture.

### Champion Select State
- 20 PLRD records are present in the real Nexus data; their runtime menu
  ordering and screen consumer are not yet authenticated.
- Japanese text rendering, party formation and portrait placement remain
  unavailable until a real text/VDP1 consumer trace binds them.

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

There is no separate `nexus_v1_menu.c`; the startup menu is deliberately
split between `nexus_v1_startup_menu.c`, `nexus_v1_startup_layout.c`, and the
launcher. This keeps state transitions testable without pretending that host
rectangles are Saturn VDP coordinates.

---

## 6. Still blocked or incomplete

- Authenticated Saturn menu renderer and VDP1/VDP2 placement
- Champion-selection Saturn presentation (3D/VDP1 portraits, party formation)
- In-game ESC menu
- Save/load menu (Saturn SRAM format)
- Options/settings menu
- FMV cutscene playback for DMV0-2.AVI
- Credits screen

---

## 7. DM1 vs Nexus Menu Comparison

| Feature | DM1 | Nexus V1 |
|---------|-----|----------|
| Title screen | 2D bitmap logo | Real source decode; Saturn placement blocked |
| Start menu options | New Game / Continue | Host state/input route implemented |
| Champion roster | Western names (24) | 20 PLRD records |
| Champion select UI | Sprite-based | State/input route; Saturn presentation blocked |
| In-game menu | ESC key, 2D panel | Not impl |
| Save/load | Binary slot files | Saturn SRAM |
| Credits | Static bitmap | AVI cutscenes |
| Menu rendering | SDL blit | VDP1/VDP2 (not impl) |
| Font | ASCII | Japanese Shift-JIS |

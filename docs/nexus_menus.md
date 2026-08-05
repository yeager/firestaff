# Nexus V1 — Menu System Audit

## Sources
- `src/nexus/nexus_v1_engine.c`, `src/nexus/nexus_v1_game.c`
- `docs/nexus_overview.md`, `docs/nexus_features.md`
- `src/frontend/title_frontend_v1.c` (title screen)
- `src/frontend/dialog_frontend_pc34_compat.c` (dialog)
- `docs/menu_startup.md` (DM1 reference), `docs/dm2_menu.md` (DM2 reference)

## Overview

Nexus V1 has a host-side startup state/input route in
`src/nexus/nexus_v1_startup_menu.c`, with asset gating in
`src/nexus/nexus_v1_launcher.c`. It covers save/new-game selection, champion
selection, input receipts, and engine transition. This does not claim that the
retail Saturn menu pixels, CLUT ownership, or VDP1/VDP2 placement are known.

## DM1 Menu System Reference

DM1 menus driven by the front-end, source-locked from ReDMCSB:

| Menu | File | Entry Point |
|------|------|-------------|
| Title Screen | TITLE.C | F0437_STARTEND_DrawTitle |
| Entrance (New Game / Continue) | ENTRANCE.C | F0441_STARTEND_ProcessEntrance |
| In-Game ESC Menu | MENU.C | F0361_COMMAND_ProcessKeyPress |
| Champion Selection | SELECTOR.C | (in-game) |
| Save/Load | STARTUP2.C | F0435_STARTEND_LoadGame |

Menu flow: Title -> Entrance (New/Continue) -> Party creation -> Dungeon load -> In-game ESC menu

## Nexus-Specific Menu Considerations

### Title Screen
Real TITLE.BIN/TITLE.CG and WARNING/GAMEOVER assets are loaded and structurally
decoded. The Saturn tilemap/CLUT/runtime placement remains capture-gated; no
3D title interpretation is claimed without that evidence.

### Menu State Machine
A Nexus menu system needs states for:
- Title/Start: 3D animated logo, New Game / Continue / Options
- Champion Select: source-owned champion records (20 live PLRD/TABL records)
- Options: Display mode (fullscreen/windowed), audio levels
- In-Game Menu: ESC pause with stats, inventory, spells, save
- End Game / Credits: DMV0-2.AVI cutscenes

There is no separate `nexus_v1_menu.c`; the startup route is intentionally
split between the startup menu, layout helpers, and launcher.

### DM1 Logic vs Saturn UI
Nexus inherits DM1 game logic but the Saturn UI runs on SH-2 with VDP1/VDP2
hardware compositing. In Firestaff, the Nexus engine is decoupled from the
front-end — a Nexus menu layer must bridge front-end (menu/UI) with the
Nexus engine (3D viewport + logic).

### Champion Roster (nexus_v1_champions.c)
Nexus champion names and count come from the real PLRD/TABL resource
records: 20 live records, with 24 slots retained only for storage capacity.
Champion selection state/input exists; Saturn presentation remains blocked.

## What Exists vs Whats Missing

Implemented:
- Engine initialization with font loading (nexus_v1_init)
- Level loading from LEV*.DGN files
- 3D viewport source/material plan and fail-closed renderer route
- DMDF model loading for creatures
- CD audio/SAL provenance and selection management (playback remains capture-gated)

Still blocked or incomplete:
- Saturn title/start-menu renderer and VDP1/VDP2 placement
- Champion selection Saturn presentation (party creation pixels)
- In-game ESC menu
- Save/load menu (Saturn SRAM format)
- Options/settings menu
- FMV cutscene playback (DMV0-2.AVI)
- Credits screen

## DM1 vs Nexus Menu Comparison

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

## Next Steps
1. Authenticate Saturn menu command/palette/placement capture
2. Bind the existing startup state/input route to that capture
3. Authenticate champion portrait and HUD VDP1 ownership
4. Bridge only source-proven menu transitions into the engine game loop
5. Implement save/load using an authenticated Saturn SRAM format
6. Add FMV playback only after authentic DMV0-2.AVI ownership is proven

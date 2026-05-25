# Nexus V1 — Menu System Audit

## Sources
- `src/nexus/nexus_v1_engine.c`, `src/nexus/nexus_v1_game.c`
- `docs/nexus_overview.md`, `docs/nexus_features.md`
- `src/frontend/title_frontend_v1.c` (title screen)
- `src/frontend/dialog_frontend_pc34_compat.c` (dialog)
- `docs/menu_startup.md` (DM1 reference), `docs/dm2_menu.md` (DM2 reference)

## Overview

Nexus V1 has **no menu system implemented** in the Firestaff codebase.
`nexus_v1_engine.c` handles initialization, file loading, level loading, and
game tick — but no menu, dialog, or front-end state machine exists in
`src/nexus/`. The front-end files implement DM1 V1 (PC 3.4) compatible menus only.
No Nexus-specific menu front-end has been written.

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
Nexus title screen displays a 3D dungeon logo (different from DM1s 2D logo).
The Saturn version has a unique animated title rendered as a 3D scene,
not a static bitmap. No Nexus title screen implementation exists.

### Menu State Machine
A Nexus menu system needs states for:
- Title/Start: 3D animated logo, New Game / Continue / Options
- Champion Select: 8 Japanese champions (Syra, Leyla, Nabi, etc.)
- Options: Display mode (fullscreen/windowed), audio levels
- In-Game Menu: ESC pause with stats, inventory, spells, save
- End Game / Credits: DMV0-2.AVI cutscenes

No nexus_v1_menu.c or equivalent exists yet.

### DM1 Logic vs Saturn UI
Nexus inherits DM1 game logic but the Saturn UI runs on SH-2 with VDP1/VDP2
hardware compositing. In Firestaff, the Nexus engine is decoupled from the
front-end — a Nexus menu layer must bridge front-end (menu/UI) with the
Nexus engine (3D viewport + logic).

### Champion Roster (nexus_v1_champions.c)
Nexus has 24-champion roster with Japanese names and Shift-JIS text.
No champion selection UI exists in the codebase.

## What Exists vs Whats Missing

Implemented:
- Engine initialization with font loading (nexus_v1_init)
- Level loading from LEV*.DGN files
- 3D viewport renderer (nexus_v1_viewport_render)
- DMDF model loading for creatures
- CD audio track management (no playback yet)

Not Yet Implemented:
- Title screen / start menu
- Champion selection screen (party creation)
- In-game ESC menu
- Save/load menu (Saturn SRAM format)
- Options/settings menu
- FMV cutscene playback (DMV0-2.AVI)
- Credits screen

## DM1 vs Nexus Menu Comparison

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

## Next Steps
1. Define nexus_v1_menu_state_t enum and state machine
2. Implement title screen renderer
3. Implement champion selection UI
4. Bridge menu state machine with nexus_v1_engine game loop
5. Implement save/load using Saturn SRAM format (SRAM.BIN)
6. Add FMV playback for DMV0-2.AVI cutscenes

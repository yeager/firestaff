# Nexus V1 — Menu System Audit

## Sources
- `src/nexus/nexus_v1_engine.c`, `src/nexus/nexus_v1_game.c`
- `docs/nexus/nexus_overview.md`, `docs/nexus/nexus_features.md`
- `src/frontend/title_frontend_v1.c` (title screen)
- `src/frontend/dialog_frontend_pc34_compat.c` (dialog)
- `docs/shared/menu_startup.md` (DM1 reference), `docs/dm2/dm2_menu.md` (DM2 reference)

## Overview

Nexus V1 has host-side startup routing in `src/nexus/nexus_v1_startup_menu.c`,
with asset gating in `src/nexus/nexus_v1_launcher.c`. The reducer in `src/nexus/nexus_v1_retail_title_menu.c` models the
observed title prompt opening the three-choice menu, New Game selected by
default, Down to Continue, Down to Configuration, and documented Accept
results. M11 now retains that selection state and consumes those keyboard
inputs separately from the generic Firestaff save/champion menu. This is an
input-state integration only: it does not draw the selected retail menu, and
New Game, Continue, and Configuration still have no production destination
execution route. Continue availability deliberately does not use Firestaff save files;
the Saturn BKR intake and payload-to-engine route are not integrated here.
The historical New Game comparison reported five TITLE.BIN draws and 1,495
matching nontransparent screenshot pixels. The current
`verify_nexus_title_sprite_screenshot.py` verifier checks the five-draw
Japanese Continue layout, including per-pixel CRAM color and priority; it does
not verify the New Game layout. The retail Continue route also reached a
loaded dungeon with a BKR image whose hash matches the retained four-champion
image in `.firestaff/data/nexus`; its transfer chain was not retained.
Full-frame Firestaff rendering, option behavior, and mouse targeting remain
unverified.

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
- Title/Start: authentic New Game/Continue/Configuration states and bounded
  pad transitions are recorded; the reducer is not wired into runtime startup.
  Five source-bound sprite draws are verified for Japanese Continue. New Game
  has historical source-comparison evidence; full menu composition, options
  behavior, and mouse targeting remain unbound.
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

Present as bounded host/source diagnostics:
- Engine initialization with font loading (nexus_v1_init)
- Level loading from LEV*.DGN files
- 3D viewport source/material plan and fail-closed renderer route
- DMDF model loading for creatures
- CD audio/SAL provenance and selection management (playback remains capture-gated)

Still blocked or incomplete:
- Saturn title/start-menu renderer and VDP1/VDP2 placement
- Champion selection Saturn presentation (party creation pixels)
- In-game ESC menu
- Firestaff Saturn BKR menu and engine handoff (directory/block-chain parsing
  and exact payload extraction are implemented)
- Options/settings menu
- FMV cutscene playback (DMV0-2.AVI)
- Credits screen

## DM1 vs Nexus Menu Comparison

| Feature | DM1 | Nexus V1 |
|---------|-----|----------|
| Title screen | 2D bitmap logo | Real source decode; Saturn placement blocked |
| Start menu options | New Game / Continue | M11 selection input state; renderer and destination routes blocked |
| Champion roster | Western names (24) | 20 PLRD records |
| Champion select UI | Sprite-based | State/input route; Saturn presentation blocked |
| In-game menu | ESC key, 2D panel | No source-bound Nexus route |
| Save/load | Binary slot files | Saturn SRAM |
| Credits | Static bitmap | AVI cutscenes |
| Menu rendering | SDL blit | VDP1/VDP2 consumer unbound |

## Next Steps
1. Authenticate Saturn menu command/palette/placement capture
2. Bind the existing startup state/input route to that capture
3. Authenticate champion portrait and HUD VDP1 ownership
4. Bridge only source-proven menu transitions into the engine game loop
5. Implement save/load using an authenticated Saturn SRAM format
6. Add FMV playback only after authentic DMV0-2.AVI ownership is proven

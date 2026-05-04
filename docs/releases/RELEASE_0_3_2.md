# Firestaff v0.3.2

**Release date:** 2026-05-04

## Highlights

59 DM1 V1 source-locked modules — the most complete recreation of Dungeon Master's original engine to date. This release adds the engine integration layer, movement command dispatch, and core subsystems needed for a playable game loop.

## New DM1 V1 modules (since v0.3.1)

All traced to ReDMCSB WIP20210206 source files:

- **Engine integration layer** (`dm1_v1_engine`) — master init/tick/shutdown wiring all 58+ subsystems following DM.C F0449 init order and GAMELOOP.C F0002 tick order
- **Dungeon data store** (`dm1_v1_dungeon_data`) — central aggregator for map, party, champions, groups, events, objects, game time
- **Movement command dispatch** (`dm1_v1_movement`) — COMMAND.C circular queue (G0432-G0435), CLIKMENU.C turn/move execution, VBLANK.C step timing with 47 source-locked line references
- **Game state machine** (`dm1_v1_game_state`) — STARTUP1/2.C 12-state FSM with validated transitions
- **Input polling** (`dm1_v1_input_poll`) — INPUT.C 64-entry ring buffer (F1097-F1099), mouse tracking, numpad remap
- **Game over & title** (`dm1_v1_game_over`) — ENDGAME.C death effects, TITLE.C 18-step swoosh zoom
- **Game loop & frame timing** (`dm1_v1_game_loop`) — VBLANK.C F0577 interrupt sim, G2586 pause/resume, F0002 tick pipeline
- **Dungeon decompressor** (`dm1_v1_dungeon_decompressor`) — DECOMPDU.C F0455, G0525-G0532 file header parsing
- **Field/teleporter effects** (`dm1_v1_field_teleporter_effect`) — MOVESENS.C F0263 teleporter trigger, PROJEXPL.C particles
- **Creature viewport** (`dm1_v1_creature_viewport`) — GROUP.C G0217 creature table (21 types), depth-scaled sprites
- **Save/load system** (`dm1_v1_save_load_system`) — LOADSAVE.C F0433/F0434, 20-slot file management
- **Blit/fill primitives** (`dm1_v1_blit_fill`) — BLIT.C F0132 with transparency/XOR/flip, FILLBOX.C F0133
- **Dialog/scroll messages** (`dm1_v1_dialog_scroll`) — DIALOG.C G2062 dialog sets, SCRLMGMT.C queue
- **Click/mouse routing** (`dm1_v1_click_routing`) — CLIKVIEW.C F0372, CLIKCHAM.C F0367, COMMAND.C zone dispatch
- **Graphics loader** (`dm1_v1_graphics_loader`) — LZW decompression + GRAPHICS.DAT bitmap extraction
- **Fade/transition** (`dm1_v1_fade_transition`) — screen fade, swoosh overlay effects
- **Portrait panel** (`dm1_v1_portrait_panel`) — champion portrait + HUD rendering
- **Wall ornament** (`dm1_v1_wall_ornament`) — alcove/ornament depth-coordinate rendering

## i18n / Localization

- PO-based runtime translation system active (no libintl dependency)
- Domains: `dm1`, `csb`, `dm2`, `startup-menu`
- Swedish (`sv`) and English (`en`) catalogs seeded
- Fallback chain: active language → English → key text

## Build

- 59 `dm1_v1_*_pc34_compat` modules
- Clean build with `-Wall -Wextra` on GCC 14 and Clang 17
- CI: GitHub Actions green (macOS + Linux + Windows cross-compile)

## Stats

- ~8,500 new lines of source-locked C code (this release)
- ~25,000 total DM1 V1 lines across all 59 modules

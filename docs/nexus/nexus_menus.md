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
default, vertical navigation, and documented Accept results. M11 retains that
selection state and consumes keyboard input separately from Firestaff's
generic save/champion menu. Up/Down and W/S update the internal selection;
English retail Ymir evidence confirms Down to Continue and Up back to New Game.
Firestaff draws only the capture-verified foreground sprites for English New
Game/Continue and Japanese New Game/Continue/Configuration selections. The
shared background and full-screen composition remain unbound. New Game Accept
now reaches the authenticated `DM.BIN`/`LEV01.DGN` loader and records the
retail level-1 start pose for keyboard/controller and mouse input, but the
viewport/HUD handoff still restores the title. Continue/save loading and
Configuration destination behavior are not integrated. Continue availability
deliberately does not use Firestaff save files; the Saturn BKR intake and
payload-to-engine route are not integrated here.
The `verify_nexus_title_sprite_screenshot.py` verifier checks the Japanese and
English New Game/Continue layouts, including retail `TITLE.BIN` offsets, VDP1
placement, CRAM color, priority, English 16-color palettes against exact
`TITLE.BIN` ranges, and every visible sprite pixel. The dedicated
`verify_nexus_japanese_configuration_capture.py` checks the Japanese
Configuration-selected state. The icon palette is unique at
`0x1486`; seven identical copies of the text palette are present in `TITLE.BIN`.
The English layouts each match 1,451
pixels in authentic Ymir captures. Separate Japanese New Game and Configuration
verifiers cover their captured foreground layouts. The background bitmap and
full-screen composition remain unbound, so only the per-state foreground
renderer is admitted. See the [English title-menu sprite receipt](../../parity-evidence/verification/nexus_english_title_menu_sprite_join_20261027/README.md).
The paired New Game/Continue captures share byte-identical VDP2 VRAM, CRAM,
and registers; only VDP1 memory/framebuffer/command bytes differ. This confirms
the selection change is confined to the observed sprite hardware state while
leaving the shared NBG0 source unbound.
The Ymir retail Continue route also reached a loaded dungeon with a BKR image
whose hash matches the retained four-champion image in `.firestaff/data/nexus`;
its transfer chain was not retained. This is reference-emulator evidence, not
Firestaff Continue support. Full-frame Firestaff menu composition, setting
behavior, and mouse targeting outside the verified row remain unverified.

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
  retail transitions are recorded. M11 wires keyboard Action and vertical
  selection into an internal reducer, with source-bound foreground rendering
  for English New Game/Continue and Japanese New Game/Continue/Configuration.
  The shared background and full composition remain unbound. New Game reaches
  the authenticated level-start loader but remains blocked by viewport/HUD
  receipts; Continue/save and Configuration destinations remain unimplemented.
  Options behavior and mouse targeting outside the verified rows also remain
  unverified.
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
- Complete Saturn title/start-menu composition and its VDP1/VDP2 background;
  only source-bound per-state foreground sprites are rendered
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
| Start menu options | New Game / Continue | M11 selection and foreground sprites; New Game loader receipt verified, but viewport/HUD handoff blocked; Continue/save route absent |
| Champion roster | Western names (24) | 20 PLRD records |
| Champion select UI | Sprite-based | State/input route; Saturn presentation blocked |
| In-game menu | ESC key, 2D panel | No source-bound Nexus route |
| Save/load | Binary slot files | Saturn SRAM |
| Credits | Static bitmap | AVI cutscenes |
| Menu rendering | SDL blit | VDP1/VDP2 consumer unbound |

## Next Steps
1. Bind the shared Saturn background and full-screen composition to a complete
   source-owned display capture
2. Admit the New Game route past independent viewport/HUD gates and capture the
   resulting Firestaff presentation against the same retail state
3. Authenticate champion portrait and HUD VDP1 ownership
4. Integrate Continue with the authenticated Saturn BKR payload and save
   selection, and implement Configuration destination behavior from source
   captures
5. Verify title-to-level input timing and pose against matching retail captures
6. Add FMV playback only after authentic DMV0-2.AVI ownership is proven

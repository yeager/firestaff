# Firestaff TODO - Open Work

This file tracks remaining work only. Completed work belongs in `DONE.md`.

## Maintenance Cadence

- 🔧 Update `TODO.md` and `DONE.md` at least twice per day while Firestaff work is active.
- 🔧 Keep `TODO.md` limited to things that still need fixing, building, verification, or release follow-up.
- 🔧 Keep `DONE.md` limited to finished, verified work.

## Legend

- ❌ Not started
- 🔧 In progress / partial
- 🐛 Known bug

## Current Priority Queue

- 🔧 Data-directory picker polish: start menu can switch between configured/default roots; add a proper browse/manual path flow for arbitrary user-selected game-data folders.
- 🔧 Extend the game-data scan report with machine-readable output if release/installer tooling needs it.
- 🔧 Add focused regression coverage for DM1/CSB/DM2 missing-DUNGEON launch blocking in addition to the existing CSB launcher gate.

## Dungeon Master (DM1)

### DM1 V1

- 🔧 Original DOS capture parity: five specific paired evidence sets are blocked. Details and honest status labels at `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`. Minimum runbook at `docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`.
  - Viewport: original pass94 captures exist (2026-04-28) but are impaired — frames 03–06 have duplicate SHA256, pass80 classifier reclassifies them as `entrance_menu`/`wall_closeup` instead of `dungeon_gameplay`. DOSBox input route failed to enter dungeon. New capture session with working dungeon-entry sequence required.
  - Wall: no paired original wall screenshot exists. Wall composition is source-locked only.
  - Collision: no paired original collision transcript exists. Collision logic is source-locked only.
  - Creature-chain: no paired original creature screenshot exists. Creature render is source-locked only.
  - Champion-panel: Firestaff V1 captures exist (party_hud_four_champions_vga.ppm, party_hud_statusbox_gfx_vga.ppm) but no paired original DM1 PC 3.4 champion panel screenshot exists.
  Canonical game data verified: DUNGEON.DAT SHA256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`, GRAPHICS.DAT SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`. Firestaff-side gates, source locks, and runtime routing are complete.
- 🔧 Viewport rendering regression gates: broad wall/floor/ceiling/door/ornament/pit/stair/source routes were previously source-locked; current CTest run is failing `v1_viewport_floor_ornament_stair_gate`, `v1_viewport_front_wall_depth_gate`, and `v1_viewport_pit_floor_ornament_bug64_gate`.
- 🔧 Inventory/chest polish beyond source-locked routes: core slot/body/chest/backpack/source routes are implemented; remaining work is broader chest runtime detail coverage and pixel-polish evidence.
- 🔧 Inventory panel regression gates: core item/inventory routes were previously source-locked; current CTest run is failing `m11_inventory_scroll_panel_render_source_lock` and `m11_inventory_mouth_visual_blit_source_lock`.
- 🐛 P1 visual bugs needing capture/repro: missing or incorrect viewport walls, champion Z-order/floating, champion mirrors not visible, and blurry wall inscriptions. Treat each as unconfirmed until it has a reproducible capture or focused probe.

### DM1 V2.0 / V2.1 / V2.2

- 🔧 Phase 3 - Modern UI overlay hardening: HUD/action route gates exist; remaining work is optional inventory, champion, rune, and action-panel polish without bypassing V1 command routes or inventory transactions.
- 🔧 Phase 4 - Lighting and visual effects hardening: palette/projectile metadata gates and field/projectile VFX binding gates exist; remaining work is full enhanced lighting, shadows, broader field effects, and deterministic fallback coverage.
- 🔧 Phase 5 - Smooth movement presentation hardening: runtime bridge/gates and optional Custom/V2 smooth turn-pan camera backend exist; remaining work is broader interpolation coverage and launcher UI polish while preserving V1 cooldowns, collision, sensors, creature timing, and redraw cadence.
- 🔧 Phase 6 - Touch/controller ergonomics hardening: route gates exist; remaining work is broader V2-only gesture/controller affordances with V1 touch/click parity preserved.
- 🔧 Phase 7 - V2 verification suite hardening: presentation-disabled state-hash gate exists; remaining work is full side-by-side V1/V2 deterministic input scripts plus screenshot/pixel gates for enhanced presentation.

## Chaos Strikes Back (CSB)

### CSB V1

- 🔧 Phase 2 - Dungeon data model: synthetic CSB dungeon loader/model probe exists and loader/free-cycle safety is covered; remaining work is real CSB asset ingestion and runtime structure parity without DM1-only assumptions.
- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, CSB-only D3L2/D3R2 and D2L2/D2R2 draw-order/frame gates, F0107 back-wall ornament routing, and initial viewport gates exist; remaining work includes actual ornament blits, F0108 floor ornaments, F0115 creature/item/projectile pass, F0111 door panel, and `CustomBackgrounds`.
- 🐛 Runtime handoff: the M12 launch/profile intent is valid for hash-matched CSB assets; remaining work is title/intro/import path, CSB-specific viewport integration, and end-to-end playability verification.

### CSB V2.0 / V2.1 / V2.2

- ❌ Phase 2 - Enhanced asset pipeline.
- 🔧 Phase 3 - Enhanced UI overlays: scaffolded (HUD compass/depth/gold/champion bars/action strip/chaos indicator, csb_v2_hud_overlay_pc34.h/.c, build+probe pass).
- ❌ Phase 4 - Enhanced lighting and magic effects.
- 🔧 Phase 5 - Smooth movement and viewport interpolation: `firestaff_csb_v2_smooth_movement_probe` passes 58/58. `firestaff_dm2_v2_smooth_movement_probe` 36/54 pass — CSB V2 ✅, DM2 V2 ✅, CSB V1 🔧.
- ❌ Phase 6 - Touch/controller ergonomics.
- 🔧 Phase 7 - V2 verification suite: scaffolded; deterministic boot/dungeon/combat/save/probe gates exist; remaining work is full side-by-side V1/V2 screenshot/pixel gates (`test_csb_v2_phase7_verification` failing, regressed from 🔧).

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- 🔧 Phase 2 - Dungeon/world data model: DM2 map, object, tile, and world-state ingestion (267/268 pass, outdoor level gate failing, `test_dm2_v1_dungeon` 0/4 pass — regressed from ✅).
- 🔧 Phase 3 - Rendering pipeline: viewport, UI chrome, items, outdoor/indoor, palette behavior (13/17 pass, `test_dm2_v1_asset` 0/4 pass — regressed from ✅).
- ❌ Phase 4 - Mechanics parity: movement, interactions, shops/NPCs, doors, pressure plates, triggers, combat, magic, and timeline.
- ❌ Phase 5 - Creature/combat parity: complete DM2 creature AI, projectile, damage, death/drop, and sound behavior.
- 🔧 Phase 7 - Save/import compatibility verification: `test_dm2_v1_save_load` is failing in the current CTest run.

## Dungeon Master Nexus

### Nexus V1

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked; remaining work is launcher/game-loop handoff and real Saturn asset-path proof.
- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates.
- 🔧 Phase 5 - Mechanics parity hardening: runtime bridge and probe coverage for movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound.

### Nexus V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 7 - V2 verification suite.

## Theron's Quest

### Theron V1

- 🔧 Runtime handoff/playability proof: hash-verified Track 02 availability is wired through validator/startup/menu state; remaining work is positive real-asset launch through the Theron parser/rendering path.

### Theron V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting/effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Cross-Cutting

### Launcher and Settings

- 🔧 Start-menu feature hardening: first-pass persistence exists for quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, and all five per-game option slots; remaining work is polished UI flow, runtime handoff for every option, save export/import, session timer, manual/docs launcher, cloud sync, and Custom/V2 smooth-turn-pan toggles.

### Touch and Controller Support

- ❌ Gesture navigation for runtime movement and turning.
- ❌ UI scaling and touch-target audit across launcher and game views.

### Accessibility

- ❌ Screen reader integration for launcher and game-critical state.
- 🔧 High-contrast presentation hardening: launcher output is remapped to a restricted high-contrast palette; remaining work is in-game overlay coverage.
- 🔧 Configurable font sizing hardening: launcher `fontScale` affects M12 text rendering; remaining work is in-game overlays and UI-fit coverage.

## Known Bugs

- 🐛 Viewport/collision reports without capture manifests must stay as bugs until paired original PC 3.4 evidence or a reproducible local probe exists.

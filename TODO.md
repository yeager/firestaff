# Firestaff TODO — Open Work

Status per 2026-05-19 v2.4.0.

## Legend

- ❌ Not started
- 🐛 Known bug

---

## DM1 V1 — Core Gameplay

  **✅ AUDIT COMPLETE (2026-05-24/25)** — All core gameplay systems source-locked and documented. See DONE.md for full list.
  **🔧 REMAINING: Implementation phases (V2.0-V2.2)** — See V2 sections below.


### Movement & Collision

### Creature System

  **GAP (C25/C26):** C25 (Lord Order) and C26 (Grey Lord) are structurally safe (default: FIREBALL handles them) but not explicitly handled in dm1_v1_creature_ai_behavior_pc34_compat.c. No DM1_CREATURE_TYPE_LORD_ORDER/GREY_LORD constants in behavior header. Not reachable in any original dungeon (BUG0_13). Acceptable for normal play. Modder/dungeon-editor placement would work by-accident, not by-design.

### Champion System

  **GAP (C01-C24 stats):** All 24 Hall of Champions champions (C01-C24) have STUB stat values — not source-locked to ReDMCSB G0243. 5 flag bugs: C12 Black Flame (LEVITATION/NON_MATERIAL inverted), C20 Materializer (missing both LEVITATION+NON_MATERIAL), C14 Couatl, C15 Vexirk, C21 Water Elemental, C24 Lord Chaos (each missing LEVITATION flag). TIER_FULL champions C10/C11/C13 need stat corrections.
  **OKLART (portrait sensorData):** m11_game_view.c:8995 is correct — ReDMCSB DUNGEON.C:2612 stores value+1 but DUNVIEW.C:3916 post-decrement cancels it; both code paths yield identical 0-based sheet index; confirmed no bug (commit 62411518).

### Inventory & Items
  **🔧 Save/load integration GAP** — m11_sl_* slot infrastructure has ZERO callers; F5/F9 keyboard shortcuts not wired; G2018 quit-guard absent
  **🔧 Object interaction stub GAP** — m11_obj_use() is a stub; zero call sites; needs delegation to item handlers (ITEM.C/ITEMUSE.C)
  **🔧 Group management wiring GAP** — m11_group_add_active() defined but never called; C006/F0267 group spawning not wired
  **🔧 Teleporter/pit wiring GAP** — ALL 10 functions in dm1_v1_teleporter_pit_pc34_compat.c are orphaned; no movement pipeline wiring


## DM1 V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM1 V2 value.
- ❌ V2.0 filtered presentation mode selectable from menu, using original DM1 graphics plus CRT scanlines, palette correction, and sharpening without changing DM1 V1 gameplay state, timing, collision, save/load, or source-locked command routes

- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for walls, creatures, objects, projectiles, fonts, palette/light levels, and title/entrance surfaces
- ❌ Phase 3 — Modern UI overlay: optional HUD/inventory/champion/rune/action panels that mirror V1 commands without bypassing source-locked click routes or inventory transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced lighting/shadows, palette interpolation, field/teleporter/projectile effects, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked V1 movement ticks without changing cooldowns, collision, sensors, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: V2-only gesture/controller affordances mapped onto existing command routes, with V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## CSB V1 (Chaos Strikes Back)

  **✅ AUDIT COMPLETE (2026-05-24/25)** — All CSB V1 systems source-locked and documented in docs/source-lock/csb*.md and docs/.
  **🔧 REMAINING: Implementation phases (Phase 1-7)** — See TODO below.


- ❌ Phase 1 — Boot/profile split: separate CSB runtime profile from DM1, including asset discovery, menu launch, save namespace, deterministic config, and variant-specific diagnostics
- ❌ Phase 2 — Dungeon data model: source-lock CSB dungeon.dat parsing differences, map metadata, object records, wall formats, champion transfer/import state, and start-position semantics
- ❌ Phase 3 — Rendering parity: CSB wall/door/floor/ornament/creature/item/projectile rendering, including back-wall ornaments and four-sided wall decoration rules
- ❌ Phase 4 — Mechanics parity: CSB-specific sensors, actuators, teleporters, pits, doors, pressure plates, end conditions, and dungeon logic that diverges from DM1
- ❌ Phase 5 — Creature/combat parity: CSB creature roster, AI differences, attacks/projectiles, drops, sounds, and combat constants
- ❌ Phase 6 — Utility/import flow: CSB utility disk behavior, champion import path, reincarnation/resurrection differences, and saved-party interoperability
- ❌ Phase 7 — Verification suite: canonical CSB asset manifests, parser probes, deterministic input scripts, viewport/pixel gates, save/load round trips, and source-evidence manifests

## CSB V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined CSB V2 value.
- ❌ Phase 0 — V1 parity gate: CSB V2 may not change CSB V1 command semantics, dungeon timing, source-locked collision, save/load data, or CSB-specific mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 0.5 — V2.0 filtered presentation: original CSB graphics with CRT scanlines, palette correction, and sharpening, isolated from CSB V1 gameplay state and separately verifiable from V2.1 upscale and V2.2 modern visuals
- ❌ Phase 1 — Presentation scaffold: split CSB V1 gameplay state from V2 render/input presentation, keep CSB V1 as the default runtime path, and add deterministic CSB V2 config/profile persistence
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for CSB walls, doors, ornaments, creatures, objects, projectiles, fonts, palette/light levels, title, and utility/import surfaces
- ❌ Phase 3 — Modern UI overlay: optional CSB HUD, inventory, champion, rune, action, utility, and import panels that mirror CSB V1 commands without bypassing source-locked click routes or transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced CSB lighting/shadows, palette interpolation, teleport/projectile/field effects, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked CSB movement ticks without changing cooldowns, collision, sensors, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: CSB V2-only gesture/controller affordances mapped onto existing CSB command routes, with CSB V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side CSB V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## DM2 V1 (Skullkeep)

- ❌ Phase 1 — Runtime profile split: separate DM2/Skullkeep boot profile from DM1/CSB, including menu launch, asset roots, save namespace, platform/version diagnostics, and deterministic config
- ❌ Phase 2 — Data formats: source-lock DM2 dungeon and graphics formats from SKWin/SKWINSPX/DMDC2 references, including GDAT categories, dungeon records, text, item records, actuators, doors, pits, teleports, ornate data, and variant/platform differences
- ❌ Phase 3 — Core world model: implement DM2 map loading, party placement, map transitions, outdoor/interior state, timers, object database, and deterministic world-state hashing
- ❌ Phase 4 — Rendering pipeline: source-lock Skullkeep wall/floor/door/ornament/item/creature/projectile/cloud rendering, palette/light handling, UI surfaces, title/intro assets, and GDAT-backed animation frames
- ❌ Phase 5 — Movement and interaction: port DM2 movement, click routing, item pickup/placement, containers, shop/trader interactions, doors, ladders, pits, teleports, buttons, generators, and mine/cart-specific routes
- ❌ Phase 6 — Creature, combat, spells, and environmental systems: source-lock DM2 creature AI, attacks/projectiles, champion actions, spells/clouds, weather/ambient timers, sounds, drops, and progression constants
- ❌ Phase 7 — Save/import compatibility: support DM2 save/load, PC savegame interoperability where source-backed, champion state persistence, object/container state, and cross-version diagnostics
- ❌ Phase 8 — Verification suite: canonical DM2 asset manifests, parser probes, GDAT/dungeon record fixtures, deterministic input scripts, pixel/viewport gates, save/load round trips, and source-evidence manifests tied to SKWin/skproject references

## DM2 V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM2 V2 value.
- ❌ Phase 0 — V1 parity gate: DM2 V2 may not change Skullkeep command semantics, world timing, movement/collision, save/load data, object state, shop/trader behavior, or source-locked mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 0.5 — V2.0 filtered presentation: original DM2 graphics with CRT scanlines, palette correction, and sharpening, isolated from DM2 V1 gameplay state and separately verifiable from V2.1 upscale and V2.2 modern visuals
- ❌ Phase 1 — Presentation scaffold: split DM2 V1 gameplay state from V2 render/input presentation, keep DM2 V1 as the default runtime path, and add deterministic DM2 V2 config/profile persistence
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for DM2 GDAT-backed walls, floors, doors, objects, creatures, clouds, projectiles, fonts, palette/light levels, title/intro, and UI surfaces
- ❌ Phase 3 — Modern UI overlay: optional DM2 HUD, inventory, champion, rune/action, shop/trader, map/transition, and container panels that mirror DM2 V1 commands without bypassing source-locked transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced Skullkeep lighting, weather/ambient presentation, cloud/projectile/field effects, palette interpolation, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked DM2 movement/world ticks without changing cooldowns, collisions, actuators, timers, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: DM2 V2-only gesture/controller affordances mapped onto existing DM2 command routes, with DM2 V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side DM2 V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## DM Nexus V1

- ❌ Phase 0 — Provenance gate: hash-lock exact Dungeon Master Nexus disc/images, file manifests, compression/container formats, region/version metadata, and any available primary technical references before parser or runtime work
- ❌ Phase 1 — Runtime profile split: separate Nexus boot/runtime profile from DM1/CSB/DM2, including menu launch, asset roots, save namespace, platform diagnostics, deterministic config, and unsupported-feature messaging
- ❌ Phase 2 — Data formats: source-lock Nexus dungeon, map, object, text, champion, monster, sound, and graphics/model formats; document every variant before converting data into Firestaff structures
- ❌ Phase 3 — Core world model: implement Nexus map loading, party placement, transitions, timers, object database, event/trigger records, and deterministic world-state hashing from provenance-locked fixtures
- ❌ Phase 4 — Rendering pipeline: source-lock Nexus-specific wall/floor/object/creature/projectile/UI/title rendering, palette/texture/model handling, and deterministic fallback paths for unsupported 3D assets
- ❌ Phase 5 — Mechanics parity: implement Nexus movement, click/input routes, item interactions, doors, pits, teleporters, triggers, champion state, inventory, spells, combat, creature AI, drops, and sounds only after source/provenance evidence is locked
- ❌ Phase 6 — Save/import compatibility: support Nexus save/load and champion/world persistence where format evidence is available, with explicit diagnostics for unknown or unsupported save variants
- ❌ Phase 7 — Verification suite: canonical Nexus asset manifests, parser fixtures, deterministic input scripts, viewport/pixel or model-frame gates, save/load round trips, and source-evidence manifests tied to exact disc/version hashes

## DM Nexus V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source/provenance-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM Nexus V2 value.
- ❌ Phase 0 — V1 parity gate: Nexus V2 may not change Nexus V1 command semantics, world timing, movement/collision, save/load data, champion state, object state, or source/provenance-locked mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 0.5 — V2.0 filtered presentation: original Nexus graphics/model-frame presentation with CRT scanlines, palette correction, and sharpening where provenance permits, isolated from Nexus V1 gameplay state and separately verifiable from V2.1 upscale and V2.2 modern visuals
- ❌ Phase 1 — Presentation scaffold: split Nexus V1 gameplay state from V2 render/input presentation, keep Nexus V1 as the default runtime path, and add deterministic Nexus V2 config/profile persistence
- ❌ Phase 2 — Graphics/model pipeline: source/provenance-preserving upscale or presentation path for Nexus textures, models, wall/floor/object/creature/projectile/UI/title assets, palettes/materials, and deterministic unsupported-asset fallbacks
- ❌ Phase 3 — Modern UI overlay: optional Nexus HUD, inventory, champion, spell/action, map, and diagnostics panels that mirror Nexus V1 commands without bypassing source/provenance-locked routes or transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced Nexus lighting, model-frame presentation, projectile/field effects, palette/material interpolation, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source/provenance-locked Nexus movement/world ticks without changing cooldowns, collisions, triggers, timers, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: Nexus V2-only gesture/controller affordances mapped onto existing Nexus command routes, with Nexus V1 touch/click parity preserved where source/provenance evidence exists
- ❌ Phase 7 — V2 verification suite: side-by-side Nexus V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus viewport/pixel or model-frame gates for V2 presentation

## Theron's Quest V1 (PC Engine/TurboGrafx-16)

- ❌ Theron's Quest launch entry in menu
- ❌ Phase 0 — Provenance gate: hash-lock exact Theron's Quest PC Engine/TurboGrafx-16 disc/image, file manifests, data file formats, and primary technical references; Japanese version released 1992-09-18, English version 1993 in USA
- ❌ Phase 1 — Runtime profile split: separate Theron's Quest boot/runtime profile from DM1/CSB/DM2/Nexus, including menu launch, asset roots, save namespace (no in-dungeon saves — only between dungeons), platform diagnostics, deterministic config
- ❌ Phase 2 — Data formats: source-lock Theron's Quest dungeon, object, text, champion, creature, and graphics formats; "light" version — only a subset of DM1 items, creatures, and spells; 7 mini-dungeons, some copied/inspired by DM1/CSB
- ❌ Phase 3 — Core world model: implement Theron's Quest map loading, party placement (Theron + 3 champions), transitions, timers, object database, champion skill/stat persistence (Theron keeps skills/stats between dungeons; champions lose skills/items)
- ❌ Phase 4 — Rendering pipeline: source-lock Theron's Quest wall/floor/object/creature/projectile/UI/title rendering, palette handling, and deterministic fallback for PC Engine planar graphics
- ❌ Phase 5 — Mechanics parity: implement movement, click/routes, doors, pits, teleporters, altar-of-vi resurrection, champion state, inventory, combat, creature AI, drops, sounds after source evidence is locked
- ❌ Phase 6 — Dungeon progression: implement 7-dungeon sequence, per-dungeon item reset, between-dungeon save, and seven-quest-item retrieval goal
- ❌ Phase 7 — Save/import compatibility: support between-dungeon save/load and champion/world persistence where format evidence is available
- ❌ Phase 8 — Verification suite: canonical Theron's Quest asset manifests, parser fixtures, deterministic input scripts, viewport/pixel gates, save/load round trips

**Reference:** http://dmweb.free.fr/games/therons-quest/

## Theron's Quest V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source/provenance-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track completion separately; never report a single combined Theron's Quest V2 value.
- ❌ Phase 0 — V1 parity gate: Theron's Quest V2 may not change V1 command semantics, timing, or locked mechanics unless behind an explicit V2 toggle
- ❌ Phase 0.5 — V2.0 filtered presentation: original PC Engine graphics with CRT scanlines, palette correction, and sharpening
- ❌ Phase 1 — Presentation scaffold: split V1 gameplay state from V2 render/input presentation, keep V1 as default
- ❌ Phase 2 — Graphics pipeline: source/provenance-preserving upscale for textures, walls, floors, objects, creatures, UI/title assets
- ❌ Phase 3 — Modern UI overlay: optional HUD, inventory, champion, spell/action, map, and diagnostics panels that mirror V1 commands
- ❌ Phase 4 — Lighting and visual effects: enhanced lighting, projectile/field effects, palette interpolation
- ❌ Phase 5 — Smooth movement: interpolation between locked movement ticks without changing cooldowns, collisions, or timing
- ❌ Phase 6 — Touch/controller ergonomics: V2-only gesture/controller affordances mapped onto existing command routes
- ❌ Phase 7 — V2 verification suite: side-by-side V1/V2 deterministic probes plus viewport/pixel gates for V2 presentation

## Cross-Cutting Features

### Touch Support

- ❌ Gesture navigation (swipe to turn, tap to move)
- ❌ UI scaling for touch targets

### Accessibility

- ❌ Screen reader integration
- ❌ High-contrast mode
- ❌ Configurable font size

### Audio

## Known Bugs (need repro)

1. No open wall/collision report without a capture manifest. Use the DM1 V1 wall/collision runtime capture gate for exact map/x/y/direction and screenshots before accepting new reports.

## DM1 V1 — GAP Fixes (2026-05-25)

- 🔧 Save/load integration — wire DM1_SaveGame/LoadGame to menu/input; add m11_sl_* slot picker callers; implement G2018 quit-guard prompt
- 🔧 Item pickup wiring — add m11_inventory_can_pickup(); wire m11_obj_pickup() to viewport click path; add pickup command bridging
- 🔧 Object interaction — replace m11_obj_use() stub with real implementation delegating to dm1_inventory_consume_potion_pc34 etc.
- 🔧 Group management wiring — call m11_group_add_active() from sensor_trigger when C006 triggers; wire C006/F0267 chain
- 🔧 Teleporter wiring — connect m11_check_teleporter() to movement_pipeline; add F0267 activation calls from sensor_trigger


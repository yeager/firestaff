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

### Champion System



### Inventory & Items




## DM1 V2.0 / V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.0 is the README-defined filtered original-graphics path with CRT scanlines, palette correction, and sharpening; V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM1 V2 value.
- ✅ V2.0 filtered presentation mode — CRT scanlines, palette correction, dither cleanup, sharpening implemented (commits 72162e42, 5267edf1, ac493c36, f6ee27aa)

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
- 🔧 Phase 3 — Rendering parity: CSB wall/door/floor/ornament/creature/item/projectile rendering, including back-wall ornaments and four-sided wall decoration rules
  - ✅ Back-wall (D3L2/D3R2) frame tables, wall draw spec entries, and element routing
  - ✅ Near-wall (D2L2/D2R2) frame tables and WALL/TELEPORTER routing
  - ✅ D3L2/D3R2 parity-aware wall bitmap selection (F0676/F0677)
  - ✅ D2L2/D2R2 parity-aware wall bitmap selection (F0678/F0679)
  - ✅ csb_v1_viewport_render_frame wires dungeon_grid for element routing
  - 🔧 Deferred (pass604+): F0107 wall ornaments, F0108 floor ornaments, F0115 creature/item/projectile, F0111 door panel, CustomBackgrounds (CSBWin/CSBCode.cpp:26)
- ✅ Phase 4 — Mechanics parity: CSB-specific sensors, actuators, teleporters, pits, doors, pressure plates, end conditions, and dungeon logic that diverge from DM1 (docs/source-lock/csb_v1_phase4_mechanics_parity_H2239.md)
- ✅ Phase 5 — Creature/combat parity: CSB creature roster, AI differences, attacks/projectiles, drops, sounds, and combat constants (docs/source-lock/csb_v1_phase5_creature_combat_H2242.md · src/csb/csb_v1_monster_pc34_compat.{c,h})
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
- ✅ Phase 7 — Save/import compatibility: support DM2 save/load, PC savegame interoperability where source-backed, champion state persistence, object/container state, and cross-version diagnostics (src/dm2/dm2_v1_save_load.{c,h}: SUPPRESS codec, slot manager, game state block, global vars, spell effects, timers, minions, inventories, leader possession, PC save detection)
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
- ✅ Phase 3 — Core world model: implement Nexus map loading, party placement, transitions, timers, object database, event/trigger records, and deterministic world-state hashing from provenance-locked fixtures
- ✅ Phase 4 — Rendering pipeline: source-lock Nexus-specific wall/floor/object/creature/projectile/UI/title rendering, palette/texture/model handling, and deterministic fallback paths for unsupported 3D assets (docs/source-lock/nexus_v1_phase4_rendering_pipeline_H0357.md · src/nexus/nexus_v1_palette.{c,h} · src/nexus/nexus_v1_rasterizer.{c,h} · src/nexus/nexus_v1_ui_surfaces.{c,h})
- ❌ Phase 5 — Mechanics parity: implement Nexus movement, click/input routes, item interactions, doors, pits, teleporters, triggers, champion state, inventory, spells, combat, creature AI, drops, and sounds only after source/provenance evidence is locked
- ✅ Phase 6 — Save/import compatibility: support Nexus save/load and champion/world persistence where format evidence is available, with explicit diagnostics for unknown or unsupported save variants
- ✅ Phase 7 — Verification suite: canonical Nexus asset manifests, parser fixtures, deterministic input scripts, viewport/pixel or model-frame gates, save/load round trips, and source-evidence manifests tied to exact disc/version hashes (docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md · probes/nexus_v1_asset_manifest_probe.c · probes/nexus_v1_model_frame_gate_probe.c · probes/nexus_v1_viewport_gate_probe.c · probes/nexus_v1_save_load_round_trip_probe.c · scripts/verify_nexus_v1_asset_manifest.py · scripts/verify_nexus_v1_input_script.py · scripts/generate_nexus_v1_fixtures.py · scripts/generate_nexus_v1_asset_manifest.py · scripts/fixtures/nexus_v1_asset_sizes.py · scripts/fixtures/nexus_v1_disc_file_hashes.py)

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
- ✅ Phase 0 — Provenance gate: hash-lock confirmed PC Engine/TurboGrafx-16 disc image hashes (MD5), CD track structure (18 tracks, Track 02=data), JP/US differences documented, data format hypotheses documented; Japanese version 1992-09-18, English USA 1993; 'light' version — subset of DM1 items/creatures/spells; 7 mini-dungeons; Theron persistent, companions reset per dungeon (docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md: JP MD5 b7afb338ad31be1025b53f9aff12d73a, US MD5 f23601102138f87c33025877767ebf76)
- ✅ Phase 1 — Runtime profile split: separate Theron's Quest boot/runtime profile from DM1/CSB/DM2/Nexus, including menu launch, asset roots, save namespace (no in-dungeon saves — only between dungeons), platform diagnostics, deterministic config
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

## DM1 V1 — Partial Implementations (moved from DONE.md 2026-05-26)

These subsystems have source-locked code present but explicit gaps remain. Tracked here until the remaining work closes.

- 🔧 Creature group spawning (C006 floor sensor group generator) — successful empty-square materialization is source-locked with fixed group-slot reuse, square insertion, party-map active-state seed, F0180 event37 wandering hookup, delayed C65 re-enable, F0185/F0245 buzz dispatch, event60/61 blocked-destination defer (5ace9238), Lord Chaos adjacent random retry (0a12e07d), narrow F0267 teleporter destination/cross-map placement with covered audible buzz/wandering scheduling (191c9d3c), group move-removal planning coverage (91578481), F0267 open-pit fall damage/death, lower-map insertion, and carried group-slot fall-kill drops (9db12dbf), generated/deferred non-square insertion projectile non-impact preservation (850c1541), generated/deferred creature-not-allowed rejection plus carried-slot drop handling (1741688d), and moving fixed-possession partial/death drops (365b4b8f); ordinary moving-group projectile impact/removal (dab7b40d), group-specific teleporter rotation, and multi-hop teleporter audible buzz order (d3ef5834) are source-locked; **remaining**: original DOS capture parity for broader chained runtime proof
- 🔧 Champion stats panel — eye-click runtime shows source-locked HP/stamina/mana, all six statistic families, four base skill levels, F0351 skill-level/statistic names (2fcd982d), CHAMDRAW status-value formatting/zone routing (d504de4c), true current-vs-maximum statistic color rows (ea743e8c), panel-HUD statistic color/format helper and status box model (dc4cbb37), runtime current/maximum statistic row wiring (4c671c34), F0351 text-run layout/color model plus M11 stats text routing coverage (ac3efa45), and Firestaff indexed-framebuffer draw path for the colored per-stat panel (2355619c) are source-locked; **remaining**: original PC 3.4 capture/comparator proof for true original-vs-Firestaff framebuffer parity
- 🔧 Full inventory panel (8 body slots + 2 hand slots + backpack) — C507..C536 source slot-box bridge, full backpack runtime storage/pickup/place path plus all C520..C536 source slot round-trips (78412579), champion inventory slot pickup/place hash refresh (fb747c23), PC34 open-chest slot setter (461139c9), and open action-hand chest icon remap (ba92e3a7) are source-locked; **remaining**: panel polish/chest runtime details
- 🔧 Equip/unequip items to body slots — PC34 slot masks, status hand-slot route resolution, and leader-hand/body-slot swap transaction are source-locked (6acbf589, a85fb55d slot validation); **remaining**: full inventory/backpack/chest storage expansion
- 🔧 Backpack/chest container management — M11 V1 open action-hand chest state, C537..C544 panel route hit-testing, visible chest slot drawing, leader-hand/chest slot swaps, middle visible chest-slot pickup compaction (e99b5839), all C520..C536 backpack source slot runtime round-trips (78412579), action-hand chest panel close/reopen persistence, PC34 open-chest slot setter (461139c9), and open chest C145 action-hand icon remap (ba92e3a7) are source-locked; **remaining**: broader full panel polish
- 🔧 V2.1/V2.2 presentation modes selectable from menu — M12_PRESENTATION_V21_UPSCALED / V22_MODERN enum values and menu cycle exist, but V2.1 upscale and V2.2 modern pipelines are not yet implemented; **remaining**: V2.1/V2.2 phases per the V2 Enhanced Modes sections above
- 🔧 CSB launch entry in menu — g_entryTemplate has CSB entry (gameId "csb"), "RUNTIME NOT READY" block REMOVED in menu layer; CSB dungeon loading via m11_resolve_builtin_dungeon_path confirmed; M11_GameView_Start path wired for gameId="csb"; title screen display relies on M11 rendering pipeline (same as DM1); **remaining**: verify title/intro plays, confirm dungeon renders correctly for CSB dungeon geometry (2098-byte dungeon), CSB-specific intro sequence
- 🔧 DM2 launch entry in menu — g_entryTemplate has DM2 entry (gameId "dm2"); blocked on DM2 runtime handoff; **remaining**: DM2 V1 phases
- 🔧 Nexus launch entry in menu — g_entryTemplate has Nexus entry (gameId "nexus1"); blocked on Nexus runtime handoff; **remaining**: Nexus V1 phases
- 🔧 Touch controls config option exists — settings tab has Touch Controls (Off/Minimal/Full/Large) and Large Touch Targets options persisted via config; **remaining**: real touch gesture navigation and UI scaling (see Cross-Cutting Features → Touch Support)
- 🔧 Accessibility module exists (firestaff_accessibility) — src/engine/firestaff_accessibility.c (250 lines) and include/firestaff_accessibility.h scaffolded; **remaining**: screen reader integration, high-contrast mode, configurable font size (see Cross-Cutting Features → Accessibility)
- 🔧 VIEWPORT RENDERING wall draw loop — door frames implemented (D3/D2/D1 via dm1_viewport_3d_get_wall_frame + F0104/F0105) committed 30ccdfc0; non-door wall draw loop wired via s_draw_order/wall_set in dm1_viewport_3d_draw_frame committed 54be3b3c; **remaining**: asset system to populate `g_dm1_wall_frame_bitmaps` (still NULL), D4 far-object rendering pass, full door occlusion verification


## DM1 V1 — P1 Visual Bugs (from LIST.md 2026-05-26)

- 🔴 Viewport walls not rendering — walls missing from viewport, but map shows correct layout; need runtime capture with exact map/x/y/direction and screenshots before fixing
- 🔴 Champion Z-order / floating — champion images rendered at wrong Z-layer; VBLANK.C draw order issue
- 🔴 Champion mirrors not visible — GRAPHICS.DAT bitmap indices for mirror objects not wired to viewport render
- 🔴 Wall inscriptions blurry — PANEL.C/DUNVIEW.C inscription blit resolution issue
- 🔴 Mac app icon missing — firestaff.icns needs to be added to .app bundle Contents/Resources

## CSB V1 — Runtime Handoff (NEW 2026-05-26)

- 🔴 CSB V1 launch handoff — m12_game_supported("csb") returns true but launch shows "RUNTIME NOT READY"; need to trace menu→game loop path and implement M12_MENU_INPUT_ACCEPT handler for CSB entry
- 🔴 CSB V1 viewport rendering — csb_v1_viewport_pc34_compat.c needs integration into main_loop_m11.c game loop
- 🔴 CSB V1 dungeon entrance — load sequence: CSB Title → Import DM1 Champions → Dungeon entrance; csb_v1_character_pc34_compat.c::csb_v1_import_dm1_save() needs wiring
- 🔴 CSB V1 title screen — csb_v1_game.c has title screen logic but not wired to M12 menu handoff

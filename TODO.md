# Firestaff TODO — Open Work

Status per 2026-05-19 v2.4.0.

## Legend
- 🔧 Partially implemented
- ❌ Not started
- 🐛 Known bug

---

## DM1 V1 — Core Gameplay

### Movement & Collision

- 🔧 Viewport/wall occlusion — DM1 V1 side-field occlusion now has source-locked D3/D2, D1, and D0 evidence manifests and focused viewport regression coverage through 17811281; remaining wall/viewport parity gaps still need capture-backed closure

### Creature System

- 🔧 Creature type-specific behavior — Giggler steal, Ghost/non-material melee gating, quarter-square melee cell shuffle, adjacent creature projectile chance, and fixed possession drop payloads are source-locked (45fec7c5); ordinary melee weapons no longer hit non-material creatures, while Vorpal/Disrupt and materializer/projectile explosion harm paths are accepted; other type specials remain
- 🔧 Creature group spawning (C006 floor sensor group generator) — successful empty-square materialization is source-locked with fixed group-slot reuse, square insertion, party-map active-state seed, F0180 event37 wandering hookup, delayed C65 re-enable, F0185/F0245 buzz dispatch, event60/61 blocked-destination defer (5ace9238), Lord Chaos adjacent random retry (0a12e07d), and narrow F0267 teleporter destination/cross-map placement with covered audible buzz/wandering scheduling (191c9d3c); remaining pit fall damage/death/drop, projectile impact/removal, teleporter rotation parity, creature-not-allowed deletion/drop, and multi-hop audible parity remain

### Champion System

- 🔧 Champion stats panel — eye-click runtime now shows source-locked HP/stamina/mana, all six statistic families, four base skill levels, and CHAMDRAW status-value formatting/zone routing (d504de4c); broader panel polish remains

### Inventory & Items

- 🔧 Full inventory panel (8 body slots + 2 hand slots + backpack) — C507..C536 source slot-box bridge and full backpack runtime storage/pickup/place path and PC34 open-chest slot setter are source-locked (461139c9); remaining panel polish/chest runtime details stay open
- 🔧 Equip/unequip items to body slots — partially: PC34 slot masks and leader-hand/body-slot swap transaction are source-locked; full inventory/backpack/chest storage expansion remains pending
- 🔧 Backpack/chest container management — M11 V1 open action-hand chest state, C537..C544 panel route hit-testing, visible chest slot drawing, leader-hand/chest slot swaps, action-hand chest panel close/reopen persistence, and PC34 open-chest slot setter are source-locked (461139c9); broader full panel polish remains pending
- 🔧 Item identification — potion eye-panel power-prefix display and M11 leader-hand runtime wiring are source-locked for Priest > 1, including the original empty-flask quirk; weapon eye-panel name/attribute description formatting is source-locked for POISONED/BROKEN/CURSED (94094865); item weight line display is source-locked (35461e0b); armor BROKEN/CURSED, junk consumable/compass/waterskin state lines, scroll/container panel routes, and object-description panel layout are source-locked (440eaca1); remaining item-identification polish is runtime/render integration only

## DM1 V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM1 V2 value.
- 🔧 V2.1/V2.2 presentation modes selectable from menu
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for walls, creatures, objects, projectiles, fonts, palette/light levels, and title/entrance surfaces
- ❌ Phase 3 — Modern UI overlay: optional HUD/inventory/champion/rune/action panels that mirror V1 commands without bypassing source-locked click routes or inventory transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced lighting/shadows, palette interpolation, field/teleporter/projectile effects, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked V1 movement ticks without changing cooldowns, collision, sensors, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: V2-only gesture/controller affordances mapped onto existing command routes, with V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## CSB V1 (Chaos Strikes Back)

- 🔧 CSB launch entry in menu
- ❌ Phase 1 — Boot/profile split: separate CSB runtime profile from DM1, including asset discovery, menu launch, save namespace, deterministic config, and variant-specific diagnostics
- ❌ Phase 2 — Dungeon data model: source-lock CSB dungeon.dat parsing differences, map metadata, object records, wall formats, champion transfer/import state, and start-position semantics
- ❌ Phase 3 — Rendering parity: CSB wall/door/floor/ornament/creature/item/projectile rendering, including back-wall ornaments and four-sided wall decoration rules
- ❌ Phase 4 — Mechanics parity: CSB-specific sensors, actuators, teleporters, pits, doors, pressure plates, end conditions, and dungeon logic that diverges from DM1
- ❌ Phase 5 — Creature/combat parity: CSB creature roster, AI differences, attacks/projectiles, drops, sounds, and combat constants
- ❌ Phase 6 — Utility/import flow: CSB utility disk behavior, champion import path, reincarnation/resurrection differences, and saved-party interoperability
- ❌ Phase 7 — Verification suite: canonical CSB asset manifests, parser probes, deterministic input scripts, viewport/pixel gates, save/load round trips, and source-evidence manifests

## CSB V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined CSB V2 value.
- ❌ Phase 0 — V1 parity gate: CSB V2 may not change CSB V1 command semantics, dungeon timing, source-locked collision, save/load data, or CSB-specific mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 1 — Presentation scaffold: split CSB V1 gameplay state from V2 render/input presentation, keep CSB V1 as the default runtime path, and add deterministic CSB V2 config/profile persistence
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for CSB walls, doors, ornaments, creatures, objects, projectiles, fonts, palette/light levels, title, and utility/import surfaces
- ❌ Phase 3 — Modern UI overlay: optional CSB HUD, inventory, champion, rune, action, utility, and import panels that mirror CSB V1 commands without bypassing source-locked click routes or transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced CSB lighting/shadows, palette interpolation, teleport/projectile/field effects, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked CSB movement ticks without changing cooldowns, collision, sensors, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: CSB V2-only gesture/controller affordances mapped onto existing CSB command routes, with CSB V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side CSB V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## DM2 V1 (Skullkeep)

- 🔧 DM2 launch entry in menu
- ❌ Phase 1 — Runtime profile split: separate DM2/Skullkeep boot profile from DM1/CSB, including menu launch, asset roots, save namespace, platform/version diagnostics, and deterministic config
- ❌ Phase 2 — Data formats: source-lock DM2 dungeon and graphics formats from SKWin/SKWINSPX/DMDC2 references, including GDAT categories, dungeon records, text, item records, actuators, doors, pits, teleports, ornate data, and variant/platform differences
- ❌ Phase 3 — Core world model: implement DM2 map loading, party placement, map transitions, outdoor/interior state, timers, object database, and deterministic world-state hashing
- ❌ Phase 4 — Rendering pipeline: source-lock Skullkeep wall/floor/door/ornament/item/creature/projectile/cloud rendering, palette/light handling, UI surfaces, title/intro assets, and GDAT-backed animation frames
- ❌ Phase 5 — Movement and interaction: port DM2 movement, click routing, item pickup/placement, containers, shop/trader interactions, doors, ladders, pits, teleports, buttons, generators, and mine/cart-specific routes
- ❌ Phase 6 — Creature, combat, spells, and environmental systems: source-lock DM2 creature AI, attacks/projectiles, champion actions, spells/clouds, weather/ambient timers, sounds, drops, and progression constants
- ❌ Phase 7 — Save/import compatibility: support DM2 save/load, PC savegame interoperability where source-backed, champion state persistence, object/container state, and cross-version diagnostics
- ❌ Phase 8 — Verification suite: canonical DM2 asset manifests, parser probes, GDAT/dungeon record fixtures, deterministic input scripts, pixel/viewport gates, save/load round trips, and source-evidence manifests tied to SKWin/skproject references

## DM2 V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.1 is the 10x source-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM2 V2 value.
- ❌ Phase 0 — V1 parity gate: DM2 V2 may not change Skullkeep command semantics, world timing, movement/collision, save/load data, object state, shop/trader behavior, or source-locked mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 1 — Presentation scaffold: split DM2 V1 gameplay state from V2 render/input presentation, keep DM2 V1 as the default runtime path, and add deterministic DM2 V2 config/profile persistence
- ❌ Phase 2 — Graphics pipeline: source-asset-preserving upscale path for DM2 GDAT-backed walls, floors, doors, objects, creatures, clouds, projectiles, fonts, palette/light levels, title/intro, and UI surfaces
- ❌ Phase 3 — Modern UI overlay: optional DM2 HUD, inventory, champion, rune/action, shop/trader, map/transition, and container panels that mirror DM2 V1 commands without bypassing source-locked transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced Skullkeep lighting, weather/ambient presentation, cloud/projectile/field effects, palette interpolation, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source-locked DM2 movement/world ticks without changing cooldowns, collisions, actuators, timers, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: DM2 V2-only gesture/controller affordances mapped onto existing DM2 command routes, with DM2 V1 touch/click parity preserved
- ❌ Phase 7 — V2 verification suite: side-by-side DM2 V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus screenshot/pixel gates for V2 presentation

## DM Nexus V1

- 🔧 Nexus launch entry in menu
- ❌ Phase 0 — Provenance gate: hash-lock exact Dungeon Master Nexus disc/images, file manifests, compression/container formats, region/version metadata, and any available primary technical references before parser or runtime work
- ❌ Phase 1 — Runtime profile split: separate Nexus boot/runtime profile from DM1/CSB/DM2, including menu launch, asset roots, save namespace, platform diagnostics, deterministic config, and unsupported-feature messaging
- ❌ Phase 2 — Data formats: source-lock Nexus dungeon, map, object, text, champion, monster, sound, and graphics/model formats; document every variant before converting data into Firestaff structures
- ❌ Phase 3 — Core world model: implement Nexus map loading, party placement, transitions, timers, object database, event/trigger records, and deterministic world-state hashing from provenance-locked fixtures
- ❌ Phase 4 — Rendering pipeline: source-lock Nexus-specific wall/floor/object/creature/projectile/UI/title rendering, palette/texture/model handling, and deterministic fallback paths for unsupported 3D assets
- ❌ Phase 5 — Mechanics parity: implement Nexus movement, click/input routes, item interactions, doors, pits, teleporters, triggers, champion state, inventory, spells, combat, creature AI, drops, and sounds only after source/provenance evidence is locked
- ❌ Phase 6 — Save/import compatibility: support Nexus save/load and champion/world persistence where format evidence is available, with explicit diagnostics for unknown or unsupported save variants
- ❌ Phase 7 — Verification suite: canonical Nexus asset manifests, parser fixtures, deterministic input scripts, viewport/pixel or model-frame gates, save/load round trips, and source-evidence manifests tied to exact disc/version hashes

## DM Nexus V2.1 / V2.2 — Enhanced Modes

- 🧭 V2 split: V2.1 is the 10x source/provenance-preserving upscale path; V2.2 is the modern Dungeon Master feel with hybrid generated graphics. Track TODO, status, completion %, and worker scope separately; never report a single combined DM Nexus V2 value.
- ❌ Phase 0 — V1 parity gate: Nexus V2 may not change Nexus V1 command semantics, world timing, movement/collision, save/load data, champion state, object state, or source/provenance-locked mechanics unless behavior is behind an explicit V2 presentation toggle
- ❌ Phase 1 — Presentation scaffold: split Nexus V1 gameplay state from V2 render/input presentation, keep Nexus V1 as the default runtime path, and add deterministic Nexus V2 config/profile persistence
- ❌ Phase 2 — Graphics/model pipeline: source/provenance-preserving upscale or presentation path for Nexus textures, models, wall/floor/object/creature/projectile/UI/title assets, palettes/materials, and deterministic unsupported-asset fallbacks
- ❌ Phase 3 — Modern UI overlay: optional Nexus HUD, inventory, champion, spell/action, map, and diagnostics panels that mirror Nexus V1 commands without bypassing source/provenance-locked routes or transactions
- ❌ Phase 4 — Lighting and visual effects: enhanced Nexus lighting, model-frame presentation, projectile/field effects, palette/material interpolation, and HiDPI-safe composition with deterministic fallback
- ❌ Phase 5 — Smooth movement presentation: interpolation/camera easing between source/provenance-locked Nexus movement/world ticks without changing cooldowns, collisions, triggers, timers, creature timing, or redraw cadence
- ❌ Phase 6 — Touch/controller ergonomics: Nexus V2-only gesture/controller affordances mapped onto existing Nexus command routes, with Nexus V1 touch/click parity preserved where source/provenance evidence exists
- ❌ Phase 7 — V2 verification suite: side-by-side Nexus V1/V2 deterministic probes proving identical gameplay state hashes for the same input script plus viewport/pixel or model-frame gates for V2 presentation

## Cross-Cutting Features

### Touch Support

- 🔧 Touch controls config option exists
- ❌ Gesture navigation (swipe to turn, tap to move)
- ❌ UI scaling for touch targets

### Accessibility

- 🔧 Accessibility module exists (firestaff_accessibility)
- ❌ Screen reader integration
- ❌ High-contrast mode
- ❌ Configurable font size

## Known Bugs (need repro)

1. No open wall/collision report without a capture manifest. Use the DM1 V1 wall/collision runtime capture gate for exact map/x/y/direction and screenshots before accepting new reports.

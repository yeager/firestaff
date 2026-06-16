# Firestaff DONE - Completed Work

This file tracks completed capabilities by game. It is not a changelog; see git history and release notes for chronology.

## Legend

- ✅ Done / verified
- 🔒 Source-locked against original references

## Dungeon Master (DM1)

### DM1 V1 - Runtime and Source-Lock

- ✅ Movement and collision: cardinal movement, turning, wall/door/fake-wall blocking, cooldowns, stairs, pits, teleporters, blocked self-damage, empty-party group cleanup, and deterministic capture gates.
- ✅ Viewport rendering: wall/floor/ceiling slices, doors, frames, ornaments, inscriptions, pits, stairs, creatures, projectiles, explosions, floor items, alcoves, occlusion, palette dimming, HiDPI scaling, and teleporter visuals.
- ✅ Viewport wall evidence hardening: `g_dm1_wall_frame_bitmaps` is source-locked to the PC34 `G2107`/door-frame offset model and guarded by an asset-free null-write regression.
- ✅ Door-front occlusion pixel-zone gate: all 11 source-locked front-door branches prove rear cells are masked by door pixels and front cells draw after the door pass.
- 🔒 DOR-01 F0715 front-door toggle resolver source-lock pin (commit `73c9b1e1`): `test_dm1_v1_dor01_f0715_door_resolve_toggle_action_pc34_compat` 18/18 assertions, FNV-1a 32-bit hash `0xEC4F85A7`, pins the ReDMCSB door actuator branch (adjacent to F0275_SENSOR_IsTriggeredByClickOnWall) contract for door state 0/4/5 -> CLOSE/OPEN/DESTROYED, animating state 1/2/3 -> snap-OPEN, vertical-bit 0x08 -> doorVertical, F0715 purity (no square byte mutation), and outResult population on early reject.
- ✅ Creature and combat systems: creature groups, AI, attacks, deaths, drops, XP, projectile attacks, sounds, fleeing, special positioning, possession drops, Black Flame behavior, generator/teleporter/fall/drop cases, and Lord Chaos constants.
- ✅ Spells and magic: rune UI, spell casting, mana/skill checks, projectiles, shields, light/dark, open-door magic, poison cloud behavior, and spell failure paths.
- ✅ Champions: recruitment, active selection, health/stamina/mana bars, skill/XP updates, death/resurrection, stats panel routing, weight/load behavior, and stamina regeneration.
- ✅ Inventory and items: leader hand, alcoves, throwing, torches/light, floor pickup, scrolls, potions, food/water, item descriptions, chest/backpack routes, equip/unequip, fountains, and source-blocked direct key action.
- ✅ Survival, sensors, entrance, save/load, audio, and data loading: food/water decay, rest, stamina, sensor/timeline behavior, title/entrance flow, save/load routes, sound routing, and DUNGEON.DAT/GRAPHICS.DAT ingestion.
- ✅ Source-lock verifier hardening: viewport/walls landable metadata, wall-clip source audit, side-wall source-row clipping, D3/D2 wall-ornament order, front-cell collision, D0/D1 visible-square draw-order, wall-alcove C2548, champion stat panel, and ambient dungeon sound gates now resolve current local code/source boundaries and reflect the closed no-ambient-loop source boundary.
- 🔒 DM1 source-lock audit completed across movement, rendering, creatures, combat, spells, champions, inventory, survival, sensors, entrance, save/load, audio, and data structures.

### DM1 V2.0 / V2.1 / V2.2

- ✅ V2.0 filtered presentation: config, CRT scanlines, palette correction, dither cleanup, sharpening, renderer integration, and launcher/menu integration.
- ✅ V2 parity/presentation scaffold: Phase 0 and Phase 1 command routing, deterministic config, profile boundary, and launch-smoke verification.
- ✅ V2.1 asset pipeline: Phase 2 source-preserving upscale/EPX pipeline, deterministic cache behavior, fallback handling, and probe coverage.
- ✅ V2 presentation slices: HUD/action route gate, palette/projectile metadata gates, smooth-movement runtime bridge, touch/controller route gate, and presentation-disabled state-hash gate.
- ✅ DM1 V2 smooth turn pan backend: optional Custom/V2 turn-pan setting persists through config, the Phase 5 bridge can start pan-enabled turns, and the camera exposes a presentation-only viewport pan offset while V1 command direction changes remain source-owned.
- ✅ DM1 V2 Phase 4 field/projectile VFX binding gate: source explosion thing IDs map to V2 overlay/emitter families, fluxcage remains field-only, unknown things are rejected, and invalid source palette lighting falls back deterministically.

## Chaos Strikes Back (CSB)

### CSB V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, profile-specific asset discovery, boot state, diagnostics, and hash-matched launch boundary.
- ✅ Phase 2 dungeon-data probe slice: synthetic CSB dungeon loading, square/thing accessors, door table, sensor helpers, endgame helpers, world-model behavior, and CSB-vs-DM1 difference checks are covered by `firestaff_csb_v1_dungeon_model_probe`.
- ✅ Launch/profile fixture: the Atari ST asset-pair manifest and CSB launch-intent gate now recognize hash-matched CSB assets as valid for the M12 profile boundary while keeping gameplay, save, and pixel parity as non-claims.
- ✅ Source-lock audit coverage for CSB startup, utility, dungeon loading, wall rendering, champion import, weapons, magic, creatures, combat, and save behavior.
- ✅ Phase 4 - Mechanics parity slices for CSB-specific movement/interaction/runtime behavior.
- ✅ Phase 5 - Creature and combat parity slices.
- ✅ Phase 6 - Utility/import flow: champion import from DM1 saves (256-byte CSB champion block format, DM1 116-byte record → CSB block conversion, ReDMCSB SAVEGAME.C F0100-F0120 state machine), utility disk flow state machine (INIT→INSERT_DISK→VERIFY_DISK→DISK_OK→SELECT_ACTION→IMPORT/LOAD/NEW→DONE), disk verification, import confirmation, save-game load stub; headless probe `firestaff_csb_v1_utility_import_probe` passes 33/33 tests.
- ✅ Completed rendering slices: D3/D2 wall-table mapping, parity bitmap selection, grid routing, and initial CSB viewport source-lock gates.
- ✅ CSB V1 viewport Phase 3 gate: D3L2/D3R2 and D2L2/D2R2 draw-order, coordinate, frame, and PC34 zone contracts are source-locked against F0676-F0679/F0128.
- ✅ CSB V1 back-wall ornament routing gate: D3L2/D3R2 wall cases source-lock their F0107 ordinal slots and view-wall indices, while D2L2/D2R2 prove the no-F0107 return path.

### CSB V2.0 / V2.1 / V2.2

- ✅ Phase 5 smooth-movement runtime bridge: `csb_v2_smooth_movement.c` provides visual walk (ease-out cubic), turn (ease-out quad), and stairs (ease-in-out cubic + vertical camera offset) interpolations over 1 V1 tick (55ms). Global state is driven via a `V2_AnimClock*` to `csb_v2_smooth_update_from_clock`. Headless probe `firestaff_csb_v2_smooth_movement_probe` covers lifecycle, walk N/S/E/W, turn 8 directions, stairs with vertical offset, and deterministic input coverage; ctest target `test_csb_v2_smooth_movement` passes 50/50. Plus binding seam: `csb_v2_runtime.c` (CSB_V1_RuntimeProfile, `bind_to_v1`/`is_bound`/`force_sync`/`v1_tick`/`render_frame`) auto-triggers walk/turn/stairs on V1 deltas (F0365/F0366/F0364). Integration test `test_csb_v2_smooth_runtime_binding` 12 groups/43 asserts pass; ctest 2/2 (CSB smooth + CSB runtime binding).

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, asset discovery, launcher state, and runtime selection.
- ✅ Phase 7 - Save/import compatibility verification.
- ✅ Phase 8 - Verification-suite scaffold and probes.
- ✅ Source-lock audit coverage for DM2 boot, dungeon/data loading, rendering, items, creatures, combat, spells, shops/NPCs, save behavior, and verification paths.

### DM2 V2.0 / V2.1 / V2.2

- ✅ Phase 5 smooth-movement runtime bridge: `dm2_v2_smooth_movement.c` provides struct-based `DM2_V2_SmoothState` with walk (ease-out cubic), turn (ease-out quad, shortest-path normalised), and stairs (ease-in-out cubic + vertical camera offset) interpolations over 1 V1 tick (55ms). Probe `firestaff_dm2_v2_smooth_movement_probe` covers lifecycle, walk N/S/E/W, turn 8 directions, stairs, deterministic input coverage, and pixel gate. Ctest target `test_dm2_v2_smooth_movement` passes 79/79. Plus binding seam: `dm2_v2_runtime.c` (DM2_V1_RuntimeProfile, `bind_to_v1`/`is_bound`/`force_sync`/`v1_tick`/`render_frame`) on top of SKULL.ASM T520 (party/movement tick) + T048 (input dispatch) + T560 (dungeon viewport) + T600 (outdoor viewport). Stairs takes priority over walk when both change. Integration test `test_dm2_v2_smooth_runtime_binding` 12 groups/43 asserts pass; ctest 2/2 (DM2 smooth + DM2 runtime binding).

## Dungeon Master Nexus

### Nexus V1

- ✅ Phase 0 - Provenance and source audit setup for Saturn DMDF/DGN references.
- ✅ Phase 1 - Runtime profile and launch/profile boundary scaffolding.
- ✅ Phase 2 - Data format ingestion for Nexus dungeon and supporting Saturn data structures.
- ✅ Phase 3 - Core world model and runtime state mapping.
- ✅ Phase 4 - Rendering pipeline slices and viewport/source-lock scaffolding.
- ✅ Phase 5 - Mechanics parity implementation for movement, click routing, item use, doors, pits, teleporters, triggers, combat, AI, and sound routes.
- ✅ Phase 6 - Save/import compatibility, including Nexus V1 save/load round-trip probe coverage.
- ✅ Phase 7 - Verification-suite coverage for compile, save/load, and runtime-state paths.
- 🔒 Source-lock audit coverage for Nexus DMDF/DGN loading, sensors, movement, input, inventory, doors, triggers, combat, AI, sound, save/load, and launch/runtime boundaries.

### Nexus V2

- ✅ Phase 5 smooth-movement runtime bridge: `nexus_v2_smooth_movement` exposes walk/turn/stairs visual state with ease-out cubic / ease-out quad / ease-in-out cubic interpolations, `nexus_v2_smooth_tick` auto-starts walk/turn animations on position/angle deltas with a `has_prev` baseline guard, and shortest-path turn normalization matches DM2/CSB V2 conventions. `test_nexus_v2_smooth_movement` ctest target passes 27/27; headless probe `firestaff_nexus_v2_smooth_movement_probe` passes 64/64 (lifecycle, walk N/S/E/W, turn 7 directions, stairs with vertical offset, auto-detect, wrap-around shortest path, null safety).
- ✅ Phase 6 touch/controller affordances: V2-only touch swipe, edge-strafe, D-pad, and dual analog stick affordances for the Saturn gamepad are mapped to `NEXUS_CMD_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT/STRAFE_LEFT/RIGHT` (1-6) through `nexus_v2_touch_controller_affordance.c` with V1 mouse/touch/click parity preserved (rejected when `v2PresentationEnabled=0`). Source-locked against ReDMCSB CLIKMENU.C:142-174/180-390 (F0365 turn / F0366 move), COMMAND.C:2045-2155 (F0380 queue dispatch), GAMELOOP.C:164-219 (V1 input wait loop), REALTIME.ASM T048, and the Saturn SDK joystick mapping / NEXUS.BIN input surface. Ctest target `test_nexus_v2_touch_controller_affordance` covers movement command mapping, input kind classification, V2 acceptance, V1 parity guard, name strings, source evidence, NONE affordance, and route kind constants. Headless probe `firestaff_nexus_v2_touch_controller_affordance_probe` passes 294/294 (API surface, mapping, classification, V2 route, V1 guard, NONE, idempotency, Saturn-specific right-stick turn-only, cross-game shape consistency with DM1/DM2/CSB sibling affordances, source evidence, route kind constants).

## Theron's Quest

### Theron V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ Phase 5 - Initial mechanics implementation for movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds.
- ✅ Phase 6 - Dungeon progression probe coverage.
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.

## Cross-Cutting

### Launcher and Settings

- ✅ M12 settings persistence bridge: quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, and all five per-game option slots round-trip through the startup menu probe.

### Touch and Input

- ✅ Launcher and entrance click-zone scaffolding.
- ✅ DM1 touch/click routes for movement, turning, status/champion selection, and item interaction.
- ✅ Nexus V2 touch/controller affordance layer: 16 affordances (4 touch swipes, 2 edge-strafe, 4 D-pad, 4 left stick, 2 right stick) mapped to `NEXUS_CMD_*` (1-6) with V1 parity guard.

### Accessibility

- ✅ Accessibility manifest writer and launcher/game-state scaffold.
- ✅ Launcher high-contrast palette and configurable font-scale foundation with M12 probe coverage.

### Platform and Packaging

- ✅ macOS Debug CMake build path.
- ✅ CI Phase A headless probe path.
- ✅ Release packaging scripts for macOS, Windows, and Linux preview builds.
- ✅ macOS app bundle icon resource wiring.

# Firestaff DONE - Completed Work

This file tracks completed capabilities by game. It is not a changelog; see git history and release notes for chronology.

## Maintenance Cadence

- ✅ `TODO.md` and `DONE.md` now document the rule that they must be updated at least twice per day while Firestaff work is active.
- ✅ `TODO.md` is reserved for remaining fixes/builds; `DONE.md` is reserved for completed work.
- ✅ Worker refill focus now supports the project priority order: DM1 V1, Theron V1, CSB V1, DM1 V2, CSB V2, DM2 V1, Theron V2, Nexus V1, Nexus V2.

## Legend

- ✅ Done / verified
- 🔒 Source-locked against original references

## Dungeon Master (DM1)

### DM1 V1 - Runtime and Source-Lock

- ✅ Movement and collision: cardinal movement, turning, wall/door/fake-wall blocking, cooldowns, stairs, pits, teleporters, blocked self-damage, empty-party group cleanup, and deterministic capture gates.
- ✅ Viewport wall evidence hardening: `g_dm1_wall_frame_bitmaps` is source-locked to the PC34 `G2107`/door-frame offset model and guarded by an asset-free null-write regression.
- ✅ Viewport D2L side-wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D2L wall frame clips source X 61..71 into viewport X 0..10, preserves C10 transparency, and leaves clipped/out-of-frame pixels untouched.
- ✅ Viewport parity wall fix: PC34 D3L2/D3R2 parity side-wall draws now select the opposite native `G2107_WallSet` bitmap once, flip it through a ReDMCSB-style scratch path, and keep a pixel regression for the no-prewired-temp route.
- ✅ Viewport regression sweep: `v1_viewport_floor_ornament_stair_gate`, `v1_viewport_front_wall_depth_gate`, and `v1_viewport_pit_floor_ornament_bug64_gate` are green again after restoring the explicit wall-free/wall-like source-bound helpers.
- ✅ Wall inscription source-font regression: readable D1C wall inscriptions now use the dedicated PC34 `M648_GRAPHIC_INSCRIPTION_FONT`/GRAPHICS.DAT index 258 with 8-pixel source centering, guarded by `firestaff_m11_inscription_font_probe`.
- ✅ Door-front occlusion pixel-zone gate: all 11 source-locked front-door branches prove rear cells are masked by door pixels and front cells draw after the door pass.
- ✅ Hall champion mirror visibility gate: `dm1_v1_champion_mirror_visibility_runtime` opens the DM1 V1 runtime, draws the known Hall mirror route, and pixel-matches the D1C wall portraits against the source GRAPHICS.DAT champion portrait strip.
- ✅ Hall champion mirror Z-order slice: `dm1_v1_champion_mirror_zorder_runtime` opens the DM1 V1 runtime, pixel-proves north/south/east D1C champion mirror portraits, and checks west-facing Hall side poses do not leave a floating D1C portrait.
- ✅ Creature and combat systems: creature groups, AI, attacks, deaths, drops, XP, projectile attacks, sounds, fleeing, special positioning, possession drops, Black Flame behavior, generator/teleporter/fall/drop cases, and Lord Chaos constants.
- ✅ Spells and magic: rune UI, spell casting, mana/skill checks, projectiles, shields, light/dark, open-door magic, poison cloud behavior, and spell failure paths.
- ✅ Champions: recruitment, active selection, health/stamina/mana bars, skill/XP updates, death/resurrection, stats panel routing, weight/load behavior, and stamina regeneration.
- ✅ Inventory chest compact-close regression: sparse open-chest contents now stay covered by a source-locked test that proves compact close returns the full non-empty count, honors a smaller output buffer, clears the open-chest panel, hides chest slots, and drops chest load.
- ✅ DM1 V1 chest visible-slot close gate: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C` only rewrites the eight visible C537..C544 chest slots on close, detaching a ninth linked object that never entered `G0425_aT_ChestSlots`.
- ✅ Survival, sensors, entrance, save/load, audio, and data loading: food/water decay, rest, stamina, sensor/timeline behavior, title/entrance flow, save/load routes, sound routing, and DUNGEON.DAT/GRAPHICS.DAT ingestion.
- ✅ Source-lock verifier hardening: viewport/walls landable metadata, wall-clip source audit, side-wall source-row clipping, D3/D2 wall-ornament order, front-cell collision, D0/D1 visible-square draw-order, wall-alcove C2548, champion stat panel, and ambient dungeon sound gates now resolve current local code/source boundaries and reflect the closed no-ambient-loop source boundary.
- 🔒 DM1 source-lock audit completed across movement, rendering, creatures, combat, spells, champions, inventory, survival, sensors, entrance, save/load, audio, and data structures.

### DM1 V2.0 / V2.1 / V2.2

- ✅ V2.0 filtered presentation: config, CRT scanlines, palette correction, dither cleanup, sharpening, renderer integration, and launcher/menu integration.
- ✅ V2 parity/presentation scaffold: Phase 0 and Phase 1 command routing, deterministic config, profile boundary, and launch-smoke verification.
- ✅ V2.1 asset pipeline: Phase 2 source-preserving upscale/EPX pipeline, deterministic cache behavior, fallback handling, and probe coverage.
- ✅ V2.2 modern asset pipeline: generated/modern art path, provenance/fallback contracts, and `dm1_v22_asset_pipeline` probe coverage.
- ✅ V2 presentation slices: HUD/action route gate, palette/projectile metadata gates, smooth-movement runtime bridge, touch/controller route gate, and presentation-disabled state-hash gate.
- ✅ DM1 V2 smooth turn pan backend: optional Custom/V2 turn-pan setting persists through config, the Phase 5 bridge can start pan-enabled turns, and the camera exposes a presentation-only viewport pan offset while V1 command direction changes remain source-owned.
- ✅ DM1 V2 Phase 4 field/projectile VFX binding gate: source explosion thing IDs map to V2 overlay/emitter families, fluxcage remains field-only, unknown things are rejected, and invalid source palette lighting falls back deterministically.

## Chaos Strikes Back (CSB)

### CSB V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, profile-specific asset discovery, boot state, diagnostics, and hash-matched launch boundary.
- ✅ Phase 2 dungeon-data probe slice: synthetic CSB dungeon loading, square/thing accessors, door table, sensor helpers, endgame helpers, world-model behavior, and CSB-vs-DM1 difference checks are covered by `firestaff_csb_v1_dungeon_model_probe`.
- ✅ Real CSB PC 3.4 DUNGEON.DAT ingestion: the V1 dungeon loader now accepts the hash-verified FTL-compressed CSB dungeon, decompresses/swaps it through the ReDMCSB FTL path, parses the 44-byte header and MAP descriptors, and keeps synthetic fixture coverage intact.
- ✅ Launch/profile fixture: the Atari ST asset-pair manifest and CSB launch-intent gate now recognize hash-matched CSB assets as valid for the M12 profile boundary while keeping gameplay, save, and pixel parity as non-claims.
- ✅ Source-lock audit coverage for CSB startup, utility, dungeon loading, wall rendering, champion import, weapons, magic, creatures, combat, and save behavior.
- ✅ Phase 4 - Mechanics parity slices for CSB-specific movement/interaction/runtime behavior.
- ✅ Phase 5 - Creature and combat parity slices.
- ✅ Phase 6 - Utility/import flow: champion import from DM1 saves (256-byte CSB champion block format, DM1 116-byte record → CSB block conversion, ReDMCSB SAVEGAME.C F0100-F0120 state machine), utility disk flow state machine (INIT→INSERT_DISK→VERIFY_DISK→DISK_OK→SELECT_ACTION→IMPORT/LOAD/NEW→DONE), disk verification, import confirmation, save-game load stub; headless probe `firestaff_csb_v1_utility_import_probe` passes 33/33 tests.
- ✅ Completed rendering slices: D3/D2 wall-table mapping, parity bitmap selection, grid routing, and initial CSB viewport source-lock gates.
- ✅ CSB V1 viewport Phase 3 gate: D3L2/D3R2 and D2L2/D2R2 draw-order, coordinate, frame, and PC34 zone contracts are source-locked against F0676-F0679/F0128.
- ✅ CSB V1 back-wall ornament routing gate: D3L2/D3R2 wall cases source-lock their F0107 ordinal slots and view-wall indices, while D2L2/D2R2 prove the no-F0107 return path.
- ✅ CSB V1 F0108/F0111 route gate: D3L2/D3R2 source-lock the floor-view indices, pit/corridor/door-front F0108 calls, door-front F0115 pass orders, and F0111 door panel zones.
- ✅ Phase 7 - Verification suite: deterministic boot, dungeon, combat, save/import, and rendering probes pass.

### CSB V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock before V2 work.
- ✅ Phase 1 - V2 launch/profile separation.
- ✅ Phase 7 deterministic verification gate: `test_csb_v2_phase7_verification` is green again after restoring CSB's accumulated V1-tick phase reporting while keeping render-frame deltas for smooth movement.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, asset discovery, launcher state, and runtime selection.
- ✅ Real-asset probe regression sweep: `probe_dm2_v1_asset_loader`, `probe_dm2_v1_dungeon_loader`, `probe_dm2_v1_object_model`, `probe_dm2_v1_world_state`, and `test_dm2_v1_save_load` pass against the local hash-verified PC English data.
- ✅ Phase 6 - Utility/import flow: DM2-specific load/start flow and compatibility behavior.
- ✅ Phase 8 - Verification-suite scaffold and probes.
- ✅ Source-lock audit coverage for DM2 boot, dungeon/data loading, rendering, items, creatures, combat, spells, shops/NPCs, save behavior, and verification paths.

### DM2 V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock before V2 work.
- ✅ Phase 1 - V2 launch/profile separation: DM2_V2_PHASE_DOMAIN_LAUNCH and _PROFILE gates implemented; 42/42 probe pass; commit 22838e8f.
- ✅ Phase 2 - Enhanced asset pipeline.
- ✅ Phase 3 - Enhanced UI overlays: HUD compass/depth/gold/champion bars/action strip, launcher/menu integration, and probe coverage.
- ✅ Phase 4 - Enhanced lighting and outdoor effects: palette mapping, shadow rendering, outdoor atmosphere VFX, and probe coverage.
- ✅ Phase 5 - Smooth movement and viewport interpolation: visual walk/turn/stair interpolation state, viewport query hooks, and source-evidence strings while preserving V1 tick ownership of game-state movement; runtime binding and deterministic input coverage complete.
- ✅ Phase 6 - Touch/controller ergonomics: touch/controller affordance probe passes 4/4.
- ✅ Phase 7 - V2 verification suite.

## Dungeon Master Nexus

### Nexus V1

- ✅ Phase 0 - Provenance and source audit setup for Saturn DMDF/DGN references.
- ✅ Phase 1 - Runtime profile and launch/profile boundary scaffolding.
- ✅ Phase 2 - Data format ingestion for Nexus dungeon and supporting Saturn data structures.
- ✅ Phase 3 - Core world model and runtime state mapping.
- ✅ Phase 4 - Rendering pipeline slices and viewport/source-lock scaffolding.
- ✅ Phase 6 - Save/import compatibility, including Nexus V1 save/load round-trip probe coverage.
- ✅ Phase 7 - Verification-suite coverage for compile, save/load, and runtime-state paths.
- 🔒 Source-lock audit coverage for Nexus DMDF/DGN loading, sensors, movement, input, inventory, doors, triggers, combat, AI, sound, save/load, and launch/runtime boundaries.

### Nexus V2.0 / V2.1 / V2.2

- ✅ Phase 2 - Enhanced asset pipeline and upscaler probe coverage.
- ✅ Phase 3 - Enhanced UI overlays.
- ✅ Phase 4 - Enhanced lighting and Saturn presentation effects.
- ✅ Phase 6 - Touch/controller ergonomics.

## Theron's Quest

### Theron V1

- ✅ Track 02 bank evidence slice: the US ISO now has a regression probe for the unique 9-word bank-stride descriptor candidate at `0x1584`, while the JP Rev 1 zero-filled ISO is explicitly classified as insufficient evidence instead of claiming an offset.
- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Phase 4 - Rendering pipeline: Theron viewport/UI presentation and asset-selection probes pass.
- ✅ Phase 5 - Initial mechanics implementation: movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds have focused probe coverage.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ M11 runtime handoff now branches Theron's Quest away from the DM1 DUNGEON.DAT loader into a Track 02 boot profile, Theron world state, native viewport/UI renderer, idle tick, and basic movement input path.
- ✅ MyAbandonware JP Rev 1 and US Track 02 ISO images are hash-recognized by the scanner/boot path, report Theron's Quest READY, and direct-launch into the Theron M11 viewport with deterministic fallback assets.
- ✅ Phase 6 - Dungeon progression probe coverage.
- ✅ Phase 7 - Save/import compatibility.
- ✅ Phase 8 - Verification-suite coverage for deterministic launch, dungeon progression, mechanics, rendering, and save/load paths.
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.

## Cross-Cutting

### Launcher and Settings

- ✅ M12 settings persistence bridge: quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, and all five per-game option slots round-trip through the startup menu probe.
- ✅ M11 inventory source-lock regression sweep: `m11_inventory_scroll_panel_render_source_lock` and `m11_inventory_mouth_visual_blit_source_lock` are green on local macOS data roots as well as the legacy CI path.
- ✅ Recursive game-data scanner: `--scan-data` / `--scan-game-data` reports found and missing required files by MD5 hash, ignores filename and folder layout, scans stored ZIP entries plus ISO/BIN ISO 9660 contents, and treats TITLE/FTL/intro extras as non-blocking.
- ✅ Archive game-data handoff: DM1/CSB/DM2 required files found inside ZIP/ISO containers are materialized into the Firestaff asset cache and launched through normal runtime file paths.
- ✅ Game-data launch gating: DM1/CSB/DM2 now require both GRAPHICS and DUNGEON hashes before launch, Nexus/Theron require their primary hash marker, and direct `--game` launches are blocked when required data is missing.
- ✅ Start-menu data status wiring: game cards reflect required hash availability, a data-directory settings row exists, and missing required data prevents launch.
- ✅ Start-menu game-data popup: startup automatically scans the configured data directory, shows an OK popup when no game data is found, and unavailable games now report the specific required files missing from the hash-verified scan.

### Touch and Input

- ✅ Launcher and entrance click-zone scaffolding.
- ✅ DM1 touch/click routes for movement, turning, status/champion selection, and item interaction.

### Accessibility

- ✅ Accessibility manifest writer and launcher/game-state scaffold.
- ✅ Launcher high-contrast palette and configurable font-scale foundation with M12 probe coverage.

### Platform and Packaging

- ✅ macOS Debug CMake build path.
- ✅ CI Phase A headless probe path.
- ✅ Firestaff queue failed-job cleanup: fixed the recurring Nexus V1 mechanics, CSB V2 phase-gate, CSB V1 DSA output-dir, and stale DM2 probe-argument failures; requeued the failed pool entries and verified the worker pool returned to 0 failed jobs.
- ✅ Phase A / headless M12 no-data asset-status scan probe: `firestaff_m12_no_data_scan_probe` exercises M12_AssetStatus_Scan across non-existent dir, NULL/empty requestedDataDir, NULL status pointer, fresh struct poisoning, and re-scan determinism. The probe isolates itself from the host user data tree by pointing HOME / FIRESTAFF_DATA / XDG_DATA_HOME / XDG_CONFIG_HOME at a fresh empty tempdir. 15 top-level invariants (INV_N01..INV_N15) expand to 208 sub-assertions, all pass in ctest `m12_no_data_scan` (~50ms, no game data, no SDL window).
- ✅ Release packaging scripts for macOS, Windows, and Linux preview builds.
- ✅ macOS app bundle icon resource wiring.

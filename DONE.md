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
- ✅ DM1 V2 presentation-mode selection: `dm1_v2_presentation_mode_pc34` module (include/dm1_v2_presentation_mode_pc34.h, src/dm1v2/dm1_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the DM1 V2 presentation runtime with V22→V21 fallback when the modern asset pack is absent. `dm1_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=dm1). Per-mode settings defaults: `v2_settings_apply_v20_defaults()` (no upscale, palette correction on), `v2_settings_apply_v22_defaults()` (2x scale, bilinear on, per-material palette), alongside the existing V2.1. V22 entry branch initialises `m11_v22_shapes_init()` for modern shape table. CTEST target `test_dm1_v2_presentation_mode_pc34` passes 50/50, headless probe `firestaff_dm1_v2_presentation_mode_probe` passes 30/30. Source-locked against ReDMCSB COMMAND.C F0359 LoadGameSettings, CLIKMENU.C F0365/F0366 V1 source-locked turn/move, CSBWin/Graphics.cpp:3186 V2.0 filter pair, CSBWin/Viewport.cpp:7290 V2.1 EPX-style blit, DM1 PC 3.4 GRAPHICS.DAT V2.2 modern asset pack.

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

- ✅ CSB V2 presentation-mode selection: `csb_v2_presentation_mode_pc34` module (include/csb_v2_presentation_mode_pc34.h, src/csb/csb_v2_presentation_mode_pc34.c) parallel to the DM1 V2 module. M12_PRESENTATION_* enum → CSB_V2_PM_V1_FAITHFUL/V20/V21/V22 with the same V22→V21 fallback. `csb_v2_presentation_mode_set_m12()` is called from M11_GameView_Start (gameId=csb). CSB and DM1 presentation-mode globals are independent (set DM1 V22 + CSB V1: DM1 stays V22, CSB stays V1). CTEST target `test_csb_v2_presentation_mode_pc34` passes 36/36, headless probe `firestaff_csb_v2_presentation_mode_probe` passes 27/27. Source-locked against ReDMCSB COMMAND.C F0359/F0361, CLIKMENU.C F0365/F0366, DUNGEON.C:35-44 CSB direction step tables, GAMELOOP.C:150-155 V1 tick cadence, ENTRANCE.C CSB prison door, CSBWin/Viewport.cpp:7290, CSBWin/Chaos.cpp:60-69 DSA dispatch, CSBWin/Graphics.cpp:3186.
- ✅ Phase 5 smooth-movement runtime bridge: `csb_v2_smooth_movement.c` provides visual walk (ease-out cubic), turn (ease-out quad), and stairs (ease-in-out cubic + vertical camera offset) interpolations over 1 V1 tick (55ms). Global state is driven via a `V2_AnimClock*` to `csb_v2_smooth_update_from_clock`. Headless probe `firestaff_csb_v2_smooth_movement_probe` covers lifecycle, walk N/S/E/W, turn 8 directions, stairs with vertical offset, and deterministic input coverage; ctest target `test_csb_v2_smooth_movement` passes 50/50. Plus binding seam: `csb_v2_runtime.c` (CSB_V1_RuntimeProfile, `bind_to_v1`/`is_bound`/`force_sync`/`v1_tick`/`render_frame`) auto-triggers walk/turn/stairs on V1 deltas (F0365/F0366/F0364). Integration test `test_csb_v2_smooth_runtime_binding` 12 groups/43 asserts pass; ctest 2/2 (CSB smooth + CSB runtime binding).
- ✅ CSB V2.2 modern shape book (9-square): `csb_v22_shapes.c` (include/csb_v22_shapes.h) supplies `csb_v22_shape_for_cell`, `csb_v22_shape_for_view_square`, `csb_v22_wall_shape_get`, `csb_v22_floor_shape_get`, `csb_v22_material_get`, `csb_v22_material_count`, `csb_v22_shape_for_prison_door`, and the 9-square bridge. Source-locked against CSBWin/Viewport.cpp:7290 and ReDMCSB M034_SQUARE_TYPE. Fix in `csb_v22_floor_shape_get`: stairs direction was reading `flags & 0x01` but `flags` was already masked to 0xF0 so the lower bit was always zero, making both `0x10` and `0x11` resolve to STAIRS_UP. Changed to `base & 0x01` so `0x10` → up (dir=0) and `0x11` → down (dir=1). ctest target `test_csb_v22_shapes_pc34` passes 54/54 (1 pre-fix FAIL was `stairs down`). Headless probe `firestaff_csb_v22_shapes_probe` passes 21/21. Disjoint from CSB V2.0/V2.1/V2.2 selection API and from the DM1 V2.2 shape book (the two are mirrors, not duplicates, because CSB has the prison door and the CSB-specific stair material book).
- ✅ CSB V2.1 texture upscale test+probe (9-square + panel + V22 EPX): `test_csb_v2_texture_upscale_pc34` and `firestaff_csb_v2_texture_upscale_probe` had pre-fix test bugs that made `t_epx_2x`, `t_9square_viewport`, `t_panel`, and `t_present_mode_v22_triggers_epx` (and the matching probe checks) fail when the source pattern starts with 0. The `epx_buf[0] != 0` check was wrong because EPX writes the source pixel P to the corresponding output, so for `src[0] = 0` the EPX rule writes 0 to `epx_buf[0]` and the check always failed. Fix: `t_9square_viewport` and `t_panel` now `memset(epx_buf, 0xCC, ...)` before calling and check `epx_buf[0] != 0xCC`. `t_epx_2x` keeps the `{10,20,10,20}` input but documents that all four EPX neighbour predicates miss on this pattern (so output is the P-fallback column-stripe), with new expectations `dst[0]==10 && dst[1]==10 / dst[2]==20 && dst[3]==20 / ...`. Probe mirrors the same sentinel approach. Local verification: `ctest -R csb_v2|csb_v22|dm1_v2_shape_runtime` 12/12, `firestaff_csb_v2_texture_upscale_probe` 13/13, `test_csb_v2_texture_upscale_pc34` 30/30.

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
- ✅ Phase 5 render-pipeline smooth-movement tick (commit `7ca73871`): `Nexus_V2_RenderPipeline` now owns a `Nexus_V2_SmoothState`, `nexus_v2_pipeline_init()` calls `nexus_v2_smooth_init()` and logs the smooth_movement mode, the new `nexus_v2_pipeline_tick(pipe, game_x, game_y, game_angle)` records raw V1 state per tick and auto-triggers walk/turn animations on position/angle deltas, and `nexus_v2_pipeline_render()` derives camera position/angle from the smooth state when `smooth_movement` is enabled and falls back to the raw V1 state otherwise. The render signature changed from explicit `(cam_x, cam_y, cam_z, cam_dir)` to `(game_x, game_y, game_angle)` to make the contract explicit that the pipeline owns the interpolation. Builds clean in Release and Debug with zero warnings.

## Theron's Quest

### Theron V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ Phase 4 - Rendering pipeline: viewport, tile renderer, palette, and UI chrome are wired into the Theron static library; rendering probes (`firestaff_theron_v1_viewport_renderer_probe`, `firestaff_theron_v1_tile_renderer_probe`) and the rendering integration test (`test_theron_rendering`) are built and green.
- ✅ Phase 5 - Mechanics implementation for movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds, with a 50-assertion mechanics hardening probe (`firestaff_theron_v1_mechanics_hardening_probe`) and a deterministic teleporter-chain probe (`firestaff_theron_v1_teleporter_chain_probe`).
- ✅ Phase 5 - Shop and world-serialization regressions: price-table guard (`test_theron_v1_shop_price_table`) and purchase-state round-trip (`test_theron_v1_world_serialize_purchase_state`) cover parser bounds and party-block atomicity.
- ✅ Phase 5 - Direct-launch path: hash-verified Track 02 loading without re-walking the data root is covered by `test_theron_v1_direct_launch` and the M11 handoff `test_theron_v1_m11_direct_launch`.
- ✅ Phase 5 - Launcher scan reuse: `test_theron_v1_launcher_scan_reuse` exercises the `M12_AssetStatus_Test*` helper path and proves the M12 launcher reuses the verified Theron path and hash on refresh.
- ✅ Phase 6 - Dungeon progression probe coverage.
- ✅ Phase 7 - Save/load coverage: `test_theron_v1_save_load`, `test_theron_v1_save_header_rejection`, and the `firestaff_theron_v1_track02_bank_probe` lock the save header, slot layout, and Track 02 bank signal contracts.
- ✅ Phase 8 verification suite wire-up: test_theron_v1_direct_launch, test_theron_v1_m11_direct_launch, test_theron_v1_launcher_scan_reuse, test_theron_v1_dungeon_progression, test_theron_v1_save_load, test_theron_rendering, test_theron_v1_save_header_rejection, test_theron_v1_shop_price_table, test_theron_v1_world_serialize_purchase_state, plus probes firestaff_theron_v1_teleporter_chain_probe, firestaff_theron_v1_mechanics_hardening_probe, firestaff_theron_v1_viewport_renderer_probe, firestaff_theron_v1_tile_renderer_probe, firestaff_theron_v1_track02_bank_probe are all wired into ctest and pass (17/17 dungeon progression, 9/9 save/load, 18/18 rendering, 3 NEW direct-launch + M11 + scan-reuse tests, 3 NEW viewport/tile/track02 probes).
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.

### Theron V2.0 / V2.1 / V2.2

- ✅ Theron V2 presentation-mode selection: `theron_v2_presentation_mode_pc34` module (include/theron_v2_presentation_mode_pc34.h, src/theron/theron_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the Theron V2 presentation runtime. `theron_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=theron). Fallback chain V22→V21 when the modern asset pack is absent. Three independent presentation-mode globals (DM1/CSB/Theron) verified by `t_independent_from_dm1_csb`. CTEST target `test_theron_v2_presentation_mode_pc34` passes 40/40, headless probe `firestaff_theron_v2_presentation_mode_probe` passes 23/23. Source-locked against ReDMCSB COMMAND.C F0359, CLIKMENU.C F0365/F0366, MOVESENS.C:475-538, THQUEST.ASM T400/T520/T560/T600/T700/T800/T900, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7.

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

### Build and CI Health

- ✅ Compile-warning cleanup (commit `47f7bb8c`): silenced 270+ Clang and GCC warnings across all targets so the strict-warnings CI matrix (`-Wall -Wextra -Werror` on macos-14/ubuntu-24.04/windows-2022) goes green. Categories fixed: `-Wunused-variable / -Wunused-const-variable / -Wunused-parameter / -Wunused-but-set-variable / -Wunused-local-typedef` (documentation arrays, stub function bodies, the 4 leftover `AssetMd5*` typedefs in `csb_v1_runtime_pc34_compat.c`); `-Wswitch` for the 10 CSB-specific view-square cases in `dm1_v1_viewport_3d_pc34_compat.c` (D3L2/D3R2/D2L2/D2R2 macros, applied via `set_source_files_properties` to avoid offsetting the line counter that parity-evidence source-lock verifiers pin); `-Wcomment` for two missing `*/` closings in `memory_creature_ai_pc34_compat.c` and the dm1 special-square interaction probe plus two nested-`/*` cases in `cloud_sync_m12.h` and the launcher menu text; `-Wincompatible-pointer-types-discards-qualifiers` for the const-mismatch on `F0735_COMBAT_ResolveChampionMelee_Compat` (declaration in `memory_combat_pc34_compat.h` was wrong — the function mutates `statisticLuck`) and the 3 Theron viewport call sites that passed a const leader to a non-const accessor (switched to the existing `_c` variants of `theron_v1_party_leader` / `theron_v1_party_getChampion`); `-Wmissing-field-initializers` for the `g_config` `source_light_floor` field on `DM2_V2_AssetPipelineConfig` and `CSB_V2_AssetPipelineConfig`; `-Wsign-compare` in `theron_v1_dungeon_progression_test.c`. CMake `-Wno-maybe-uninitialized` and `-Wno-restrict` flags are now guarded behind `CMAKE_C_COMPILER_ID STREQUAL "GNU"` so Clang/MSVC do not warn about unknown warning options. Verification: Release and Debug builds both produce zero code warnings and zero errors; ctest reports the same 10 pre-existing failures with and without this commit (parity-evidence line-drift from prior watchdog passes, not caused by this change). Remaining CI-cleanup items are tracked under `TODO.md` Cross-Cutting → Build and CI Health.
- ✅ v2.8.0 GitHub Actions release: full multi-platform release (commit `5864933b`, tag `v2.8.0`, published 2026-06-16 09:47 UTC). All five release jobs green: macOS arm64 (1m26s), macOS x86_64 (2m27s), Linux arm64 (2m19s), Linux x86_64 (2m02s), Windows x86_64 (2m35s), plus Publish (26s). 16 assets published to https://github.com/yeager/firestaff/releases/tag/v2.8.0: macOS arm64/x86_64 .dmg + .zip, Windows .zip + .exe installer, Linux x86_64/arm64 .deb + .rpm, plus per-platform and combined SHA256 manifests. Four additional CMake fixes shipped in the same release cycle to make the cross-platform link work: (a) moved `m11_game_text_latin_extended_glyphs.c` from `firestaff_m11` to `firestaff_m10` so the helper symbol is co-located with its caller `m11_find_glyph_utf8` (commit `435f7644`); (b) added `libm` to the `test_csb_v2_smooth_runtime_binding` link line under the existing `UNIX AND NOT APPLE` guard (commit `86cfb1f7`, for `sinf`/`cosf` from `csb_v2_lighting_dynamic.c`); (c) added `libm` to the `test_nexus_v2_smooth_movement` link line under the same guard (commit `2ff2865f`, for `fmodf` from `nexus_v2_smooth_movement.c`); (d) re-linked `firestaff_m12` after `firestaff_m11` for `test_theron_v1_m11_direct_launch` so the GNU/MinGW link resolves the cross-archive `m11_game_view.c → M12_*` references that macOS ld resolves implicitly (commit `3d93db95`). All three linker-issues were pre-existing on Linux/Windows even before the v2.8.0 work and only surfaced when the strict-`ld` CI matrix was exercised end-to-end; the TODO entry about duplicate-libraries warnings is now resolved (macOS ld emits the warning, which is benign and known).

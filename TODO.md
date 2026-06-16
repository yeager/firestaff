# Firestaff TODO - Open Work

This file tracks remaining work only. Completed work belongs in `DONE.md`.

- ✅ 2026-06-16 Per-game V2 settings in M12 menu config: `M12_Config` + `M12_MenuSettingsState` extended with `csbV2*` and `theronV2*` fields (scalePercent, bilinearEnabled, crtScanlinesEnabled, crtScanlineStrength, paletteCorrectionEnabled, ditherCleanupEnabled — mirrors the existing `dm1V2*` pattern). Defaults 200% scale, 0 bilinear, 0 scanlines, 35 strength, 0 palette, 0 dither. Round-tripped through `M12_Config_SetDefaults` + the text Load + the text Save + the JSON Export + the JSON Import. New bridge modules `csb_v2_settings_pc34` + `theron_v2_settings_pc34` mirror `dm1_v2_settings_pc34`: `CSB_V2_Settings` / `Theron_V2_Settings` struct, `_from_m12_config` / `_apply_to_m12_config` / `_apply_to_runtime` (pushes scale + bilinear into the respective `_upscale_init`). CTEST: `test_csb_v2_settings_pc34` 23/23, `test_theron_v2_settings_pc34` 23/23. Probes: `firestaff_csb_v2_settings_probe` 13/13, `firestaff_theron_v2_settings_probe` 12/12. Source-locked against include/dm1_v2_settings_pc34.h, include/csb_v2_texture_upscale_pc34.h, include/csb_v22_shapes.h, include/theron_v2_texture_upscale_pc34.h, include/theron_v22_shapes.h, include/config_m12.h, THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE. **Wire-up done:** `M11_GameView_OpenSelectedMenuEntry` in src/engine/m11_game_view.c now reads `menuState->settings.csbV2*` / `theronV2*` and calls `csb_v2_settings_apply_to_runtime()` / `theron_v2_settings_apply_to_runtime()` right before `M11_GameView_Start`, so the saved scale + bilinear reach the live V2 runtime. New headless probe `firestaff_m12_v2_settings_wire_up_probe` 16/16: scale 100/200/400 → runtime 1/2/4 for both CSB + Theron, bilinear 0/1 round-trips, invalid values clamp, CSB + Theron globals are independent. DM1 V2 settings already had their own dispatch via `M11_Render_Set*` inside `M11_GameView_Start`.
- 🔧 2026-06-16 V2 presentation-mode pass: dm1_v2_presentation_mode_pc34 + csb_v2_presentation_mode_pc34 modules wired to M11_GameView_Start (spec->presentationMode is now pushed into the per-game V2 runtime for gameId=dm1 and gameId=csb). v2_settings_apply_v20_defaults (V2.0 filtered), v2_settings_apply_v22_defaults (V2.2 modern) added alongside the existing v2_settings_apply_v21_defaults. dm1_v22_shapes.c now explicitly included in the firestaff_v2 lib so the V22 entry branch resolves m11_v22_shapes_init. New CTEST targets: test_dm1_v2_presentation_mode_pc34 (50/50), test_csb_v2_presentation_mode_pc34 (36/36). New headless CI probes: firestaff_dm1_v2_presentation_mode_probe (30/30), firestaff_csb_v2_presentation_mode_probe (27/27). 23/23 Phase A invariants still pass. The 166/166 green set confirms the M12_PRESENTATION_V1_ORIGINAL / V20_FILTERED / V21_UPSCALED / V22_MODERN enum now reaches the per-game V2 runtime; remaining work is the actual CSB V2.1 upscale + CSB V2.2 modern asset pack authoring (the selection API is ready, the per-mode V2 modules are still TODO for CSB), plus Phase 8 m11_v22_shape_for_cell() runtime use from the V22 active branch.
- ✅ 2026-06-16 CSB V2.1 + V2.2 follow-up: csb_v2_texture_upscale_pc34 (mirror of dm1_v2_texture_upscale with csb_ prefix, 9-square viewport + panel helpers), csb_v22_shapes (9-square shape book with CSB-only PRISON_DOOR/CHAOS_RUNE/DSA_SCROLL/LORD_ORDER), dm1_v2_shape_runtime (V2.2 shape dispatch wrapper around m11_v22_shape_for_cell, with M11_GameView_Start hook). CTEST: test_csb_v2_texture_upscale_pc34 30/30, test_csb_v22_shapes_pc34 54/54, test_dm1_v2_shape_runtime_pc34 46/46. Probes: firestaff_csb_v2_texture_upscale_probe 13/13, firestaff_csb_v22_shapes_probe 21/21, firestaff_dm1_v2_shape_runtime_probe 12/12. The CSB V2.1 upscale + CSB V2.2 shape modules are now wired into csb_v2_presentation_mode_set() so they activate when the user picks CSB V2.1 or V2.2. **V2.0 filter config also done (this pass):** new `csb_v2_filter_config_pc34` module stores the per-frame filter toggles (crtScanlinesEnabled, crtScanlineStrength, paletteCorrectionEnabled, ditherCleanupEnabled) as a module-level global; `csb_v2_settings_apply_to_runtime` now also calls `csb_v2_filter_config_apply()` so the M11 launch wire-up pushes both upscale AND filter config in a single call. CTEST: test_csb_v2_filter_config_pc34 26/26. Probe: firestaff_csb_v2_filter_config_probe 21/21. Remaining work: actual CSB modern asset pack authoring at ~/.firestaff/assets/csb/modern/ + GPU renderer integration (the runtime dispatch is in place but the renderer is a follow-up).
- 🔧 2026-06-16 Theron V1 wire-up: 3 previously-unwired V1 tests (theron_v1_direct_launch_test, theron_v1_launcher_scan_reuse_test, theron_v1_m11_direct_launch_test) + 2 unwired probes (firestaff_theron_v1_viewport_renderer_probe, firestaff_theron_v1_tile_renderer_probe) are now in ctest + the linker, all green. firestaff_theron_v1_track02_bank_probe also wired. THERON_SOURCES glob extended to also match src/theron/theron_v2_*.c (otherwise theron_v2_presentation_mode would be a dangling include). M12 lib now compiles with FIRESTAFF_ASSET_STATUS_TESTING=1 so the launcher_scan_reuse test's M12_AssetStatus_Test* symbols are in the .a. All 9 Theron V1 tests + 5 Theron V1 probes pass.
- ✅ 2026-06-16 Theron V2 presentation-mode selection: theron_v2_presentation_mode_pc34 module (include + src) maps M12_PRESENTATION_V1/V20/V21/V22 onto the Theron V2 presentation runtime. M11_GameView_Start extended to call theron_v2_presentation_mode_set_m12() when gameId=theron. Three independent presentation-mode globals (DM1/CSB/Theron) verified. CTEST: test_theron_v2_presentation_mode_pc34 40/40, firestaff_theron_v2_presentation_mode_probe 23/23. **Follow-up (now done):** Theron V2.1 upscale module (theron_v2_texture_upscale_pc34 mirror of csb_v2_texture_upscale for HuC6260 256x224 NTSC base, with NTSC fullscreen + 192x160 dungeon-viewport helpers), Theron V2.2 shape book (theron_v22_shapes 4×3 layout with Theron-only TELEPORTER/ALARM/SECRET_DOOR/FLOODED/LIT_TORCH shapes), both wired into theron_v2_presentation_mode_set() so they activate on V2.1/V2.2 selection. CTEST: test_theron_v2_texture_upscale_pc34 25/25, test_theron_v22_shapes_pc34 62/62. Probes: firestaff_theron_v2_texture_upscale_probe 14/14, firestaff_theron_v22_shapes_probe 26/26. **V2.0 filter config also done (this pass):** new `theron_v2_filter_config_pc34` module parallels the CSB filter config for the PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline. CTEST: test_theron_v2_filter_config_pc34 24/24. Probe: firestaff_theron_v2_filter_config_probe 18/18. Remaining work: actual Theron modern asset pack at ~/.firestaff/assets/theron/modern/ + DM1 V2.2 actual GPU render path (the dispatch hook is in place via dm1_v2_shape_runtime_v22_active()).
- ✅ 2026-06-16 csb_v2_texture_upscale test-bug (RESOLVED, awaiting push): `t_epx_2x` (and downstream 9square/panel/V22 EPX checks) expected nearest-2x output from `csb_v2_upscale_epx()` but the actual EPX rule returns P when neighbour conditions are not met. Resolved in two ways: (1) `t_epx_2x` now documents the P-fallback and expects the column-stripe nearest output (row 0 = src col 0, row 1 = src col 0 still, since the 2x2 P=src[0]/src[1] drives both y-stride rows). (2) `t_9square_viewport`, `t_panel`, and `t_present_mode_v22_triggers_epx` use a `memset(epx_buf, 0xCC, ...)` sentinel so the EPX-wrote-it check is `epx_buf[0] != 0xCC` (not `!= 0`, which was always false when src[0] = 0). Same approach applied to the `firestaff_csb_v2_texture_upscale_probe` and the new Theron V2.1 probe/test. Ctest `csb_v2_texture_upscale` 30/30, probe 13/13. DM1 V2.1 `dm1_v2_upscale_epx` shares the same rule but the DM1 test (where it exists) was not yet audited; pending check.
- 🐛 2026-06-16 csb_v2_texture_upscale_pc34 test-bug (RESOLVED locally, awaiting push): `t_epx_2x` (and downstream 9square/panel/V22 EPX checks) expected nearest-2x output from `csb_v2_upscale_epx()` but the actual EPX rule returns P when neighbour conditions are not met. Resolved in two ways: (1) `t_epx_2x` now documents the P-fallback and expects the column-stripe nearest output (row 0 = src col 0, row 1 = src col 0 still, since the 2x2 P=src[0]/src[1] drives both y-stride rows). (2) `t_9square_viewport`, `t_panel`, and `t_present_mode_v22_triggers_epx` use a `memset(epx_buf, 0xCC, ...)` sentinel so the EPX-wrote-it check is `epx_buf[0] != 0xCC` (not `!= 0`, which was always false when src[0] = 0). Same approach applied to the `firestaff_csb_v2_texture_upscale_probe`. Local: `ctest -R csb_v2_texture_upscale` now 30/30, probe 13/13. DM1 V2.1 `dm1_v2_upscale_epx` shares the same rule but the DM1 test (where it exists) was not yet audited; pending check.

## Legend

- ❌ Not started
- 🔧 In progress / partial
- 🐛 Known bug

## Dungeon Master (DM1)

### DM1 V1

- 🔧 Original DOS capture parity: five specific paired evidence sets are blocked. Details and honest status labels at `docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`. Minimum runbook at `docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`.
  - Viewport: original pass94 captures exist (2026-04-28) but are impaired — frames 03–06 have duplicate SHA256, pass80 classifier reclassifies them as `entrance_menu`/`wall_closeup` instead of `dungeon_gameplay`. DOSBox input route failed to enter dungeon. New capture session with working dungeon-entry sequence required.
  - Wall: no paired original wall screenshot exists. Wall composition is source-locked only.
  - Collision: no paired original collision transcript exists. Collision logic is source-locked only.
  - Creature-chain: no paired original creature screenshot exists. Creature render is source-locked only.
  - Champion-panel: Firestaff V1 captures exist (party_hud_four_champions_vga.ppm, party_hud_statusbox_gfx_vga.ppm) but no paired original DM1 PC 3.4 champion panel screenshot exists.
  Canonical game data verified: DUNGEON.DAT SHA256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`, GRAPHICS.DAT SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`. Firestaff-side gates, source locks, and runtime routing are complete.
- 🔧 Inventory/chest polish beyond source-locked routes: core slot/body/chest/backpack/source routes are implemented; remaining work is broader chest runtime detail coverage and pixel-polish evidence.
- 🐛 P1 visual bugs needing capture/repro: missing or incorrect viewport walls, champion Z-order/floating, champion mirrors not visible, and blurry wall inscriptions. Treat each as unconfirmed until it has a reproducible capture or focused probe.

### DM1 V2.0 / V2.1 / V2.2

- 🔧 Phase 3 - Modern UI overlay hardening: HUD/action route gates exist; remaining work is optional inventory, champion, rune, and action-panel polish without bypassing V1 command routes or inventory transactions.
- 🔧 Phase 4 - Lighting and visual effects hardening: palette/projectile metadata gates and field/projectile VFX binding gates exist; remaining work is full enhanced lighting, shadows, broader field effects, and deterministic fallback coverage.
- 🔧 Phase 5 - Smooth movement presentation hardening: runtime bridge/gates and optional Custom/V2 smooth turn-pan camera backend exist; remaining work is broader interpolation coverage and launcher UI polish while preserving V1 cooldowns, collision, sensors, creature timing, and redraw cadence.
- 🔧 Phase 6 - Touch/controller ergonomics hardening: route gates exist; remaining work is broader V2-only gesture/controller affordances with V1 touch/click parity preserved.
- 🔧 Phase 7 - V2 verification suite hardening: presentation-disabled state-hash gate exists; remaining work is full side-by-side V1/V2 deterministic input scripts plus screenshot/pixel gates for enhanced presentation.
- 🔧 Phase 8 - V2.2 modern asset pipeline: presentation-mode selection API is now wired through M11_GameView_Start (dm1_v2_presentation_mode_set_m12 + csb_v2_presentation_mode_set_m12), m11_v22_shapes_init() is called on V22 entry, and v2_settings_apply_v22_defaults() / v2_settings_apply_v20_defaults() / v2_settings_apply_v21_defaults() exist per mode. Remaining work: actual modern art pack authoring + per-cell m11_v22_shape_for_cell() runtime usage from the V22 active branch + per-mode pixel/material verification gates.
- ✅ V2 presentation-mode selection wiring: dm1_v2_presentation_mode_pc34 + csb_v2_presentation_mode_pc34 modules (include/dm1_v2_presentation_mode_pc34.h, include/csb_v2_presentation_mode_pc34.h, src/dm1v2/dm1_v2_presentation_mode_pc34.c, src/csb/csb_v2_presentation_mode_pc34.c) expose M12_PRESENTATION_* selection with V22→V21→V20→V1 fallback chain. M11_GameView_Start in src/engine/m11_game_view.c calls dm1_v2_presentation_mode_set_m12() / csb_v2_presentation_mode_set_m12() right after the spec is built. CTest + headless probe coverage: 50+36 unit + 30+27 probe assertions across both modules (test_dm1_v2_presentation_mode_pc34 / test_csb_v2_presentation_mode_pc34 / firestaff_dm1_v2_presentation_mode_probe / firestaff_csb_v2_presentation_mode_probe), all pass alongside the 23/23 Phase A invariants.

## Chaos Strikes Back (CSB)

### CSB V1

- 🔧 Phase 2 - Dungeon data model: synthetic CSB dungeon loader/model probe exists and loader/free-cycle safety is covered; remaining work is real CSB asset ingestion and runtime structure parity without DM1-only assumptions.
- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, CSB-only D3L2/D3R2 and D2L2/D2R2 draw-order/frame gates, F0107 back-wall ornament routing, and initial viewport gates exist; remaining work includes actual ornament blits, F0108 floor ornaments, F0115 creature/item/projectile pass, F0111 door panel, and `CustomBackgrounds`.
- ✅ Phase 6 - Utility/import flow: champion import from DM1 saves (256-byte CSB block format, DM1→CSB record conversion), import state machine (ReDMCSB SAVEGAME.C F0100-F0120), utility disk flow state machine (INIT→INSERT_DISK→VERIFY_DISK→DISK_OK→SELECT_ACTION→IMPORT/LOAD/NEW→DONE), headless probe passes 33/33 tests.
- ✅ Phase 7 - Verification suite: add deterministic boot, dungeon, combat, save/import, and rendering probes.
- 🐛 Runtime handoff: the M12 launch/profile intent is valid for hash-matched CSB assets; remaining work is title/intro/import path, CSB-specific viewport integration, and end-to-end playability verification.

### CSB V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock before V2 work.
- ✅ Phase 1 - V2 launch/profile separation.
- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API is wired (csb_v2_presentation_mode_set_m12, m12PresentationMode 0..3 → CSB_V2_PM_V1_FAITHFUL/V20_FILTERED/V21_UPSCALED/V22_MODERN). Remaining work: actual CSB modern asset pack (mirror of DM1's ~/.firestaff/assets/dm1/modern/ at ~/.firestaff/assets/csb/modern/) + CSB V2.1 upscale module (port of dm1_v2_texture_upscale) + CSB V2.2 modern shape module (port of dm1_v22_shapes for CSB's 9-square layout).
- 🔧 Phase 3 - Enhanced UI overlays: scaffolded (HUD compass/depth/gold/champion bars/action strip/chaos indicator, csb_v2_hud_overlay_pc34.h/.c, build+probe pass). Mode selection gate added in this pass (csb_v2_presentation_mode_is_v22() / is_v21() / is_v20() / is_v1()) so the HUD overlay can branch on the active mode.
- ❌ Phase 4 - Enhanced lighting and magic effects.
- 🔧 Phase 5 - Smooth movement and viewport interpolation: `csb_v2_smooth_movement.c` (50/50) + `csb_v2_runtime.c` binding seam (12 test groups, 43/43) on top of F0365/F0366/F0364; remaining work is wiring `csb_v2_runtime_bind_to_v1(profile)` into the actual CSB V1 game-loop entry point, CSB M11 game view bridge, and pixel/presentation gates.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ✅ Phase 2 - Dungeon/world data model: complete DM2 map, object, tile, and world-state ingestion.
- ✅ Phase 3 - Rendering pipeline: viewport, UI chrome, items, outdoor/indoor, palette behavior (pass).
- ❌ Phase 4 - Mechanics parity: movement, interactions, shops/NPCs, doors, pressure plates, triggers, combat, magic, and timeline.
- ❌ Phase 5 - Creature/combat parity: complete DM2 creature AI, projectile, damage, death/drop, and sound behavior.
- ✅ Phase 6 - Utility/import flow: implement DM2-specific load/start flow and compatibility behavior.

### DM2 V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock before V2 work.
- ✅ Phase 1 - V2 launch/profile separation: DM2_V2_PHASE_DOMAIN_LAUNCH and _PROFILE gates implemented; 42/42 probe pass; commit 22838e8f.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting and outdoor effects.
- 🔧 Phase 5 - Smooth movement and viewport interpolation: struct-based `dm2_v2_smooth_movement.c` (79/79) + `dm2_v2_runtime.c` binding seam (12 test groups, 43/43) on top of SKULL.ASM T520/T048/T560/T600; remaining work is wiring `dm2_v2_runtime_bind_to_v1(profile)` into the actual DM2 V1 game-loop entry point, DM2 M11 game view bridge (dungeon T560 + outdoor T600), and pixel/presentation gates.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Dungeon Master Nexus

### Nexus V1

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked; remaining work is launcher/game-loop handoff and real Saturn asset-path proof.
- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates.

### Nexus V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock: `nexus_v2_phase_gate_pc34.c` (include/nexus_v2_phase_gate_pc34.h) classifies 19 V1/V2 domains (11 V1-source-locked gameplay + 9 V2-presentation-eligible). V1-locked domains (DMDF_DGN_LOADING, SATURN_ISO_READER, GAME_STATE_INIT, CHAMPION_PARTY, CREATURE_AI, SPELL_MAGIC, MOVEMENT, SAVE_LOAD, SOUND_DRIVER, RASTERIZER, INVENTORY) always return v1SourceLocked=1 and v2PresentationAllowed=0 regardless of V2 toggles. V2-eligible domains (RENDER_PRESENTATION, SMOOTH_MOVEMENT_PRESENTATION, DYNAMIC_LIGHTING_PRESENTATION, HUD_OVERLAY, PARTICLE_EFFECTS, ATMOSPHERE, INPUT_PRESENTATION, CONFIG_PRESENTATION, UPSCALER) require v2PresentationEnabled=1; CONFIG_PRESENTATION additionally requires v2ConfigPersistenceEnabled=1. Source-locked against NEXUS.C / NEXUS2.C / NEXUS.BIN, nexus_v1_iso_reader.c, nexus_v1_dmdf_model.c, nexus_v1_dungeon.c, nexus_v1_engine.c, nexus_v1_game.c, nexus_v1_champions.c, nexus_v1_creatures.c, nexus_v1_movement.c, nexus_v1_combat.c, nexus_v1_magic.c, nexus_v1_inventory.c, nexus_v1_save_load.c, nexus_v1_sound.c, nexus_v1_rasterizer.c, ReDMCSB CLIKMENU.C:142/180 F0365/F0366, COMMAND.C:2045 F0380, MOVESENS.C:316-345 F0267, HuC6260/HuC6270 VDC/VCE, THQUEST.ASM T400-T900. Ctest `nexus_v2_phase_gate_pc34` 240/240, headless probe `firestaff_nexus_v2_phase0_v1_compatibility_lock_probe` 218/218.
- ✅ Phase 1 - V2 launch/profile separation: headless probe `firestaff_nexus_v2_phase1_launch_profile_separation_probe` 60/60. Verifies launch gate (GAME_STATE_INIT is V1-source-locked, always allowed; RENDER_PRESENTATION blocked when V2 off, allowed when V2 on), profile gate (CONFIG_PRESENTATION requires BOTH v2PresentationEnabled=1 AND v2ConfigPersistenceEnabled=1, with V2 off/persist on still blocked by V1 gate), Nexus V1 asset hash separation (0DMSTRT.BIN 8a026f1...d20b6, DM.BIN 3bbca12...ad124, LEV00.DGN 24e3b3c...83d9a, LEV15.DGN df8ccdf...ef0aa, ITEM.IBS fc32ca5...30c1, FACE.BIN d733f50...21e22, FILE_LISTING.txt 6526c88...9091b), cross-game hash separation (Nexus LEV00.DGN ≠ DM1 DUNGEON.DAT d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 ≠ CSB DUNGEON.DAT 6695d2acebce49f95db1d8f3a5c733de), V1-only default behaviour, headless-safety (no game data files loaded). Ctest `nexus_v2_phase1_launch_profile_separation` 1/1.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting and Saturn presentation effects.
- 🔧 Phase 5 - Smooth movement and viewport interpolation: smooth-movement runtime bridge (`nexus_v2_smooth_movement.c`) with walk/turn/stairs interpolations, tick-driven auto-detection of position/angle deltas, shortest-path turn normalization, and probe coverage (`firestaff_nexus_v2_smooth_movement_probe` 64/64 + `test_nexus_v2_smooth_movement` 27/27); remaining work is runtime binding into the V1 game loop, broader deterministic input coverage, and pixel/presentation gates.
- ✅ Phase 6 - Touch/controller ergonomics: V2-only touch swipe, edge-strafe, D-pad, and dual analog stick affordances for the Saturn gamepad mapped to `NEXUS_CMD_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT/STRAFE_LEFT/RIGHT` (1-6), with V1 mouse/touch/click parity preserved (affordances rejected when `v2PresentationEnabled=0`). Ctest target `test_nexus_v2_touch_controller_affordance` covers mapping/classification/routing/name/source-evidence invariants; headless probe `firestaff_nexus_v2_touch_controller_affordance_probe` passes 294/294 (API surface, movement command mapping, input kind classification, V2 presentation route, V1 parity guard, NONE handling, idempotency, Saturn-specific right-stick-turn-only, cross-game shape consistency with DM1/DM2/CSB sibling affordances, source evidence, route kind constants).
- ❌ Phase 7 - V2 verification suite.

## Theron's Quest

### Theron V1

- 🔧 Runtime handoff/playability proof: hash-verified Track 02 availability is wired through validator/startup/menu state and 14 Theron V1 probes/tests are green locally (`theron_v1_rendering`, `theron_v1_save_header_rejection`, `theron_v1_shop_price_table`, `theron_v1_world_serialize_purchase_state`, `theron_v1_direct_launch`, `theron_v1_m11_direct_launch`, `theron_v1_launcher_scan_reuse`, `theron_v1_track02_bank`, `theron_v1_viewport_renderer`, `theron_v1_tile_renderer`, `theron_v1_mechanics_hardening`, `theron_v1_teleporter_chain`, `theron_v1_dungeon_progression`, `theron_v1_save_load`); remaining work is positive real-asset launch with a JP/US Track 02 BIN/ISO pair.
- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds; remaining work is real-asset gameplay traces and broader cross-route runtime evidence.
- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, and shop price-table regressions are green; remaining work is cross-slot import/export against real Track 02 saves.
- 🔧 Phase 8 - Verification suite: launch, dungeon progression, mechanics, rendering, save/load, direct-launch, M11-handoff, launcher scan-reuse, and Track 02 bank probes/tests are all green locally (15/15); remaining work is a single end-to-end positive-launch evidence pass with a real asset.

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

### Build and CI Health

- 🔧 Linker "ignoring duplicate libraries" warnings: target-link-order artifact, not a code issue, but worth a one-line fix to silence the link-time noise.
- 🔧 Pre-existing ctest parity-evidence failures (7 failures / 440 passes in Release at last count): `dm1_v2_launch_smoke_pc34`, `v1_status_refresh_order_redmcsb_gate`, `dm1_v1_viewport_3d_source_lock`, `pass623_dm1_v1_input_capture_readiness_bridge`, `pass625_dm1_v1_original_transcript_row_preflight`, `pass626_dm1_v1_original_transcript_turn_redraw_route` are parity-evidence line-drift from prior watchdog passes (line numbers pinned to older code that has since been re-anchored); `nexus_v2_lighting` has a CMake `add_test` entry pointing at `test_nexus_v2_lighting` with no underlying binary. Each failure needs a one-line read of the actual report under `parity-evidence/verification/<name>/manifest.json` to confirm whether the gate needs the line re-pinned, the test source fixed, or the entry removed.
- 🔧 Watchdog parity-evidence manifests: parity-evidence files are refreshed by automated watchdog passes on every regression run. Manifests may report transient `FAIL` on gates whose line number has shifted (see the line-drift bullets above) or where a recent change has altered the test binary output; verify against the current source before treating any one FAIL as a real regression.
- 🔧 Lefthook pre-commit hook: the repo's pre-commit pipeline expects `lefthook` on `$PATH`; the hook currently no-ops gracefully on dev machines without it, but CI runners should install it or wire the equivalent CMake-level check so commit messages and staged-file lint both run on every push.

## Known Bugs

- 🐛 Viewport/collision reports without capture manifests must stay as bugs until paired original PC 3.4 evidence or a reproducible local probe exists.

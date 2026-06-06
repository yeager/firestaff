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
- ✅ Viewport D2C center wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D2C wall frame clips source X 16..71 into viewport X 60..115, preserves C10 transparency, and leaves pre-blit/adjacent/out-of-frame pixels untouched.
- ✅ Viewport D3C far center wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D3C wall frame clips source X 18..63 into viewport X 74..119, preserves C10 transparency, and leaves pre-blit/adjacent/out-of-frame pixels untouched without claiming full real-asset wall-set parity.
- ✅ Viewport D3L/D3R far side wall pixel gate: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB F0116/F0117 wall routes, G0163 frame clips, C10 transparency, clipped edge writes, and neighboring pixels for the ordinary D3 side walls without claiming full real-asset wall-set parity.
- ✅ Viewport D1C center wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D1C wall frame clips source X 48..127 into viewport X 32..111, preserves C10 transparency, and leaves clipped/pre-blit/out-of-frame pixels untouched for the only square draw with `wall_case_returns=false`.
- ✅ Viewport D1R right-edge wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D1R wall frame clips source X 0..63 into viewport X 160..223, preserves C10 transparency, and leaves pre-frame/right-edge pixels untouched.
- ✅ Viewport D0L narrow side-wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB D0L wall frame clips the resolved 16-wide source span to viewport X 0..15, preserves C10 transparency, and keeps the champion panel beneath the wall by leaving viewport column 16 untouched even though the frame's `right_x=31` nominally allows a 32-wide wall.
- ✅ Viewport D0L/D0R parity wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves party-side wall parity selects the opposite native wall bitmap, flips it through the ReDMCSB F0105 path, preserves C10 transparency, and keeps the neighboring viewport zone untouched.
- ✅ Viewport D2L2/D2R2 near-side wall pixel slice: the DM1 V1 viewport 3D source-lock test now proves the MEDIA720 D2L2/D2R2 routes use the ReDMCSB F0678/F0679 C707/C708 zones, preserve C10 transparency, clip native/parity side-wall strips at the viewport edges, and keep the no-F0115 thing-pass contract explicit.
- ✅ Viewport parity wall fix: PC34 D3L2/D3R2 parity side-wall draws now select the opposite native `G2107_WallSet` bitmap once, flip it through a ReDMCSB-style scratch path, and keep a pixel regression for the no-prewired-temp route.
- ✅ Viewport D3L2/D3R2 wall parity gate: the DM1 V1 viewport 3D source-lock test now proves the ReDMCSB F0676/F0677 far-side wall/parity routes, G0711/G0712 frame metadata, C10 transparency, clipped edge writes, wall-return behavior, and non-wall F0115/field paths.
- ✅ Viewport side-wall pixel clip gate: D2L/D2R/D1R synthetic wall pixels now prove ReDMCSB G0163 source offsets, transparent-color skips, and right-edge clipping; D1L proves the fully clipped no-write case.
- ✅ Viewport F0098 floor/ceiling fallback pixel slice: the DM1 V1 viewport 3D source-lock test now proves ReDMCSB F0098 row ownership for the current asset-free fallback by clearing the black-area and floor rows, preserving the intervening wall band, and resetting the floor/ceiling dirty flag without claiming full real-asset floor/ceiling bitmap parity.
- ✅ Viewport F0108 floor-ornament metadata slice: the DM1 V1 viewport 3D source-lock test now proves each covered floor-field square carries the ReDMCSB F0108 floor-ornament call/no-call source anchors, including BUG0_64 open-pit-overdraw ordering, while explicitly not claiming full real-asset floor-ornament bitmap parity.
- ✅ Viewport regression sweep: `v1_viewport_floor_ornament_stair_gate`, `v1_viewport_front_wall_depth_gate`, and `v1_viewport_pit_floor_ornament_bug64_gate` are green again after restoring the explicit wall-free/wall-like source-bound helpers.
- ✅ Wall inscription source-font regression: readable D1C wall inscriptions now use the dedicated PC34 `M648_GRAPHIC_INSCRIPTION_FONT`/GRAPHICS.DAT index 258 with 8-pixel source centering, guarded by `firestaff_m11_inscription_font_probe`.
- ✅ Door-front occlusion pixel-zone gate: all 11 source-locked front-door branches prove rear cells are masked by door pixels and front cells draw after the door pass.
- ✅ Hall champion mirror visibility gate: `dm1_v1_champion_mirror_visibility_runtime` opens the DM1 V1 runtime, draws the known Hall mirror route, and pixel-matches the D1C wall portraits against the source GRAPHICS.DAT champion portrait strip.
- ✅ Hall champion mirror Z-order slice: `dm1_v1_champion_mirror_zorder_runtime` opens the DM1 V1 runtime, pixel-proves north/south/east D1C champion mirror portraits, and checks west-facing Hall side poses do not leave a floating D1C portrait.
- ✅ Hall champion mirror candidate panel slice: `dm1_v1_champion_mirror_candidate_panel_runtime` opens the DM1 V1 runtime, pixel-proves the C040 resurrect/reincarnate panel is drawn on top of the front mirror cell only while a candidate is selected (100% opaque match on open, baseline leak on closed), and that resurrect/cancel close the panel and re-arm/disable the mirror route as the ReDMCSB REVIVE.C F0282 source dictates.
- ✅ Hall champion mirror candidate overlap slice: `dm1_v1_champion_mirror_candidate_panel_runtime` now also proves a pending candidate keeps the mirror route live until confirm and that the C040 panel wins over the lower D1C champion portrait area.
- ✅ Hall champion mirror candidate select-miss slice: `dm1_v1_champion_mirror_candidate_panel_runtime` now proves a west-facing side/no-front-mirror call to `M11_GameView_SelectFrontMirrorCandidate()` rejects without opening C040, appending a candidate, changing candidate identity, or enabling inventory, source-locked to ReDMCSB `REVIVE.C` and `DUNVIEW.C`.
- ✅ Hall champion mirror candidate reincarnate-click slice: `dm1_v1_champion_mirror_candidate_panel_runtime` now opens the C040 panel, clicks the source reincarnate box, and proves redraw, candidate/inventory clearing, champion retention, mirror-route disablement, and C040 pixel clearing against ReDMCSB `COMMAND.C` and `REVIVE.C`.
- ✅ Champion panel Firestaff-side pixel slice: `dm1_v1_champion_panel_pixels_runtime` opens the DM1 V1 runtime with real assets, renders a deterministic four-champion V1 HUD, and checks status-box fill/name zones, bar pixels, champion icons, and GRAPHICS.DAT-backed hand slot boxes without claiming original DOS parity.
- ✅ Champion panel status-state pixel slice: `dm1_v1_champion_panel_status_states_runtime` extends the champion-panel pixel gate with the secondary status-box states the existing probe intentionally does not cover — C008 dead status box (HP=0), C032 POISONED label (96×15 GRAPHICS.DAT blit), C037/C038/C039 shield border stack (fire+spell+party), C015 damage indicator (45×7), the 16×16 status hand icon inside the 18×18 slot box, and V1 name text glyph pixels for all four slots including the dead champion's name print through CHAMDRAW.C F0292:816-842.
- ✅ DM1 V1 original-capture route classifier hardening: `docs/parity/tools/dosbox_state_detector.py` now (a) divides per-pixel (not per-channel) so the 0.30 calibrated band matches the documented expectation, (b) drops the dead 0.70/0.10 envelope for a single calibrated 0.135 band, (c) ships a synthetic-fixture `--self-test` plus a corrected pass94 ground-truth table that reclassifies frames 05-06 as `dungeon_gameplay` (the runbook's "wall_closeup" label was itself produced by the same broken envelope), (d) fixes the `Image.LONEST` typo in `docs/parity/tools/compare_captures.py`, and (e) extracts the runbook's inline `dosbox_capture_session.py` into a real runnable scaffold at `docs/parity/tools/dosbox_capture_session.py` with `--dry-run` that walks the state machine against the synth fixtures.  The two new CTest gates (`dm1_v1_original_capture_state_detector_self_test`, `dm1_v1_original_capture_session_dry_run`) catch classifier regressions in CI so the next DOSBox live attempt is no longer blocked by a buggy classifier.
- ✅ DM1 V1 original-capture route preflight gate: `docs/parity/tools/dosbox_capture_preflight.py` runs the runbook §1 SHA256 checks (DUNGEON.DAT `d90b6b1c...`, GRAPHICS.DAT `2c3aa836...`) and writes a hardened `dosbox_capture.conf` whose settings are pinned to the runbook §2 values (machine=svga_s3, memsize=16, cpu core=dynamic, cycles=max, frameskip=0, windowresolution=1024x768, output=opengl).  The preflight refuses to write a conf that contains the historical pass94 failure-mode values (svga_paradise, memsize=4, core=normal, cycles=3000) and records a JSON receipt (`preflight.receipt.json`) with the verified SHA256s, the rendered settings, the chosen launch command (`DM.EXE`), and the current Firestaff git HEAD so the next capture-session manifest can cite the receipt.  CTest gate `dm1_v1_original_capture_preflight_self_test` exercises the matching case, the SHA-corruption case, the pass94 conf-pin-violation case, and the hardened-conf sanity case against hermetic synthetic fixtures (no real game data required).  This is the upstream gate for the runbook §2 "machine=svga_s3 is non-negotiable" requirement; the next live attempt that runs the preflight cannot reproduce the pass94 conf shape.
- ✅ Creature and combat systems: creature groups, AI, attacks, deaths, drops, XP, projectile attacks, sounds, fleeing, special positioning, possession drops, Black Flame behavior, generator/teleporter/fall/drop cases, and Lord Chaos constants.
- ✅ Spells and magic: rune UI, spell casting, mana/skill checks, projectiles, shields, light/dark, open-door magic, poison cloud behavior, and spell failure paths.
- ✅ Champions: recruitment, active selection, health/stamina/mana bars, skill/XP updates, death/resurrection, stats panel routing, weight/load behavior, and stamina regeneration.
- ✅ Inventory chest compact-close regression: sparse open-chest contents now stay covered by a source-locked test that proves compact close returns the full non-empty count, honors a smaller output buffer, clears the open-chest panel, hides chest slots, and drops chest load.
- ✅ DM1 V1 chest visible-slot close gate: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C` only rewrites the eight visible C537..C544 chest slots on close, detaching a ninth linked object that never entered `G0425_aT_ChestSlots`.
- ✅ DM1 V1 chest open-path chain intact + last-slot pickup tail detach: `test_m11_inventory_full_panel_runtime_pc34_compat` now also proves ReDMCSB `CHEST.C` `F0333` only reads `G0425_aT_ChestSlots` and leaves the 9th..12th tail items reachable through the 8th visible item's `next` pointer, and that `m11_process_v1_chest_slot_box_click` on `C544` rewrites the same `next` pointer so close then proves the tail is unreachable from `container.slot`.
- ✅ DM1 V1 open-chest source-pixel slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves real GRAPHICS.DAT rendering keeps the C033 slot-box border around populated C537, blits the exact source dagger icon subrect via `F0033_OBJECT_GetIconIndex`, and leaves empty C538 as a full C033 box with no object-icon overdraw.
- ✅ DM1 V1 open-chest second-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` advances from C537 to C538 through the linked object list and blits the second object's own source icon (torch C004) instead of reusing the first object's dagger C032.
- ✅ DM1 V1 open-chest third-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` advances through C537/C538/C539 and blits each linked object's own source icon: dagger C032, torch C004, and morningstar C046.
- ✅ DM1 V1 open-chest fourth-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` advances through C540 and blits the fourth linked object's own source icon (arrow C131) while preserving the earlier dagger/torch/morningstar slots.
- ✅ DM1 V1 open-chest fifth-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` continues linked-list traversal through C541 and blits the fifth linked object's own source icon (Slayer C052) while preserving the earlier C537-C540 dagger/torch/morningstar/arrow slots.
- ✅ DM1 V1 open-chest sixth-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` continues linked-list traversal through C542 and blits the sixth linked object's own source icon (Sling C053) while preserving the earlier C537-C541 dagger/torch/morningstar/arrow/Slayer slots.
- ✅ DM1 V1 open-chest seventh-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` continues linked-list traversal through C543 and blits the seventh linked object's own source icon (Rock C054) while preserving the earlier C537-C542 dagger/torch/morningstar/arrow/Slayer/Sling slots.
- ✅ DM1 V1 open-chest eighth-slot icon slice: `test_m11_inventory_full_panel_runtime_pc34_compat` now proves ReDMCSB `CHEST.C F0333` continues linked-list traversal through C544 and blits the eighth linked object's own source icon (Poison Dart C055) while preserving the earlier C537-C543 dagger/torch/morningstar/arrow/Slayer/Sling/Rock slots.
- ✅ Survival, sensors, entrance, save/load, audio, and data loading: food/water decay, rest, stamina, sensor/timeline behavior, title/entrance flow, save/load routes, sound routing, and DUNGEON.DAT/GRAPHICS.DAT ingestion.
- ✅ Source-lock verifier hardening: viewport/walls landable metadata, wall-clip source audit, side-wall source-row clipping, D3/D2 wall-ornament order, front-cell collision, D0/D1 visible-square draw-order, wall-alcove C2548, champion stat panel, and ambient dungeon sound gates now resolve current local code/source boundaries and reflect the closed no-ambient-loop source boundary.
- 🔒 DM1 source-lock audit completed across movement, rendering, creatures, combat, spells, champions, inventory, survival, sensors, entrance, save/load, audio, and data structures.

### DM1 V2.0 / V2.1 / V2.2

- ✅ V2.0 filtered presentation: config, CRT scanlines, palette correction, dither cleanup, sharpening, renderer integration, and launcher/menu integration.
- ✅ V2 parity/presentation scaffold: Phase 0 and Phase 1 command routing, deterministic config, profile boundary, and launch-smoke verification.
- ✅ V2.1 asset pipeline: Phase 2 source-preserving upscale/EPX pipeline, deterministic cache behavior, fallback handling, and probe coverage.
- ✅ V2.2 modern asset pipeline: generated/modern art path, provenance/fallback contracts, and `dm1_v22_asset_pipeline` probe coverage.
- ✅ V2 presentation slices: HUD/action route gate, palette/projectile metadata gates, smooth-movement runtime bridge, touch/controller route gate, and presentation-disabled state-hash gate.
- ✅ DM1 V2 side-by-side verification seed: the presentation-disabled state-hash test now runs paired V1/V2 lanes through the phase gate and hashes deterministic pixel-buffer scaffolding while preserving V1 command truth.
- ✅ DM1 V2 smooth turn pan backend: optional Custom/V2 turn-pan setting persists through config, the Phase 5 bridge can start pan-enabled turns, and the camera exposes a presentation-only viewport pan offset while V1 command direction changes remain source-owned.
- ✅ DM1 V2 Phase 4 field/projectile VFX binding gate: source explosion thing IDs map to V2 overlay/emitter families, fluxcage remains field-only, unknown things are rejected, and invalid source palette lighting falls back deterministically.
- ✅ DM1 V2 side-by-side presentation seed: `firestaff_dm1_v2_side_by_side_presentation_seed_probe` builds the DM1 PC 3.4 entry state fixture once, renders it through the V2 viewport renderer to a V1 lane and a V2 lane, asserts byte-identical 224x136 framebuffers (presentation-disabled parity gate), enforces zero mismatches through `dm1_v2_vp_compare_viewport_region` over the full viewport, and emits a stable 64-bit FNV-1a side-by-side seed (`V1=224 + gap=8 + V2=224`) for future visual diffs, while keeping the V1 movement command route pinned to V1 source.
- ✅ DM1 V2 side-by-side seed C001..C006 truth table: the same probe now exercises every V1 source command (ReDMCSB DEFS.H:238-243 C001 TURN_LEFT, C002 TURN_RIGHT, C003 MOVE_FORWARD, C004 MOVE_RIGHT, C005 MOVE_BACKWARD, C006 MOVE_LEFT) through `dm1_v2_movement_command_route_for_presentation(0, ...)` and asserts that each row reports `routeKind == V1_SOURCE`, `v2PresentationEnabled == 0`, `sourceCommand == runtimeCommand == C-id`, so no row can be silently re-routed by a future V2 presentation change. Stable hash: `sideBySideHash=cf0cbcce6f491525`, `v1Hash=v2Hash=ae4c479ad4c4a725`.

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
- ✅ CSB V1 F0108 floor blit parity gate: D3L2/D3R2 now source-lock F0108 ordinal/index handling, `G0191` bitmap increments, `G0195` coordinate-set selection, `C1500 + viewFloor` zones, D3R2 flip, and C10 transparent blit behavior for door-front, pit, and corridor floor ornament routes.
- ✅ CSB V1 F0111 door panel zone/clip gate: D3L2/D3R2 now source-lock the far-door panel base zones, COORD.C parent records, 48x41 native bitmap to 48x40 clip contract, open-door skip, state-zone shifts, C10 transparency, and F0115 rear/front ordering around F0111.
- ✅ CSB V1 F0115 projectile metadata gate: D3L2/D3R2 now source-lock the ReDMCSB F0115 projectile row/zone contract, including G2028 row selection, `C2900_ZONE_ + row * 4 + ViewCell`, thing-list restart, cell-match requirement, D3 front-cell suppression, and the door-front rear/front F0115 ordering evidence around F0111.
- ✅ CSB V1 F0115 creature metadata gate: D3L2/D3R2 now source-lock creature visibility rows, group-marker capture, missing-row rejection, `C3200_ZONE_ | MASK0x8000` zone/coordinate selection, and object-before-creature-before-projectile ordering against ReDMCSB `DUNVIEW.C`, `DEFS.H`, and `COORD.C`.
- ✅ CSB V1 F0115 item/explosion metadata gate: D3L2/D3R2 now source-lock item blit zones through `C2500_ZONE_ | MASK0x8000`, pile-shift advancement, C10 `F0791` dispatch, explosion restart rows through `G2034/G2035`, rebirth/centered/side `C3000/C3007/C3014/C3031` zones, and fluxcage field deferral.
- ✅ CSB V1 F0107 wall ornament blit gate: D3L2/D3R2 now source-lock ordinal-zero skip, ordinal-to-current-map index conversion, `C1004_ZONE_WALL_ORNAMENT + CoordinateSet*15 + ViewWall`, C30/C14 scaled-bitmap selection, D3R2 horizontal flip, C10 `F0791` transparent blit behavior, and synthetic indexed-pixel copy/skip proof.
- ✅ Phase 7 - Verification suite: deterministic boot, dungeon, combat, save/import, and rendering probes pass.
- ✅ Boot → runtime handoff in one step: `csb_v1_boot_enter_game()` now actually loads the verified DUNGEON.DAT into `runtime.dungeon_handle`, sets `runtime.dungeon_asset.kind`, copies `entrance_map_index` / `start_map_index` from the boot profile, and is regression-tested by `test_csb_v1_boot_runtime_handoff` (30/30 checks, source-locked against ReDMCSB ENTRANCE.C F0806 / LOADSAVE.C F0435 and CSBWin/CSBCode.cpp:6800-6950).

### CSB V2.0 / V2.1 / V2.2

- ✅ Phase 0 - V1 compatibility lock before V2 work.
- ✅ Phase 1 - V2 launch/profile separation.
- ✅ Phase 7 deterministic verification gate: `test_csb_v2_phase7_verification` is green again after restoring CSB's accumulated V1-tick phase reporting while keeping render-frame deltas for smooth movement.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, asset discovery, launcher state, and runtime selection.
- ✅ Real-asset probe regression sweep: `probe_dm2_v1_asset_loader`, `probe_dm2_v1_dungeon_loader`, `probe_dm2_v1_object_model`, `probe_dm2_v1_world_state`, and `test_dm2_v1_save_load` pass against the local hash-verified PC English data.
- ✅ DM2 V1 save/load header hardening: direct slot loads now enforce the 0xBEEF/0xDEAD DM2 slot magic used by scans, reject stale/cross-game slot files, and recover from `SKSave.bak` when the primary slot is corrupt.
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
- ✅ Direct hash-verified Theron Track 02 file scans now populate the matched version/required-file status without building normal search roots or running root-wide hash lookups; `test_asset_status_scan_metrics` covers the no-rescan path with a synthetic known hash.
- ✅ Phase 6 - Dungeon progression probe coverage.
- ✅ Phase 7 - Save/import compatibility.
- ✅ Phase 8 - Verification-suite coverage for deterministic launch, dungeon progression, mechanics, rendering, and save/load paths.
- ✅ Theron V1 direct-launch (hash-verified path) slice: `theron_v1_boot_load_verified_path()` builds a usable boot profile from an already-hash-verified Track 02 path with zero `stat()` probes (asserted via `theron_v1_boot_rescan_call_count()`). Regression: `test_theron_v1_direct_launch` covers all four locked-in JP/US BIN/ISO MD5s, refuses unknown hashes, and is wired into `ctest -R theron`. Composes with the M12 catalog-level `m12_try_match_direct_theron_request()` direct-path matcher.
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.

## Cross-Cutting

### Launcher and Settings

- ✅ CSB V1 runtime handoff gate: `test_csb_v1_boot_profile_smoke` now verifies a verified boot profile transfers CSB variant, asset paths, archive kind, save-root override, start position, title/entrance state, difficulty, and Chaos Magic initialization into `CSB_V1_RuntimeProfile`; the focused local test is green.
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

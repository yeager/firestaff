# Firestaff DONE - CSB

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Chaos Strikes Back (CSB)

- ✅ 2026-07-30 CSB V1/V2.x window-scale ownership: an explicit
  `--scale-mode` survives startup-menu configuration application, while game
  presentation dimensions remain off-screen surface dimensions rather than
  resizing the user window. The source C407 Prison pointer therefore maps
  through the active FIT rectangle at 320x200 and 960x600 in V1, V2.0, V2.1
  and V2.2. Verification: real-PC3.4 entrance-pointer/title/HUD/V2.2 runtime
  regressions pass.

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
- ✅ CSB V1 DSA trigger actuator-target guard: `test_csb_v1_dsa_trigger_single_step_pc34_compat` now covers `csb_v1_chaos_trigger` out-of-range `script_id` rejection (negative, == `script_count`, `CSB_V1_MAX_DSA_SCRIPTS`, `+1`, and saturated `INT_MAX` all return -1 without mutating the known script), null state pointer rejection, and a sanity pass that a valid `script_id` still triggers and mutates flags after the rejection path. CTest registered and PASS 82/82 assertions, 0 failures.
- ✅ 2026-06-22 CSB V1 boot→first-viewport-frame gate CTest: registered existing data-free `test_csb_v1_boot_viewport_render_gate` as CTest target `csb_v1_boot_viewport_render_gate`. The 47/47 source-locked fixture proves the boot→runtime→`csb_v1_viewport_render_frame()` boundary is deterministic: the handoff exposes the live dungeon handle, current level, party seed, and asset paths that M11's `fs_game_render_viewport()` reads (ReDMCSB ENTRANCE.C F0806 lines 409-441, LOADSAVE.C F0435 lines 1940-1944, DUNGEON.C F0151 lines 1423-1475, DUNVIEW.C F0128 lines 8318-8542, CSBWin/CSBCode.cpp:6800-6950); the single-frame render is byte-identical for two CSB viewport configs fed the same handoff state; the render is contained to the source-locked 224×136 viewport region (ReDMCSB VIEWPORT.C M091_BITPLANE_SIZE) so the M11 chrome/HUD is not clobbered; the NULL-viewport guard keeps the M11 stage-integration path safe; the column-major *thing* data (bits 5..14 of the 16-bit square record) survives the handoff so depth ordering reads back the expected `csb_v1_dungeon_get_first_thing` index; the M11 reset path (init→enter→cleanup→enter) is bit-stable, so alt+F4 / new-game flows keep the source-locked CSB_V1_START_PARTY_* seed and CSB_V1_DIFFICULTY_HARD. Verification: focused build PASS and `ctest --test-dir build -R csb_v1_boot_viewport_render_gate --output-on-failure` PASS 1/1. This is regression/contract coverage only; it does not load real CSB game data, run DOSBox, capture pixels, write manifests, or claim CSB runtime/original parity.
- ✅ 2026-06-28 CSB V1 PC 3.4 quickplay/startup dungeon-handle probe + rescan-clearing fix: new `probes/csb/firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe.c` registered as CTest `csb_v1_pc34_quickplay_dungeon_handle` (labels `tier1;csb;quickplay;boot_handoff;dungeon_handle;rescan;skip_safe`, TIMEOUT 60). The probe pins the verified DUNGEON.DAT handle handoff boundary (H1-H4: runtime owns a non-NULL handle after `csb_v1_boot_enter_game()` that exposes at least one readable level through `csb_v1_dungeon_get_square_type()`, `csb_v1_dungeon_get_current()` equals the runtime-owned handle, current level is map 0, profile+runtime reach RUNTIME_READY + CSB_STATE_TITLE) and the rescan-clearing boundary (H5-H7: a failed `csb_v1_boot_scan_assets()` clears `profile->runtime.dungeon_handle`, the global singleton, all verification flags, paths, md5s, and variant_id, blocks `csb_v1_boot_enter_game()` until a fresh verification cycle, and `csb_v1_boot_cleanup()` is idempotent). The probe also exercises the re-launch boundary (H8: a successful rescan back into the verified dir releases the previous handle before `enter_game()` re-establishes a fresh one). PC-real-asset path runs end-to-end when `FIRESTAFF_CSB_PC_DATA` (or argv[1] / `~/.firestaff/data/csb`) carries the canonical PC 3.4 EN pair (`GRAPHICS.DAT 61fbfd56887c94adc26888a9491c6611` + `DUNGEON.DAT 6695d2acebce49f95db1d8f3a5c733de`); synthetic-fixture path (1 level, 2x2, legacy 16-bit square record format accepted by `csb_v1_dungeon_load()`) drives the same production code paths on hosts without user-supplied CSB data so CI stays deterministic. **Bug found and fixed:** the previous `csb_v1_boot_scan_assets()` only cleared profile metadata on rescan (asset flags + paths + md5s + variant_id), NOT the runtime-owned `profile->runtime.dungeon_handle` or the `csb_v1_dungeon_get_current()` singleton — so a rescan that lost the CSB assets could leave the runtime still pointing at the previous heap-allocated dungeon, and a subsequent `csb_v1_boot_enter_game()` that failed verification would silently keep serving the previous dungeon through the cleared profile paths. The fix in `src/csb/csb_v1_boot.c` calls `csb_v1_dungeon_unload()` (which frees `raw_data`/`dsa_offsets` and clears `s_current_dungeon` + `s_current_level`) and `free(profile->runtime.dungeon_handle)` when either the runtime handle or the singleton is alive, so the rescan-driven path now releases the previous dungeon before the rescan-driven profile fields are populated. Same release contract as `csb_v1_boot_cleanup()`, but scoped to the dungeon-only boundary so the next `enter_game()` still owns the full runtime re-init. Source-locked against ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance), LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon load + map 0), DUNGEON.C F0237 (dungeon load entry), and DUNGEON.C F0173/F0174 lines 2724-2755 (dungeon release path). Local verification: `cmake --build build --target firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe --parallel` clean, direct `./build/firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe` PASS 37/37 checks (path=real-assets), re-run with `FIRESTAFF_CSB_PC_DATA=/tmp/firestaff-empty-csb-data` PASS 23/23 checks (path=synthetic-fixture), `git stash` of the `csb_v1_boot.c` fix + rerun shows the probe correctly catches the bug (4 FAIL on H5b/H5c/H8c/H8d), focused CTest `csb_v1_pc34_quickplay_dungeon_handle` PASS 1/1, broader CSB regression set (`ctest -L csb`) PASS 0 failures across 70+ tests including `test_csb_v1_boot_profile_smoke` (51/51), `firestaff_csb_v1_pc_real_asset_launch_probe` (20/20), and `csb_v1_required_complete_launches`, full project `cmake --build build --parallel` clean exit 0, `git diff --check` clean. Honest scope: data-free and real-asset regression/contract coverage only — no full CSB V1 playability claim, no game data committed, no README claim.
- ✅ 2026-06-29 CSB V1 PC real-asset ornament capture provenance hardening: extended `probes/csb/firestaff_csb_v1_pc_real_asset_ornament_blit_probe.c` so the existing skip-safe real-asset D1C ornament gate writes a JSON sidecar at `/tmp/csb_pc_real_ornament_capture_manifest.json` beside the deterministic PPM and SHA256 files. The manifest records the verified PC 3.4 GRAPHICS.DAT MD5, resolved asset path, selected bitmap index/inline dimensions/payload span, 320x200 framebuffer geometry, D1C floor-band capture rows, ReDMCSB F0108/F0115/DEFS.H source anchors, tally counts, and explicit non-claims. The probe now also asserts the selected bitmap payload span stays inside GRAPHICS.DAT and that the manifest MD5 is the canonical PC CSB graphics hash. Verification: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`, focused build of `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` PASS (edited probe clean; build surfaced a pre-existing unrelated DM1 self-comparison warning in `src/dm1/dm1_v1_bitmap_arrow_pointer_pc34_compat.c`), direct probe PASS 30/30 on the local real PC 3.4 CSB pair with capture sha256 `5e489ae14354d791e12a9474bbb44027eaac1be8e1021491d9d88dcef8ba9de1`, and focused CTest `csb_v1_pc_real_asset_ornament_blit` PASS 1/1. Honest scope: provenance/capture tooling only; no game data committed, no original DOS pixel parity claim, and broader viewport/HUD captures plus M11 CSBgraphics.dat override binding remain open.
- ✅ 2026-06-26 CSB V1 CSBgraphics.dat index classifier + skip-safe real-asset scan probe: new bounded CSBWin custom-graphics override classifier (`csb_v1_csbgraphics_dat_classify` in include/csb_v1_csbgraphics_dat_classify.h + src/csb/csb_v1_csbgraphics_dat_classify.c) reads the on-disk count + parallel compressed/decompressed size tables and the optional `0x8001` little-endian sentinel that CSBWin `OpenCSBgraphicsFile` (`CSBWin/Graphics.cpp:1838`) and `ReadGraphicsIndex` (`CSBWin/Graphics.cpp:1918`) emit. It reports byte_order, count, total_compressed, total_decompressed, payload_offset, payload_bytes_avail, max_compressed, and max_decompressed without performing any LZW decompression or payload decode. Bounds: rejects NULL args, files < 4 bytes, empty count, count > 8192, payload_offset > size (truncated tables / header), and total_compressed > payload_bytes_avail (overflow). Source-locked against CSBWin/Graphics.cpp:1643 `LocateNthGraphic` (payload offset = `NumGraphic * 4 + 2` / `+ 4` when the LE marker is present), CSBWin/Graphics.cpp:1717 `ReadGraphic` (cluster-cache loop), CSBWin/data.cpp:1936 `Signature` (file MD5 split as two uint32 words), ReDMCSB F0200_DMMISC_ReadCompressedGraphic + GRAPH21.C F0914, plus greatstone d_items.html "Graphics.dat file format" + dmweb Data Files page. CTest `csb_v1_csbgraphics_dat_classify_unit` (data-free, synthetic fixtures) PASS 11/11: argument-rejected, too-small-rejected, empty-count-rejected, oversized-count-rejected, big-endian round-trip, 0x8001 little-endian marker round-trip, total-compressed overflow rejection, LE-marker-too-small rejection, truncated-tables rejection, max-tracking, and source-evidence/result-name string checks. Companion `csb_v1_csbgraphics_dat_real_scan` (include/csb_v1_csbgraphics_dat_real_scan.h + src/csb/csb_v1_csbgraphics_dat_real_scan.c) mirrors the HCSB.HTC real-scan pattern: known MD5 list + `asset_find_by_md5_list` discovery + virtual-container materialization via `asset_extract_virtual_path` + parse + cached buffer/resolved-path/matched-md5/label ownership. The default known-hash list is intentionally empty (CSBgraphics.dat is a CSBGraffer / CSBWin Viewport Compiler product, not an original CSB asset; there is no canonical reference hash to gate discovery on until a user stages a real file under `~/.firestaff/data/csbwin-custom/<label>/`). The probe `firestaff_csb_v1_csbgraphics_dat_real_scan_probe` SKIPs cleanly when the list is empty, and CTest `csb_v1_csbgraphics_dat_real_scan` (labels `tier2;csb;csbwin_custom_resource;real_data;skip_safe`, TIMEOUT 120) PASS. Locally verified end-to-end by staging a synthetic CSBgraphics.dat under `/tmp/csbgraphics_test_dir/csbwin-custom/synthetic/` with a temporary hash registration in `g_known_hashes[]`: probe runs the full 22/22 real-data checks (cache loaded + buffer ownership + matched MD5/label + index invariants + second-scan byte-identical determinism), then the synthetic hash is removed so the committed known-hash list stays empty. Both files compile clean under strict `-std=c99 -Wall -Wextra -Werror -pedantic`; the empty known-hash table uses a trailing `{ "", "", 0u }` sentinel so the public build does not rely on the C23 empty-initializer extension. `git diff --check` clean. Disjoint from the existing CSB V1 Hint Oracle classifier + scan (`csb_hint_oracle_htc` + `csb_hint_oracle_htc_real_scan`, HCSB.HTC format), the CSB V1 save/load (`csb_v1_save_load_pc34_compat`, 512-byte obfuscated header), and the DM1 V1 custom dungeon importer (`custom_dungeon_m12` + `dm1_v1_custom_dungeon_loader`, DUNGEON.DAT header). This is regression/contract coverage plus a skip-safe readiness gate; it does not decode a CSBgraphics.dat payload, does not override a CSB runtime graphic, does not draw a CSBWin overlay, and does not bind into M11/M12. Remaining work for `docs/FIRESTAFF_GAP_LIST.md` row C3 / A3 (CSBWin custom resource handling) is the actual LZW/payload decoder, the M11 viewport override hook, the full CSBWin `dmsave`/`csbgame` import evidence, and a real user-staged CSBgraphics.dat with a verified MD5 to populate the default known-hash list.

### CSB V2.0 / V2.1 / V2.2

- ✅ 2026-07-30 CSB V2.0 deterministic source/runtime capture: the filter
  regression samples the first stable post-door F0128 frame at tick 4 instead
  of a later live-HUD redraw. It proves unchanged V1/V2.0 source bytes while
  requiring V2.0's presented filter output to differ. Verification: the
  complete real-PC3.4 CSB CTest lane passes 116/116.

- ✅ CSB V2 presentation-mode selection: `csb_v2_presentation_mode_pc34` module (include/csb_v2_presentation_mode_pc34.h, src/csb/csb_v2_presentation_mode_pc34.c) parallel to the DM1 V2 module. M12_PRESENTATION_* enum → CSB_V2_PM_V1_FAITHFUL/V20/V21/V22 with the same V22→V21 fallback. `csb_v2_presentation_mode_set_m12()` is called from M11_GameView_Start (gameId=csb). CSB and DM1 presentation-mode globals are independent (set DM1 V22 + CSB V1: DM1 stays V22, CSB stays V1). CTEST target `test_csb_v2_presentation_mode_pc34` passes 36/36, headless probe `firestaff_csb_v2_presentation_mode_probe` passes 27/27. Source-locked against ReDMCSB COMMAND.C F0359/F0361, CLIKMENU.C F0365/F0366, DUNGEON.C:35-44 CSB direction step tables, GAMELOOP.C:150-155 V1 tick cadence, ENTRANCE.C CSB prison door, CSBWin/Viewport.cpp:7290, CSBWin/Chaos.cpp:60-69 DSA dispatch, CSBWin/Graphics.cpp:3186.
- ✅ CSB V2.2 modern-asset pipeline (2026-06-19): new `csb_v22_modern_assets_pc34.c/.h` (include + src) mirrors `dm1_v2_modern_assets_pc34` with CSB-specific paths (`~/.firestaff/assets/csb/modern/`) and CSB source-locks (ReDMCSB LIGHT.C F0212 / DUNVIEW.C F0128 CSB 9-square viewport / PANEL.C F0354 / COMMAND.C:108-113,254-291 + CSBWin/Viewport.cpp:7290 / CSBWin/Chaos.cpp:60-69 DSA dispatch). API: `csb_v22_set_manifest_path(dataDir)` (walks up 2 levels → assets/csb/modern/), `csb_v22_validate_manifest(path)` (returns 1=complete, 0=partial, -1=invalid), `csb_v22_modern_assets_available()` (1 when wall_shapes+floor_shapes+creature_shapes all present), `csb_v22_set_installed/get_installed`, `csb_v22_set_epx_cache_warm/get_epx_cache_warm`, `csb_v22_best_available_shape_source(mode)` (fallback chain V2.2→V2.1 cold/warm→V2.0), `csb_v22_get_missing_placeholder` (16×16 magenta), `csb_v22_get_shape_path(category, id, ...)` (manifest lookup), `csb_v22_shape_source_name(src)`, `csb_v22_source_evidence()`. CSB_V22_ShapeSource enum: V1_ORIGINAL/V2_FILTERED/V2_UPSCALED/V2_MODERN (parallel to DM1_V22_ShapeSource). Ctest `test_csb_v22_modern_assets_pc34` passes 33/33 (path resolution, manifest validation missing/empty/partial, installed/epx flag round-trips, full fallback chain with state transitions, name strings, placeholder, source evidence citation). Wired into `CMakeLists.txt` (test binary + ctest target). CMakeLists: `csb_v22_modern_assets_pc34.c` listed in CSB V2 lib sources.
- ✅ CSB V2.2 first-cut asset pack (2026-06-19): `.openclaw/tmp/csb_v22_asset_author.py` procedural generator (deterministic seed 0xC5B1) installs 5 PNGs + manifest v1.0.0 to `~/.firestaff/assets/csb/modern/`. Assets: `wall_shapes/wall_dungeon_01.png` (1024×1024, CSB chaos-purple accent on dungeon stone), `floor_shapes/floor_prison_01.png` (1024×1024, prison iron-grey tile), `creature_shapes/creature_chaos_fiend_01.png` (1024×1536, chaos fiend silhouette with Lord Order gold eyes), `ui_chrome/panel_lord_order_01.png` (1024×1024, gold panel frame), `champion_portraits/champion_warrior_csb_01.png` (1024×1536, iron-grey armor with Lord Order gold accent). Manifest v1.0.0 with top-level category keys for strict validator. Smoke: `csb_v22_set_manifest_path("/Users/bosse/.firestaff/data/csb")` resolves correctly, `csb_v22_modern_assets_available()=1` end-to-end for real CSB data dir. `m11_v22_set_installed(csb_v22_modern_assets_available())` flips CSB V2.2 installed flag. CSB-specific palette accents: CHAOS_PURPLE (CSB chaos magic), IRON_GREY (prison iron), LORD_GOLD (Lord Order endboss). Pattern follows DM1 v1.1.0 procedural first cut; real PBR hero art (openai/gpt-image-2) for CSB is a follow-up batch.
- ✅ Phase 5 smooth-movement runtime bridge: `csb_v2_smooth_movement.c` provides visual walk (ease-out cubic), turn (ease-out quad), and stairs (ease-in-out cubic + vertical camera offset) interpolations over 1 V1 tick (55ms). Global state is driven via a `V2_AnimClock*` to `csb_v2_smooth_update_from_clock`. Headless probe `firestaff_csb_v2_smooth_movement_probe` covers lifecycle, walk N/S/E/W, turn 8 directions, stairs with vertical offset, and deterministic input coverage; ctest target `test_csb_v2_smooth_movement` passes 50/50. Plus binding seam: `csb_v2_runtime.c` (CSB_V1_RuntimeProfile, `bind_to_v1`/`is_bound`/`force_sync`/`v1_tick`/`render_frame`) auto-triggers walk/turn/stairs on V1 deltas (F0365/F0366/F0364). Integration test `test_csb_v2_smooth_runtime_binding` 12 groups/43 asserts pass; ctest 2/2 (CSB smooth + CSB runtime binding).
- ✅ CSB V2.2 modern shape book (9-square): `csb_v22_shapes.c` (include/csb_v22_shapes.h) supplies `csb_v22_shape_for_cell`, `csb_v22_shape_for_view_square`, `csb_v22_wall_shape_get`, `csb_v22_floor_shape_get`, `csb_v22_material_get`, `csb_v22_material_count`, `csb_v22_shape_for_prison_door`, and the 9-square bridge. Source-locked against CSBWin/Viewport.cpp:7290 and ReDMCSB M034_SQUARE_TYPE. Fix in `csb_v22_floor_shape_get`: stairs direction was reading `flags & 0x01` but `flags` was already masked to 0xF0 so the lower bit was always zero, making both `0x10` and `0x11` resolve to STAIRS_UP. Changed to `base & 0x01` so `0x10` → up (dir=0) and `0x11` → down (dir=1). ctest target `test_csb_v22_shapes_pc34` passes 54/54 (1 pre-fix FAIL was `stairs down`). Headless probe `firestaff_csb_v22_shapes_probe` passes 21/21. Disjoint from CSB V2.0/V2.1/V2.2 selection API and from the DM1 V2.2 shape book (the two are mirrors, not duplicates, because CSB has the prison door and the CSB-specific stair material book).
- ✅ CSB V2.1 texture upscale test+probe (9-square + panel + V22 EPX): `test_csb_v2_texture_upscale_pc34` and `firestaff_csb_v2_texture_upscale_probe` had pre-fix test bugs that made `t_epx_2x`, `t_9square_viewport`, `t_panel`, and `t_present_mode_v22_triggers_epx` (and the matching probe checks) fail when the source pattern starts with 0. The `epx_buf[0] != 0` check was wrong because EPX writes the source pixel P to the corresponding output, so for `src[0] = 0` the EPX rule writes 0 to `epx_buf[0]` and the check always failed. Fix: `t_9square_viewport` and `t_panel` now `memset(epx_buf, 0xCC, ...)` before calling and check `epx_buf[0] != 0xCC`. `t_epx_2x` keeps the `{10,20,10,20}` input but documents that all four EPX neighbour predicates miss on this pattern (so output is the P-fallback column-stripe), with new expectations `dst[0]==10 && dst[1]==10 / dst[2]==20 && dst[3]==20 / ...`. Probe mirrors the same sentinel approach. Local verification: `ctest -R csb_v2|csb_v22|dm1_v2_shape_runtime` 12/12, `firestaff_csb_v2_texture_upscale_probe` 13/13, `test_csb_v2_texture_upscale_pc34` 30/30.
- ✅ CSB V2.0/V2.1/V2.2 launch-res floor in `M12_StartupMenu_GetLaunchIntent` (commit `44280458`): V2.0/V2.1/V2.2 with stored `M12_RES_320x200` is promoted to `M12_RES_640x400` at launch time because the upscaled/modern render path requires `>=640x400`. The floor lives ONLY in `GetLaunchIntent` (not in `m12_enforce_mode_constraints`) so the row cycle stays full (INV_M12_18: 320x200 -> 640x400 -> 1280x960 -> 1920x1080 -> 2560x1440). V1 original is NOT subject to the floor (locked to 320x200). Test `test_csb_v2_resolution_selector_gate_m12` and `test_dm1_v2_launch_smoke_pc34` updated to assert the 640x400 launch-res for V2.x with stored 320x200; ctest green, m12_startup_menu probe 55/55 invariants including INV_M12_17B + INV_M12_18.
- ✅ CSB V2.0/V2.1/V2.2 settings persistence in M12 menu config: extended `M12_Config` + `M12_MenuSettingsState` with `csbV2ScalePercent` / `csbV2BilinearEnabled` / `csbV2CrtScanlinesEnabled` / `csbV2CrtScanlineStrength` / `csbV2PaletteCorrectionEnabled` / `csbV2DitherCleanupEnabled` (mirrors the existing `dm1V2*` pattern, defaults 200% scale, 0 bilinear, 0 scanlines, 35 strength, 0 palette, 0 dither). Round-tripped through `M12_Config_SetDefaults` + the text Load + the text Save + the JSON Export + the JSON Import. New bridge module `csb_v2_settings_pc34` (include/csb_v2_settings_pc34.h, src/csb/csb_v2_settings_pc34.c) mirrors `dm1_v2_settings_pc34`: `CSB_V2_Settings` struct, `csb_v2_settings_from_m12_config` / `csb_v2_settings_apply_to_m12_config` / `csb_v2_settings_apply_to_runtime` (pushes scale + bilinear into `csb_v2_upscale_init` + filter toggles into `csb_v2_filter_config_apply`). ctest target `test_csb_v2_settings_pc34` passes 23/23, headless probe `firestaff_csb_v2_settings_probe` passes 13/13. **Wire-up done:** `M11_GameView_OpenSelectedMenuEntry` reads `menuState->settings.csbV2*` and calls `csb_v2_settings_apply_to_runtime()` right before `M11_GameView_Start`, so the user's saved CSB V2 settings reach the live runtime. New `csb_v2_upscale_get_scale()` + `csb_v2_upscale_get_bilinear()` accessors let the wire-up probe verify the live runtime. Headless probe `firestaff_m12_v2_settings_wire_up_probe` passes 16/16 (CSB scale 100/200/400 → runtime 1/2/4, bilinear 0/1 round-trip, invalid values clamp, independent of Theron). **Filter config wired:** new `csb_v2_filter_config_pc34` module (include/csb_v2_filter_config_pc34.h, src/csb/csb_v2_filter_config_pc34.c) stores the per-frame filter toggles (crtScanlinesEnabled, crtScanlineStrength, paletteCorrectionEnabled, ditherCleanupEnabled) as a module-level global; `csb_v2_settings_apply_to_runtime` now also calls `csb_v2_filter_config_apply()` so the M11 launch wire-up pushes both upscale AND filter config in a single call. ctest target `test_csb_v2_filter_config_pc34` passes 26/26, headless probe `firestaff_csb_v2_filter_config_probe` passes 21/21 (apply + get + sanitize + clamp + wireup + independent of upscale). **Per-frame filter chain dispatch done:** new `csb_v2_filter_chain_apply_indexed(fb, w, h)` + `csb_v2_filter_chain_apply_rgba(rgba, w, h)` helpers consult the live filter config and apply the enabled filters (dither + palette interp for indexed, CRT scanlines for RGBA). `fs_game_render_v2` (src/engine/firestaff_game_loop.c) now calls them between V1 render and indexed-to-RGBA conversion (indexed) + after the conversion (RGBA), gated on `state->config.game == FS_GAME_CSB`. ctest target `test_csb_v2_filter_chain_pc34` passes 16/16 (V1 default 0 filters, dither only, palette only, both on = 2, RGBA all off, scanlines on, null/zero safety, 2x2 too-small skipped, 3x3 minimum, 4x4 full, end-to-end wireup from M12 settings). Headless probe `firestaff_csb_v2_per_frame_filter_dispatch_probe` passes 10/10. Source-locked against include/dm1_v2_settings_pc34.h, include/csb_v2_texture_upscale_pc34.h, include/csb_v22_shapes.h, include/csb_v2_presentation_mode_pc34.h, include/csb_v2_filters.h, include/config_m12.h. CSB V2.0 filter chain is now end-to-end: M12 menu -> csb_v2_settings_apply_to_runtime -> csb_v2_filter_config_apply -> csb_v2_filter_chain_apply_{indexed,rgba} -> V1/RGBA framebuffer pixels.

- ✅ CSB V2 Phase 4/5/6 combined runtime wire-up (2026-06-17): three new modules — `csb_v2_lighting_runtime.c/h` (V1→V2 lighting bridge wrapping `csb_v2_light_tick` + `csb_v2_light_update_flicker`), `csb_v2_smooth_movement_runtime.c/h` (V1→V2 smooth bridge wrapping `csb_v2_smooth_init` + walk/turn/stairs triggers), `csb_v2_touch_runtime.c/h` (V1→V2 input bridge routing through `csb_v2_touch_controller_affordance_route(v2_enabled, aff)` then translating `DM1_V2_MovementCommand` → `Dm1V1QueuedCommandPc34Compat`). All three use the unified CSB_V2_PhaseGateConfig gate (`v2PresentationEnabled` + `v2ConfigPersistenceEnabled`). Glob picks up all three new sources automatically. Source-locked against ReDMCSB LIGHT.C F0380 (lighting), GROUP.C:1695-1770 (smooth movement), COMMAND.C:108-113/254-291 + CLIKMENU.C:142/180 + GAMELOOP.C:164-219 (touch input), CSBWin/resurrect/CsbV2* (CSBWin reimpl), sibling dm2_v2_*_runtime.c + nexus_v2_*_runtime.c patterns. Combined headless probe `firestaff_csb_v2_runtime_wireup_probe` passes 60/60 (lighting init/shutdown + V2 off no-op + V2 enabled tick + partial-gate rejection + force_active bypass + V1 invariant + toggle cycles + post-shutdown rejection + source evidence; smooth init/shutdown + V2 off no-op + walk/turn/stairs triggers gated on V2 + partial-gate rejection + force_active bypass + V1 invariant + toggle cycles + post-shutdown rejection + source evidence; touch init/shutdown + V2 off no-op + V2 partial no-op + NONE rejection + 6 movement affordances translate to correct DM1_V1_COMMAND_* + count monotonic + coordinate pass-through incl. negative + null out rejection + force_active bypass + V1 invariant + toggle cycles + post-shutdown rejection + source evidence).

## 2026-08-09 CSB viewport D0L/D0R + F0109 + F0110 (commit 7be579a7d)

- ✅ D0L/D0R side wall rendering (F0125/F0126): view squares 9/10, depth 0,
  32x136 source frames, C10 transparency. Uses existing wallset enums.
- ✅ F0109 door ornament rendering: bridges CSB viewport to DM1
  `dm1_v1_door_ornament_info_for_ordinal_pc34()`. D1 native 48x88, D2/D3
  scaled with palette remap (G0200/G0201). G0207 coordinate set table.
- ✅ F0110 door button rendering: bridges to DM1 door button frame and
  palette remap. G0208 coordinate sets for D3R/D3C/D2C/D1C positions.
  D1C native with clickable box C05, D2/D3 scaled.

## 2026-08-09 CSB ABI fixes (commit 0158bdd54)

- ✅ Movement tick counters G0310/G0311 corrected from int to uint16_t.
- ✅ Thing handle sentinel corrected to 0xFFFF (DOS 16-bit ABI).

## 2026-07-28 CSB Entrance palette

- Corrected CSB's real Entrance palette from DM `C07/G8148` to the distinct
  `C28_ENTRANCE_CSB/G8174` VGA row specified by ReDMCSB `ENTRANCE.C` and
  `VIDEODRV.C`. This removes green edge artifacts from the real C004/C002/C003
  page. The real-data startup, pointer, and terminal-handoff tests pass.

## 2026-07-28 CSB native quicksave package binding

- The M11 CSB quicksave/resume regression now recreates the same
  hash-verified package identity before it asks the standalone runtime loader
  to validate a native v12 save. This preserves the deliberate package fence
  and proves party pose, clock and thrown-projectile state round-trip.

## 2026-07-28 CSB saved skin override verification

- Reauthenticated the CSBWin saved-EXPOOL skin fixture with its FNV receipt.
  The phase-7 runtime test now proves a verified saved `EDT_Skins` column
  overrides dungeon skin bytes without allowing altered tail data.

## 2026-07-28 CSB macOS startup smoke

- Ran the local original `GRAPHICS.DAT`/`DUNGEON.DAT` package in the macOS
  application window. PRESENTS, CHAOS and the C004 Entrance page presented
  coherent source graphics with their CSB palette routes; the source-locked
  Entrance pointer integration and the V1, V2.1 and V2.2 runtime handoffs
  also passed. This is a startup smoke, not a replacement for the remaining
  CSBWin DSA-bearing original-save corpus.

## 2026-07-28 CSB title/save and bonus-dungeon regression repair

- Repaired the CSB boot-to-runtime regression suite to assert the actual
  ReDMCSB `TITLE.C F0437` cadence: PRESENTS 60 VBlanks, CHAOS shrink 20,
  CHAOS hold 20, and STRIKES BACK 2. The save fixture now models the source
  timeline invariant before exporting. Bonus-dungeon fixtures also prove the
  current fail-closed package rule: an unregistered synthetic `DUNGEONB.DAT`
  cannot replace the authenticated dungeon owner. The complete focused
  handoff suite passes.

## 2026-07-28 CSB front-wall sensor input

- Corrected the direct M11 fixture to use the same current-dungeon ownership
  established by the CSB boot handoff. It now verifies the live `F0276`
  front-wall route: ornament click, queued sensor event, and remote door
  mutation all reach the runtime without bypassing the source ownership gate.
- ✅ 2026-07-28 CSB V2.x full regression verification. Built the actual V2
  test executables and ran the registered V2.0/V2.1/V2.2 HUD-overlay,
  runtime, modern-asset admission, in-place-draw, and per-cell-routing tests:
  5/5 pass. This validates the implemented presentation routes only; real
  CSB art review and packaged-app capture remain open in TODO.md.
- ✅ 2026-07-28 CSB Utility Disk direct-archive admission. Fixed the shared
  hash finder so a user-selected archive file is searched as a container,
  rather than silently being treated as a directory. The change covers both
  single-MD5 and MD5-list callers and is regression-tested with a directly
  selected ZIP. The real local Amiga CSB `Software.7z` now admits its verified
  `HCSBF.HTC` payload directly: the Hint Oracle panel decodes 219 hints,
  5,036 locations, 512 pages, and its real French first page with 36/36 probe
  checks. This is package admission, not a claim that the full Utility Disk
  menu or original-save corpus is complete.
- ✅ 2026-07-28 CSB V2.2 real-material admission. The live CSB presentation
  selector now requires both an installed V2.2 manifest and the existing
  `FINISHED_REAL` material gate before it activates modern rendering. A
  partial or provenance-free pack falls back to V2.1 instead of painting
  placeholder cells over original game data. Explicit test overrides remain
  available solely for the isolated V2.2 route tests. The presentation-mode
  regression passes 37/37, including the no-reviewed-material fallback.
- ✅ 2026-07-28 CSB V2.x complete registered-suite verification. Built the
  exact probe and unit-test targets instead of invoking the repository-wide
  all-target build, then ran every registered CSB V2.0/V2.1/V2.2 CTest:
  37/37 pass. Coverage includes V1 compatibility, filters, upscale,
  settings, resolution, smooth movement, lighting, VFX, touch/controller,
  HUD, presentation selection, V2.2 material routing, and source locks.
  This verifies the implemented V2.x code paths; the separate reviewed-art
  requirement for live V2.2 remains explicitly open in TODO.md.
- ✅ 2026-07-28 CSB V1 real-data title runtime smoke. Direct M11 boot probes
  against the locally hash-matched PC CSB `GRAPHICS.DAT` (MD5
  `61fbfd56887c94adc26888a9491c6611`) reached the source-owned title phases
  at frames 0, 60, 79, and 101 of the 102-frame sequence. The receipts report
  `csb-title-1`, `csb-title-2`, `csb-title-21`, and `csb-title-22`, with an
  active startup route and no runtime ticks before entrance handoff. This is
  live original-data route evidence; visual pixel comparison remains open.
- ✅ 2026-07-28 CSB V1 real-data startup handoff verification. The local
  original-data suite passes 4/4: terminal handoff, full startup sequence,
  presentation receipt, and F0128 Entrance-to-M11 binding. This exercises the
  actual CSB asset session and entrance consumer rather than only synthetic
  startup fixtures. Pixel-for-pixel visual capture remains separately open.
- ✅ 2026-07-28 Audio backend default. SDL3/CoreAudio playback is now enabled
  by default, so shipped games no longer require
  `FIRESTAFF_AUDIO_ENABLE_SDL=1` just to emit decoded source audio. Setting it
  to `0` remains the explicit headless opt-out. The default policy was
  verified by the new `m11_audio_default_sdl_dummy` regression plus the
  source-locked DM1 PSG and CSB SWSH transports; failed source validation
  still stays silent rather than substituting generated audio.
- ✅ 2026-07-28 CSB new-game dungeon-header position. Removed the fixed
  `(5,5)` CSB start pose. The PC34 loader retains
  `DUNGEON_HEADER.InitialPartyLocation` and both CSB boot paths decode it as
  ReDMCSB `LOADSAVE.C` F0435 does. The local original package boot-probes at
  map 0, `(9,0)`, direction 2; loader and boot/viewport regressions pass.
- ✅ 2026-07-28 CSB V2 production runtime binding. M11 now initializes,
  binds, ticks, renders, and shuts down the CSB V2 smooth runtime only for
  admitted enhanced CSB modes. The handoff preserves a V2.2-to-V2.1
  material-gate fallback. Verified with the 43-check smooth-runtime suite,
  518-check M12/M11 CSB handoff suite, and a direct original-data V2 boot.
- ✅ 2026-07-28 CSB V2 selected-menu and final-RGBA verification. The
  production M12-to-M11 test now opens V2.0 and V2.1 with the local
  hash-verified PC34 package, proves their V2 runtime binding survives a
  presented frame, and proves the title is still source-owned. V2.2 is
  explicitly rejected by M12 until its existing `FINISHED_REAL` material gate
  passes. CSB V2.0's optional CRT scanline pass now runs on the final M11
  RGBA surface after indexed/palette presentation, leaving the source indexed
  frame and palette receipt untouched. Verification: M11 launcher handoff
  533/533, filter-chain 16/16, and filter-config 26/26 pass.
- ✅ 2026-07-28 CSB F0115 armour and junk composition audit. Removed the
  stale TODO claim that armour and non-food junk were blocked: the active
  M11 CSB item path already resolves every armour subtype (58) and junk
  subtype (53) through the source-locked native `G0237 -> G0209` mapping,
  loads only the matching `GRAPHICS.DAT` entry, and fails closed otherwise.
  The local real-asset composition test passes.
- ✅ 2026-07-28 CSBWin DSA tracing resume handoff. Both verified CSBWin
  resume entry points now invoke the existing EXPOOL `DSAINDEX::ReadTracing`
  parser after staging the authenticated body and before publishing the
  runtime profile. The new runtime regression builds a real-shape `0x05070000`
  EXPOOL record and proves all eight tracing words plus the enabled-bit count
  become available through `csb_v1_runtime_get_csbwin_dsa_tracing()`.
- ✅ 2026-07-28 CSB V2.x current-tree verification. Built every registered
  CSB V2.0/V2.1/V2.2 target and ran the exact CTest selection: 41/41 pass.
  The verified code paths cover V1 compatibility, presentation selection,
  resolution/settings, filters including final-RGBA CRT, HUD and smooth
  runtime binding, touch/controller, lighting, V2.2 shape/per-cell/in-place
  rendering, and the reviewed-material admission gate. This does not promote
  unreviewed art or replace the separately tracked packaged-app capture.
- ✅ 2026-07-28 CSB Atari ST `ANIMATE.SCR` parser. Added a strict read-only
  parser for the documented big-endian instruction stream: all 30 commands,
  fixed parameter widths, bounded output, unknown-opcode/truncation rejection,
  and required Stop termination. The unit test also parses the local original
  1,802-byte `ANIMATE.SCR` from the staged Atari hard-disk archive.
- ✅ 2026-07-28 CSB Atari ST DMCSB1 container correction. The Atari loader
  now reads the format's two complete size tables rather than incorrectly
  treating them as compressed/decompressed pairs, caches authoritative item
  offsets, and copies raw items without sending them through LZW. A new
  unit target covers both compressed and raw items and accepts the real
  87-item `ANIMATE.DAT` layout from the original utility disk.
- ✅ 2026-07-28 CSB Atari ST title-asset consumer. `ANIMATE.SCR` now proves
  every original load command maps to its documented P4B1/IMG1/SND1 item
  family, and Firestaff converts the original P4B1 palette plus a full-screen
  IMG1 title item into a 320x200 RGBA frame. The test uses the staged Atari
  utility-disk files and rejects unknown item families rather than substituting
  a PC34 title asset.

- ✅ 2026-07-28 CSB Atari ST animation-script execution trace. Firestaff now
  executes the original 288-instruction `ANIMATE.SCR` control flow against the
  original `ANIMATE.DAT` slots, including its real loops, 101 palette fades,
  waits, 17 expands, 18 blits, two presentation calls and two sounds. The
  semantic trace is driven solely by the staged Atari utility-disk files; the
  remaining task is to connect its framebuffer operations to the CSB host
  presenter.
- ✅ 2026-07-28 CSB Atari ST script-owned final frame. The animation consumer
  now resolves the final active screen and palette from the executed original
  script state and rasterizes the resulting IMG1/P4B1 pair. The staged script
  selects item 75 with palette 21; no caller-provided or PC34 substitute asset
  is permitted on this route.
- ✅ 2026-07-28 CSB V2.x fresh build verification. Rebuilt the CSB V2
  library and every registered CSB V2 test executable, then ran the complete
  `^(csb_v2_|csb_v22_)` CTest selection: 41/41 pass. This covers V2.0
  presentation/filter/settings/smooth-runtime routes, V2.1 texture upscale,
  and V2.2 shapes, in-place drawing, per-cell material routing and the
  reviewed-art gate.
- ✅ 2026-07-28 CSB Atari ST animation-pair discovery. `ANIMATE.SCR` and
  `ANIMATE.DAT` are now found by their verified original hashes in a loose
  package or supported archive. The resolver requires both files to come from
  the same directory or container and returns virtual archive paths for the
  shared extractor rather than guessing by filename or mixing releases.
- ✅ 2026-07-28 CSB Atari ST archive materialization. A discovered archive
  pair now extracts into a dedicated cache directory and both resulting files
  are revalidated against their original hashes before use. Loose files take
  the same hash gate without copying. The direct and stored-ZIP routes pass
  against the staged original files.
- ✅ 2026-07-28 CSB Atari ST launcher render route. One public entry point
  now discovers the verified pair under a data root, materializes archive
  entries if needed, and renders the script-selected final frame. It passes
  against both the loose staged package and a stored-ZIP package.
- ✅ 2026-07-28 CSB Atari ST presented-frame route. The animation trace now
  records the original screen and active palette at every `Set-screen` VBlank
  and can rasterize each such source-selected frame. The local script proves
  two presentation calls: IMG1 36/P4B1 7, then IMG1 35/P4B1 2.
- ✅ 2026-07-28 CSB Atari ST launcher presentation route. The public
  data-root interface now rasterizes an individual source `Set-screen` frame
  as well as the final screen, including discovery, archive materialization
  and active-palette selection in one call.
- ✅ 2026-07-28 CSB full labelled regression. After building the one shared
  explosion-runtime executable missing from the local build tree, the complete
  CTest `csb` label selection passed 64/64. This includes V1 startup, save,
  DSA, viewport, HUD and Atari tests together with the V2.x suite.
- ✅ 2026-07-28 CSB Atari ST indexed host handoff. The source presentation
  route now emits the original 320x200 four-bit image indices plus its decoded
  P4B1 palette, rather than requiring a lossy RGBA-to-index conversion before
  M11 can present it.
- ✅ 2026-07-28 CSB Atari ST root-indexed host route. The launcher-facing
  data-root path now discovers, materializes and returns an original indexed
  `Set-screen` image and P4B1 palette in one call, leaving M11 no local file
  or RGBA conversion step.
- ✅ 2026-07-28 CSB Atari ST M11 startup presentation. Atari ST 2.0/2.1
  startup now selects the verified original `ANIMATE.SCR`/`ANIMATE.DAT` pair,
  applies its source P4B1 palette to M11, and presents the original indexed
  first `Set-screen` page. PC34 profiles retain their existing title route.
- ✅ 2026-07-28 CSB Atari ST presentation timing. The script trace now records
  the source VBlank of every real `Set-screen`; M11 advances only between
  those original presentation pages according to the live startup tick.
- ✅ 2026-07-28 CSB Atari ST framebuffer player. The verified original script
  now replays IMG1 expansion, empty image allocation, display coordinates and
  transparent 48x34 blits into its active 320x200 indexed screen through a
  requested source VBlank. The public data-root route preserves that indexed
  framebuffer and P4B1 palette without a PC34 substitute.
- ✅ 2026-07-28 CSB V2.x complete regression. Re-ran the full registered
  `^(csb_v2_|csb_v22_)` selection after the Atari work: 41/41 tests pass,
  covering V2.0 filtering/settings, V2.1 upscale, and V2.2 HUD, shapes,
  in-place drawing, per-cell routing and material gates.
- ✅ 2026-07-28 CSB Atari ST M11 framebuffer clock. M11 now converts the
  source-owned 55 ms startup ticks into 50 Hz Atari VBlanks, caches the last
  source framebuffer, and consumes the real replay route only for ST 2.0/2.1
  title startup. The original-data test proves a visible compositor change
  between the two source `Set-screen` pages.
- ✅ 2026-07-28 CSB Atari ST palette fade timing. ReDMCSB `ANIM.C` and
  `PALETTE.C` show that the Atari animation waits `delay + 1` VBlanks in
  `F0436_STARTEND_FadeToPalette` before committing the target P4B1 palette.
  The trace and framebuffer replay now follow that order instead of switching
  palettes immediately.
- ✅ 2026-07-28 CSB Atari ST direct startup. The verified Atari ST 2.x
  hard-disk package no longer attempts to open the incompatible PC34 TITLE.C
  session. M11 now reports and presents the real `ANIMATE.SCR`/`ANIMATE.DAT`
  startup at 50 Hz, and the boot probe confirms the active `animate-scr`
  route from the original `GRAPHICS.DAT` hash. `ANIM.C`'s VBlank before each
  bitmap blit is also reflected in trace and framebuffer timing.
- ✅ 2026-07-28 Direct archive game selection. The batched hash finder now
  treats a selected archive itself as a search root, matching the established
  single-hash path. Asset-status scanning retains an explicit `.7z`, `.zip`,
  or `.iso` root instead of replacing it with its parent directory. The real
  CSB Atari `Software.7z` now materializes verified `GRAPHICS.DAT` and
  `DUNGEON.DAT` into the runtime cache and boot-probes the original
  `animate-scr` route; it no longer launches an unrelated PC34 installation.
- ✅ 2026-07-28 CSB Atari ST `ANIM.C` compositor correction. The live player
  now uses ReDMCSB's item boxes as real source clips and destination origins,
  preserves the original transparent-colour and item-attribute fields, and
  implements opcodes 19-28 rather than treating them as synthetic loop state.
  The staged original animation and all CSB V2.x checks pass; an Atari boot
  probe reaches the active `animate-scr` route at source VBlank 495.
- ✅ 2026-07-28 CSB Atari ST FTLCODE handoff. M11 now records the final
  VBlank from the verified `ANIMATE.SCR` trace and crosses to the existing
  hash-verified CSB runtime only at that source boundary, matching `ANIM.C`'s
  `Pexec("A:\\FTLCODE")` ownership without entering the incompatible PC34
  TITLE.C/ENTRANCE.C route. A real original-package 1,000-frame boot probe
  reaches runtime with `DUNGEON.DAT` loaded at map 0, `(9,0)`, direction 2.
- ✅ 2026-07-28 CSB Atari ST animation audio. The verified ANIMATE.SCR route
  now carries both original opcode-12 SND1 calls through their source VBlank
  and Timer-A period into M11. `SOUND.C F0060` packed amplitudes and F0061's
  three PSG loud tables are decoded to the SDL transport with source hashes;
  malformed, altered, or invalid-period data is rejected without a fallback.
  The staged original proves SND1 item 86 followed by item 85 at period 112.

- ✅ 2026-07-28 CSB V2 cardinal turn-pan host consumption. M11 now consumes
  CSB V2's active cardinal turn animation after source viewport composition,
  using the shared 8.8 subpixel pan sweep. The V1 turn command remains
  source-owned and discrete. Targeted V2 runtime, source-lock and turn-pan
  tests pass for right, left, completion and non-cardinal no-op paths.

- ✅ 2026-07-28 CSB Hint Oracle test-root isolation. The real-data variant
  probe no longer recursively walks a user's game library during normal
  CTest execution. Real HCSB.HTC coverage is explicit through
  `FIRESTAFF_CSB_HTC_DATA` or a path argument; catalog verification remains
  automatic and fast.

- ✅ 2026-07-28 CSB probe fixture isolation. Hint Oracle real-data probes and
  the first-viewport probe now use their deterministic no-data paths during
  normal CTest execution. Explicit CSB data paths still exercise the original
  archive and PC material routes, while the ordinary regression gate no
  longer stalls on a user's installed game corpus.

- ✅ 2026-07-28 CSB C001 title-palette correction. The shared CSB
  PRESENTS/CHAOS/STRIKES special palette now uses the source-required
  dark-blue base `(0,0,109)` rather than the stale dim-blue `(0,0,73)`;
  the canonical PC3.4 real-data launch gate now accepts the C001 PRESENTS
  palette phase. Pixel decode and title/Entrance composition remain open.

- ✅ 2026-07-28 CSB V2.x verification sweep. All 41 registered CSB
  V2.0/V2.1/V2.2 CTest contracts pass, covering presentation modes, HUD,
  input, lighting, filters, settings, resolution selection and V2.2 artpack
  routing. This verifies implemented V2.x behavior; it does not close the
  remaining original-data V1 startup-composition work.

- ✅ 2026-07-28 CSB PC34 complete startup-chain gate. The canonical original
  GRAPHICS.DAT/DUNGEON.DAT now passes one chronological real-data proof:
  source Swoosh, C001 PRESENTS/CHAOS/STRIKES palettes, all 31 C004/C002/C003
  door pages, C017/C040 panel handoff, terminal F0807 package ownership and
  the decoded DUNGEON.DAT initial pose `(9,0)` facing `2`. The runtime and
  its HUD party state now receive that same source pose. Verification:
  `csb_v1_pc_real_asset_launch`, the full 93-test `ctest -L csb` regression,
  and the 41-test V2.0/V2.1/V2.2 subset.

- ✅ 2026-07-28 CSB app boot across presentation modes. The built `firestaff`
  executable reaches the original PC34 runtime through its M11 path in V1,
  V2.1 and V2.2: 260 source frames plus the Prison input reach map `0`,
  DUNGEON.DAT pose `(9,0,2)`, and a live runtime tick in each mode. This is
  a direct app-level boot check; visual packaged-window capture remains open.
- ✅ 2026-07-29 CSB V2.0/V2.1 source-HUD consumption. V2.0 and V2.1 now keep
  ReDMCSB PANEL.C's terminal PC3.4 C017 inventory and C040 transparent
  candidate surfaces instead of painting the former procedural V2 HUD over
  them. The real-package Prison-to-F0128 regression opens inventory, compares
  C017 at its original `(0,33)` geometry, and proves C040 composes at `(80,85)`
  with source index-6 transparency in V1, V2.0, and V2.1. V2.2 remains
  fail-closed until a reviewed material pack owns its replacement chrome.

- ✅ 2026-07-29 CSB V2.2 installation gate. The live F10 graphics selector
  no longer treats a nonempty `.fsart` configuration path as installed CSB
  art. It exposes V2.2 only after the CSB-specific complete manifest/cache
  gate succeeds, preventing a DM1 or arbitrary archive from selecting a
  false CSB modern presentation. The runtime graphics-popup regression covers
  the cross-game archive case.

- ✅ 2026-07-29 CSB V2.2 complete-material gate. A short manifest containing
  only real declared assets no longer satisfies `FINISHED_REAL`; all eight
  required CSB viewport, UI, entrance, and chaos-rune slots must be real and
  present. The finished-art gate regression covers the one-real-slot case.

- ✅ 2026-07-29 CSB V2.2 per-game selector gate. The runtime F10 selector now
  asks the CSB finished-material gate directly instead of trusting M12's
  cross-game `v22_modern_assets_installed` bit. A completed pack for another
  game therefore cannot show a CSB V2.2 option that would resolve back to
  V2.1 at runtime.

- ✅ 2026-07-29 CSB V2.2 presentation isolation. CSB mode resolution now
  checks `csb_v22_modern_assets_available()` together with its finished-art
  gate rather than the DM1/M11 global asset API, preventing one game's art
  installation from affecting another game's renderer.

- ✅ 2026-07-29 CSB archive swoosh-media materialization. Archive-backed CSB
  launches now copy all PC3.4 `SWSHSND.C`-accepted filenames, including
  `SWSHSND.DAT`, into the ordinary runtime cache beside `GRAPHICS.DAT`. The
  existing source loader still enforces the exact 9,078-byte raw sample
  contract; no generated sound is introduced. The CSB ZIP/ISO cache test
  verifies the `SWSHSND.DAT` copy path.
- ✅ 2026-07-29 CSB V2.2 per-game F10 admission. The live graphics popup no
  longer requires M12's cross-game artpack-installed flag after the complete
  CSB provenance gate succeeds. A CSB-only complete category manifest now
  reaches V2.2 from V2.1 with that global flag explicitly clear; the focused
  runtime-popup CTest locks the route. Incomplete or synthetic CSB packs
  remain hidden/fail-closed.
- ✅ 2026-07-29 CSB Artpack Studio manifest handoff. The native CSB V2.2
  manifest parser is now regression-tested with the pretty-printed category
  JSON emitted by Artpack Studio. Its required wall, floor, creature, UI and
  portrait entries validate and make the critical CSB categories available.
  This validates the pack-format boundary only; source-derived full-route
  material remains required before V2.2 is admitted for gameplay.
- ✅ 2026-07-29 CSBWin DSA production `STKOP_Message` binding. The runtime
  candidate now allocates a `TT_ParameterMessage` from the restored fixed
  timer pool, preserves the independent `parameterMessageSequence`, writes
  the original `(EDT_MessageParameters << 24)|timerID` DB11 record through an
  existing exact-size free node, restores the CSBWin heap ordering, and only
  publishes the timer/EXPOOL mutation after the whole DSA action validates.
  Zero through 29 parameters are supported; zero keeps CSBWin's two-word DB11
  record instead of becoming a delete. The regression locks source timer
  fields and actual EXPOOL bytes and proves a later invalid opcode rolls back
  both. `ctest -L csb` passes 96/96.
# CSB V2.2 viewport material consumption (2026-07-29)

- ✅ 2026-07-29 CSB V2.2 M11 renderer ownership: the live CSB path now
  initializes and consumes `csb_v22_inplace_draw`, updates the CSB 3x3 shape
  cache from its own sampled cells, and paints through the CSB route gate.
  M11 no longer invokes the DM1 V2.2 cache for a CSB session. The previously
  stale shared 1920x1080 placeholder rectangles were replaced by the active
  PC34 source-frame coordinates, so the cache and both CSB V2.2 consumers
  agree before final presentation scaling. Cache-open failure resolves to
  V2.1 rather than drawing foreign art. V2.2 gate, in-place, swap and
  real-data Entrance probes pass, as does a live PC3.4 V2.2 boot to map 0.

- ✅ 2026-07-29 CSBWin DSA direct `CAST` / `FILTEREDCAST` transaction:
  authenticated DSA execution now admits both source opcodes only as
  runtime-owned operations, keeps Magic.cpp's exact signed 14-word
  `SPELL_PARAMETERS` layout, and invokes its owner only after all later
  source words have succeeded. Incomplete payloads, missing owners and a
  later invalid word fail closed without a cast. The focused DSA regression
  and the full 98-test CSB CTest lane pass. Full Magic.cpp effects and
  indirect routes remain explicitly open in `CSB-DSA-FULL-OPCODE-FAMILY`.

- 2026-07-29 CSB V2.2 manifest lookup now selects the exact asset object,
  including pretty-printed manifests with separate provenance metadata. The
  old outer-object walker classified only the first entry of a 29-slot pack.
  The focused finished-art gate regression passes, and a locally generated
  PC3.4 source pack reports 29/29 real records.
- 2026-07-29 CSB V2.2 source-pack launch: M12 no longer has the stale
  DM1/Nexus-only V2.2 policy. It now permits CSB only when the same
  CSB-specific `FINISHED_REAL` material gate is true. A local PC3.4 pack
  generated by `scripts/build_csb_v22_source_fsart.py` starts V2.2 through
  the original Enter/Entrance handoff, reaching map 0 at `(9,0,2)` with a
  live runtime tick (`presentationMode=3`, `640x400`).

- The active CSB V2.2 viewport swap now routes every painted cell through
  `csb_v22_inplace_route_cell()` and therefore consumes the same depth-specific
  `(category, asset_id)` pairs as the finished-art admission gate. The former
  swap renderer used its stale generic mapping (`wall_dungeon_01`, etc.), which
  could not consume a completed 29-slot CSB package. It now retains raw cell
  values with the 3x3 swap cache and resolves the canonical route at render
  time. `firestaff_csb_v22_viewport_swap_probe` is now registered in CTest;
  its bounded cache uses canonical ids and proves all nine cells paint across
  four directions. Focused V2.2 regression: 4/4 pass.
- 2026-07-29 CSB V2.x Prison pointer regression: boot-probe now presents the
  active V2 surface before mapping a click. Real PC3.4 data proves G0445
  Prison entry reaches runtime in V1, V2.0 and V2.1 at 320x200 and 960x600.
- ✅ 2026-07-29 CSBWin DSA bounded `DELMON`/`INSMON` C04 ownership:
  authenticated direct opcode words now stage the exact source pop order and
  publish only after the complete action succeeds. The runtime uses loaded
  C04 group records, source horizontal-size limits and active-group state;
  it preserves no-group/no-room no-ops, refuses final-creature deletion, and
  never substitutes a synthetic monster. 2026-07-29 follow-up completes the
  coupled `Monster.cpp` timer/ITEM16 path: `DELMON` removes the selected
  A/B TIMER, renumbers later timer functions and compacts both saved and live
  ITEM16 status; `INSMON` duplicates A0/B0 and status 0 for the appended
  creature. The action commits C04, TIMER, M10 timeline receipts and ITEM16
  only after the candidate heap revalidates. Focused runtime coverage uses a
  fear group with real C04 and A0/A1 ownership, plus stack rollback coverage.
- ✅ 2026-07-29 CSB DSA monster-group receipt: the public DSA runtime receipt
  now binds each accepted `DELMON`/`INSMON` to its exact source LOCATIONREL,
  operand, C04 Thing and post-mutation C04 FNV-1a. Receipt retrieval fails
  after raw C04 drift, so callers cannot treat a stale stack acceptance as a
  live CSBWin world mutation.
- ✅ 2026-07-29 CSBWin DSA direct `STKOP_Move` PC3.4 cell owner: the
  authenticated interpreter now stages CSBWin `MoveObject.cpp`'s exact
  ten-word operand sequence and invokes it only after the complete DSA action
  has been accepted. The loaded candidate dungeon owns direct cell-to-cell
  DB5..DB13 selection through source type/position/depth masks, relinks the
  real Thing chain and writes the selected destination position atomically.
  The core receipt marks the opcode as a runtime-owned dungeon mutation. The
  focused regression proves operand order, late-word rollback and publication
  into a real byte-map PC3.4 list. Cursor, character, monster and chest
  endpoints remain explicitly open. Multi-position `STRandom` now consumes
  the staged GAMEBLOCK2 LCG state, and `I_MOVE` uses the identical
  parameter-backed source transaction.
- ✅ 2026-07-29 CSBWin DSA `STKOP_Del` / `STKOP_I_Del`: authenticated DSA
  now stages the original `(object, location)` request until its full program
  has been consumed. The real PC3.4 cell owner unlinks supported DB3/DB5/
  DB7/DB8/DB10 Things and returns the raw record to F0166 using the original
  free sentinel. Focused tests cover operand order, indirect expansion,
  rollback, core ownership and byte-map publication. Cursor/champion and
  chained-record locations remain explicitly fail-closed.
- ✅ 2026-07-29 CSBWin DSA `STKOP_Add` / `STKOP_I_Add`: authenticated DSA
  now stages CSBWin's original `(positionMask, LOCATIONREL, object)` operands
  and consumes the staged GAMEBLOCK2 `STRandom` state for a multi-bit position
  mask. The loaded PC3.4 candidate uses F0166 to allocate a same-type DB3/
  DB5/DB6/DB7/DB8/DB10 record, copies only source payload bytes 2..N, creates
  an `ENDOFLIST` node and appends the requested positional Thing atomically.
  Focused direct, indirect, rollback and raw-byte-map regressions pass;
  recursive DB4/DB9 copies and non-cell destinations stay fail-closed.
- ✅ 2026-07-29 CSBWin DSA `STKOP_Throw` / `STKOP_I_Throw`: the authenticated
  stack VM now preserves Timer.cpp's `(objectType, objectLocation,
  launchLocation, direction, range, damage, decayRate)` order until full
  acceptance. The runtime consumes the existing CSB F0810/timeline owner for
  original spell Things and ordinary selected cell objects, unlinks a physical
  source Thing only when projectile creation succeeds, and keeps unsupported
  source kinds as original no-ops. Direct and indirect operand regressions and
  the full CSB suite pass.
- ✅ 2026-07-29 CSBWin DSA `THROW` runtime publication: opcode 61 is now
  admitted by the same authenticated-core gate as its implemented executor.
  Accepted `ThrowMissile` work publishes the candidate F0810 projectile and
  its first move event atomically, while a rejected action leaves live state
  unchanged. The focused FIREBALL regression and `ctest -L csb` pass 98/98.
- ✅ 2026-07-29 CSBWin DSA indirect `THROW` source order: the runtime
  regression now uses CSBWin's reverse `INDIRECT` stack payload and proves
  `I_THROW` reaches the same F0810 FIREBALL/timeline owner as direct `THROW`.
- ✅ 2026-07-29 CSB executable boot sweep: local PC3.4 CSB data was exercised
  through the real Entrance command in V1, V2.0, V2.1 and selected V2.2.
  Each reached the map-0 runtime handoff; the incomplete V2.2 art gate
  correctly resolved to V2.1 rather than presenting unreviewed material.
## 2026-07-29 - CSB V2.1 EPX startup capture

- Fixed the CSB V2.1 post-presentation verifier to compare the actual
  Scale2x/EPX indexed source page before checking the source-owned special
  palette. The prior nearest-neighbour comparison rejected correct V2.1 C001-
  C005 frames after presentation.
- Added `csb_v21_presented_startup_capture`, an opt-in PC3.4 regression that
  requires real PRESENTS, CHAOS, STRIKES BACK, and Entrance captures.

## 2026-07-29 - CSB terminal F0128 capture

- ✅ 2026-07-29 Indexed runtime screenshot palette preservation. `M11_Screenshot_CaptureCurrent` now prefers the renderer's active source-owned RGB6 palette over the generic VGA fallback. This makes CSB/DM2 indexed runtime captures match their actual rendered palette; the standalone BMP regression proves RGB6 expansion and preserves the legacy fallback.

- Fixed the boot-probe terminal path to draw and present the post-Entrance
  F0128 dungeon frame before it writes optional indexed and host screenshots.
  A receipt for map 0 can no longer be accompanied by the stale Entrance page.
- Real PC3.4 V1/V2.0/V2.1/V2.2 pointer and completed V2.2-artpack runtime
  checks pass with the captured runtime frame.

## 2026-07-29 - CSB real-data first viewport gate

- ✅ 2026-07-29 CSB V2.x verification refresh: all 47 registered V2.0,
  V2.1 and V2.2 contracts pass in the external Ninja build. The lane covers
  presentation selection, filters, lighting, HUD, touch/controller routing,
  viewport/artpack admission and source-backed startup captures. It does not
  claim a finished V2.2 artpack when one has not passed its separate gate.

- The V1 first-viewport probe now searches ordinary hash-verified files before
  archive containers, matching the boot scanner's loose-install priority.
  This avoids opening unrelated large archives when a local PC3.4 pair exists.
- The real-data contract accepts the `DUNGEON.DAT` header's initial party pose
  through the same `LOADSAVE.C F0435` handoff used by runtime, rather than
  incorrectly demanding the missing-data fallback. The full CSB lane passes
  101/101 with local PC3.4 data.
- ✅ 2026-07-29 CSB original big-endian save-header admission: the CSBWin
  GAMEBLOCK1 classifier now validates both PC little-endian and original
  Atari-style big-endian word streams, records the proven order, and discovers
  all original `CSBGAME.DAT` through `CSBGAME4.DAT` slots. The external real
  `CSBGAME2.DAT` corpus validates with C29 and the source checksum invariant.
  Its existing original-Atari body decoder and Resume/runtime handoff verify
  and load the two-champion dungeon without a replacement file. CSBWin PC
  body import/export remains separately fail-closed. Verification: classifier
  unit 20/20, loader-boundary unit 158/158, external header probe 9/9,
  external decode/runtime probes, and full CSB CTest lane 103/103.
- ✅ 2026-07-29 CSBWin save-body ownership audit: the existing runtime already
  imports authenticated PC GAMEBLOCK2, ITEM16, CHARDESC, TIMER, timer queue
  and validated EXPOOL through `csb_v1_runtime_apply_csbwin_resume_file()`.
  The matching bounded exporter preserves source header/timer/EXPOOL evidence
  and rejects a changed heap rather than writing a plausible replacement.
  Existing runtime import/export and core round-trip regressions cover this
  path. The open save gap is original Atari/Amiga dungeon-data write-back,
  not the PC body importer.
- ✅ 2026-07-29 Original CSB GAMEBLOCK2 write-back: Firestaff now patches the
  documented big-endian clock, RNG, hand and party pose fields in an
  authenticated original save, re-encrypts Block2/Block3 and rebuilds the
  DMWeb checksum relationship in Block1. The real `CSBGAME2.DAT` corpus
  reopens with the changed state while its complete embedded dungeon payload
  remains byte-identical. Unowned character, ITEM16, timer and dungeon edits
  are deliberately preserved until their source layouts are fully owned.
- ✅ 2026-07-29 Original CSB runtime save export: a resumed original save can
  now be written through `csb_v1_runtime_write_original_atari_save_to_path()`.
  It derives only source-owned GAMEBLOCK2 state from the live profile, writes
  a temporary sibling before rename, and refuses an unverified source or
  unsupported profile. The real `CSBGAME2.DAT` runtime regression reopens the
  result, proves the new clock and proves the embedded dungeon is unchanged.
- ✅ 2026-07-29 CSB V2.2 source-pack runtime verification: the default
  `~/.firestaff/data/csb` path resolves through its external-volume symlink
  to `firestaff-csb-v22-pc34-source`, not the unrelated local AI/procedural
  first-cut manifest. Its 29 canonical material routes declare the
  hash-verified PC3.4 `GRAPHICS.DAT` origin with `syntheticContent: false`.
  Direct V2.2 boot remains mode 3 through Prison and paints nine V2.2
  viewport cells. The complete CSB test lane passed 103/103.
- ✅ 2026-07-29 CSB original-save champion write-back: the authenticated
  Atari/Amiga export now re-encrypts the source-owned portions of every
  800-byte champion record alongside GAMEBLOCK2. Names, titles, pose, action
  state, vital statistics, skills/experience, inventory references, load and
  shield state round-trip through the real `CSBGAME2.DAT` corpus; the source
  character checksum and outer save checksums are rebuilt. Unknown character
  bytes, ITEM16, timers, timer queue and embedded dungeon data remain
  untouched. The corpus test changes a champion name and health, reloads the
  saved result, and the CSB lane passes 103/103.
- ✅ 2026-07-29 CSB PC3.4 sound-table source binding: Firestaff now exposes
  ReDMCSB `DATA.C:1260-1302`'s exact I34E/I34M 35-row `SOUND_DATA` mapping,
  including source graphics 671-712, Timer-A periods, priorities and distance
  envelopes. This replaces the incorrect assumption that CSBWin's unrelated
  22-row `sound1772` table or GRAPHICS.DAT Graphic 562 could supply PC3.4
  audio. The table is deliberately only a route binding: the unproven PC audio
  payload format is still fail-closed. Verification: audio unit 53/53 and
  full `ctest -L csb` 103/103 passed.
- ✅ 2026-07-29 CSB PC3.4 original sound-payload admission: Firestaff now
  loads ReDMCSB `DATA.C:1260-1302` sound routes from their real `GRAPHICS.DAT`
  records, validates F0060's big-endian payload length, and exposes only the
  original bytes after that length word. The verified PC3.4 switch record
  (Graphic 672) admits 128 bytes from the local hash-pinned file. This is a
  source-data boundary, not an invented PCM conversion; reproducing each PC
  sound device remains tracked under `CSB-SOUND-MUSIC-MEDIA`. Verification:
  focused audio test 57/57 and `ctest -L csb` 103/103 passed.
- ✅ 2026-07-29 CSB V2.x real-data startup verification: the installed PC3.4
  package passed all three executable startup gates. V2.0 preserves the four
  original PRESENTS/CHAOS/STRIKES/Entrance palettes with its indexed filters;
  V2.1 preserves the same source sequence through EPX; and a complete V2.2
  pack preserves the source startup, enters map 0 after Prison input, paints
  source-mapped cells, and captures the terminal runtime frame. Verification:
  `csb_v20_filtered_startup_capture`, `csb_v21_presented_startup_capture`,
  and `csb_v22_source_artpack_runtime` passed with local original data.
- ✅ 2026-07-29 CSB PC3.4 runtime sound transport: CSB events now use their
  ReDMCSB `DATA.C`-selected `GRAPHICS.DAT` sample, rather than the DM1 SND3
  namespace or a procedural marker. `IO.C` F0060 framing is verified against
  local Graphic 672; the host transport preserves the signed source bytes and
  IBMIO F8119 PIT divisor (112/138/145/150). Changed data or invalid divisors
  fail closed. `csb_v1_pc34_runtime_audio_transport` and
  `csb_v1_audio_runtime_pc34_compat` pass.
# 2026-07-29 - CSB V2.2 F0128 corruption guard

- Removed the production use of the unverified 3x3 rectangular V2.2 painter.
  It corrupted real F0128 perspective frames with horizontal bands. V2.2 now
  keeps the source-owned viewport intact while retaining the selected modern
  material route for the forthcoming receipt-owned projection consumer.
- ✅ 2026-07-29 CSB V2.2 route-provenance reader: source artpacks now expose
  an explicit route record with category, asset id, original GRAPHICS.DAT
  index and native dimensions. The reader rejects absent records rather than
  inventing a projection. This is the metadata prerequisite for joining a
  V2.2 asset to ReDMCSB F0128's real clip/mask command; it does not itself
  enable modern viewport drawing. Focused metadata test passes.
- ✅ 2026-07-29 CSB V2.2 F0128 source-artpack projection status: generated
  source manifests now state whether a route is actually eligible for F0128
  projection. D1/D2 retain their ReDMCSB-bound statuses; the decodable 44x38
  `door_d2_01` preview is `blocked_native_g0693`, because the genuine D3
  side-door route is the separate native-packed 48x41 G0693 span through
  F0489/F0488. Verification: local original PC3.4 source-artpack build,
  manifest inspection, and Artpack Studio self-test PASS.
- ✅ 2026-07-29 M12 launcher mouse geometry: the modern settings-tab hit
  target now uses the renderer-owned 34px tab height, and the DM1 game-options
  view maps each visible four-column tile to its own control. Patch, language,
  cheats, speed, aspect and resolution clicks therefore no longer fall through
  to a different full-width row. Verification: `menu_hit_launch_direct_click_m12`,
  `menu_hit_settings_tab_m12`, `m12_menu_mouse`, and
  `m12_menu_row_hit_height_audit` PASS; `firestaff` builds.
- ✅ 2026-07-29 CSB V2.x startup verification repair: the real PC3.4
  V1/V2.0/V2.1/V2.2 handoff probes now use ReDMCSB's source-owned C200
  Enter command, rather than hard-coded window coordinates whose meaning
  changes with the host scale mode. The direct C407 entrance-pointer regression
  remains separate. The title-plan regression now locks TITLE.C's actual first
  CHAOS zoom frame (16x4 at source step 2), not a later 48x12 approximation.
  Verification: complete `ctest -L csb` 104/104 PASS.
- ✅ 2026-07-29 CSB PC3.4 runtime-audio consumption: M11 now observes each
  source-owned completed `SOUND.C` F0064/F0065 play while synchronizing the
  live CSB profile and forwards its exact `GRAPHICS.DAT` PCM payload through
  the authenticated IBMIO F8119 transport. This covers both immediate and
  one-tick-delayed plays reached by keyboard or pointer input, without a
  marker/SND3 fallback. Verification: `firestaff` build,
  `test_csb_v1_pc34_runtime_audio_transport`, and the local-PC3.4
  V2.0/V2.1 startup plus Prison-pointer probes PASS.
## 2026-07-29 - CSB V2.2 F0128 door-command consumption

- Bound the two proven PC3.4 front-door routes to the live CSB V2.2
  compositor. `door_d0_01` and `door_d1_01` now require their exact
  GRAPHICS.DAT provenance, `C10_COLOR_FLESH` transparency, and the original
  F0128 clip/order before their replacement pixels can appear.
- The replacement runs immediately after its V1 source command, so later
  F0115 Thing overlays remain in original draw order. D3 and all unproven
  material types stay on V1 instead of using rectangle fallback art.
- Verified with Ninja and the focused V2.2 source-artpack, route, cache, and
  real PC3.4 viewport CTest lane. The direct cache regression supplies an
  admitted D1 and D2 records and requires an opaque pixel to alter only each
  original F0128 clip while alpha-zero pixels preserve the source framebuffer;
  a changed transparency contract is rejected.

## 2026-07-29 - CSB D3 door-set source binding

- Corrected the CSB D3 F0111 receipt from a captured, invalid hard-coded
  `GRAPHICS.DAT` index 693 to ReDMCSB `DUNVIEW.C:2651-2658`'s actual
  `M633 + DoorSet * C003` selection. The acceptance gate now admits only the
  four PC3.4 D3 records 246, 249, 252 and 255, and rejects D2/D1 neighbours.
  The focused real-data D3L2/D3R2 regression passes against the local CSB
  `GRAPHICS.DAT`. The remaining D3 item is the real F0488 44x38-to-48x41
  native-buffer decode/padding, which stays explicitly blocked in TODO.

- Follow-up: extracted ReDMCSB `MEMORY.C` and `IMAGE2.C` show that the PC/I34
  F0111 branch calls `F0616_CopyBitmap` after F0489; it does not use the
  legacy MEDIA009 `M075_BITMAP_BYTE_COUNT(48,41)` copy. The PC3.4 D3 contract
  now retains the original record metadata (44x38) and rejects the former
  fictitious padding buffer. The focused real-data D3 route regression passes.

- The broad first-frame materialization target now links `firestaff_csb_v2`,
  matching the production V2.2 call edge. Its D3 fixtures consume the same
  DoorSet-0 record 246 and 44x38 packed/expanded contract as the PC/I34
  implementation. The target now links and passes instead of hiding the V2.2
  consumer behind unresolved symbols.

## 2026-07-29 - CSB V2.0 full-surface startup capture

- ✅ Strengthened `csb_v20_filtered_startup_capture` to validate the actual
  top-down BMP geometry and storage size and to require non-black pixels over
  at least half of each captured axis. Four files alone can no longer hide a
  host-scaling regression that collapses PRESENTS, CHAOS, STRIKES BACK, or
  Entrance into a partial strip. The same portable assertion is shared by the
  V2.1 EPX capture regression. Verified against the local original PC3.4
  `GRAPHICS.DAT`/`DUNGEON.DAT` package.
- ✅ 2026-07-29 CSB PC3.4 G0693/G0694/G0695 door-set receipt correction:
  D1 and D2 no longer validate unrelated fixed `GRAPHICS.DAT` records 558 and
  694. All three F0111 depths now use ReDMCSB `DUNVIEW.C:2651-2658`:
  `M633 + DoorSet * C003 + depth` (DoorSet 0: D3=246, D2=247, D1=248).
  The generated source `.fsart` uses nearest-neighbor cache normalization
  rather than Lanczos and records D3's true 44x38 F0616 source surface as
  `blocked_f0791_projection`. Verification: real local CSB `GRAPHICS.DAT`
  receipts for D1/D2/D3 and the full 107-test CSB CTest lane pass.
- ✅ 2026-07-29 CSB V2.2 D3 F0791 native-door consumption: the D3L2/D3R2
  F0128 command consumer now admits the real G0693 DoorSet record 246 only
  when its `.fsart` provenance retains the PC/I34 44x38 surface. It draws
  that raster at the source-owned C3700/C3710 clip origin, leaving the final
  4x2 clip margin and all C10-transparent source pixels untouched. The source
  pack marks this as `admitted_d3_f0791_native`; it no longer invents a
  48x41 or scaled D3 panel. Verification: source-pack manifest inspection and
  focused V2.2 admission/cache CTests pass.
- ✅ 2026-07-29 CSB selected `.fsart` runtime materialization: a launcher
  selection now reaches M11, is extracted into a MD5-keyed user cache through
  the shared ZIP reader, and binds its manifest plus `v22_inplace_cache.bin`
  before FAMG re-admits V2.2. A real PC3.4 boot probe with the source-derived
  pack reaches map 0 in `presentationMode=3`; this does not claim that the
  remaining live F0128 perspective-cell renderer is complete.
# Completed work

- 2026-07-29: Made CSB V2.2 F0128 door replacement visibility-aware. Modern
  art is now admitted only for a sampled, closed `M034_SQUARE_TYPE` door at
  the source command's actual depth/lane; corridors, missing cells and open
  doors preserve the completed V1 frame. Verified by focused CSB startup,
  materialization and V2.2 route/cache CTests.

- 2026-07-29: Extended the CSB V2.2 F0128 art-command bridge to all real
  PC3.4 map DoorSets. The live dungeon header now selects `G0693/G0694/G0695`
  with ReDMCSB's `M633 + DoorSet * C003 + depth` formula, so V2.2 cannot paint
  a DoorSet-0 asset over later maps. Verified by the CSB F0128 material,
  V2.2 route/cache and startup sequence CTests.

- 2026-07-29: Wired CSB V2.2 modern door art to a source-owned ReDMCSB
  `DUNVIEW.C F0128` command plan. M11 now authenticates the active PC3.4
  `GRAPHICS.DAT` catalog and passes the D1/D2/D3 DoorSet-0 commands
  (248/247/246) with original clipping, transparency and draw order to the
  V2.2 renderer. Unsupported source archives remain V1, fail closed.
  Verified by the focused CSB startup, F0128 material and V2.2 cache CTests.

- 2026-07-29: Fixed CSB `TITLE.C F0437` CHAOS title geometry. The first
  visible reverse-zoom frame is 48x12 and the final full 320x80 C425 raster
  remains present through the source `Delay(20)` hold. Verified by
  `csb_v1_startup_receipt_coherence_pc34_compat`.
- ✅ 2026-07-29 CSB PC3.4 CHAOS title cadence: restored the real C001 zoom
  geometry after an erroneous 48x12 shortcut skipped the first two frames.
  TITLE.C now starts at 16x4 for source step 2, reaches full 320x80 only at
  step 21, and begins the 20-VBlank hold there. The packaged runtime capture,
  host route, full title/HUD/Entrance handoff and local PC3.4 real-asset
  launch probe all pass.
- ✅ 2026-07-29 CSB V2.2 source-artpack runtime counter: the real-data V2.2
  startup regression no longer treats the current Prison ingress's absence of
  a closed F0111 door as a permanent ban on V2.2 painting. It requires the
  source-derived pack and a well-formed runtime replacement counter, accepts
  zero for this door-free ingress, and accepts a positive count for a later
  route that consumes an admitted F0128 door clip. The command-level test
  retains exact provenance, clip, transparency and draw-order coverage.
  Verification: `csb_v22_source_artpack_runtime`,
  `csb_v22_inplace_draw_pc34`, `csb_v22_inplace_route_pc34`, and
  `csb_v1_boot_runtime_handoff` PASS.
- ✅ 2026-07-29 CSB V2.2 source-HUD ownership: CSB V2.x no longer enters
  DM1's generic vertical-slice HUD path.  That path replaced genuine
  C017/C040 pixels with host status panels even when V2.2 had no admitted
  F0128 replacement.  CSB now keeps its source-owned runtime page outside
  the receipt-admitted viewport; the V2.2 test captures matched V1
  byte-for-byte outside F0128's `(48,33)-(271,168)` aperture after Prison
  handoff. Verification: CSB V1 boot/handoff, V2.0/V2.1 pointer, V2 HUD and
  V2.2 source-artpack runtime regressions PASS.
- ✅ 2026-07-29 CSB V2.2 zero-replacement frame lock: the real-data V2.2
  runtime regression now requires full 320x200 byte identity with V1 when
  `csbV22CellsPainted=0`. The F0128 aperture is exempt only when an
  authenticated door projection was actually admitted, preventing an
  uncounted modern or generic render path from altering the source frame.
  Verification: `csb_v22_source_artpack_runtime` PASS.
- ✅ 2026-07-29 CSB V2.2 source-startup frame lock: the V2.2 real-data
  runtime probe now launches a paired V1 source session and compares the
  stable PRESENTS (palette 4) and completed Entrance (palette 7) host pages
  byte-for-byte. CHAOS/STRIKES remains covered through its source VBlank
  phase/cadence receipt because a wall-clock capture can legitimately land on
  different animation frames. Verification:
  `csb_v22_source_artpack_runtime` PASS.
- ✅ 2026-07-29 CSB V2.0 live filter ownership: the real-PC3.4 V2.0 test
  now boots through Prison with CRT scanlines, palette correction and dither
  cleanup enabled. It proves the indexed 320x200 source frame is byte-identical
  to V1 while the 960x540 presented surface changes, so filters are live only
  after source-page ownership is complete. Verification:
  `csb_v20_filtered_startup_capture` PASS.
- ✅ 2026-07-29 CSB V2.1 live EPX/upscale ownership: the real-PC3.4 V2.1
  runtime test now pairs the completed Prison handoff with V1. The source
  320x200 page remains byte-identical, while the matching presented surface
  differs under V2.1's EPX/upscale path. Verification:
  `csb_v21_presented_startup_capture` PASS.
- ✅ 2026-07-29 CSB V2.x current-suite verification: rebuilt against the
  current CSB worktree and ran `ctest -L csb` in
  `/Volumes/Extern-disk/firestaff-csb-build`; all 107/107 CSB tests pass in
  47.85 seconds. The lane includes the real PC3.4 V2.0 filtered and V2.1 EPX
  startup captures, plus the V2.2 source-artpack runtime boundary. It is
  implementation evidence only: V2.2 still remains fail-closed until the
  TODO-listed reviewed-material and packaged-capture requirements are met.
- ✅ 2026-07-29 CSB C140 disk/save menu runtime: Ctrl-S no longer sets an
  inert `csbDiskMenuActive` flag. It opens the existing source-dialog surface
  with Save Game, Load Game and Cancel; keyboard navigation, Enter/Escape and
  pointer choices share one modal state, and accepted choices call the
  existing CSB F0433/F0435 QuickSave/QuickLoad owners. Verified by
  `csb_v1_keyboard_commands_pc34_compat`, `csb_v1_utility_save_transaction_pc34_compat`,
  and `csb_v1_save_export_import` (3/3 PASS).
- ✅ 2026-07-29 CSB source-owned entrance Resume pose verification: the M11
  resume gate now retains the actual new-game pose decoded from verified
  `DUNGEON.DAT` while proving that native, CSBWin and raw `CSBGAME` candidates
  are not applied before the Entrance Resume command. This removes the stale
  synthetic `(5,5,North)` test assumption while preserving the delayed
  F0435-owned load boundary. Verification:
  `csb_v1_m11_startup_resume_gate` PASS.

- ✅ 2026-07-29 CSB save export/import probe integrity: the mandatory
  synthetic save gate now supplies a valid provenance path for its v2.1
  envelope and reports its summary before any optional external-corpus skip.
  A failed v2.0 or v2.1 production-loader round-trip can therefore no longer
  pass CTest merely because no user-staged `.csbsave` exists. Verified by
  `firestaff_csb_v1_save_export_import_probe` (16/16 checks).

- ✅ 2026-07-29 CSB PC3.4 runtime audio event ordering: completed SOUND.C
  F0064 immediate plays and F0065 pending flushes now retain a bounded,
  source-ordered sequence for M11. Multiple valid sound events between host
  synchronizations each reach the authenticated `GRAPHICS.DAT` PCM transport;
  an expired history fails closed rather than inventing a marker. Verified by
  `csb_v1_audio_runtime_pc34_compat` (64 checks) and the PC3.4 transport
  regression.

- ✅ 2026-07-30 CSB PC34 champion-HUD input: visible status boxes and
  right-click inventory toggles now use the shared ReDMCSB `COMMAND.C G0447`
  C007..C015 command surface when CSB owns the runtime. The route updates
  CSB's party mirror rather than falling into DM1 dungeon state. Verified by
  `test_m11_csb_leader_hand_no_dm1_fallback` and `ctest -L csb` (109/109).

- ✅ 2026-07-30 CSB PC34 action-pointer runtime: the focused M11 CSB
  regression now follows the original `COMMAND.C` C116 action-icon and C114
  action-row routes through F0389/F0391. It proves the runtime-backed dagger
  STAB action updates the CSB champion, clears the action menu, and never
  uses DM1 world state.
- ✅ 2026-07-30 CSB F0128 viewport baseline: the CSB runtime bridge now uses
  `dm1_viewport_3d_init()` before it applies its live dungeon configuration.
  This preserves the shared ReDMCSB G2107/G2110 default wall and door-frame
  tables instead of starting the F0128 frame with zeroed legacy slots.
  Verification: `csb_v1_boot_viewport_render_gate`,
  `csb_v1_m11_prison_runtime_hud_pc34`, and `ctest -L csb` (109/109).

- ✅ 2026-07-30 CSB special-palette screenshot fidelity: F12 now captures
  the renderer's presented RGBA frame during C001-C005 startup phases, so a
  user screenshot has the same source-owned palette as the window rather
  than a later dungeon palette. Verification: V2.0/V2.1/V2.2 real-data
  startup capture regressions.

- ✅ 2026-07-30 Launcher game-data directory persistence: a folder-picker
  placeholder such as `.` can no longer enter through the public data-root
  setter and is reported as unchanged. Empty status display now says `NOT
  SET`, never `.`. Verified by `test_m12_data_dir_cancel`.

- ✅ 2026-07-30 Launcher selected-directory ownership: start-menu state now
  retains the last accepted game-data directory independently of transient
  asset-scanner/dialog output. A macOS/SDL `.` token therefore cannot replace
  the visible picker location or the next config write after a valid folder
  has been selected. Verified by `m12_data_dir_cancel` and
  `asset_status_data_dir_change_cache_invalidation`.

- ✅ 2026-07-30 CSB current-main verification: rebuilt current `main` and
  ran the registered CSB CTest lane in
  `/Volumes/Extern-disk/firestaff-csb-build`. The first 102 registered tests
  passed, then the remaining 10 real-data/title/V2.x tests were rerun
  explicitly: all 112/112 pass. This covers V1 startup/Entrance, V2.0 and
  V2.1 presentation, V2.2 source-artpack admission, HUD, viewport, controller
  input and F10 graphics-popup contracts. It does not close the still-open
  V2.2 F0128 wall/floor/thing material-projection work.

- ✅ 2026-07-30 CSB V2.x feature verification: all 51 registered V2.0,
  V2.1 and V2.2 regressions pass with local PC3.4 data. The lane covers
  startup/title/Entrance, source and presented captures, Prison pointer
  handoff, runtime HUD ownership, filters, scale/EPX, input affordances,
  lighting and the reviewed V2.2 artpack gate. Unadmitted V2.2 F0128
  material families remain open under `CSB-V22-LIVE-M11-RUNTIME-CONSUMPTION`.

- ✅ 2026-07-30 CSB shared spell-panel pointer input: CSB now admits
  ReDMCSB `COMMAND.C` C100 and C101..C109 through the same source-owned
  C009/C011 geometry as DM1. The focused M11 regression proves panel open
  and `Lo` rune entry while preserving the separate, still-open CSBWin cast
  executor boundary.

- ✅ 2026-07-30 CSB V2.x current-main verification: all 51 tests selected by
  the intersected `csb` and `v2` CTest labels pass. The lane covers V2.0
  filters, V2.1 presented startup/upscale, V2.2 reviewed-art gates and
  runtime HUD/input/settings/lighting paths. This does not admit unreviewed
  V2.2 viewport materials; those remain fail-closed by design.

- ✅ 2026-07-30 CSB GAMEBLOCK spell-panel consumption: the C009/C011 spell
  panel now uses the fresh CSB runtime party mirror, just as the C028 and
  action surfaces do. The real-save M11 Prison regression forces the retained
  M11 party empty and proves the source spell panel still draws from the
  actual CSB GAMEBLOCK champion state.

- ✅ 2026-07-30 CSB GAMEBLOCK spell-panel input: C100 now refreshes the CSB
  runtime party mirror before CASTER.C resolves the source caster. The same
  real-save regression proves that an empty stale M11 party no longer blocks
  a valid mouse-open request.

- ✅ 2026-07-30 CSB live mouse-party synchronization: the CSB GAMEBLOCK
  mirror now refreshes once after startup dispatch and before every live
  movement, champion, action, and spell hit test, so those command surfaces
  share one authoritative party view.

- ✅ 2026-07-30 CSB live keyboard-party synchronization: F1-F4 and other
  live CSB keyboard commands now refresh the GAMEBLOCK mirror after startup
  dispatch and before command routing. The real-save M11 regression proves
  F1 opens the source champion inventory even when the retained M11 party was
  deliberately emptied.

- ✅ 2026-07-30 CSB current regression receipt: rebuilt current `main` and
  ran `ctest -L csb` in `/Volumes/Extern-disk/firestaff-csb-build`; all
  111/111 registered CSB tests passed. The intersected V2.x lane remains
  51/51. This verifies the implemented V1/V2.0/V2.1/V2.2 paths and preserves
  the explicit fail-closed boundaries for unreviewed V2.2 F0128 materials and
  CSBWin direct spell execution.

- ✅ 2026-07-30 CSB title/Entrance visual composition guard: strengthened the
  real PC3.4 executable capture contract beyond palette counts. It now checks
  PRESENTS C001 placement, C425 CHAOS zoom bounds, C426 STRIKES BACK span, and
  full-page C002-C005 Entrance coverage, catching decoder-stride and blit
  placement regressions that could otherwise retain plausible colors.
  Verification: `csb_v1_title_entrance_visual_contract` PASS.

- ✅ 2026-07-30 CSB controller bridge regression: extracted the shared SDL
  gamepad action/axis translation into `m11_game_input_mapping.c` and added
  a data-free CSB contract for the default map, menu/gameplay contexts, stick
  directions, and dead-zone. This verifies that CSB receives the same M11
  COMMAND.C input tokens as keyboard and pointer routes.

- ✅ 2026-07-30 CSBWin DSA silent-cast binding: the production candidate
  runner now accepts only `Magic.cpp::CastSpell` action 1 with disabled-time
  `-1`, CSBWin's explicit no-message/no-state-change abort. The runtime test
  proves `STKOP_Cast` publishes that exact fourteen-word request; all other
  spell classes remain intentionally fail-closed pending complete owners.

- ✅ 2026-07-30 CSB original-save direct runtime handoff: original Atari
  `MINI.DAT` resume now takes a dedicated `LOADSAVE.C F0435` route rather than
  pretending it completed TITLE.C/ENTRANCE.C. It verifies the loaded package,
  dungeon, source HUD assets and live viewport before drawing F0128, while
  keeping normal startup's C001-C005/C017/C040 terminal receipt unchanged.
  Verification: `csb_v1_m11_prison_runtime_hud_pc34` with the real 42,815-byte
  `HardDisk/2009-02-22 PP/MINI.DAT` corpus member.

- ✅ 2026-07-30 CSB local macOS presentation capture: the production SDL
  binary captured actual post-present PRESENTS, CHAOS, STRIKES and Entrance
  frames from the verified PC3.4 package. Normal-palette runtime capture
  remains intentionally separate until it has an equally source-owned gate.

- ✅ 2026-07-30 CSB V2.2 F0128 command ownership: admitted original D1/D2/D3
  door commands now replace pixels exactly once inside the source F0128 draw
  stream, with the verified clip, C10 transparency and pack provenance. M11
  prepares the V2.2 cache before F0128 and receives the source-pass count;
  it no longer replays an overlay after composition. Verification: 112/112
  CSB CTests, including `csb_v22_inplace_draw_pc34`,
  `csb_v22_inplace_route_pc34`, and `csb_v22_source_artpack_runtime`.

- ✅ 2026-07-30 CSB normal runtime presented capture: the opt-in capture path
  now writes one normal-palette page only after the current verified package
  session and completed F0128 source receipt agree. It deliberately permits
  later source-owned object passes to change the final aperture rather than
  requiring a stale base-raster hash. Verification:
  `csb_v22_source_artpack_runtime` captures the authenticated runtime page in
  both V1 and V2.2.

- ✅ 2026-07-30 CSB current-main regression verification: rebuilt the current
  main worktree and ran `ctest -L csb`; all 112 registered CSB regressions
  pass. The V2.x intersection remains 51 tests covering the implemented
  V2.0/V2.1/V2.2 feature surface. Unadmitted V2.2 F0128 material families
  remain intentionally source-V1 under `CSB-V22-LIVE-M11-RUNTIME-CONSUMPTION`.

- ✅ 2026-07-30 CSB PC34 title/Entrance cadence: M11 now consumes the
  authenticated 55 ms CSB profile tick while the source-owned title or
  Entrance is active, rather than advancing at a 20 ms host cadence. This
  preserves `TITLE.C F0437`'s PRESENTS/CHAOS sequence and final
  `Delay(2)` STRIKES BACK hold on modern Macs. Verification:
  `csb_v1_m11_startup_vblank_cadence`, `csb_v2_entrance_pointer_boot_probe`,
  `csb_v21_presented_startup_capture`, and
  `csb_v20_filtered_startup_capture` pass.
- ✅ 2026-07-30 CSB V2.2 artpack provenance hardening: imported artpack
  category, asset and source-file fields are now constrained to safe path
  components, compact manifest entries read their fields before `}`, and a
  route provenance record requires a syntactically valid SHA-256 digest.
  Verification: `csb_v22_modern_assets_pc34`, `csb_v22_inplace_draw_pc34`,
  `csb_v22_inplace_route_pc34`, and `csb_v22_source_artpack_runtime` PASS.
- ✅ 2026-07-30 CSB V2.2 live door-record hash binding: F0128 D1/D2/D3 door
  commands now carry the SHA-256 of their exact compressed PC3.4
  `GRAPHICS.DAT` catalog record. V2.2 admits a source artpack surface only
  when that runtime hash matches its manifest provenance; a mismatched record
  fails closed. Verification: `csb_v22_inplace_route_pc34`,
  `csb_v1_boot_runtime_handoff`, and the real-data
  `csb_v22_source_artpack_runtime` PASS.
- ✅ 2026-07-30 CSB DSA cast-runner audit: corrected stale TODO status after
  verifying that the saved-timer/filter runner already binds
  `csb_v1_runtime_dsa_cast_spell` into the transactional stack context.
  `STKOP_Cast` action-1 silent-abort coverage is present in
  `csb_v1_dsa_copy_runtime_handoff`; `I_CAST` and `I_FILTEREDCAST` remain
  intentionally fail-closed because their CSBWin indirect parameter corpus
  lacks the fourteenth `SPELL_PARAMETERS` word.

- ✅ 2026-07-30 CSB dynamic runtime sprite decoder ownership: projectile,
  explosion, item, D0-pattern and creature draws now install CSB PC3.4
  GRAPHICS.DAT records via the CSB IMG3/LZW decoder before M11 consumes them.
  The cache is package-scoped and records rejected entries, preventing generic
  DM1 cache fallbacks and repeated decode attempts. Verification:
  `test_csb_v1_viewport_phase3_rendering` PASS.

- ✅ 2026-07-30 CSB runtime panel decoder ownership: C009--C013 action,
  spell and movement panel blits now acquire their source surfaces through
  CSB PC3.4 IMG3/LZW before M11 cache consumption. This preserves the same
  source pixels in V1, V2.0, V2.1 and V2.2. Verification: the focused
  V1/V2.x startup, HUD and V2.2 source-artpack suite passes 8/8.

- ✅ 2026-07-30 CSB full regression receipt: `ctest -L csb` passes 112/112
  tests from the current Ninja build. The run includes V2.0, V2.1 and V2.2
  presentation, startup, HUD, filter, source-artpack and runtime-handoff
  coverage. This verifies implemented behavior only; the source-data gaps
  explicitly retained in `TODO.md` remain open.

- ✅ 2026-07-30 CSB champion-row decoder ownership: source-owned C008/C028,
  hand-frame, poison, damage and shield records are installed through CSB
  PC3.4 IMG3/LZW as one package-scoped CHAMDRAW set. V1/V2.0/V2.1 runtime HUD
  plus V2.2 presentation paths now fail closed instead of consuming DM1 cache
  pixels. Verification: 10 focused V1/V2.x tests PASS.

- ✅ 2026-07-30 CSB object-icon decoder ownership: shared F0038/F0386 icons
  now resolve from the authenticated CSB PC3.4 package in action, hand and
  inventory paths. The V2.1 source-page capture passed three consecutive
  isolated runs, alongside V1/V2.0/V2.1/V2.2 focused HUD and viewport tests.

- ✅ 2026-07-30 CSB V2.x source-page HUD isolation: V2.0/V2.1/V2.2 now keep
  the original C017/C040 champion composition in the 320x200 source page and
  apply their visual changes only after source rendering. Dialog backdrops and
  patches follow the same source decoder route. Direct V2.1 and V2.2 runtime
  capture scripts PASS.
# CSB V2.x source-title runtime gate (2026-07-30)

- Added `csb_v2_title_boot_probe`: with real local PC3.4 `GRAPHICS.DAT` and
  `DUNGEON.DAT`, V1, V2.0, V2.1 and admitted V2.2 must all remain on the
  original C001 PRESENTS stage at TITLE.C F0437 VBlank 50. The probe locks
  `csb-title-1`, source title state, frame `50/102` and each mode's resolved
  presentation identity. It prevents filters, upscale or V2.2 selection from
  silently shortcutting title playback into Entrance. This is a regression
  gate for the real startup chain, not a claim that unreconciled V2.2 F0128
  material families are complete.
# CSB DSA FILTEREDCAST owner guard (2026-07-30)

- `STKOP_FilteredCast` no longer accepts the otherwise side-effect-free
  `action=1, disableTime=-1` CastSpell branch. CSBWin `Magic.cpp::DSACastSpell`
  runs `CallSpellFilter` first, so treating the later silent CastSpell result
  as sufficient skipped original actuator/EXPOOL effects. Direct `CAST` keeps
  its proven silent-abort route; filtered casts remain fail-closed until that
  full source transaction is implemented. The runtime DSA regression covers
  both outcomes.

# CSBWin real-save resume corpus (2026-07-30)

- The staged CSBWin `csbgame3.dat` corpus now completes production resume:
  verified Extended Features, C29 GAMEBLOCK1, 12-byte sequenced TIMER stream,
  436 timer slots, party state and source-file provenance. The focused
  `test_csb_v1_csbwin_save_provenance_pc34_compat` passes 14/14 with
  `FIRESTAFF_CSBWIN_REAL_SAVE` set to the staged original file. Its later
  dungeon payload remains opaque rather than being falsely consumed as EXPOOL;
  full tail loading and write-back remain tracked in `CSB-ORIGINAL-SAVE-CORPUS`.
- ✅ 2026-07-30 CSB active-map F0094/F0095 material ownership: `MAP.D` now
  decodes the source FloorSet/WallSet nibbles alongside its DoorSets, and the
  live M11 PC3.4 provider selects floor/ceiling records `78 + FloorSet * 2`
  and the 15 G2107 wall records from `86 + WallSet * 40 + 7`. Decoded caches
  are invalidated when the active level changes set. Verification:
  `csb_v1_dungeon_loader_pc34_compat` passes 16/16, including nonzero
  FloorSet/WallSet/DoorSet decoding.
- ✅ 2026-07-30 CSBWin source spell-table binding: the decoded PC3.4
  `GRAPHICS.DAT` graphic `0x230` now yields CSBWin's 25 original `SPELL`
  records at offset `0x404`, retaining its rune ID, required skill, skill kind
  and descriptor. Lookup follows `Magic.cpp::Incantation2Spell`'s source
  packing, so future CSB casts cannot borrow DM1 definitions. Verification:
  `csb_v1_magic_rune_cost_pc34_compat` passes source-table decode, lookup and
  descriptor checks; the full CSB lane remains green.
- ✅ 2026-07-30 CSBWin standard GRAPHICS.DAT decoder boundary: Firestaff now
  parses the authentic Atari ST DMCSB1 size tables before handing C001--C005
  to the existing CSBWin `ExpandGraphic`-compatible planar decoder. This
  verifies the supplied CSBWin package's 320x200 title/entrance/credits
  sources and 128x161 door sources without relabelling it as PC3.4 or
  generating artwork.
  Verification: opt-in `csb_v1_m11_prison_runtime_hud_pc34` real-package
  checks pass, together with V1/V2.0/V2.1 HUD, title/Entrance, and six V2.2
  presentation-material contracts.

- ✅ 2026-07-30 CSB V2.x current verification: all 52 registered CSB V2.0,
  V2.1 and V2.2 CTest contracts pass from the current Ninja build with local
  PC3.4 data. Coverage includes original title/Entrance, real Prison handoff,
  HUD ownership, F10 graphics settings, filters, resolution, V2.1 capture,
  and the reviewed V2.2 source-artpack route. This verifies implemented
  behavior only; unadmitted F0128 material families remain open in `TODO.md`.
# CSBWin TAG0088b2 wall projection rectangles (2026-07-30)

- ✅ The CSBWin viewport mapping now exposes the exact `wallRectangles[]`
  index for every visible wall lane, independently of source-bitmap and
  F3R2 mirror selection. `test_csb_v1_csbwin_layout_0232` passes against the
  local Atari-ST `GRAPHICS.DAT`.

# CSB V2.2 missing-art containment (2026-07-30)

- ✅ Retired the 16×16 magenta checkerboard missing-art surface from the CSB
  V2.2 modern-asset boundary. A missing modern bitmap now reports a null,
  zero-sized surface, leaving the source-owned V1 command intact rather than
  injecting generated pixels. `test_csb_v22_modern_assets_pc34` covers this
  fail-closed contract. This does not claim completion of the still-open
  source-material bindings in `CSB-ORIGINAL-REPLACE-001`.

# CSBWin runtime diagnostic-chrome removal (2026-07-31)

- ✅ 2026-07-31 CSB source-dungeon boot boundary: `csb_v1_boot_enter_game()`
  now promotes only the ReDMCSB DUNGEON_HEADER/MAP byte-map (`square_bytes ==
  1`) into live runtime state. The compatibility loader still accepts its old
  16-bit shape for parser unit tests, but boot frees it and keeps the title
  handoff dungeon-less. `test_csb_v1_boot_runtime_handoff` uses the real map
  layout and proves the legacy fixture is rejected (501 assertions).

- ✅ 2026-07-31 CSB source-owned custom-background masks: verified runtime
  sessions now ignore caller-provided fixture rectangles and use only the
  selected CSBgraphics `BACKGROUND_MASK` decode. A missing or malformed source
  mask leaves the layer unapplied rather than painting generated coverage.
  ReDMCSB/CSBWin `Viewport.cpp` applies the room skin layers in source order;
  `test_csb_v1_csbgraphics_runtime_plan` proves a poisoned fixture mask is
  rejected in a real session (192 assertions).

- ✅ The authentic Atari-ST runtime viewport and C232 HUD composition no
  longer emits the host-only `ATARI RUNTIME READY` or `CSBWIN SOURCE FRAME -
  EXTENDED CELLS REQUIRED` status strings. `ANIM.C` hands off directly to
  FTLCODE, and the completed C232/TAG0088b2 page remains its only pixel/text
  owner. The M12/M11 handoff regression includes both absence checks; its
  PC3.4 lane passes, while the test retains its explicit skip until the local
  Atari-ST package is supplied through `FIRESTAFF_CSB_ATARI_ST_ROOT`.

# CSB F0113 generated-field containment (2026-07-31)

- ✅ A live, source-verified CSB viewport now refuses the shared
  geometry-only cyan teleporter fill. ReDMCSB `DUNVIEW.C F0113` owns a native
  field bitmap, so a session without a checked source span leaves that area
  no-draw. Data-free geometry tests retain their fill helper. Verification:
  `test_csb_v1_viewport_phase3_rendering` passes 2,655 checks, including the
  real-session no-cyan regression.

# CSB V2 HUD generated-chrome containment (2026-07-30)

- ✅ Retired the procedural compass, numeric font, champion bars, action
  icons and chaos-rune overlay from the CSB V2 HUD render entry points. The
  compatibility state API remains available, but both direct and runtime
  render calls are strict no-draw, including when the V2 phase gate permits
  presentation. PC3.4 C017/C040 and Atari ST C232 remain the only HUD pixel
  owners. Verification: `test_csb_v2_hud_overlay_pc34`,
  `test_csb_v2_hud_runtime` and `firestaff_csb_v2_hud_overlay_probe` pass;
  the real Atari-ST M12/M11 handoff regression passes 589 checks. This does
  not promote unbound F0128 viewport materials.

# CSB V2.2 source-export admission hardening (2026-07-30)

- ✅ The V2.2 material gate no longer treats an arbitrary non-placeholder
  generator label as provenance. It admits only
  `original_csb_pc34_graphics_dat`, the identifier emitted by
  `scripts/build_csb_v22_source_fsart.py` for decoded, hash-verified PC3.4
  `GRAPHICS.DAT` records. A disk-resident `pbr_hero`, AI or reviewer-labelled
  PNG remains partial even if it declares an admitted projection. Verification:
  `test_csb_v22_finished_art_material_gate_pc34` passes 823/823 and
  `firestaff_csb_v22_finished_art_material_gate_probe` passes 203/203;
  modern-assets and in-place command regressions remain green. This is an
  admission boundary, not a claim that currently unbound F0128 routes render
  in V2.2.

# M12 startup-menu reliability pass (2026-07-30)

# CSB Entrance pre-open composition (2026-07-30)

- ✅ The source-owned CSB `OPENING_DELAY` state now keeps C004 with its
  closed C002/C003 door strips visible until the first real opening step.
  The previous host-surface gate accepted only the closed and moving-door
  variants, which left a black frame during the legitimate delay. The
  F0128 aperture receives the same verified C004 source during this state;
  no synthetic frame or door geometry was added. Focused startup composition
  and pointer regressions pass.

# CSB source-owned presentation audit (2026-07-30)

- ✅ 2026-07-31 CSB D2L2/D2R2 F0111 partly-open-door synthetic blit removal:
  removed the contract-only C10 framebuffer writer and its generated fixture.
  The source-lock probe now covers ReDMCSB branch selection, D2 lane routing,
  zones, clipping and transparency metadata without manufacturing pixels.

- ✅ 2026-07-31 CSB D1L2/D1R2 F0111 partly-open-door synthetic blit removal:
  removed the C10 fixture writer from the D1 side-door contract. The probe
  retains F0111/F0122/F0123 dispatch, frame and zone evidence, but cannot
  create a viewport pixel without authenticated source material.

- ✅ 2026-07-31 CSB D0L2/D0R2 F0111 partly-open-door synthetic blit removal:
  removed the mirrored C10 fixture writer. ReDMCSB cell order, mirror setup,
  zone and ornament-keepout evidence remain in the metadata-only probe.

- ✅ 2026-07-31 CSB D2L2/D2R2 F0111 partly-open fixture removal: removed the
  unauthenticated C10 writer and retained the source-bound clip, zone and
  transparency metadata only.

- ✅ 2026-07-31 CSB D2L/D2R wall fixture removal: removed the C10 frame-copy
  helper. The source-lock test retains ReDMCSB D2 wall dimensions and routing
  metadata without generating viewport pixels.

- ✅ 2026-07-31 CSB D3L/D3R wall fixture removal: removed the matching C10
  frame-copy helper and retained ReDMCSB D3 wall geometry metadata only.

- ✅ 2026-07-31 CSB D3L2/D3R2 F0115 fixture removal: removed the
  unauthenticated C10 writer while keeping ReDMCSB routing, cells and flip
  metadata in the source-lock test.

- ✅ Rechecked the production order start, menu boundary, Entrance, HUD and
  viewport against the local hash-verified CSB packages. The PC3.4 real-data
  launch probe passed 75 checks: C001 title phases, C002/C003/C004 entrance
  composition across all 31 opening pages, and C017/C040 terminal HUD
  ownership all use decoded original pixels. The Atari M12/M11 handoff passed
  589 checks using its separate CSBWin-owned graphics path. Shared PC3.4 HUD
  and viewport sprite helpers install the active CSB graphic before drawing
  and leave the destination unchanged when it is missing; no marker, generic
  DM1 surface or generated replacement was admitted. This records an audit,
  not completion of the remaining CSB HUD or full viewport work.

# CSB thrown-object projectile containment (2026-07-30)

- ✅ Removed the production fallback that drew a 16×16 object-atlas icon when
  a source-bound thrown-object projectile had no resolved perspective bitmap.
  ReDMCSB `DUNVIEW.C` F0115 instead branches through
  `T0115015_DrawProjectileAsObject`, selecting the G0209/M612 native object
  bitmap for the C2900/F0791 lane. The cell now remains unchanged until that
  native projection is bound; data-free geometry probes retain their explicit
  diagnostics. Verification: `test_csb_v1_viewport_phase3_rendering` (2649
  assertions) and `test_m11_csb_leader_hand_no_dm1_fallback` pass.

- ✅ Hardened the same source-bound F0115 group path for malformed C04 data:
  an undecodable creature type now remains no-draw instead of reaching the
  diagnostic group cross. The marker remains available only to data-free
  geometry callers. Verification: `test_csb_v1_viewport_phase3_rendering`
  passes 2649 assertions.

- ✅ M11 now propagates source ownership into the object-sprite drawer
  binding, matching its existing projectile and group bindings. A verified
  CSB session therefore cannot reopen the icon/marker fallback when an F0115
  object bitmap fails to decode. Verification: the phase-3 viewport contract
  passes 2649 assertions and `test_m11_csb_leader_hand_no_dm1_fallback`
  passes.

# CSB V2.x verification (2026-07-30)

- ✅ Rebuilt current `main` with Ninja and ran the registered CSB V2.x lane
  against the local PC3.4 corpus: 52 of 53 tests passed. The sole skipped
  test, `csb_v22_source_artpack_runtime`, correctly requires a complete
  independently admitted V2.2 material pack. The installed source export
  declares its unfinished F0128 wall/floor routes as `unbound`, so the host
  resolves V2.2 to V2.1 rather than drawing guessed modern pixels. V1,
  V2.0, V2.1, settings, filter, input, HUD, title, Entrance and the admitted
  V2.2 command boundaries are all covered by this lane.
- ✅ 2026-07-30 M12 startup-settings interaction cleanup: the modern launcher
  now gives every ordinary settings value explicit `<` / `>` controls; a
  label click selects without silently changing its value. Long labels and
  values are fitted inside their own half of the row. `FPS OVERLAY` is now a
  visible, persisted Graphics setting and feeds the existing M11 runtime
  overlay. `menu_hit_settings_tab_m12` covers the new row and both pointer
  directions; the focused M12 suite passes 10/10.
- ✅ 2026-07-30 CSBWin `0x22e` door-layout recovery: Firestaff now decodes
  the original nine `DoorRectsF1R1..DoorRectsF3L1` ten-state projection
  families plus all seven top-track and eight frame `RectPos` records from
  Atari `GRAPHICS.DAT` item `0x22e`. The offsets are source-locked to
  `CSBWin Data.h` immediately before `wallRectangles[]`; real Atari data and
  the M12/M11 handoff boundary pass 589 assertions. This intentionally
  recovers geometry only: actual door panels still require the source DB0
  state/type and `DoorGraphic[3][2]` material owner.

# 2026-07-31 — M12 global launcher preferences

- The start menu now owns and persists a real global renderer preference,
  SDL playback-device name, master/music/SFX volume, display brightness, and
  launcher language.  Changing the global language synchronizes all five
  game launch profiles, so the same language is handed to DM1, CSB, DM2,
  Nexus, and Theron's Quest rather than only changing launcher text.
- Playback-device choices are enumerated from SDL at runtime and a vanished
  saved device falls back to SDL's real system-default output.  Brightness is
  a host presentation adjustment applied after source pixels are decoded; it
  never changes original palettes or game data.  Renderer selection is read
  before renderer creation, with the SDL software renderer selected only on
  explicit request.
- Verification: `cmake --build build-dm2-main-verify --target firestaff
  firestaff_m12_language_cycle_layout_probe --parallel 4` passed;
  `SDL_VIDEODRIVER=dummy ./build-dm2-main-verify/firestaff_m12_language_cycle_layout_probe`
  passed 63/63; `git diff --check` passed.

- ✅ 2026-07-31 CSB F0134/F0135 arbitrary-fill closure: removed the unused
  public helper and real-data fixture that copied authentic C017/C040 HUD or
  C002/C003 entrance rasters before overwriting them with caller-selected
  palette indices. No runtime consumer existed, and the helper had neither a
  ReDMCSB command nor a material receipt authorizing those pixels. Original
  startup and viewport paths remain the sole pixel owners. Verification:
  configure/build and the PC3.4 real-asset startup and viewport regressions.
- Follow-up: the selected playback device is now bound immediately after M12
  initialization, before title or entrance audio can open an SDL stream; game
  reinitialisation retains the same preference through the existing launch
  handoff.
- ✅ 2026-07-31 M12 global-preference contract verification: the public
  settings-row API now identifies the playback-device and display-brightness
  rows, and the launcher handoff regression covers their exact values. It
  also drives the global language picker and proves that the chosen language
  propagates to all five game launch slots. Verification:
  `test_m12_launcher_options_runtime_handoff` passes.

- ✅ 2026-07-31 DM2 sensor fixture removal: removed the dormant five-entry
  pressure-plate and eight-entry trigger catalogs, including their invented
  coordinates, item IDs, timer periods, messages, door targets and creature
  spawns. Both public adapters now return no records and reject all mutation
  attempts until the original SKProject dungeon record-chain and actuator
  owners are decoded. Verification: `test_dm2_v1_pressure_plate_pc34_compat`
  and `test_dm2_v1_trigger_pc34_compat` pass; both focused admission probes
  pass; `test_dm2_v1_door_button_toggle_pc34_compat` passes 12/12.

- ✅ 2026-07-31 DM2 merchant identity closure: an unbound runtime now starts
  with `DM2_NPC_NONE` rather than a fabricated friendly merchant. The blocked
  coordinate-only NPC route therefore exposes neither a local identity nor a
  dialogue selection before a real AI-33 DB creature and its CCM/UI chain are
  admitted. Verification: `test_dm2_v1_shop_pc34_compat` 6/6,
  `test_dm2_v1_runtime_shop_pc34_compat` 11/11, and the hash-verified
  `test_dm2_v1_m11_startup_profile_gate` pass with local DM2 data.

- ✅ 2026-07-31 CSB F0115 orphan group/projectile blit closure: removed two
  unused public pixel writers and their isolated test targets. They accepted
  caller-owned sprite buffers with no M11 runtime consumer or material
  receipt, despite source-index checks. The active F0115 object, group and
  projectile runtime drawers still require decoded original graphics and
  remain no-draw on a missing material. Verification: configure/build and
  PC3.4 real-asset viewport/startup regressions.
- ✅ 2026-07-31 DM2 door-strength source correction: replaced the unrelated
  0x10/0x11 lookup and fabricated 6-or-1 fallback in
  `dm2_v1_query_door_strength_receipt()` with SKProject's exact
  `QUERY_DOOR_STRENGTH`: `DOORS/dtWordValue/GDAT_DOOR_STRENGTH` (`0x0f`).
  Missing source field now rejects rather than inventing a value. Source:
  `SKWIN/SkWinCore.cpp:61120-61127`, `SKWIN/defines.h:656`. Verification:
  `test_dm2_v1_gdat_querydb_receipts` passes 118/118, including the local
  real-`GRAPHICS.DAT` querydb checks.
- ✅ 2026-07-31 CSB D0C F0111 orphan-compositor closure: removed the public,
  test-only center-door compositor and its contract fixture. It had no M11
  runtime consumer, so supplied live-frame receipts could not make it a
  production material binding. The authenticated viewport materializer is
  unchanged and remains the sole production pixel owner. Verification:
  configure/build and PC3.4 real-asset viewport/startup regressions.
- ✅ 2026-07-31 DM1 PC34 F0433 save-section correction: the source command
  writer now emits the five F0420-obfuscated parts contiguously, with their
  sizes owned by F0433/F0435 and decrypted GLOBAL_DATA rather than a private
  Firestaff length prefix. The legacy fixture envelope remains explicitly
  isolated. Verification: `dm1_v1_f0433_save_command_pc34_compat` passes.
- ✅ 2026-07-31 CSB F0115 native-object material-boundary closure: moved the
  direct object blit behind M11's verified `M11_AssetSlot` boundary and
  removed its public caller-buffer API and isolated test target. The source
  mapping remains ReDMCSB-owned; an object draws only after M11 installs and
  decodes its original PC3.4 graphics record. Verification: configure/build
  and PC3.4 real-asset viewport/startup regressions.
- ✅ 2026-07-31 DM1 PC34 corpus expansion: provenance-attested the third
  operator-supplied DOSBox `DMSAVE.DAT` (`5bcee58c`, 48,561 bytes) and added
  it to the external corpus. It passes the real `DUNGEON.DAT`-backed F0435 ->
  native quicksave -> F0433 -> F0435 roundtrip, raising the verified corpus
  to three original saves. Its empty C13/C03/C04 runtime state is documented
  separately and does not claim unobserved coverage. Reference: ReDMCSB
  `LOADSAVE.C F0419/F0420/F0433/F0435` and DMweb Saved Game Files.

- ✅ 2026-07-31 CSB projectile-marker fallback removal: removed the
  coloured cross emitted for an unbound F0115 projectile. The original route
  needs its perspective-selected native bitmap, so all missing projectile
  material now remains no-draw, including data-free configurations.

- ✅ 2026-07-31 CSB object-marker fallback removal: removed the public
  object-colour cross renderer and all unbound object fallback branches.
  ReDMCSB `DUNVIEW.C F0115` projects native object material; a missing sprite
  or icon callback now leaves the source page intact.

- ✅ 2026-07-31 CSB group-marker fallback removal: removed the public
  creature-colour cross renderer and its data-free fallback branch. ReDMCSB
  `DUNVIEW.C F0115` requires the perspective-selected creature graphic; if
  the native sprite cannot be supplied, the group is now no-draw rather than
  a coloured diagnostic symbol. The existing real sprite callback remains
  unchanged.

- ✅ 2026-07-31 CSB first-backdrop synthetic-pixel contract removal: removed
  the unconsumed custom-background fixture that claimed F0098/F0107 ordering
  through fixed generated colours and a synthetic MASK 0x8000 sample. It had
  no M11 consumer and no decoded `GRAPHICS.DAT` bitmap/mask pair. Source
  ordering remains documented in ReDMCSB `DUNVIEW.C F0098/F0107`; a future
  visible backdrop must bind original material before drawing.

- ✅ 2026-07-31 CSB Hint Oracle layout-placeholder removal: removed the
  isolated shape/ASCII-sketch API, its synthetic layout test and its
  real-data probe. It was not consumed by M11 and explicitly admitted an
  empty hint title as a placeholder, while neither ReDMCSB nor CSBWin
  provides a corresponding generic panel raster. The existing HCSB.HTC
  parser and source-bound UI routes remain; a future visible Hint Oracle must
  bind the actual decoded source graphics and font before it can draw.

- ✅ 2026-07-31 CSB C001 presentation-cadence contract: corrected the stale
  receipt constants from 101 ticks and a two-tick CHAOS hold to the real
  `TITLE.C F0437:455-463` sequence: 60 PRESENTS ticks, 20 zoom blits,
  `F0022_MAIN_Delay(20)` on full CHAOS, then `Delay(2)` on STRIKES BACK.
  The focused receipt test asserts the independent 102-tick/20-hold values,
  so a later presentation gate cannot silently reintroduce the old cadence.

- ✅ 2026-07-31 CSB D1C F0108 real-material boundary: removed the isolated
  seed-driven floor/ceiling/ornament trace, its generated-pixel hash, public
  self-test contract and data-free CTest target. The remaining F0108 helpers
  are only C10 transparency and ReDMCSB's `C1500 + CoordinateSet * 11 +
  ViewFloor` zone formula, consumed by the PC3.4 real-data ornament probe
  over decoded, hash-verified `GRAPHICS.DAT` bytes. Source: ReDMCSB
  `DUNVIEW.C` F0108:3940-4011 and `DEFS.H`:2088; dmweb documents PC CSB
  `GRAPHICS.DAT` as DMCSB1 big-endian. Verification: configured build of
  `firestaff`, `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` and
  `firestaff_csb_v1_pc_real_asset_launch_probe`; focused CTest 3/3 passes;
  real ornament probe 29/29 and real startup/title/entrance/HUD probe 75/75
  pass with `/Users/bosse/.firestaff/data/csb`.

- ✅ 2026-07-31 CSB F0108 footprints-contract removal: removed the orphaned
  public plan, its synthetic-contract CTest and stale audit references. The
  module had no M11 or real-data consumer and never decoded an original
  ornament bitmap, palette or mask. Source facts remain covered by the
  source-bound F0108 paths; ReDMCSB reference is `DUNVIEW.C`
  F0108:3940-4011. Verification: configured build of `firestaff` and both
  PC3.4 real-asset probes, focused CTest 3/3, ornament 29/29 and
  startup/title/entrance/HUD 75/75 with `/Users/bosse/.firestaff/data/csb`.

- ✅ 2026-07-31 CSB D2C partly-open-door contract removal: removed the
  redundant public F0111 metadata/probe module and its data-free CTest. It
  had no M11 consumer and no original bitmap, palette or mask decoder. The
  active D2C F0111 source-material route remains unchanged. Reference:
  ReDMCSB `DUNVIEW.C` F0111:4218-4337 and F0121:7244-7389. Verification:
  configured `firestaff` plus both PC3.4 real-asset probes, focused CTest
  3/3, ornament 29/29 and startup/title/entrance/HUD 75/75 with
  `/Users/bosse/.firestaff/data/csb`.

- ✅ 2026-07-31 M11 CLI presentation overrides: `--fullscreen` and
  `--no-vsync` now update the same M11 window-mode and VSync settings that
  the startup menu applies before any DM1 frame is presented; neither option
  is an advertised no-op anymore. Verification: `firestaff --menu --duration
  0 --no-vsync --fullscreen` exits successfully with SDL's dummy driver, and
  the direct-launch default-options gate covers the unset override state.
- ✅ 2026-07-31 DM2 companion fixture closure: `dm2_v1_companion_add()` no
  longer constructs an NPC from caller-provided name and combat values or
  seeds loyalty/AI defaults. The compatibility API now rejects without
  changing state until a live DB creature plus CAII/CCM, inventory and
  dialogue ownership is imported. Verification:
  `test_dm2_v1_companion_source_gate` 6/6 passes.
- ✅ 2026-07-31 DM2 V2 lighting admission closure: M11 now gives V2 lighting
  a separate disabled production gate, rather than enabling procedural sky,
  fog and lightning state through the generic V2 presentation selection.
  Activation now waits for a live V1 `ENVIRONMENT` GDAT weather receipt
  (timer, original image and palette chain). The source-backed V1 weather
  renderer remains the only admitted outdoor pixel owner. Source:
  SKProject `SKWIN/SkWinCore.cpp::ENVIRONMENT_DRAW_DISTANT_ELEMENT` and
  `SKULLWIN/c_weather.cpp`. Verification:
  `test_dm2_v1_m11_startup_profile_gate` passes with real DM2 data.
- ✅ 2026-07-31 DM2 shop item-ID fixture closure: removed the final fixed
  potion, weapon, food and light item IDs plus their fabricated stack limits.
  The unavailable shop API now treats every unbound object as a single item;
  a real DB/GDAT record must provide any quantity rule. Verification:
  `test_dm2_v1_shop_pc34_compat`, `test_dm2_v1_spell_pc34_compat`, and
  `test_dm2_v1_m11_startup_profile_gate` pass.
- ✅ 2026-07-31 DM2 projectile-payload ownership closure: removed the live
  dispatcher’s fabricated centre cell, target-derived direction, fixed
  kinetic/step energy and zero tick. Creature, spell and bomb entry points
  now fail closed until the original DB creature + CCM/timer event payload is
  imported; only explicitly compiled test fixtures can construct a projectile.
  Source: SKProject `SKULLWIN/c_creature.cpp::DM2_PROCEED_CCM` and
  `c_tim_proc.cpp::DM2_STEP_MISSILE`. Verification:
  `test_dm2_v1_projectile_pc34_compat` 24/24,
  `test_dm2_v1_projectile_step_pc34_compat` 16/16, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 DM2 trigger-spawn ownership closure: removed the private
  trigger-target shortcut that called `ALLOC_NEW_CREATURE` with fabricated
  direction `0` and health multiplier `8`. No trigger can now create a
  creature until the original DB14/CCM/timer payload is decoded. Source:
  SKProject `SKULLWIN/c_creature.cpp::DM2_INVOKE_ACTUATOR` and
  `CREATE_MINION`. Verification: `test_dm2_v1_runtime_shop_pc34_compat`
  11/11 and real-data `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 DM2 trigger/plate spawn cleanup: removed the remaining
  no-op spawn dispatcher and its fixed Dragoth-minion argument. Generic
  trigger and pressure-plate targets now contain no latent creature payload;
  only a future decoded DB14/CCM/timer route may allocate one. Verification:
  `test_dm2_v1_runtime_shop_pc34_compat` 11/11 and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.
- ✅ 2026-07-31 DM1 PC3.4 M516 source-slot handoff: corrected F0435's
  champion-inventory import to translate ReDMCSB `DEFS.H` persisted
  `C00_READY_HAND..C29_BACKPACK` ordinals into Firestaff's source-layout
  inventory indices. The former direct copy put saved hand/equipment objects
  in unrelated panel slots, producing wrong item placement and misleading
  F0337 torch input. The same mapping now protects runtime fingerprints and
  source-byte receipts; the PC3.4 fixture writes its action hand at persisted
  ordinal `1`. References: ReDMCSB `LOADSAVE.C F0435`, `DEFS.H`,
  `COMMAND.C C507..C536`, and DMweb's PC saved-game format page. Verification:
  `dm1_v1_original_save_pc34_handoff` and the real-data
  `dm1_v1_original_save_pc34_backed_corpus_roundtrip` pass.

- ✅ 2026-07-31 DM1 PC3.4 M516 export/import symmetry: F0433 now reverses
  the Firestaff panel-layout mapping before serializing `M516_CHAMPION::Slots`;
  F0796 applies the same persisted-order translation on direct import. A
  live changed ready/action hand or equipment slot therefore retains its
  original PC34 ordinal across F0435 -> F0433 -> F0435. References: ReDMCSB
  `DEFS.H`, `LOADSAVE.C F0433/F0435`; DMweb saved-game format documentation.
  Verification: native exporter/importer, handoff, tail-less backed and
  real backed-corpus PC34 roundtrip tests pass.

- ✅ 2026-07-31 DM1 PC3.4 a6fa347b resumed-light verification: reran the
  exact externally owned DOSBox save after the M516 slot mapping repair. The
  Firestaff V1 capture is now visibly lit and resumes the same map/runtime
  route as the original DOSBox frame; no artificial torch or magical-light
  value was added. Direct headless startup with the exact save also produced
  frames for V1, V2.0, V2.1 and V2.2, and the full DM1 V2 family passes 87/87.
  References: ReDMCSB `LOADSAVE.C F0435`, `PANEL.C F0337`, and DMweb's PC
  saved-game format documentation. Scope: runtime/save-light regression;
  this is not a pixel-by-pixel original capture claim.

- 2026-07-31: Closed M11's remaining DM2 leader-hand local-name lookup.
  A DM2 ObjectID can no longer be rendered as a retired catalog label or a
  diagnostic handle; name output remains blocked pending the original decoded
  DB-record → GDAT `dtText/0x18` → `FORMAT_SKSTR` route.

- 2026-07-31: Closed the DM2 save/load dialogue's fixture-only missing
  encryption-flag fallback. Its labels are now admitted only with the
  original `GDAT 0/0/dtWordValue/0` transform owner and the matching
  `DIALOG_BOXES/0x81/dtText` payloads.

- ✅ 2026-07-31 DM1 host-capture receipt integrity: `render_sdl_m11` now
  distinguishes a real SDL presentation window from the headless `dummy`
  driver. The HoC boot receipt cannot report macOS-window or release-app
  capture for an internal test framebuffer. A real macOS window still needs
  an external original-frame comparison; this change prevents synthetic
  readiness from obscuring that open requirement.

- 2026-07-31: Removed DM2's latent shop inventory/gold writeback bridge.
  Without the original `SHOP_GLASS` DB14/CCM/WALL_GFX ownership, no local
  shop state can write arbitrary ObjectIDs or gold values into a live session.
- ✅ 2026-07-31 DM2 spell-timer no-fabrication boundary: the live runtime no
  longer installs the provisional DM2-007 light, enchantment, poison, cloud,
  missile and summon timer handlers. Their old cloud/missile/summon paths
  could write DB14/DB4 records using replacement duration, energy and
  creature-type values; the remaining timer bodies also lacked the complete
  source-owned hero/session state. `DM2_PROCEED_TIMERS` now acknowledges such
  unbound types without gameplay mutation until a full original timer, DB
  record, hero and GDAT ownership handoff is present. Source reference:
  SKProject `SKWIN/c_tim_proc.cpp::DM2_PROCEED_TIMERS`,
  `DM2_PROCESS_TIMER_19`, `DM2_STEP_MISSILE`, and
  `DM2_ALLOC_NEW_CREATURE`.
- ✅ 2026-07-31 DM2 CAII mode-owner boundary: the compatibility setter for
  CAII byte `0x1a` now always rejects caller-authored slot/mode values. It had
  existed solely to manufacture the source's `0x13` delete branch in a test;
  runtime mode writes must instead come from the original CCM/record owner.
  The CAII free regression now proves that an arbitrary mode cannot trigger
  deletion or an invoke-message side effect. Source: SKProject
  `SKWIN/c_1c9a.cpp:5921-5929`.
- ✅ 2026-07-31 DM2 CAII allocation-fixture isolation: arbitrary CAII array
  capacity is no longer accepted by the production runtime. The only callers
  were focused tests, which now compile the helper with
  `FIRESTAFF_DM2_CAII_TESTING`; ordinary builds return failure until DM2_INIT
  binds the original `ddat.v1e08a0` session/save owner. This prevents a
  host-selected pool size from creating a live creature simulation. Source:
  SKProject `startend.cpp:467-494`, `DM2_1c9a_3c30`.
- ✅ 2026-07-31 DM2 creature-spawn fixture isolation: the exported helper no
  longer turns caller-authored creature type, map, coordinates, direction and
  health multiplier into a live production creature. It returns failure in
  normal builds; focused tests and probes compile it explicitly with
  `FIRESTAFF_DM2_CREATURE_TESTING`. Original creation remains pending the
  DB4/current-map/record-chain/RNG binding required by SKProject
  `skcrture.cpp:6380-6430`, `ALLOC_NEW_CREATURE`.
- ✅ 2026-07-31 DM2 creature-animation fixture isolation: arbitrary sequence
  and frame-info words no longer mutate a production creature. Only test
  targets may use the helper; a live route remains pending the complete
  CCM/GDAT animation receipt from SKProject `skcrture.cpp:1595-1658`.

- ✅ 2026-07-31 DM2 spell-timer record-fabrication closure: removed the
  remaining cloud, missile and summon timer bodies that constructed DB14,
  DB10 and DB4 records from reduced timer coordinates/effect words and
  guessed duration, energy or owner fields. Those timer types now acknowledge
  the event without a gameplay mutation until their original DB14/DB4 record,
  actor, direction and timer ownership are decoded. Source: SKProject
  `SKULLWIN/c_tim_proc.cpp:4195-4213` (`DM2_PROCESS_TIMER_19`), `442-563`
  (`DM2_STEP_MISSILE`) and `4268-4280` (`DM2_ALLOC_NEW_CREATURE`).
  Verification: `test_dm2_v1_spell_cast_player_pc34_compat` 142/142 and
  real-data `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB D2C synthetic door-composition removal: removed the
  detached D2C F0111 contract module, its simulated one-pixel composition,
  its receipt-only test target, and obsolete audit references. It had no M11
  consumer and selected a purported door asset only through an isolated test
  receipt. The active CSB viewport continues to bind and blit the verified
  PC 3.4 door raster through the source-command compositor instead.
  References: ReDMCSB `DUNVIEW.C F0121/F0111/F0115` and CSBWin
  `Viewport.cpp` door routing. Verification: focused first-frame material
  test against the local PC 3.4 `GRAPHICS.DAT`, CSB boot/viewport CTests,
  and `git diff --check`.
- ✅ 2026-07-31 DM2 light-timer table binding: replaced the provisional
  one-step light ramp with the exact 16-byte `table1d6702` from SKProject
  `SKULLWIN/dm2data.cpp:60-64`. `DM2_PROCESS_TIMER_LIGHT` now applies the
  original adjacent-table delta and positive doubling from
  `c_tim_proc.cpp:918-959`, and rejects timer values outside that source
  table rather than extrapolating a host curve. Verification:
  `test_dm2_v1_spell_cast_player_pc34_compat` 143/143 and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB detached D0/D1 viewport-contract cleanup: removed the
  non-M11 D0L2/D0R2 F0111/F0115 receipt, D1C F0111 receipt, their private
  test-only pixel simulations and CTest targets. The first-frame check now
  obtains its retained provenance hashes directly from the verified local PC
  3.4 `GRAPHICS.DAT`. The D1C F0115 order decoder remains because the
  realdata ornament probe consumes its ReDMCSB F0115/C10 logic. References:
  ReDMCSB `DUNVIEW.C F0115/F0124/F0125/F0126/F0128` and CSBWin
  `Viewport.cpp`. Verification: focused first-frame material test 109/109,
  realdata ornament probe 29/29, build, and `git diff --check`.
- ✅ 2026-07-31 DM2 spell-cast timer fixture isolation: a successful cast
  receipt can no longer enqueue a reduced `0x46`/`0x47`/`0x19`/`0x1e`/`0x5e`
  timer in a normal build. Such a timer lacks the original DB14/DB4 owner,
  actor state, direction and source timing required by SKProject
  `SKULLWIN/c_events.cpp` and `c_tim_proc.cpp`; the enqueue envelope is now
  available only to the focused source-shape test target. Verification:
  spell-cast source test 143/143, production timer gate pass, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB detached D2L2/D2R2 overlay cleanup: removed the isolated
  F0115 item/explosion metadata compositor and its duplicate test targets.
  It had no M11 or runtime consumer and could only compose caller-supplied
  pixels inside its own tests. The active viewport remains limited to its
  verified source-data routes. References reviewed: ReDMCSB `DUNVIEW.C
  F0115/F0678/F0679` and CSBWin `Viewport.cpp`. Verification: focused
  first-frame material test 109/109, realdata viewport CTests 4/4, build, and
  `git diff --check`.
- ✅ 2026-07-31 DM2 CCM no-stream execution closure: the legacy
  `dm2_v1_ccm_run()` entry point no longer interprets its host program counter
  as a source `b_1a` creature command and can therefore no longer manufacture
  a movement/action from no imported CCM data. It now fails closed without
  state mutation; executable CCM input must use the decoded source stream
  route. Source: SKProject `SKULLWIN/c_creature.cpp:2930-3212`
  (`DM2_PROCEED_CCM`). Verification: CCM regression 51/51, source-matrix
  alignment 7/7, `firestaff_dm2` build, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB no-raster overlay cleanup: removed the final synthetic
  icon-to-palette-colour helper and updated the data-free CSB viewport
  regression to require no draw when no verified object/creature raster is
  bound. This keeps compact dungeon decoding coverage without presenting
  coloured markers as game material. References: ReDMCSB `DUNVIEW.C F0115`
  and CSBWin `Viewport.cpp`. Verification: `test_m11_csb_leader_hand_no_dm1_fallback`,
  real-data first-frame material test 109/109, M11 build, and `git diff --check`.
- ✅ 2026-07-31 CSB detached viewport-spec cleanup: removed the unconsumed
  D0L/D0R F0111, D2C F0111, and D3C wall contract modules plus their private
  tests and audit rows. They decoded no original bytes and had no M11 caller;
  real door and wall material remains owned by the source-command compositor.
  References reviewed: ReDMCSB `DUNVIEW.C F0111/F0118/F0121/F0125/F0126/F0128`
  and CSBWin `Viewport.cpp`. Verification: M11 build, local-PC34 first-frame
  material test 109/109, and `git diff --check`.
- ✅ 2026-07-31 CSB detached side/depth viewport cleanup: removed the
  non-M11 D1L/D1R F0111 receipt, D2L2/D2R2 clipped-front receipt, and D3L2
  F0115 projectile contract, together with their isolated test targets. The
  active source-command compositor retains its own D3 door and real-data
  material routes. References reviewed: ReDMCSB `DUNVIEW.C F0111/F0115/F0122/F0123`
  and CSBWin `Viewport.cpp`. Verification: M11 build, local-PC34 first-frame
  material test 109/109, and `git diff --check`.
- ✅ 2026-07-31 DM2 CCM GDAT-field inference closure: boot no longer scans
  arbitrary `CREATURE_AI` fields and promotes the first decodable byte stream
  into a live CCM program. A creature without the source-owned program and
  operand record now performs no CCM action instead of rebuilding operands
  from reduced local fields. The fixture-only explicit import remains solely
  for decoder coverage. Source: SKProject `SKULLWIN/c_creature.cpp`
  `DM2_PROCEED_CCM` and `SKWIN/SkWinCore.cpp`
  `EXTENDED_LOAD_AI_DEFINITION`. Verification: production CCM field-probe
  gate, fixture CCM runtime bridge 84/84, `firestaff_dm2` build, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB V2.2 legacy swap cleanup: removed the retired raw-cell
  3×3 swap API, its no-draw compatibility renderer, and the probe/test targets
  that existed only to preserve that obsolete placeholder boundary. The active
  V2.2 route remains the F0128 command/provenance gate; unbound material stays
  on the original V1 page. References: ReDMCSB `DUNVIEW.C F0128` and CSBWin
  `Viewport.cpp`. Verification: M11 build, V2.2 in-place route 145/145,
  front-wall projection 82/82, in-place draw 66/66, and `git diff --check`.
- ✅ 2026-07-31 CSB detached backdrop/frame cleanup: removed orphaned D1L/D1R
  receipt declarations plus the unconsumed dual/second custom-backdrop, D1C
  F0115 frame, and D2L2/D2R2 door contract modules and their tests. None
  decoded source raster bytes or had an M11 consumer; the source-command
  compositor remains the viewport owner. References reviewed: ReDMCSB
  `DUNVIEW.C F0111/F0115` and CSBWin `Viewport.cpp`. Verification: M11 build,
  local-PC34 first-frame material test 109/109, and `git diff --check`.
- ✅ 2026-07-31 DM2 viewport GRAPHICSSET receipt closure: an invalid
  `UPDATE_GFXSET` selector or missing command hash no longer becomes graphics
  set zero. It clears the active material and recalculated-light receipts, so
  a new scene cannot inherit pixels or light from the previous source
  transaction. Source: SKProject `SKWIN/SkWinCore.cpp::UPDATE_GFXSET`
  (14493-14525) and `RECALC_LIGHT_LEVEL` (5093-5131). Verification:
  `test_dm2_v1_scene_light_control`, real-data scene/wall-plan viewport
  probes, DM2 M11 viewport composition, `firestaff_dm2`, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 CSB V2.2 generic-painter cleanup: removed the obsolete 3×3
  `csb_v22_inplace_render_pass()` API and its synthetic-cache probes. That
  path could never identify the original raster, clipping or draw order, so
  it was not a valid material binding. V2.2 replacement is now exclusively
  command-bound: the F0128 command must carry the decoded original span,
  its material hash, live source palette and a provenance-verified route;
  otherwise V1 pixels remain untouched. References: ReDMCSB `DUNVIEW.C F0111`
  / `F0128` and CSBWin `Viewport.cpp`. Verification: focused V2.2 command
  tests, M11 build, real-data first-frame material test, and `git diff --check`.

- ✅ 2026-07-31 CSB V2.2 dead material-route API cleanup: removed the orphaned
  per-cell PBR routing declaration. Its stated implementation, tests and probe
  did not exist, it had no runtime consumer, and it described synthetic channel
  completion rather than original CSB material. The remaining V2.2 route is the
  live F0128 command/provenance gate. References: ReDMCSB `DUNVIEW.C F0111/F0128`
  and CSBWin `Viewport.cpp`. Verification: source-consumer audit, M11 build,
  real-data first-frame material test, and `git diff --check`.

- ✅ 2026-07-31 DM1 HoC C127 side/depth mirror-frame rendering: the F0107
  C127 path now selects its original C345/C346 backing before the ordinary
  map-local ornament lookup across all source-defined D1-D3 slots. C026
  remains limited to the D1C portrait overlay, so distant/side mirror frames
  appear without a floating champion portrait. Verification: real PC34
   `m11_dm1_hoc_real_mirror_viewport_material`, side/depth material receipt,
   and champion-mirror contract tests pass; source reference ReDMCSB
   `DUNVIEW.C F0107` / Graphic558's 15 projection coordinates.

- ✅ 2026-07-31 CSB V2 dead-hook cleanup: removed the two unreferenced Phase-0
  headers that supplied only no-op viewport, lighting, chaos, minimap and asset
  hooks. No M11/M12 or CSB runtime source included them, so they could not bind
  original data and only preserved a false implementation surface. The active
  V2 route remains the source-gated presentation and F0128 command path.
   References: ReDMCSB `DUNVIEW.C F0111/F0128` and CSBWin `Viewport.cpp`.
   Verification: whole-tree consumer audit, M11 build, real-data viewport test,
   and `git diff --check`.

- ✅ 2026-07-31 CSB V2.2 generic cell-cache API cleanup: removed the public
  3×3 cell bitmap and asset-id lookups. They had no M11 caller and could not
  carry F0128 source identity, clip, palette or draw order. The remaining cache
  lookup is internal to the admitted, source-bound F0128 command path.
  Verification: `test_csb_v22_inplace_draw_pc34` 57/57 and `git diff --check`.

- ✅ 2026-07-31 CSB V2.2 generic-cell implementation cleanup: removed the
  retired 3×3 route-table implementation itself after its public API was
  retired. No disabled synthetic cache logic remains in the compositor source;
  cache access is exclusively command-bound F0128 material replacement.
  Verification: `test_csb_v22_inplace_draw_pc34` 57/57 and `git diff --check`.

- ✅ 2026-07-31 CSB entrance synthetic-door cleanup: removed the isolated
  palette-filled closed/opening-door fallback API and its pixel assertions.
  Entrance frames now require the decoded original C002/C003 door surfaces.
  Reference: ReDMCSB `ENTRANCE.C`; verification: entrance composite test and
  `git diff --check`.
# CSB Amiga startup fallback removed (2026-08-07)

- ✅ Selected A31/A35 packages no longer enter the unrelated PC3.4
  `TITLE.C`/`ENTRANCE.C` startup session. The native `TITL.DAT`/APPA.C chain
  is left fail-closed until its application handoff is source-bound, so M11
  cannot present PC title, entrance, HUD or viewport pixels as Amiga output.
- ✅ The real A31 launcher boundary now first materializes the selected
  nested 7z→ADF package and verifies its selected `GRAPHICS.DAT` identity
  plus the original `DUNGEON.DAT` pair. A fail-closed M11 result can no
  longer hide a missing or cross-bound package core.

# CSB PC34 launcher regression isolation (2026-08-07)

- ✅ The real-media M12→M11 PC34 startup and V2 handoff probes now select the
  hash-verified `pc34-en` row explicitly. A persisted user preference for
  Atari ST, Amiga or FM Towns can no longer redirect the PC34 TITLE.C F0437 /
  ENTRANCE.C F0806 assertions or cause a null-session crash. Native-platform
  probes keep their own explicit selections.
# CSB Atari ST selected-package startup repair (2026-08-07)

- ✅ Fixed the mixed-root direct-launch regression: a selected Atari ST
  package now materializes `GRAPHICS.DAT` and its paired `DUNGEON.DAT` from
  the same original disk image, rather than inheriting the generic cache's
  FM Towns dungeon receipt.
- ✅ Nested archive-disk sibling resolution now preserves the outer archive
  path exactly, so `archive.7z::disk.msa::GRAPHICS.DAT` resolves its sibling
  as `archive.7z::disk.msa::DUNGEON.DAT`.
- ✅ The M11 Atari path has a decoded-only source cache for authenticated ST
  graphics. It cannot invoke the incompatible PC `GRAPHICS.DAT` parser.
  Real `ANIMATE.SCR` reaches FTLCODE through direct `--platform atari-st`
  boot probe with the verified `ebf6…`/`6695…` pair.

# CSB Atari ST original graphics runtime capture (2026-08-07)

- ✅ Replaced the incorrect DM1 LZW reader in the Atari DMCSB1 item loader
  with CSB's `Graphics.cpp`-compatible LZW/RLE decoder. The authentic stream
  now reaches the big-endian IMAGE1/IMAGE2 decoder rather than the PC3.4
  IMG3 path; C001–C005, C232 HUD and 022e viewport material therefore retain
  original source ownership.
- ✅ Bound the startup raster decoder to that source route and retained a
  record receipt for each decoded surface. The selected Atari ST launcher
  handoff now passes 1,498 checks, including `ANIMATE.SCR`, both SND1 streams,
  FTLCODE VBlank handoff and the first HUD/viewport frame, with no PC chrome
  fallback.
- ✅ Registered the scanner-supported ST 2.0/2.1 English GRAPHICS.DAT SHA-256
  (`7cceef26…`) alongside the older real-data fixture. The real-asset probe
  accepts either documented original fingerprint and passes 54/54 checks on
  the selected ST corpus.

# CSB FM Towns direct-launch package selection (2026-08-07)

- ✅ Direct CSB scans now materialize the same authenticated package that
  owns the required `GRAPHICS.DAT` receipt. A mixed CSB data root therefore
  no longer lets an earlier Amiga/Atari catalogue row make verified FM Towns
  F31E/F31J data unavailable.
- ✅ Verified with the supplied archive corpus: `firestaff --data-dir
  ~/.firestaff/data/csb --game csb --platform fm-towns --boot-probe` reaches
  the FM Towns cache and source-locked boot handoff; the real TITLE.ANM →
  SWITCHTW → CHTWE.EXP test also passes.

# CSB selected-package cache handoff (2026-08-07)

# CSB Atari ST runtime fail-closed session boundary (2026-08-07)

- ✅ Atari ST's ANIM.C→FTLCODE route has no PC3.4 TITLE.C/C017 runtime
  session. `m11_csb_live_hud_session_ready` now rejects that absent receipt
  before any PC34 terminal access, so a failed Atari C232/F0128 presentation
  remains a blank source rejection rather than dereferencing a null session.
- ✅ Verification: default `csb_v1_m11_launcher_handoff_boundary` passes
  with dummy audio; the supplied Atari archive no longer crashes at the
  PC34-session boundary. Its remaining C232/F0128 delivery failure remains
  explicitly open in TODO.

# CSB FM Towns cross-volume CD cache (2026-08-06)

- ✅ Materializing verified F31 English/Japanese ZIP media no longer fails
  after successful extraction when the user-selected data directory and the
  Firestaff cache are on different mounted volumes.
- ✅ The cache retains the original `FMTOWNS.IMG` and matching CUE sheet for
  source-owned CDDA, then the real FM Towns title, SWITCHTW, Prison door,
  HUD and viewport handoff completes from that cache.
- ✅ Verified against the supplied FM Towns original ZIP: `--scan-data`
  reports CSB READY, and the original-media F31 English handoff regression
  passes without unpacking the archive by hand.

- ✅ Extracted the supplied retail US archive through its original CUE layout
  and verified the assembled Track 02 ISO against the authenticated
  `ceb02343868f80cec899e9b239aff2da` identity.
- ✅ The instrumented Mednafen receipt records the correct System Card and
  Track 02 provenance, while keeping startup animation/menu/later-level
  promotion closed because no game-owned CD read occurred in the available
  headless capture (`non_system_card_pcecd_reads=0`).
- ✅ No synthetic animation frame, input route, or later-level decoder was
  promoted from this incomplete capture.

# CSB complete cross-platform data inventory (2026-08-06)

- ✅ The hash scanner now treats the recommended fast candidates as launch
  shortcuts only, never as a reason to stop inventorying a data root.
- ✅ A shared CSB corpus now reports its independently verified Atari ST
  package, including nested `.7z` disk media, alongside cache, Amiga and FM
  Towns matches. The real Atari archive regression materializes the original
  launch pair and each authenticated startup/utility sidecar from that root.
- ✅ PC 3.4 is no longer falsely reported when the matching `GRAPHICS.DAT`
  exists only in an Amiga 3.1 package. The scanner verifies the A31E
  `TITL.DAT` companion hash for every shared-payload candidate and preserves a
  separate PC package only when its provenance is distinct.
- ✅ The selected CSB archive now always crosses M12→M11 through a
  version-private runtime cache, even when it was the scanner's first match.
  The real A31M archive handoff retains `CSB_V1_VARIANT_AMIGA31_MULTI` and
  the `csb-amiga31-multi` cache root rather than silently booting generic
  PC34 bytes.
- ✅ The shared CSB launcher regression now treats an unavailable PC-only V2
  corpus as a skip, rather than failing an independent real A31E handoff
  because a materialized PC receipt happened to exist elsewhere on disk.

# CSB Amiga 3.1 multilingual program receipt (2026-08-07)

- ✅ Corrected the original EN/FR/DE ADF identity from A31E to A31M. ReDMCSB
  `COMPILE.H` maps A31M's `APPB.FTL` to the language selector and `KAOS.FTL`
  to the game executable, matching the verified Greatstone package.
- ✅ M12 now materializes `ANIM`, `APPA`, `APPB`, `BJELoad_R`, `CNFG`, `GRF1`,
  `MEM1`, `USIO` and `VDEO` only after their original ADF MD5 identities
  match. The real 7z → ADF handoff regression checks all nine members, the
  graphics/dungeon pair, and M11's fail-closed platform boundary.

- ✅ TQTR containers with an extended declared VRAM segment now advance to the
  declared VCE offset before loading the palette snapshot. The previous path
  read VCE immediately after the 64 KiB Firestaff slice and could admit a
  shifted palette from a valid larger capture container.
- ✅ Added a container regression with extended VRAM/VCE spans and a real
  BGR333 palette check. Theron launcher, direct-start, rendering, real VDC/VCE,
  HuC6280 disassembly and production-source-boundary tests remain green.

# CSB FM Towns C06 Utility palette material (2026-08-06)

- ✅ C06 now exposes the exact 16-entry F31 `C09_ICON` palette as native
  six-bit RGB, with no PC3.4 or host-colour substitution. ReDMCSB
  `CEDT018.C:829-838` supplies the clear/black/apply/restore order and
  `CEDT027.C:45-62` supplies the entries.
- ✅ The real F31E and F31J handoff regressions verify the original C06 P3
  envelope, menu bytes, input rectangles and palette material. The F31E
  editor now consumes this palette in M11; F31J remains gated pending its
  native Shift-JIS glyph consumer.

# CSB FM Towns C06 English editor frame (2026-08-06)

- ✅ The English SWITCHTW Utility action now enters an M11 rendering path for
  the original empty C06 editor. It uses authenticated `UTILE.EXP` strings,
  the raw M653 font, C09_ICON palette and the exact `CEDT006.C` F7030/F7034/
  F7042 box, button and swatch order. No host text or inferred layout is used.
- ✅ C06 QUIT returns through the original English AUTOEXEC loop and observes
  SWITCH.C's 60-VBlank delay. The F31J utility screen and all file-picker,
  save and portrait-edit transactions remain deliberately closed pending
  their native text and state consumers.
- ✅ The source CEDT006 palette selector now follows F7043/F7036: every
  C09_ICON row moves the white selected-swatch exterior while retaining the
  indexed source colour. This state is local to the editor and cannot alter a
  portrait, champion or save.

- ✅ M11 no longer renders authentic Track 02 font tiles through unverified
  host render-plan coordinates or inferred palette styles.
- ✅ Startup input and phase transitions remain available; visible startup
  text now requires the same captured VDC/VCE presentation route as bitmap
  art, preventing a synthetic menu from appearing over real media.

# CSB Amiga runtime sound-table admission (2026-08-06)

- ✅ M11 now consumes those real Amiga PCM payloads for an Amiga CSB session.
  It preserves the signed sample bytes and the ReDMCSB `SOUND.C` F0709 period
  path (`72800 / SOUND_DATA.Period`) before resampling solely for the host
  device. The PC3.4 PIT/SND3/marker route is not used for an admitted Amiga
  payload. Verification: dedicated transport regression, the 76-check
  original-`GRAPHICS.DAT` audio suite with the Amiga 3.3 corpus, and the
  unchanged PC3.4 transport regression. Voice allocation and stereo-volume
  capture remain explicitly open; no game data was copied or committed.

- ✅ Added the separate ReDMCSB Amiga `SOUND_DATA` table and a fail-closed
  `GRAPHICS.DAT` payload loader. It preserves the Amiga Graphic 671–712
  routing and hardware period, including the entries where Amiga differs from
  PC3.4. The local Amiga 3.3 switch record is admitted as 130 original PCM
  bytes at period 112. Verification: focused audio test, 76 passed checks.
  No game data was copied or committed.

- ✅ Removed the remaining production path that could paint host ASCII glyphs
  through the obsolete flat FONT256 bridge. The data-free layout probe keeps
  its explicit fixture raster lane; production now returns no-draw until the
  Saturn page/tilemap/attribute and placement consumer is authenticated.

# CSB Amiga GRAPHICS.DAT sound payload correction (2026-08-06)

- ✅ Corrected the Amiga CSB sound boundary to follow ReDMCSB `SOUND.C`
  F1051/F0709: playback length comes from the selected `GRAPHICS.DAT` item
  table and the PCM view begins two bytes into that direct-loaded record. The
  leading bytes are no longer misread as a synthetic PCM length. The new
  container-to-view gate admits only an Amiga DMCSB2 item with matching
  compressed/decompressed length and preserves its original sample bytes.
  Verification: the focused audio suite passed 74 checks, the F0060 suite
  passed, and the original Amiga 3.3 Graphic 672 switch record produced the
  source-derived 130-byte view. No game data was copied or committed.

- ✅ Corrected the Nexus sound runtime diagnostic so an unbound Saturn
  Red-Book track is reported as `selection-only` rather than as a missing host
  audio file. The real 16-level SAL/MAP corpus remains metadata-only and
  SFX/CDDA playback stays gated. Verification: `test_nexus_v1_sound_runtime_receipt`
  and `test_nexus_v1_sound_gameplay`; no game data was copied or committed.

# CSB FM Towns graphics real-cache regression (2026-08-06)

- ✅ `test_csb_v1_fmtowns_graphics_dat` now takes
  `FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR`, the scanner-materialized F31 runtime
  root, before considering the legacy private data layout. It verifies both
  `CDATA` and `CJDATA` on the real cache, including C695/M653's 768-byte raw
  `NOT_EXPANDED` record. English and Japanese cache runs pass 40/40 checks.

# CSB FM Towns C06 text metrics (2026-08-06)

- ✅ Added the separate F31E `CEDTTXT.C F7338_` renderer contract for C06:
  printable-ASCII-minus-`0x20`, five source pixels, six-pixel advance and
  baseline placement. It consumes only the admitted raw C695/M653 bitmap and
  rejects Shift-JIS, which remains owned by the unbound
  `F0952_JAPANESE_Print` path. The FM Towns graphics regression pins those
  metrics and passes 40/40 checks against both real cache roots.

# CSB FM Towns C06 portrait planar conversion (2026-08-06)

- ✅ Corrected the `.CMP` portrait decoder from an invented packed-nibble
  interpretation to the actual F31 `PORTRAIT.C F7251` Atari ST four-plane
  conversion, including F31's swapped chunky-nibble order from
  `CEDTINCO.C F7276`. A bitplane fixture locks palette values 5/10, and all
  24 supplied portraits pass against each English and Japanese cache.

- ✅ 2026-08-06 CSB FM Towns M653 font material: F31 `GRAPHICS.DAT` record
  C695 is now copied as its original 768-byte, one-bit `NOT_EXPANDED` font
  payload. M11 binds it before the real FM Towns title plays for both English
  and Japanese data, rather than misclassifying it as an IMG2 image. Evidence:
  ReDMCSB `DEFS.H M653_GRAPHIC_FONT` and `TEXT.C:2019-2022`. Verification:
  `test_csb_v1_fmtowns_graphics_dat` plus real-cache English/Japanese
  `test_csb_v1_fmtowns_m11_game_handoff`.

- ✅ 2026-08-06 CSB FM Towns C06 menu input: retained the original F31E/F31J
  `CEDTDATA.C G2272_MouseInputs` rectangles for Load Champions, Save
  Champions, Make New Adventure, Revert, Undo and Quit. The new source-space
  decoder preserves inclusive 320×200 boundaries and passes the actual C06
  command ordinal to a future owner; it does not substitute the generic PC34
  utility flow. ReDMCSB `CEDT006.C` lines 1401–1529 is the dispatch evidence.
  Verification: real-cache English and Japanese handoff tests plus the FM
  Towns boot-profile smoke test.

- ✅ 2026-08-06 CSB FM Towns CDDA pause/continue: the dedicated source CDDA
  stream now preserves its queued raw CUE/IMG span while CSB's F31 music
  switch is off, matching ReDMCSB `MUSIC.C` F0740/F0738
  `cdr_pause`/`cdr_continue`. Paused time no longer consumes the M11
  source-duration counter, and F0719's later source track request resumes a
  paused SDL device before it replaces the span. No decoded replacement or
  PC music fallback is introduced. Verification: `cmake --build build
  --target firestaff`, the CSB FM Towns ANM/CD probes (skip-safe without the
  materialized CUE/IMG pair), and `git diff --check`.

- ✅ 2026-08-06 CSB FM Towns ending handoff: a verified F31E/F31J Game
  victory now enters the original `ENDING.ANM` through the existing F2275
  decoder instead of falling through to the PC34 endgame surface. The M11
  bridge follows ReDMCSB `STARTUP2.C F0750`: game music pauses, `ENDING.ANM`
  owns its Timer-A frames and CDDA TD/TR requests, and completion retains the
  final original frame until the normal host return-to-launcher teardown.
  It never chains a win into `SWITCHTW.EXP`. Verification: real-cache
  `test_csb_v1_fmtowns_anm` (419 frames/5 352 Timer-A ticks) and
  `test_csb_v1_fmtowns_m11_game_handoff`, plus `cmake --build build --target
  firestaff` and `git diff --check`.

- ✅ 2026-08-06 CSB scanner edition inventory: `firestaff --scan-data` now
  lists every hash-verified CSB edition alongside the required launch pair.
  This makes independently admitted FM Towns English and Japanese CDATA/CJDATA
  payloads visible even if another original edition owns the selected cache.
  The report retains each archive/member provenance and does not promote
  optional source media into a launch requirement. Verification: a real CSB
  data-root scan reports Amiga 3.1 plus both FM Towns editions;
  `cmake --build build --target firestaff` and
  `git diff --check` pass.

- ✅ DM1 FM Towns no longer borrows the PC34 startup presentation. A selected
  `fmtowns-en` or `fmtowns-ja` edition is routed around the PC34
  `SWSH -> TITLE -> ENTRANCE` transaction and opens only its selected,
  hash-verified Towns payload. This is intentionally fail-closed for native
  presentation: `EDM.EXP`/`JDM.EXP` remain the sole owners of title animation
  and `TMENU.EXP` of the menu until their P3/TBIOS rendering, timing and input
  are decoded. Verification: `test_dm1_v1_startup_intro_state_machine_gate`.

- ✅ DM1 FM Towns discovery now admits the real `Dungeon-Master_FM-Towns_JA-EN.zip`
  only after the original MODE1/2048 ISO yields the registered English or
  Japanese `GRAPHICS.DAT` + `DUNGEON.DAT` MD5 pair. Required-file rows preserve
  archive/member provenance instead of relying on filenames. The selected FM
  Towns version can be materialized into a private cache with the real
  `EDM.EXP`/`JDM.EXP`, `TMENU.EXP`, `TMENU.ICN` and language data; the menu does
  not silently fall back to PC34 startup media. Verification:
  `test_firestaff_fmtowns_disc`, all five FM Towns DM1 unit tests, direct
  archive scan, and a DM1 boot probe with the real FM Towns archive. FM Towns
  executable animation/menu decoding and original capture remain tracked in
  `DM1-FMTOWNS-STARTUP-ANIMATION-MENU`.

- ✅ 2026-08-06 CSB FM Towns CDDA runtime transport: added a bounded
  file-backed allocator for a selected original CUE track. It validates raw
  2352-byte sectors and returns the exact 44.1 kHz signed-LE stereo bytes to
  the caller, including the source image end for track 31. No PCM is made or
  substituted. The real Victor CUE/IMG regression now verifies track 2's
  CUE-derived byte span both through the streaming extractor and runtime
  allocator. M11 binding remains tracked separately.

- ✅ 2026-08-06 DM1 FM Towns executable-title compositor: added a bounded
  `dm1_v1_fmtowns_title` frame composer for the HMA-240 EDM.EXP
  `DO_TITLE_ANIMATION` plan. It consumes only decoded original
  `GRAPHICS.DAT` graphic 1 and the verified executable receipt, producing
  PRESENTS at y=90, the 18 reverse zoom frames from 48x12 at (136,74) to
  320x80 at (0,40), then TITLE_MASTER at y=118. The geometry comes from the
  verified P3 load image (`EDM.EXP + 0xc3d1..0xc726`): SI/BX begin at 320/80,
  decrement by 16/4 for 18 prepared bitmaps, and the source/destination
  rectangles are checked by `dm1_v1_fmtowns_startup_receipt`. This is a
  data-owned compositor, not the PC34 TITLE.DAT/C001 path. M11 timing,
  CD-track-2 playback and TMENU interaction remain explicitly open.
  Verification: `test_dm1_v1_fmtowns_title`.

- ✅ 2026-08-06 DM1 FM Towns M11 title-order fix: the production title loop
  now uses that same EDM.EXP receipt-bound compositor for every frame. It
  therefore keeps PRESENTS visible, starts at the original centred 48x12
  bitmap and grows by 16x4 through the 18 prepared frames before drawing
  TITLE_MASTER. This removes the previous opposite-direction M11-only zoom;
  title timing, CDDA and TMENU input are still separately capture-gated.
  Verification: `test_dm1_v1_fmtowns_title`, `cmake --build build --target
  firestaff` and `git diff --check`.

- ✅ 2026-08-06 DM1 FM Towns PRESENTS/VBlank title handoff: M11 now keeps
  EDM.EXP's initial PRESENTS page separate from the first 48x12 zoom bitmap,
  then advances the native 18-frame zoom at the source's 60 Hz VBlank cadence
  and retains the two final VBlank waits before returning. The evidence is the
  real English `EDM.EXP` P3 sequence at `DO_TITLE_ANIMATION +0xc3f0`,
  `+0xc563` and `+0xc5b9`; no invented title hold or PC34 frame is used.
  Verification: `test_dm1_v1_fmtowns_title`, real-cache
  `test_dm1_v1_fmtowns_startup`, `cmake --build build --target firestaff` and
  `git diff --check`.

- ✅ 2026-08-06 DM1 FM Towns title CDDA handoff: M11 now starts the original
  CUE/BIN track 02 at the EDM title boundary, before the first source-owned
  frame is shown. The exposed game-view entry point rejects non-FM Towns,
  disabled-music and out-of-range calls, and never substitutes PC `SONG.DAT`.
  The normal map dispatcher remains responsible for changing track after
  title handoff. Verification: `test_dm1_v1_fmtowns_title`,
  `test_dm1_v1_fmtowns_cd_audio`, `cmake --build build --target firestaff`
  and `git diff --check`.

- ✅ 2026-08-06 DM1 FM Towns TMENU record parser: startup admission now
  structurally parses the original 256-byte `TMENU.INF` as its two TownsOS
  launch records. It verifies the fixed 128-byte record boundaries and their
  actual executable/path pairs, `JDM     .EXP` / `\\JDM.EXP` followed by
  `EDM     .EXP` / `\\EDM.EXP`, while retaining the title bytes without
  inventing a host glyph conversion. The old loose substring check is gone.
  Verification: real HMA-240 cache through `test_dm1_v1_fmtowns_startup`,
  `test_dm1_v1_fmtowns_title`, `cmake --build build --target firestaff` and
  `git diff --check`.

- ✅ DM1 FM Towns startup-owner gate: added a source-bound receipt for the
  real HMA-240 root startup chain. It verifies `AUTOEXEC.BAT`, the selected
  English `EDM.EXP` or Japanese `JDM.EXP` Phar Lap P3 owner, `TMENU.EXP`,
  `TMENU.ICN` and `TMENU.INF` by MD5 and records the original title (track 2),
  Hall of Champions (track 3) and entrance (track 5) CD-audio owners. M12 now
  rejects a materialized FM Towns cache if that native startup set is missing,
  mixed or altered. Verification: `dm1_v1_fmtowns_startup` against the real
  cache, the five existing DM1 FM Towns tests, and the real-archive DM1 boot
  probe. This does not claim P3/TBIOS pixel decoding; that remains TODO.

- ✅ DM1 FM Towns native owner-symbol gate: the startup receipt now checks the
  real `TMENU.EXP` TownsOS/file-browser bindings, confirms that `TMENU.INF`
  selects the requested `EDM.EXP` or `JDM.EXP`, and requires the original game
  executable's title-animation, title, menu, dungeon, 3D-graphics and CD-song
  owner symbols. This prevents a hash-correct but mismatched startup set from
  being presented as a native FM Towns boot. Verification: the real English
  HMA-240 cache passes `dm1_v1_fmtowns_startup`; no P3 instruction or TBIOS
  pixel decoding is claimed.

- ✅ DM1 FM Towns Phar Lap P3 envelope gate: added a bounded parser for the
  documented level-1 P3 header and validated the real EDM/TMENU metadata,
  including header/runtime/load-image bounds, symbol-table bounds, memory
  requirement and initial EIP. The receipt records EDM's actual `0x200` load
  image offset, `0x46941` load image size and `0x42a48` initial EIP from the
  original executable instead of treating the P3 file as an opaque blob.
  Verification: the real English cache passes `dm1_v1_fmtowns_startup` and the
  six-test FM Towns regression set. Instruction and TBIOS pixel decoding remain
  explicitly open.

- ✅ Disabled the synthetic English combat-log overlay for authenticated DM1
  source sessions. The real PC34 `TEXT.C`/C015 message lane remains visible;
  the diagnostic overlay is still available for non-source diagnostic worlds.
  Verification: `test_dm1_v1_combat_log_pc34_compat` (10 tests), source-name
  guards, real object names and backed original-save roundtrip all pass.

- ✅ Removed the launcher Item Encyclopedia's invented cross-game names,
  descriptions, weights, attack values and defense values. It now exposes the
  exact DM1 PC34 subtype names used by the ReDMCSB/M11 object consumer and
  labels numeric properties as `PC34 SOURCE` until an authenticated live
  DUNGEON.DAT owner is attached. No generated item facts remain in this view.
- ✅ Replaced the launcher Bestiary's approximate HP ranges, invented
  weaknesses/lore and generated art indices with the exact ReDMCSB PC34
  G0243 creature identities/base-health records C00-C26. Uncaptured attack,
  placement, weakness and pixel ownership remains explicitly unavailable.
- ✅ Removed the unused legacy creature renderer's approximate sprite-index
  table, guessed distance rectangles and synthetic health-bar draw. Its API
  now fails closed; active DM1 creature presentation remains owned by the
  authenticated M11 group/GRAPHICS.DAT consumer.
- ✅ Re-audited the active DM1 V1 M11 renderer against the real PC34
  `GRAPHICS.DAT`/`DUNGEON.DAT` path and the `DM1-ORIGINAL-REPLACE-003` through
  `DM1-ORIGINAL-REPLACE-026` inventory. No open production placeholder was
  found: source sessions either consume an authenticated decoded surface or
  fail closed/no-draw. The remaining DM1 TODO entries are original-save
  corpus breadth and external original/Mac capture, not an untracked
  synthetic renderer.
- ✅ Promoted three fresh Firestaff DM1 v1 runtime frames to the README from
  the real PC34 data and operator save route. These are Firestaff captures
  only; they do not count as original-vs-Firestaff pixel parity evidence.

# ✅ 2026-07-11 CSB CSBWin runtime-resume transaction: `csb_v1_runtime_apply_csbwin_resume_report()` now validates every declared CHARDESC and timer-queue reference, stages GAMEBLOCK2, champion, ITEM16 and timer handoff in a candidate profile, then publishes it only on complete success. A rejected section reference preserves both the live profile and the shared dungeon level. Source lock: CSBWin `SaveGame.cpp` lines 1768-1867. Verification: targeted Ninja build and `test_csb_v1_runtime_tick_accumulator`, plus focused CTest.

# ✅ 2026-07-14 CSB PC34 first-opening-door package capture

The staged-data M12-to-M11 launcher boundary now captures the first visible
CSB entrance-opening frame and verifies its real C002 and C003 bytes directly
in the presented 320x200 raster: C002 `[0..100]` lands at `(0,30)` and C003
`[4..126]` lands at `(109,30)`, each for 161 rows. This locks the PC34
F0438/F0807 first-step crop and destination geometry without introducing a
synthetic surface. Source: ReDMCSB `ENTRANCE.C` F0438/F0807 lines 142-304;
CSBWin `Graphics.cpp::ReadGraphic` is the corresponding package-read boundary.
Verification: `test_csb_v1_m11_launcher_handoff_boundary` passes 327/327
against local hash-verified CSB data.

# ✅ 2026-07-14 CSB real title/HUD/door launcher visual capture

The real-data M12-to-M11 CSB launcher handoff regression now captures all
three source title phases alongside the existing entrance-door and terminal
HUD checks. It advances the live title through ReDMCSB `TITLE.C F0437`'s
CHAOS zoom/hold into STRIKES BACK, verifies the decoded `C001` C426 crop is
drawn byte-for-byte at `(0,118)` with C00 transparency, confirms the C426
palette, and requires the capture to differ from CHAOS. Existing checks retain
the real opening-door frame and exact C017/C040 HUD composition. Verification:
`test_csb_v1_m11_launcher_handoff_boundary` passed 289/289 against staged CSB
data. Source: ReDMCSB `TITLE.C F0437` lines 424-463 and `ENTRANCE.C F0806`
lines 775-826.

- ✅ 2026-07-14 DM1 HoC champion time-effects cadence fix: M11 now applies
  F0331 only every 64 active ticks or 16 resting ticks after game time advances.
  This prevents accelerated food/water loss and starvation death. Verification:
  `dm1_v1_champion_needs_pc34_compat_integration`.

# ✅ 2026-07-14 CSB terminal presented-frame runtime handoff

M11 now records the successfully presented 320x200 indexed CSB framebuffer
through a CSB-owned fact builder that requires the terminal real package
session: C001 PRESENTS/CHAOS/STRIKES, F0807 door completion, and C017/C040.
The boot receipt retains only the actual-frame hash, dimensions, and macOS
app/window facts. It remains fail-closed outside a valid terminal session and
does not promote an app capture without the existing release/app receipt.
Source lock: ReDMCSB `TITLE.C F0437` and `ENTRANCE.C F0438/F0807`; CSBWin
`Graphics.cpp ReadGraphic`. Verification: CMake build of `firestaff_m11` and
`git diff --check` passed. The broad `test_csb_v1_boot_runtime_handoff` still
has pre-existing failures in unrelated synthetic-fixture assertions.

# ✅ 2026-07-14 CSB PC package presentation probe

`csb_v1_pc_package_presentation` now drives one hash-verified PC34 CSB
package through the production session's C001 PRESENTS/CHAOS/STRIKES phases,
closed and opening C004+C002+C003 entrance-door composites, F0807 completion,
and C017/C040 HUD. The opt-in probe has no image, palette, or fallback
fixture: unavailable or non-PC34 media skips, and any non-package route fails.
It records the existing ReDMCSB TITLE.C/ENTRANCE.C and CSBWin indexed-graphics
contract boundary without promoting a custom CSBgraphics.dat override.

# ✅ 2026-07-14 CSB real-package credits consumption

The opt-in PC34 package-presentation probe now drives the production startup
session through ReDMCSB `ENTRANCE.C F0442/F0806`'s credits state and presents
decoded `C005` before the normal C004/C002/C003 door sequence and C017/C040
HUD handoff. It accepts one source surface only for credits and still skips
when the hash-verified local package is unavailable; no generated credits
screen, text fallback, or wrapper can satisfy the probe. Source boundary:
ReDMCSB `ENTRANCE.C F0442/F0806`; independent archive-read boundary: CSBWin
`Graphics.cpp::ReadGraphic`. Verification: focused Ninja build and
`csb_v1_pc_package_presentation` (skip-safe without local PC34 media).

# ✅ 2026-07-14 CSB package CHAOS hold consumption gate

The terminal PC34 package receipt now requires the complete four-phase
`TITLE.C F0437` playback mask and distinct C001 CHAOS zoom/hold consumption
facts before it can authorize the F0807 C017/C040 HUD handoff. The fixture-free
package probe advances an authenticated session through source step 21, the
full-size CHAOS hold, and samples its package-backed C001 surface. Terminal
session coverage rejects both an omitted hold phase and a missing hold receipt;
no title pixels, palettes, or fallback surfaces are generated. Source boundary:
ReDMCSB `TITLE.C F0437`; indexed asset ownership follows CSBWin
`Graphics.cpp::ReadGraphic`.

# ✅ 2026-07-14 CSB hash-receipted M12/M11 package startup gate

The real M12-to-M11 CSB launcher route now begins with the same hash-verified
PC34 `GRAPHICS.DAT` plus `DUNGEON.DAT` receipt used by the package scanner. It
does not promote an otherwise launchable directory: title C001, the C002/C003
door sequence, and terminal C017/C040 HUD assertions run only after the
receipt confirms both source files and the production session remains bound to
them. Missing or non-PC34 media is an explicit skip, with no generated art or
fallback surface. Source boundaries: ReDMCSB `TITLE.C` F0437 and `ENTRANCE.C`
F0806/F0807; CSBWin `Graphics.cpp::ReadGraphic`. Verification:
`test_csb_v1_m11_launcher_handoff_boundary`.

# ✅ 2026-07-14 CSB compact ParameterB timer integration coverage

The Phase 7 CSB runtime regression now materially exercises the existing
CSBWin compact `LocalState=2` `DB3::ParameterB` route: `ParameterB=4` selects
the authenticated action through the saved `TT_STONEROOM`, `TT_OPENROOM`, and
`TT_FALSEWALL` runners, while a widened high-bit value rejects before any DSA
dispatch. Source: CSBWin `data.cpp` `DB3::MakeBig`/`ParameterB` and `DSA.cpp`
`GetState`/`ProcessDSATimer6`. Verified by CTest
`csb_v1_phase7_verification` and `csb_v1_dsa_queued_localstate2_timer`.

# ✅ 2026-07-14 CSB real-package presented title, door and HUD captures

`test_csb_v1_m11_launcher_handoff_boundary` now records the actual M11
320x200 indexed framebuffer after each package-driven C001 PRESENTS, CHAOS,
and STRIKES BACK title phase, the closed and first-opening C002/C003 door
states, and terminal C017 HUD. Every capture is bound to the production
presentation receipt and its framebuffer hash, after existing source-byte and
geometry checks. That receipt validates the complete C001-C005/C017/C040
session so title and door frames can be recorded before the terminal HUD
phase. The opt-in route still accepts only the hash-verified PC34
`GRAPHICS.DAT` plus `DUNGEON.DAT` pair and does not create fallback art.
Source boundary: ReDMCSB `TITLE.C F0437`, `ENTRANCE.C F0438/F0807`, and
CSBWin `Graphics.cpp::ReadGraphic`.
## 2026-07-23 DM1 C13 F0435 corpus/runtime identity gate

The external-only PC34 corpus receipt now ties original C3 EVENT and C4
TIMELINE byte identities to the C13 state restored by F0435, then to the
candidate-to-runtime adoption. It additionally requires matching party,
GLOBAL_DATA/map, ACTIVE_GROUP, and runtime timeline fingerprints. The gate
does not change M11 and does not promote generated saves as corpus evidence.
Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus`.

## 2026-07-23 DM1 F0435 sensor/launcher save replay identity

Original PC34 C003/C004 floor sensors and C014-C018 wall launcher/endgame
sensor records now retain their indexed eight-byte tail records across F0435
staging and runtime adoption. Admission additionally fences the authenticated
C3 EVENT and C4 TIMELINE identities, runtime map state, and normalized
timeline. Missing or drifting original bytes revoke the receipt; positive
evidence remains external-corpus only. Verification:
`dm1_v1_original_save_pc34_handoff` and
`dm1_v1_original_save_pc34_external_corpus`.

## 2026-07-23 DM1 F0435 C000-C002 world layout adoption

Original PC34 map header, raw thing-list slots, G0280/SquareFirstThings and
GLOBAL_DATA now cross F0435 staging/adoption as one source-owned receipt. The
fence includes exact tail roundtrip bytes, raw table fingerprints, map/time,
and C3/C4 timeline identity. Any pointer, slot, map, time or raw-tail drift
revokes admission; positive evidence remains external-corpus only.
Verification: `dm1_v1_original_save_pc34_handoff` and
`dm1_v1_original_save_pc34_external_corpus`.

- ✅ 2026-07-23 DM1 F0115 source-bound object/pile/projectile handoff:
  DM1-owned rendering input now admits only decoded source-owned PC34 pixels
  whose GRAPHICS.DAT index matches C2500/G0209 floor objects, F0142/G0209
  thrown objects, or C2900/M613 native projectiles. It preserves C10/F0791,
  source-zone placement, and pile offsets; missing, unowned, or mismatched
  material becomes no-draw. Verification:
  `dm1_v1_f0115_source_material_handoff_pc34_compat` passed.

- ✅ 2026-07-23 DM1 F0115 per-square scheduler admission:
  a verified object/pile/projectile handoff now converts to the existing
  F0128 scheduler input only when its source-owned pixels, graphic identity,
  C10/F0791 contract, geometry, and material fingerprint are still present.
  A no-draw handoff cannot enter the scheduler. Verification:
  `dm1_v1_f0115_source_material_handoff_pc34_compat` and
  `dm1_v1_f0115_square_material_scheduler_pc34_compat` passed.
- ✅ 2026-07-23 DM1 F0115 live C14/C15 material gate: viewport
  materialization now retains each active projectile/explosion receipt but
  promotes it to a renderable entry only after its exact graphic index matches
  a caller-verified, decoded PC34 `GRAPHICS.DAT` surface. Native C14 and C15
  fail closed without that material; associated-object C14 cannot borrow a
  native projectile surface. Verification:
  `dm1_v1_viewport_runtime_materialization_pc34_compat` passed.

## 2026-07-23 DM1 C13 F0435 stale/revocation fence

External PC34 C13 admission now has a source/runtime-only stale fence between
F0435 staging/adoption and later presentation. It revokes on provenance,
timeline, active-group, GLOBAL_DATA/map, or F0238 queue drift, and does not
touch M11 or F0134/F0115. Positive admission remains restricted to an
operator-supplied original corpus. Verification: `dm1_v1_original_save_pc34_handoff`
and `dm1_v1_original_save_pc34_external_corpus`.
- ✅ 2026-07-23 DM1 F0248/F0213 source-owned launcher lifecycle: all DM1
  wall-launcher families now reserve, link, and bind a raw PC34 C14 before
  publishing their first C48/C49 event; a loaded world with no authentic C14
  slot fails closed. The C49 index is written back to the exact raw owner.
  Loaded projectile impacts now similarly require a reserved C15 and live C25
  publication before creating an explosion runtime entry. Verification:
  `dm1_v1_f0248_explosion_launcher_runtime_pc34_compat`,
  `dm1_v1_f0248_new_object_launcher_runtime_pc34_compat`,
  `dm1_v1_f0248_square_object_launcher_runtime_pc34_compat`, and
  `dm1_v1_f0213_f0220_explosion_runtime_pc34_compat` passed.

## 2026-07-23 DM1 F0435 C03/C04 runtime adoption identity

The original-save corpus receipt now propagates the authenticated C03 EVENT
and C04 TIMELINE raw byte identities through F0435 staging and the
candidate-to-runtime adoption. It independently checks their counts,
fingerprints, runtime event count, and normalized timeline identity. No M11
path is changed; positive proof remains external-corpus only. Verification:
`dm1_v1_original_save_pc34_handoff` and
`dm1_v1_original_save_pc34_external_corpus`.

## 2026-07-23 DM1 F0110/F0112/F0113 wall/ornament/field material gate

The non-door viewport lanes now produce source-only receipts for side-wall
backing, non-inscription wall ornaments, and visible/open C05 teleporter
fields. Each binds its decoded PC34 `GRAPHICS.DAT` pixels, raw hash-checked
`DUNGEON.DAT` byte provenance, ReDMCSB destination geometry, palette row and
draw phase; field admission also requires its real mask surface. Missing or
tampered material fails closed. No M11, F0111, F0114, door, C10/C11, or
inscription source changed. Verification with installed real PC34 data:
`dm1_v1_viewport_wall_field_original_material_gate`,
`dm1_v1_wall_ornament_pc34_compat`, and
`dm1_v1_field_teleporter_effect_pc34_compat` passed.

## 2026-07-23 DM1 F0115/F0219 D1-D3 creature/item material gate

The normal object and creature lanes now emit source-only D1-D3 receipts from
decoded PC34 `GRAPHICS.DAT` and a hash-checked raw `DUNGEON.DAT` corridor
byte. The receipts preserve C00..C03 cell ownership, full decoded-source crop,
C2500 scale/pile shift geometry, C10 transparency, and original D3/D2
creature palette maps. Tiny real item sprites remain valid when ReDMCSB's
distance scale produces a one-pixel dimension. Missing, tampered, or
foreign-cell material fails closed. No M11, wall, ornament, field, F0111,
F0114, inscription, C14, or C15 route changed. Verification with real data:
`dm1_v1_f0115_f0219_creature_item_material_gate` and
`dm1_v1_f0115_source_material_handoff_pc34_compat` passed.

## 2026-07-23 DM1 F0115 D0/D1 near object and decoration material gate

The near-square source gate now admits only real PC34 `GRAPHICS.DAT` floor,
ceiling, F0108 floor-ornament, and C05..C10 normal-object surfaces with a
hash-checked raw `DUNGEON.DAT` corridor provenance. It retains source crop,
destination geometry, identity palette, F0098/F0108/F0115 draw order, and
C00..C03 normal-object cell ownership. C14/C15, walls, doors, fields, M11,
and the completed D1-D3 creature/item lane are excluded. Missing, foreign, or
tampered source material fails closed. Verification:
`dm1_v1_f0115_near_object_decoration_material_gate`.

## 2026-07-23 DM1 F0344/F0658 HUD source-material gate

F0344/F0658 now has a source-only material receipt for PC34 C010 action
surface crops owned by C079/C077/C011, C009/C011 spell rows, C020 panel,
C030/C031/C032 labels, and the raw 768-byte M653 glyph bitplane. The receipt
retains C00/C04 text roles, C12 label transparency, exact crops, zones, and
PANEL.C source rows. Missing, foreign, or tampered surfaces/glyphs fail
closed. No M11, viewport, or save route changed. Verification with installed
real PC34 data: `dm1_v1_f0344_f0658_hud_material_gate`.

## 2026-07-23 DM1 F0344/F0658 consumer repair

The F0344/F0345 consumer now preserves the previously admitted C020,
C05/C14/C08/C11, and action/spell source paths. HoC C040/C026 admission is
required only for HoC, not ordinary inventory; C020 blits as its opaque PC34
surface, and only C030/C031/C032 use C12 transparency. F0387 rejects any
action-plan destination that diverges from its original C079/C077/C011 box.
No synthetic panel, text, glyph, or action/spell fallback was added.

## 2026-07-23 DM1 champion-panel PC34 material admission

F0292/F0293/F0296/F0302 now require one real GRAPHICS.DAT admission for the
C008 status box, C017 inventory raster, C026 portrait atlas, C028 icons,
C032 poison, C015/C016 damage, C033-C035 hand slots, M653 glyph plane, and
the original 16-colour palette. The existing M11 top-row consumer clears the
source zones when any required material is absent or altered; it does not
fall back to a host font or procedural panel. Verification:
`dm1_v1_champion_panel_material_gate`.

## 2026-07-23 DM1 F0114-adjacent F0104 floor/pit/stairs material gate

Floor pits and stairs now produce source-only PC34 material receipts. Each
receipt binds the ReDMCSB plan's graphic index, source/destination geometry,
native palette map, and fingerprint of the decoded `GRAPHICS.DAT` indexed
surface. Missing, foreign, tampered, or out-of-bounds material fails closed.
No M11 game-view, door, C10, or C11 route changed. Verification with installed
real PC34 data: `dm1_v1_floor_pit_pc34_compat`,
`dm1_v1_stairs_render_pc34_compat`, and
`dm1_v1_floor_pit_stairs_original_material_gate` passed.
- ✅ 2026-07-23 DM1 F0248/F0810 C14/C15 live-effect material receipts: added
  a DM1-only source gate binding each projectile/explosion render/save receipt
  to the exact raw PC34 `DUNGEON.DAT` C14/C15 row, decoded object identity,
  `GRAPHICS.DAT` indexed pixels and original 16-colour palette. Raw/decoded
  drift, absent palette, or unowned pixels remains no-draw; no synthetic
  material route is admitted. Verification:
  `dm1_v1_f0115_source_material_handoff_pc34_compat`.
- ✅ 2026-07-23 DM1 F0810/F0811 source-bound throw/replay lifecycle:
  F0328/F0810 receives an explicit receipt for the loaded raw PC34 carried
  object, including the full source input and raw-object fingerprints.
  Original-save C48/C49 replay now binds and fingerprints the exact raw C14
  record before an F0811 movement entry may materialize. Missing raw objects,
  host-only ids, and raw/decoded drift fail closed; the C14/C15 material gate
  remains unchanged. Verification: `dm1_v1_throw_shoot_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff`.
- ✅ 2026-07-23 CSB F0245/F0248 C010/C018 replay identity: C010 launcher
  dispatch now requires an exact packed sensor-cell and the already-required
  live PC34 wall event, square and Thing-chain identity; C018 stays behind the
  same common wall-event gate while retaining F0731's source endgame-cell
  evaluator. Native save clock replay is covered by an MD5-gated original
  C010 probe. C011/M11 and all excluded event families are untouched.
  Verification: C010 save, C018 runtime, and original C010 replay tests pass.
- ✅ 2026-07-23 DM1 original-save ACTIVE_GROUP source-link fencing: raw C04
  active records now receive a separate F0435 stage/adoption receipt binding
  each live GROUP Thing through the restored current-map SquareFirstThing
  chain, saved position, packed cells/directions/aspects, global map and
  C03/C04 timeline identity. Any byte, SFT, Thing, map or timeline drift
  revokes the receipt. C000-C002, C29-C41, party/champion, and M11 are
  untouched. Verification: `dm1_v1_original_save_pc34_handoff` and
  `dm1_v1_original_save_pc34_external_corpus`.
- ✅ 2026-07-23 DM1 C14/C15 production graphics catalog: projectile and
  explosion materialization now requires the authenticated PC34 decoded
  catalog, pixel fingerprint, F0248/F0142/G0209 ownership receipt and native
  palette. Missing or drifted material is no-draw. Verification:
  `dm1_v1_viewport_runtime_materialization_pc34_compat`,
  `dm1_v1_f0115_source_material_handoff_pc34_compat`, and
  `m11_dm1_throw_projectile_runtime_materialization_pc34`.
- ✅ 2026-07-23 CSB DSA `STKOP_SetNewState`: forced state is now admitted
  only through an authenticated PC34 LocalState/tail receipt. Save or dungeon
  drift is rejected before dispatch. Verification: `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_save_runtime_admission_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge`.
- ✅ 2026-07-23 CSB DSA conditions and triggers: `AND`, `OR`, `NOT`, and
  conditional trigger dispatch now carry the authenticated restored PC34
  condition identity. Unknown owners or receipt drift are rejected.
- ✅ 2026-07-23 DM1 F0209 event runtime: C04/SFT/ACTIVE_GROUP admission now
  binds the source F0267 movement and F0179-to-F0208-to-F0238 timeline
  handoff. Missing or drifted PC34 world/timeline state is fail-closed.
- ✅ 2026-07-23 DM1 F0227/F0228 LoS and direction admission: live M10 group
  reactions now require raw C04, active-group/map, C29-C41 timeline and
  original-RNG preview identity before reaching F0209; drift is a no-op.
- ✅ 2026-07-23 DM1 HoC champion top row: the live C150-C218/F0287 path now
  consumes the authenticated PC34 party/status geometry with a real-data
  runtime probe; it records no synthetic positive evidence.
- ✅ 2026-07-23 CSBWin DSA MESSAGE/DESSAGE timer payload receipt: restored
  DSA scheduling now retains the source delay and switch action together with
  route, target and event type. Unknown owners and any payload drift reject
  before reuse. Verification: focused MESSAGE, PC34 save-handoff and restored
  timer regressions pass.
- ✅ 2026-07-23 CSB F0213-F0220 C15/F0115 fail-closed consumption: removed
  the host marker fallback from the F0115 explosion pass. Missing original
  material is no-draw while C15/C25 runtime ownership remains intact.
- ✅ 2026-07-23 CSBWin DSA arithmetic/bitwise save receipt: authenticated
  `STKOP` arithmetic now records `GLOBALSTORE`'s post-write PC34 EXPOOL hash
  in the restored-timer receipt. Stale save-tail identity, missing ownership,
  divide-by-zero, and stack over-/underflow fail closed. Verification:
  `csb_v1_dsa_trigger_single_step_pc34_compat`,
  `csb_v1_dsa_admitted_restored_timer_bridge`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` pass.
- ✅ 2026-07-23 CSBWin DSA Execute return/frame fault receipt: restored PC34
  timers now bind `DSA.cpp::Execute()`'s explicit return value, balanced
  GOSUB frame counts, and missing-program return boundary to the loaded
  save/DSA owner. Return-value or frame-balance drift fails closed;
  `EX_GOSUB` keeps its source behavior of ignoring its child return. Verification:
  `csb_v1_dsa_trigger_single_step_pc34_compat`,
  `csb_v1_dsa_admitted_restored_timer_bridge`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` pass.
- ✅ 2026-07-23 DM1 F1146-F1165 and CSB F0886-F0925 batch: fail-closed DM1
  I/O ownership plus source-gated CSB media/palette/swoosh primitives add no
  synthetic UI, graphics, timing, or actions. Verification:
  `dm1_v1_f1146_f1165_io_owner_audit`,
  `csb_v1_f0886_f0905_source_ownership_pc34_compat`, and
  `csb_v1_f0906_f0925_swoosh_primitive_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 F1086-F1105 batch: source-bound platform/input ownership
  leaves unsupported PC34 paths fail-closed without synthetic input, UI,
  graphics, or timing. Verification:
  `dm1_v1_f1086_f1105_platform_input_source_audit_pc34_compat`.

- ✅ 2026-07-23 DM1 F1006-F1025 and F1106-F1125 batch: source-bound PC34
  command/palette owners and fail-closed media/platform paths add no synthetic
  UI, graphics, timing, or actions. Verification:
  `dm1_v1_f1006_f1025_source_ownership_pc34_compat` and
  `dm1_v1_f1106_f1125_media_owner_audit`.

- ✅ 2026-07-23 CSB F0866-F0885 batch: source boundaries reject unsupported
  PC34 paths without synthetic graphics, UI, timing, or actions. Verification:
  `csb_v1_f0866_f0885_source_boundary_pc34_compat`.

- ✅ 2026-07-23 DM1 F1066-F1085 and CSB F0846-F0865 batch: source-bound
  supported ownership and explicit fail-closed Amiga/unmapped boundaries add
  no synthetic UI, graphics, timing, or actions. Verification:
  `dm1_v1_f1066_f1085_amiga_owner_audit` and
  `csb_v1_f0846_f0865_unmapped_boundary_pc34_compat`.

- ✅ 2026-07-23 DM1 F1046-F1065 and CSB F0826-F0845 batch: source-bound
  DM1 platform/save ownership and source-gated CSB boundaries leave all
  unavailable routes fail-closed without synthetic UI, graphics, timing, or
  actions. Verification:
  `dm1_v1_f1046_f1065_platform_save_source_audit_pc34_compat` and
  `csb_v1_f0826_f0845_source_boundary_pc34_compat`.

- ✅ 2026-07-23 DM1 F1026-F1045 batch: source-audited platform-owner
  boundaries leave unsupported PC34 paths fail-closed without synthetic
  platform behavior, graphics, UI, or timing. Verification:
  `dm1_v1_f1026_f1045_platform_owner_audit`.

- ✅ 2026-07-23 CSB F0806-F0825 batch: startup owner admission requires
  authenticated PC34 package material; missing/legacy paths fail closed
  without substitute startup UI, graphics, timing, or actions. Verification:
  `csb_v1_f0806_f0825_startup_source_admission_pc34_compat`.

- ✅ 2026-07-23 DM1 F0946-F1005/L0966-L0985 and CSB F0786-F0805 batch:
  source-bound DM1 ownership/provenance and source-gated CSB panel/layout
  contracts fail closed with no synthetic rendering, input, or presentation.
  Verification: `dm1_v1_f0946_f0965_source_ownership_pc34_compat`,
  `dm1_v1_l0966_l0985_champion_owner_audit`,
  `dm1_v1_f0986_f1005_graphics_platform_source_audit_pc34_compat`, and
  `csb_v1_f0786_f0805_panel_layout_raw_pc34_compat`.

- ✅ 2026-07-23 CSB F0766-F0785 batch: source-bound owners require
  authenticated PC34 package admission; missing/legacy material fails closed
  without substitute UI, graphics, timing, or actions. Verification:
  `csb_v1_f0766_f0785_source_admission_pc34_compat`.

- ✅ 2026-07-23 DM1 F0926-F0945 batch: source-bound platform/loader ownership
  leaves missing source bodies and host-only boundaries fail-closed without
  synthetic loading or presentation. Verification:
  `dm1_v1_f0926_f0945_platform_loader_source_audit_pc34_compat`.

- ✅ 2026-07-23 DM1 F0886-F0925 and CSB F0746-F0765 batch: source-bound DM1
  media/bitplane/palette/sound/primitive owners and source-gated CSB
  memory/language contracts fail closed without synthetic rendering or host
  behavior. Verification:
  `dm1_v1_f0886_f0905_source_ownership_pc34_compat`,
  `dm1_v1_f0906_f0925_pc34_owner_audit`, and
  `csb_v1_f0746_f0765_memory_language_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 P0866-P0885 and CSB F0706-F0725 batch: source-bound DM1
  parameter provenance and CSB authenticated package admission leave
  copy-protection/missing-package paths fail-closed. Verification:
  `dm1_v1_p0866_p0885_parameter_source_audit_pc34_compat` and
  `csb_v1_f0706_f0725_package_admission_pc34_compat`.

- ✅ 2026-07-23 DM1 F0826-F0865 batch: local-symbol references are bound to
  verified callable owners and unavailable PC34 owners are explicit
  fail-closed. Verification:
  `dm1_v1_f0826_f0845_local_symbol_boundary_pc34_compat` and
  `dm1_v1_f0846_f0865_pc34_owner_audit`.

- ✅ 2026-07-23 CSB F0726-F0745 batch: source-gated media/filename contracts
  reject unavailable material without fabricated files, media, or
  presentation. Verification:
  `csb_v1_f0726_f0745_media_filename_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 F0786-F0825 batch: runtime-panel/media/text ownership is
  source-bound to real PC34 material and unknown paths remain fail-closed.
  Verification:
  `dm1_v1_f0786_f0805_runtime_panel_source_audit_pc34_compat` and
  `dm1_v1_f0806_f0825_pc34_owner_audit`.

- ✅ 2026-07-23 DM1 F0726-F0745 batch: source-locked PC34 no-op and existing
  owner boundaries avoid fabricated mappings, input, graphics, or timing.
  Verification: `dm1_v1_f0726_f0745_source_ownership_pc34_compat`.

- ✅ 2026-07-23 DM1 F0766-F0785 batch: PC34 file/mouse ownership is
  source-audited and unknown paths stay fail-closed without host substitutes
  or synthetic input. Verification: `dm1_v1_f0766_f0785_pc34_owner_audit`.

- ✅ 2026-07-23 CSB F0666-F0705 batch: presentation/video/input contracts are
  source-gated on authenticated PC34 material and cannot render fallback
  screens or invoke fallback input. Verification:
  `csb_v1_f0666_f0685_presentation_material_pc34_compat` and
  `csb_v1_f0686_f0705_video_input_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 F0686-F0705 and F0746-F0765 batch: source-bound runtime
  graphics, memory, and I/O owners reject unproven material without invented
  visuals or host actions. Verification:
  `dm1_v1_f0686_f0705_runtime_graphics_source_audit_pc34_compat` and
  `dm1_v1_f0746_f0765_pc34_owner_audit`.

- ✅ 2026-07-23 DM1 F0541-F0560 and F0706-F0725 batch: PC34 platform and
  I/O/graphics owners are source-audited; Amiga/IIGS-only and unproven routes
  fail closed without substitute input, UI, or graphics. Verification:
  `dm1_v1_f0541_f0560_platform_boundary_pc34_compat` and
  `dm1_v1_f0706_f0725_pc34_owner_audit`.

- ✅ 2026-07-23 CSB F0646-F0665 batch: source-gated text/bitmap/palette/click
  contracts reject unavailable material rather than rendering substitute
  surfaces or invoking fallback input. Verification:
  `csb_v1_f0646_f0665_text_bitmap_click_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 F0666-F0685 batch: source-bound endgame/graphics owners
  reject unproven material without substitute graphics, text, or dialogs.
  Verification:
  `dm1_v1_f0666_f0685_endgame_graphics_source_audit_pc34_compat`.

- ✅ 2026-07-23 DM1 F0646-F0665 batch: source-bound text, timeline, bitmap,
  palette, and click owners retain fail-closed missing-material behavior.
  Verification:
  `dm1_v1_f0646_f0665_text_bitmap_palette_click_source_audit_pc34_compat`.

- ✅ 2026-07-23 DM1 F0621-F0645 and CSB F0600-F0620 batch: source-bound
  champion/layout and core-material owners require authenticated PC34 data;
  missing material cannot produce fallback UI, graphics, or actions.
  Verification: `dm1_v1_f0621_f0645_champion_layout_source_audit_pc34_compat`
  and `csb_v1_f0600_f0620_core_material_pc34_compat`.

- ✅ 2026-07-23 CSB F0621-F0645 batch: champion/layout/font/text ownership is
  source-gated on authenticated PC34 material; unavailable paths do not
  fabricate UI or text. Verification:
  `csb_v1_f0621_f0645_champion_layout_text_raw_pc34_compat`.

- ✅ 2026-07-23 DM1 F0600-F0620 batch: source-bound existing dialog,
  graphics-memory, bitmap, zone, and action-list owners with fail-closed
  missing material. Verification:
  `dm1_v1_f0600_f0620_memory_graphics_source_audit_pc34_compat`.

- ✅ 2026-07-23 DM1 F0561-F0581 batch: entrance/platform source ownership
  requires authentic material and leaves unsupported Amiga/floppy/VBlank
  paths fail-closed. Verification:
  `dm1_v1_f0561_f0581_entrance_platform_source_ownership_pc34_compat`.

- ✅ 2026-07-23 DM1 F0481-F0540 and CSB F0526-F0585 batch: source-gated
  graphics/cache/platform contracts retain existing owners only when
  authentic material is present; unavailable original paths fail closed.
  Verification: `dm1_v1_f0481_f0500_graphics_cache_source_receipt_pc34_compat`,
  `dm1_v1_f0501_f0520_graphics_platform_source_audit_pc34_compat`,
  `dm1_v1_f0521_f0540_graphics_runtime_source_audit_pc34_compat`,
  `csb_v1_f0526_f0545_platform_input_raw_pc34_compat`, and
  `csb_v1_f0566_f0585_platform_boundary_pc34_compat`.
- ✅ 2026-07-23 DM1 F1126-F1145 and F1166-F1185 source batch: I/O, USIO,
  and animation ownership is source-bound to authentic PC34 material;
  unavailable paths remain fail-closed without synthetic behavior. Verification:
  `dm1_v1_f1126_f1145_source_ownership_pc34_compat` and
  `dm1_v1_f1166_f1185_usio_anim_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 F1186-F1205 source batch: animation-step ownership now
  requires authentic PC34 material; missing bodies and raw animation input
  remain fail-closed. Verification:
  `dm1_v1_f1186_f1205_anim_step_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 Save & Quit: F0433 save callers now use the real user save
  directory and create it before write; missing directories no longer surface
  as file-not-found. Verification: `dm1_v1_save_path_pc34_compat`.
- ✅ 2026-07-23 DM1 F1206-F1245 source batch: I/O, animation and audio
  ownership require authentic PC34 material; missing paths remain fail-closed.
  Verification: `dm1_v1_f1206_f1225_source_ownership_pc34_compat` and
  `dm1_v1_f1226_f1245_anim_audio_source_audit_pc34_compat`.
- ✅ 2026-07-23 CSB F0926-F1005 source batch: platform, loader, and graphics
  boundaries require authentic PC34 material; unproved routes remain
  fail-closed. Verification:
  `csb_v1_f0966_f0985_source_boundary_pc34_compat` and
  `csb_v1_f0986_f1005_graphics_source_boundary_pc34_compat`.
- ✅ 2026-07-23 DM1 F1246-F1265 source batch: animation/media ownership is
  audited and unsupported routes remain fail-closed without synthetic paths.
  Verification: `dm1_v1_f1246_f1265_owner_audit`.
- ✅ 2026-07-23 DM1 F1266-F1305 and CSB F1026-F1045 source batch: DM1 input,
  language and FIO boundaries plus CSB platform-video routes are source-gated;
  missing PC34 material remains fail-closed. Verification: three focused tests.
- ✅ 2026-07-23 DM1 F1306-F1325 source batch: existing FIO owners are
  retained and unsupported boundaries remain fail-closed. Verification:
  `dm1_v1_f1306_f1325_fio_owner_audit`.
- ✅ 2026-07-23 CSB F1006-F1065 source batch: source/save-platform boundaries
  require authentic PC34 material and unproved paths remain fail-closed.
  Verification: `csb_v1_f1006_f1025_source_boundary_pc34_compat` and
  `csb_v1_f1046_f1065_save_platform_pc34_compat`.
- ✅ 2026-07-23 DM1 F1326-F1385 source batch: media, FIO/floppy, swoosh and
  vblank ownership is audited; unsupported routes remain fail-closed.
  Verification: three focused compatibility tests.
- ✅ 2026-07-23 CSB F1086-F1105 source batch: input ownership is source-gated;
  unproved routes remain fail-closed. Verification:
  `csb_v1_f1086_f1105_input_boundary_pc34_compat`.
- ✅ 2026-07-23 DM1 F1386-F1405 and F1426-F1445 source batch: local ownership
  is source-bound and absent source intervals are explicit/fail-closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 DM1 F1406-F1425 and CSB F1066-F1125 source batch: unmapped,
  Amiga and media boundaries are explicit/fail-closed without substitutes.
  Verification: three focused compatibility tests.
- ✅ 2026-07-23 DM1 F1446-F1485 source batch: local ownership is source-bound
  and absent ranges are explicit/fail-closed. Verification: two focused tests.
- ✅ 2026-07-23 DM1 F1486-F1505 and CSB F1146-F1165 source batch: non-PC34
  switch plus copy-protection/USIO boundaries are source-gated/fail-closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 DM1 F1526-F1545 and CSB F1126-F1145 source batch: workstation,
  AES, and source-route boundaries are source-gated/fail-closed. Verification:
  two focused compatibility tests.
- ✅ 2026-07-23 DM1 F1506-F1525 source batch: authentic PC34 source ownership
  is required and unsupported paths remain fail-closed. Verification:
  `dm1_v1_f1506_f1525_source_ownership_pc34_compat`.
- ✅ 2026-07-23 CSB F1166-F1185 source batch: USIO/animation ownership is
  source-gated and unproved routes remain fail-closed. Verification:
  `csb_v1_f1166_f1185_usio_anim_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 F1586-F1605 and F1646-F1665 source batch: TOS/AES,
  Switch and video paths are documented and PC34 remains fail-closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 DM1 F1626-F1645 and CSB F1206-F1225 source batch: authentic
  PC34 ownership is required and unsupported paths stay fail-closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 DM1 F1686-F1705 and CSB F1186-F1205 source batch: USIO/ANIM
  ownership retains verified sources and unproved PC34 paths fail closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 CSB F1226-F1245 source batch: animation/audio ownership is
  source-gated and missing source bodies remain fail-closed. Verification:
  `csb_v1_f1226_f1245_anim_audio_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 F1666-F1685 and F1726-F1745 source batch: INT1/USIO
  ownership is source-bound; absent callable ranges are explicit/fail-closed.
  Verification: two focused tests.
- ✅ 2026-07-23 CSB F1266-F1285 source batch: SWSH/platform ownership is
  source-gated and unsupported routes remain fail-closed. Verification:
  `csb_v1_f1266_f1285_swsh_platform_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 F1706-F1725 and F1786-F1825 source batch: MUSC/floppy and
  animation ownership is source-gated; unproved routes remain fail-closed.
  Verification: two focused tests.
- ✅ 2026-07-23 DM1 F1746-F1785 and CSB F1246-F1325 source batch: debug/error,
  source/language/FIO routes are source-gated and unproved PC34 paths fail
  closed. Verification: three focused compatibility tests.
- ✅ 2026-07-23 DM1 F1866-F1905 source batch: hint ownership is audited and
  unproved PC34 routes remain fail-closed. Verification:
  `dm1_v1_f1866_f1905_hint_owner_audit`.
- ✅ 2026-07-23 DM1 F1906-F1945 and CSB F1326-F1405 source batch: hint, FIO,
  SWSH and vblank ownership is source-gated; missing PC34 material fails closed.
  Verification: three focused tests.
- ✅ 2026-07-23 DM1 F1826-F1865/F1946-F1985 and CSB F1446-F1485 source batch:
  verified PC34 owners remain admitted; all other boundaries fail closed.
  Verification: three focused compatibility tests.
- ✅ 2026-07-23 DM1 F1986-F2025/F2066-F2104 and CSB F1406-F1525 source batch:
  editor/hint and unmapped/Switch/VDI routes are source-gated; unproved paths
  fail closed. Verification: four focused compatibility tests.
- ✅ 2026-07-23 DM1 F2026-F2065 source batch: editor/input ownership is
  source-bound and unavailable PC34 paths remain fail-closed. Verification:
  `dm1_v1_f2026_f2065_source_ownership_pc34_compat`.
- ✅ 2026-07-23 DM1 L0001-L0050 and CSB F1526-F1605 inventory batch: local,
  platform, AES/TOS routes are source-gated and unsupported paths fail closed.
  Verification: three focused tests.
- ✅ 2026-07-23 DM1 G0001-G0050 inventory batch: graphics-state globals are
  source-bound and unverified globals remain fail-closed. Verification:
  `dm1_v1_g0001_g0050_graphic562_source_audit_pc34_compat`.
- ✅ 2026-07-23 DM1 P0001-P0050 and CSB F1606-F1685 inventory batch:
  parameter, VDI and platform routes are source-gated/fail-closed. Verification:
  three focused tests.
- ✅ 2026-07-23 DM1 G0051-G0100 and C001-C004/E/R/S inventory batch: global
  and special ownership is source-audited; unproved boundaries fail closed.
  Verification: two focused compatibility tests.
- ✅ 2026-07-23 DM1 P0051-P0100 and CSB F1686-F1765 inventory batch: text/
  sound, USIO/MUSC and source routes are source-gated/fail-closed. Verification:
  three focused tests.
- ✅ 2026-07-23 DM1 G0101-G0150/M0001-M0050 and CSB F1806-F1845 inventory
  batch: graphics globals, macro labels and memory/I/O routes are source-audited
  and unproved PC34 paths fail closed. Verification: three focused tests.
- ✅ 2026-07-23 DM1 G0151-G0200/M0051-M0100/P0101-P0150 and CSB F1766-F1885
  inventory batch: graphics globals, macros, parameters, media and hint/I/O
  routes are source-audited and unproved PC34 paths fail closed. Verification:
  five focused tests.
# 2026-07-23 - CSB F2246-F2285

Completed the ReDMCSB Towns-memory ownership batch with a focused PC34
compatibility test. Unproved routes stay fail-closed.
# 2026-07-23 - CSB-007 Existing Monster-Kill EXPOOL Writeback

Implemented the source-bounded CSBWin `ESTAT_NumMonsterKilled` writeback for
an existing authenticated four-word EXPOOL record. Missing counters are never
invented or allocated. Focused recovery test passes.
# 2026-07-23 - CSB Package Presentation Probe

Aligned TITLE source-step identity with the M11 playback frame and restored the
neutral terminal palette for C017/C040. The real CSB package presentation probe
passes all 27 checks from title through entrance and HUD.
- ✅ 2026-07-23 DM1 compact `SquareFirstThings` mutation: M11 fixed
  possession and projectile-tail insertion delegate real PC34 map mutation
  to ReDMCSB DUNGEON.C F0514, preserving compact slot order, thing-list flags,
  and cumulative columns. Added a compact-table fixed-possession regression.
  Verification: `m11_creature_fixed_possession_runtime_source_lock` and
  `dm1_v1_thing_list_mutation_f0162_f0163_f0164_pc34_compat`.

- ✅ 2026-07-23 CSB Utility Disk package admission: M11 now verifies the
  Utility/HUD path through a decoded C004/C002/C003 startup session instead
  of accepting the release wrapper. Verification:
  `csb_v1_m11_utility_capture_admission` and the real package presentation
  probe (27/27).
- ✅ 2026-07-23 DM1 original-PC34 corpus discovery: the recursive scanner
  validates the real 512-byte ReDMCSB SAVEHEAD.C header before a full-file
  corpus slot is consumed, so unrelated game media cannot exhaust the
  bounded result array ahead of an arbitrary-named PC34 save. Added the
  fixture-free `firestaff_dm1_v1_original_save_pc34_real_corpus_probe` CMake
  diagnostic. Local `data/dm1` scan found 40 files and zero qualified saves;
  no original bytes were invented or certified. Verification:
  `dm1_v1_original_save_classifier_pc34` and the real-corpus probe.

- ✅ 2026-07-24 Runtime graphics panel: F10 now opens a mouse- and
  keyboard-driven three-page graphics panel while a game is running. It
  switches admitted V1/V2.0/V2.1/V2.2 presentation modes, scaling, aspect,
  filter, window, palette, CRT, dither, sharpening, phosphor, pixel-grid,
  motion-blur, dynamic-lighting, and turn-pan settings live. V1 keeps
  source-faithful effects locked and V2.2 is unavailable without an admitted
  artpack. Dynamic light and turn-pan reread the persisted setting on the
  next render or accepted movement tick. Verification:
  `m11_runtime_graphics_popup`, `m11_v1_action_area_geometry_pc34_compat`,
  `dm1_v1_swsh_psg_audio_pc34_compat`, and
  `csb_v1_viewport_phase3_rendering`.
- ✅ 2026-07-24 DM1 E0013/E0014/E0015/E0017/E0061 and S0080/S0081 platform timing bundle: a source-audited 20 ms PC34/PAL host scheduler now owns Timer-C no-op, keyboard/MIDI, palette, VBlank, Timer-A sound, DMA completion, and floppy power contracts. E0017 gates the live DM1 VBlank counter; unavailable host services fail closed instead of being synthesized. Verification: `dm1_v1_platform_timing_exception_pc34_compat` and `dm1_v1_s0080_s0081_media_platform_boundary_pc34_compat` pass, and `firestaff` builds with Ninja.

- ✅ 2026-07-24 CSB expansion package/save isolation: standard registry
  packages and explicitly registered custom DUNGEONB files now have a
  byte-verified package identity and separate save namespace. Version-12
  native saves reject a different active package before runtime mutation;
  filename-only candidates stay rejected. Verification:
  `csb_v1_expansion_package_admission` passes.

- ✅ 2026-07-24 CSB Utility import confirmation: a source-validated DM1
  candidate is now isolated from the committed party during preview. Reject
  and cancel discard it; explicit acceptance atomically commits it before
  `NEW_GAME`. Verification: `csb_v1_utility_import_confirmation_pc34_compat`,
  `csb_v1_utility_flow_action_contract`, and focused boot-handoff coverage.
- ✅ 2026-07-24 CSBWin original-save admission: the resume path now validates
  the complete preserved DB11/EXPOOL chain before it stages runtime state and
  retains the accepted file's FNV, core offset, CSB key verdict, game id and
  path as source provenance. A malformed tail rolls back without touching the
  prior live runtime. Verification:
  `csb_v1_csbwin_save_provenance_pc34_compat` and
  `csb_v1_save_import_path_pc34_compat`.

- **CSB F0115 first-object native graphic mapper (G0209):** Done 2026-07-25.
  Implements `csb_v1_viewport_f0115_object_native_graphic_pc34` and
  `csb_v1_viewport_f0115_blit_first_object_native_family_pc34` with
  CSB-specific direct-table mapping for all 6 thing types
  (weapon[46]/armour[58]/junk[52]/potion[21]/container/scroll) to native
  graphics 498-583. Blit applies C10 transparency and conditional horizontal
  flip for multi-graphic aspect leaders. Replaces m11_game_view.c stubs.
  Verification: `csb_v1_f0115_first_object_real_asset_pc34_compat`.

- **CSB viewport test suite bulk integration:** Done 2026-07-25.
  Wired 44 CSB viewport tests into CMakeLists.txt covering walls (D0-D3,
  all positions), doors (F0111 partly-open, front-clipped, door frames),
  floor/ceiling ornaments (F0095, F0108), center fields, custom backgrounds
  (backdrops, room slots, masks, pass order, source locks), sidewall
  backdrops, F0108 footprints, wall ornaments (F0107), and F0115 projectile
  routing. All 44 build and pass. Three tests with deep transitive
  dependencies (f0115_projectile_metadata, d0l2_d0r2_f0111_f0115_route_receipt,
  d2l2_d2r2_f0111_partly_open) deferred until they can link against the full
  library. Pre-existing build failures in chaos_magic (unused static
  functions) fixed with __attribute__((unused)).

- **DM1 test batch v3.0.121 — 6 suites, 44 tests:** Done 2026-07-25.
  stairs_level (9 tests, Q-DM1-04): init/add/check/use stair, add level,
  transition query, tick. palette_font (8 tests, Q-DM1-03): constants,
  palette/font init, default palette, set palette, custom colors, skill
  names, font alloc. amiga_platform_boundary (7 tests, Q-DM1-08): boundary
  queries for F0513/F0535/F0551/F0557/F1111, is_portable, source evidence.
  f0740_f0743_music_source (7 tests, Q-DM1-08): constants, state init
  (musicOn=1 default), struct layouts, bind nonexistent, pause unauthenticated.
  floor_feature_material (7 tests, Q-DM1-03): palette route enum, struct
  layouts, FNV1a hash (null/data), find source (null/no match).
  champion_runtime_source_m11_bridge (6 tests, Q-DM1-07): command kind enum,
  init, struct layouts, source evidence.

- **DM1 test batch v3.0.122 — 6 suites, 64 tests:** Done 2026-07-25.
  champion_needs (12 tests, Q-DM1-07): constants, bar colors, scent capacity,
  struct layouts, scent ordinal empty, bar width/color, stamina amount, bar
  render command. sound (10 tests, Q-DM1-08): 35 sound constants, play modes,
  music constants, emission routes, init, party position, music toggle, sound
  name/data, request play. combat (15 tests, Q-DM1-05): attack types, wound
  masks/indices, outcome enum, creature sizes, weapon constants, init
  (alive=1, health=100), group init, armor defense, scaled product, max load,
  movement ticks, source evidence. creature_render (14 tests, Q-DM1-03): 27
  creature types, size constants, graphic masks, aspect masks, pose enum,
  render list init, aspects table, direction delta, type name, coordinate set,
  transparent color, palettes D3/D2. f0341_scroll_material (7 tests, Q-DM1-06):
  constants, struct layouts, FNV1a, receipt empty. f0351_stats_material
  (6 tests, Q-DM1-06): struct layouts, FNV1a, receipt empty.

- **DM1 test batch v3.0.123 — 6 material suites, 30 tests:** Done 2026-07-25.
  f0352_eye_material (6 tests, Q-DM1-06): arrow/eye constants, struct layouts,
  FNV1a, receipt empty. f0355_inventory_material (5 tests, Q-DM1-06): panel
  constants, receipt struct, FNV1a. f0659_shield_material (5 tests, Q-DM1-07):
  shield trio constants, receipt struct, FNV1a. f0661_damage_material (4 tests,
  Q-DM1-05): damage dimensions, receipt struct, FNV1a. f0662_invisibility_material
  (5 tests, Q-DM1-07): champion icon constants, palette changes table, FNV1a.
  f0663_smoke_material (5 tests, Q-DM1-03): smoke pattern constants, palette
  changes table, FNV1a.

- **DM1 test batch v3.0.124 — 6 material suites, 32 tests:** Done 2026-07-25.
  f0732_f0735_fill_material (6 tests, Q-DM1-07): spell/viewport fill
  constants, box struct, FNV1a, clear null. f0115_f0219_creature_item_material
  (6 tests, Q-DM1-03): item/creature kinds, provenance, receipt null.
  f0115_near_object_decoration_material (5 tests, Q-DM1-03): near material
  kinds, provenance, receipt null. f0342_object_description_material (5 tests,
  Q-DM1-06): panel constants, operation kinds, FNV1a, receipt empty.
  f0731_f0734_inventory_zone_material (5 tests, Q-DM1-06): zone constants,
  receipt struct, FNV1a. f0675_scaled_material (5 tests, Q-DM1-03): struct
  layouts, FNV1a, receipt null.
# 2026-08-06 — CSB cache platform isolation

The FM Towns CSB cache installer now removes the four hash-pinned Amiga title
sidecars before materializing the selected CD image. This prevents a prior
Amiga scan from pairing `TITL.DAT`, `ENDA.DAT`, `KAOS.FTL` or `SWSH.FTL` with
FM Towns `GRAPHICS.DAT`; each title route now remains tied to one original
platform package.
- 2026-08-06 Nexus save round-trip test stability: moved the large native
  `Nexus_V1_World` and champion-pool test objects from the small process stack
  to heap-owned state. The test now reaches and passes its existing
  `nexus_v1_save_full` -> `nexus_v1_load_full` `party_x` gate without changing
  serialized bytes or promoting the native FNXS format to Saturn-card parity.
- ✅ 2026-08-06 DM1 chest eye/C071 mutation and external-build verification:
  an eye click with a held real Thing now keeps the source CHEST.C F0334
  close and leader-hand mutation successful when authenticated C101 object
  panel art is unavailable; only the visual panel is suppressed, with no
  host substitute. The pass1091 inventory-slot verifier now honors the
  `FIRESTAFF_BUILD_DIR` CMake environment for out-of-tree Ninja builds.
  Focused DM1 regression: 58/58 tests passed, including the full-leader-hand
  C539/C071/floor-drop chain and pass1091.
- ✅ 2026-08-06 DM1 V2.2 screenshot receipt honesty: the source-owned V2
  screenshot probe now requires an authenticated finished real V2.2 artpack
  and reviewer receipt before emitting V2.2 rows. Without that real pack it
  emits 12 authenticated V1/V2.0/V2.1 rows and explicitly omits V2.2 instead
  of recording the unchanged V1 framebuffer as modern art. The receipt verifier
  accepts both the 12-row no-pack state and the full 16-row state when a real
  pack is present. Verification: probe and `dm1_v2_source_owned_screenshot_receipts`
  passed with the real PC34 `DUNGEON.DAT`; 15/15 probe invariants passed.
- ✅ 2026-08-06 DM2 FM Towns English M11 dialogue rendering: the active
  `c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL` no longer stops at M11's
  former image-only panel blit. M11 now delegates it to the source-owned DM2
  viewport renderer, which retains the Japanese CD panel, raw4 geometry and
  local palette while drawing the compiled heading and authenticated
  PC-English GDAT labels with the original `dt07/0` font and action-table
  palette remap. The whole panel fails closed when any source material is
  absent. Verification: `test_dm2_v1_dialogue_box_viewport_real_data`,
  `test_dm2_v1_m11_startup_profile_gate`, and the FM Towns direct/ZIP English
  companion regression pass against user-supplied original media.
- ✅ 2026-08-06 DM2 GDAT structure-stub fail-closed gate: retired the false
  `valid` receipt from the incomplete
  `DM2_READ_GRAPHICS_STRUCTURE` compatibility seam. SKProject
  `bgdat.cpp:1027-1141` proves that a real structure load includes header
  validation, ULP/ENT1 allocation and image-allocator setup; none can be
  inferred from two caller-populated words. The API now returns failure and
  clears its receipt until those original owners are implemented. The focused
  GDAT compatibility test verifies the rejection.
- ✅ 2026-08-06 Nexus boot placeholder removal: replaced the obsolete
  `CHAMPIONS.DAT` validation with the hash-verified European `RLOWFIX.BIN`
  champion/CRET source consumed by the engine. The real Nexus boot hash scan
  passes without a false missing-champion diagnostic.
- ✅ 2026-08-06 DM2 creature-combat synthetic-result gate: the former
  provider-only creature-combat wrapper no longer converts one authenticated
  AIDefinition Defense value into host-calculated damage or a kill. SKProject
  `skchamp.cpp::CALC_PLAYER_ATTACK_DAMAGE` (1402-1545) requires live champion
  and hand records, CMDSTR action values, target record, map difficulty/light,
  source RNG, GDAT item fields and skill/stamina/poison writeback. The public
  bridge now returns a specific incomplete-contract receipt and no result
  until those original inputs and effects are connected. The focused combat
  regression covers missing, unproven and even proven Defense callbacks.
- ✅ 2026-08-06 Nexus HUD placeholder audit: corrected the V2 HUD integration
  test to require zero framebuffer writes while Saturn HUD/VDP1/VDP2 capture
  is absent. The test now matches the production no-op gate and passes 9/9;
  the standalone overlay smoke test passes 46/46 without synthetic pixels.
- ✅ 2026-08-06 CSB ZIP→ADF scanner admission: the shared hash scanner now
  falls through from its native ZIP reader to the existing nested-disk
  archive route when a ZIP member is an ADF/ST/MSA image.  It retains the
  complete `archive.zip::disk.adf::FILE` receipt and materializes the same
  member for the ordinary CSB runtime cache.  Regression coverage adds the
  ZIP→ADF lookup/extraction case to `test_asset_find_by_hash`.  Verified
  against the supplied `Chaos Strikes Back (FTL).zip` Amiga package: CSB
  changes from `MISSING` to `READY` and emits the hash-verified cache files.
- ✅ 2026-08-06 CSB native ZIP→ADF scanner hardening: ZIP-distributed Amiga
  (and Atari ST/MSA) disk images now use Firestaff's bounded ZIP-deflate and
  native disk-filesystem readers before considering any external tool. The
  resulting `archive.zip::disk.adf::FILE` receipt materializes through the
  same in-process path, so normal CSB ZIP→ADF packages remain launchable
  without 7zz/7z/bsdtar. The retained external path is only a fallback for
  ZIP compression methods Firestaff does not own. `test_asset_find_by_hash`
  now disables all external archive tools while proving nested ZIP→ADF lookup
  and extraction; the supplied `Chaos Strikes Back (FTL).zip` also reports
  CSB `READY` under an empty tool PATH.
# 2026-08-06 CSB FM Towns C06 palette receipt

- ✅ Replaced the standalone C09_ICON palette copy with the indexed RGB6
  table read from the selected, hash-verified C06 executable. `UTILE.EXP`
  uses raw offset `0x17DB0`; `UTILJ.EXP` uses `0x17E18`. The reader verifies
  all 16 source indices and the trailing `0xFF` entry before M11 receives
  any palette value. ReDMCSB `CEDT027.C:45-62` defines C09_ICON and
  `CEDT018.C:829-838` selects it for the editor.
- ✅ The original F31E and F31J Game→Utility handoff test passes against the
  local retail FM Towns archive. Japanese text and C06 file/save/portrait
  transactions remain fail-closed pending their native owners.
# 2026-08-06 CSB FM Towns TITLE Timer-A hold

- ✅ The real F31 title regression now proves the final `TITLE.ANM` frame
  stays visible through 605 source Timer-A ticks. `SWITCHTW` may bind only on
  tick 606, using the original `18 * (1024 - 100)` microsecond Timer-A period
  rather than the 16 ms host wake. The following 60-VBlank switch-page delay
  remains independently checked for both English and Japanese retail media.

# 2026-08-06 CSB FM Towns F31J text-owner audit

- ✅ Recovered the F31J C06 text ownership boundary from ReDMCSB
  `JAPANESE.C` and `CEDT030.C`. The F31/F20 FM Towns build calls the EGB
  `sjisString` service; PC-98 port-I/O and X68000 IOCS glyph paths are other
  platform branches. The retail `T_OAK2.EXE` and `OAK2USR.DIC` do not prove a
  game-owned C06 bitmap font, so Japanese Utility drawing remains correctly
  fail-closed instead of borrowing a host font or an unrelated system file.
# 2026-08-06 CSB Atari MSA save-disk admission

- ✅ Added a strict Magic Shadow Archiver reader for CSB's Atari ST save-disk
  media. It accepts only the documented big-endian `0x0E0F` header, bounded
  per-track RLE and GEMDOS/FAT12 root-file chains; malformed tracks, broken
  chains and absent files are rejected before any save bytes are exposed.
- Verification: a data-free 9-sector FAT12 regression passes, and the real
  `Chaos Strikes Back for Atari ST Save Disk.msa` from the local retail archive
  decodes to its declared 9-sector, two-sided, 80-track 720 KiB image.

# 2026-08-06 CSB Atari MSA FAT byte-order handling

- ✅ Corrected the MSA root-file reader to select the decoded GEMDOS/FAT12
  boot-sector byte order independently of the big-endian MSA wrapper. This
  keeps Atari-order images working and admits the little-endian FAT layout on
  the authenticated blank retail Save Disk without fabricating a save record.
- Verification: both synthetic Atari- and DOS-order FAT12 extraction paths,
  plus full decoding of the original two-sided 720 KiB MSA image, pass in
  `test_csb_v1_atari_msa`.
# 2026-08-06 CSB FM Towns C06 placeholder removal

- ✅ Removed the live M11 reconstruction of the FM Towns Utility editor. It
  drew C06 coordinates with generic boxes and the PC3.4 M653 font even though
  the real UTILE/UTILJ EGB text and editor consumers are not yet bound.
  Utility now remains on the authenticated SWITCHTW page instead of replacing
  it with host-composed pixels.
- Verification: the real-media FM Towns handoff regression admits the
  authentic UTILE program, menu bytes and C09 palette, then confirms that a
  Utility click is fail-closed and preserves the SWITCHTW raster.

# 2026-08-06 CSB FSSB save-envelope production isolation

- ✅ Removed the Firestaff-only FSSB export/import wrapper and its Utility
  transaction from `firestaff_m10`. Both reconstruct a CSBGAME-shaped party
  buffer and are therefore contract tests, not an original CSBGAME or CSBWin
  save route.
- ✅ The focused tests compile those helpers explicitly. Production save/resume
  remains owned by the authenticated Atari/Amiga/CSBWin readers; no
  source-labelled path string can authorize a synthetic runtime handoff.
- ✅ Added `csb_fssb_production_boundary`, which guards both CMake exclusions
  and the explicit test-only source list against future broad-glob regressions.
- ✅ `verify.yml` runs that boundary as a required cross-platform check rather
  than leaving it in the best-effort full CTest catalogue.

# 2026-08-06 CSB compact-roster production isolation

- ✅ Removed the historical compact `CSBGAME` roster reader from
  `firestaff_m10`. It describes only a party header and records; it does not
  carry ReDMCSB `LOADSAVE.C F0435`'s complete original save body.
- ✅ CSBWin discovery remains read-only in production and reports that a full
  body is required. The focused importer tests and probe still compile the
  reader explicitly, and a required CI boundary prevents a broad M10 glob
  from restoring it.

# 2026-08-06 CSB CMP fixture production isolation

- ✅ Removed the portrait-only CMP helper from `firestaff_m10`. Its contract
  fixtures can initialise a party slot, but an original CMP does not own the
  champion's vitals, skills, inventory, world state or slot ownership.
- ✅ The focused CMP tests and real-media probe compile the helper explicitly.
  Production remains limited to an overlay on an authenticated champion, and
  CI now rejects a broad-glob regression.

# 2026-08-06 CSB hidden-item safety-loader isolation

- ✅ Removed the unconsumed CSB Atari/Amiga hidden-code safety loader from
  `firestaff_m10`. It remains explicitly available to its real-media probe
  and focused regression, where the dmweb/ReDMCSB hidden GRAPHICS.DAT entries
  are checked without making a synthetic bitmap route part of production.
- ✅ A required CI source boundary prevents the broad M10 source glob from
  relinking that test/probe helper until a live platform renderer owns it.

# 2026-08-06 CSB viewport side-wall contract isolation

- ✅ Removed the D2L/D2R and D3L2/D3R2 wall-contract traces from
  `firestaff_m10`. They record ReDMCSB branch order with local source/dest
  buffers, but do not bind an authenticated CSB viewport material source.
- ✅ Their focused regressions now compile the traces explicitly, and CI
  rejects a broad-glob relink until a real-data viewport consumer owns them.

# 2026-08-06 CSB viewport contract-bundle isolation

- ✅ Removed the D1C F0115, D1L2/D1R2 F0111, D2L2/D2R2 F0111/wall and D3C
  F0107/F0108 contract traces from `firestaff_m10`. They retain ReDMCSB/CSBWin
  branch evidence and local-buffer checks, but no live M11 caller consumes
  authenticated CSB bitmap material through them.
- ✅ The four focused regressions and the D1C real-asset ornament probe now
  own their sources explicitly. CI rejects a broad-glob relink.

# 2026-08-06 CSB PC34/A31E cache provenance fix

# ✅ 2026-07-12 CSB F0267 object sensor-to-event bridge: loaded ordinary-object movement now carries each F0276 remote floor-sensor result through F0272 target-square resolution and schedules its F0268 `TIMELINE_EVENT_SQUARE_STATE` in source order. The route supports source event types fakewall, teleporter, pit, and door; it preserves same-map target context, target cell, SET/CLEAR/TOGGLE effect, and the one-tick minimum for zero delay. The existing square-state dispatcher owns the final mutation. Source: ReDMCSB `MOVESENS.C F0267/F0268/F0272/F0276` and `TIMELINE.C F0242/F0244/F0250/F0251`. Verified by `test_csb_v1_f0267_loaded_chain_pc34_compat`, including an object C004 sensor routed to a delayed door event.

# ✅ 2026-07-12 CSB F0276 object source-unlink ordering: the live C49 associated-object teleporter route now calls the real-format C004 sensor pass both after source materialization/link and immediately after its source unlink, before target relink. The shared C004 path models ReDMCSB `MOVESENS.C F0276` `AddThing ^ RevertEffect` behavior, including HOLD resolving to SET for addition and CLEAR for removal. Object pit and stairs hops use the same source-removal pass. Regression: `test_csb_v1_runtime_tick_accumulator` locks the C49 C004/C05 source chain and its coalesced pending CLEAR result; `test_csb_v1_teleporter_rotation_runtime_pc34_compat` passes 114/114. The broad runtime accumulator has one pre-existing unrelated `MOVE_FORWARD boundary reaches the bounded open-step runtime movement` failure.

# ✅ 2026-07-12 CSB F0276 C001 object pressure-plate chain: extended the live real-format ordinary-object sensor pass from C004 to source C001 floor plates. The C001 path follows ReDMCSB `MOVESENS.C F0276` lines 1608-1655 and 1664-1667: it evaluates the pre-link-equivalent object/group/party occupancy state, then resolves `AddThing ^ RevertEffect` and HOLD. The dedicated C49 associated-object → object-scope C05 regression proves source C001/C05 preservation, target relink, and the ordered add SET/source-unlink CLEAR result. Verification: `test_csb_v1_f0276_object_chain_pc34_compat` passed 8/8; `test_csb_v1_f0267_loaded_chain_pc34_compat` passed.

# ✅ 2026-07-12 CSB F0276 audible C004 object route: triggered Audible C004 floor-object sensors now request ReDMCSB's prioritized switch sound before publishing their ordinary F0272/F0268 square event. Source: `MOVESENS.C F0276` lines 1770-1772 and `SOUND.C F0064`. Verification: the dedicated real-format C49 materialization regression passed 7/7 and locks `SOUND_SWITCH`, volume 64, priority 4, one audio request, and the target fakewall SET event; the focused F0267/F0276 CTest group passed 3/3.

# ✅ 2026-07-12 CSB F0276/F0272 C004 OnceOnly writeback: a triggered real-format C004 floor-object sensor now disables itself before publishing its first effect, preserving its data bits. Source: ReDMCSB `MOVESENS.C F0272` lines 1191-1193 and F0276 trigger path. Verification: the dedicated C49 materialization regression passed 7/7 and proves sensor-type zeroing plus the initial fakewall SET event; the focused F0267/F0276 CTest group passed 4/4.

# ✅ 2026-07-12 CSB F0276/F0272 C004 Value timing: C004 object sensor remote effects now carry the original four-bit `Remote.Value` delay into F0268 timeline scheduling instead of always using the current tick. Source: ReDMCSB `MOVESENS.C F0272` lines 1194-1203. Verification: the dedicated C49 materialization regression passed 6/6 and proves a source `Value=3` trigger at game time 1 queues its fakewall SET for time 4; the focused F0267/F0276 CTest group passed 5/5.

# ✅ 2026-07-12 CSB F0276/F0272 C004 target-cell semantics: remote C004 effects now preserve `Remote.TargetCell` only for wall targets; fakewalls, doors, pits, teleporters, and corridors queue `CELL_NORTHWEST` as in ReDMCSB F0272. Source: `MOVESENS.C F0272` lines 1201-1207. Verification: the dedicated C49 C004 regression passed 5/5 and proves an encoded cell 3 fakewall target queues cell 0; the focused F0267/F0276 CTest group passed 6/6.

# ✅ 2026-07-12 CSB F0276 C004 Revert ordering: locked the existing source `AddThing ^ RevertEffect` behavior with a real-format C49 associated-object → object-scope C05 chain. A non-HOLD Revert C004 suppresses the source materialization/addition and publishes its SET only when F0267 unlinks the object before teleporter relink. Source: ReDMCSB `MOVESENS.C F0276` lines 1663-1694 and 1760-1778. Verification: the dedicated regression passed 7/7; the focused F0267/F0276 CTest group passed 7/7.

# ✅ 2026-07-12 CSB F0276/F0270/F0271 C004 LocalEffect: object-triggered local C004 effects now retain the final local `CLEAR`/`TOGGLE` while scanning the square and rotate the complete source sensor run only after the pass, with no F0268 remote event. Source: ReDMCSB `MOVESENS.C F0270` lines 1080-1098, F0271 lines 1100-1158, and F0272/F0276 local-effect path. Verification: the dedicated C49 two-sensor regression passed 6/6; the focused F0267/F0276 CTest group passed 8/8. The separate C10 steal-skill local effect remains outside this bounded rotation route.

# ✅ 2026-07-12 CSB F0276 C007 floor-creature group route: C04 group relocations now run source/destination F0276 passes and use the real-format sensor scan for C002/C007 group eligibility. A C007 sensor on the destination publishes its normal F0272/F0268 fakewall SET event after the group is relinked. Source: ReDMCSB `MOVESENS.C F0267` lines 800-867 and `F0276` lines 1658-1778, especially C007 lines 1712-1715. Verification: `test_csb_v1_f0276_group_creature_sensor_pc34_compat` passed 4/4; the manually rebuilt focused nine-binary F0276 group also passed with strict runtime compilation.

# ✅ 2026-07-12 CSB DSA transfer runner: the authenticated filter callback now promotes only CSBWin `Execute`'s already source-locked transfer-only `JUMP`/`GOSUB` subset. It invokes the bounded complete transfer chain, publishes its final state and receipt only on success, and preserves the caller parameter surface. Unsupported targets, malformed paths, and depth/transfer limits remain rejection paths. No world opcode or synthetic state transition is enabled. Source: CSBWin `DSA.cpp` lines 764-849 and 5053-5293. Verified by `test_csb_v1_dsa_trigger_single_step_pc34_compat`: 124 assertions, 0 failures.

# ✅ 2026-07-12 CSB DSA runtime binding: `csb_v1_runtime_resolve_csbwin_dsa_filter_binding()` now follows CSBWin `Monster.cpp` / `DSA.cpp` selector ownership: a verified type-47 DB3 actuator contributes `word2` bits 7..11, the staged `DSALevelIndex[level][selector]` resolves its absolute DSA ID, and that ID must own an imported authenticated action before it is usable. `csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner()` then prepares the existing pure-stack callback only for an exact selected action. It rejects missing level-table slots and unowned IDs without a fallback. Verified by `test_csb_v1_phase7_verification`: 324 passed, 0 failed.

# ✅ 2026-07-12 CSB EXPOOL global-variable DSA handoff: CSBWin saves now restore their contiguous `EDT_Database | EDBT_GlobalVariables | i` EXPOOL records into Firestaff's bounded source-sized DSA global bank before the existing tracing handoff, matching `SaveGame.cpp`'s sixteen-`ui32` record order and first-missing-record stop. A malformed present record rejects transactionally, and authenticated filter runners inherit the restored bank rather than a synthetic zero bank. Source: CSBWin `SaveGame.cpp` global-variable save/load loops and `data.cpp EXPOOL::Locate`. Verified by `test_csb_v1_phase7_verification`: source-order two-record import plus malformed-record preservation.

# ✅ 2026-07-12 CSB DSA global-bank runtime commit: the profile-owned CSBWin global bank is now rehydrated into an authenticated pure-stack runner immediately before execution and receives its `GLOBALSTORE` result only after the existing full-action commit succeeds. Caller/stale runner globals cannot become profile state, while world and filter opcodes remain outside this route and EXPOOL serialization remains open. Source: CSBWin `DSA.cpp` `EX_GLOBALFETCH`/`EX_GLOBALSTORE` and `SaveGame.cpp` global-variable ownership. Verified by `test_csb_v1_phase7_verification`: authenticated `GLOBALSTORE` updates runner and profile bank together.

# ✅ 2026-07-12 CSB DSA global EXPOOL writeback: a successful authenticated `GLOBALSTORE` now stages the profile global bank and rewrites its existing CSBWin `EDT_Database | EDBT_GlobalVariables` payload words in the preserved EXPOOL tail before committing either caller parameters or runtime state. The tail hash is refreshed, so the existing CSBWin core exporter retains the real updated record rather than a stale copy. Missing, malformed, truncated, oversized, or partial-record tails reject without publication. Source: CSBWin `SaveGame.cpp` global-variable save loop and `data.cpp EXPOOL::Locate`. Verified by `test_csb_v1_phase7_verification`: one source record updates the runner, profile bank, and located EXPOOL little-endian word.

# ✅ 2026-07-12 CSB DSA global save-export handoff: the existing bounded CSBWin core-save exporter is now regression-locked after an authenticated `GLOBALSTORE`. It verifies the emitted body, resolves the exported `EDT_Database | EDBT_GlobalVariables` record via the same source EXPOOL lookup, and proves the committed little-endian word survives the runtime-to-core-save boundary. Source: CSBWin `SaveGame.cpp` global-variable write loop and `data.cpp EXPOOL::Locate`. Verified by `test_csb_v1_phase7_verification`.

# ✅ 2026-07-12 CSB DSA global native-save handoff: Firestaff-native CSB saves now have a regression that proves the updated preserved EXPOOL tail survives native save/reload and is rehydrated into the source-sized DSA global bank. Source: CSBWin `SaveGame.cpp` global-variable load order before DSA tracing. Verified by `test_csb_v1_phase7_verification`: `GLOBALSTORE` -> native save -> reload retains the record and value.

# ✅ 2026-07-12 CSB EXPOOL save-policy handoff: CSBWin `EDBT_DisableSaves` is now staged transactionally from the preserved EXPOOL tail and blocks the Firestaff runtime save entry point after a native reload. A missing record permits saves; a malformed/truncated tail rejects before live state publication. Source: CSBWin `SaveGame.cpp` lines 1972-1976 and `CSB.h` `EDT_Database` / `EDBT_DisableSaves`. Verified by `test_csb_v1_phase7_verification`: source record -> native save/reload -> save refusal.

# ✅ 2026-07-12 CSB authenticated DSA filter runner: added `csb_v1_csbwin_dsa_run_authenticated_filter_stack_action()` as the runtime callback for the supported CSBWin `ProcessDSAFilter` pure stack subset. It requires exact pointer identity with the imported `(dsa,state,ordinal)` action, stages the signed parameter surface and its owned global bank, and publishes a receipt only after a complete supported action. Forged pointers and world-mutating `AMPERSAND` code leave all caller state unchanged. Source: CSBWin `DSA.cpp` `ProcessDSAFilter`/`ProcessDSATimer6` lines 5315-5460 and `Execute` lines 5053-5293. Verified by `test_csb_v1_dsa_trigger_single_step_pc34_compat`: 123 assertions, 0 failures.

# ✅ 2026-07-12 CSB DSA attack-filter ABI: `csb_v1_dsa_filter_attack_preprocess_live()` now maps the complete 20-word CSBWin `ATTACK_PARAMETERES` surface exactly as `Monster.cpp:916-938,1164-1167` copies it through `pDSAparameters+1`. This fixes the prior nine-word, incorrectly ordered bridge and preserves mutations to monster position/origin, range/damage, party direction/distance, projectile flags, hero target, sound, `disableTime`, and signed poison suppression. The callback still restores the caller's loaded level. `test_csb_v1_phase7_verification` covers source order plus mutations in the middle and tail of the struct. Verified: 321 passed, 0 failed.

# ✅ 2026-07-13 CSB saved TT_STONEROOM DSA runner preparation

`csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner()` now
consumes a validated CSBWin `Timer.cpp::ProcessTT_STONEROOM` function-6
receipt into the existing profile-owned pure-stack runner and returns only the
exact imported `DSAAction` selected by `ProcessDSATimer6`. The bridge checks
the source `(dsa, state, column)` identity again before publishing the runner.
It neither persists a master state nor enables world/filter opcodes;
unproven `LocalState=2` ParameterB and source-unimplemented `LocalState=3`
routes remain blocked. Verified by Ninja and `csb_v1_phase7_verification`.
Source: CSBWin `Timer.cpp::ProcessTT_STONEROOM`, `DSA.cpp::ProcessDSATimer6`
lines 5329-5450.

# ✅ 2026-07-13 CSB saved EXPOOL SETSKIN writeback

`csb_v1_runtime_set_csbwin_saved_skin()` now follows CSBWin `DSA.cpp`
lines 3122-3135 and `data.cpp` lines 1523-1567, 2130-2167: it changes the
exact packed cell byte, refreshes the tail FNV receipt, invalidates the HUD
skin cache, deletes an all-zero column, and can consume a pre-existing
source-owned exact-size DB11 free node for a resized column. Altered or
truncated tails, malformed DB11 links, and writes requiring `EXPOOL::enlarge`
still reject with no mutation. Verification: Ninja plus
`csb_v1_saved_skin_expool_writeback`.

# ✅ 2026-07-13 CSB EXPOOL DB11 node validation

The CSBWin `EXPOOL::Read`/`Write` bridge now proves every saved DB11 node is
an original `data.cpp EXPOOL::enlarge()` slot: its block header size matches,
the node starts at `block + 1 + n * size`, and the complete node stays inside
that 64-word DB11 block. A malformed free-list pointer cannot overwrite a
DB11 header during DSA `SETSKIN`; the candidate tail is discarded unchanged.
Verified by Ninja and `csb_v1_saved_skin_expool_writeback`.
# ✅ 2026-07-16 CSB TIMELINE F0240 first-event expiry receipt

CSB now exposes a source-named `F0240_TIMELINE_IsFirstEventExpired` receipt
over the live runtime timeline heap. The receipt reads only the CSB
`timeline_queue` root and runtime `game_time`, compares ReDMCSB's low-24-bit
`TIME(Map_Time) <= G0313_ul_GameTime` predicate, reports empty timelines as
non-expired, and rejects malformed heap roots without creating substitute
events. This closes the CSB TIMER symbol gap for F0240 only; broader F0261
event execution and DSA/save-corpus breadth remain separate. Verification:
`cmake --build build-local-ninja --target test_csb_v1_boot_runtime_handoff
-j2`, `ctest --test-dir build-local-ninja -R '^csb_v1_boot_runtime_handoff$'
--output-on-failure`, and `git diff --check` passed.

# ✅ 2026-07-16 CSB TIMELINE F0261 runtime tick receipt

CSB now exposes a source-named `F0261_TIMELINE_Process` receipt over the live
runtime tick path. The receipt records the live `timeline_queue` before and
after `csb_v1_runtime_tick_v1()`, drains expired events through the existing
ReDMCSB heap processor, preserves future events, and rejects malformed heaps
before ticking. It does not create timer/event substitutes or a synthetic DSA
corpus. Verification: `cmake --build build-local-ninja --target
test_csb_v1_boot_runtime_handoff -j2`, `ctest --test-dir build-local-ninja
-R '^csb_v1_boot_runtime_handoff$' --output-on-failure`, and
`git diff --check` passed.

# ✅ 2026-07-16 CSB TIMER F2262 CMake/test closure

The existing CSB `F2262_TIMER_A_EVENT` PC34 input-wait Timer A boundary is now
registered in CMake and mapped in the ReDMCSB audit/disposition tables. It
increments the wait-for-input VBlank counter, sets the stop-waiting flag at the
source threshold, and keeps the FM-Towns sound counter/fade path explicitly
unavailable for PC34 instead of synthesizing audio state. Verification:
`cmake --build build-local-ninja --target
test_csb_v1_f2262_timer_a_event_pc34_compat -j2`, `ctest --test-dir
build-local-ninja -R '^csb_v1_f2262_timer_a_event_pc34_compat$'
--output-on-failure`, and focused `git diff --check` passed.

# ✅ 2026-07-16 CSB ReDMCSB save/header/champion byte-helper cluster

The CSB-owned ReDMCSB save helper cluster is now CMake-registered and mapped
in the callable audit/disposition tables: F7055-F7058 checksum/obfuscation,
F7061/F7062 save-header read/write preparation, F7063 opaque 22-part dungeon
stream checksum, F7064 champion name/title padding, F7065/F7066 portrait slot
clear/rebind, and F7067/F7068 C31 portrait get/set. These helpers remain
byte-transaction boundaries only; they do not synthesize CSBWin DSA state,
runtime timers, champion layouts, or dungeon semantics. Verification:
the seven focused `redmcsb_f70xx_*_pc34_compat` CTests passed plus focused
`git diff --check`.

# ✅ 2026-07-16 CSB ReDMCSB F7059/F7060 dungeon-part checksum audit closure

The existing CSB-owned F7059/F7060 dungeon-part checksum helper is now
CMake-registered and closed in the ReDMCSB callable audit/disposition tables.
It only accumulates caller-owned, already-read or to-be-written dungeon-part
bytes with PC34 16-bit wraparound; no file transport, dungeon layout, CSBWin
extension, DSA, timer, or runtime state is inferred. Verification:
`test_redmcsb_f7059_dungeon_part_checksum_pc34_compat` builds and its focused
CTest passes.

# CSB M11 F0435 F9 provenance (2026-07-17)

- Native F9 reload now publishes an immutable F0435 header/Dungeon receipt
  and rechecks it before each CSB runtime tick. A corrupted native-header
  candidate is rejected rather than falling through to CSBWin; CSBWin stays
  available only through its own classifier/import receipt. Verification:
  corpus-backed `csb_v1_m11_f0435_f9_reload` PASS;
  `csb_v1_boot_runtime_handoff` PASS (469 assertions); isolated `firestaff`
  build and `git diff --check` PASS.

# CSB F0437 M11 title-prelude boundary (2026-07-17)

- M11's first title presentation tick already owns frame 1. The resume gate
  now advances only the remaining PRESENTS ticks before requiring F0437 frame
  60/source step 2 for CHAOS zoom. The focused F9 provenance CTest remains
  green, and the broad resume gate no longer reports the title-prelude or
  CSBWin F9 failures; 15 independent entrance, utility, HUD, and timer
  assertions remain.

# CSB F0806 entrance door-finish boundary (2026-07-17)

- The M11 resume helper now drives ReDMCSB's 20 delay ticks, all 31 C002/C003
  door steps, and the final source tick that emits `door_opening_finished`.
  Entrance dismissal and terminal HUD handoff now pass in the broad resume
  gate. Isolated `firestaff` build and `git diff --check` pass; eight separate
  utility-overlay, draw-plan, and CSBWin-timer assertions remain.

# CSB M11 utility startup-raster capture admission (2026-07-17)

- The ENTRANCE.C utility/HUD-menu render plan is now admission evidence only:
  M11 presents the existing source-owned startup raster only while the
  hash-verified startup package and release-capture identities match the
  active session. Missing, stale, or mismatched package/capture receipts leave
  the page no-draw; the old generated text/panel renderer remains unused.
  `csb_v1_m11_utility_capture_admission` covers a real CSB boot plus DM1
  utility route, current raster admission, stale capture rejection, package
  mismatch rejection, and missing-capture rejection. Verification: focused
  CTest PASS; `firestaff` build and `git diff --check` PASS. The broad resume
  gate now has only the independent CSBgraphics-plan, custom-background-mask,
  and CSBWin-timer assertions remaining.

# CSB M11 CSBgraphics declaration capture boundary (2026-07-17)

- The resume-gate viewport regression no longer treats a synthetic,
  cache-replaced runtime-plan entry as presentation material. M11 retains the
  direct source viewport when an entry lacks the operator-owned live-frame
  declaration and verified capture identity; no broad plan loop, fallback
  pixels, or generated surface is permitted. Positive structural coverage of
  the declaration, palette, source path/MD5, frame/door identity, and raster
  consumer remains in `csb_v1_csbgraphics_runtime_plan`. Verification:
  focused CTest PASS; resume gate now has only the independent
  custom-background-mask and CSBWin timer-queue failures; `firestaff` and
  `git diff --check` PASS.

# CSB M11 custom-background mask provenance (2026-07-17)

- Boot now hands a CSBWin `pSkinDef` bitmap/mask plan to the F0128 viewport
  only when its cache path and MD5 equal the profile's hash-verified
  GRAPHICS.DAT and the palette-source receipt is current. A replaced cache,
  missing palette evidence, or stale identity leaves the custom background
  no-draw; M11 continues with the authenticated base viewport and does not
  synthesize a raster. Regression coverage stages an unproven bitmap/mask pair
  in the resume gate and requires no framebuffer change, while
  `csb_v1_csbgraphics_runtime_plan` retains the focused declared-plan/mask
  coverage. Verification: focused CTest PASS; resume gate now has only the
  independent CSBWin timer-queue failure; `firestaff` and `git diff --check`
  PASS.

# CSB D3L2/D3R2 G0693 runtime material-plan admission (2026-07-17)

- The M11-bound first-frame material plan can now carry the real far-side
  ReDMCSB `F0676/F0677` D3L2/D3R2 F0111 door pair when both routes share one
  verified `GRAPHICS.DAT` G0693 payload receipt. The pair preserves the
  source order ahead of D2, separate C3700/C3710 clips (24/88, 28, 48x40),
  C10 transparency, path/hash provenance and distinct route identities. A
  lone D3 route, zero side hash, source mismatch, or incomplete material
  receipt remains no-draw and cannot perturb the established D0/D1/D2 plan.
  Verification: `csb_v1_viewport_first_frame_materialization_pc34_compat`
  and `csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat` PASS with the local
  `GRAPHICS.DAT`; isolated `firestaff` build and `git diff --check` PASS.

# CSB D2C G0694 first-frame capture admission (2026-07-17)

- The required D2C `F0121 -> F0111` first-frame command now consumes a
  value-owned receipt for original `GRAPHICS.DAT` G0694 rather than trusting
  its payload FNV alone. Admission requires the real-graphics/no-synthetic/
  no-fallback flags, G0694 item identity, nonempty source span and matching
  FNV, plus ReDMCSB C3760, 64x61 and C10 route facts. Missing or mixed capture
  evidence rejects the complete material plan and leaves the raster no-draw.
  Verification: `csb_v1_viewport_first_frame_materialization_pc34_compat`,
  `csb_v1_viewport_d2c_f0111_door_front_pc34_compat`, and
  `csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat` PASS with the local
  `GRAPHICS.DAT`; isolated `firestaff` build and `git diff --check` PASS.

# CSB D2C/D3 decoded-span capture intake (2026-07-17)

- First-frame material binding now requires a source-owned D2/D3 capture
  receipt before any raster consumption. The receipt repeats one original
  GRAPHICS.DAT path/MD5, exact palette capture FNV and nonzero capture
  identity, then binds G0694's 64x61 decoded span to D2C and, when the far
  pair is planned, one G0693 48x41 decoded span to both D3 routes. Every
  span's pointer, length and FNV must equal both the receipt and the matching
  plan command. Missing or mutated palette/source/span evidence rejects before
  rasterization. This is provenance intake only: it creates no decoder,
  palette, pixels or fallback image. Verification:
  `csb_v1_viewport_first_frame_materialization_pc34_compat`,
  `csb_v1_viewport_d2c_f0111_door_front_pc34_compat`, and
  `csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat` PASS; isolated
  `firestaff` build and `git diff --check` PASS.

# CSB G0693/G0694 F0488 native-span expansion (2026-07-17)

- Added the source-bounded F0489-to-F0488 viewport step for D2C/D3 door
  material. A capture must identify original GRAPHICS.DAT and its palette by
  matching path/MD5/FNV facts, retain the route receipt's raw G0694/G0693
  payload identity and provide exactly 32x61 or 24x41 native packed bytes.
  The adapter expands only the proven 4bpp high/low-nibble layout into 64x61
  and 48x41 indexed spans, records distinct decoded FNVs and binds those to
  the already admitted D2/D3 plan before rasterization. No direct mapping from
  a G0693/G0694 native bitmap index to a GRAPHICS.DAT entry was assumed.
  Truncated native spans, palette mutation, source drift and absent D3 data
  reject; raster also rechecks the retained capture identity and decoded FNVs
  so post-bind span mutation is no-draw. Verification:
  `csb_v1_viewport_first_frame_materialization_pc34_compat`,
  `csb_v1_viewport_d2c_f0111_door_front_pc34_compat`, and
  `csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat` PASS; isolated
  `firestaff` build and `git diff --check` PASS.

# CSB G0693/G0694 GRAPHICS.DAT table-provenance gate (2026-07-17)

- Reviewed the available ReDMCSB/CSBWin route evidence and did not find an
  original table that equates F0489's native bitmap cache indices G0693/G0694
  with a direct GRAPHICS.DAT entry number. The viewport now admits only the
  bounded original big-endian `0x8001` header plus compressed/decompressed
  entry-table span, records its FNV/path/MD5/native-index provenance, and
  explicitly marks the native-to-entry mapping unproven. F0489/F0488 accepts
  neither that receipt nor any inferred index, so it publishes no decoded
  span and the D2/D3 raster remains no-draw. This preserves the already
  source-attested decoded-span consumer while closing the unsupported native
  decode path. Verification:
  `csb_v1_viewport_first_frame_materialization_pc34_compat`,
  `csb_v1_viewport_d2c_f0111_door_front_pc34_compat`, and
  `csb_v1_viewport_d3l2_d3r2_f0111_door_pc34_compat` PASS with the local
  `GRAPHICS.DAT`; isolated `firestaff` build and `git diff --check` PASS.

# CSB F0276 C004 object-sensor runtime completion (2026-07-17)

- Completed the source-owned ReDMCSB `MOVESENS.C F0267/F0276` C004 object
  path. The ordinary-object chain now evaluates removal before source unlink,
  applies `AddThing ^ RevertEffect` before HOLD translation, writes OnceOnly
  through the loaded sensor record, routes Audible through the existing CSB
  audio owner, keeps local CLEAR/TOGGLE inside the source cell-run rotation,
  and schedules remote effects through F0272/F0268 with the packed delay and
  non-wall north-west target-cell rule. No generic sensor queue, substitute
  effect, or synthetic raster route was added. Verification:
  `csb_v1_f0276_object_audio_pc34_compat`,
  `csb_v1_f0276_object_once_only_pc34_compat`,
  `csb_v1_f0276_object_delay_pc34_compat`,
  `csb_v1_f0276_object_target_cell_pc34_compat`,
  `csb_v1_f0276_object_revert_pc34_compat`, and
  `csb_v1_f0276_object_local_effect_pc34_compat` PASS; isolated `firestaff`
  build and `git diff --check` PASS.
# CSB F0276 C005 stairs-sensor runtime route (2026-07-17)

- Added the source-owned ReDMCSB `MOVESENS.C F0267/F0276` C005 route before
  `CLIKMENU.C F0364` changes a party's level. The loaded C03
  `PARTY_ON_STAIRS` record now reaches the existing F0272/F0268 consumer while
  its source staircase is still current, preserving the raw OnceOnly write,
  `Remote.Value` timestamp, prioritized switch sound, local sensor-run effect,
  and non-wall north-west target rule. A non-stairs C03 record does not mutate
  raw Dungeon bytes, audio state, or the timeline. No generic queue, UI path,
  or synthetic stairs behavior was added. Verification:
  `csb_v1_f0276_party_c005_stairs_pc34_compat` plus the six C004 F0276
  regressions PASS; isolated `firestaff` build and `git diff --check` PASS.

# CSB F0276 C008 leader-hand possession route (2026-07-17)

- Completed the source-owned ReDMCSB `MOVESENS.C F0274/F0276` C008
  possession path. After its existing live CHARDESC-slot scan, CSB runtime
  now reads the owned GAMEBLOCK2/party `LeaderHandThing` exactly once and
  follows a C144 container only through its loaded `CONTAINER.Slot` chain.
  Missing and stale source thing identities fail closed before F0272/F0268;
  no M11 inventory projection, generic queue, synthetic state, audio, or
  timeline behavior was introduced. Verification:
  `csb_v1_f0276_party_c008_leader_hand_pc34_compat`, the C005 regression, and
  six C004 F0276 regressions PASS (8/8); isolated `firestaff` build and
  `git diff --check` PASS.

# CSB F0276 C009 PC34 version-checker route (2026-07-17)

- Completed the exact ReDMCSB `MOVESENS.C F0276` C009 party-addition gate.
  The runtime keeps the original compiled PC34 comparison (`Remote.Data <=
  34`) private to the F0276 consumer, so no caller or restored save may select
  a substitute engine mode. A passing loaded C03 record publishes only through
  the existing F0272/F0268 event path and F0261 subsequently mutates the real
  fakewall byte. An over-bound record rejects with no timeline or raw-Dungeon
  mutation. No synthetic queue, UI, audio, or timeline owner was introduced.
  Verification: `csb_v1_f0276_party_c009_version_pc34_compat`, C005, C008,
  and six C004 F0276 regressions PASS (9/9); isolated `firestaff` build and
  `git diff --check` PASS.

# CSB F0248 C010 launcher save handoff (2026-07-17)

- Completed focused lifecycle coverage for the existing source-owned ReDMCSB
  `TIMELINE.C F0247/F0248` C010 double-explosion launcher. A loaded raw C03
  and matching native C06 wall event emit exactly two launcher-owned lightning
  projectiles and their C49 movement events; the current CSB save handoff
  restores those emitted records while the boot-owned raw OnceOnly sensor
  remains disabled. A changed wall-cell identity reaches F0261 but publishes
  neither projectile nor mutation. No generic queue, UI route, or substitute
  projectile state was added. Verification:
  `csb_v1_f0248_c010_launcher_save_pc34_compat`, C008/C009 and six C004
  regressions PASS (9/9); isolated `firestaff` build and `git diff --check`
  PASS.
- 2026-07-17 Theron G8 FIFO sidecar lifecycle binding: added an immutable
  capture-only join between the validated G8 FIFO sidecar, the existing opaque
  artifact corpus source-trace MD5, and M11's current media scan epoch. It
  stores only G8 metadata and identities, is explicitly capture-required and
  no-draw, and clears on source-trace or lifecycle-epoch drift. It does not
  touch the closed loader-output consumer, import an artifact, or promote a
  dungeon route. Verification: focused G8 sidecar and lifecycle-binding CTests
  PASS in `build-theron-trace-md5`; `git diff --check` PASS.
- 2026-07-17 Theron G8 FIFO sidecar artifact/M11 capture-required hardening:
  M11 now accepts the G8 sidecar only against its exact lifecycle-bound opaque
  artifact-corpus copy. The capture-only receipt pins the artifact bundle and
  capture-plan identities, and alternate corpus instances clear it before any
  consumer, route, bitmap, or draw path can observe it. Verification: focused
  G8 sidecar and lifecycle-binding CTests PASS in `build-theron-trace-md5`;
  `git diff --check` PASS.
- 2026-07-17 Theron G8 FIFO capture-data binding: retained the existing
  source-backed G8 FIFO row's offset, reader/writer PCs, logical/physical
  destinations, and byte value as opaque capture-required/no-draw metadata.
  Its fingerprint is rechecked at lifecycle consumption, so altered retained
  capture data, source-trace, lifecycle, corpus, or capture-plan evidence
  rejects before M11 can retain the receipt. No loader-output consumer, route,
  bitmap, palette, decoder, or drawing path was added. Verification: focused
  G8 sidecar and lifecycle-binding CTests PASS in `build-theron-trace-md5`;
  `git diff --check` PASS.
- 2026-07-17 Theron G8 FIFO sequence/length/window binding: the exact
  source-backed one-row G8 capture now retains its FIFO sequence bounds,
  one-byte length, and half-open source window alongside a rechecked identity.
  That identity is carried only through the existing opaque artifact-corpus,
  capture-plan, and M11 capture-required lifecycle join; sequence, length, or
  window drift rejects before M11 retains it. No consumer converter, route,
  bitmap, palette, decoder, or drawing behavior was promoted. Verification:
  focused G8 sidecar and lifecycle-binding CTests PASS in
  `build-theron-trace-md5`; `git diff --check` PASS.
- 2026-07-17 Theron G8 FIFO capture-file identity binding: added the exact
  capture file's canonical MD5, full-file FNV-1a, and strict one-row count to
  a rechecked identity carried through the source-trace, opaque artifact
  corpus, capture-plan, and M11 capture-required lifecycle join. Capture-file
  MD5/FNV/count drift rejects before M11 retains the metadata. No consumer,
  route, bitmap, palette, decoder, or drawing behavior was promoted.
  Verification: focused G8 sidecar and lifecycle-binding CTests PASS in
  `build-theron-trace-md5`; `git diff --check` PASS.
- 2026-07-17 Theron G8 READ(6) capture-CDB binding: the canonical G8 capture
  file is now tied to the already disassembled sequence-4 `3840`/`1f1840`
  dispatch, `A/X/Y=20/ff/04`, and exact READ(6) CDB `08 00 12 fb 01 00`
  (LBA `4859`, one sector). Its capture-CDB identity is carried only through
  the current source-trace, opaque artifact-corpus, capture-plan, and M11
  capture-required lifecycle join. Callsite or CDB drift clears the active
  M11 receipt; no consumer, route, bitmap, palette, decoder, or drawing path
  was promoted. Verification: focused G8 sidecar and lifecycle-binding CTests
  PASS in `build-theron-trace-md5`; `git diff --check` PASS.

# CSB F0275 C011 wall-click save handoff (2026-07-17)

- Completed the source-owned ReDMCSB `MOVESENS.C F0275` C011 wall-click
  lifecycle. The production runtime-hand route consumes only the loaded C03
  sensor and matching C05 object, clears that leader-hand object, rotates the
  source cell, and schedules the existing F0272/F0268 fakewall path. Native
  reload now synchronizes an already boot-owned CSBWin GAMEBLOCK2 hand mirror
  from restored `PARTY.LeaderHandThing`, preventing divergent live hand state.
  A mismatched object type leaves the hand, raw bytes, and timeline untouched.
  No synthetic queue or UI path was added. Verification:
  `csb_v1_f0275_c011_wall_click_save_pc34_compat` PASS in
  `build-csb-verify`.

# CSB F0275 C012 generator save handoff (2026-07-17)

- Added focused lifecycle coverage for ReDMCSB `MOVESENS.C F0275` C012.
  The production runtime-hand route admits only an empty source-owned hand,
  allocates the bounded F0167 C05 arrow record, rotates the loaded C03 cell,
  and schedules F0272/F0268. Native reload retains the generated party hand,
  synchronizes the boot-owned CSBWin mirror, and preserves the pending event.
  A nonempty hand rejects before allocation, raw mutation, or timeline output.
  No synthetic queue or UI path was added. Verification:
  `csb_v1_f0275_c012_generator_save_pc34_compat` PASS in
  `build-csb-verify`.

# CSB title/Entrance capture admission (2026-07-17)

- Reconciled the signed C001 title receipt with the ReDMCSB TITLE.C F0437
  phase plan: frame 79/step 21 is the final CHAOS plan and frame 80/step 21
  is the first STRIKES plan. The new admission accepts only the matching
  frame, source step, phase, C001 rectangle, blit mode, and source palette;
  a relabelled capture is rejected before presentation. M11's real CSB-data
  boundary also consumes F0806's pre-open delay before publishing the first
  C002/C003 door frame. No synthetic title frame, palette, or Entrance image
  was added. Verification in `build-csb-verify`:
  `csb_v1_m11_launcher_handoff_boundary`,
  `csb_v1_m11_startup_resume_gate`,
  `csb_v1_title_capture_admission_pc34_compat`, and
  `csb_v1_startup_img3_decode_pc34_compat` PASS.

# CSB real C001 raster boundary correction (2026-07-17)

- Fixed the M11 render-view owner so it uses the signed TITLE.C F0437 stage,
  not the ambiguous source-step threshold, to select C001 geometry and
  palette. Steps 20 and 21 occur in the final CHAOS wave as well as at the
  first STRIKES boundary; the old threshold could therefore present a real
  CHAOS image using the STRIKES crop. The real local PC34 `GRAPHICS.DAT`
  sequence now proves frame 79's CHAOS crop/palette, frame 80's STRIKES
  crop/palette, and the subsequent C004/C002/C003 Entrance raster session.
  Verification in `build-csb-verify`: real startup sequence, M11 boundary,
  M11 resume gate, IMG3 decode, and title-capture admission tests PASS.

# CSB Entrance opening and first HUD palette admission (2026-07-17)

- Added a source-backed pre-frame palette admission at the F0438/F0807
  boundary. The C004/C002/C003 opening route accepts only the real CSB
  Entrance palette, while the first C017/C040 PANEL.C runtime frame accepts
  only neutral palette state after Entrance has released it. Rejecting before
  frame construction prevents a forged plan from changing session
  presentation metadata. The local PC34 `GRAPHICS.DAT` sequence verifies the
  final C004+C003 image, the first C017+C040 raster, and both wrong-palette
  rejects. Verification in `build-csb-verify`: real startup sequence,
  terminal-handoff, M11 boundary, and M11 resume-gate tests PASS.

# CSB first-runtime HUD and door-capture lifecycle (2026-07-17)

- Bound the PANEL.C C017/C040 raster consumer to a completed F0807 terminal
  session, and bound the F0438 C002/C003 opening capture to the preceding
  live Entrance stage. A pre-F0807 panel request and post-HUD opening capture
  now reject. Runtime HUD frame construction retains the recorded Entrance
  palette fact while using the neutral C017/C040 palette, so the terminal
  proof cannot be invalidated by its own consumer. The real local PC34
  `GRAPHICS.DAT` sequence compares the emitted C017 bytes directly and
  exercises both lifecycle rejects. Verification in `build-csb-verify`:
  real startup sequence, terminal-handoff, M11 launcher boundary, and M11
  resume-gate tests PASS.

# CSB M11 real C001 phase-capture lifecycle (2026-07-17)

- Replaced M11's route-derived title phase hashes with raster hashes captured
  from the verified active C001 session: PRESENTS frame 0, CHAOS zoom frame
  60, CHAOS hold frame 79, and STRIKES frame 80. Each source plan must pass
  the existing title admission and produce one real, non-legacy title host
  raster with the plan's palette; missing, duplicate, wrong-stage, or
  synthetic witnesses reject before release presentation. The M11 boundary
  regression compares every retained hash with the matching real source
  raster and rejects a mutated legacy wrapper hash. Verification in
  `build-csb-verify`: M11 launcher boundary, M11 resume gate, real startup
  sequence, and terminal-handoff real-data tests PASS.

# CSB startup VBlank host cadence (2026-07-22)

- M11 now schedules active CSB title and Entrance frames at the original
  20 ms VBlank cadence. Regular CSB gameplay retains its speed-adjusted
  200 ms source tick, so the correction is confined to C001/C004 startup.
- The focused cadence test and the real local CSB package startup-sequence
  regression pass.

# CSB F0275 C013 live-dungeon ownership (2026-07-22)

- The C013 front-wall bridge now requires the same loaded `Dungeon.dat` and
  active level that own the live CSB party before it can derive a sensor
  square or mutate an object chain. A stale global dungeon or level fails
  closed.
- The original-corpus regression verifies that stale ownership is rejected.
  It only exercises a positive C013 route when the supplied original dungeon
  actually contains one; the local corpus does not, so it reports `SKIP`.

# CSB F0275 C004 typed hand removal (2026-07-22)

- C004 now has its own matching-hand path: it consumes the original hand
  object and queues the normal F0272/F0261 target effect without borrowing
  C011/C017's final-same-cell condition.
- The focused runtime regression covers a later C03 in the same cell and
  verifies that neither sensor is rotated or removed.

# CSB title-to-Entrance runtime handoff (2026-07-22)

- M11 now captures the C001 STRIKES BACK sample at source frame 100, after
  the complete CHAOS hold, rather than the obsolete frame-80 boundary. The
  release-app receipt consequently remains valid for the real title session.
- When the terminal title tick hands control to ENTRANCE.C, M11 consumes that
  tick before accepting the first Entrance plan. This prevents the former
  black-screen rejection at the title/Entrance boundary.
- The real local CSB M12-to-M11 handoff regression passes 500/500.


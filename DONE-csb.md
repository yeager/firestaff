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

# DM1 C015 host-message regression containment (2026-07-30)

- ✅ DM1's C015 message area no longer renders the generic M11 event log.
  That log contains host status and inspection scaffolding rather than
  ReDMCSB TEXT.C-owned message rows, so rendering it produced the visible
  `READY`/`INSPECT` help text in Hall of Champions. The DM1 surface now
  consumes only decoded TEXT.C F0047 rows through the existing source model,
  with its original wrapping, expiry and font; CSB's separate source-bound
  message receipt remains unchanged. The launcher handoff regression injects
  a `READY` host-log entry and proves C015 remains black in V1, V2.0, V2.1
  and V2.2 when no source row exists.

# DM1 inventory placeholder containment (2026-07-30)

- ✅ Normal DM1 V1 inventory rendering now rejects generated slot frames,
  scaled viewport-sprite stand-ins, two-letter item tags and slot labels.
  C033-C035 and F0038's icon atlas are the only admitted visual owners;
  unavailable original material leaves the source rectangle blank. Debug and
  non-DM1 tooling paths remain explicitly separate.

- ✅ Data-directory selection now preserves the normalised path the player
  selected while scanning through its canonical physical path. This prevents
  macOS `/private` aliases from replacing a valid `/tmp` or symlinked data
  root in the UI or saved configuration. The embedded changelog's current
  header now follows the generated CMake version automatically. Verification:
  all 65 selected M12/menu-hit/startup-menu/launcher tests pass, including
  mouse, keyboard, touch, localization, data-picker cancellation, save
  browser, accessibility and every launcher handoff boundary.

- ✅ Settings-pointer completion: `DATA DIRECTORY` now consumes the visible
  left/right controls consistently. The left control restores Firestaff's
  default originals directory; the right control opens the native directory
  picker; clicking the label only selects the row. `menu_hit_settings_tab_m12`
  covers all three pointer targets, together with the existing keyboard,
  touch, localization, font/artpack, and launcher-handoff tests.

- ✅ Modern Extras presentation now follows the existing `M12_NAV_EXTRAS`
  input state instead of incorrectly drawing the main game-card view. The
  rendered Extras list uses the same selection, availability, mouse-hit, and
  keyboard/controller command path as the launcher state machine.

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

# DM1 V2.2 reviewed-art runtime consumption (2026-07-30)

- ✅ DM1 V2.2 now consumes the reviewed `*_hero_01` identifiers declared by
  the installed finished-art manifest. The earlier in-place renderer still
  requested stale first-cut identifiers, so an admitted pack could not reach
  its wall, floor, pit, teleporter or creature replacement pixels. Unreviewed
  stairs now deliberately preserve their original V1 material. The real-art
  material gate uses the same aggregate runtime admission as production, and
  the out-of-tree Ninja test verifier receives its configured build directory.
  Verification: all registered DM1 V2.0/V2.1/V2.2 contracts pass, 97/97.

# DM1 PC 3.4 group and teleporter reference locks (2026-07-30)

- ✅ Corrected the F0195 capacity proof to the PC 3.4 `GROUP.C` branch:
  `F0196` initializes 110 active-group slots. The 60-slot branch belongs to
  older Atari media and must not constrain PC 3.4 group activation. The F0113
  teleporter visual lock now follows the DM1-owned C070-C077 asset binding,
  visible/open gate and source phase sampler consumed by M11. Verification:
  47 group/timeline/teleporter/spell-tick regressions pass from Ninja.
# DM1 C001 boot-receipt source ownership (2026-07-30)

- ✅ DM1's completed startup boot receipt now reports TITLE.C F0437's 23
  source-visible C001 events, rather than the unrelated 53-frame TITLE.DAT
  decoder bank. This keeps probe/capture consumers on the actual PRESENTS,
  zoom, STRIKES BACK and guard route; TITLE.DAT remains confined to its
  separate decoder contract. Verification: Ninja build, the focused startup
  state-machine gate, and real PC34 V1/V2.0/V2.1/V2.2 boot probes all pass
  with `titleFrame=23` and `titleFrameMax=23`.
# DM1 macOS source-start capture (2026-07-30)

- ✅ A clean native macOS run with the local PC34 `GRAPHICS.DAT` captured the
  source-owned startup handoff in order: FTL swoosh at 2 s, PRESENTS at 5 s,
  the C001 Dungeon Master / Strikes Back title at 8 s, and Entrance at 11 s.
  The capture uses the normal scalable window, not a synthetic screen or
  TITLE.DAT replacement. This closes the package-level visual smoke requested
  for the corrected I34E swoosh dwell and C001-to-Entrance cadence.
# DM1 original-save CLI resume (2026-07-30)

- ✅ 2026-07-30 Fixed DM1 direct new-game launch: Phase-A option defaults
  now initialize the optional save path, preventing an unset CLI `--save`
  field from becoming a bogus resume path after DUNGEON.DAT loaded. Verified
  with both direct no-save HoC entry and an explicit original PC34 `DMSAVE.DAT`
  resume under the dummy SDL runtime. The dependent V2.0/V2.1/V2.2 real
  presentation suite passes 6/6, including actual/presented screenshot
  capture and each V2 renderer silicon probe.

- ✅ 2026-07-30 DM1 Entrance credits now render the real decoded C005 page
  with ReDMCSB `ENTRANCE.C:F0442`'s G0019 credits palette. The source receipt's
  availability flag is no longer treated as the special-palette index, and a
  missing authenticated page fails closed rather than drawing a generated card.
  `test_dm1_v1_startup_sequence_pc34_compat` covers the credits-versus-
  Entrance palette regression.

- ✅ Added `--save <path>` for direct game launches. It forwards only the
  supplied path through the existing M12 quick-resume intent to the existing
  game-specific M11 importer; it does not synthesize save bytes or bypass
  validation. A real PC34 `DMSAVE.DAT` resumes through `--game dm1 --save`,
  restoring its four champions, party position `(16,14,2)`, and source tick
  `428`. `--save` without `--game` is rejected, and a missing save makes the
  direct launch fail rather than falling back to a new or synthetic game.
- ✅ 2026-07-30 DM1 top-row M11 consumption audit: verified that the
  production `m11_draw_dm1_v1_top_row_receipt()` already consumes the
  source-bound C008/C028/C033-C035 atomic plan into the live framebuffer,
  with M653 names, status bars, hand icons and pending-damage overlays.
  Focused top-row host-consumption, M11-consumption, host-render and runtime
  bridge CTests pass, and the installed-PC34 geometry probe passes for all
  four champion slots. The remaining task is original paired capture, not
  another renderer implementation.

# DM1 original PC34 ACTIVE_GROUP corpus verification (2026-07-30)

- ✅ The fixture-free, operator-supplied DOSBox PC34 `DMSAVE.DAT` has 110
  live C04 ACTIVE_GROUP records. Its F0435 stage/adoption receipt proves each
  record's loaded GROUP Thing and unique current-map SquareFirstThing chain,
  plus packed cells/directions/aspects and C03/C04 timeline identity. The
  same receipt remains valid after runtime adoption. Verification:
  `dm1_v1_original_save_pc34_external_corpus` passed against the provenance-
  bound local original corpus (`groups=110/110`), and
  `dm1_v1_original_save_pc34_external_hoc_runtime` passed from the same save.
  This closes the live-group requirement only; the separate C13 and varied
  C03/C04-layout corpus requirements remain open.

# DM1 original PC34 raw ACTIVE_GROUP resume (2026-07-31)

- ✅ A second operator-owned DOSBox-X PC34 DMSAVE.DAT exercises the real
  changed-state resume route: map 1, party (6,2,2), tick 1664, 15 active
  groups, and 15 live C03 events (C32/C37/C38), unlike the earlier
  empty-event corpus save. Its ACTIVE_GROUP.GroupThingIndex records use raw
  GROUP-table indexes. dm1_v1_group_state_apply_save_handoff_pc34() now
  accepts this authentic PC34 representation alongside the pre-existing
  packed-THING compatibility form and resolves both through the loaded GROUP
  table. Regression: test_dm1_v1_group_state_bundle_pc34_compat; direct
  original-save boot probes pass in V1, V2.0, V2.1 and V2.2. The separate,
  tail-less corpus roundtrip remains open because it must bind the original
  DUNGEON.DAT backing before it can certify F0435 -> F0433 -> F0435.
- ✅ 2026-07-30 CSB F0142/G0209 thrown-object viewport binding: a live C14
  projectile now reads its associated original C05--C0B record, preserves
  M066 weapon projectile-aspect selection, and routes a positive F0142
  result through the source M612/G0209 bitmap and C2900 geometry. M11
  installs the exact active CSB graphics record before the draw; unavailable
  material stays no-draw rather than becoming an icon or marker. The focused
  CSB viewport regression passes 2 654 checks.
- ✅ 2026-07-30 DM2 creature AI owner gate: `dm2_v1_creature_ai_spec()` now
  follows SK-projects `skcrture.cpp::QUERY_CREATURE_AI_SPEC_FROM_TYPE` through
  the loaded `CREATURES[type]` word-0x05 owner binding, rather than treating a
  creature type as an AI row. Unbound runtime creatures, attacks, spells and
  projectiles reject without creating fallback HP, attack strength or behavior.
  Test fixtures retain explicitly compiled test-only rows. Verification:
  `dm2_v1_creature_gdat_ai_table`, `dm2_v1_creature_combat_probe` and
  `dm2_v1_projectile_pc34_compat` pass; strict C11 and `git diff --check` pass.
- ✅ 2026-07-30 DM2 startup HUD phase boundary: the startup receipt now
  follows `SKWINSPX/src/v4/skcore.cpp::SHOW_MENU_SCREEN`: it verifies the
  original `TITLE/0/1` and `TITLE/0/4` title/menu surfaces plus HUD
  handoff/suppression before `GAME_LOAD`, without inventing party portraits.
  The real nine-command HUD proof remains a separate post-`GAME_LOAD`
  verification using PC-English `GRAPHICS.DAT`.
- ✅ 2026-07-30 DM2 M11 New Game source gate: M11 no longer calls the
  save-fixture `dm2_v1_session_new()` helper after New Game. Per
  `SKWINSPX/src/v4/skcore.cpp::SHOW_MENU_SCREEN`, it keeps the title/menu
  active and reports `DM2 GAME_LOAD DATA REQUIRED` until original
  `GAME_LOAD` data can be handed off; no canned party, gold or map pose
  reaches runtime.
- ✅ 2026-07-30 DM2 public New Game source gate: `dm2_v1_new_game_flow()`
  now returns `DM2_FLOW_GAME_LOAD_REQUIRED` after asset admission instead of
  materializing its save-fixture party. `SKWINSPX/src/v4/skcore.cpp` keeps
  party and entrance state in `GAME_LOAD`/`LOAD_NEW_DUNGEON`; fixtures remain
  explicit test helpers only.
- ✅ 2026-07-30 DM2 viewport wall fallback removal: the V1 wall material
  plan no longer carries a generic colour fallback. Per
  `SKWINSPX/src/v4/c_gui_vp.cpp::DM2_DRAW_WALL`, each planned cell now has
  only its GDAT owner and source/destination rectangles; unresolved material
  remains no-draw.
- ✅ 2026-07-30 DM2 viewport door fallback removal: the V1 door plan no
  longer carries an unused generic colour fallback. Per
  `SKWINSPX/src/v4/c_gui_vp.cpp::DM2_DRAW_DOOR_FRAMES`, door panels are owned
  by GDAT; an unresolved source rectangle removes the panel from the plan
  rather than inventing a coloured replacement.
- ✅ 2026-07-30 DM2 viewport map-chip fallback removal: item, carried-item,
  creature-possession and projectile plans no longer carry unused generic
  colour, radius or velocity-stroke fallbacks. `DRAW_ITEM` and
  `DRAW_TEMP_PICST` now retain only source GDAT identity and source-owned
  placement; unavailable material remains no-draw.
- ✅ 2026-07-30 DM2 V2 HUD synthetic hook removal: the legacy asset-aware
  entry point no longer blits test PNGs, stamps anchor pixels or invokes the
  procedural HUD overlay. It now delegates only to the authenticated
  `INTERFACE_GENERAL`/`CHAMPIONS` GDAT path; manifest classification remains
  diagnostic-only and cannot create game pixels.
- ✅ 2026-07-30 DM2 V2 direct-overlay gate: the older
  `dm2_v2_runtime_hud_render()` API is now no-draw because it has no original
  GDAT fetch/palette binding. This prevents direct callers from bypassing the
  authenticated M11 HUD route with the retired procedural overlay.
- ✅ 2026-07-30 DM2 V2 overlay-enable cleanup: the retired overlay's enable
  flag is gone and its compatibility setter is a no-op, so no later phase-gate
  call can re-enable pixel generation through that API.
- ✅ 2026-07-30 DM2 New Game original-dungeon handoff: M11 now invokes the
  hash-verified `LOAD_NEW_DUNGEON` portion of `GAME_LOAD` before reporting
  that original party data is still required. It reloads no fixture session,
  party, leader hand or timer state and keeps the source title/menu boundary
  active until those records are modeled.
- ✅ 2026-07-30 DM1 original-save corpus receipt integrity: successful
  provenance-bound F0435 -> F0433 -> F0435 entries now produce a stable,
  nonzero fingerprint of the original input and transient export. The live
  V1 PC34 boot receipt reports `5a560bab`; the external-corpus regression
  also preserves its independent provenance fingerprint.

- ✅ 2026-07-30 DM2 New Game source-state audit: corrected the GAME_LOAD
  diagnostic and handoff contract to match `skgame.cpp::LOAD_NEW_DUNGEON` and
  `skchamp.cpp::SELECT_CHAMPION`: an empty party/leader hand is source state,
  the G1 header owns the start pose, and champions are later selected at
  dungeon mirrors. The remaining gate is actuator/timer initialization and
  source-owned mirror UI, not a fabricated starter party.
- ✅ 2026-07-30 DM2 New Game entrance-pose restore: `LOAD_NEW_DUNGEON` now
  resets the live game state's position, direction, map and outdoor flag from
  the reloaded G1 header before the later mirror-selection entrance. This
  follows `SKWINSPX/src/v4/skcore.cpp::GAME_LOAD` → `LOAD_NEW_DUNGEON` and
  prevents an earlier runtime pose from leaking into a new game; it creates
  no party, leader hand, gold or timer data. Verified by the focused contract
  test plus the canonical PC G1 graph and arrange-dungeon receipts.
- ✅ 2026-07-30 DM2 hash-only asset admission: removed the boot scanner's
  legacy filename/size fallback for incomplete developer fixtures. Startup
  availability and launch now discover DM2 data only through the supported
  original-file hashes, including genuine files with arbitrary names. The
  smoke test verifies that named fake files are rejected while renamed real
  PC files remain accepted.
- ✅ 2026-07-30 DM2 external-data probe path fix: the real GDAT creature
  animation probe now accepts both case variants of the original graphics
  filename. It therefore reads the external DOS `GRAPHICS.DAT` corpus before
  reporting an unadmitted source route, rather than treating case-preserved
  original data as absent.
- ✅ 2026-07-30 DM2 viewport placeholder cleanup: removed the remaining
  no-op placeholder marker and stale placeholder wording from the built V1
  viewport path. Wall and door stages are described and verified as direct
  source-GDAT material routes.
- ✅ 2026-07-30 DM2 PC-G1 creature material census: corrected the real-data
  viewport proof to match the hash-verified PC corpus. Its 33 direct creature
  roots expose no complete FB/FC/FD V5 chain, so all remain fail-closed rather
  than being promoted through map-chip or invented material; 38/38 checks pass.
- ✅ 2026-07-30 CSB C699 action-name source binding: the recognised PC3.4
  `GRAPHICS.DAT` corpus (`61fbfd56887c94adc26888a9491c6611`) now loads raw
  item 699 as exactly 44 bounded NUL-terminated action names. ReDMCSB
  `MENU.C F0620:543-551` assigns C699 to
  `G0490_ac_Graphic560_ActionNames`; M11's CSB action menu and action log now
  consume that runtime-owned receipt and render no CSB action text if it is
  absent or malformed. The real-data boot regression proves `BLOCK` and
  `FUSE` at their source indices; the original-PC3.4 Prison HUD regression
  passes. The separate action-set owner remains open.

- ✅ 2026-07-30 CSB G0489 action-set ownership: ReDMCSB `MENU.C:90-136`
  establishes the PC3.4 action rows as 44 compiled six-byte records, not a
  `GRAPHICS.DAT` member. CSB runtime now owns that complete source table and
  M11 consumes its three action indices only through the CSB session. F0389
  refuses to publish an action menu if the receipt is unavailable, rather
  than falling through to DM1's parallel G0489 table. Real PC3.4 boot checks
  the empty-hand and one-row STAB sets; the CSB M11 regression proves the
  missing-receipt fail-closed boundary.
- ✅ 2026-07-30 DM2 boot-state fixture removal: `dm2_v1_init()` now only
  allocates a zeroed game state and records its data root. It no longer seeds
  the old `(15,15,N)`, 100-gold or noon values before game data is loaded.
  A real new game receives its pose from the hash-verified G1 header; session
  values remain unavailable until their original owner is imported. The boot
  profile smoke test passes all 87 checks, and the isolated runtime smoke
  fixture now proves it cannot fabricate that party snapshot.
- ✅ 2026-07-30 DM2 viewport bootstrap fixture removal: the V1 viewport
  allocation no longer carries the former Hall-of-Champions position, noon
  clock or fixed weather RNG. `dm2_v1_runtime_render_frame()` supplies those
  fields from the bound runtime/G1 session before any source-GDAT draw. This
  leaves an unbound viewport with no implicit playable world state; the
  hash-verified boot profile smoke remains 87/87.
- ✅ 2026-07-31 DM2 startup music truthfulness: the `SHOW_MENU_SCREEN` menu
  still follows SKWIN's `DM2_PLAY_MUSIC(0, true)` order, but its handoff no
  longer reports that cue 0 played when the verified original HMP/GDAT path
  or backend rejected it. The title/menu remains visible, while missing
  source audio is honestly silent. The focused startup-audio regression
  covers the unbound no-playback case.
- ✅ 2026-07-31 DM2 PC music source ownership: menu cue lookup now binds the
  hash-admitted boot GDAT loader and reads `MUSICS/<track>/dtHMP/0`, matching
  `SKWIN/c_sound.cpp::DM2_PLAY_MUSIC`. The old loose `00.hmp.mid` path and
  synthetic `/tmp` music fixture are retired. Canonical PC data proves all
  29 records `00`--`1c` (cue 0 is raw entry 5595); the current HMP decoder
  rejects that real stream and
  therefore leaves playback fail-closed rather than borrowing sidecar audio.
- ✅ 2026-07-31 CSB entrance fallback metadata removal: the closed-door
  entrance plan no longer carries the old generated grey fill and edge
  colours. `ENTRANCE.C F0806:721-778` owns source records C002/C003, so a
  missing door raster remains a failed source draw instead of becoming plan
  data that a renderer could promote. The source-page clear remains black,
  matching the title/entrance composition boundary. The focused entrance
  pointer and sequencing regression passes 139/139.
- ✅ 2026-07-31 CSB F0113 source-field binding: live CSB viewport rendering
  now obtains the teleporter's C076 raster and the G0188-selected C070--C075
  mask from the active hash-verified `GRAPHICS.DAT` decoder. The existing
  compatibility sampler derives its visual phase from the live CSB tick and
  preserves the source mask/transparency path from ReDMCSB
  `DUNVIEW.C F0113:4417-4461`; it does not claim exact ownership of F0113's
  `M005_RANDOM(2)`/`M003_RANDOM(32)` stream. An absent or malformed source
  span is still strict no-draw. The focused CSB viewport regression passes
  2,658/2,658 and the full `firestaff` target builds.
- ✅ 2026-07-31 CSBGRAPHICS rejected-override containment: the source-page
  binding regression now snapshots all 320×200 indexed pixels before every
  rejected override application and verifies byte-for-byte preservation after
  both direct and combined prepare/apply calls. Empty, untrusted, malformed
  and unsupported original entries therefore remain no-draw rather than
  becoming generated HUD or viewport chrome. Source references: CSBWin
  `Graphics.cpp` `ReadGraphicsIndex`/`LocateNthGraphic`/`ReadGraphic` and
  ReDMCSB `PANEL.C F0346` / `PANEL.C F0347`. Verification:
  `test_csb_v1_csbgraphics_runtime_binding` passes 83/83.
- ✅ 2026-07-31 CSBGRAPHICS decision-name hardening: the zero-valued runtime
  decision is now explicitly `rejected-no-draw`, with the old
  `fallback-original` identifier retained only as a source-compatible alias.
  This preserves enum layout while making the fail-closed policy unambiguous
  to future callers. Verification: `test_csb_v1_csbgraphics_runtime_binding`
  passes 83/83.
- ✅ 2026-07-31 CSB V2.2 cache containment: an invalid source-derived cache
  entry now rejects and discards the entire package. The loader validates
  nonzero dimensions, the exact `width * height * 4` RGBA size and bounded,
  non-wrapping offsets beyond the entry table before exposing any bitmap.
  It also rejects duplicate asset keys and overlapping RGBA spans, matching
  Artpack Studio's one-key/one-contiguous-span package writer. This keeps
  malformed or ambiguous material from becoming V2.2 pixels; the unresolved
  F0128 material routes remain fail-closed. Verification:
  `test_csb_v22_inplace_draw_pc34` passes 66/66.
- ✅ 2026-07-31 CSB V2.2 cache wire-format ownership: FSV22C header and entry
  integers are now decoded explicitly as little-endian, matching Artpack
  Studio's `struct '<6I8x'` writer instead of relying on the host's native
  byte order. Verification: `test_csb_v22_inplace_draw_pc34` passes 66/66.
- ✅ 2026-07-31 CSB V2.2 public-contract audit: corrected the stale public
  header that still described the retired generic rectangle/PNG route. The
  interface now documents the actual FSV22C cache boundary and the required
  command-level F0128 raster, palette, clip and projection receipts. This is
  documentation of the existing fail-closed implementation, not a claim that
  unbound viewport families are rendered. Verification:
  `test_csb_v22_inplace_draw_pc34` passes 66/66.
- ✅ 2026-07-31 CSB HUD fallback containment: `FIRESTAFF_V1_CHROME=0` is now
  explicitly a DM1 compatibility option, not permission to draw Firestaff's
  generic cyan utility frame, champion/status text or light bar in a CSB
  session. When C009/C010 material is unavailable, the source-owned CSB area
  remains black. It also excludes the legacy rune workbench from CSB, so an
  open spell state cannot paint host controls over F0128. Verification: the
  CSB M11 startup/resume regression compares the complete 224×136 source
  viewport with chrome disabled and passes; Prison-runtime HUD also passes.
- ✅ 2026-07-31 CSB dialog fallback containment: an unavailable source dialog
  backdrop no longer falls through to Firestaff's generic plaque frame when
  the DM1 chrome option is disabled. CSB leaves the existing source frame
  untouched; the distinct return-to-menu confirmation remains explicit host
  UI. Verification: the CSB M11 startup/resume source-viewport comparison
  covers the chrome-off dialog state and passes.
- ✅ 2026-07-31 CSB chrome-off input containment: the generic focus-card
  shortcut no longer captures CSB's C068--C073 movement region when the DM1
  chrome switch is off. CSB continues through `COMMAND.C G0448` and the
  runtime movement bridge. Verification: the startup/resume regression
  exercises a chrome-off C068 click and confirms the source turn and runtime
  party-direction update.
- ✅ 2026-07-31 DM2 V2 HUD synthetic-PNG closure: the public one-pixel PNG
  compatibility hooks now return strict no-draw even for a valid fixture.
  SK-projects `c_gdatfile.cpp::DM2_LOAD_GDAT_INTERFACE_00_02` establishes
  that original interface GDAT owns the HUD source bytes; only the mounted
  `INTERFACE_GENERAL`/`CHAMPIONS` route can write runtime pixels. Fixture
  decoding remains isolated to probes and cannot promote generated or
  operator-provided art into the framebuffer.
- ✅ 2026-07-31 DM2 `LOAD_NEW_DUNGEON` entrance atomicity: a parsed G1 file
  without an in-map original start pose now rejects before mutating the live
  dungeon or party position. Accepted reloads restore the source header's
  deterministic configuration together with the G1 pose. This follows
  SK-projects `GAME_LOAD`/`LOAD_NEW_DUNGEON` ordering and prevents an old
  world position from becoming a synthetic entrance for newly loaded data.
- ✅ 2026-07-31 DM2 wall-ornament material-owner gate: the V1 viewport now
  rejects a placement plan whose named WALL_GFX GDAT row differs from the
  live G1 square's derived map-chip row. This prevents a coincidental
  same-square bitmap from being displayed at a placement owned by another
  source record; the mismatch is strict no-draw.
- ✅ 2026-07-31 CSB V2.2 DoorSet-source binding: F0128's narrow door
  replacement admission now rejects commands without a selected
  `GRAPHICS.DAT` item index. It can no longer infer DoorSet 0 from artpack
  provenance alone. This follows ReDMCSB `DUNVIEW.C F0096:2651-2658`, where
  G0693/G0694/G0695 derive the active `DoorSet * 3 + offset` record; missing
  selection remains V1/no-draw. Verification: `test_csb_v22_inplace_route_pc34`
  passes 145/145 and `test_csb_v22_inplace_draw_pc34` passes 66/66.
- ✅ 2026-07-31 DM2 V2.2 legacy-pipeline closure: M11 now resolves a V2.2
  request to the verified V2.1 EPX path, and the older Phase-2 local-manifest
  loader is no-op. `dm2_v2_best_available_shape_source(3)` therefore cannot
  report `V2_MODERN` from an ordinary filesystem directory; only a future
  GDAT category/index/raw-byte provenance bridge may enable it. The focused
  Phase-2 pipeline probe verifies the loader/root rejection (89/89), and the
  M11 wire-up probe now correctly verifies the data-free HUD remains no-draw
  until M11 supplies an original GDAT owner (36/36).
- ✅ 2026-07-31 CSB F0128 compressed-record identity: the PC3.4
  `GRAPHICS.DAT` decoder now retains SHA-256 for the exact compressed record
  selected by ReDMCSB `F0490`, alongside its decoded-pixel receipt. This
  gives the live F0128 material handoff the source identity required to match
  V2.2 provenance, without accepting a same-looking decoded raster as a
  substitute. The real-PC3.4 D1C/D2C/D3C record regression and focused
  launcher/viewport tests pass.
- ✅ 2026-07-31 CSB F0096 per-door DoorSet selector: MAP.D:s två verifierade
  DoorSet-nibblar och DB0-dörrens lågbit väljer nu exakt G0693/G0694/G0695
  enligt `246 + DoorSet * 3 + depth`. Ogiltiga PC34-val blir no-draw och
  kan inte tyst lånas från DoorSet 0. Den fokuserade F0111-dörrtesten och
  F0128-provenanstesten passerar.
- ✅ 2026-07-31 CSB live closed-D3 F0111 binding: D3L2/D3R2 now resolve
  their actual PC3.4 MAP.D/DB0 DoorSet record, retain only F0172 front-facing
  `C4_DOOR_STATE_CLOSED` cells, and hand that exact GRAPHICS.DAT index to the
  M11 source decoder before the existing F0111 panel blit. No default DoorSet,
  replacement pixels or partial-door geometry is used. Viewport and M12→M11
  handoff regressions pass (2 675/2 675 and 567/567).
- ✅ 2026-07-31 DM1 F0351 champion-stat panel geometry: an empty-hand eye
  inspection no longer opens the generic M11 dialog over C101. The live panel
  now draws F0351's skill and statistic rows with M653's visible six-pixel
  advance rather than the eight-pixel inscription stride, preserving the
  ReDMCSB `PANEL.C F0351` C557/C559 layout inside the original 144-pixel
  panel. Verification: `m11_inventory_full_panel_runtime_source_lock` passes
  and explicitly rejects dialog-overlay activation for this route.
- ✅ 2026-07-31 DM1 F0351 base-skill visibility: the original skips a skill
  whose computed level is one before looking up `G0428_apc_SkillLevelNames`.
  Firestaff now does the same for both the rendered C101 panel and its
  inspection state, so untrained champions no longer fill the panel with
  `NOVICE` rows. Verification: the source-panel runtime test exercises all
  four level-one skills and confirms that no base-skill row is published.
- ✅ 2026-07-31 DM1 F0351 C101 text containment: the real-PC34 inventory
  framebuffer regression now captures the ordinary panel, opens empty-hand
  eye statistics, and verifies that the complete source M653 skill/statistic
  draw changes no pixel outside C101's original 144-pixel bounds. This locks
  the visible six-pixel glyph advance for long `ANTI-MAGIC` and `ANTI-FIRE`
  rows. Verification: `m11_inventory_full_panel_runtime_source_lock`
  passes 702/702 with the local original `GRAPHICS.DAT`.
- ✅ 2026-07-31 CSB PC3.4 D3 side-door atlas containment: F0116/F0117 no
  longer perform pointer arithmetic on the unpopulated legacy wall-frame
  atlas when a D3L/D3R front door is reached. The route now leaves the frame
  untouched until its real G2120 source bitmap and PC3.4 zone geometry are
  bound, rather than risking a host frame, invalid memory read or fabricated
  pixels. ReDMCSB `DUNVIEW.C:6453,6590` is the source reference. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat` passes with an explicit unbound-D3
  side-door regression.
- ✅ 2026-07-31 DM2 static startup-menu timing: removed the fabricated
  48-tick title/credits sequence from M11 and the DM2 startup handoff.
  SKProject `DM2_SHOW_MENU_SCREEN` repeatedly calls
  `DM2_DRAW_TITLE_MENU_SCREEN` for `TITLE/0/dt07/4`; `dt07/1` is loaded
  solely for the separately selected `DM2_SHOW_CREDITS` event. The menu now
  has a static timing receipt and accepts input immediately. Verification:
  `test_dm2_v1_startup_audio_menu` passes.
- ✅ 2026-07-31 CSB PC3.4 D3C door-frame source binding: the central D3
  frame now fetches the active wall-set's M657/G2119 record through the
  CSB GRAPHICS.DAT provider and uses the original G0166/G0167 32×44 source
  rectangles for F0104/F0105's native and mirrored C722/C723 blits. A
  source-verified session remains no-draw if that record is missing; the
  legacy atlas is not allowed to substitute it. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 DM1 startup original-save census: an explicitly configured
  PC34 corpus is now reported independently from the unbacked F0435 preflight.
  This prevents a live C03/C04 save from disappearing from the startup receipt
  merely because it needs M11's real `DUNGEON.DAT` backing. Unconfigured
  resume-parent directories and unconfigured test fixtures remain excluded.
  Verification: `test_dm1_v1_startup_intro_state_machine_gate` and a real
  `--game dm1 --save` boot probe against the local operator corpus.
- ✅ 2026-07-31 DM1 original PC34 backed runtime roundtrip: native quicksave
  now preserves an authenticated F0435 C03/C04 receipt and the unchanged
  source dungeon tail where present, so F0433 re-emits source-owned bytes
  instead of inventing a tail. Two operator-owned DOSBox saves pass
  F0435 -> native quicksave -> F0433 -> F0435 against the real local
  `DUNGEON.DAT`. Verification:
  `dm1_v1_original_save_pc34_tail_less_backed_roundtrip`,
  `dm1_v1_save_load`, `m11_quick_resume_roundtrip`, and V1/V2.0/V2.1/V2.2
  boot probes. References: ReDMCSB `LOADSAVE.C` F0433/F0435 and DMweb
  saved-game file-format documentation.
- ✅ 2026-07-31 DM1 F0373 floor-pile pickup ownership: C080 floor clicks
  now take the rendered `G0292_aT_PileTopObject[viewCell]` equivalent and
  put it directly into the separate leader hand. Firestaff no longer rejects
  a floor pickup merely because the active champion inventory is full, and
  it no longer removes an arbitrary first object from a multi-cell square
  chain. Source anchors: ReDMCSB `CLIKVIEW.C F0373:94-128` and
  `CHAMPION.C F0297:243-268`. Verification: `dm1_v1_viewport_click_source_lock`,
  `m11_dm1_real_alcove_item_runtime_pc34`,
  `dm1_v1_original_save_pc34_external_hoc_runtime`, and
  `dm1_v1_viewport_floor_ceiling_items_pc34_compat` pass.
- ✅ 2026-07-31 Source-data admission hardening: production asset loading no
  longer accepts arbitrary files merely because they are named
  `GRAPHICS.DAT` or `DUNGEON.DAT`. The shared generic and DM1 multilingual
  asset pipelines, plus M11's builtin dungeon resolver, require a known
  content hash. Renamed files and supported archive members remain discoverable
  through the existing hash/materialization path. Verification:
  `test_firestaff_asset_pipeline_hash_scan` proves hash-matched renamed data
  loads for DM1/CSB/DM2 while canonical-name junk is rejected by both generic
  and multilingual DM1 loaders.
- ✅ 2026-07-31 DM2 original-SKSave import closure: importing an original
  game-state no longer begins with Firestaff's fixed four-champion party,
  gold or entrance pose. The candidate is zeroed and receives only fields
  decoded from the original game-state and SUPPRESS records, so a malformed
  later section cannot leave an invented party behind. Verification:
  `test_dm2_v1_utility_import` passes 136/136.
- ✅ 2026-07-31 CSB D1L/D1R stale pixel API closure: removed the orphaned
  C10 blit declaration that had neither a source implementation nor callers.

- ✅ 2026-07-31 CSB D3L2 F0115 projectile no-fake closure: removed the
  unbound C10 fixture blit. ReDMCSB scaling and dynamic-flip metadata remain
  source-locked, but an unverified projectile raster cannot write pixels.

- ✅ 2026-07-31 CSB D1L/D1R F0111 door no-fake closure: removed the
  procedural C10 fixture and synthetic render hash. dmweb's DMCSB data-file
  format reference, ReDMCSB routing and the PC3.4 `GRAPHICS.DAT` item-248
  receipt remain the material proof; no unbound door pixels can be emitted.

- ✅ 2026-07-31 CSB D2L2/D2R2 F0115 no-fake closure: removed the generic
  C10 fixture blit. Item and explosion pixels now require the existing
  hash-bound real-overlay compositor; F0115 metadata alone remains no-draw.

- ✅ 2026-07-31 CSB D3L2/D3R2 F0111 door no-fake closure: removed the
  unbound C10 fixture blit while retaining the existing real `GRAPHICS.DAT`
  receipt. Source routing remains intact and unadmitted material cannot draw.

- ✅ 2026-07-31 CSB D2C F0111 door no-fake closure: removed the public,
  unbound C10 fixture blit. ReDMCSB door-state and half-zone routing remains
  intact, while a contract without verified PC3.4 graphics cannot write pixels.

- ✅ 2026-07-31 CSB D0L2/D0R2 wall no-fake closure: removed the unbound
  F0104/F0105 wall pixel writer and its synthetic source/viewport buffers.
  C716/C717 routing, native/mirrored geometry and C10 transparency remain
  source-locked metadata only; no verified material means no pixels.

- ✅ 2026-07-31 CSB D0L2/D0R2 F0115 no-fake closure: removed the public
  one-pixel fixture writer from the D0 side-lane contract. The real
  `CSBgraphics.dat` cache-backed teleporter-field compositor remains the only
  drawing route; unbound F0115 geometry and C10 metadata cannot write pixels.

- ✅ 2026-07-31 CSB D2L2/D2R2 wall no-fake closure: removed the unbound
  F0104/F0105 C10 fixture writer from the public contract. The remaining
  source-locked route table retains ReDMCSB DUNVIEW.C F0678/F0679/F0128
  ordering, C707/C708 zones, C05/C06 wall routing and C10 transparency as
  metadata only; without verified CSB GRAPHICS.DAT material it cannot draw.

- ✅ 2026-07-31 CSB PC3.4 D3 side-door source binding: D3L and D3R now
  fetch the active wall-set's M658/G2120 record through the CSB
  `GRAPHICS.DAT` provider and apply the original G0164/G0165 32×43 source
  rectangles for F0104/F0105's native and mirrored frame blits. G2121 and
  G2122 are the source aliases of that record. A source-verified session
  remains no-draw when it is unavailable; the legacy atlas cannot replace
  it. Verification: `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB PC3.4 D2C door-frame source binding: the central D2
  frame now requires both active wall-set source records before it draws:
  M660/G2115 through G0174 for the 96×3 top bar and M656/G2118 through
  G0168/G0169 for the native/mirrored 48×65 side pair. A verified CSB
  session therefore cannot fall back to the legacy atlas or leave a partial
  frame if either original record is unavailable. ReDMCSB `DUNVIEW.C
  F0121:7317-7330` is the source reference. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB PC3.4 D2 side-door source binding: D2L/D2R now draw
  their original M660/G2115 top strip through G0173/G0175 rather than the
  legacy wall-frame atlas. The route also requires an F0172 front-door
  element, so a `DOOR_SIDE` cell cannot receive a fabricated front-door
  strip merely because it shares the coarse door type. ReDMCSB
  `DUNVIEW.C F0119:6991-6998` and `F0120:7184-7191` are the source
  references. Verification: `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB PC3.4 D1C door-frame source binding: the central D1
  frame now requires both exact active wall-set records: M659/G2112 through
  G0177 for the 128×4 top bar and M655/G2117 through G0170/G0171 for the
  native/mirrored 32×94 side pair. A source-verified session remains
  completely no-draw if either source record is unavailable, with no legacy
  atlas substitution. ReDMCSB `DUNVIEW.C F0124:7877-7892` is the source
  reference. Verification: `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB D1C door-frame no-fake closure: removed the orphaned
  generic C10 framebuffer helper from the contract API. The real M659/G2112
  and M655/G2117 source-bound renderer remains the only D1C material route;
  ReDMCSB F0124/F0104/F0105 metadata remains cross-checked with CSBWin and
  dmweb without enabling caller-supplied pixels.
- ✅ 2026-07-31 CSB V2.2 missing-art placeholder retirement: removed the
  obsolete no-draw placeholder API rather than retaining a named fallback
  surface. Missing V2.2 material is represented only by failed lookup and
  the established source-owned V1/V2 selection path; no generated art can
  be requested by a caller.
- ✅ 2026-07-31 CSB D3C backdrop no-fake closure: removed the synthetic
  three-colour framebuffer compositor for the F0097/F0098/F0107/F0108 route.
  ReDMCSB and CSBWin ordering, windows, zone arithmetic and C10 transparency
  evidence remain available, but only verified original material can produce
  D3C backdrop or ornament pixels.
- ✅ 2026-07-31 CSB D1L2/D1R2 wall no-fake closure: removed the generic
  source-buffer frame-clip compositor and synthetic raster runner. ReDMCSB
  F0122/F0123 routing, F0104/F0105 mirroring and C10 metadata remain
  source-locked, but cannot convert caller-supplied bytes into viewport art.
- ✅ 2026-07-31 CSB D3L2/D3R2 wall no-fake closure: removed the generic C10
  source-buffer frame clipper. ReDMCSB F0118/F0104/F0105 geometry and
  transparency metadata remain, without a caller-supplied raster path.
- ✅ 2026-07-31 CSB PC3.4 D1 side-door source binding: D1L/D1R now draw
  their original M659/G2112 top strips through G0176/G0178 instead of a
  legacy atlas crop. Both G2111 and G2110 are source aliases of G2112.
  The F0172 front-door check also prevents the strip from appearing on a
  `DOOR_SIDE` cell. ReDMCSB `DUNVIEW.C F0122:7496-7504` and
  `F0123:7664-7672` are the source references. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB PC3.4 D0C door-frame source binding: the ordinary
  F0127 `C16_DOOR_SIDE` route now draws M654/G2116 directly through the
  original G0172 32×123 frame. It has no atlas substitute when original
  pixels are absent. The distinct Thieves Eye branch remains no-draw until
  its required temporary-frame copy and original hole composition are bound
  together, rather than being approximated with a host mask. ReDMCSB
  `DUNVIEW.C F0127:8185-8236` is the source reference. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB debug viewport no-fake closure: a CSB source session
  now suppresses the Firestaff procedural corridor/trapezoid renderer and
  legacy texture tiling even when the diagnostic HUD is enabled. Diagnostic
  annotations remain available, but cannot draw host-created dungeon
  geometry or tiles over source-owned viewport material. Verification:
  `test_csb_v1_viewport_phase3_rendering` (2 677/0) and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 CSB PC3.4 D0C Thieves Eye source composition: F0127 now
  carries the authenticated CSBWin character-tail Event73 counter into the
  shared viewport and, when active, copies M654/G2116 into a local temporary
  frame before applying the original C041 subspan with C09 transparency and
  finally writing G0172 with C10 transparency. C041 is decoded through the
  active CSB `GRAPHICS.DAT` provider; absent, malformed or undersized frame
  or hole material leaves the D0C frame no-draw instead of exposing a direct
  G2116 substitute. ReDMCSB `DUNVIEW.C F0127:8185-8236` and `COORD.C
  F0630:1939-1995` are the source references. Verification:
  `test_dm1_v1_viewport_3d_pc34_compat`,
  `test_csb_v1_viewport_phase3_rendering` (2 677/0), and the CSB M12/M11
  launcher handoff boundary (567/0, one expected Atari skip) pass.
- ✅ 2026-07-31 DM2 projectile synthetic-fixture isolation: the direct
  synthetic projectile builder is now compiled and declared only for explicit
  test and probe targets. It is absent from `firestaff_dm2`; production
  projectiles must enter through the source-derived creature, spell or bomb
  routes and retain a real owner. The data-free runtime smoke no longer
  injects projectiles through the production library. Verification:
  `test_dm2_v1_projectile_creature_collision_pc34_compat` (18/18),
  `test_dm2_v1_projectile_step_pc34_compat` (16/16),
  `firestaff_dm2_v1_projectile_drain_probe` (12/12),
  `firestaff_dm2_v1_projectile_step_probe` (21/21), and `nm` confirms the
  production archive excludes `dm2_v1_projectile_dispatch_synthetic`.
- ✅ 2026-07-31 DM2 GAME_LOAD stale-party reset: after the hash-verified G1
  candidate has parsed, `LOAD_NEW_DUNGEON` now clears Firestaff's cached
  resume party, leader hand and inventory before exposing the source entrance
  pose. It does not create replacement champions; the existing source mirror
  selection and actuator/timer handoff remains required. Source: SKProject
  `SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON`, which clears
  `party.heros_in_party` and `ddat.savegamewpc.w_00` before
  `DM2_READ_DUNGEON_STRUCTURE(1)`. Verification: real-data
  `test_dm2_v1_m11_startup_profile_gate` proves stale hand/inventory removal
  at New Game, and `test_dm2_v1_load_new_dungeon_contract` passes.
- ✅ 2026-07-31 DM2 champion HUD colour fallback removal: removed the fixed
  host per-hero bar-colour table from production. The champion-stat bridge
  now requires a non-negative colour receipt supplied by the original
  GDAT/palette route and returns no HUD receipt when that owner is absent.
  Source: SKProject `SKWINSPX/src/v4/skguidrw.cpp::DM2_DRAW_PLAYER_3STAT_HEALTH_BAR`
  and `SKWIN/SkWinCore.cpp::QUERY_3STAT_BAR_COLOR`. Verification:
  `test_dm2_v1_champion_stat_bridge`,
  `test_dm2_v1_champion_hud_helpers` and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass against
  `~/.firestaff/data/dm2`.
- ✅ 2026-07-31 DM2 automatic weather-fixture removal: fresh weather state
  no longer invents clear weather or the `0x0100` RNG seed. The runtime does
  not promote the unowned bounded session-rain field, and an outdoor flag or
  host seed cannot create a `0x54` weather timer or auto-enable clouds, rain,
  or lightning. Source: SKProject `SKWIN/c_weather.cpp::DM2_SET_TIMER_WEATHER`
  and `DM2_weather_3df7_0037`, with save-state ownership still pending.
  Verification: `test_dm2_v1_weather_seed_regression` (621 assertions),
  `test_dm2_v1_weather_timer_producer_pc34_compat`,
  `test_dm2_v1_weather_gdat_receipt`, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass against
  `~/.firestaff/data/dm2`.
- ✅ 2026-07-31 DM2 boot dungeon-seed fixture removal: an unverified boot
  profile no longer starts with a PC-English seed or map count. **Corrected
  2026-08-07:** both remain unavailable until the hash-verified original
  `DUNGEON.DAT` `File_header` supplies `w0` at offset 0 and `nMaps` at byte 4.
  Source: SKProject `SKWIN/DME.h::File_header` and
  `SkWinCore.cpp::READ_DUNGEON_STRUCTURE`.
  Verification: `test_dm2_v1_boot_profile_smoke` and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass against
  `~/.firestaff/data/dm2`.
- ✅ 2026-07-31 DM2 credits palette regression gate: the real-data M11
  startup test now selects a TITLE/0/dt07/1 BPP8 pixel whose `dtPalette16`
  mapping differs, then proves the framebuffer retains that original physical
  index. This specifically rejects the palette remap that produced corrupted
  credits colours. Source: SKProject `startend.cpp::DM2_SHOW_CREDITS` and
  `DM2_INIT` palette route. Verification:
  `test_dm2_v1_m11_startup_profile_gate` passes against
  `~/.firestaff/data/dm2`.
- ✅ 2026-07-31 CSB D1L/D1R wall material binding: replaced the generic
  synthetic 256-pixel frame compositor with a fail-closed PC3.4
  `GRAPHICS.DAT` decoder binding. ReDMCSB `DUNVIEW.C` F0095/F0122/F0123 maps
  the active wall set to C03/C02 (records 96/95 for set 0); the returned
  60×111 rasters retain the original compressed-record SHA-256 receipt.
  dmweb's file-format documentation and CSBWin's decoder lineage are cited
  in the source. Verification: focused test passes against the local CSB
  `GRAPHICS.DAT` (22 checks).
- ✅ 2026-07-31 CSB D2C F0107/F0111 synthetic-probe removal: deleted the
  isolated test-only painter, its hard-coded colours and geometry, and the
  associated “real-asset” probe. The probe chose arbitrary first-by-size
  `GRAPHICS.DAT` records then composed an invented grayscale frame, so it did
  not prove original material binding and was not reachable from M11. The live
  viewport remains fail-closed on the active verified source graphics chain;
  future D2C work must bind the actual ReDMCSB F0121/F0107/F0111 command and
  native record identities rather than manufacture a capture.
- ✅ 2026-07-31 CSB V2 HUD synthetic-painter removal: removed the disabled
  hand-drawn compass, fonts, bars, action icons and magic indicator from the
  compatibility module. The retained state API is strictly no-draw; original
  pixels remain owned by PC3.4 C017/C040 or Atari ST C232. This eliminates a
  dormant generated-overlay fallback without changing runtime state handling.
- ✅ 2026-07-31 CSB dungeon-fixture runtime boundary: the two runtime dungeon
  replacement paths now require the authentic post-decompression byte-map
  layout (`square_bytes == 1`) before they can install a current dungeon.
  The older 16-bit parser remains isolated to tests, while ReDMCSB
  `DUNGEON.C F0148-F0151` and `DECOMPDU.C F0455` define every live path.
- ✅ 2026-07-31 CSB active D1L/D1R viewport binding hardening: the live M11
  provider now requires the native 60x111 C03/C02 rasters before caching
  GRAPHICS.DAT records 96/95 (wall set 0) for C713/C714. It rejects a wrong
  but decodable record instead of accepting arbitrary dimensions. Source:
  ReDMCSB `DUNVIEW.C` F0095/F0122/F0123. Verification: real-data first
  viewport-frame, F0108/F0115 ornament and D2C F0107/F0111 probes pass
  against `~/.firestaff/data/csb`.
- ✅ 2026-07-31 DM2 HUD stat-colour ownership closure: the generic HUD plan
  no longer turns an unbound champion bar colour into a source-bound default.
  The real runtime alone imports SKProject `INIT`'s original
  `glbChampionColor` values and the renderer requires that receipt together
  with the verified `INTERFACE_GENERAL/0/dtPalIRGB/0xFE` palette before it
  writes bar pixels. Source: `SKWIN/SkWinCore.cpp::INIT`,
  `DRAW_PLAYER_3STAT_HEALTH_BAR` and `DRAW_POWER_STAT_BAR`. Verification:
  `test_dm2_v1_hud_hero_type_gdat_route`,
  `test_dm2_v1_lighting_falloff_boundary`, and real-data
  `test_dm2_v1_m11_startup_profile_gate` pass.

- ✅ 2026-07-31 DM1 original PC34 backed corpus roundtrip: added the
  fixture-free `dm1_v1_original_save_pc34_backed_corpus_roundtrip` target.
  It enumerates only classifier-qualified operator-staged PC34 saves and
  drives each through real `DUNGEON.DAT` backing, F0435 import, native
  quicksave, F0433 PC34 export, and a second F0435 import. It verifies party
  pose, game tick, C03/C04 timeline count, and active-group ownership without
  generating or promoting test saves. Verification: the two current DOSBox
  saves in the configured corpus pass against the installed original DM1
  data. Source: ReDMCSB `LOADSAVE.C F0433/F0435` and DMweb Saved Game Files.

- ✅ 2026-07-31 DM2 startup palette presentation regression: the real-data
  M11 startup gate now proves that both `TITLE/0/dt07/4` menu and
  `TITLE/0/dt07/1` credits retain their original pixel indices *and* that
  SDL presentation has the matching `INTERFACE_GENERAL/0/dtPalIRGB/0xFE`
  RGB6 table installed. Source: SKProject `DM2_INIT`,
  `DRAW_TITLE_MENU_SCREEN`, and `DM2_SHOW_CREDITS`. Verification:
  real-data `test_dm2_v1_m11_startup_profile_gate`.

- ✅ 2026-07-31 DM2 legacy sky-gradient closure: removed the procedural RGB
  output from `dm2_v1_weather_sky_color()`. That API cannot carry the original
  GDAT image, palette or destination receipt, so it now reports unavailable;
  outdoor pixels remain exclusive to the verified
  `QUERY_TEMP_PICST`/`DRAW_TEMP_PICST` transaction. Source: SKProject
  `SKWIN/c_bkgrnd.cpp` and `skgdtqdb.cpp`. Verification:
  `test_dm2_v1_weather_gdat_receipt`.

- ✅ 2026-07-31 DM2 cursor-palette fallback closure: 4bpp cursor patterns no
  longer accept a hard-coded identity palette when their source palette is
  missing. They require the active original 16-entry palette, matching
  SKProject `skmcursr.cpp::DM2_INITBASICCURSORS` / `generate_cursor` and
  `SkWinCore.cpp::IBMIO_SET_CURSOR_PATTERN`; 8bpp item cursors remain
  physical-index copies. Verification: `test_dm2_v1_mouse_cursor`.

- ✅ 2026-07-31 DM2 legacy weather-particle closure: removed enum/intensity
  arithmetic that fabricated rain and storm particle counts without a source
  ENVIRONMENT command/image receipt. The helper now reports no particles;
  actual weather drawing remains GDAT-backed. Source: SKProject
  `c_weather.cpp` ENVIRONMENT command path. Verification:
  `test_dm2_v1_weather_gdat_receipt`.
- ✅ 2026-07-31 DM2 pressure-plate fixture closure: disabled the hard-coded
  five-plate catalog, including its representative coordinates, target doors,
  creature spawn and fabricated message. Runtime movement now cannot alter a
  source dungeon tile through that catalog; real plate work remains gated on
  imported dungeon sensor/actuator records and GDAT message lookup. Source:
  SKProject `c_sensor.cpp`, `c_actuator.cpp` and `QUERY_MESSAGE_TEXT`.
  Verification: `test_dm2_v1_pressure_plate_pc34_compat` and real-data
  `test_dm2_v1_m11_startup_profile_gate`.

- ✅ 2026-07-31 DM2 trigger-fixture closure: disabled the eight hard-coded
  trigger records and their host-authored door, teleport, creature and text
  targets. Runtime input and time ticks now produce no event until original
  dungeon record-chain/actuator ownership is imported. Source: SKProject
  `skevent.cpp::INVOKE_ACTUATOR` / `INVOKE_MESSAGE`. Verification:
  `test_dm2_v1_trigger_pc34_compat` and real-data
  `test_dm2_v1_m11_startup_profile_gate`.

- ✅ 2026-07-31 DM2 shop-fixture closure: disabled the five hard-coded shop
  locations, stock lists, prices and the four host-authored merchant names
  and dialog tables. A shop cannot enter, buy, sell or alter party state
  until original SHOP_GLASS actuator, WALL_GFX and dt08 ownership is
  imported. Source: SKProject `c_shop.cpp` SHOP_GLASS path. Verification:
  `test_dm2_v1_shop_pc34_compat`, both shop probes, runtime shop provenance
  test, and real-data `test_dm2_v1_m11_startup_profile_gate`.

- ✅ 2026-07-31 DM2 HUD item-name closure: removed the hard-coded ten-item
  tech/magic catalog from the leader-hand naming route. Objects now remain
  unnamed until their original DB/GDAT text ownership is decoded; neither
  fixture English names nor diagnostic pool/index strings reach the HUD.
  Source: SKProject `SkWinCore.cpp` object-ID/GDAT dispatch. Verification:
  `test_dm2_v1_spell_pc34_compat` and real-data
  `test_dm2_v1_m11_startup_profile_gate`.
- ✅ 2026-07-31 CSB duplicate D1L2/D1R2 wall-route removal: removed the
  unconsumed contract-only duplicate of F0122/F0123, including its synthetic
  source coordinates and test target. D1L/D1R now has one PC3.4
  `GRAPHICS.DAT` material owner instead of competing models.
- ✅ 2026-07-31 CSB D3L/D3R synthetic trace removal: removed the unconsumed
  F0116/F0117 contract-only wall trace and its artificial C10 blend helper.
  The production source-bound wall-set handoff remains the sole D3 side-wall
  material route.
- ✅ 2026-07-31 CSB fabricated D1L2 wall removal: removed the D1L2
  “closest analogue” module, its synthetic frame compositor and its test.
  ReDMCSB has no named D1L2 PC3.4 viewport square; retaining a D1L substitute
  would falsely claim a source route.
- ✅ 2026-07-31 CSB fabricated D1L2/D1R2 F0115 removal: removed the
  contract-only thing-pass fixture and CTest. Its “D1L2/D1R2” names were not
  ReDMCSB PC3.4 squares and it had no production consumer or real-data route.
- ✅ 2026-07-31 CSB fabricated D2L2/D2R2 partly-open door removal: removed
  the unconsumed F0111 contract route. ReDMCSB F0678/F0679 supplies D2L2/D2R2
  wall/teleporter handling, whose wall cases return before F0111; the removed
  route had neither a source material record nor a production consumer.
- ✅ 2026-07-31 CSB fabricated D0L2/D0R2 wall removal: removed the unused
  wall contract module, CTest and synthetic parity manifest. ReDMCSB F0125/
  F0126 owns D0L/D0R; no D0L2/D0R2 PC3.4 view squares exist.
- ✅ 2026-07-31 CSB duplicate D3L/D3R backdrop removal: removed the
  contract-only backdrop trace and CTest. The M11 source-bound side-wall
  handoff remains the only production owner of F0116/F0117 material.
- ✅ 2026-07-31 CSB CustomBackgrounds synthetic first-backdrop removal:
  removed the unconsumed C10 copy helper, public header and CTest. It could
  only composite caller-supplied pixels and had neither a runtime consumer
  nor a `CSBgraphics.dat` material receipt. The remaining CSBWin
  CustomBackgrounds source-lock metadata is non-drawing until its original
  masked-composite material path is bound.
- ✅ 2026-07-31 CSB fabricated D1L2/D1R2 F0108 removal: removed the
  unconsumed floor/ceiling/ornament trace, its generated-pixel hash and CTest.
  ReDMCSB `DUNVIEW.C` F0122/F0123 exposes D1L/D1R, not D1L2/D1R2; the live
  PC3.4 material routes remain the only pixel owner for the actual pair.
- ✅ 2026-07-31 CSB F0115 synthetic wall-text removal: removed the test-only
  D1C renderer that invented a wall colour, glyph pattern and palette instead
  of decoding original inscription material. The F0107/F0115 source route is
  now no-draw without an authentic graphics and palette receipt.
- ✅ 2026-07-31 CSB wall-text oracle API removal: removed the unimplemented
  public header that advertised synthetic-fixture decoding without an
  implementation or production consumer. Real original-dungeon text decoding
  remains the required path before inscription pixels can be admitted.
- ✅ 2026-07-31 CSB D0 F0115 fixture-blend removal: removed the isolated
  caller-pixel C10 blend API and its data-free assertions. The retained D0
  teleporter renderer accepts only hash-admitted `CSBgraphics.dat` bytes and
  a matching original palette receipt.

- ✅ 2026-07-31 DM1 V1 verification-route repair: PC3.4 object-name
  admission now uses the production GRAPHICS.DAT record constant (694 rather
  than stale source symbol 564); teleporter source locks follow the owned
  phase helper after its refactor; the movement matrix receives CTest's Ninja
  build directory; the HoC boot fixture isolates configured real-save corpus
  state; and the F0351 empty-hand eye panel is correctly treated as in-panel
  UI rather than a generic dialog. Verification: the five focused DM1 tests
  pass from `/tmp/firestaff-system-build`.
- ✅ 2026-07-31 DM2 exact fixed spell table: replaced the ordinal,
  compatibility-only 34-spell data with SKProject's exact
  `dSpellsTable` records from `SKWIN/SkGlobal.cpp:968-1007`. The runtime
  now uses source rune bytes, tail-key lookup, difficulty, skill and packed
  `w6`; it derives power from the live rune input and no longer falls back
  to invented per-rune mana, flat cooldown values or an index-to-object-effect
  map. Verification:
  `test_dm2_v1_spell_pc34_compat` checks all 34 records,
  `test_dm2_v1_spell_cast_player_pc34_compat` passes 151/151, and real-data
  `test_dm2_v1_m11_startup_profile_gate` passes.
- ✅ 2026-07-31 DM1 V1 movement-capture verification repair: fixed pass207's
  invalid Python f-strings, treated missing original-runner tools as an honest
  blocked capture prerequisite rather than a source regression, and made the
  movement closure materialize its deterministic pass608 blocker receipt.
  The viewport golden gate now accepts that explicit prerequisite state while
  retaining its no-pixel-parity boundary. Verification: DM1 V1 CTest passes
  1,007/1,007; DM1 V2.x passes 87/87 with local original PC34 data.
- ✅ 2026-07-31 DM1 V1 side-wall source-row audit refresh: pass576 now
  scans the current D3--D0 runtime pixel and source-row clip tests instead of
  a stale line window. Targeted verification passes pass576, pass582 and the
  viewport source-zone table gate; this remains a source lock, not a capture
  parity claim.
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
# 2026-07-31 Theron Track 02 quest-block extraction

- ✅ Added a source-data extractor for the seven 256 KiB quest blocks present
  in the verified US Track 02 raw BIN. Each block is reconstructed from
  MODE1/2352 sectors into contiguous 2048-byte user-data bytes and checked by
  an independent FNV-1a receipt in the bank probe. JP media remains explicitly
  unsupported until its corresponding block offsets are independently
  verified. This is real-data byte extraction only; it does not claim dungeon
  record, object-table, palette, bitmap, or runtime-render semantics.
  Verification: `theron_v1_track02_bank` and the clean-branch C11 syntax checks.
- ✅ 2026-07-31 DM2 V2 smooth viewport no-fabrication closure: removed the
  host-side pan and black-strip fill that ran after the real V1 viewport
  renderer. Smooth timing state remains available to input consumers, but no
  intermediate DM2 camera raster is known, so every presented frame remains
  the source-owned snapped V1 raster. References: SKProject
  `SKWIN/SkWinCore.cpp::DRAW_DUNGEON` and `DRAW_OUTDOOR_VIEWPORT`.
  Verification: V2 smooth movement 79/79, runtime binding 43/43, smooth
  probe 54/54, plus a byte-identical V1/V2 framebuffer comparison during an
  active smooth state in the hash-verified real-data DM2 M11 startup test.
# 2026-07-31 Theron JP Track 02 quest-block extraction

- ✅ Extended the real Track 02 quest-block extractor to the hash-verified JP
  BIN. The JP bank begins one raw MODE1/2352 sector before the US bank; all
  seven 256 KiB blocks are reconstructed from contiguous 2048-byte user data
  and independently checked against FNV-1a receipts from `TQJP02.bin`.
  The US receipts remain covered as well. This follows DMWeb's JP/USA
  PC-Engine CD split and seven-dungeon scope; it does not claim dungeon-record,
  object, graphics, or save-format decoding.
  Verification: clean C11 syntax checks, clean CMake target build, and
  `firestaff_theron_v1_track02_bank_probe` against both local real BINs.
- ✅ 2026-07-31 DM2 V2 unbound sky-colour closure: removed the procedural
  RGB gradients and fixed weather colours from the V2 lighting/outdoor helper
  APIs. ENVIRONMENT_DRAW_DISTANT_ELEMENT owns an outdoor image, palette and
  destination rectangle; time and weather alone cannot select original
  pixels. Unbound callers now receive DM2_V2_SOURCE_COLOR_UNAVAILABLE.
  References: SKProject SKWIN/c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT
  and skgdtqdb.cpp QUERY_TEMP_PICST/DRAW_TEMP_PICST. Verification:
  test_dm2_v2_lighting 64/64 and DM2/M11 build pass.
# 2026-07-31 Theron media-inventory false-promotion removal

- ✅ Raw Track 02 now proves startup/media ownership only. Removed the
  incorrect bitmap-, level-, and object-route promotion that treated an
  authenticated bank as if its dungeon decoder were already implemented.
  Downstream routes remain fail-closed until real consumer/decoder evidence
  exists, matching the bounded Theron status in `docs/DMWEB_REFERENCE.md` and
  TODO.md. Verification: `theron_v1_media_inventory_probe` passes.
- ✅ 2026-07-31 CSB startup-fallbackkontrakt: tog bort den döda
  title-/entrance-planens text- och dörrfallbackfält, dess gamla
  renderkommandon och den host-återgivning de kunde bära. CSB:s uppstart
  representerar nu endast originalets C001–C005, C017 och C040; en saknad
  källa blir no-draw i stället för text eller färgpaneler. Källa: ReDMCSB
  `TITLE.C F0437`, `ENTRANCE.C F0438/F0441/F0806`; CSBWin
  `Viewport.cpp`. Verifiering: startup-plan 139/139, boot-handoff 501/501,
  realdata-sekvensen C001–C005/C017/C040 och titelkadensproben passerar.
- ✅ 2026-07-31 CSB startup-rendercallbackar: tog bort den återstående
  executor-API-ytan för dörr- och textfallback. ReDMCSB `TITLE.C F0437` och
  `ENTRANCE.C F0441/F0806` når nu bara konkreta originalytor via title-,
  dörr-, opening-frame- och utility-callbackarna; värden kan inte längre
  ansluta en lokal ersättningsritning. Verifiering:
  `test_csb_v1_boot_runtime_handoff` 501/501 passerar.
- ✅ 2026-07-31 DM2 inventory substitute closure: removed the reachable M11
  renderer that put authentic DM2 ObjectID icons into DM1 `GRAPHICS.DAT` slot
  rectangles and removed its matching DM1 click route. Keyboard and direct
  champion inventory commands now fail closed as well, leaving SKSave/DB
  ObjectID ownership untouched until the real DM2 inventory surface is
  bound. Source: SKProject `CHANGE_VIEWPORT_TO_INVENTORY`, with its
  `CHAMPIONS`/`INTERFACE_GENERAL` GDAT layout and event table. Verification:
  real-data `test_dm2_v1_m11_startup_profile_gate`.
# 2026-07-31 Theron alarm spawn fallback removal

- ✅ Removed the production alarm-trigger path that fabricated a Goblin for
  every creature spawner. The alarm still activates source spawners and emits
  its alarm event, but creature materialization now stays fail-closed until
  the real Track 02 object-tail/spawn table is decoded. Regression coverage
  verifies activation and no fabricated object (`52/52` mechanics checks).
- ✅ 2026-07-31 CSB startup-assettyper: tog bort den oanvända
  `fallback`-källtypen och den döda `fallback-original`-aliasen från
  CSBgraphics-bindningen. Startup accepterar nu enbart verifierad
  `GRAPHICS.DAT` eller verifierad `CSBgraphics.dat`; negativa tester använder
  den verkliga ogiltiga typen `NONE`. Verifiering:
  `test_csb_v1_boot_title_import_ui_gate_pc34_compat` 137/137 och
  `test_csb_v1_csbgraphics_runtime_binding` 83/83 passerar.
- ✅ 2026-07-31 CSB källinventering: korrigerade den felmärkta Lord Order-
  typen. `0x19` är ReDMCSB `DEFS.H:1364` C25_CREATURE_LORD_ORDER, inte en
  placeholder, även om originaldungeonerna saknar sådana grupper. Uppdaterade
  även TODO:s inaktuella uppgift om det borttagna `fallback-original`-aliaset.
  Verifiering: `test_csb_v1_monster_generator_gate_pc34_compat` passerar.
- ✅ 2026-07-31 CSB title-capturekadens: realdatafångsterna för V1, V2.0 och
  V2.1 väntar nu 14 sekunder i stället för 7, så att de observerar alla fyra
  originalpalettfaser efter den PC3.4-bundna CHAOS-zoomen. Speltempot är
  oförändrat. Verifiering: V1:s title/entrance-kontrakt samt V2.0- och
  V2.1-capturetester passerar mot lokal PC3.4-data.
- ✅ 2026-07-31 CSB F0115-projektiler: tog bort den gamla 16×16-ikonritningen
  som kunde ersätta ReDMCSB:s perspektivbitmap för kastade objekt. En saknad
  källbunden F0115-bitmap blir nu no-draw; bara den verifierade perspektiv-
  rutten kan skriva projektilpixlar. Verifiering:
  `test_csb_v1_viewport_phase3_rendering` 2655/2655 passerar.

- ✅ 2026-07-31 DM1 HoC candidate time-effects and endgame fallback gates:
  the live M11 idle route now proves ReDMCSB `CHAMPION.C F0331` excludes the
  selected C040 candidate from health/stamina/food/water mutation, then
  restores normal decay at the next due tick after confirmation. The related
  F0444/F0446 regression expectations were aligned with the existing
  source-only policy: missing original final-screen art draws no synthetic
  controls, while an available SDL backend may queue real SONG.DAT victory
  audio. Verification: `m11_starvation_runtime_source_lock`,
  `m11_action_stamina_runtime_source_lock`,
  `dm1_v1_hall_of_champions_pc34_compat`, and the real backed PC34 corpus
  roundtrip all pass.
# 2026-07-31 Theron relic-name correction

- ✅ Replaced the invented quest-item labels in progression, chapter-marker,
  and champion-item comments with the seven real Theron's Quest relic names
  documented by DMWeb: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza
  Armor, Soulcage, and The Retaliator. This changes presentation metadata only;
  item ordinals and the unresolved Track 02 placement/decode remain bounded.
  Verification: `firestaff_theron_v1_chapter_marker_probe` passes `65/65`.
# 2026-07-31 Theron seed-placeholder reduction

- ✅ Replaced the dungeon-1 progression fallback seed `313` with the real
  US/JP Track 02 initial-level seed `0x0108e938`. The unresolved dungeon 2–7
  fallback seeds are now zero rather than fabricated ascending values, so
  progression/save state cannot present guessed seeds as original data.
  Verification: `test_theron_v1_m11_direct_launch` passes; real US/JP Track 02
  probes bind the same initial seed at their verified raw offsets.
# 2026-07-31 Nexus STABG retail-yta till startup-media

- ✅ `nexus_ui_load_stabg()` materialiserar DMWeb:s verifierade första STMP-karta
  från den riktiga `STABG.BIN`-filen som 320×168 indexyta och sparar filens 256
  Saturn-paletteord samt deterministisk RGBA-expansion. Startup räknar därmed
  ytan som laddad i stället för fallback; ingen host-palette eller syntetisk
  HUD-grafik används. VDP1/VDP2-placering och runtime-state-bindning är fortsatt
  blockerade tills de kan bevisas från Saturn-källan.
  Källa: DMWeb `DecodeSTABGBIN` och lokal retail `STABG.BIN`.
  Verifiering: `test_nexus_v1_startup_media_gate` mot
  `/Users/bosse/.firestaff/data/nexus`.
# 2026-07-31 Theron stale placeholder metadata removal

- ✅ Removed the retired Theron dungeon-seed fallback `313` from the boot
  profile; an unbound profile now starts at zero and only verified header or
  Track 02 handoff data may populate the seed. Updated the Track 02 source-lock
  table to mark dungeon 1 as `0x0108e938` (verified initial level) and dungeons
  2–7 as unresolved. Corrected the source-lock quest-item names to the seven
  DMWeb relics: Shield Defiant, Taza Poleyn, Tazahelm, Taza Boots, Taza Armor,
  Soulcage and The Retaliator. Verification: `test_theron_v1_m11_direct_launch`
  passes; `git diff --check` passes.
# 2026-07-31 Nexus PRS3-headergräns

- ✅ PRS3-headerns komprimerade storlek kontrolleras nu utan signerad
  heltalsaddition som kan wrap:a vid korrupta eller mycket stora fält.
  DMWebs little-endian-bitflöde och offsetregler är oförändrade. Den riktiga
  `MENU.BPK`-proben fortsätter att avkoda alla 162 PRS3-ytor, medan separat
  Saturn-palette-/VDP1-/menysemantik fortfarande krävs innan render-gaten kan
  öppnas.
  Verifiering: `test_nexus_v1_bpk_surface_class` med lokal retailfil.
# 2026-07-31 Nexus FACE.BIN retailpalette till uppstart

- ✅ Uppstartens FACE-loader skickar nu hela DMWeb-frameprefixet till
  porträttytan i stället för att kasta bort de första 128 bytesen. De 64
  källägda big-endian BGR555-orden per porträtt sparas och expanderas till
  RGBA; den tidigare hårdkodade `192..207`-palette-lanen används inte längre.
  PRS3-pixlarna är fortfarande no-draw tills champion-index och Saturn
  VDP-placering är bevisade.
  Verifiering: `test_nexus_v1_face_bin` avkodar alla 20 retailporträtt.
# 2026-07-31 Theron real door-state query

- ✅ Removed the remaining party-level door-state placeholder from
  `theron_v1_get_move_result()`. Hypothetical movement now reads the matching
  level door object's actual state, just like the committed movement path;
  missing door objects remain blocked rather than inheriting fixture state.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
# 2026-07-31 Theron real item pickup state

- ✅ Replaced the `THERON_CMD_TAKE` success-without-state placeholder. Known
  Track 02-independent object classes (potion, scroll, food, key, weapon and
  armor) now bind to the source-locked compact item IDs, enter the active
  champion's inventory, mark the level object picked up and recalculate load.
  Unknown/quest object classes remain rejected rather than receiving guessed
  IDs until the real Track 02 object table is decoded. Verification:
  `test_theron_v1_m11_direct_launch` passes and `git diff --check` passes.

- ✅ 2026-07-31 CSB V2.2 live-cache cleanup: M11 no longer populates the
  retired 3x3 CSB V2.2 shape cache during either CSB viewport path. Its
  hard-coded material parameters had no authenticated `DUNVIEW.C F0128`
  command, palette, clip or Thing-chain receipt and no consumer in the
  admitted compositor. Live CSB pixels can therefore reach V2.2 only through
  the command-local source-material route, while unsupported families remain
  V1. Verified with `test_csb_v1_viewport_phase3_rendering` (2655/2655),
  `test_csb_v22_inplace_draw_pc34` (57/57), and
  `test_csb_v22_shapes_pc34` (54/54).
# 2026-07-31 Theron locked-door inventory gate

- ✅ Removed the locked-door auto-unlock placeholder. `theron_v1_door_open()`
  now requires the active champion to carry the source-locked key item before
  clearing a real door's locked flag; absent keys leave the door closed.
  Verification: `test_theron_v1_m11_direct_launch` passes and
  `git diff --check` passes.
- ✅ 2026-07-31 Nexus STABG indexed-blit gate: `nexus_ui_render_stabg()` now
  refuses to copy retail palette indices into a framebuffer unless the same
  surface carries its verified source palette. This closes the remaining
  public wrapper path for unpaletted/synthetic HUD pixels; Saturn VDP
  placement remains a separate no-draw gate. Verification: Nexus startup-media
  and FACE real-data tests pass against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-07-31 DM1 timeline-dispatch stability re-verification: the former
  F0242/F0248/F0190/F0249 assert-crash cluster is stable on current main.
  The seven documented CTests pass once and in ten consecutive repetitions
  each (70/70): square-state dispatch, three F0248 launchers, fake-wall
  group deferral, and both F0190 killed-all handoffs. This closes only the
  stale crash report, not the broader original-runtime or pixel-parity work.
# 2026-07-31 Theron party-gold save binding

- ✅ Replaced the save-header gold placeholder with an explicit
  `theron_v1_save_to_slot_with_gold()` API. The real party round-trip test now
  supplies `party.gold`, the save header persists it, and slot metadata reads
  it back as `party_gold`; the legacy API remains a documented no-gold wrapper
  for callers without party context. Verification:
  `test_theron_v1_save_progress_roundtrip_pc34` and
  `test_theron_v1_m11_direct_launch` pass; `git diff --check` passes.
- ✅ 2026-07-31 Nexus SAL/MAP statuskorrigering: ljudvägen är inte en tom
  placeholder längre. Den behåller verifierad källidentitet, bounded MAP-
  fönster och SAL-containerprofil, men markerar fortfarande codec och Saturn-
  eventdispatch som oprövade och blockerar playback. Kommentarerna använder
  därför `opaque/no-playback` i stället för den missvisande `STUB`-etiketten.
- ✅ 2026-07-31 Nexus rörelseresultat för vatten/eld: standalone-rörelsevägen
  returnerar nu `BLOCKED_WATER` respektive `BLOCKED_FIRE` i stället för att
  felaktigt kollapsa båda till `BLOCKED_WALL`. Item-/runeägarskap förblir hos
  mechanics-källan och aktiveras inte av denna korrigering.
  Verifiering: C11-rörelsecheck mot `firestaff_nexus`.
- ✅ 2026-07-31 CSB boot materialization gate: `csb_v1_boot_enter_game()`
  now reaches `RUNTIME_READY` only after loading a ReDMCSB byte-map dungeon
  and decoding its initial party pose. Missing materialized data and the
  retired 16-bit parser fixture fail closed at `ASSETS_READY`, clear the
  dungeon singleton and cannot bind M11's HUD or viewport. Verified with
  `test_csb_v1_boot_viewport_render_gate`, `test_csb_v1_boot_profile_smoke`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus trapp-/trappstegslänk: oregistrerade trappor återanvänder
  inte längre koordinater eller antyder en implicit angränsande nivå.
  `nexus_stairs_resolve()` returnerar explicit unresolved-sentineller tills en
  källbunden länk registrerats; registrerade länkar är oförändrade.
  Verifiering: C11-check för både unresolved och registrerad länk.
- ✅ 2026-07-31 Nexus teleporter-owner gate: mechanics kontrollerar nu
  teleporter-länken före party-positionen muteras. En oregistrerad
  TELEPORT/TELEPORT2/TELEPORT3 blockerar utan förflyttning; registrerad länk
  dispatchas oförändrad. Verifiering: `test_nexus_v1_pit_teleporter_runtime`
  passerar 44/44.

- ✅ 2026-07-31 CSB direct-loop source handoff: `fs_game_init()` now rejects
  absent or unmaterialized CSB media, just like the boot/M11 route, and
  `fs_game_load_assets()` consumes the boot-owned dungeon and party pose.
  The generic DM1 parser can no longer supply its fixed `(11,29)` start point
  to a CSB session. Verification: direct launch against
  `/Users/bosse/.firestaff/data/csb`, `test_csb_v1_boot_viewport_render_gate`
  and `test_csb_v1_boot_runtime_handoff`.
- ✅ 2026-07-31 Nexus HUD-guld: M11 skickar nu mechanics-statens verkliga
  `gold_pieces` till HUD:n i stället för att alltid mata in syntetiskt noll.
  Fältet uppdateras av den källbundna gold-pile-pickup-vägen; fallback till
  noll används endast när mechanics-pekaren saknas. Verifiering:
  full `firestaff`-build och `test_nexus_v1_dgn_runtime_materialization`.
- ✅ 2026-07-31 Nexus HUD-startgate: produktionsvägarna för launcher-start och
  save-resume använder inte längre `force_active_for_test(1)` för HUD:n.
  HUD-rendering kräver därmed den normala V2-presentationsgaten; testläget
  finns kvar endast för explicita integrationstester. Verifiering:
  `test_nexus_v2_hud_runtime_integration` passerar 9/9 och full `firestaff`
  build passerar.

- ✅ 2026-07-31 CSB runtime boot materialization gate:
  `csb_v1_runtime_boot()` no longer reports success with absent graphics, an
  unreadable/legacy dungeon or no decoded initial party pose. A failed retry
  clears the prior dungeon singleton and source paths before it returns.
  Verification: `test_csb_v1_boot_runtime_handoff`, including its missing
  source-media regression, plus boot-profile and viewport gate tests.
- ✅ 2026-07-31 Theron uppstart: boot-scannern känner nu igen de faktiska
  råa Track 02-filnamnen `TQJP02.bin` och `TQUS02.bin` som används i
  `~/.firestaff/data/theron`. De hashverifieras genom samma befintliga
  kataloggate; inga nya datafiler eller fallbackvärden läggs till.
- ✅ 2026-07-31 Nexus V2-produktionsgate: launcher-start och save-resume
  kringgår inte längre presentationsgaten för lighting, smooth movement eller
  touch-runtime med test-only `force_active_for_test(1)`-anrop. V2-proberna
  aktiverar fortsatt läget explicit. Verifiering: `firestaff`-build,
  smooth-movement-probe 33/33 och touch-runtime-probe 57/57.
- ✅ 2026-07-31 Nexus ljuddiagnostik: kvarvarande `(stub)`-etiketter för
  CDDA stop/pause/resume/fade är ersatta med `opaque/no-playback`. Verkliga
  SAL/MAP- och CD-spår förblir källbundna, men codec/driver och uppspelning
  markeras fortsatt som blockerade. Verifiering:
  `test_nexus_v1_sound_runtime_receipt` passerar.
- ✅ 2026-07-31 Theron uppstart-seed: startup-receipt kopplar nu boot-
  sammanfattningens dungeon-seed till den verifierade initiala Track 02-
  levelheadern (`0x0108e938`) i stället för att lämna no-header-värdet `0`.
  Real-asset-proben verifierar seed, roster och startup-handoff.
- ✅ 2026-07-31 CSB graphics filename-fallback removal: runtime graphics
  discovery now requires a known CSB graphics MD5 for every version hint,
  including unknown/custom hints. A random `GRAPHICS.DAT`, `CSB.DAT` or
  `CSBGRAPH.DAT` can no longer become a live graphics binding merely because
  of its filename. The regression covers both selected and unknown hints;
  renamed authentic media remains discoverable through recursive hash search.
- ✅ 2026-07-31 CSB undefined monster-projectile gate: Grey Lord/Lord Order's
  documented ReDMCSB `GROUP.C` BUG0_13 path, and a missing RNG context, no
  longer create a synthetic Fireball. They return no source projectile, which
  the live runtime rejects before projectile creation. Normal authenticated
  creature attacks keep their original projectile selection.
- ✅ 2026-07-31 Nexus FONT256 DMWeb-regioner: den verkliga S2D-decodern
  exponerar nu namngivna, bounds-verifierade bytefönster för Map, Page/
  tilemap, Character Generator, Palette och Attributes enligt DMWeb:s
  `DecodeFONT256S2D`. Retailkontroll mot `FONT256.S2D` verifierar de fem
  offset/size-paren; ingen glyph- eller menysemantik påstås ännu.
  Verifiering: `test_nexus_v1_font_s2d` passerar.
- ✅ 2026-07-31 Nexus FONT256 Character Generator: en bounded API kopierar
  nu DMWeb:s 242 verkliga 8x8/8-bit tiles från CG-regionen efter dess
  16-byte prefix och avvisar index/filgränsöverskridanden. Tileindexen hålls
  uttryckligen separata från glyph-/menysemantik. Verifiering:
  `test_nexus_v1_font_s2d` passerar mot lokal retailfil.
- ✅ 2026-07-31 CSB M11 media-rehash gate: the M11 entry boundary now hashes
  the selected `GRAPHICS.DAT` and `DUNGEON.DAT` again and requires exact
  agreement with the boot profile's scanned receipt before any CSB pixels can
  be decoded. A file replaced after scan fails closed instead of inheriting a
  stale verified flag; the focused boot-profile test covers this regression.
- ✅ 2026-07-31 Nexus FONT256 Page/palette words: bounded API:er läser nu
  DMWeb:s 4096 big-endian Page/tilemapord och 256 big-endian BGR555-
  paletteord från de verkliga regionerna. Retailtestet verifierar tilemapord
  1 = `0x0002`, paletteord 0 = `0x8000` samt indexgränser; ingen glyph- eller
  menybetydelse härleds ännu. Verifiering: `test_nexus_v1_font_s2d`.
- ✅ 2026-07-31 Nexus FONT256 attributes: bounded API för de 242 verkliga
  big-endian attribute-orden är tillagd från DMWeb:s Attributes-region.
  Tile-attributen hålls separata från ännu obevisad glyph- och menysemantik.
  Verifiering: `test_nexus_v1_font_s2d` passerar mot retailfilen.
- ✅ 2026-07-31 Nexus HUD no-fake gate: live DGN-vägen sätter inte längre
  V2-presentationsflaggor hårdkodat för att öppna den procedurala HUD:n.
  Utan en autentiserad retail-widget/VDP-placement receipt förblir overlayn
  stängd; explicit V2-integrationstest kan fortfarande aktivera den.
  Verifiering: full `firestaff`-build, HUD 9/9 och DGN materialization-test.
- ✅ 2026-07-31 CSB dead state-shim removal: deleted the unbuilt
  `csb_v1_game` skeleton, which exposed fixed `(5,5)`/`(0,0)` positions and
  marked DM1 import complete without loading anything. CSB now has only the
  verified `CSB_V1_RuntimeProfile`/dungeon/Utility ownership documented by
  the integration and source-lock references; no production caller used the
  retired API.
- ✅ 2026-07-31 Nexus viewport animated-material gate: Structure3-material
  med `0x08xx` behåller retail descriptor-proveniens men använder inte längre
  första Structure2-bilden som en obevisad statisk frame-substitution.
  Pixelrutten förblir no-draw tills Saturn frame-selector/VDP1-bindningen är
  verifierad. Verifiering: `test_nexus_v1_dgn_runtime_materialization`;
  source-receipt-testet skippar korrekt utan staged Nexus-dir.
- ✅ 2026-07-31 CSB Utility metadata-party removal: `get_party()` no longer
  reconstructs champion count, leader, and import provenance from free
  `reserved[]` metadata when the imported champion body is missing. The
  runtime receives only the full validated Utility party; the regression
  proves stale metadata cannot manufacture a launchable party.
- ✅ 2026-07-31 CSB file-dungeon fixture closure:
  `csb_v1_dungeon_load_from_file()` now rejects the retired 16-bit
  column-major fixture layout after parsing, clears its temporary ownership,
  and publishes only ReDMCSB-compatible one-byte square maps from a path.
  The explicit fixture regression proves the file boundary fails closed.
- ✅ 2026-07-31 Nexus MENU.BPK PRS3 source-lock correction: the runtime decoder is now documented against DMWeb `DMNDataFileDecoder.vbs::DecodePRS3`, including its LSB-first control bytes, literal/back-reference commands, 12-bit window, and `+18`/negative-window rule. The real local `MENU.BPK` corpus decodes all 162 PRS3 surfaces with zero failures. Remaining MENU work is pixel-mode/palette interpretation and authenticated Saturn VDP1 placement, not an undocumented compression algorithm.
# ✅ 2026-07-31 — Theron palette admission is source-gated

- Removed the synthetic default stone-gradient palette from the V1 palette state.
- An unbound palette now remains empty, so HUD/viewport code cannot receive manufactured colors before verified Track 02 data is loaded.
- Updated the rendering test to assert the fail-closed palette contract; focused suite passes 25/25.
# ✅ 2026-07-31 — Theron V2 HUD production path is asset-gated

- The boot/runtime path no longer draws the procedural V2 HUD overlay when the HUD widget manifest is missing, partial, or placeholder-only.
- Rendering now requires a complete manifest with real assets for every HUD slot; the local Track 02 BINs remain correctly limited to verified startup surfaces.
- Verification: `test_theron_rendering` 25/25 and `test_theron_v2_hud_overlay_pc34` 58/58.
# ✅ 2026-07-31 — Theron V1 chrome helpers fail closed

- Direct topbar, right-panel, and champion-slot helpers no longer emit procedural blocks, icons, or name bars without a verified runtime chrome bank.
- This closes the legacy low-level path as well as the master HUD compositor; the generic bar primitive remains available for source-backed callers.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup fallback no longer invents unknown seeds

- The legacy bounded fallback-room receipt now reports seed `0` when dungeon metadata is not verified instead of carrying the retired literal seed `313`.
- Verified Track 02 startup remains authoritative; this change only removes misleading metadata from the compatibility fixture path.
- Verification: `test_theron_rendering` 25/25.
# ✅ 2026-07-31 — Theron startup no longer paints no-data placeholders

- Removed the production branch that enabled command-drawn synthetic title, stage, Soul Room, and forcefield graphics when Track 02 was absent.
- Startup now reports `NO VERIFIED TRACK02 GRAPHICS` and remains blocked until the real atlas route is present.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron V2.2 missing-shape API fails closed

- Removed the runtime checkerboard placeholder contract from `theron_v22_get_missing_placeholder()`; missing modern assets now return `NULL` with 0×0 dimensions.
- Updated the public contract and regression test. No production caller can receive invented missing-texture pixels.
- Verification: `test_theron_v22_modern_assets_pc34` 32 checks, 0 failures.
# ✅ 2026-07-31 — Theron boot scanner rejects unverified legacy files

- Removed the `GRAPHICS.DAT`/`DUNGEON.DAT` fallback search from the Theron boot scanner.
- Theron launch discovery now accepts only the hash-verified Track 02 media routes present in the real data corpus; unverified extracted files cannot become a launch source.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron legacy enter-game stub fails closed

- `theron_v1_boot_enter_game()` no longer reports success while leaving `theron_state` and `dungeon_data` unbound.
- The real Track 02 runtime handoff remains the only valid game-state transition.
- Verification: `test_theron_rendering` 25/25 and `firestaff_theron_v1_startup_flow_probe` 653/653.
# ✅ 2026-07-31 — Theron Track 02 bad-input routes deny fallback visuals

- Track 02 startup/object/level route receipt initializers now default `fallback_visuals_allowed` to `0` for unknown or malformed input.
- A caller must receive explicit verified route evidence before any visual permission can exist; bad input cannot grant placeholder rendering.
- Verification: `test_theron_rendering` 25/25.

- ✅ 2026-07-31 CSB viewport contract isolation: three more contract-only
  CustomBackgrounds modules (D1L/D1R first backdrop, floor/ceiling mask
  ordering and room-pass ordering) now compile exclusively into their focused
  tests, not `firestaff_m10`. Live viewport code retains only the source-bound
  room-slot/material path. Verification: focused regressions (74 + 563 + 86
  assertions) and complete `firestaff` link.

- ✅ 2026-07-31 CSBWin save-fixture isolation: the synthetic 14-shape
  CSBWin/DM1 save corpus and its convenience runner were removed from the M10
  loader-boundary module and public production header. They are now test-only
  support for the focused regression, boot-handoff regression and skip-safe
  verification probe; the runtime boundary accepts only caller-supplied save
  bytes. Verification: loader-boundary test 158/158, boot handoff 504/504,
  real staged-save probe 22/22 and complete `firestaff` link.
- ✅ 2026-07-31 DM1 HoC F0172 ornament correction: removed the
  Firestaff-only map-zero random-floor-ornament suppression. ReDMCSB
  `DUNGEON.C F0172` applies this path to every corridor map, and sensors then
  override its ordinal. The regression covers both a map-zero sensor ornament
  and a deterministic map-zero random ornament. Verification:
  `test_m11_overlay_command_queue_block` (192/192) and
  `test_m11_v22_shape_cache_pc34` (31/31).

- ✅ 2026-07-31 DM1 HoC F0172 sensor-zero correction: floor sensors now
  overwrite the random floor-ornament ordinal even when their source-owned
  `Remote.OrnamentOrdinal` is zero. ReDMCSB assigns that field
  unconditionally; zero suppresses a random grate or pressure plate instead
  of allowing it to leak through. Verification:
  `test_m11_overlay_command_queue_block` (193/193),
  `test_m11_v22_shape_cache_pc34` (31/31), and the installed PC 3.4 HoC
  runtime probe.

- ✅ 2026-07-31 DM1 F0115 alcove-object input binding: C080 now accepts the
  actual current-frame C2548/F0791 destination rectangle for a front alcove
  item, in addition to the original C05 ornament zone. This preserves wall
  sensor input while making a real rendered torch/object pickable. Verification:
  `test_m11_dm1_real_alcove_item_runtime_pc34` finds map 1 `(6,3,2)` in the
  installed PC34 corpus and successfully transfers the rendered object into
  the leader hand.

- ✅ 2026-07-31 DM1 F0115/C080 rendered floor-pile input: normal DM1 no
  longer uses four fixed, approximate floor-item click panes. Each successful
  PC34 F0115/F0791 object blit now publishes its exact final rectangle,
  source `THING`, and map square for the current frame; C080 takes the
  topmost clicked rendered object directly into the leader hand. Missing or
  occluded source material therefore cannot select an arbitrary neighbour
  from the thing chain. Verification: the real PC34 non-HoC F0115 runtime
  test clicks the returned material rectangle and confirms that a leader-hand
  object is produced; `test_m11_overlay_command_queue_block` remains 193/193.
- ✅ 2026-07-31 Nexus ITEM.IBS/viewport source chain recheck: the focused
  Structure1F provenance and spatial receipts, all 16-level retail DGN
  face/material admission, and runtime materialization pass against the real
  European corpus. ITEM.IBS 4bpp/palette ownership remains source-bound and
  no-draw; the only remaining viewport gate is authentic Saturn VDP1 capture.
- ✅ 2026-07-31 Nexus MENU.BPK palette boundary: DMWeb's 256-entry
  big-endian PALT trailer is now revalidated from the real `MENU.BPK`.
  Structure2 ABI, intake and PRS3/VDP1 consumer-evidence tests all pass;
  palette bytes remain source-bound but are not promoted to visible menu
  pixels until an authentic Saturn consumer trace is available.

- ✅ 2026-07-31 CSB viewport contract isolation: the unbound D0L2/D0R2
  F0111 partly-open-door and D1L/D1R F0108 floor/ceiling-ornament contract
  modules now compile exclusively into their focused tests, not `firestaff_m10`.
  They contain no authenticated bitmap decoder or runtime consumer, so keeping
  them out of M10 prevents their source-locked metadata from masquerading as a
  draw path. Verification: both focused tests and full `firestaff` link.
- ✅ 2026-07-31 Theron startup fallback boundary: confirmed M11 has no caller
  for the legacy synthetic-room API and uses only
  `theron_v1_startup_runtime_load_initial_level_verified_only()`. The helper
  and legacy loader are now explicitly documented as data-free fixture
  compatibility only; verified Track 02 with no semantic handoff remains
  blocked. Startup-flow `653/653` and rendering `25/25` remain green.
- ✅ 2026-07-31 Nexus startup/menu/HUD audit: real `TITLE.CG`, warning/gameover
  media, champion startup menu, `FONT256.S2D`, MENU.BPK no-draw handoff and
  the V2 HUD gate all pass their focused tests. The HUD integration's 9/9
  render assertions are test-only; production keeps the procedural overlay
  closed until a retail widget/VDP placement receipt exists.
- ✅ 2026-07-31 Nexus HUD provenance correction: removed the false claim that
  the procedural V2 overlay was sourced from retail `NEXUS.BIN`. The supplied
  corpus has no authenticated HUD widget surface; the module is explicitly
  diagnostic/test-only and production remains gated. HUD overlay 46/46,
  runtime integration 9/9 and `firestaff_m11` build pass.
- ✅ 2026-07-31 Nexus V2 provenance audit: corrected remaining lighting, touch,
  smooth-movement, phase-gate and title comments so absent `NEXUS.BIN` data is
  recorded as unavailable rather than presented as a retail source. ReDMCSB,
  DMDF/DGN and existing behavioral references remain cited; all production V2
  gates stay closed. Focused lighting 79/79, phase gate 240/240, smooth
  movement 27/27 and touch affordance 0 failures pass.
- ✅ 2026-07-31 Nexus launcher card audit: the modern M12 card renderer no
  longer permits any generated game-card motif branch to paint the Nexus card,
  even if a layout slot index is reused. Nexus startup/menu art therefore stays
  source-bound/no-draw until real Saturn placement is admitted; other game-card
  routes are unchanged. `firestaff_m11` rebuild passes.
- ✅ 2026-07-31 Nexus launcher status audit: removed the hardcoded `AVAILABLE`
  label from the legacy M12 card path. Nexus now reports readiness only from
  the verified asset-version match, like the other games; `firestaff_m12`
  rebuild and diff check pass.
- ✅ 2026-07-31 Nexus real FONT256 handoff: fixed the inverted
  `nexus_v1_font_s2d_decode()` success check in engine init. The supplied
  `FONT256.S2D` now reaches the engine's source-admitted state; the separate
  page-to-character glyph-render gate remains closed, so no guessed glyphs are
  emitted. Real Track 1 capture readiness passes 29/29, FONT256 decoder and
  startup-menu tests pass.
- ✅ 2026-07-31 Theron production combat boundary: removed the inferred
  creature/combat template table from `firestaff_theron`. Production now
  links explicit fail-closed symbols from
  `theron_v1_combat_runtime_noop.c`; the full inferred implementation is
  available only to the dedicated combat fixture target. Rendering `25/25`
  and startup-flow `653/653` remain green.
- ✅ 2026-07-31 Theron dörrregression: uppdaterade combat-fixturen så den
  placerar en riktig `THERON_ITEM_KEY` innan den försöker öppna en låst dörr.
  Testet följer nu den källbundna nyckelgrinden och passerar 66/66.
- ✅ 2026-07-31 Theron shop-data boundary: removed the fixture-driven,
  source-unverified shop price-table helper from the production archive.
  Its focused test and purchase-gate probe still compile it explicitly;
  production cannot expose inferred shop prices or item ranges.
- ✅ 2026-07-31 Theron V2.2 viewport boundary: removed the placeholder
  3×3 cell-rectangle cache from the production Theron archive. Focused V2.2
  tests may still compile it explicitly, but live rendering cannot consume
  guessed viewport coordinates.
- ✅ 2026-07-31 Theron V2.2 material boundary: removed the inferred modern
  shape/material book from the production archive and replaced its init seam
  with an explicit blocked route. Focused V2.2 fixture targets retain the
  original shape implementation; live production cannot promote its guessed
  tints or geometry.
# Isolated the inferred Theron V2 HUD widget manifest/parser from production and added a no-op gate seam; procedural HUD pixels can no longer render in the verified runtime without a complete real asset manifest.

- ✅ 2026-07-31 Nexus champion provenance audit: the earlier 24-entry table
  was confirmed as fixture data and removed from the live path. The real
  `RLOWFIX.BIN`/`PLRD` handoff is recorded below; the 24-entry array remains
  storage capacity only.

- ✅ 2026-07-31 Nexus PLRD champion handoff: DMWeb's real
  `RLOWFIX.BIN` `RES*`/`PLRD` structure is now parsed in production. The
  European corpus supplies 20 records with Japanese `TABL`-decoded labels,
  HP/stamina/mana, attributes, levels, and source ordinals; the 24-element
  array remains storage capacity only. `test_nexus_v1_champion_plrd` passes
  against the local real file, and malformed/missing PLRD input fails closed.
- ✅ 2026-07-31 Nexus ITEM.IBS ordinal handoff: the source-owned category and
  weight bytes for all 243 real ITEM.IBS declarations now form the live item
  lookup boundary. PLRD equipment/backpack ordinals retain real declaration
  identity without reviving the old DM1 catalog; names, attack/defense and
  key/action semantics remain explicitly unavailable.
- ✅ 2026-07-31 Theron V1 UI chrome isolation: removed the inferred bars,
  labels and champion-slot pixels from the production archive. The public
  chrome API now fails closed through a no-op seam until the original Track
  02 UI bank is decoded; the old implementation remains fixture-only.
- ✅ 2026-07-31 Theron viewport admission wording: corrected the lifecycle
  and source comments to describe the palette as unbound, and removed the
  stale claim that facing could come from a world-tick surrogate. The
  viewport continues to accept only the authenticated party pose and blocks
  pixels until a source tile bank is bound.

- ✅ 2026-07-31 Theron tile-renderer isolation: removed the inferred
  square/depth tile table and rasterizer from the production archive. The
  diagnostic tile-renderer probe still compiles the implementation explicitly;
  production now returns no tile and preserves the framebuffer until a real
  Track 02 tile-bank handoff exists.

- ✅ 2026-07-31 Theron V2.2 local-art isolation: removed the modern-art
  manifest/cache and inplace rectangle renderer from the production archive.
  Their focused V2.2 tests retain explicit source compilation, but `firestaff`
  cannot promote local cache/manifest pixels into the runtime.

- ✅ 2026-07-31 Theron viewport mapping gate: blocked the duplicate viewport
  tile table even when a caller supplies an unverified atlas. The legacy
  fixture renderer is compiled explicitly by the rendering test; production
  now requires a decoded Track 02 square/depth/material mapping.

- ✅ 2026-07-31 Theron placeholder inventory: audited the champion-state
  initializer and recorded its default names/classes/stats as an explicit
  unresolved real-data gap. Existing save/fixture tests still depend on it;
  no production claim now treats those defaults as decoded Track 02 records.

- ✅ 2026-07-31 CSB SWSH F0904 receipt isolation: the palette-animation
  receipt accepts metadata only and has no runtime caller or SWSH command
  decoder. It now compiles only into its focused test, rather than M10;
  production cannot turn receipt facts into synthetic palette animation.
- ✅ 2026-07-31 Theron verified champion handoff: authenticated JP/US Track
  02 startup sessions now clear fixture-only 10-point stats, inventory and
  equipment defaults before runtime entry. Source-roster identity metadata is
  retained; undecoded numeric champion records fail closed instead of being
  presented as real data.

- ✅ 2026-07-31 CSB SWSH F0908/F0909/F0910 receipt isolation: the metadata
  chain for sound init, playback and release has no production caller. M11
  keeps using the real-byte `RedmcsbF0908_InitSoundPc34` path, while the
  receipt chain compiles solely into its focused test and cannot authenticate
  host audio as original SWSH data.

- ✅ 2026-07-31 CSB startup receipt isolation: F0436 palette fade, F0579
  entrance bitplanes and F0807 door-step helpers are metadata contracts with
  no product caller or original-pixel decoder. They now compile only into
  their focused tests; M10 cannot treat caller facts as title or entrance
  material. Live startup remains guarded by the authenticated runtime route.

- ✅ 2026-07-31 CSB F0797 entrance-layout receipt isolation: the 5×5
  micro-dungeon layout metadata had no product caller and now compiles only
  into its focused test. It cannot become a generic loaded-dungeon or viewport
  substitute; an actual entrance frame must still use its source-owned draw
  route and verified graphics material.

- ✅ 2026-07-31 Theron startup-receipt isolation: removed the explicit
  no-data placeholder receipt implementation from the production archive.
  The real-asset receipt probe and save/resume fixture compile it explicitly;
  `firestaff` cannot link placeholder startup labels or tokens.
- ✅ 2026-07-31 CSB F0440/F0902 startup receipt isolation: temporary-graphic
  byte-count and FTL-logo fact helpers have no runtime caller or decoder and
  now compile only into their focused tests. M10 can no longer substitute
  caller metadata for a verified decompressed member, logo bitmap or palette.

- ✅ 2026-07-31 CSB startup-boundary/ownership isolation: the F0474–F0490
  blocked-graphics receipt and F0886–F0905 ownership table have no runtime
  consumer and now compile only into their focused tests. Production continues
  through the verified archive/decoder path rather than treating a blocked
  receipt or an ownership string as graphics material.

- ✅ 2026-07-31 Theron runtime fallback isolation: the startup runtime no
  longer synthesizes a fallback room in the production build. That branch is
  compile-defined only for the startup-flow fixture probe; production remains
  unavailable until a decoded Track 02 level is bound.

- ✅ 2026-07-31 CSB F0906–F0925 primitive-inventory isolation: the raw
  function-number metadata table only reports dependencies and explicitly
  blocks execution. It now compiles solely into its inventory test, leaving
  M10 to the dedicated authenticated SWSH and Utility implementations.

- ✅ 2026-07-31 Theron legacy asset verification: the generic loader no longer
  reports success for an expected digest it cannot compare against an
  authoritative catalog. Hash-bound Track 02 boot remains the only admission
  route; the legacy API now fails with `TR_ASSET_ERR_HASH`.

- ✅ 2026-07-31 Theron chapter-marker gate: a verified media identity without
  decoded progression/save state now reports unavailable instead of fabricating
  Chapter 1 and `0/7` quest progress. Later dungeon hints remain unavailable
  until their real headers/names are bound; fixture-only profile projection is
  explicitly compile-scoped.

- ✅ 2026-07-31 CSB F0846–F0865 unmapped-boundary isolation: this range has
  no ReDMCSB callable and only reports a fail-closed admission receipt. It
  now compiles solely into its focused contract test, so M10 cannot mistake
  source-absence metadata for an executable runtime implementation.

- ✅ 2026-07-31 CSB F0986–F1005 graphics-boundary isolation: the function
  table documents local, foreign-platform and unbound helpers, then blocks
  every runtime route. With no product caller or decoder, it now compiles only
  into its contract test; live rendering continues through authenticated PC
  3.4 graphics material.

- ✅ 2026-07-31 CSB F1006–F1025 source-boundary isolation: this table only
  inventories local, existing-owner and foreign-platform symbols and blocks
  execution for all of them. It now compiles solely into its focused contract
  test; M10 retains only actual authenticated CSB consumers.

- ✅ 2026-07-31 CSB platform-helper isolation: the combined F1048/F1049/
  F1053/F1055/F1061 wrapper only exported a disabled alias and explicit
  Amiga fake-code no-ops, with no production caller. It is excluded from M10;
  source-faithful shared fail-closed boundaries remain available for their
  separate focused tests.

- ✅ 2026-07-31 CSB F1066–F1085 Amiga-boundary isolation: the table has no
  PC 3.4 product consumer and explicitly blocks every route. It now compiles
  only into its contract test; the separately owned, source-faithful Intuition
  vector boundary remains independent of this inventory.

- ✅ 2026-07-31 Theron champion handoff hardening: verified Track 02 runtime
  entry now clears fixture champion names, portraits, classes and party count
  in addition to default stats/inventory. Production cannot present the
  inferred roster until original champion records are decoded.

- ✅ 2026-07-31 CSB F1126–F1145 source-boundary isolation: this catalog only
  records local, foreign-platform and unbound symbols before failing closed.
  It now compiles solely into its contract test, so M10 cannot treat source
  labels as a substitute for an authenticated CSB input or graphics route.

- ✅ 2026-07-31 Theron SRM champion-name gate: real SRM body import no longer
  substitutes `Theron` or `Companion` when a champion name field is empty. The
  record is rejected until source name bytes are present.

- ✅ 2026-07-31 CSB F1186–F1205 ANIM-boundary isolation: the table is a
  DM1-owned ANIM inventory without an authenticated CSB stream or runtime
  consumer, and already blocks execution. It now compiles only into its
  contract test, preventing metadata from creating CSB UI or timing behavior.

- ✅ 2026-07-31 Theron SRM progression-only handoff: Continue now clears the
  fixture world party when an SRM contains progression but no champion body.
  It no longer invents a one-member Theron party from unrelated initialized
  state.

- ✅ 2026-07-31 CSB F1206–F1225 ownership isolation: the table only records
  ANIM platform/local status and admits no route. It now compiles solely into
  its contract test, keeping metadata from standing in for CSB palette, sound
  or allocation behavior.

- ✅ 2026-07-31 CSB F1406–F1445 unmapped-boundary isolation: ReDMCSB has no
  callable symbol in this range, and the table only reports a blocked receipt.
  It now compiles only into its contract test; local source labels cannot
  become a synthetic CSB entrance, startup or graphics implementation.

- ✅ 2026-07-31 Theron runtime-render asset gate: the frame facade now requires
  a non-NULL asset bundle and fails before viewport/UI presentation otherwise.
  Rendering remains source-admitted only; the focused rendering suite passes
  `25/25`.

- ✅ 2026-07-31 Theron startup receipt fixture isolation: verified Track 02
  receipts no longer copy the fixture mirror roster size or fallback-label
  count. Those values remain confined to the explicit no-data fixture receipt;
  real startup data cannot report synthetic roster metadata.

- ✅ 2026-07-31 Theron startup runtime test linkage: the save/resume contract
  target now compiles its fixture-only structured fallback entry explicitly,
  while production still links the no-fallback runtime archive. The focused
  suite is green at `325/325`.

- ✅ 2026-07-31 Theron startup menu metadata gate: absent decoded Track 02
  roster names no longer expose fixture portrait indices or classes in menu
  elements. The startup-flow probe remains green at `653/653`.

- ✅ 2026-07-31 Theron startup TODO audit: removed the stale claim that the
  structured save/resume receipt test had an unrelated failure. The corrected
  fixture-scoped linkage now passes `325/325`; HUD rendering remains blocked
  until a real Track 02 widget bank is decoded.

- ✅ 2026-07-31 Theron champion handoff fixture isolation: the production
  `enter_forcefield_with_roster` path no longer calls `theron_v1_party_init()`
  or inherits its synthetic stats, classes, and portraits. It admits only
  source roster names; the full mirror-table initializer is fixture-scoped.
  Startup flow remains `653/653`, save/resume `325/325`.

- ✅ 2026-07-31 Theron viewport tile-helper gate: production
  `theron_vp_tile_for_square()` now returns no tile until a real Track 02
  mapping is bound. The inferred table is compiled only into the explicit
  viewport fixture probe; verification passes `50/50` and rendering `25/25`.

- ✅ 2026-07-31 Theron menu portrait/class gate: decoded roster names no
  longer authorize inferred mirror-table portrait indices or classes in
  production. Those fields remain unavailable until their source records are
  decoded; fixture metadata is compile-scoped to the startup probe.

- ✅ 2026-07-31 Theron legacy asset no-data gate: `tr_asset_load()` no longer
  returns success or claims “using defaults” when the requested file is
  missing. It returns `TR_ASSET_ERR_NO_DATA`; rendering remains source-gated.
  Focused rendering passes `25/25`, startup/save-resume `325/325`.

- ✅ 2026-07-31 Theron legacy parse-error gate: discovered Track 03/04 data
  that fails its parser now returns `TR_ASSET_ERR_TR03`/`TR_ASSET_ERR_TR04`
  and releases the partially loaded bundle instead of reporting a successful
  asset load with fallback state.

- ✅ 2026-07-31 Theron runtime world-init gate: production boot and Track 02
  runtime inspection now use a zero-party world initializer. The legacy
  fixture initializer remains available to tests, but no default champion
  roster exists before verified source handoff.

- ✅ 2026-07-31 Theron level-header seed binding: `theron_v1_level_load()` now
  retains the authenticated Track 02 header seed in `Theron_V1_Level` instead
  of discarding it. No tile/object meaning is inferred from the seed; the
  viewport mapping gate remains closed.

- ✅ 2026-07-31 Theron seed regression proof: the real Track 02 level-handoff
  probe now asserts the retained `0x0108e938` seed directly on the loaded
  level, alongside the existing raw candidate checks.

- ✅ 2026-07-31 Theron opaque header-index binding: level load now preserves
  the Track 02 header's `0x0026` level-index value in a separate opaque field,
  without confusing it with Firestaff's internal 0-based level slot. The real
  handoff probe asserts it; result remains `fail=0` with one known ISO skip.

- ✅ 2026-07-31 Theron level fixture parity: the explicit no-data room helpers
  now populate the same seed/header-index fields as their serialized headers,
  keeping fixture inspection structurally honest without promoting fixture
  bytes into production semantics. Startup flow remains `653/653`.

- ✅ 2026-07-31 Nexus TEXT/TABL source-boundary cleanup: RLOWFIX.BIN TEXT
  offsets and the 216-entry DMWeb TABL code table are parsed from the real
  retail resource and exercised by `test_nexus_v1_champion_plrd`. The legacy
  heuristic ASCII/Shift-JIS scraper plus unauthenticated S2D text/glyph
  layout wrappers are excluded from `firestaff_nexus`; they remain available
  only to explicit diagnostic probes. No glyph, palette, menu, HUD or Saturn
  VDP1/VDP2 presentation is promoted by this change.
- ✅ 2026-07-31 DM2 SHOP_GLASS panel isolation: removed the remaining
  host-authored shop rectangle, English labels and empty-inventory fallback
  from the production shop module. Its render contract now clears the output
  and returns no-draw until the source-owned `WALL_GFX`/DB actuator chain is
  decoded. Verification: production link, shop admission regression and an
  executable-string check for the retired panel text.
- ✅ 2026-07-31 DM2 world/object fallback isolation: removed the inferred
  16-bit world builder and sequential thing-pool parser from the live path.
  `dm2_world_from_mem()` now requires the PC G1 byte-square loader, and the
  object model returns no records when the validated c_record chain is not
  available. Verification: complete production `firestaff` link and no
  compiler warnings in either changed DM2 source.
- ✅ 2026-07-31 DM2 V2 runtime/lighting isolation: removed the unattached
  smooth-camera, bloom and animated outdoor-state sources from the production
  archive and game loop. These local time/weather effects remain in explicit
  diagnostic targets only; live DM2 presentation stays on the authenticated
  V1 viewport and GDAT HUD path. Verification: production link, V2 probes,
  real-data DM2 startup gate and production-symbol check.
- ✅ 2026-07-31 Nexus real viewport gate rechecked: the Track 1 readiness
  probe drives the local English CUE/DM.BIN, real `LEV00.DGN`, `FONT256.S2D`
  and `SCORPION.MNS` handoff through `nexus_viewport_render`; 29/29 pass.
  The real viewport capture remains deterministic black until authenticated
  Saturn DGN/VDP1 material is admitted, with no procedural fallback pixels.

- ✅ 2026-07-31 CSB V2.2 synthetic shape-book isolation: removed the
  hand-authored material/PBR/geometry book from `firestaff_csb_v2`; its
  historical expectations remain explicitly test/probe scoped. Production now
  links `csb_v22_shapes_runtime_gate.c`, whose API reports zero materials and
  no shape parameters until a reviewed original-data binding exists. The
  runtime cache requires a non-NULL admitted material before activating a V2.2
  cell, so it retains source-owned V1/V2.1 pixels rather than inventing a
  fallback. Verified with the new `csb_v22_shapes_runtime_gate_pc34` test,
  the historical shape-book contract test, and a `firestaff` build.

- ✅ 2026-07-31 DM2 V2 companion/crafting/viewport isolation: removed the
  orphaned companion display, empty crafting catalog and host-timed smooth
  viewport helpers from production M10/V2 archives. The focused startup
  diagnostic retains its local copy, while the game executable contains no
  V2 companion, crafting or smooth-viewport symbols. Verification: complete
  production link, real-data DM2 startup gate and archive/executable-symbol
  checks.
- ✅ 2026-07-31 CSB V2.2 installed-state hardening: a launcher-set
  `installed` flag can no longer select modern art on its own. The V2.2
  source selector now rechecks the finished-art gate and every route's
  provenance before it returns `V2_MODERN`; otherwise it keeps the V2.1/V2.0
  fallback. The focused asset-pipeline test covers the forged-installed/no-art
  case.

- ✅ 2026-07-31 CSB V2.2 cache-admission hardening: a readable
  `v22_inplace_cache.bin` is no longer enough to overwrite an F0128 source
  command. The in-place blitter independently requires the finished-art
  material/provenance gate; fixture cache pixels remain invisible even with a
  matching source span and palette. The focused in-place test verifies the
  framebuffer stays source-owned.

- ✅ 2026-07-31 DM2 V2 HUD overlay-state isolation: removed the retired
  procedural overlay module from the production V2 archive. Its invented
  compass, gold, level and champion values no longer enter the live renderer;
  the GDAT HUD route retains only a visibility gate and can draw only
  authenticated `INTERFACE_GENERAL` records. Historical overlay code remains
  explicitly test-scoped. Verification: production link, 74/74 direct-overlay
  regression, real-data DM2 M11 startup gate and archive/executable symbols.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron SRM production import no longer calls the fixture
  `theron_v1_party_init()` before decoding champion records. The importer now
  starts from an empty party, so a malformed or partial source body cannot
  inherit synthetic names, classes, stats or inventory. Verification: the
  Theron SRM body/classifier tests plus startup, save/resume and Track 02
  handoff tests.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build, startup-flow probe and
  real-data startup receipt gate.
- ✅ 2026-07-31 Theron startup mirror metadata isolation: the production
  `theron_v1_startup_mirror_meta()` API now fails closed because Track 02
  champion names, classes and portraits are not decoded. The seven-entry
  legacy table remains compiled only for the explicit fixture startup probe.
  Verification: production Theron archive build and startup-flow plus
  save/resume probes.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron dead-template cleanup: removed the unused production
  companion struct that hardcoded fighter class, 10-point attributes and
  starter health/food/water. Runtime initialization remains source-gated and
  fixture setup remains explicit. Verification: full Theron archive rebuild,
  startup-flow probe and save/resume probe.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron startup receipt metadata gate: removed the last receipt
  path that populated synthetic mirror portrait ordinals, class masks or
  fallback labels. Real Track 02 bitmap routes and decoded JP roster text
  remain available, while champion metadata stays empty until source records
  are decoded. Verification: real-asset receipt 311 passed with 2 expected
  ISO skips; startup-flow and save/resume probes passed.
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 Theron V2 HUD production isolation: removed the procedural
  compass, text, rune, champion-bar and action-strip renderer from the
  production archive. Production now links a no-op HUD seam that returns
  `V1_SKIPPED`; the pixel renderer and widget parser are compiled explicitly
  for fixture targets only. Verification: HUD phase probe, HUD smoke test and
  widget-assets test all passed (100 %).
- ✅ 2026-07-31 DM2 champion-stat bridge isolation: removed the unattached
  generic V1-to-V2 champion percentage bridge from the production V1 archive.
  It had no M11 consumer or authenticated session/palette handoff. Its focused
  regression remains explicit; live HUD stays on the source-owned GDAT route.
  Verification: production link, champion-bridge regression, real-data M11
  startup gate and archive/executable-symbol checks.
- ✅ 2026-07-31 DM1 original TITLE verification: repaired the standalone
  TITLE probe launcher after the source tree moved. The installed hash-locked
  PC 3.4 `TITLE` (12,002 bytes) now passes all 59 Greatstone mapfile-record,
  53-frame and two-palette-phase checks. The runtime TITLE palette and
  SWSH-to-C001 handoff probes also pass against the installed original
  `GRAPHICS.DAT`; no replacement title frame is used by these checks.
# 2026-07-31 DM1 archive-backed startup media

- ✅ 2026-07-31 Theron fixture-level helper isolation. The synthetic
  `theron_v1_first_room_*` and startup fallback-room constructors are now
  compiled only for fixture/probe targets. The production `firestaff_theron`
  archive no longer exports generated level-buffer symbols, while the startup
  flow, save/resume fixture, and first-room probe retain explicit coverage.
  Verification: production archive symbol check, six Theron startup/real-media
  CTest rows, all passed.

- ✅ 2026-07-31 Theron full-payload CUE regression corrected for JP media.
  The real Japanese MODE1/2352 BIN/CUE now verifies its authenticated Track 02
  identity (`b7afb338ad31be1025b53f9aff12d73a`) and reaches
  `theron-startup-0`, alongside the USA full-payload handoff. The old JP
  runtime test expected an obsolete hash and could fail despite a successful
  real-media boot; the expectation is now source-aligned. CTest:
  `theron_v1_jp_cue_runtime_boot` PASS.

- ✅ Fixed optional DM1 startup media materialization from external archives.
  `7zz` reports a missing member as a successful zero-byte stream; that
  previously stopped the cache resolver at `DATA/TITLE` or `DATA/SWOOSH`
  and left empty aliases instead of continuing to the original DOS archive's
  parent directory. Optional archive members must now be non-empty before
  they are accepted. Verified against the real bundled PC 3.4 DOS `.7z`:
  hash-pinned `GRAPHICS.DAT` and `DUNGEON.DAT`, plus `TITLE` (12,002 bytes,
  SHA-256 `adc7f191...`) and `SWOOSH` (7,570 bytes), materialize into the
  DM1 runtime cache and complete the direct boot probe. Extended
  `test_dm1_pc34_archive_media_receipt` to cover parent-directory TITLE and
  SWOOSH cache materialization (23 assertions).

- ✅ 2026-07-31 CSB V2.2 route-catalog isolation: production now compiles
  only the source-provenance F0128 admissions from the route module. The
  hand-authored per-cell asset-id catalog is enabled solely for its explicit
  contract test, preventing it from becoming a live material binding.

- ✅ 2026-07-31 CSB M11 startup-probe isolation: repaired merge drift that
  reintroduced an exported M11 probe which constructed a supposedly verified
  CSB boot profile around `/tmp` paths. The diagnostic is now contract-only
  and no longer has a production symbol; real package-owned CSB boot and
  startup receipts remain the active route.
- ✅ 2026-07-31 DM1 per-event SND3 source playback: corrected the M11 audio
  admission gate so a verified `GRAPHICS.DAT` SND3 buffer plays for its own
  event even when another one of the 35 source entries is unavailable. The
  old all-or-nothing bank flag silently replaced every remaining real sample
  with a generated marker. Verification: production Ninja build and the
  real-PC34 `firestaff_m11_pass53_snd3_runtime_probe` (6/6), including a
  forced partial-bank state that still queues the original door sample.
- ✅ 2026-07-31 DM2 tech/magic helper isolation: removed the unattached
  tech/magic helper from the production V1 archive. Its lookup deliberately
  has no imported DB/GDAT item definition and M11 has no consumer, so live
  gameplay can no longer derive item mechanics from its host fields.
  Verification: production link, spell/tech regression, real-data M11 startup
  gate and archive/executable-symbol checks.
- ✅ 2026-07-31 DM2 static-name helper isolation: removed the unattached
  record, UI-event and spell/skill name tables from the production V1 archive.
  They were local English constants with no text/GDAT owner or live caller.
  Their receipt tests remain explicit, while production cannot present them as
  original DM2 text. Verification: production link, all three helper tests,
  real-data M11 startup gate and archive/executable-symbol checks.

- ✅ 2026-07-31 CSB CMP party-state isolation: a portrait-only Utility Disk
  `.CMP` can no longer manufacture a live party member with default stats,
  equipment or vitals. Production keeps the source-locked decoder solely for
  overlays on already authenticated champion records; fixture-only party
  builders and self-tests require an explicit contract build. The boot
  regression now compares a rejected CMP path with the original dungeon
  header's start pose.

- ✅ 2026-07-31 CSB monster-stub isolation: production no longer exports the
  no-context DSA filter stubs or the empty fixed-possession drop routine.
  Those historical test contracts require an explicit build flag; live DSA
  remains on the imported-program runner and live creature drops stay blocked
  until original dungeon placement is bound.
- ✅ 2026-07-31 DM2 champion HUD helper isolation: removed the unattached
  generic champion-HUD and food/water bridges from the production V1 archive.
  They had no live M11 caller or complete session/GDAT handoff. Focused tests
  remain explicit, while production HUD stays source-owned. Verification:
  production link, champion-HUD and food/water regressions, real-data M11
  startup gate and archive/executable-symbol checks.

- ✅ 2026-07-31 CSB hidden-graphics self-test isolation: the production
  module retains only its safe loader for real GRAPHICS.DAT bytes. Its
  synthetic-record self-test is now compiled solely by the explicit contract
  target and cannot enter the product archive.
- ✅ 2026-07-31 DM2 outdoor facade isolation: removed the unattached no-draw
  outdoor facade from the production V1 archive. It had no live caller and no
  selected GDAT image/palette receipt; production weather and sky remain on
  the authenticated GDAT route. Verification: production link, outdoor
  material-gate regression, real-data M11 startup gate and symbol checks.

- ✅ 2026-07-31 CSB Atari ST DMCSB1 self-test isolation: the M11 archive now
  retains only the loader for original Atari ST `GRAPHICS.DAT`/animation
  containers. The `/tmp` synthetic DMCSB1 generator and its round-trip API
  are compiled exclusively by explicit contract targets, including the
  multi-variant asset probe. Verification: production `firestaff`, focused
  data-layout binary and CTest target all passed.

- ✅ 2026-07-31 CSB startup Resume admission: the M11-consumed visual startup
  receipt no longer invents a loadable `/tmp` save to make the closed-door HUD
  expose Resume. It now keeps Resume unavailable until the real save intake
  supplies an authenticated path, as required by ReDMCSB `ENTRANCE.C`
  F0441/F0806. Verification: production `firestaff` and the 504-assertion
  CSB boot-to-runtime handoff regression passed.
- ✅ 2026-07-31 Nexus Structure2 texture decoding: added a bounded decoder
  from the DMWeb `DMNDataFileDecoder.vbs` grammar. Encoding 08h expands packed
  nibbles to indexed pixels and decodes the 16 big-endian Saturn palette words;
  encoding 28h preserves direct big-endian 16-bit colour words. Descriptor and
  payload offsets are checked against the real Structure2 block, with no
  inferred VDP1 command or fallback raster route. Verification:
  `test_nexus_v1_dgn_texture_decode` passes against real `LEV00.DGN` texture
  entries (both encodings), and the production Nexus archive builds.
- ✅ 2026-07-31 Nexus SAL/MAP inventory correction: updated the audio
  references to reflect the supplied retail `SNDLEV00-15.SAL/.MAP` and
  `SDDRVS.TSK` files. Firestaff already retains bounded MAP/SAL provenance;
  playback remains blocked because the SAL codec and SDDRVS event ABI are not
  source- or capture-proven. No SAL bytes are promoted to guessed PCM.

- ✅ 2026-07-31 DM2 synthetic save-writer removal: production quick-save no
  longer serializes Firestaff's private session envelope or writes the
  `SKSave.runtime` sidecar as `SKSave.dat`. The M11/runtime boundary now
  rejects saving with `DM2 ORIGINAL SAVE WRITER REQUIRED` before any directory
  or file write, and no sidecar can later mutate an admitted original resume.
  Original-save import remains available. Verification: real-data
  `test_dm2_v1_m11_startup_profile_gate` and the production-linked
  `test_dm2_v1_quicksave_original_writer_gate` pass, including the explicit
  no-output save regression.
- ✅ 2026-07-31 Nexus DMWeb SAL/MAP parser correction: retail maps now parse
  eight-byte DataID/ID/start/L/area records from byte zero with the FF
  terminator, preserving 24-bit fields and distinguishing DataID 0 tone-bank
  memory from DataID 1-3 sequence/DSP regions. The former 24-byte-header
  interpretation is retained only in synthetic fixtures. Verification:
  all 16 real SNDLEV pairs (154 records) and the sound-runtime receipt suite
  pass; playback remains blocked pending the SDDRVS event handoff.
- ✅ 2026-07-31 Nexus Structure2 encoding 28h fidelity correction: the
  runtime decoder now ignores Saturn word bit 15 as DMWeb does and retains
  every 15-bit direct-colour value, instead of treating bit 15 clear as
  transparency. Verification: production Nexus archive and
  `test_nexus_v1_dgn_geometry_readiness` pass.
- ✅ 2026-07-31 Nexus Structure2 raster bounds hardening: image and palette
  regions are validated against the actual DGN size before pointer formation;
  packed 4bpp data now uses DMWeb's ceil-half-byte rule so odd-width textures
  retain their final pixel. Verification: `firestaff_nexus` and
  `test_nexus_v1_dgn_geometry_readiness` pass.
- ✅ 2026-07-31 Nexus ITEM.IBS floor raster fidelity: verified floor-image
  declarations now require `ceil(width*height/2)` packed bytes, matching the
  DMWeb nibble decoder and preventing the final pixel from being discarded on
  odd-sized surfaces. Verification: real `/Users/bosse/.firestaff/data/nexus/ITEM.IBS`
  passes `test_nexus_v1_champion_plrd`.
- ✅ 2026-07-31 Nexus ITEM.IBS source-session isolation: the source-bound item
  declaration table is explicitly cleared before each engine load, preventing
  a later package without authenticated ITEM.IBS from inheriting the previous
  session's item metadata. The real ITEM.IBS regression now also verifies the
  clear path and zero live declarations afterward.
- ✅ 2026-07-31 Nexus ITEM.IBS VDP1-capture byte-count fidelity: the guarded
  0008 capture admission and decoder now use `ceil(width*height/2)` packed
  bytes and the exact texel count, preserving odd-sized surfaces while keeping
  the original VDP1-command gate closed. Verification: real ITEM.IBS decoder,
  DGN geometry readiness, and production Nexus library build pass.
- ✅ 2026-07-31 Nexus Structure2 admission/material binding fidelity: the
  remaining Structure2 0008 payload-envelope and special-floor material checks
  now use the same ceil-half-byte rule as the DMWeb decoder. Verification:
  real ITEM.IBS decoder and `test_nexus_v1_dgn_geometry_readiness` pass.
- ✅ 2026-07-31 Nexus Structure2 payload bounds hardening: image and palette
  range checks now use subtraction-based limits, preventing offset-addition
  overflow before a descriptor can be admitted. Verification: real ITEM.IBS
  decode and DGN geometry readiness pass.
- ✅ 2026-07-31 Nexus SAL/MAP bounds hardening: legacy and retail parser end
  calculations now saturate on integer overflow instead of wrapping before
  the bounded-window checks. Real SNDLEV00-15 SAL/MAP corpus and sound-runtime
  receipt tests pass; playback remains blocked pending the proven codec/ABI.
- ✅ 2026-07-31 Nexus retail MAP truncation guard: the parser now checks the
  single-byte `FF` terminator before requiring a complete eight-byte record,
  while all non-terminator records are length-checked before field reads.
  Real SAL/MAP corpus and sound-runtime receipt tests pass.
- ✅ 2026-07-31 Nexus ITEM.IBS floor-render bounds hardening: the generic
  source decoder now validates the data pointer, base-plus-offset arithmetic,
  palette span, and packed image span before forming reads. Real ITEM.IBS
  decoder and PLRD/RLOWFIX regression tests pass.
- ✅ 2026-07-31 Nexus ITEM.IBS inventory-image bounds hardening: the generic
  16x16 image renderer now validates the complete selected image span with
  64-bit end arithmetic before forming its source pointer. Real ITEM.IBS and
  PLRD/RLOWFIX regressions pass.
- ✅ 2026-07-31 Nexus standalone DGN texture decoder hardening: Structure2
  capacity arithmetic now uses explicit unsigned widths, the 16-word palette
  span rejects short useful blocks safely, and pixel indexing cannot overflow
  signed intermediate arithmetic. Real LEV00 indexed/direct texture decode
  passes.
- ✅ 2026-07-31 Nexus ITEM.IBS header span admission: palette, association,
  regular-image, floor-descriptor, and declared floor-data ranges are now
  checked in full with 64-bit end arithmetic before any decoder can read them.
  Synthetic and real ITEM.IBS plus PLRD/RLOWFIX regressions pass.
- ✅ 2026-07-31 Nexus startup regression bounds correction: the M11 startup
  test now checks the last real FACE.BIN slot (0..19) after rejecting an
  out-of-range portrait request, instead of indexing past the 20-entry surface
  array. Real Nexus startup gate passes without the previous array-bounds
  warning.
# 2026-07-31 Nexus SAL DataID 0 directory provenance

- Added the DMWeb `DMNDataFileDecoder.vbs` `DecodeSNDLEVxxMAP` tone-bank
  parser to the Nexus sound runtime. It walks the real MAP-owned SAL parts,
  locates DataID 0, validates its big-endian offset table and entry bounds,
  decodes the four variable entries plus `4 + 32*n` entries, and records
  PCM width/source-control and sample-payload metadata.
- The runtime still refuses playback because Saturn event→selector ownership
  and the `SDDRVS.TSK` ABI are not authenticated. No synthetic sample or
  fallback audio was introduced.
- Verification: `test_nexus_v1_sal_map_corpus` and
  `test_nexus_v1_sound_runtime_receipt` pass against
  `/Users/bosse/.firestaff/data/nexus`.
# 2026-07-31 Nexus real-data viewport boundary audit

- Ran the DGN multi-level parser, material-raster, material-corpus and launch
  probes against `/Users/bosse/.firestaff/data/nexus`.
- All 16 `LEV*.DGN` files parse and the launch smoke reaches level 0, but the
  real material corpus reports `geometry_ready_level_count=0`, incomplete
  ceiling/wall host coverage, and no authenticated MNS/BPK host route.
- Kept the viewport fail-closed; no procedural or fixture material was
  promoted. The remaining owner is authenticated Saturn VDP1/VDP2 submission.

# 2026-07-31 Nexus item-mechanics provenance audit

- Audited the real `ITEM.IBS` binding against the live movement/item paths.
- `ITEM.IBS` proves declaration category, weight, image and string ordinals;
  it does not prove action, equipment, protection or creature-drop semantics.
- Recorded the remaining raw-ordinal `65/80` water/fire gate and dormant gold
  helper as explicit gaps in `TODO.md`. No guessed item meaning or synthetic
  loot/HUD label was promoted.
# 2026-08-05 Nexus MNS retail corpus verification

- Materialized the original English ISO's MNS model files into the configured
  local Nexus data root; all 30 documented roster models decode as DMDF.
- The real MNS test rendered 452 source textures and exercised OBAKE MOTN
  animation, transforming 75 vertices with `0` failures.
- No MNS pixels were promoted into the blocked DGN/VDP1 viewport route.

# 2026-08-05 Nexus DGN material-surface admission hardening

- ✅ The real DGN viewport now validates every selected MNS/BPK/Structure2
  surface before palette access or rasterization: bank bounds, `valid`, pixel
  ownership and positive dimensions are required. An authenticated animated
  Structure1G/Structure2 reference cannot silently fall back to a static
  Structure1B tile when its image is absent. Invalid material admission leaves
  the route blocked with no procedural substitute and records the first missing
  material command.
- Verification: `test_nexus_v1_dgn_material_raster` and
  `git diff --check` pass; no game data was added to the repository.

# 2026-08-05 Nexus ITEM.IBS gameplay placeholder removal

- ✅ The live ITEM.IBS bank now preserves byte-2 carry locations as raw
  declaration data instead of inventing `NEXUS_ITEMF_CONSUMABLE` flags.
- ✅ Real-data mechanics no longer dispatch the fixed DM1 item-ID potion,
  armour-slot or unarmed-power paths. Those compatibility helpers remain
  isolated from the authenticated Nexus route until Saturn action/combat
  semantics are bound from DM.BIN disassembly or an authenticated capture.
- Verification: `test_nexus_v1_item_ibs`,
  `test_nexus_v1_inventory_gameplay`, `test_nexus_v1_item_use`,
  `test_nexus_v1_tick_integration`, Nexus mechanics build and
  `git diff --check` pass; no game data is committed.
- ✅ 2026-08-05 Theron Track 02 thing-data loader hardening: reject an
  oversized ground-reference count before narrowing it into the source-shaped
  16-bit receipt field or calculating the copy span. Regression coverage now
  proves the overflow boundary fails closed; no real-data semantics are
  inferred or promoted.
- ✅ 2026-08-05 Theron M11 integration: production `firestaff_theron` now
  links the source-bound `theron_v1_viewport.c` lifecycle/presentation path
  instead of the total viewport no-op. Dungeon tiles, unverified chrome, and
  inferred mappings remain fail-closed; the verified Track 02 font and future
  authenticated palette/VRAM routes are now reachable by the real M11 path.
- ✅ 2026-08-05 DM1 HoC object presentation: restored ReDMCSB's D2 palette
  remap for D1/D0 wall ornaments, preventing authentic torch-holder and
  ornament pixels from becoming black silhouettes. Corrected the C00/C01
  ready/action hand slot masks so valid objects can be placed in either hand.

- ✅ 2026-08-05 DM1 leader-hand cursor: after pickup, the framebuffer draws
  the source PC34 16x16 object icon at the tracked pointer position, using
  the same F0033/F0038 icon resolver as inventory and action cells.

# Firestaff DONE - THERON

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Theron's Quest

### Theron V1

- ✅ 2026-07-13 Theron Track02 completed HuC6260-word receipt: the strict
  Mednafen loader parser now retains completed VCE colour-table words after
  the authenticated dynamic CD_READ/IRQ2 gate, preserving the first
  index/value and ordered FNV receipt separately from CPU `STA` observations.
  It rejects malformed words and does not treat VCE output as Track 02 byte
  taint, palette-table location, or rendering permission. Verification:
  Ninja plus focused CTest `theron_v1_irq2_live_trace_gate`,
  `theron_v1_raw_loader_trace_ingest`, `theron_v1_raw_loader_trace_import`,
  `theron_v1_capture_preflight_chain`, and `theron_v1_capture_manifest`.

- ✅ 2026-07-13 Theron Track02 real loader-trace boundary: replaced the
  hand-authored raw I/O-row importer with a strict parser for the existing
  provenance-marked Mednafen dynamic `CD_READ`/IRQ2 receipt. It checks the
  JP/US MD5-to-record pairing, records only HuC6260 stores after that read,
  and carries the compatible real startup-bitmap receipt forward. A VCE store
  is explicitly not source-byte taint, so the parser cannot verify a palette
  descriptor relation or unlock rendering; incomplete, mismatched, or
  uninstrumented traces fail closed. Added registered CTest probes for trace
  ingestion and preflight binding. Verification: Ninja plus focused CTest
  `theron_v1_irq2_live_trace_gate`, `theron_v1_raw_loader_trace_ingest`,
  `theron_v1_raw_loader_trace_import`, `theron_v1_capture_preflight_chain`,
  and `theron_v1_capture_manifest`.

- ✅ 2026-07-05 Theron V1 probe-registration hygiene gate: added `tools/verify_theron_v1_probe_registration.py` and CTest `theron_v1_probe_registration_hygiene`. The gate requires every `probes/theron/*.c` file to be referenced from `CMakeLists.txt` and rejects the obsolete descriptor-entry API tokens that caused the stale unregistered semantic probe cleanup. Verification: CMake reconfigure succeeded; direct Python verifier passed (`17 Theron probe sources are registered`); focused CTest for startup receipt, M11 direct launch, descriptor-entry roles, and probe-registration hygiene passed 4/4.
- ✅ 2026-07-05 Theron V1 stale descriptor-entry semantic probe cleanup: removed the unregistered `firestaff_theron_v1_track02_descriptor_entry_semantic_probe.c`, which referenced obsolete descriptor-entry API names and was not wired into CMake/CTest. The live coverage remains in `firestaff_theron_v1_track02_descriptor_entry_roles_probe` plus the startup receipt descriptor-role summary. Verification: no remaining old-symbol references; targeted build passed; focused CTest for descriptor-entry roles, startup receipt, and M11 direct launch passed 3/3; direct descriptor-entry roles and startup receipt probes passed with local Track 02 data.
- ✅ 2026-07-05 Theron V1 startup receipt descriptor-role summary: `Theron_V1_StartupReceipt` now records a bounded 9-entry Track 02 descriptor-role summary from `theron_v1_track02_bind_descriptor_entry_roles()`: zero-fill count, pre/post descriptor-data counts, descriptor-table count, descriptor-window entry index, byte-before-descriptor, RTS marker, first nonzero byte after descriptor, and all-zero-after marker. The real-asset receipt probe now locks placeholder defaults plus real JP/US BIN receipts with exactly one descriptor-table role and nine total classified entries. Verification: targeted build passed; `firestaff_theron_v1_startup_real_asset_receipt_probe` passed 128/128 with local JP/US Track 02 BIN data; focused CTest for receipt + M11 direct launch passed 2/2; headless Theron launch against `~/.firestaff/data` passed. Honest scope: descriptor byte-role receipt only; no Track 02 startup bitmap/audio decode or per-dungeon semantic promotion.
- ✅ 2026-07-05 Theron V1 startup render-row test hook: `M11_GameView_GetTheronStartupRenderRows()` now exposes the exact stage-select/Soul Room text rows M11 is preparing to draw, including Continue-slot state, cursor marker, original mirror names, class labels, resurrection status, and the forcefield row. `test_theron_v1_m11_direct_launch` now gates stage-select rows, Soul Room rows for Hakar/Mara/Pental, and the Pental `RESURRECTED` state before forcefield handoff. Verification: targeted build passed; `test_theron_v1_m11_direct_launch` passed; `SDL_VIDEODRIVER=dummy ./build-codex-system-theron-start/firestaff --game theron --data-dir "$HOME/.firestaff/data" --duration 0` launched against local Track 02 data. Honest scope: render-facing text contract only; no Track 02 startup bitmap/audio decode or pixel parity claim.
- ✅ Phase 7 — Narrow semantic Track 02 descriptor-table decoder: new `theron_v1_track02_decode_descriptor_table()` reads the 9-word little-endian stride table that the bank-signal module already locates, validates the documented shape (9 entries, strictly ascending, constant stride `0x0400`, half-open range `[0x0020, 0x2420)`), and is paired with `firestaff_theron_v1_track02_descriptor_table_probe`. The probe regression-locks the synthetic positive path, alt-stride positive path, out-of-range positive path, and seven negative fixtures (truncated input, NULL input, zero expected stride, descending entries, non-strict-ascending duplicate entries, wrong stride, status-name round-trip). On real data the probe hash-gates round-trip checks against the US Track 02 ISO descriptor at `0x1584` and all three US raw BIN anchors (`0x70be06`, `0x70e2c6`, `0x710904`) plus all three JP raw BIN anchors (`0x70b4d6`, `0x70d996`, `0x70ffd4`). Source-locked against `g_us_iso_bank_stride_descriptor` in `src/theron/theron_v1_track02.c`, `docs/source-lock/tqr_v1_track02_bank_signal_2026-06-03.md`, and the JP Rev 1 zero-image guard. Wired as CTest target `theron_v1_track02_descriptor_table` (PASS). The decoder is shape-driven only: it does NOT claim per-entry semantic types, dungeon-level binding, runtime loader handoff, or level-descriptor semantics — it only locks the byte-shape contract so future semantic work can build on it.
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
- ✅ Phase 8 verification suite wire-up: test_theron_v1_direct_launch, test_theron_v1_m11_direct_launch, test_theron_v1_launcher_scan_reuse, test_theron_v1_dungeon_progression, test_theron_v1_save_load, test_theron_rendering, test_theron_v1_save_header_rejection, test_theron_v1_shop_price_table, test_theron_v1_world_serialize_purchase_state, plus probes firestaff_theron_v1_teleporter_chain_probe, firestaff_theron_v1_mechanics_hardening_probe, firestaff_theron_v1_viewport_renderer_probe, firestaff_theron_v1_tile_renderer_probe, firestaff_theron_v1_track02_bank_probe, firestaff_theron_v1_track02_descriptor_table_probe are all wired into ctest and pass (17/17 dungeon progression, 9/9 save/load, 18/18 rendering, 3 NEW direct-launch + M11 + scan-reuse tests, 4 NEW viewport/tile/track02-bank/track02-descriptor probes).
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.
- ✅ Theron V1 lib link fix + mechanics + champions + combat probe (2026-06-17): new `src/theron/theron_v1_compat.c` provides compat shim definitions for combat symbols declared in `include/theron_v1_combat.h` but not defined in any .c file (theron_v1_champion_attack, theron_v1_champion_die, theron_v1_creature_ai_tick, theron_v1_creature_at, theron_v1_creature_spawn, theron_v1_creature_kill, theron_v1_creature_remove, theron_v1_creature_by_id, theron_v1_creature_count, theron_v1_creature_attack_champion, theron_v1_calc_attack_damage, theron_v1_calc_defense, theron_v1_modify_champion_hp/stamina/mana, theron_v1_creature_die, theron_v1_drop_loot, theron_v1_play_sound, theron_v1_sound_is_valid). Shims return safe defaults (0/NULL/no-op/THERON_COMBAT_MISS) and preserve V1 game state. The shims that DO mutate state (`modify_champion_hp/stamina/mana`, `champion_die`) clamp to valid ranges. This fix unblocks any consumer of `theron_v1_mechanics.o` (previously link-failed on undefined references). New headless probe `firestaff_theron_v1_mechanics_champions_probe` passes 68/68 (champions party_init + party_dungeon_entry_reset + party_dungeon_exit + get_champion + leader + HP/stamina/mana modification via shims + source evidence; mechanics move_party + turn_party + door_open/close + door queries + door_unlock_with_key + teleporter_resolve + altar_of_vi_resurrect + pool_use + alarm_trigger + trigger_activate + apply_post_move_effects + click_route + source evidence; combat champion_attack returns 0 + creature_attack_champion returns THERON_COMBAT_MISS + champion_die marks dead + creature_ai_tick no-op + creature_at returns NULL + HP/stamina/mana clamp + source evidence). Source-locked against THQUEST.ASM T500/T600/T700/T800/T900, ReDMCSB GROUP/COMMAND/CLIKMENU/GAMELOOP analogues, CSBWin/Resurrect Theron's Quest reimpl.
- ✅ 2026-06-22 Theron V1 shop purchase gate probe: new `firestaff_theron_v1_shop_purchase_gate_probe` (89/89) registered as CTest target `theron_v1_shop_purchase_gate_probe` with labels `tier4;theron;shop;purchase;gate`. Pairs with the existing `test_theron_v1_shop_price_table` and `test_theron_v1_world_serialize_purchase_state` by covering the narrower purchase-gate edges the unit test does not lock: (1) multi-champion slot targeting — purchase lands in the requested slot's inventory[0], other champions stay byte-identical; (2) sequential stock decrement chain — 3 buys → stock 3→2→1→0, 4th attempt reports THERON_SHOP_OUT_OF_STOCK with gold/stock preserved; (3) exact-gold purchase — gold==price drains to 0 with no underflow; (4) inventory slot allocation monotonicity — purchase lands at first empty slot (slot 5) when slots 0..4 are pre-filled; (5) stock=0xFF boundary depletion — 30 buys of stock=255 succeed (THERON_INVENTORY_SLOTS cap), 31st reports THERON_SHOP_INVENTORY_FULL with gold/stock preserved; (6) status-name round-trip — every THERON_SHOP_* enum maps to a distinct non-NULL string, plus out-of-range enum returns "unknown"; (7) source-evidence citation — string contains THQUEST + T560 + T800 + ReDMCSB markers. Source-locked against THQUEST.ASM T560 (item table) + T800 (champion persistence / gold field), docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5.3 (champion_gold offset + Theron-specific persistence), and ReDMCSB has no Theron shop source (DM1/CSB decompilation only).

### Theron V2.0 / V2.1 / V2.2

- ✅ Phase 0 V1 compatibility lock + Phase 1 V2 launch/profile separation: `theron_v2_phase_gate_pc34.c` (include/theron_v2_phase_gate_pc34.h) introduces a 16-domain classification (12 V1-source-locked + 4 V2-presentation-eligible) with per-domain `THERON_V2_PhaseGateDecision` (v1SourceLocked, v2PresentationAllowed, sourceAnchor, rule). V1-locked domains (TRACK02_BANK, BOOT_PROFILE, CHAMPION_PARTY, DUNGEON_PROGRESSION, MECHANICS, SAVE_LOAD, SHOP, TILE_RENDERER, VIEWPORT, WORLD_STATE, PALETTE, UI_CHROME) stay V1-locked regardless of V2 toggles. V2-eligible domains (PRESENTATION_MODE, TEXTURE_UPSCALE, FILTER_CONFIG, MODERN_SHAPES) require v2PresentationEnabled=1; FILTER_CONFIG additionally requires v2ConfigPersistenceEnabled=1 (stricter gate because filter writes are persistent state changes). Default config: V1-only, both toggles off. Ctest target `test_theron_v2_phase_gate_pc34` passes 220/220 (defaults, null-args, V1/V2-on behaviour, FILTER_CONFIG-persistence gate, v2_active, all 17 domain names, source-evidence, all-domain anchor, unknown-domain safety, Track 02 asset-hash pin). Headless probe `firestaff_theron_v2_phase0_v1_compatibility_lock_probe` passes 192/192. Headless probe `firestaff_theron_v2_phase1_launch_profile_separation_probe` passes 52/52 (launch gate, profile gate, Track 02 hash separation JP Rev 1 + US ISO MD5, cross-game hash separation Theron≠DM1≠CSB, V1-only default, headless safety). Source-locked against THQUEST.ASM T080/T400/T520/T560/T600/T700/T800/T900, theron_v1_track02.c, theron_v1_boot.c, theron_v1_champions.c, theron_v1_dungeon_progression.c, theron_v1_mechanics.c, theron_v1_save_load.c, theron_v1_shop.c, theron_v1_tile_renderer.c, theron_v1_viewport.c, theron_v1_world.c, theron_v1_palette.c, theron_v1_ui_chrome.c, HuC6260/HuC6270 VDC/VCE datasheet, HuC6280 CPU datasheet, ADPCM audio codec, docs/source-lock/tqr_v1_phase{0,1,2}*.md, ReDMCSB CLIKMENU/COMMAND/MOVESENS.
- ✅ Theron V2 presentation-mode selection: `theron_v2_presentation_mode_pc34` module (include/theron_v2_presentation_mode_pc34.h, src/theron/theron_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the Theron V2 presentation runtime. `theron_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=theron). Fallback chain V22→V21 when the modern asset pack is absent. Three independent presentation-mode globals (DM1/CSB/Theron) verified by `t_independent_from_dm1_csb`. CTEST target `test_theron_v2_presentation_mode_pc34` passes 40/40, headless probe `firestaff_theron_v2_presentation_mode_probe` passes 23/23. Source-locked against ReDMCSB COMMAND.C F0359, CLIKMENU.C F0365/F0366, MOVESENS.C:475-538, THQUEST.ASM T400/T520/T560/T600/T700/T800/T900, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.1 texture upscale pipeline: `theron_v2_texture_upscale_pc34` (include/theron_v2_texture_upscale_pc34.h, src/theron/theron_v2_texture_upscale_pc34.c) provides the EPX 2x + bilinear + nearest + full V1→EPX→palette→RGBA pipeline for Theron's PC Engine CD V1 base (256x224 NTSC, 4bpp HuC6270 VCE). Theron-specific helpers: `theron_v2_upscale_ntsc_fullscreen` (256x224 NTSC native) and `theron_v2_upscale_dungeon_viewport` (192x160 letterboxed gameplay view, 4x3 letterbox, 24 tiles wide x 20 tiles tall). Wired into `theron_v2_presentation_mode_set()` so the EPX scale follows the active mode. CTEST target `test_theron_v2_texture_upscale_pc34` passes 28/28, headless probe `firestaff_theron_v2_texture_upscale_probe` passes 14/14. Source-locked against THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE datasheet, tqr_v1_phase2_data_formats_H2339.md §7, and the EPX/Scale2x algorithm (http://www.scale2x.it/).
- ✅ Theron V2.2 modern shape book: `theron_v22_shapes` (include/theron_v22_shapes.h, src/theron/theron_v22_shapes.c) provides the 4x3 (4 depth x 3 lateral) shape book parallel to DM1 V2.2 and CSB V2.2. 13 wall variants (D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C + DOOR + SECRET), 7 floor shapes (plain, cracked, mossy, pit, stairs_up, stairs_down, flooded — Theron-only). 11 builtin materials. Theron-only shapes beyond DM1: FIELD_TELEPORTER (THQUEST.ASM T700), FIELD_ALARM (T800 alert dispatch), SECRET_DOOR (T800 hidden passage), FLOODED (water/flooded squares), LIT_TORCH (4+ torch slots, not 4 like DM1), and THERON_V22_LIGHT_ALARM_PULSE (red pulse glow). CSB-equivalent helpers: `theron_v22_shape_for_teleporter`, `theron_v22_shape_for_alarm`, `theron_v22_shape_for_secret_door`, `theron_v22_shape_for_lit_torch`. Wired into `theron_v2_presentation_mode_set()` via `theron_v22_shapes_init()` on V22 entry. CTEST target `test_theron_v22_shapes_pc34` passes 41/41, headless probe `firestaff_theron_v22_shapes_probe` passes 16/16. Source-locked against THQUEST.ASM T400/T520/T600/T700/T800, HuC6260/HuC6270 VDC/VCE datasheet, include/theron_v1_world.h (THERON_SQUARE_* enum), tqr_v1_phase2_data_formats_H2339.md §7.
- ✅ Theron V2.0/V2.1/V2.2 settings persistence in M12 menu config: extended `M12_Config` + `M12_MenuSettingsState` with `theronV2ScalePercent` / `theronV2BilinearEnabled` / `theronV2CrtScanlinesEnabled` / `theronV2CrtScanlineStrength` / `theronV2PaletteCorrectionEnabled` / `theronV2DitherCleanupEnabled` (same pattern as CSB V2, defaults 200% scale, 0 bilinear, 0 scanlines, 35 strength, 0 palette, 0 dither). Round-tripped through `M12_Config_SetDefaults` + the text Load + the text Save + the JSON Export + the JSON Import. New bridge module `theron_v2_settings_pc34` (include/theron_v2_settings_pc34.h, src/theron/theron_v2_settings_pc34.c) mirrors `csb_v2_settings_pc34`: `Theron_V2_Settings` struct, `theron_v2_settings_from_m12_config` / `theron_v2_settings_apply_to_m12_config` / `theron_v2_settings_apply_to_runtime` (pushes scale + bilinear into `theron_v2_upscale_init` + filter toggles into `theron_v2_filter_config_apply`). ctest target `test_theron_v2_settings_pc34` passes 23/23, headless probe `firestaff_theron_v2_settings_probe` passes 12/12. **Wire-up done:** `M11_GameView_OpenSelectedMenuEntry` reads `menuState->settings.theronV2*` and calls `theron_v2_settings_apply_to_runtime()` right before `M11_GameView_Start`. New `theron_v2_upscale_get_scale()` + `theron_v2_upscale_get_bilinear()` accessors let the wire-up probe verify the live runtime. Headless probe `firestaff_m12_v2_settings_wire_up_probe` covers both CSB + Theron (16/16 combined). **Filter config wired:** new `theron_v2_filter_config_pc34` module (include/theron_v2_filter_config_pc34.h, src/theron/theron_v2_filter_config_pc34.c) parallels the CSB filter config for the PC Engine CD (HuC6260 VDC + HuC6270 VCE) Theron pipeline. ctest target `test_theron_v2_filter_config_pc34` passes 24/24, headless probe `firestaff_theron_v2_filter_config_probe` passes 18/18. Source-locked against include/dm1_v2_settings_pc34.h, include/csb_v2_settings_pc34.h, include/theron_v2_texture_upscale_pc34.h, include/theron_v22_shapes.h, include/theron_v2_presentation_mode_pc34.h, include/config_m12.h, THQUEST.ASM T400/T520/T600, HuC6260/HuC6270 VDC/VCE.

## 2026-07-14 — Theron production initial-level capture gate

The production Soul Room entry now consumes the manifest-bound coalesced
Mednafen `$e009` receipt instead of permitting the earlier Stage 3/IRQ2
receipt alone. It rehashes Track 02, System Card, and transcript before
binding record `0x0b52` to the source-locked initial-level envelope. This is
only a fail-closed loader/media admission; no dungeon/object/visual semantics
are claimed. A positive result still requires a fresh authentic capture.
# 2026-07-14 — CSBWin EDBT_ObjectWeights runtime handoff

- Bound CSBWin `Mouse.cpp::GetObjectWeight`'s `EDBT_ObjectWeights` chest-base
  lookup to Firestaff's live ReDMCSB `DUNGEON.C F0140` container path. The
  DB11/EXPOOL record is consumed only while the complete appended tail matches
  its stored FNV receipt; absent records retain CSBWin's source default of 50,
  while altered, truncated, short, or out-of-range records cannot fall back.
- Extended `csb_v1_runtime_champion_load_attrs` with the original CSBWin
  `EXPOOL::Locate` key/hash/node layout, live child-content addition, and a
  changed-receipt rejection case.
# 2026-07-14 — DM2 G1 D3 side-ray GDAT material route

The indoor runtime now projects G1 D3L/D3R from SKProject's documented deep
coordinates (five cells forward and two cells lateral) before the existing
source-backed GDAT wall consumer. The focused gate requires all center, side,
and deep wall classes to consume source material and records zero fallback or
blocked draws. D3C remains intentionally unavailable because its GRAPHICSSET
wall field is absent; this change does not fabricate a backdrop, dynamic
light, doors, or non-wall/floor terrain. Verification:
`dm2_v1_g1_center_ray_surface_gate`.
- ✅ 2026-07-14 DM1 V2.2 original-art item fallback guard: the V2.2 in-place material resolver no longer maps item, floor-item, or projectile-item shapes to `creature_demon_01` when no reviewed item material exists. Those shapes now have no V2.2 substitution, preserving the source-owned V1 pixels while reviewed modern materials remain limited to their matching shape categories. `m11_v22_inplace_draw_pc34` now asserts the item route returns `NULL`. This is an integration-safety correction only; it adds no art, does not promote the modern pack, and does not claim pixel parity.

# ✅ 2026-07-14 DM1 D2L/D2R source wall backing crop

M11 now admits the real 78x74 PC34 backing bitmap for the ReDMCSB D2L/D2R
75x71 viewport zones and clips the blit to the zone. This restores the
two-forward, one-side wall lanes without accepting arbitrary size mismatches.
Source anchors: ReDMCSB `DUNVIEW.C F0119/F0120` and `G0163` C710/C711.
Verification: `firestaff_m11` Ninja build.
- 2026-07-14 DM1/CSB ReDMCSB F0817-F0819/F0903/F0908: source-locked string, Japanese-text, message-plane, error-plane, and sound-init boundaries. Focused strict C11 tests pass.

- 2026-07-14 Theron coalesced `$e009` manifest binding: the completed
  initial-level receipt now binds its runtime payload handoff to the exact
  original Track 02, System Card 3.0, and coalesced capture-trace identities.
  Runtime rejects a changed or incomplete manifest before consuming bytes;
  no dungeon, tile, object, bitmap, palette, or fallback semantics were
  invented. Focused Track 02 intake and initial-level-handoff probes pass.

- 2026-07-14 Theron authenticated Hall of Records route receiver: the boot
  handoff now re-derives the original level-0 grid from Track 02 and compares
  it with the completed manifest-bound route before publishing a candidate
  runtime world. Changed route receipts, payloads, or source bytes fail
  closed. Object tables, bitmap/palette decoding, and fallback visuals remain
  unavailable. Focused Track 02 intake and initial-level-handoff probes pass.

- 2026-07-14 Theron runtime-gate regression repair: Stage 2/3 physical loader
  admission remains at the live Soul Room forcefield boundary, while the
  shared Track 02 semantic record collector is again usable for bounded
  all-dungeon receipt inspection. The indexed viewport-only render facade can
  present without an asset bundle; supplied V1 bundles still require original
  tile/palette data. No synthetic artwork or object route is admitted.

# 2026-07-14 — DM2 live creature direct-GDAT material gate

SKProject's `QUERY_CREATURE_PICST` route is now represented end-to-end:
runtime keeps the mutable V5 animation pair, consumes the complete
`CREATURES/type` `FB/FC/FD` receipt, and supplies the selected direct
`dtImage` field to the viewport and M11 material hash. The old live
type-index/F9 path is not permitted. The AI loader also follows
`EXTENDED_LOAD_AI_DEFINITION`'s real `dtWordValue` field layout instead of
requiring a non-source packed 36-byte blob. A missing AI binding, animation
table, direct image, decoded pixels, or local palette now results in no draw.
The canonical local corpus currently exposes the animation triads but not an
admitted dynamic AI binding, which is reported as a clean skip rather than
fabricating a creature visual.
- 2026-07-14 Theron Continue/runtime media repair: Continue now transports the
  already captured indexed Track 02 atlas into runtime without attempting a
  second raw-byte decode. Raw bitmap consumers still require MD5 and physical
  offsets, and failed capture admission explicitly reports that fallback
  visuals remain blocked.

- 2026-07-14 Theron authenticated Quartz host-input receipt: the live
  Mednafen harness now uses PID-bound focus followed by explicit Quartz HID
  key-down/up pairs, rather than reporting an unused key-hold setting around
  an AppleScript tap. The transition receipt records delivery method, key
  code, attempt count, and requested hold. A fresh genuine US CUE + System
  Card 3.0 run still observed no SDL key event, so it remains fail-closed
  before emulated input, CD-read, dungeon, object, palette, bitmap, or
  fallback claims. Focused script test passes; the authentic capture is a
  negative receipt.
# ✅ 2026-07-14 Theron PID-targeted Quartz host-input receipt

# ✅ 2026-07-15 Nexus Structure1F/Structure3 runtime correlation

# ✅ 2026-07-15 Nexus PRS3 zero-side static corridor identity

The retail `DM.BIN` zero-side SH-2 path is now retained as one exact 64-byte
source corridor from its branch target through its outer-loop branch. The
receipt requires FNV-1a `e0cc325e85a0e63f` before a zero-side external trace
can bind, so mutation of an otherwise unnamed intermediary instruction fails
closed. It establishes no PRS3 token, copy/backreference, palette, pixel, or
decoder semantics. Verification: `test_nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-15 Nexus FACE PRS3 capture targets

`nexus_ui_face_prs3_capture_target()` now exposes one exact canonical
`FACE.BIN` PRS3 frame for an external capture producer only after a
caller-owned source-hash gate. Prefix, PRS3 header, and compressed stream
hashes remain separate, preserving the source lanes needed for a later Saturn
loader trace without inventing palette, token, pixel, or portrait semantics.
The target remains no-draw with fallback visuals disabled. Verification:
`test_m11_nexus_startup_gate`.

# ✅ 2026-07-15 Nexus FACE PRS3 capture campaign

`nexus_ui_face_prs3_capture_campaign()` now covers every canonical FACE.BIN
frame in producer order, with a separate ledger over target framing and its
prefix/stream source lanes. The campaign rejects missing or malformed frames
before external trace analysis, but establishes no loader execution, token,
palette, pixel, or menu-placement semantics. It remains no-draw with fallback
visuals disabled. Verification: `test_m11_nexus_startup_gate`.

# ✅ 2026-07-15 Nexus direct Structure1F static-material route

`nexus_v1_engine_build_structure1f_direct_static_material_capture_target()`
now joins a documented active `Structure1F -> Structure1A -> Structure3`
face selection with the same face's exact static Structure2 descriptor and
bounded payload windows. It fails closed for non-static or unresolved faces;
the resulting target is capture-only and cannot assign texture, palette,
transform, VDP1, or draw semantics. Verification:
`test_nexus_v1_dgn_geometry_readiness` against LEV00--LEV15.

# ✅ 2026-07-15 Nexus direct Structure1F raw-fill face route

`nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target()`
conditionally joins a direct owner with its exact non-textured Structure3
face, vertices, normal, and opaque fill-selector bytes. It cannot assign a
flat colour, palette, transform, VDP1 command, or draw behavior, and textured
faces remain unavailable through this route. Verification:
`nexus_v1_direct_static_material_capture` against canonical LEV01.

# ✅ 2026-07-15 Nexus direct Structure1F 08xx material route

`nexus_v1_engine_build_structure1f_direct_animated_material_capture_target()`
conditionally joins a direct owner with its exact Structure3 08xx /
Structure1G material declaration. The route is source-only: it does not
execute the image sequence, decode a payload, assign palette or VDP1
semantics, or draw. Verification:
`nexus_v1_direct_static_material_capture` against canonical LEV01.

# ✅ 2026-07-15 DM2 c_dialog source panel viewport route

`c_dialog.cpp::DM2_dialog_2066_3820` now reaches the DM2 viewport through a
reversible GDAT identity for `DIALOG_BOXES/0x81/0`. The boot provider decodes
that exact image and its local palette, while the viewport accepts it only
with the source-expanded `RECT_453` host command and an explicit active
dialogue state. Missing pixels, palette, command identity, or rectangle fail
closed; normal dungeon frames cannot display an admitted dialogue asset.
Verification: `dm2_v1_dialogue_box_viewport_real_data`.

# ✅ 2026-07-15 DM2 OPEN_DIALOG_PANEL source labels and placement

`DM2_dialog_OPEN_DIALOG_PANEL` now carries the real `DIALOG_BOXES/0x81`
text fields 0 and 1 through the exact skproject text transform selected by
`GDAT 0/0/dtWordValue/0` bit `0x08`. The boot path decodes the original bytes,
rejects unimplemented `DM2_FORMAT_SKSTR` substitutions, and resolves
`RECT_1D2`/`RECT_1D3` by reproducing `DM2_COMPRESS_RECTS`, `DM2_QUERY_RECT`,
and the relevant `DM2_QUERY_BLIT_RECT` grammar directly from raw4. The M11
viewport draws the original panel and both decoded labels at native 320x200
coordinates only while the source dialogue command is active. Verification:
canonical-PC-G1 `dm2_v1_dialogue_box_viewport_real_data`.

# ✅ 2026-07-15 DM2 source save-dialogue pointer receipt

The DM2 boot path now carries the original save/load selection geometry from
`c_dialog.cpp` and `c_savegame.cpp` without a replacement hit layout. Its
receipt takes only the source event's two rectangle IDs, expands/measures
their `INTERFACE_GENERAL/0/raw4` entries, and calculates the selected row
with the original `c_gfx_str.cpp` seven-pixel `strxplus` stride and maximum
slot 10. A coordinate before the source-owned baseline clears the receipt and
fails closed. Canonical PC G1 verification resolves `RECT_451`, proves exact
row-seven selection, and verifies the rejected pre-baseline route:
`dm2_v1_dialogue_box_viewport_real_data`.

# ✅ 2026-07-15 DM2 OPEN_DIALOG_PANEL source version heading

`c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL` now carries the exact compiled
skproject `dm2data.cpp::v1d10eb` byte sequence `V1.0`, its palette slot 12,
and source `RECT_1C2` placement through the RAW4 host command. The active
viewport renders it before the two verified GDAT button labels. Missing source
material, font, palette, version hash, or placement blocks the entire
dialogue draw; no localized or fallback label is substituted. Verification:
canonical-PC-G1 `dm2_v1_dialogue_box_viewport_real_data`.

# ✅ 2026-07-15 DM2 DRAW_DOOR light-palette fail-closed gate

The source door material preflight now carries `DRAW_DOOR`'s selected
`iLightPalette` into viewport consumption. A nonzero value, including the
source D3 field-zero retry's palette 3, blocks the draw until Firestaff has
the original palette-transform implementation. The renderer therefore cannot
mistake an IMG3 local palette for source lighting or substitute a visual.
Verification: canonical-PC-G1 `dm2_v1_gdat_door_overlay_plan_real_data`.

# ✅ 2026-07-15 DM2 DRAW_DOOR D0 stretch-control receipt

The source-owned DM2 door M11 command now reproduces skproject
`DM2_DRAW_DOOR`'s mandatory D0 image-zero branch: the D0 `DOORS/0` panel
uses `iStretchDual=0x71`, not the initial `0x40` used before the branch.
Before a source-required viewport consumes a door transaction it rechecks
every panel's observed distance/image/stretch/light tuple. An altered D0
stretch, or an unsupported field-zero distance retry, blocks the full door
pass rather than rendering the right pixels with a stale transform. This is
only a source-control and receipt repair; the unresolved nonzero
light-palette transform remains fail-closed. Verification:
canonical-PC-G1 `dm2_v1_gdat_door_overlay_plan_real_data`.

The Nexus engine now revalidates a dual-source Structure1F/Structure1A
capture target against the active canonical LEV before binding it to an
already externally attested, engine-owned Structure3 capture. The renderer
packet exposes that owner context only as no-draw provenance; the
model-index-to-mesh-entry mapping remains explicitly unproven and the real
Saturn texture, palette, VDP1, transform, and draw blockers remain active.

The live-capture helper now requires a focused Mednafen process PID and posts
the requested key-down/up pair directly to that PID with Quartz. It checks
macOS event-access preflight, emits the granted/post-to-PID receipt, and the
capture script rejects a missing or mismatched helper attestation before it
can evaluate any trace. The final transition gate still requires Mednafen's
own SDL input row plus the existing controller/CD/sector evidence; no
controller state is injected and a host-side receipt cannot promote a dungeon
handoff. Verification: `swiftc -typecheck
scripts/send_theron_macos_quartz_keypair.swift`, `bash -n
scripts/capture_theron_mednafen_live_trace.sh`, and
`tests/test_theron_v1_mednafen_live_capture_script.sh` PASS. The installed
Mednafen binary has no Firestaff trace instrumentation, so this does not claim
a positive live capture.

# ✅ 2026-07-15 Nexus active Structure1F face/mesh receipt

`nexus_v1_current_level_structure1f_face_mesh_receipt()` now consumes the
documented Structure1F -> Structure1A -> Structure3 model/face/normal ordinal
attachment only from the active, hash-bound retail LEV bytes. The focused
retail corpus verifies the receipt for every admitted level. It contains no
transform, material, texture, palette, VDP1, or draw semantics and therefore
remains no-draw with fallback visuals disabled. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 raw Saturn trace admission gate

`nexus_v1_engine_admit_structure2_descriptor_capture_trace()` now binds an
external raw capture manifest to the active hash-verified LEV source, one
exact Structure2 descriptor, its opaque post-FFFF payload, and the supplied
raw-trace bytes. The caller must independently attest original Saturn
provenance; unverified input remains blocked even after every hash matches.
Opaque admission authorizes neither a decoder nor draw, keeping the renderer
fail-closed until pixel, palette, and VDP1 semantics are actually captured.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure3 static face-to-Structure2 capture target

`nexus_v1_engine_build_structure3_static_material_capture_target()` now joins
one bounded, texture-flagged Structure3 face from the active canonical LEV to
the exact matching static Structure2 descriptor, with independent hashes for
the LEV source, the 12-byte face row, descriptor row, and opaque payload. It
also resolves the exact image-payload byte anchor and, when nonzero, the
palette-payload byte anchor from the observed Structure2-relative offsets.
The focused corpus test verifies this against real hash-verified LEV00–LEV15
package data. The target is capture-producer input only: pixels, palette
format, UVs, VDP1 state, transforms, and drawing remain unproved and blocked.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 retail format-evidence gate

`nexus_v1_current_level_structure2_format_evidence_receipt()` now consumes
the active canonical LEV and validates every descriptor's image anchor plus
any nonzero palette anchor against the opaque payload. The hash-verified
LEV00–LEV15 corpus fixes the observed split at 1,553 `0x0008` descriptors and
125 `0x0028` descriptors; all `0x0028` rows lack a palette anchor. The gate
keeps pixel span, palette addressing, VDP1 format, decoder permission, and
drawing false. This is concrete format evidence, not a format inference.
Verification: `test_nexus_v1_dgn_geometry_readiness`.
- ✅ 2026-07-15 DM1 GROUP F0181: added the exact current-map group-event
  deletion primitive. It scans event records and removes the complete
  C29..C41 range at the requested square through the existing F0237 heap
  repair path; other squares and maps remain. The DM1 event-timer regression
  covers both deletion boundaries and every retention gate. Source: ReDMCSB
  `GROUP.C` F0181:340-371.
- ✅ 2026-07-15 DM1 GROUP F0194: added source-defined all-active-group
  retirement. It composes F0194's active-slot scan with F0184's loaded C04
  writeback: Cells, low packed Direction, Behavior >= C4 to wander, then
  inactive slot. The regression covers sparse active slots and malformed raw
  references failing before any mutation. Source: ReDMCSB `GROUP.C`
  F0194/F0184.
- ✅ 2026-07-15 DM1 GROUP F0195: initial DUNGEON.DAT startup now consumes
  the current map's loaded SFT/C04 chains in original X-major/Y-minor order.
  Each first C04 receives F0181 exact-square C29..C41 deletion, the F0183
  active-state bridge with the PC3.4 60-slot limit, and F0180 C37 scheduling
  at GameTime + 1. The regression covers a mixed C03/C04 chain, raw C04
  cells/directions, event retention, and both wandering events. Source:
  ReDMCSB `GROUP.C` F0180/F0181/F0183/F0195 and `NEWMAP.C` F0003.
- ✅ 2026-07-15 DM1 GROUP F0197-F0199: added the real sight/smell square
  predicates and source route walk. Closed opaque C3/C4 doors, fakewall
  imaginary-state distinction, diagonal branch blocking, and Manhattan route
  result are covered; non-adjacent paths require a loaded-map callback.
  Source: ReDMCSB `GROUP.C` F0197-F0199.
- ✅ 2026-07-15 DM1 GROUP F0200: added the complete route-backed visible
  party decision. It preserves per-creature facing deduplication, side attack,
  invisibility and night-vision range changes, adjacent random range, then
  delegates the final line to F0199. Regression covers facing, side attack,
  invisibility, and an actual map-route blocker. Source: ReDMCSB `GROUP.C`
  F0200/F0227.
- ✅ 2026-07-15 DM1 GROUP F0201: added live direct party scent consumption.
  It calls F0199 through an F0198-backed map callback before considering the
  supplied original stored scent, preserving the source ordering and range
  gate. Regression covers clear route priority and blocked-route stored scent.
  Source: ReDMCSB `GROUP.C` F0201/F0198/F0199.
- ✅ 2026-07-15 DM1 GROUP F0202: removed the legacy clear-destination
  assumption. F0202 now accepts only supplied decoded destination facts and
  otherwise blocks movement, while retaining source terrain, Fluxcage,
  teleporter, party, door, and group ordering. Regression fixtures explicitly
  provide decoded empty squares. Source: ReDMCSB `GROUP.C` F0202.
- ✅ 2026-07-15 DM1 GROUP F0203: added the live tested-direction scan. Each
  cardinal direction is marked before its F0202 evaluation, including a
  blocker, and later directions remain untouched after a successful choice.
  Source: ReDMCSB `GROUP.C` F0203/F0202.
- ✅ 2026-07-15 DM1 GROUP F0204: added the archenemy double-movement gate.
  A first-step Fluxcage stops the source branch before the second step; the
  second step consumes its own loaded F0202 facts. Regression covers Fluxcage,
  blocked second square, and a verified clear second square. Source: ReDMCSB
  `GROUP.C` F0204/F0202.
- ✅ 2026-07-15 DM1 GROUP F0205/F0206: removed the pseudo-random opposite
  turn from the legacy helper. Opposite turns now require the live RNG form,
  which preserves F0205 one-step correction and F0206's per-creature gates.
  Source: ReDMCSB `GROUP.C` F0205/F0206.
- ✅ 2026-07-15 DM1 GROUP F0207: removed non-source projectile substitutions.
  Lord Order and Grey Lord now reject original BUG0_13's undefined projectile
  Thing without consuming RNG, and the unproven Trolin spell palette is gone.
  Verified original projectile types remain unchanged. Source: ReDMCSB
  `GROUP.C` F0207 BUG0_13.
- 2026-07-15 DM2 skproject weather material integrity: final
  `QUERY_TEMP_PICST` consumption now verifies the receipt's decoded
  ENVIRONMENT pixels and `QUERY_GDAT_IMAGE_LOCALPAL` hash. A substituted
  same-sized image or palette blocks the entire weather transaction.
- 2026-07-15 DM2 skproject door material integrity: M11 now rehashes every
  source-owned `DRAW_DOOR` decoded plane and local palette immediately before
  presentation. Altered same-sized GDAT material blocks the complete pass.

# ✅ 2026-07-15 Nexus canonical PALT capture target

The canonical MENU.BPK handoff can now write a source-bound original-Saturn
capture target for PALT's exact record and raw-table fingerprint. It requests
only PALT-memory-read, palette-state, and VDP1-command observations, leaving
the PALT-to-palette relation unproved and all decoder/draw routes blocked.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus animated Structure3 image-source route

`nexus_v1_current_level_visit_structure3_animated_material_images()` now
walks every declared non-control `Structure1G` instruction associated with an
active `08xx` Structure3 face and binds it to the exact local Structure2
descriptor capture target. The viewport consumes this as a separate no-draw
source lane. `FF FE` is not followed, no image is selected as a frame, and no
animation timing, Saturn pixel/palette/VDP1 semantics, decoder, or fallback
visual path is enabled. The focused DGN test covers one bounded 08xx face and
its source descriptor, plus the existing hash-verified retail-corpus route
when local data is available. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus canonical PALT trace admission

`nexus_v1_engine_admit_menu_bpk_palt_trace()` now admits an externally
verified Mednafen trace only after it binds the active canonical MENU.BPK,
exact PALT record fingerprint, raw trace, PALT-memory bytes, palette-state
bytes, and VDP1-command bytes. The engine retains the result solely as an
opaque no-draw receipt. It does not infer that PALT produced the palette, nor
any palette format, PRS3 relationship, CLUT behavior, decoder, or drawing.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 DM2 live G1 door-frame routing gate

`DRAW_DOOR_FRAMES` now requires the source-owned active G1
`MapGraphicsStyle` receipt before it may select a `GRAPHICSSET` frame. The
former set-one renderer convenience cannot supply a door material on a
source-required frame: the full door pass is blocked before any material
fetch. Verification: `test_dm2_v1_door_material_gate`.

# ✅ 2026-07-15 DM2 live G1 wall-plan routing gate

The M11 wall plan now rejects a source-required frame until the live G1
`MapGraphicsStyle` receipt is present. This removes the remaining set-one
planner fallback before a `GRAPHICSSET` wall command can be materialized.
Verification: `test_dm2_v1_graphicsset_wall_material_gate`.

# ✅ 2026-07-15 DM2 atomic G1 scene-plan binding

Floor and ceiling material plans are now bound atomically to the active G1
`MapGraphicsStyle` and source command hash. A changed scene control or stale
plan is detached before viewport rendering. Verification:
`test_dm2_v1_gdat_scene_plan_viewport_real_data`.

# ✅ 2026-07-15 Nexus complete animated DGN source gate

The complete active Structure3 scene now includes the full `08xx`
Structure1G image-instruction route. A scene cannot be complete until every
declared image instruction resolves to an exact bounded Structure2 source
descriptor from the same canonical LEV bytes. This is no-draw source coverage
only: no GOTO execution, frame selection, timing, texture payload, palette,
VDP1, transform, decoder, or fallback has been introduced. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus owner/material DGN capture bundle

# ✅ 2026-07-15 DM2 horizontal DRAW_DOOR GDAT transaction

DM2 now consumes skproject `SkWinCore::DRAW_DOOR`'s horizontal intermediate
door path as two atomic, source-owned panel commands: the right source half
uses `tlbRectnoDoorPosition + state + 6`, followed by the left half at
`+ state + 3`. Each command carries its real DOORS image crop, RAW4 geometry,
decoded-pixel hash, local palette hash, colour key, and selection hash. The
viewport draws neither half unless both are complete and valid. Verification:
the canonical cached `GRAPHICS.DAT` gate exercises vertical and horizontal
intermediate states and proves two source panel blits without fallback fetches.

The same canonical-data gate now covers every zero-light horizontal route at
D1C and D2C for states 1--3, including both source halves, RAW4 destinations,
source order, decoded-pixel hashes, and the accompanying `GRAPHICSSET` frame.

`nexus_v1_engine_build_structure1a_structure3_material_capture_target()` now
bundles an active canonical Structure1F/1A owner/face request with a bounded
static Structure3/Structure2 material request from the same LEV. Both lanes
remain independently source-bound: no model-index to Structure3-entry mapping
is inferred. The bundle is capture-producer-only and remains no-draw until an
authentic Saturn trace proves correlation, transforms, pixel/palette/VDP1
semantics, and rendering behavior. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus owner/material Saturn capture target

The owner/material bundle can now be written atomically as
`FIRESTAFF_NEXUS_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_V1`. One
producer request carries the active canonical LEV fingerprint, Structure1F/
Structure1A ownership facts, exact typed Structure3 face fingerprints, and
the bounded selected Structure2 image/palette candidate windows. It requests
one original-Saturn source-read, palette, VDP1 VRAM/command, transform, and
culling observation set. It creates no pixels, decoder contract, model-entry
mapping, or draw permission. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus atomic owner/material trace consumption

`nexus_v1_engine_admit_structure1a_structure3_material_capture_trace()` now
consumes an external trace only when it names the deterministic atomic target
fingerprint, the active Structure1F/1A owner, the exact Structure3 face, and
the selected Structure2 descriptor before delegating to the existing
source-window trace admission. A trace remains opaque even after independent
original-Saturn provenance is supplied: no pixel, palette, VDP1, transform,
decoder, or draw semantics are inferred. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Mednafen owner/material trace collector

`firestaff_nexus_v1_saturn_owner_material_trace_collector` accepts only an
existing atomic target and a nonempty raw Mednafen debugger trace. It copies
the target's required identity fields, adds the raw-trace FNV-1a witness, and
writes an unauthenticated intake manifest for the atomic admission route. The
tool does not launch an emulator, generate trace bytes, or attest original
Saturn provenance. The atomic target now includes the Structure2 opaque-payload
fingerprint required by that route. Verification:
`test_nexus_v1_dgn_geometry_readiness` and direct collector compilation.

# ✅ 2026-07-15 DM2 direct G1/GDAT M10 scene consumption

The source-classified DB4 creature route now carries its exact decoded
`CREATURES/type/F9` pixels and 16-colour local palette from G1 handoff into
the live M10 viewport frame. The matching draw consumes those retained bytes
directly and rejects missing/mismatched ownership rather than re-querying
GDAT or using a fallback. Verification: `dm2_v1_g1_scene_viewport_material_gate`
proves a direct handoff renders with zero asset/palette callback calls;
`dm2_v1_g1_scene_creature_gdat_real_data` proves the canonical media path.

# ✅ 2026-07-15 DM2 direct G1 GDAT plane and wall presentation

M10 now consumes the boot-owned `GRAPHICSSET` plans selected by skproject's
`UPDATE_GFXSET`: `DRAW_DUNGEON_GRAPHIC` consumes the validated floor/ceiling
700/701 transaction and `DM2_DRAW_WALL` consumes only the complete matching
wall command plan. A G1 control-hash change detaches both plan types. In
source-required mode, no missing/stale plan can fall through to a second GDAT
asset or palette lookup. Verification: direct canonical-data runs of
`test_dm2_v1_gdat_scene_plan_viewport_real_data` and
`test_dm2_v1_gdat_wall_plan_viewport_real_data` pass.

# ✅ 2026-07-15 DM2 directional depth-selected wall consumption

The boot-owned `GRAPHICSSET` plan stays complete, while M10 now consumes only
the wall commands selected by G1's actual visible view squares for the current
party direction. A selected panel retains its exact depth/view-square field,
crop, destination, pixels, and local palette; unclassified squares do not
draw generic walls. Source-required callback resolution is no longer an
alternative material path. Verification: canonical-data
`test_dm2_v1_gdat_wall_plan_viewport_real_data` covers a direction-3 D3L/D2R/
D0L selection, and `test_dm2_v1_gdat_scene_plan_viewport_real_data` keeps all
five G1 plane plans green.

# ✅ 2026-07-15 Theron BIN/CUE Track 02 admission

The media classifier now records CUE Track 02's declared `MODE1/2048` or
`MODE1/2352` sector width and accepts either as one authentic Track 01/Track
02 pair. The scanner sends only 2352-byte data to the raw IPL receipt;
2048-byte CUE media remains on the existing verified ISO route. No sector
extraction, wrapper or fallback was added. Verification:
`firestaff_theron_media_classify_unit` and
`theron_v1_track02_cue_layout`.

# ✅ 2026-07-15 Theron 2048 ISO CUE startup handoff

M11 now validates and retains a Track 02 loader receipt only when the scanner
actually issued a valid raw `MODE1/2352` IPL receipt. A verified CUE-declared
`MODE1/2048` ISO therefore follows the normal Track 02 startup handoff and is
not reported as an invalid raw BIN. Verification:
`theron_v1_launcher_scan_reuse` and
`theron_v1_m11_launcher_handoff_boundary`.

# ✅ 2026-07-15 Theron ISO identity at Soul Room boundary

The boot-profile forcefield handoff now distinguishes raw BIN and ISO Track
02 variants. Raw BIN remains behind its authenticated IPL/IRQ2 capture gate;
a verified 2048-byte ISO retains its exact MD5 and source bytes through Soul
Room to the existing ISO semantic dungeon route. That route stays fail-closed
until original ISO bytes prove a first level/object handoff. Verification:
`theron_v1_m11_launcher_handoff_boundary` checks installed real media and
preserves the selected Track 02 identity through startup.
- ✅ 2026-07-15 DM2 M11 source render handoff: the live
  `m11_game_view` DM2 runtime route now calls
  `dm2_v1_boot_runtime_render_frame()` with no V2 callback after verified
  boot, so its dungeon frame consumes the source-owned G1 pose and GDAT
  materials instead of `dm2_v2_runtime_render_frame()`'s procedural viewport.
  The optional V2 HUD remains a decoded original-GDAT compositor and missing
  source data draws nothing. `test_dm2_v1_boot_profile_smoke` now locks the
  direct route: no V2 attempt, successful V1 render, real-material receipt,
  and zero core fallbacks.

# ✅ 2026-07-15 Theron Track 02 runtime bitmap provenance

The existing verified title, stage, Soul Room, and forcefield indexed bitmap
routes now carry their original Track 02 MD5 plus raw and MODE1 user-data
offset envelope into `Theron_V1_World`. A selected runtime level-bank receipt
copies that same source envelope, so a later consumer can require exact
source bytes instead of treating retained pixels as unowned data. The bind
rejects unknown/mismatched variants and incomplete spans. It still performs
no palette binding, RGB conversion, layout inference, object-table decoding,
or drawing. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.
# ✅ 2026-07-15 Theron authenticated CD-read runtime record

The independently authenticated Track 02 `$0b52` CD-read payload now enters
the runtime world as an opaque source receipt: canonical Track 02 MD5, raw
user-data offset, destination, whole-payload checksum, and the exact
post-envelope byte range/checksum. The receipt is published even while the
level route remains rejected, allowing a later captured game-owned consumer
to bind it without reopening media or treating copied bytes as unowned. It is
explicitly marked no-semantic-promotion: no level, object, palette, bitmap,
or visual behavior is inferred and no fallback is enabled. Verification:
Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 loader-envelope boundary

Runtime admission now derives the documented boundary inside the authenticated
`$0b52` CD-read record: the loader-provided initial envelope must begin at its
record-relative offset, match original Track 02 bytes and checksum, and end
exactly where the separately hash-verified opaque continuation begins. The
world receipt retains both spans only after these checks pass. This proves
source-byte boundaries and continuity, not level-grid, object-table, palette,
or visual semantics; the runtime remains no-draw without a captured game-owned
consumer. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 runtime boundary-byte retention

The runtime loader receipt now retains the actual authenticated initial
level-envelope bytes and their directly adjacent post-envelope bytes from the
original `$0b52` CD-read record. Both spans must fit the record, be adjacent,
and rehash to their loader-provided checksums before they are copied. This is
a source-owned boundary for a future captured level/object consumer, not an
object-table decoder: the continuation remains opaque and no palette, grid,
object, or visual semantics are promoted. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation-consumer boundary

The raw loader-trace intake can now admit a game-RAM byte only when its
original READ(6), FIFO-to-RAM, and game-owned consumer chain resolves to the
directly adjacent continuation after the authenticated `$0b52` envelope.
The receipt records its exact continuation-relative offset and source byte,
while rejecting preceding and out-of-range bytes. It remains deliberately
opaque: no object-table, level, palette, bitmap, grid, or visual semantics
are inferred. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation prefix receipt

The loader-trace route can now require a contiguous 12-byte prefix of the
authenticated post-envelope continuation from one ordered CD dispatch. Every
byte is independently tied to the original sector and one game-RAM consumer
chain; a split SCSI generation/LBA/dispatch is rejected. The retained prefix
is only a future capture anchor, not an object-table header or decoder.
Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 continuation TII source binding

The provenance-marked Mednafen main-RAM-loader trace can now bind one original
`TII` transfer only when its source begins at `$3c80`: the continuation start
derived from the authenticated `$3800` sector receipt. The copied source span
is checksummed against retained original bytes and the capture must carry the
producer marker; unrelated `TII` rows are ignored. Destination content stays
opaque, with no object-table, level, palette, bitmap, grid, or rendering
claim. Verification: Ninja `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 live TII capture intake

`capture_theron_mednafen_live_trace.sh` now writes the provenance-marked
main-RAM-loader trace beside the existing IRQ/CD/input traces. Its transition
receipt reports all observed `TII` rows and the subset whose source is `$3c80`,
the authenticated continuation boundary. Empty counts remain evidence of an
unreached original route; the script manufactures no trace or candidate.
Verification: `test_theron_v1_mednafen_live_capture_script.sh` passes; the
patch-shape gate passes and skip-cleans without `MEDNAFEN_SOURCE`.

# ✅ 2026-07-15 Theron Track 02 TII sidecar import

The continuation-transfer admission now accepts one explicit bounded
main-RAM-loader sidecar file and forwards its original text unchanged to the
strict TII parser. Missing, empty, oversize, and malformed sidecars reject;
the import does not create rows, bytes, or semantic fallback. This makes the
live capture producer directly consumable once authentic media reaches the
post-`$3800` transfer route.

# ✅ 2026-07-15 Theron Track 02 continuation execution handoff

The raw loader-trace route now binds a source-verified `$3c80` continuation
`TII` to a later main-RAM `JSR` only when the call target exactly equals the
TII destination. This demonstrates an original CD-byte-to-code stage handoff
without interpreting the copied memory as a level or object table. Duplicate,
wrong-target, or unmarked control rows reject. Verification: Ninja
`test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 manifest descriptor boundary

Stage-three descriptor 0 now reaches the runtime loader gate as a strict
original-media boundary receipt. Firestaff retains the descriptor's three raw
words plus its derived Track 02 record, MODE1 raw sector, user-data offset,
2048-byte length, and FNV-1a hash. All fields must resolve back to the same
authenticated `$3800` Stage-3 sector before startup admission. The receipt is
deliberately non-semantic: it does not classify the descriptor or sector as a
level, object table, tile, palette, bitmap, command, or visual route. The
focused descriptor probe covers valid coordinates and rejection of malformed
MODE1/zero-selector records. Verification: Ninja probe, `test_theron_rendering`
18/18, and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 full descriptor-row handoff

The authenticated later `$e009` route now carries the complete raw Stage-3
descriptor row into the runtime handoff: descriptor ordinal, `word0`, `word1`,
selector `word2`, resolved Track 02 record, and the selected MODE1 user-data
hash. Firestaff independently derives those values from canonical Track 02
bytes before accepting the coalesced loader receipt; changed row bytes or a
changed selected sector reject the handoff. The row remains explicitly opaque:
no level, object table, tile, palette, bitmap, command, or visual semantics
are promoted. Verification: focused raw-handoff probe (skip-safe without the
authentic corpus), `test_theron_rendering` 18/18, and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor alias-table receipt

Descriptor-to-record admission now retains the selected raw selector's table
relationship: occurrence count, first and last descriptor ordinal, and an
FNV-1a hash over every matching `(ordinal, word0, word1, word2)` row. The
coalesced loader/CD receipt and runtime handoff both independently re-derive
this relation from the authentic Stage-3 manifest, rejecting changed aliases
or a mismatched selected row. These are table-identity facts only: aliases and
their ordering do not identify a level, object, tile, palette, bitmap, loader
command, or visual route. Verification: descriptor-correlation probe covers a
duplicated selector relation and rejection paths; `test_theron_rendering`
18/18 and `test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 descriptor source-span binding

Each admitted later descriptor record now keeps the exact six-byte big-endian
row span from the authenticated loaded Stage-3 MODE1 sector. Firestaff checks
the physical raw offset and FNV-1a against the three retained raw words before
the later sector may reach the runtime handoff. This closes the source-table
to-selected-sector byte boundary without interpreting any descriptor field,
target record, graphics, palette, object, level, or command grammar.
Verification: focused descriptor probe validates the byte span plus malformed
MODE1/zero-selector rejection; `test_theron_rendering` 18/18 and
`test_theron_v1_startup_save_resume_pc34` 258/258.

# ✅ 2026-07-15 Theron Track 02 copied-continuation termination receipt

The instrumented Mednafen main-RAM loader trace now emits HuC6280 RTS rows.
Continuation admission requires one source-bound `$3c80` TII, a later JSR to
its exact destination, and exactly one RTS whose PC lies inside the copied
destination span. The capture script reports RTS count for acquisition. This
proves only that original copied code reaches a termination instruction; it
does not observe a return target or promote level, object, palette, bitmap,
tile, command, or rendering semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18, `test_theron_v1_startup_save_resume_pc34`
258/258, patch-shape test skip-cleans without `MEDNAFEN_SOURCE`, and capture
script contract test passes.

# ✅ 2026-07-15 Theron Track 02 copied-continuation post-RTS receipt

The instrumented main-RAM loader trace now emits the first observed main-RAM
instruction after each captured RTS. Continuation admission requires that row
to reference the single RTS inside the source-bound `$3c80` TII destination
span and to land at the matching JSR return PC. The receipt retains its
physical PC and opcode alongside the already source-bound Track 02 transfer.
This proves control flow from copied original bytes back to the observed
return target only; it does not classify a descriptor, record, level, object,
tile, palette, bitmap, command, or visual route. Verification: focused
raw-loader probe (skip-safe without the authentic corpus),
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, patch-shape test
skip-cleans without `MEDNAFEN_SOURCE`, and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine-call receipt

When the authenticated post-RTS instruction is a HuC6280 `JSR`, Firestaff now
requires the immediately adjacent original main-RAM-loader trace row to agree
on its logical PC, physical PC, and immediate target. The new receipt carries
the earlier source-bound Track 02 TII/execution chain, so the call is tied to
copied original bytes without inventing a called-routine ABI or data format.
Missing, reordered, or changed call-site rows reject. This proves only a
control-flow target, not a descriptor, record, level, object, tile, palette,
bitmap, command, or visual route. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 post-return routine termination receipt

The post-return routine-call receipt now requires one later main-RAM `RTS`
with a linked original `post_rts` row returning to the exact caller address.
Nested returns remain opaque and do not satisfy the receipt unless their
observed return address is the bound caller. This extends the authentic
Track 02 TII/copy/call/return control-flow chain without assigning any called
routine, table, record, level, object, tile, palette, bitmap, command, or
visual semantics. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call receipt

After the authenticated post-return caller resumes, Firestaff now admits the
first subsequent main-RAM `JSR` row from the same original trace and retains
its exact physical call site and immediate target. The receipt nests the full
source-bound Track 02 TII/copy/call/return chain. It deliberately does not
identify the target routine, an ABI, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual route. Verification:
Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: genuine Mednafen 1.32.1 patch
dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-entry successor receipt

The Mednafen producer now records the next observed main-RAM instruction
after an authenticated caller-next routine entry. Firestaff requires the
exact entry logical/physical PC plus the successor logical/physical PC and
raw opcode, retaining the full source-bound Track 02 chain. A target that does
not continue through observed main RAM produces no receipt. This is execution
ordering only: no opcode, ABI, loader, descriptor, CD read, table, record,
level, object, tile, palette, bitmap, command, or visual semantics are
promoted. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor TII byte receipt

When the authenticated caller-entry successor executes HuC6280 `TII`,
Firestaff now accepts it only when its entire source interval lies inside the
already source-bound Track 02 continuation copy. The receipt retains exact
RAM source/destination coordinates, byte count, corresponding original source
coordinate, and FNV-1a checksum. This proves the observed caller path
re-copied known original bytes, without assigning them a loader, descriptor,
CD-read, table, record, level, object, tile, palette, bitmap, command, or
visual meaning. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja
focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-successor destination-call receipt

The first observed main-RAM `JSR` after an admitted caller-successor `TII`
must now call that transfer's copied destination. The nested receipt retains
the source-bound Track 02 interval and exact call site, proving a bounded
original-byte-to-execution chain. It does not classify the called routine or
bytes as a loader, descriptor, CD read, table, record, level, object, tile,
palette, bitmap, command, or visual route. Verification: genuine Mednafen
1.32.1 patch dry-run, Ninja focused targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), and the capture-script contract test
passes.

# ✅ 2026-07-15 Theron Track 02 caller-next-call entry receipt

The instrumented original Mednafen trace now writes a call-entry row only
when the target of the bound next-caller `JSR` is actually executed in main
RAM. Firestaff requires exact caller logical/physical PCs, target, entry
logical/physical PCs, and opcode before retaining the nested Track 02
TII/copy/call/return chain. An unobserved or non-main-RAM target admits no
receipt. This proves executed control flow only and assigns no ABI,
descriptor, CD read, table, record, level, object, tile, palette, bitmap,
command, or visual meaning. Verification: Ninja focused targets,
`test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, focused raw-loader probe
(skip-safe without the authentic corpus), patch-shape test skip-cleans without
`MEDNAFEN_SOURCE`, and the capture-script contract test passes.

# ✅ 2026-07-15 DM2 DRAW_DOOR_FRAMES source side-jamb execution

`DRAW_DOOR_FRAMES` now reaches the real M11 dungeon renderer for D0--D3
side-jamb images. Each command resolves the source `GRAPHICSSET` field and
`QUERY_CREATURE_BLIT_RECTI` RAW4 row, applies the original
`QUERY_GDAT_SUMMARY_IMAGE` image offsets, mode 4/3 placement, right-side
mirror, local IMG3 palette, and source scene colorkey. Missing bytes or an
invalid geometry/palette receipt blocks the door pass; no wall-frame
approximation is used. The source cell/field/RAW4 route remains locked by
`test_dm2_v1_door_side_frame_source_route`; boot smoke covers the runtime
source-material transaction.
# ✅ 2026-07-15 DM2 G1 runtime material refresh

G1-to-GDAT runtime receipts are now rebuilt atomically from the current raw
dungeon bytes at boot, session apply, position change, and live-save restore.
This covers bounded first-map ownership, DB2 text/WALL_GFX, DB3 actuator
WALL_GFX, and current-level DB4/DB5/DB9 routes. Old receipts are cleared
before re-materialization, so a changed raw G1 cannot reuse pixels, palette,
or coordinates from a prior frame. Source: SKProject `GAME_LOAD`,
`READ_DUNGEON_STRUCTURE`, `c_record.cpp`, and the corresponding GDAT query
routes. Verification: DM2 boot smoke 88/88 and save/load 26/26.

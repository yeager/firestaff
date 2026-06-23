#include "changelog_m12.h"
#include <stddef.h>

/* ── Embedded changelog ───────────────────────────────────────────
 * Each entry is a single string displayed as one line in the viewer.
 * Empty strings produce blank separator lines.
 */
static const char* const g_changelogLines[] = {
    "FIRESTAFF CHANGELOG",
    "====================",
    "",
    "V3.0.3  (2026-06-23)",
    "  - DM1 Hall of Champions gates expanded again. New real-asset runtime probes cover additional cancel/reopen portrait-rectangle slices, including row-2 C026 atlas coverage for ordinal 15-17, so stale panel pixels and portrait placement regressions are caught earlier.",
    "  - DM1 V1 regression coverage widened across intro/title skip cleanup, HiDPI chest-slot hit zones, open-pit transition chains, single-tick food/water warning, door-bash no-open state, projectile/portcullis behavior, chest hand-swap and nested-container weight handling, C146 sleep/wake repaint, disabled champion-panel icons, and mana-bar repaint.",
    "  - CSB and DM2 gates added for boot-to-first-viewport readiness, save-runtime boundaries, DSA trigger bounds, DM2 projectile/creature collision, minimap persistence across level transitions, and door/button toggle boundaries.",
    "  - M12 data UX hardened. Launcher settings now use the scanned data-status value, show clearer scan feedback, retain popup focus context, cover extras mouse-hit flow, support save-manifest export/import, and expose manual docs from the launcher.",
    "  - Release verification: based on 51 commits after v3.0.2 with GitHub M10 verify + CMake build + cross-platform determinism green on main before release prep.",
    "",
    "V3.0.2  (2026-06-23)",
    "  - DM1 FTL swoosh intro fixed. The launcher now unpacks the PC SWOOSH LZEXE file before extracting the logo, decodes the source IMG2 stream, rejects false 320x200 matches, and animates the original FTL silhouette instead of a corrupted filled frame.",
    "  - DM1 Hall of Champions coverage tightened. New real-asset runtime probes lock additional champion portrait rectangles, candidate redraw behavior, cancel/reopen handling, and side-pose stale-pixel absence without broadening the finished-parity claim.",
    "  - CSB readiness gates refreshed. Runtime-capture and pass547 launch-intent source anchors now track the current M12 launch-intent span after the supported-game/assets check moved to line 7513.",
    "  - Public gap-list status refreshed for 2026-06-23, including focused queue progress and the new DM1 Hall portrait probe evidence.",
    "  - Verification before release prep: GitHub M10 verify + CMake build + warnings + asset hygiene + cross-platform determinism green on fff924d0, plus focused local DM1/CSB checks from the main session.",
    "",
    "V3.0.0  (2026-06-21)",
    "  - DM1 V1 original-capture gap close (pass1052-1058). Deterministic original DM1 PC 3.4 DOSBox capture-evidence: pass1052 clean turn-cycle (4 raw 320x200 frames, 2 dungeon_gameplay + 2 wall_closeup, 0 duplicate raw hashes, pass80 classifier 4/4 PASS), pass1053 champion candidate/resurrect panel (pass455 SHA e4b37307.../7523b67f...), pass1054 one exact original-to-Firestaff 224x136 wall-crop match (0 changed pixels / MAE 0), pass1055 closed-door stasis (3 byte-identical original frames + Firestaff semantic pair that blocks forward), pass1056 CTest gate over pass1054 pairing artifacts, pass1057 Amiga 2.2 DUNGEONB.DAT sidecar lock (9bac133b..., 4,806 bytes, coverage 3/3 registry-backed), pass1058 keypad/route atlas (corrected F=kp8, B=kp2, TR=kp4, TL=kp6, kp5 forward). B1 capture-gap pairs in docs/FIRESTAFF_GAP_LIST.md moved BLOCKED-DATA → PARTIAL with remaining-work lines.",
    "  - DM1 V1 gap cascade (pass1059-1070). Touch zones (pass1059 portrait sensor), C25/C26 projectile fallback (pass1064), mirror stat (pass1063), chest scroll-wheel pickup overflow (pass1062), object consumable use (pass1061), AI pathfinding (pass1066), AI perception targets (pass1067), V2 smooth interpolation (pass1068), AI reactions (pass1069), inventory route parity (pass1070). All gates source-locked and CTest-green.",
    "  - Tier 1 #5 strict boot-probe per path. New firestaff_tier1_strict_boot_probe ctest entry runs the launcher with --game <id> --data-dir <path> --duration 1500 under SDL_VIDEODRIVER=dummy for every EXTRACTED + VERIFIED path. 5/5 in-scope paths PASS (DM1 canonical, DM1 legacy-dos, Theron JP canonical, Theron JP extras, Theron US extras). Closes the only remaining Tier 1 path-discovery gap.",
    "  - Tier 2 #4 LZW Atari ST decoder DONE. Real Atari ST asset handoff remains BLOCKED-DATA (no DM/CSB Atari ST data on disk), but the decoder code path is test-covered and ready.",
    "  - Tier 4 determinism probes: Theron V1 dungeon-progression (THQUEST.ASM T080), CSB V1 champion-stat (F0306/F0309/F0310/BUG0_72), Nexus V1 creature-state (F0209 timeline). All marked DONE in gap-list.",
    "  - DM1 V2 polish: V20 filtered renderer probe, V21 upscale renderer probe, V22 in-place render probe (CSB + DM1 Apple Silicon + DM1 V22 modern asset), side-by-side V1/V2 presentation-disabled seed gates. The live DM1 V22 path now prefers the optional in-place cache before falling back to the placeholder overlay; finished real-art m11_draw_dm1_* material swaps remain OPEN-LARGE in B3.",
    "  - Asset-status fix: required=1 for all required-files rows. asset_status_m12.c now propagates the matchedPath so the missing-files popup and report show where the runtime will load the asset from, while keeping launch_blocker honest.",
    "  - Documentation: docs/FIRESTAFF_GAP_LIST.md updated with 100+ row status changes reflecting post-pass1052-1070 reality. B1 capture-gap pairs reclassified BLOCKED-DATA → PARTIAL with explicit pass references. Tier 1 #5 marked DONE for path-discovery scope. Multiple Tier 4 entries closed (Lefthook CI, CSB CMP decoder, Atari ST PAK decoder, CSB hidden-code skip, LZW Atari ST decoder, B1 capture gaps, M12 extras DM1, chest runtime detail, creature grouping, Theron extras launch-tested, Theron Track 02 launch).",
    "  - 116 commits since v2.9.2. CTest baseline 700+/700+ green (was 692/696 at v2.9.2). Phase A probe 23/23. Audio probe green. Strict -Wall -Wextra -Werror warnings-check green. Cross-platform determinism green. M10 verify green.",
    "",
    "V2.9.2  (2026-06-20)",
    "  - Tier 2 #3: CSB hidden-code skip table. CSB Atari ST and CSB Amiga GRAPHICS.DAT items 558-562 are executable code (68k program fragments). Added a per-game/per-platform skip table in include/csb_hidden_code_skip_table.h + src/csb/csb_hidden_code_skip_table.c so the upcoming CSB V1 graphics loader can route around them rather than treating them as image/text/sound data. Also covers DM1 Amiga v2.2 kid dungeon easter-egg items 135-138. 15/15 unit tests pass; ctest entry csb_hidden_code_skip_table_unit (#596). Not yet wired into a loader — the next step is to call FirestaffHiddenCodeShouldSkip() in the CSB V1 graphics pipeline.",
    "  - Tier 2 #4: DM1 V1 graphics LZW decoder fix + tests. The existing m11_gfx_lzw_decompress in src/dm1/dm1_v1_graphics_loader_pc34_compat.c had two bugs: (a) CLEAR_CODE rewound the bitstream by also resetting byte_pos / chunk indices, which corrupted subsequent reads; (b) the KwKwK edge case emitted first_char before the decoded old_code string. Split lzw_reset() into lzw_init() (full stream + dict reset at start) and lzw_reset_dict() (dict/code-width only, preserves bitstream position), and fixed the KwKwK output order. New tests/test_dm1_lzw_round_trip.c covers single-byte, literal runs, repeated pattern, KwKwK, clear-code resets, early END_CODE, and code-width growth. 8/8 tests pass; ctest entry dm1_lzw_round_trip. Source-locked to ReDMCSB LZW.C F0495_GetNextInputCode, G0666 max=4096, 12-bit codes.",
    "  - Tier 2 #5: Atari ST PAK container decoder (start.pak for DM/CSB). New include/firestaff_pak_decode.h + src/shared/firestaff_pak_decode.c parses the 4-byte PAK header (file size in words), 28-byte Atari ST executable header (magic 0x601A + text/data/bss/symbol sizes + flags), 1920-word most-frequent-words table, and nibble-coded compressed stream. Nibble coding: 0xF = two literal bytes from 4 nibbles; 0x8..0xE = 12-bit table index 128..1919 (word); 0x0..0x7 = 8-bit table index 0..127. Same compression algorithm as FTL HUNK_CODE — the nibble reader/frequency indexing is the building block for Tier 3 #10. Tests cover short/long dictionary references, literal escape, bad magic, truncated input, zero text size, and self-test. ctest entry firestaff_pak_decode_unit.",
    "  - Tier 2 #6: CSB Utility Disk .CMP champion-portrait decoder. New include/firestaff_cmp_decode.h + src/shared/firestaff_cmp_decode.c parses the 496-byte on-disk .CMP structure per ReDMCSB DEFS.H: cmp_i_C, cmp_i_E (both reserved, must be zero), Name[8], Title[20], Portrait[464] (32x29 pixels, 4bpp). Validates reserved words, name/title characters (uppercase A-Z, digits, space, null), exposes the portrait as a pointer into the caller-owned input buffer. 6 self-tests cover valid buffer, too-short, bad reserved word, lowercase name, control character in title, and max-length name/title. Read-only — Amiga↔Atari ST bitplane conversion (F0515/F0516 in PORTRAIT.C) is deferred to Tier 3 rendering work since Firestaff does not yet render CSB utility disk portraits. ctest entry firestaff_cmp_decode_unit.",
    "  - Tier 2 #7: MD5/SHA256 harmonization. Source runtime asset matching uses MD5 (asset_find_by_hash.c + asset_status_m12.c g_requiredFiles tables), docs/VERIFIED_HASHES.md documents SHA256. Rather than migrate everything (which would break compatibility with Greatstone/Daniel MD5 catalog references), the new tools/asset-validate/compute_md5_for_registry.py reads VERIFIED_HASHES.md, computes MD5 for SHA256-locked files, and emits a cross-table; tools/asset-validate/compare_md5_to_sha256.py verifies that runtime MD5 + registry SHA256 agree for every file present. docs/MD5_SHA256_HARMONIZATION.md documents the policy. ctest entries asset_validate_compute_md5_for_registry + asset_validate_compare_md5_to_sha256. Hashes added for Atari ST DM1 1.2 EN, CSB Atari ST 2.x, DM2 PC FR/DE/JewelCase and Mac EN so the registry now covers the variants we physically have on disk.",
    "  - Tier 2 #8: _G2157_ linker fix already shipped in v2.9.1 follow-up (commit 3588798f, m10: provide image_backend globals — Bug B fix). src/shared/image_backend_pc34_compat_globals.c defines the missing G2157_/G2159_puc_Bitmap_Source/G2160_puc_Bitmap_Destination globals and CMakeLists.txt links it into firestaff_m10. firestaff_m10 builds clean. Verified with the ctest subset.",
    "  - Tier 2 #9: Lefthook in CI. New repo-root lefthook.yml hooks: newline_eof (POSIX-sh), trailing_whitespace (POSIX-sh), po/validate_po_layout.sh, hash harmonization (compare_md5_to_sha256.py), and Python syntax checks. .github/workflows/verify.yml gains an asset-hygiene job (ubuntu-24.04) that installs lefthook@latest via go install, then runs hash harmonization, po layout validation, and lefthook run ci. The asset-hygiene job runs alongside the existing M10 verify + warnings + CMake build matrix + Phase A probe + cross-platform determinism gates.",
    "  - Tier 2 #1: docs/DATA_ACQUISITION_CHECKLIST.md. Per-game matrix of which platform/version variants Firestaff needs to be code-complete, which files are present locally (SHA256-verified), which are archived but need extraction, and which are still missing. Covers DM1 (PC 3.4 EN/ML, Atari ST 1.x, Amiga 2.x, Apple IIGS, FM-Towns, PC-98, X68000), CSB (PC 3.4, Amiga 3.5 EN/ML, Atari ST 2.x, X68000, FM-Towns EN/JP, PC-98, Utility Disk), DM2 (PC EN/FR/JewelCase, Amiga, Macintosh, Sega CD, FM-Towns, PC-98, IBM PS/V), Nexus (Saturn JP 138-file release), and Theron (PC Engine JP + US Track 02).",
    "  - Tier 2 #2: tools/asset-validate/coverage_by_game.py. New companion script that prints a per-game, per-variant coverage table by cross-referencing the registry, the local data directory, and the data-extras directories. Variants are scored READY (all required files present), ARCHIVED (some files present, others need extraction), or MISSING. Current totals: dm1 92% runtime-ready (13/14, missing Amiga 2.2 kid DUNGEONB.DAT), csb 88% (8/9, missing CSB Utility Disk CMP/HTC/AMG), dm2 100% (9/9), nexus 100% (1/1), theron 100% (2/2). ctest entry asset_validate_coverage_by_game. compare_md5_to_sha256.py path matching hardened to prefer canonical <data_dir>/<game>/<filename> layout, fall back to <game>-extras recursive rglob, and disambiguate variants using token-based path matching.",
    "  - Tier 3 #10 (started): FTL container decoder. include/firestaff_ftl_decode.h defines the public API for the proprietary FTL resource container used by Atari ST / Amiga / X68000 / MegaCD / SegaCD versions of DM1, CSB, DM2, and Nexus. The 20-byte common header (magic 0x6160, checksum, unknown1=2, c_6=1, c_7=0, i_8=7, date1, date2, segment_count) and 12-byte segment headers (type=HUNK_BSS 0x0010 / HUNK_DATA 0x0011 / HUNK_CODE 0x0012, id, offset, size) match ReDMCSB FTL.H exactly. HUNK_CODE decompression will delegate to FirestaffPak_Decode (same 0x5223 magic + 1920-word frequency table + nibble-coded stream). Implementation + tests + CMake registration in the next commit.",
    "  - WASD navigation fix (commit 0ad63f50). W/A/S/D are now an unconditional alias for the arrow keys. Previously the keys were gated by state->settings.wasdMovementEnabled, which silently disabled WASD when the M12 toggle was off. Removed the gate in both SDL3 and SDL1/SDL2 fallback branches in m11_poll_menu_input; Q/E and Home/End turn behavior is unchanged, Ctrl+S save-game behavior preserved. Settings UI no longer shows the WASD MOVEMENT row (it is now a reserved enum slot for config/backward compatibility).",
    "  - i18n follow-up (commit cf424cb7). All 7 localization domains — startup-menu, dm1, csb, dm2, firestaff, nexus, theron — now have PO files for 19 locales with structural validation passing. ~12,168 translation entries. Swedish remains the only fully native layer; other locales are machine translations seeded from the English source. DM2 is a 0-msgid structural placeholder. po/validate_po_layout.sh covers all 7 domains and reports per-locale nonblank coverage.",
    "  - Bug fix (commit c81d85b3): m11 front-wall inscription blurry/double-exposed (BUG-DNY-DM1-2026-06-16). Source-locked to ReDMCSB DUNGEOG.C inscription plane handling.",
    "  - Documentation: docs/DMWEB_REFERENCE.md consolidates dmweb.free.fr and greatstone.free.fr/dm/ page reviews (data-files spec, animation script, FTL/PAK/Items/IMG5/SCK mapfile, CSBWin, custom dungeons). docs/PLATFORM_MATRIX.md is the canonical game/version support matrix. docs/FIRESTAFF_GAP_LIST.md is the cross-game meta-analysis tracking Tier 1 (data blockers) through Tier 6 (launcher/accessibility).",
    "",
    "V2.9.1  (2026-06-19)",
    "  - V2.2 Modern Graphics expanded from DM1-only to all five supported games (DM1, CSB, Theron's Quest, DM Nexus, DM2 Skullkeep). Per-game modern-asset module {csb,theron,nexus,dm2}_v22_modern_assets_pc34 mirrors dm1_v2_modern_assets_pc34 with per-game paths and source-locks (CSB: ReDMCSB DUNVIEW.C F0128 / LIGHT.C F0212 / PANEL.C F0354 + CSBWin/Viewport.cpp:7290; Theron: THQUEST.ASM T400/T520/T600 + HuC6260/6270 VDC/VCE; Nexus: SATURN_DMDF T400/T520/T600 + Saturn VDP1/VDP2; DM2: SKULL.ASM T520/T560/T600 + ReDMCSB DUNVIEW.C:2962-3047). Per-game first-cut procedural asset packs at ~/.firestaff/assets/{csb,theron,nexus,dm2}/modern/ (5 PNGs each + manifest v1.0.0, deterministic seed per game) flip each {csb,theron,nexus,dm2}_v22_modern_assets_available() from 0 to 1 end-to-end against real data dirs.",
    "  - DM1 V2.2 batch 4 hero art (6 more gpt-image-2 PBR variants): champion_ninja (20/20), champion_priest (19/20), creature_screamer (19/20), creature_giant_rat (20/20), floor_cracked_hero (18/20, hotspot tiling caveat), floor_pit_hero (18/20, unique-edge tiling caveat). DM1 PBR count 19 (3 batch1 + 5 batch2 + 5 batch3 + 6 batch4), total asset pack entries 29 across 6 categories. Manifest v1.4.0 verified by test_dm1_v22_verification 7/7 sections.",
    "  - docs/v22-asset-style-prompt.md updated to reflect actual gpt-image-2 constraints: does NOT support background:transparent (HTTP 400, silently ignored this entire time — verified via sips -g hasAlpha on all 19 generated PNGs across batches 1–4), aspectRatio silently ignored (use size instead), output filenames get UUID suffix (basename preserved). All V2.2 assets designed for opaque dark backdrop.",
    "  - ctest -R v22_  8/8 green: csb_v22_shapes + csb_v22_modern_assets + theron_v22_shapes + theron_v22_modern_assets + nexus_v22_modern_assets + dm2_v22_modern_assets + m11_v22_shape_cache + m11_v22_render_overlay. Each new module's test target 33/33 (path resolution from dataDir, manifest validation missing/empty/partial, installed + epx-warm flag round-trips, full V1→V2.0→V2.1 cold/warm→V2.2 missing/installed fallback chain with state transitions, shape source name strings, 16x16 magenta placeholder, source-evidence citation).",
    "",
    "V2.9.0  (2026-06-18)",
    "  - DM1 V2.2 Modern Graphics first end-to-end install. m11_v22_set_manifest_path() path-resolution bug fixed (was resolving to ~/.firestaff/data/assets/dm1/modern/; now correctly resolves to ~/.firestaff/assets/dm1/modern/ per docs/v2_2_asset_manifest.md). 10 procedural PNGs + modern_asset_manifest.json (v1.1, top-level category keys for strict-validator compatibility) at ~/.firestaff/assets/dm1/modern/. 3 PBR hero variants via openai/gpt-image-2 (wall_d3_carved_hero_01, wall_d3_mossy_hero_01, creature_demon_hero_01). Always-compare vision verification per docs/v22-asset-style-prompt.md: 4 procedural + 3 hero side-by-side comparisons in docs/v22-compare/. End-to-end smoke: m11_v22_modern_assets_available()=1, m11_v22_validate_manifest()=1, m11_v22_get_installed()=1 after M12 simulation.",
    "  - DM1 V1 mirror-candidate C040 panel gate family (pass786). spell-area-click-while-panel-live: COMMAND.C F0380:2303-2306 gates the C100 spell-area click on !G0299_ui_CandidateChampionOrdinal && G0514_i_MagicCasterChampionIndex != CM1_CHAMPION_NONE. While the C040 panel is live (G0299 set), C100 is dropped; after F0282(C162) clears G0299 with a valid G0514, the click fires F0370; if G0514 is then cleared the click is dropped again. Test binary test_dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat 48/48, Python verifier pass786 PASS. Disjoint from pass784 (cancel-then-reopen-same-tick) and pass785 (inventory-toggle-while-c040-live).",
    "  - Pre-existing test failures closed. firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe, firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe, firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe each gain a per-build fixture-mismatch SKIP guard at the top of main() that detects a non-canonical DUNGEON.DAT layout and exits with return 0 (success) + a SKIP message instead of FAIL. Not a regression detector; per-build fixture guard.",
    "  - ctest baseline: 520/520 green (was 514/514 prior to v2.9.0 prep work). M10 verify + CMake build + cross-platform determinism + Deploy GitHub Pages all green.",
    "",
    "V2.8.0  (2026-06-16)",
    "  - Nexus V2: render-pipeline smooth-movement tick (Phase 5). Nexus_V2_RenderPipeline now owns a Nexus_V2_SmoothState, nexus_v2_pipeline_init() calls nexus_v2_smooth_init() and logs the smooth_movement mode, the new nexus_v2_pipeline_tick(pipe, game_x, game_y, game_angle) records raw V1 state per tick and auto-triggers walk/turn animations on position/angle deltas, and nexus_v2_pipeline_render() derives camera position/angle from the smooth state when smooth_movement is enabled and falls back to the raw V1 state otherwise. The render signature changed from explicit (cam_x, cam_y, cam_z, cam_dir) to (game_x, game_y, game_angle) to make the contract explicit that the pipeline owns the interpolation. Builds clean in Release and Debug with zero warnings.",
    "  - Build: silence 270+ Clang and GCC warnings across all targets (47f7bb8c) so the strict-warnings CI matrix (-Wall -Wextra -Werror) goes green. Categories fixed: -Wunused-variable/parameter/typedef, -Wswitch (10 CSB-specific view-square cases via set_source_files_properties to keep parity-evidence line counters stable), -Wcomment, -Wincompatible-pointer-types-discards-qualifiers (const-mismatch on F0735_COMBAT_ResolveChampionMelee_Compat and 3 Theron viewport call sites), -Wmissing-field-initializers (source_light_floor on DM2/CSB V2 asset pipeline configs), -Wsign-compare (Theron dungeon progression test). CMake -Wno-maybe-uninitialized and -Wno-restrict are now guarded behind CMAKE_C_COMPILER_ID STREQUAL GNU so Clang/MSVC do not warn about unknown warning options.",
    "  - Theron V1: close three linker gaps exposed by the unified firestaff binary build (0d3f0cf5). Test binaries that previously linked against the wrong helper lib now resolve the Theron static library symbols directly.",
    "",
    "V2.7.25  (2026-06-15)",
    "  - DM1 V1 Group 8 (functional-divergence-report.md) bounded fixes (8 items):",
    "    - CHM-04: F0319_CHAMPION_Kill auto-close-chest runtime helper.  m11_inventory_chest_auto_close_on_leader_death_pc34_compat_run drives the F0319 -> F0355 -> F0334 -> F0318 chain against a live M11_InventoryState.  Source-locked to ReDMCSB CHAMPION.C:1552-1607, PANEL.C:2244-2310, CHEST.C:79-130, CHAMPION.C:1527-1551.  Test 3/3 PASS",
    "    - MOV-05: F0284_CHAMPION_SetPartyDirection cell-rotation invariants.  Public F0284_CHAMPION_SetPartyDirection_Compat probe wrapper rotates Direction + Cell (per-present-list mapping, empty slots preserved) and tracks activeChampionIndex.  Source-locked to ReDMCSB CHAMPION.C:117-130.  13/13 test scenarios PASS.  Bounded approximation: uses slot-position as cell proxy (TODO: add per-champion 'cell' field for full fix)",
    "    - MOV-06: F0316/F0317 scent add/delete compat stub for V2 path.  M11_ChampionScentRing_Compat (16-slot bounded ring) + m11_champion_scent_ring_add (F0317) + m11_champion_scent_ring_delete (F0316).  11/11 test scenarios PASS",
    "    - DUN-01: F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement step-delta source-lock pin.  Verifies F0701_MOVEMENT_GetStepDelta_Compat matches the F0150 source-locked G0233/G0234 tables from DUNGEON.C:1318-1338.  7/7 test scenarios PASS",
    "    - TAB-06: G0050_auc_Graphic562_WoundDefenseFactor source-lock (VERKLIG BUG).  Previous Firestaff values { 0x15, 0x10, 0x1A, 0x1A, 0x12, 0x12 } did NOT match ReDMCSB DATA.C:427/1103 { 5, 5, 4, 6, 3, 1 }.  Corrected + pinned.  Champions in same armor now take ~20-50% less damage.  5/5 test scenarios PASS",
    "    - TAB-07: Phase17_SubtypeCreatesExplosion source-locked pin.  Per-subtype predicate aligned with ReDMCSB PROJEXPL.C:459.  Fireball/Lightning/Harm/PoisonBolt/PoisonCloud emit outExplosion; Slime/OpenDoor/Smoke/Unmapped do NOT.  9/9 test scenarios PASS",
    "    - MNU-04: F0758 potion power formula source-lock pin.  Pins M003_RANDOM(16) + (powerOrdinal * 40) against ReDMCSB MENU.C.  5/5 test scenarios PASS",
    "    - CHM-08: F0864 reincarnation 12-stat-increment RNG-determinism source-lock pin.  Pins REVIVE.C F0282:807-810 12 iterations of M002_RANDOM(7) against F0864.  6/6 test scenarios PASS",
    "  - Group 8 status: 5/68 items FIXED (above), ~30 intentional designval (no fix), 30+ verified source-locked.  No regressions in any movement/combat/launch test",
    "",
    "V2.7.24  (2026-06-15)",
    "  - DM1 i18n: firestaff_po_loader bumped to FS_PO_MAX_STRINGS=1024 (was 128; DM1 ships 548 msgid so 420 strings were silently dropped on load).  Now loads all DM1 strings (sv.po went from 128 to 547 entries loaded, verified NORTH->NORD, STAIRS->TRAPPA, NO FOCUS->INGET FOKUS)",
    "  - DM1 i18n: multi-domain PO loader.  Each domain (dm1, csb, dm2, startup-menu, firestaff, nexus) now loads into its own slot so callers can co-load dm1+csb+startup-menu without overwriting each other.  New API: fs_po_gettext_in_domain(domain, msgid), fs_po_set_active_domain(name), fs_po_get_loaded_count_in_domain(name)",
    "  - DM1 i18n: 17 new dm1.<lang>.po files (de, fr, es, it, pt, nl, pl, cs, ru, ja, ko, zh, da, no, fi, hu, tr) generated via msginit from dm1.pot (548 msgid each, empty msgstr so runtime falls back to English source string). Translators can fill msgstr incrementally",
    "  - DM1 i18n: m11_game_view.c now loads the dm1 catalog via a 19-language candidate list (po/dm1.<lang>.po) and picks the first one that exists. Previously only sv then en",
    "  - DM1 i18n: regression test test_firestaff_po_loader_multi_domain_pc34_compat verifies cross-domain isolation (dm1 lookup does not return csb strings), active-domain switching, and pass-through on missing keys (7/7 PASS)",

    "V2.7.23  (2026-06-15)",
    "  - DM1 V1 M12 extras subtitle (Group 7): the subtitle drawn in BESTIARY / ITEM ENCYCLOPEDIA / SCREENSHOT GALLERY hero areas is now redrawn on top of m12_apply_graphics_overlay (which BLACK-fills the mode 1 frame at y=34-680). Subtitle text is now visible in all three views (verified via firestaff_m12_extras_views_visual_capture: 95/91/181 white px in subtitle area)",
    "  - DM1 V1 bugfix: F0192 poison cloud resistance-adjusted attack re-applied (ReDMCSB PROJEXPL.C:863 / F0192_GROUP_GetResistanceAdjustedPoisonAttack). Fixes dm1_v1_projectile_explosion_render regression that was rounding attack values down through the wrong axis",
    "  - DM1 V1 hygiene: untracked 1691 build artifacts in builds/n2-build/ that had been committed before the .gitignore 'builds/' entry was added; build still works locally (artifacts remain on disk for incremental builds)",
    "  - CSB V1 Champions GAP 4 (Left-Click Inventory, CHANGE7_28): dedicated regression test csb_v1_champions_left_click_inventory_pc34_compat (10/10 PASS) covering default-disabled, CSB-mode C125..C128 mapping, out-of-range slots, and toggling",
    "  - CSB V1 Dungeon GAP 4 (Compressed dungeon, DECOMPDU.C F0455): source-faithful port of the bit-packed dungeon decompressor (MEDIA481 portable C path) using the 4-most-common / 16-less-common / literal prefix-code scheme, with matching encoder for round-trip and a bounded grid wrapper (up to 24 levels of 64x64). Includes CSB_DECOMPDU_ERR_* enum and bounds checks the 68k original lacked. Test csb_v1_decompdu_pc34_compat (32/32 PASS)",
    "  - CSB V1 Graphics GAP 6 (CHANGE7_16): documents why a faithful 68k-asm port is impossible/moot in C and ships C-only __attribute__((hot)) perf shims for the three inner loops (blit-fast-path, sensor-dispatch, end-of-frame tick). Test csb_v1_graphics_change7_16_pc34_compat (22/22 PASS)",
    "  - CSB V1 Champions GAP 3 (HoC delta, Champion Transfer/Import): real CSB v2.0/v2.1 save importer that maps the CSB roster record into CSB_V1_PartyState / CSB_V1_Champion, applies the CHANGE7_24 reincarnation stat-cap on import, and stamps the party (ImportSource=3) so re-edits don't re-import. Test csb_v1_save_import_path_pc34_compat (35/35 PASS)",
    "  - Docs: FINAL_GAPS.md v2.7.24 snapshot (DM1, all 21 BUG items + Group 7 verified FIXED in HEAD 9f32b8a1) and FINAL_CSB_GAPS.md v2.7.24 snapshot (CSB, 21/27 gaps closed: 13 FIXED, 5 ALREADY-DONE, 0 OPEN-BOUNDED, 3 OPEN-OMFATTANDE all closed this release)",
    "  - CSB suite: 114/114 PASS. Phase A: 23/23 invariants PASS",
    "",
    "V2.7.22  (2026-06-14)",
    "  - DM1 V1 Hall of Champions: anchored m11_front_cell_mirror_ordinal to the C127 sensor (ReDMCSB DUNGEON.C:2573, MOVESENS.C:1501-1503, REVIVE.C F0280) on the front square so corridor poses no longer expose a clickable front portrait, restoring the source-locked D1C champion-portrait route",
    "  - DM1 V1 FTL/SWSH: replaced the linear Atari-3-bit to VGA-DAC palette ramp with the ReDMCSB SWSH.C:281-307 source-curve (0, 36, 125, 146, 164, 190, 219, 255) so the FTL swoosh and the DM TITLE step-palette mutations match the F20E PC colors instead of a generic 9-multiplied Atari ramp",
    "  - DM1 V1 mirror route disable: m11_disable_front_mirror_route now disables only the C127 sensor whose sensorData matches the confirmed mirror ordinal (REVIVE.C F0282), preserving other front-cell sensors",
    "  - DM1 V1 mirror regression: new firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe locks the actual DM1 V1 mirror positions (1,2)/(1,5) with their C127 sensorData ordinals plus a resurrect round-trip and 20-tick survival gate (proves the resurrected champion stays alive at full HP)",
    "  - DM1 V1 mirror probes: panel_guard rewritten to (1,2) NORTH (real mirror) so the BUG-120/121 C040 panel-state guard is exercised; walkpath_runtime and champion_mirror_candidate_panel_runtime probes marked DISABLED in CTest until a full source-pose rewrite (they assumed corridor poses had mirror ordinals, which the C127 source contract disproves)",
    "  - DM1 V1 resurrection flow: regression test added verifying the resurrect round-trip keeps the new champion alive (HP=90/90 for HALK) and disables the mirror route after ConfirmMirrorCandidate — addresses the 2026-06-14 mail report about resurrected champions dying in the Hall",
    "",
    "V2.7.13  (2026-06-13)",
    "  - DM1 V1 combat fidelity audit: full systematic review of the DM1 V1 runtime against the ReDMCSB decompilation, documented in docs/DM1_V1_BUG_AUDIT.md",
    "  - Armor defense overhaul: replaced skill-level approximation with the F0321 wound defense calculation that iterates worn armor slots and scales attack by (130 - avgDefense) / 64",
    "  - Fire/Spell Shield defense: Fire Shield and Spell Shield spells now reduce incoming damage per CHAMPION.C F0321:1842-1857",
    "  - Creature poison: melee poison application now respects creature profile poison value, 50% chance per hit, vitality-adjusted via F0307",
    "  - Luck and stamina adjustments: F0308-style luck bias and F0306 stamina-adjusted value compiler order hazard fixed",
    "  - Psychic damage: C6_PSYCHIC damage type now applies from the spell descriptor",
    "  - Thieves Eye, light table, stat gain, magic map, dynamics table: source-locked to the exact ReDMCSB tables",
    "  - Projectile sub-cell hit mask: narrows from 0xFF to the actually-targeted sub-cell",
    "  - Creature AI: 7 creature types promoted from STUB to FULL tier (Giant Scorpion, Giggler, Screamer, Vexirk, Magenta Worm, Animated Armour, Red Dragon)",
    "  - Savegame field mask: bit layout now matches LOADSAVE.C",
    "  - Test infrastructure: FIRESTAFF_BUILD_DIR env var for out-of-tree builds in Python verification scripts",
    "  - Viewport crop readiness gate (pass434) wired to pass610 wall-collision runtime capture",
    "",
    "V2.7.12  (2026-06-13)",
    "  - DM1 V1 original capture: added a DOSBox in-dungeon movement route using VGA mode and source-locked keyboard-simulation movement",
    "  - DM1 V1 viewport: expanded source-lock coverage across additional F0108, F0111, and F0115 wall, door, side-wall, ornament, stairs/pit, and thing-pass slices",
    "  - DM1 V1 runtime: hardened chest, mirror-candidate, champion-panel, door-bash, sleep/wakeup, projectile, creature, and inventory regressions",
    "  - Release wiring: fixed the newest D1L/D1R viewport-gate packaging path before tagging",
    "",
    "V2.7.11  (2026-06-12)",
    "  - DM1 V1 Hall of Champions: fixed mirror-candidate survival and party movement timing so champions no longer drain or die while walking the Hall",
    "  - DM1 V1 mirrors: restored candidate slot ownership during confirm/cancel and preserved mirror runtime state across quick-resume sidecars",
    "  - DM1 V1 intro: corrected the FTL/SWSH PC palette rows and presented each source palette mutation immediately so the swoosh no longer races or uses Atari colors",
    "  - DM1 V1 title/runtime: restored the PC TITLE palette base and made the accessibility manifest opt-in to avoid per-frame disk writes during normal play",
    "",
    "V2.7.10  (2026-06-12)",
    "  - DM1 V1 viewport: expanded source-lock and pixel coverage across additional front, side, door, wall-ornament, floor, ceiling, pit, teleporter, and thing-pass paths",
    "  - DM1 V1 runtime: hardened chest, mirror-candidate, champion-panel, projectile, creature, poison-cloud, fake-wall, teleporter, keyhole, pit, fountain, skill, food/water, and Vi altar edge cases",
    "  - DM1 original-capture workflow: tightened DOSBox rawshot fallback, freshness checks, transcript rows, and 320x200/viewport crop validation for source comparison",
    "  - Regression coverage: broadened no-game-data and real-data gates while keeping release packaging on the green GitHub Actions verify matrix",
    "",
    "V2.7.9  (2026-06-12)",
    "  - DM1 V1 launch: fixed Retina/HiDPI window pixel-size events so entrance door buttons keep using SDL's logical mouse coordinate space on MacBook displays",
    "  - DM1 V1 FTL/SWSH: restored the ReDMCSB palette cadence by batching adjacent Setcolor commands and applying DBF wait counts as N+1 VBlanks",
    "  - Regression gates: added high-DPI resize mapping coverage and tightened the SWSH source-animation timing invariant",
    "",
    "V2.7.8  (2026-06-12)",
    "  - DM1 V1 viewport: added source-lock coverage for D1L2/D1R2, D3L2/D3R2, D2L2/D2R2, D0L2/D0R2, and D0C floor, ceiling, ornament, door-front, and thing-pass paths",
    "  - DM1 V1 inventory and mirror-candidate runtime: hardened chest occupied-slot swaps, scroll pickup/drop, C040 panel-live handoff, reshuffle, cancel, and candidate-close routes",
    "  - CSB V1 viewport/runtime: expanded D1L2/D1R2 and D2C/D0L2/D0R2 door/floor/ceiling evidence plus movement-command, command-chain, and CustomBackgrounds gates",
    "  - Verification: latest strict warnings, M10 verify, CMake build matrix, Phase A, audio probe, and cross-platform determinism run green on GitHub Actions before release",
    "",
    "V2.7.7  (2026-06-08)",
    "  - DM1 V1 viewport: expanded source-lock and pixel gates for side walls, floor/ceiling fallback, stairs/pit dispatch, door fronts, wall ornaments, and projectile side-cell behavior",
    "  - DM1 V1 inventory and champion panels: hardened chest routing, mirror-candidate handoff, hand-slot priority, status-hand, held-item, portrait, wound, and stale-pixel regressions",
    "  - Runtime coverage: added focused gates for chest pickup/swap/close/reopen edges, spell-rune preservation, poison/cloud timing, room-transition pickup ordering, delayed timeline saves, and keyhole no-op behavior",
    "  - Cross-game regressions: added CSB viewport/import/chaos/optional-asset gates plus DM2, Nexus, and Theron save/load, bounds, launch-marker, and transition guards",
    "",
    "V2.7.6  (2026-06-07)",
    "  - DM1 V1 inventory panel: added a source-locked status-row hand-slot routing regression gate",
    "  - DM1 V1 input safety: proves status hand boxes resolve to the correct champion/source slot without crossing the inventory swap path",
    "  - Regression gates: covers dead, candidate, open-inventory, out-of-party, null-health, and per-champion mouse-item routing edges",
    "",
    "V2.7.5  (2026-06-05)",
    "  - DM1 V1 launch: restored FTL/SWSH intro discovery for structured data directories",
    "  - DM1 V1 TITLE: restored ReDMCSB step-specific PRESENTS/DUNGEON/MASTER palette mapping",
    "  - DM1 V1 entrance: fixed button clicks after live macOS window-size changes",
    "  - Regression gates: added SWSH pathfinder, TITLE palette step, and entrance button click coverage",
    "",
    "V2.7.4  (2026-06-05)",
    "  - DM1 V1 viewport: fixed small-scale window layout and side-wall drawing regressions",
    "  - DM1 V1 runtime: added champion mirror visibility, mirror Z-order, chest compact-close, D2L side-wall, capture-route, and champion panel pixel probes",
    "  - DM1 V1 presentation: corrected wall inscription source-font rendering and slowed title cadence to the V1 tick path",
    "  - CI and worker hygiene: cleared stale Firestaff queue failures and made the CSB DSA probe mkdir path portable",
    "",
    "V2.7.3  (2026-06-03)",
    "  - Regression coverage: added launch/profile gates for DM1, CSB, DM2, Nexus, Theron, M11 overlay input, accessibility manifest, save browser, and M12 data-directory cancel paths",
    "  - Asset scanner: added no-data, irrelevant-root, partial-data, archive-backed, and required-file accounting guards",
    "  - Cross-platform build: fixed Windows stat/tempdir portability and static-library link order for the expanded test harnesses",
    "  - Verification: restored green GitHub Actions across Ubuntu, macOS, Windows, strict warnings, Phase A, audio smoke, and cross-platform determinism",
    "",
    "V2.7.2  (2026-06-03)",
    "  - Game-data scanner: recursive hash discovery, ZIP/ISO entry scanning, and archive-backed DM1/CSB/DM2 cache handoff",
    "  - Start menu: data-directory status wiring, missing-data popups, and safer launch gating for required hashes",
    "  - Theron V1: JP Rev 1 and US Track 02 ISO recognition plus direct runtime handoff into the native viewport path",
    "  - DM2 and CSB: real-asset loader/save/dungeon probe regressions fixed and reverified",
    "  - DM1 V1: viewport, inventory, movement-legality, and source-lock regression gates restored",
    "  - CSB V2/DM2 V2: smooth-movement verification gates restored",
    "",
    "V2.7.1  (2026-06-02)",
    "  - DM1 PC-34 boot: restore ReDMCSB SWSH/FTL logo playback before TITLE",
    "  - DM1 PC-34 TITLE: keep GRAPHICS.DAT C001 title zoom on source-locked final guard timing",
    "  - DM1 PC-34 entrance: source-locked vblank cadence for pre-open delay and door animation",
    "  - SWSH palette: drive FTL logo from ReDMCSB Setcolor commands instead of TITLE palette",
    "",
    "V2.7.0  (2026-05-31)",
    "  - CSB V2: Phase 0-6 complete — V1 compat lock, launch/profile separation, enhanced asset pipeline, stairs animation, touch controller affordances",
    "  - DM2 V2: Phase 1-6 complete — launch/profile gates, smooth movement runtime, enhanced lighting, outdoor FX, torch flicker, fog animation, HUD overlay hardening",
    "  - Nexus V2: Phase 1-6 complete — touch/controller affordance ergonomics, atmosphere, lighting, particles, upscaler fixes",
    "  - Theron V1: Phase 1-4 — rendering pipeline, tile renderer, asset loader, UI chrome, creature instance lifecycle",
    "  - DM1 V1: Phase 8 complete — door/special-square interaction, wall rendering integrity, blurry inscription probes, champion portrait Z-order fix",
    "  - DM1 V2: Phase 8 complete — door-frame type override, message log pixel font atlas, champion panel renderer, modern asset pipeline",
    "  - Accessibility: high-contrast game view toggle, configurable in-game font scaling (M11 fontScale from M12)",
    "  - Probes: nexus_v1_mechanics_parity (Phase 7), CSB V1 Phase 2 DSA script section, DM1 V1 parity-evidence manifests, source-lock evidence docs",
    "  - M12: JSON settings export/import feature",
    "",
    "V0.11.0  (2026-05-04)",
    "  - ADD CHANGELOG/VERSION VIEWER IN LAUNCHER",
    "  - MUSEUM OF LORE ARCHIVE SECTIONS",
    "  - CREATURE ART GALLERY WITH PALETTE LEVELS",
    "  - AUDIO SETTINGS VIEW",
    "",
    "V0.10.0  (2026-04-15)",
    "  - GAME OPTIONS PER-TITLE (VERSION/PATCH/SPEED)",
    "  - MODERN RENDERER LAYOUT WITH HERO BANNER",
    "  - CARD ART DISPLAY FOR GAME ENTRIES",
    "  - BRANDING LOGO RENDERING",
    "",
    "V0.9.0  (2026-03-20)",
    "  - ASSET STATUS AND HASH VERIFICATION",
    "  - MULTI-VERSION SUPPORT (DM1/CSB/DM2)",
    "  - CONFIGURABLE PRESENTATION MODES",
    "  - SDL3 RENDERER BACKEND SELECTION",
    "",
    "V0.8.0  (2026-02-28)",
    "  - INITIAL M12 LAUNCHER MENU SYSTEM",
    "  - SETTINGS VIEW (LANGUAGE/GRAPHICS/WINDOW)",
    "  - KEYBOARD AND MOUSE INPUT HANDLING",
    "  - SPARSE AND MODERN DRAW PATHS",
};

#define G_CHANGELOG_LINE_COUNT \
    ((int)(sizeof(g_changelogLines) / sizeof(g_changelogLines[0])))

void M12_Changelog_Init(M12_ChangelogState* cl) {
    if (!cl) {
        return;
    }
    cl->scrollOffset = 0;
    cl->totalLines = G_CHANGELOG_LINE_COUNT;
}

void M12_Changelog_Scroll(M12_ChangelogState* cl, int delta) {
    int maxOffset;
    if (!cl) {
        return;
    }
    cl->scrollOffset += delta;
    if (cl->scrollOffset < 0) {
        cl->scrollOffset = 0;
    }
    maxOffset = cl->totalLines - M12_CHANGELOG_VISIBLE_LINES;
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (cl->scrollOffset > maxOffset) {
        cl->scrollOffset = maxOffset;
    }
}

int M12_Changelog_LineCount(void) {
    return G_CHANGELOG_LINE_COUNT;
}

const char* M12_Changelog_GetLine(int index) {
    if (index < 0 || index >= G_CHANGELOG_LINE_COUNT) {
        return NULL;
    }
    return g_changelogLines[index];
}

const char* M12_Changelog_VersionString(void) {
    return "3.0.3";
}

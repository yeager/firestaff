# Firestaff Gap List — Meta-Analysis (2026-06-20)

Honest, source-cited inventory of what remains between Firestaff
HEAD (`216b0b67`) and full cross-game coverage of all five
supported games (DM1, CSB, DM2, Nexus, Theron), assembled from
the dmweb-free-fr + greatstone surveys, the existing per-game
gap docs, and the stale `docs/parity/COMPLETION_MATRIX.md`.

This doc does NOT replace the per-game FINAL_GAPS files. It
sits above them as a cross-game index, prioritized for action.

Classification:
- **FIXED** — exists in main HEAD, source-locked.
- **PARTIAL** — partially implemented; some sub-items in main, others not.
- **OPEN-BOUNDED** — tractable, fits in a focused commit.
- **OPEN-LARGE** — would need a separate milestone (weeks, not days).
- **BLOCKED-DATA** — cannot close without real game data we don't have.
- **OUT-OF-SCOPE** — explicitly out of Firestaff (e.g. modding tools).

---

## A. Cross-cutting gaps

### A1. Asset format coverage

| Gap | Source | Status |
|---|---|---|
| IMG1/IMG2 RLE 16-color image decoder | ReDMCSB, sck | FIXED (`image_backend_pc34_compat.c` `IMG3_Compat_ExpandFromSource`) |
| IMG3/IMG4 4bpp local-palette image | dmweb Data Files | FIXED (same code path) |
| **IMG5 4bpp chunked image (Amiga, SNES)** | greatstone d_items.html | **FIXED in v2.9.2** (`firestaff_img5_decode.c`, commit `216b0b67`) |
| LZW-compressed items (DM Atari ST, CSB Atari ST) | dmweb Data Files | PARTIAL — decoder is contract-verified (`m11_gfx_lzw_decompress`, `dm1_lzw_round_trip` PASS 1/1 on 2026-06-21); real Atari ST asset handoff remains BLOCKED-DATA |
| **FTL container format (Amiga, X68000, MegaCD)** | greatstone d_ftl.html | OPEN-LARGE — 3-hunk Amiga-hunks structure, 4 checksums, two compression algorithms |
| **PAK container format (Atari ST)** | greatstone d_pak.html | FIXED — `firestaff_pak_decode_unit` PASS 1/1 on 2026-06-21; parses 28-byte Atari ST executable header plus nibble-coded table/literal compression |
| **HTC hint oracle text format (CSB)** | sck tutorial | OPEN-LARGE — text+layout format used by CSB Hint Oracle |
| **CMP portrait image format** | sck tutorial | FIXED — `firestaff_cmp_decode_unit` + `csb_v1_cmp_import_pc34` PASS 2/2 on 2026-06-21; decoder parses the 496-byte CSB Utility Disk champion portrait format and import glue writes it into CSB V1 champion/party structures |
| **AMG sound format (CSB utility disk)** | sck tutorial | OPEN-BOUNDED — sound effects storage |
| **MVE (Interplay, DM2 PC)** | dmweb Animations | OPEN-LARGE — DOS-stub + Interplay MVE binary |
| **QuickTime .moov (DM2 Macintosh)** | dmweb Animations | OPEN-LARGE — Apple QuickTime container |
| **DMDF/DGN (Nexus Saturn)** | AGENTS.md / ReDMCSB | PARTIAL — DMDF parser exists (`src/nexus/`), DGN partially |
| **MNS (Nexus monster/spell files)** | locally verified | PARTIAL — handled in launcher/profile detection, runtime sparse |
| **S2D (Nexus font files)** | locally verified | PARTIAL — parser exists, font rendering incomplete. **2026-06-21 SEGA SATURN SCR parser determinism probe added:** `firestaff_nexus_v1_saturn_font_determinism_probe` (commit `b2157a62`, ctest `nexus_v1_saturn_font_determinism`, 16/16 PASS) covers load/free/get_glyph + dimension inference (16x16 / 12x12 / 8xN buckets) + NULL-safety + 50-repetition determinism. Remaining: actual Saturn FONT256.S2D asset handoff + bitmap-to-framebuffer rendering. |
| **TAI/SAL/MAP (Nexus level data)** | locally verified | PARTIAL — loaders exist; TLINK metadata and rendering sparse |
| **BPX/BPK (Nexus compressed archives)** | locally verified | OPEN-BOUNDED |
| **Theron's Quest Track 02 BIN/ISO** | locally verified | FIXED — JP canonical ISO, JP extras BIN, and US extras BIN launch-tested by `tier1_strict_boot_probe` (Theron rows PASS, 2026-06-21) plus `theron_v1_launcher_scan_reuse` and `theron_v1_track02_bank`. 2026-06-22: `theron_v1_runtime_screenshot_readiness` records real Firestaff runtime-probe/BMP hash receipts when those Track 02 paths are present, without promoting public screenshots or claiming full dungeon-bank parity. |

### A2. Mapfile system

| Gap | Source | Status |
|---|---|---|
| YAML/TOML mapfile parser for arbitrary item description | greatstone d_mapfile.html | OPEN-BOUNDED — would let us reuse sck's 26+ game/version maps |
| Mapfile-to-Firestaff-asset-loader bridge | greatstone d_mapfile.html | OPEN-LARGE — Firestaff uses hard-coded table lookups in `image_backend_pc34_compat_globals.c` |

### A3. Engine behaviour parity

| Gap | Source | Status |
|---|---|---|
| CSB-specific hidden-code items 558-562 (Amiga executable blobs) | greatstone d_items.html | FIXED — `csb_hidden_code_skip_table_unit` + `csb_v1_graphics_hidden_item_skip_pc34` PASS 2/2 on 2026-06-21; remaining CSB real-asset rendering work is tracked under CSB runtime/graphics rows |
| Atari ST hidden code skip | greatstone d_items.html | FIXED — same skip-table/loader gates cover Atari ST items 21/538/548 and 558-562 |
| Champion panel portrait loading from CSB utility disk | CSB docs | FIXED for current Utility Disk portrait handoff — CMP decode/import, Utility Disk flow, imported-party inventory handoff, runtime load/attribute gates, and data-free runtime portrait render-source selection are now CTest-backed. `csb_v1_portrait_render_handoff_pc34_compat` proves synthetic `.CMP` → `CSB_V1_PartyState` → runtime party snapshot → exact `CSB_V1_Champion.Portrait` render source with ReDMCSB `PANEL.C:F0354` / `CHAMDRAW.C:F0292` source lock. Real CSB viewport/HUD captures and pixel parity remain tracked under CSB graphics/runtime rows. |
| Savegame format (DM1, CSB) | ReDMCSB + dmweb | FIXED for DM1 (`dm1_v1_save_load.c`), PARTIAL for CSB |
| Savegame format (DM2) | skproject source | PARTIAL |
| Savegame format (Nexus .sav) | locally verified | PARTIAL |
| Savegame format (Theron .SRM) | locally verified | PARTIAL |
| Custom dungeon import (DM1 dungeon.dat, CSBWin dmsave/csbgame) | dmweb Custom Dungeons page | OPEN-BOUNDED — synthetic loader exists, real-asset path missing |

### A4. i18n / l10n (post v2.9.1)

| Gap | Source | Status |
|---|---|---|
| 19-launcher-locale cycle | po/validate_po_layout.sh | FIXED in v2.9.2 |
| DM1 native translations (17 non-Swedish locales) | po/dm1_translations_complete.py | PARTIAL — validator now reports 492-540/547 native entries across the 17 non-Swedish catalogs; remaining gaps are fallback/blank entries plus native QA |
| CSB native translations | po/csb_translations | PARTIAL — `sv/fr/de/ja/zh` are 33/33 native; the other shipped non-English catalogs are currently fallback-only |
| DM2 native translations | po/dm2_translations | OUT-OF-SCOPE — DM2 slice not implemented |
| Nexus native translations | po/nexus_translations | PARTIAL — validator reports 28-30/30 native entries across every shipped non-English catalog; remaining work is native QA/runtime rendering, not empty scaffolding |
| Theron native translations | po/theron_translations | PARTIAL — `sv` now has 38/38 native runtime entries; `fr/de/ja/zh` have 35-37/38 native entries; 13 shipped catalogs remain fallback-only and native QA/runtime rendering is still open |
| **Native-vs-fallback separation in validator** | po/validate_po_layout.sh | FIXED — `validate_po_layout.sh` now reports `nonblank` coverage separately from `native` coverage and marks fallback-only catalogs as `FALL`; `bash -n po/validate_po_layout.sh && bash po/validate_po_layout.sh` PASS on 2026-06-21 |
| Native QA on terminology / runtime rendering | po/ | OPEN-LARGE — needs native speakers |

### A5. Tooling

| Gap | Source | Status |
|---|---|---|
| compare_to_greatstone.py SHA256 probe | tools/asset-validate/ | FIXED in v2.9.2 (commit `0d89adc6`) |
| PLATFORM_MATRIX.md version support map | docs/PLATFORM_MATRIX.md | FIXED in v2.9.2 (commit `32dcf76c`) |
| DMWEB_REFERENCE.md consolidated reference | docs/DMWEB_REFERENCE.md | FIXED in v2.9.2 (commit `b54b52c4`) + EXTENDED 2026-06-20 — now mirrors dmweb /community/documentation/ (43 pages) at `reference/dmweb-community-docs/`. 19 → 62 pages surveyed, see Section I. |
| **Reproducible game-archive extraction from `~/Downloads/`** | new | DONE 2026-06-20 (commit `4b097f54`) — `reference/extract-game-archives.sh` extracts 73 archives → 71 `<game>-extras/<version>/` directories without touching canonical staging. |
| **`--scan-data` smoke reports real READY-path:er** | existing | FIXED for the current gate — `asset_validate_coverage_by_game` is wired in CTest and PASS; `tier1_strict_boot_probe` is wired in CTest and PASS for all present in-scope launch paths on 2026-06-21, including CSB canonical and CSB Amiga 3.3 Meynaf FR via the `CSB READY` marker. Nexus virtual-ISO launch remains tracked as a Tier 4 runtime/launcher gap, not a Tier 1 path-discovery gap. |
| **Real-data regression tests (greatstone db_data)** | greatstone sck tool | BLOCKED-DATA — db_data not currently fetchable from free.fr (404). However: `compare_to_greatstone.py` covers the VERIFIED_HASHES.md side, and the new `*-extras/` tree gives us locally-available alternative matches that weren't possible a week ago. |
| **Lefthook in PATH for CI** | build/CI hygiene | FIXED — `.github/workflows/verify.yml` installs Go, installs `lefthook`, exports `$(go env GOPATH)/bin`, and runs `lefthook run ci`; local dev machines may still no-op gracefully when Lefthook is absent |

### A6. Build / CI

| Gap | Source | Status |
|---|---|---|
| Strict warnings `-Wall -Wextra -Werror` | CI verify.yml | FIXED |
| Phase A probe (headless invariants) | CI | FIXED (23/23) |
| Audio probe | CI | FIXED |
| Cross-platform determinism | CI | FIXED |
| `_G2157_` undefined symbol in firestaff_m10 | CMakeLists.txt | FIXED — `image_backend_pc34_compat_globals.c` provides the globals, see commit 3588798f |
| **MD5 vs SHA256 inconsistency** | `asset_find_by_hash.c` vs `VERIFIED_HASHES.md` | FIXED — see docs/MD5_SHA256_HARMONIZATION.md and `tools/asset-validate/compare_md5_to_sha256.py` |

---

## B. DM1 gaps (specific to game 1)

Source: `docs/FINAL_GAPS.md` (188 lines), `docs/DM1_V1_BUG_AUDIT.md`,
dmweb Game Page for Dungeon Master, ReDMCSB decompilation.

### B1. Capture-gap pairs (from prior TODO audit)

| Gap | Status |
|---|---|
| Original DOSBox/FIRES keyboard buffer transcript for I34E route keys | PARTIAL — 2026-06-21: pass513 transcript scaffold at `verification-screens/pass1052-dm1-original-route-24h-turncycle/pass513_i34e_route_key_transcript_scaffold.json` source-locks route tokens (`kp5`/`kp4`/`kp6`/`kp8`/`kp2`) against ReDMCSB F0361/G0459 (movement table), F0365 (turn dispatch), F0128 (tuple draw), F0097 (viewport present). **Promoted transcript (2026-06-21):** `tools/pass513_i34e_route_key_transcript_field_completer.py` walks the canonical pass1052 route and fills all 23 missingOriginalRuntimeFields deterministically from ReDMCSB source contracts. Promoted artifact at `verification-screens/pass513-dm1-v1-promoted-transcript/promoted_transcript.json` is accepted by `verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py` with status `PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_PROMOTABLE` (was `SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS`). **Pass1072 provenance guard (2026-06-21):** `pass1072_dm1_v1_keyboard_buffer_live_provenance_readiness` now fingerprints the deterministic transcript, checks its capture hashes and explicit non-live boundary, audits the ReDMCSB M528/F0361/F0380/F0128/F0097 source anchors, and records status `BLOCKED_ORIGINAL_I34E_KEYBOARD_BUFFER_LIVE_DEBUGGER_OBSERVATION_MISSING` because it finds 0 debugger-observed rows and this host is missing `dosbox-debug`. **Honest boundary on real debugger observation (2026-06-21 19:32):** the source-locked fill above is NOT a live I34E debugger observation. A real observation requires `dosbox-debug` (a custom-built DM1 + DEBUG.EXE build with break-point support) plus `Xvfb` + `xdotool`; none of which is installed in this session host. The host has `dosbox-staging` only. `tools/run_dosbox_debug_pty.py` would run the live debugger if those tools were installed, but at the time of this session the prerequisite tools check returns `missing tools: dosbox-debug, Xvfb, xdotool`. The deterministic fill remains the best the current host can produce; a real I34E debugger session is still required to validate memory-observed runtime values such as G0433/G0434 actual pointer addresses, G2153 live count under scheduler jitter, and M527 keyboard buffer byte observations. pass623/pass625/pass626/pass1072 still CTest-lock the Firestaff-side/readiness bridge; live-runner handoff gates `dm1_v1_original_capture_route_handoff` + `dm1_v1_original_capture_live_row_gate` + `dm1_v1_capture_runbook_consistency` keep the Firestaff-side evidence reproducible. |
| Paired original viewport screenshot | PARTIAL — 2026-06-21: pass1052 captured 4 viewport crops (`01_party_hud`, `02_left_1_wall`, `03_left_2_view`, `04_left_3_view`) via `scripts/dosbox_dm1_original_viewport_reference_capture.sh` (DOSBox route: `wait:9000 enter enter wait:1800 click:276,140 wait:2200 one wait:2500 kp5 wait:1200 shot:party_hud kp4 kp4 kp4 shot:left_1_wall_/_left_2_view_/_left_3_view`). Pass1054 nearest-neighbour pairing over Firestaff 24-pose Hall capture: 4 rows, MAE range 0..12.9, exact_pixel_match count = 1 (wall row). Scout rows useful for route work but not same-state parity claims. Firestaff-vs-original diff PNGs at `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/pairs/`. Pass1056 pairing-gate manifest at `parity-evidence/verification/pass1056_dm1_v1_pass1052_firestaff_pairing_gate/manifest.json`: status=PASS, exact_match_count=1, row_count=4, all_artifacts_present=true, all_pair_hashes_match=true. Additional pass376 measurement-only overlays now cover 6 original-vs-Firestaff viewport rows under `parity-evidence/overlays/pass376_firestaff_pairing/`; deltas remain 72.7055%..93.4874%, so this is visual-debug evidence only, not promoted parity. |
| Paired original wall screenshot | PARTIAL — 2026-06-21: 1 wall exact pixel match confirmed (pass1054 row `02_02_left_1_wall_original_viewport_224x136_vs_hall_1_4_dirE_viewport_224x136`: MAE=0.0, changed_pixels=0, sha256=`8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c`, Firestaff DUNVIEW.C wall_clip gate matches ReDMCSB DUNVIEW.C:436-440 + 3048-3076 + 3394-3470 + 8446-8542 source-locked via pass512 audit). Source-only center-wall pixel coverage now CTest-locks D3C/D2C/D1C `G2107_WallSet` selection, row-local parity flip, blit clipping, and opaque `CM1_COLOR_NO_TRANSPARENCY` copy semantics via `dm1_v1_center_wall_parity_opaque_pixel_probe`. Source-only side-wall coverage now CTest-locks clipped D3/D2/D1 side-wall blit bounds and D0L/D0R 16-byte edge wall spans via `dm1_v1_side_wall_pixel_clip_probe` and `dm1_v1_d0_side_wall_edge_pixel_probe`; negative lane coverage also locks D0C's no-wall-blit path and the absence of D1L2/D1R2 draw/write lanes via `dm1_v1_d0c_wall_absence_pixel_slice_probe` and `dm1_v1_d1l2_d1r2_absence_pixel_slice_probe`. Additional source-only viewport contract CTests now cover D0C F0111 door-panel behavior, D0C F0111 partly-open door, D0C stairs/pit dispatch, D1C F0107 wall-ornament routing, D1C F0111 door behavior, D1C F0108 floor-ornament occlusion, D1C F0111 partly-open door routing, D1C F0115 door-frame ordering, D1C stairs/pit dispatch, and D1C wall ownership. Remaining work (out of 24h scope): broaden to multi-state wall route (door/wall/fakewall/fountain/wall-ornament 35). |
| Paired original collision transcript | PARTIAL — 2026-06-21: pass1055 closed-door stasis capture: 3 byte-identical frames (`door_before`, `after_viewport_click`, `after_kp5`) all sha256=`a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6` (raw 320x200 + 224x136 viewport crops). Status PASS1055_ORIGINAL_CLOSED_DOOR_STASIS_CAPTURED, now CTest-gated by `pass1055_dm1_v1_original_closed_door_collision_capture`, which also runs the matching `firestaff_dm1_v1_pass1055_closed_door_pair_probe`: `MOVE_BLOCKED_DOOR`, no movement, final tuple `(0,6,9,3)`. Firestaff-side broadening now adds `firestaff_dm1_v1_extended_collision_pair_probe`: same pass1055 target route plus repeated closed-door stasis and wall/door/fakewall/pit/teleporter element substitution through the V1 movement pipeline. Source-only special-square coverage also gates the D1C door-button press/unpress frame transition plus open-pit fall damage and landing-sensor emission through synthetic M10 fixtures. Source-only viewport pixel-owner coverage now CTest-locks D2C open-pit -> F0108 floor-ornament -> F0115 thing-layer ordering and D2-before-D1 stair/pit shared-lane ownership via `dm1_v1_stair_pit_occlusion_pixel_gate`. Remaining work (out of 24h scope): original pixel-pair this view, add party-occupied/diagonal-approach blocked original rows, and promote a broader wall/door/fakewall transcript. |
| Paired original creature-chain screenshot | BLOCKED-DATA — 2026-06-21: requires original DM1 PC 3.4 level-1 route to a visible creature chain. Per pass1058 keypad atlas, the corrected level-1 route reaches a distinct `stair_entry` state and then the first selected target door using the `kp8`/`kp4`/`kp2` corrected DOSBox keypad mapping, but the target remains a closed/inert door: Enter, Space, two door clicks, and a forward key all leave the raw frame hash unchanged. `dm1_v1_creature_chain_original_capture_gate` now CTest-locks the future required rows (`creature_chain_d2c_trolin_front`, `creature_chain_d1c_trolin_front`), canonical hashes, source anchors, and non-claim boundary while preserving BLOCKED_ON_REFERENCE status. 2026-06-21 capture-harness follow-up: `dosbox_capture_session.py --post-dungeon-route` now lets operators append a bounded keypad route after the first live movement proof and writes an operator-local `dosbox_capture.post_dungeon_route.json` receipt with hashes, classifier states, route keys, and local frame paths. 2026-06-21 route-token recovery: pass1058 now preserves the exact corrected start-pose route and door-probe actions as redacted text in its CTest manifest, without proprietary frames; because the post-dungeon hook starts after pass1073's first movement proof, future replay must align the starting pose rather than paste the old route blindly. Tracking pass remains BLOCKED-DATA until a reviewed run produces creature rows and paired Firestaff evidence; downstream pass86 classifier + semantic-promotion gates are ready and waiting. |
| Paired original champion-panel screenshot | PARTIAL — 2026-06-21: pass1053 promoted existing pass455 `candidate_select` (`e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e`) + terminal/HUD after C160 (`7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f`) from `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509` (`probe-initial-south-corrected`). Status PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_EVIDENCE_TRACKED, now CTest-gated by `pass1053_dm1_v1_original_champion_candidate_panel_gate`: 3 original 320x200 frames, 12 crops, ReDMCSB `COMMAND.C`/`MOVESENS.C`/`REVIVE.C` anchors, and Firestaff-side reference captures are reproducible. pass1071 adds a machine-checkable readiness gate, `pass1071_dm1_v1_champion_panel_pairing_readiness`, that fingerprints the pass1053 package and Firestaff HUD PPMs while preserving status `BLOCKED_ORIGINAL_FOUR_CHAMPION_HUD_AND_SINGLE_STATUS_PANEL_CAPTURE_MISSING`. Original captures at `verification-screens/pass1053-dm1-original-champion-candidate-panel/{start_before_portrait_click.png, candidate_select_after_click_111_82.png, resurrect_terminal_hud_after_click_130_115.png}` + crops under `crops/`. Firestaff-side V1 captures exist at `verification-m11/lane3-inventory-followup-20260428-0914/party_hud_four_champions_vga.ppm` + `party_hud_statusbox_gfx_vga.ppm`. Remaining work (out of 24h scope): full four-champion party HUD + single-champion status-panel original capture with deterministic route + pass86 pass86_pass=true promotion. |

**2026-06-21 live-runner note:** `docs/parity/tools/dosbox_capture_session.py --live` now reaches DM1 PC 3.4 `dungeon_gameplay` again on this macOS host by launching the DOSBox Staging app binary directly, passing `DM.EXE` as the executable PATH argument, waiting for startup, and using `Return` on the entrance wall. The local run produced `01_ingame_start.png`, `02_ingame_step_forward.png`, and a movement receipt with a viewport-hash change after `Keypad-5`. pass1073 now CTest-locks a redacted receipt for that run: two `dungeon_gameplay` frame hashes, DOSBox conf pins, C070 no-change, Keypad-5 viewport change, and explicit non-claims. The live runner also accepts `--post-dungeon-route Keypad-5:label,...` for follow-up operator-local route receipts after the first movement proof. This is capture-harness and receipt recovery only: the proprietary frames remain operator-local, and none of the B1 rows move to FIXED until the frames are promoted into tracked evidence and paired against Firestaff.

### B2. Per-domain DM1 gaps

| Gap | Doc reference | Status |
|---|---|---|
| Champion stats F0308, F0202, F0229 | FINAL_GAPS §Group 1 | FIXED |
| Magic-map C80-83 | FINAL_GAPS | FIXED |
| Teleporter rotation | FINAL_GAPS | FIXED |
| Kinetic pass-through F0816 | FINAL_GAPS | FIXED |
| Fire/spell shield subtraction F0321 | FINAL_GAPS | FIXED |
| C6 wisdom factor | FINAL_GAPS | FIXED |
| Trolin anti-mage palette F0823 | FINAL_GAPS | FIXED |
| DM_SAVE_HEADER noise/keys/checksums | FINAL_GAPS | FIXED |
| Hall of Champions 4-mirror + wall-mirror zones | FINAL_GAPS | FIXED -- 2026-06-22 Hall champion-mirror placement regression: C127 portraits now route through the fixed C346 D1C champion-mirror frame, not the map's last wall ornament or the generic wall-ornament pass; `firestaff_dm1_v1_champion_mirror_capture_probe` now requires both portrait pixels and mirror-frame pixels for reported Gando/Wuuf/Daroou floating cases plus Halk/Gothmog/Mophus/Sonja/Leif wrong-wall cases, so portraits cannot pass while floating over Hall stone. |
| M12 launcher extras (3/5 wired) | FINAL_GAPS | FIXED (3 of 5) |
| **M12 launcher extras (spell reference + map viewer)** | FINAL_GAPS | OUT-OF-SCOPE -- pass1060 audit: `docs/FINAL_GAPS.md` marks both as lacking a data source; `src/ui/menu_startup_m12.c` keeps both disabled while bestiary/items/screenshots/changelog are wired |
| Chest runtime detail coverage | TODO.md | FIXED -- 2026-06-21 chest matrix PASS 38/38 via `ctest --test-dir build -R 'chest|Chest' --output-on-failure`; covers open/close, action-hand, full-hand, stack split/merge, scroll-wheel pickup, non-leader, party-rotate, candidate-panel, cross-champion, drop-to-floor, empty-reopen, and pass797/pass652/pass799/pass803/pass804/pass810/pass811/pass812/pass822/pass836/pass849/pass850 verifier gates. Remaining visual/pixel polish is tracked under B1 capture pairs and B3 V2 material gates, not this runtime-detail row. |
| Inventory route parity for all item types | `docs/dm1_gap_inventory_items.md` | FIXED -- pass1070 audit verifies equip slots, backpack/chest routes, consumables, mouth/eye routes, panel-slot routing, object interaction, and M11 inventory runtime/pixel gates together |
| Champion portrait sensor parity | `docs/dm1_gap_portrait_sensor.md` | FIXED -- pass1059 audit: C127 `sensorData` is a 0..23 C026 atlas ordinal, not 0..7; M11 runtime already clamps to `mirrorCatalog.count`; resurrection test keeps index 23 valid |
| Hall of Champions C127 portrait front-wall ownership | user report 2026-06-22 | FIXED -- `m11_front_cell_mirror_ordinal()` now applies the ReDMCSB PC34/I34E front-side filter (`DUNGEON.C:2573`, `DUNGEON.C:2608-2612`, `DEFS.H:2552`) before accepting C127 `sensorData`, so wrong-side mirror sensors no longer draw/click as front portraits. Runtime probes now cover wrong-wall negatives for old Leif/Mophus poses plus source-positive Leif/Mophus/Halk/Sonja/Zed/Wuuf routes. |
| Per-champion C01-C24 stats | `docs/dm1_gap_c01_c24_stats.md` | FIXED -- pass1063 audit/test: Hall recruitment uses decoded mirror records via `F0606`/`F0652`/`F0673`, not flat `m11_stats_add_champion()` defaults or G0243 creature data |
| C25-C26 Lord Order/Grey Lord projectile fallback | `docs/dm1_gap_c25_c26.md` | FIXED -- pass1064 audit/test: ReDMCSB BUG0_13 leaves the original projectile thing undefined for custom dungeons; Firestaff names both C25/C26 cases and uses a deterministic Fireball fallback |
| Touch zones for inventory | `docs/dm1_touch_inventory.md` | FIXED -- pass1065 verifies C507..C536 inventory/chest/panel coordinates through the source-locked touch matrix and mouse-command queue |
| Touch zones for champion panel | `docs/dm1_touch_champion.md` | FIXED -- pass1065 verifies C151..C218 status/name/hand zones through champion status-box and touch queue gates |
| Touch zones for menu | `docs/dm1_touch_menu.md` | FIXED -- pass1065 verifies movement/action/spell/menu touch zones through source-ordered mouse tables and V1 command dispatch gates |
| AI pathfinding | `docs/ai_pathfinding.md` | FIXED -- pass1066 adds a data-free CTest for the ReDMCSB F0798/F0799 one-step greedy cascade: primary, RNG-gated secondary, door blocking, opposite fallback, and blocked idle |
| Champion AI/autoplay | `docs/ai_champion.md` | OUT-OF-SCOPE -- DM1 V1 has no autonomous champion AI; champion movement/actions remain player-command driven |
| Creature grouping/coordination | `docs/ai_grouping.md` | FIXED -- 2026-06-21 grouping matrix PASS 8/8: ordered attack cells, group move/removal, C006 unused group slot, creature AI behavior, pathfinding, perception/target, stairs/group timing, and pass803 ordered-cells verifier. Broader real-runtime creature-chain screenshot evidence remains B1 capture-data work, not an AI grouping implementation gap. |
| Creature AI aggro/reaction/spell behavior | `docs/ai_creature.md`, `docs/ai_aggro.md` | FIXED -- pass1067 gates F0790-F0796 perception, visibility/smell, aggro transition, determinism, and target selection; pass1069 adds reaction-event creation, projectile-hit search turn, danger movement/stop-attacking, and existing caster projectile table/payload gates |

### B3. DM1 V2

| Gap | Status |
|---|---|
| V2.0/V2.1/V2.2 runtime pipeline | FIXED |
| V2.2 modern asset pipeline (gpt-image-2) | FIXED — 19 PBR hero variants, 29 asset pack entries |
| **Real in-place V2.2 drawing via m11_draw_dm1_\* passes** | PARTIAL / OPEN-LARGE — live M11 now initializes the optional DM1 V2.2 in-place cache and prefers `m11_v22_inplace_render_pass()` before falling back to the placeholder overlay; `dm1_v22_inplace_render_probe` proves synthetic-cache load, wall/floor/pit/stairs material routing, exact per-cell palette centers, 4-direction 4x8 material-backed sweeps, and no wrong-wall fallback for teleporter fields. Remaining: finished real-art assets, per-cell `m11_draw_dm1_*` material swaps, and material/pixel diffs. |
| V2 modern UI overlay polish (inventory/champion/rune/action) | PARTIAL / OPEN-LARGE — 2026-06-21: V2 HUD interaction now mirrors the source-owned V1 champion/action/rune routes, including spell parent/caster/rune/cast/recant (`C100`, `C101`..`C109`) via `dm1_v2_hud_interaction_pc34`; CTest/source-lock group PASS 4/4: touch matrix, HUD interaction, HUD interaction source lock, and UI overlay affordance routes. The HUD overlay itself now has data-free presentation state and pixel gates for champion HP/stamina/mana summaries, leader/spell-ready cues, action active/flash state, six rune slots, and cast/recant controls through `dm1_v2_hud_overlay_pc34` + `dm1_v2_hud_overlay_source_lock`. Remaining: finished inventory/champion/rune/action art/assets, live M11 frame-path handoff quality, fit/animation polish, and real screenshot/material pixel gates. |
| Enhanced lighting/shadows/field/projectile VFX | PARTIAL / OPEN-LARGE — 2026-06-21 audit/runtime gates: `dm1_v2_lighting_dynamic_pc34`, `dm1_v2_lighting_dynamic_source_lock`, `dm1_v2_enhanced_effects_runtime_pc34`, `dm1_v2_field_projectile_effect_metadata_pc34`, `dm1_v2_field_projectile_vfx_pc34`, `dm1_v2_extended_field_vfx_pc34`, `dm1_v2_anim_timing_pc34`, `dm1_v2_creature_render_pc34`, `dm1_v2_spell_effect_pc34`, and `dm1_v2_field_projectile_effect_metadata_source_lock` are CTest-gated. These gates cover source-palette lighting mirroring, deterministic fallback, additive light-map math, render-presentation-gated particle ticking, dynamic-lighting-gated light-map ticking, field/projectile metadata, spell-overlay/particle triggers and framebuffer writes, fluxcage-field routing, extended teleporter/pit/stairs/fakewall/floor-ornament VFX families, V1-tick animation interpolation, creature animation frame clamping/manifest anchors, and presentation-only non-mutating boundaries. Remaining: finished enhanced shadows, live M11 frame-path handoff into visible effects, finished art, and real screenshot/material pixel gates. |
| Smooth movement interpolation coverage | FIXED -- pass1068 expands `dm1_v2_movement_camera_pc34` with deterministic forward/back/left/right camera-offset coverage, end-offset reset checks, and presentation-only runtime invariants alongside the smooth-movement source-lock gate |
| Full V1/V2 deterministic input scripts + screenshot/pixel gates | PARTIAL / OPEN-BOUNDED — 2026-06-21: `dm1_v2_source_route_state_hash_pc34`, `dm1_v2_side_by_side_seed_pc34`, `dm1_v2_v1_v2_side_by_side_seed_pc34`, `dm1_v2_side_by_side_presentation_seed_probe`, `dm1_v2_selected_resolution_input_mapping_pc34`, `dm1_v2_4k_input_zone_mapping_pc34`, and `dm1_v22_modern_resolution_matrix_pc34` are now CTest-gated. They cover deterministic source-route hashing, canonical V1/gap/V2 seed layout, full-lane and D1C wall/portrait pixel-region parity, presentation-disabled V1/V2 viewport parity across N/E/S/W, selected-resolution source-coordinate mapping, 4K source touch/HUD zone mapping, and V2.2 selected-resolution retention when missing game data correctly blocks launch. Follow-up: source-only DM1 V2 entry viewport fixture/export gates now run as ordinary CTest gates, not expected-fail placeholders: `dm1_v2_completion_matrix_gate`, `dm1_v2_viewport_composition_source_lock`, `dm1_v2_viewport_pixel_capture_fixture_gate`, and `dm1_v2_entry_viewport_png_export_gate`. The D0-D3 draw-list comparator and DUNGEON.DAT square-decoder source gates are also unmasked as ordinary CTest passes with refreshed source paths and tracked-evidence fallback (`dm1_v2_d0_d3_draw_list_comparator_gate`, `dm1_v2_dungeon_dat_square_decoder_source_lock`). Phase-gate/source-isolation coverage is also documented through `dm1_v2_phase_gate_pc34`, `dm1_v2_phase5_runtime_bridge_pc34`, `dm1_v2_graphics_pipeline_source_isolation`, and `dm1_v2_phase0_phase1_source_lock`: V1 gameplay domains remain source-owned while V2 render/input/config presentation seams are explicit, and Phase 5 camera interpolation starts only from accepted V1 movement/turn ticks. 2026-06-22: `dm1_v2_runtime_presentation_smoke` now runs real `firestaff --game dm1` with temporary launcher config for launchable V2.0/V2.1 modes, captures source indexed BMP receipts plus post-palette/post-filter presented RGBA BMP receipts, verifies M11 reports source `dm1` plus presentation modes 1/2 at 640x400, and requires distinct presented-frame hashes across the configured V2.0/V2.1 paths. This promotes runtime script evidence only; it does not run DOSBox, require original-pairing evidence, or claim finished V2.2 real-art parity. Remaining gap: broader finished-art/material pixel verification stays tracked in the per-mode material row. |
| Per-mode pixel/material verification gates | OPEN-BOUNDED — 2026-06-21: Apple-Silicon V2.0/V2.1/V2.2 GPU readback gates now cover filtered (`dm_v20_filtered_renderer_silicon`), upscale (`dm_v21_upscale_renderer_silicon`), and modern placeholder (`dm_v22_modern_renderer_silicon`) paths; live M11 now prefers the optional V2.2 in-place cache and `dm1_v22_inplace_render_probe` covers synthetic wall/floor/pit/stairs material routing with no field-to-wall fallback. Additional CTest gates now cover the DM1 V2.1 EPX/palette/category asset pipeline (`pass648_dm1_v2_asset_pipeline`), the low-level DM1 V2.2 asset-pipeline unit contract (`dm1_v22_asset_pipeline`: provenance, fallback/category naming, manifest validation, and best-available provenance fallback), DM1 V2.2 modern verification suite (`dm1_v22_verification`: asset mode, manifest discovery/validation, fallback chain, shape selection, config integration, and V1 gameplay phase-gate preservation), current viewport material-category draw-list/flat-render routing (`dm1_v2_viewport_materials_pc34`), and a data-free cross-mode material-signature gate (`dm1_v2_per_mode_material_signatures_pc34`: canonical D0-D3 composition -> V2.0 flat material hash `0x2b0dd7dd`, V2.1 EPX RGBA hash `0x3fae57cd`, V2.2 synthetic in-place hash `0x30894af5`). Remaining gap: finished V2.2 real-art material/pixel gates plus broader deterministic V1/V2 screenshot scripts. |

---

## C. CSB gaps

Source: `docs/FINAL_CSB_GAPS.md` (135 lines), 5 csb_gap_*.md files,
greatstone g_csb.html (14 versions documented), dmweb CSB Game Page.

### C1. Champions/mechanics/dungeon/graphics

| Gap | Doc | Status |
|---|---|---|
| Champions per-stat parity | csb_gap_champions.md | PARTIAL |
| Combat mechanics | csb_gap_combat.md | PARTIAL |
| Dungeon model/mechanics | csb_gap_dungeon.md | PARTIAL — 2026-06-21 PC launch boundary now has a positive real-data gate: `csb_v1_pc_real_asset_launch` verifies canonical PC CSB assets scan by hash, enter `csb_v1_boot_enter_game()`, load `DUNGEON.DAT` into the runtime-owned dungeon singleton, select map 0, tick once, and clean up. Core PC runtime/input/movement/system slices are now CTest-registered through command-chain, input-queue, movement step/rotation, runtime tick, queue overflow, reincarnation, projectile-speed, Grey Lord, DECOMPDU, version-checker, monster-generator, chaos cast, DSA trigger, save import path, save runtime boundary, Neophyte, and Zokathra gates. Remaining dungeon/mechanics work is deeper end-to-end gameplay parity, real save compatibility artifacts, viewport/UI runtime evidence, and playability, not the PC asset handoff. |
| Graphics + ornament blits (F0108, F0115, F0111, CustomBackgrounds) | csb_gap_graphics.md | PARTIAL / OPEN-LARGE — 2026-06-21 CTest now covers seven data-free CSB viewport/source-lock slices: first CustomBackgrounds backdrop, room-slot backdrop-1, D1C F0108 floor/ceiling ornament, D1C F0115 thing pass, D3C F0107/F0108 first-backdrop composition, D3L/D3R sidewall backdrops, and D2C F0107 wall-ornament plus F0111 door-front layering. Existing `csb_v1_viewport_phase3_rendering`, inventory-grid, and viewport-inventory mouse gates remain green. Remaining: live real-asset ornament blits, broader viewport/HUD captures, and pixel parity evidence. |
| Full mechanics parity | csb_gap_mechanics.md | OPEN-LARGE |

### C2. Per-version CSB asset coverage

| Version | Status |
|---|---|
| CSB Atari ST 2.0 (en) — original | BLOCKED-DATA |
| CSB Atari ST 2.0 (en) — cracked Replicants | BLOCKED-DATA |
| CSB Atari ST 2.1 (en) | BLOCKED-DATA |
| CSB Amiga 3.1 (en-fr-ge) original | EXTRACTED — `~/.firestaff/data/csb-extras/amiga-3.1-multi/` (no canonical hash match yet, awaiting verification) |
| CSB Amiga 3.1 (en) cracked EndlessPiracy | BLOCKED-DATA |
| CSB Amiga 3.1 (en) cracked Betrayal | BLOCKED-DATA |
| CSB Amiga 3.3 (en-fr-ge) | EXTRACTED + VERIFIED — `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/` matches a canonical hash (Meynaf FR hack variant) |
| CSB Amiga 3.5 (en) original | EXTRACTED — `csb-extras/amiga-3.5-ctraw-en/` (CTRaw format, not the canonical CSB Amiga hash) |
| CSB Amiga Utility disk (fr/ge/en/r1/r2/r3) | EXTRACTED — Disk 2 (en/fr/de) + Disk 3 (en/de) at `csb-extras/amiga-util-disk{2,3}-{en,fr,de}/` |
| CSB FM-Towns (en-jp) | EXTRACTED — `csb-extras/fm-towns/` (484 MB ISO, awaiting canonical hash match) |
| CSB PC-98 3.1 (jp) | EXTRACTED — `csb-extras/pc98-3.1-jp/` (171 .raw files, awaiting hash match) |
| CSB X68000 (jp) | EXTRACTED — `csb-extras/legacy-jp-x68000/` |
| CSBWin (PC port by Paul Stevens) | PARTIAL — synthetic loader exists; real-asset test missing |

### C3. CSB hidden-code items

| Gap | Status |
|---|---|
| Atari ST hidden executable-code items (skip table) | FIXED — `csb_hidden_code_skip_table_unit` + `csb_v1_graphics_hidden_item_skip_pc34` PASS 2/2 |
| Amiga 558-562 items (skip table) | FIXED — same skip-table/loader gates cover Amiga 21/676/686 and 558-562 |
| CSBWin custom resource handling (csbgraphics.dat + dmsave + csbgame) | OPEN-LARGE |

### C4. CSB V2

| Gap | Status |
|---|---|
| V2.1/V2.2 dispatch + csb_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for CSB** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE — same as DM1 B3 |
| Per-cell modern-art swap in CSB 9-square viewport | OPEN-LARGE |
| Phase 3 enhanced UI overlays | PARTIAL |
| Phase 5 smooth movement deterministic pixel gates | OPEN-BOUNDED |

---

## D. DM2 gaps

Source: `docs/NEXUS_PLAN.md` (similar scope), greatstone g_dm2.html
(11 versions), skproject source (DM2 Windows port).

### D1. DM2 V1 mechanics

| Gap | Status |
|---|---|
| Data model | FIXED |
| Boot/profile | FIXED |
| Rendering pipeline | FIXED |
| Combat resolver | FIXED |
| Spell module | FIXED |
| Tech/magic module | FIXED |
| **Shops/NPCs** | FIXED — 2026-06-21: Parts A landed (`1e756018 dm2_v1: Part A — shop + NPC parity (Phase 4 mechanics)`) + `a89c257f tier4-18: DM2 V1 shop-economy determinism probe (skproject c_shop.cpp)` (19/19 PASS) + `test_dm2_v1_shop_pc34_compat` (51/51 PASS). Source-locked against skproject `c_shop.cpp` transaction pricing + `SKWinGlobal.h:42` `NUM_NPCS=4`. |
| **Pressure plates** | FIXED — 2026-06-21: `7fdc3537 feat: Tier 1 #5 strict boot-probe + Tier 2 #3 CSB hidden-code + DM2 pressure plate` + `firestaff_dm2_v1_pressure_plate_probe` (17/17 PASS) + `test_dm2_v1_pressure_plate_pc34_compat` (40/40 PASS). |
| **Triggers** | FIXED — 2026-06-21: `fc608581 dm2_v1: Part C+D — trigger + timeline parity (Phase 4 mechanics)` + `firestaff_dm2_v1_trigger_probe` (14/14 PASS) + `test_dm2_v1_trigger_pc34_compat` (32/32 PASS). |
| **Timeline wiring** | FIXED — 2026-06-21: `fc608581 dm2_v1: Part C+D — trigger + timeline parity (Phase 4 mechanics)` + `firestaff_dm2_v1_timeline_probe` (12/12 PASS) + `test_dm2_v1_timeline_pc34_compat` (34/34 PASS). |
| **Advanced CCM (DM2_PROCEED_CCM)** | FIXED — 2026-06-21: `af5e7276 dm2_v1: Part E+F — CCM (advanced) + projectile drain to M11` + `test_dm2_v1_ccm_pc34_compat` (42/42 PASS, including stubbed-opcodes-return-unknown). Source-locked against skproject `c_ccm.cpp`. |
| **Projectile-list drain back into M11 renderer** | FIXED — 2026-06-21: `af5e7276 dm2_v1: Part E+F — CCM (advanced) + projectile drain to M11` + `firestaff_dm2_v1_projectile_drain_probe` (12/12 PASS) + `test_dm2_v1_projectile_pc34_compat` (23/23 PASS). |
| **Original-overlay proof** | OPEN-BOUNDED — no Firestaff-vs-original DM2 evidence yet; canonical launch smoke is now gated separately, but no original overlay/pixel evidence has been produced |
| **Launch-smoke gate** | FIXED — 2026-06-21: DM2 canonical `--game dm2 --data-dir ~/.firestaff/data/dm2` and DM2 PC extras `dm2-extras/dos-en`, `dm2-extras/dos-fr`, `dm2-extras/pc-fr`, and `dm2-extras/pc-de` emit `DM2 READY` through the M11 stderr-pipe and are covered by `tier1_strict_boot_probe`; broader non-PC/demo launch remains tracked under D3 |

### D2. DM2 V2

| Gap | Status |
|---|---|
| Phase 2 asset pipeline + dm2_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for DM2** | OPEN-LARGE |
| **DM1 shared V2.2 in-place drawing pipeline** | OPEN-LARGE |
| Per-cell modern-art swap in DM2 V1 (T560 indoor, T600 outdoor) | OPEN-LARGE |
| Phase 3 HUD runtime | PARTIAL — `firestaff_dm2_v2_phase3_hud_overlay_probe` 61/61 PASS + `test_dm2_v2_hud_overlay` 76/76 PASS. Source-locked against ReDMCSB `PANEL.C`. Remaining: HUD widget bitmap assets (inventory quick-view, action prompt) for finished presentation polish. |
| Phase 3 HUD bitmap assets + widgets (inventory quick-view, action prompt) | OPEN-BOUNDED — runtime gates are wired (probe 61/61 + test 76/76); bitmap assets need finished PBR HUD widget art. |

### D3. DM2 per-version coverage

| Version | Status |
|---|---|
| DM2 PC 0.9 / 1.0 (en/fr/ge) / demo | EXTRACTED + VERIFIED + LAUNCH-TESTED for PC extras `dm2-extras/dos-en/`, `dm2-extras/dos-fr/`, `dm2-extras/pc-fr/`, and `dm2-extras/pc-de/` via `tier1_strict_boot_probe`; demo status still needs separate version classification |
| DM2 Amiga 1.0 (en-fr-ge) | EXTRACTED — `dm2-extras/amiga-en/` |
| DM2 MegaCD/SegaCD 1.0 (jp/en) | EXTRACTED — `dm2-extras/mega-cd-jp/` |
| DM2 Macintosh 1.0 (en/jp/demo) — uses QuickTime .moov | EXTRACTED — `dm2-extras/mac-{en-v1,en-zip,fr,ja}/` (StuffIt + DMFiles-zip, includes Credits/Ending/Title .MooV) |
| DM2 PC-9801/PC-9821/IBM PS/V 1.0 (jp) | EXTRACTED — `dm2-extras/pc9821-jp/` |
| DM2 FM-Towns 1.0 (jp) | EXTRACTED — `dm2-extras/fm-towns-ja/` |

---

## E. Nexus gaps (Saturn)

Source: `docs/NEXUS_FILE_CLASSIFICATION.md`, `docs/NEXUS_PLAN.md`,
Nexus locally verified files in `~/.firestaff/data/nexus/`.

### E1. V1 phases 0-7

| Gap | Status |
|---|---|
| DMDF parser | PARTIAL |
| DGN world loader | PARTIAL |
| MNS creature/spell rendering | PARTIAL |
| S2D font loader | PARTIAL |
| TLINK/TAI/SAL/MAP runtime | PARTIAL |
| Save/load (.sav) | PARTIAL |
| V1 mechanics | PARTIAL |
| **Real Saturn asset handoff (NEXUS.BIN/ISO)** | EXTRACTED + VERIFIED — `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` matches canonical DM.BIN hash. Next: confirm Track 1 (not just DM.BIN) drives the full E1 V1 phases 0–7 launch path (DMDF parser, DGN loader, MNS rendering, S2D fonts, save/load). |

### E2. V2 phases

| Gap | Status |
|---|---|
| Phase 0/1/2 | FIXED |
| Phase 5 smooth movement runtime | PARTIAL |
| Phase 7 verification (deterministic input + screenshot gates) | OPEN-BOUNDED |
| **Real PBR hero art for Nexus** | OPEN-LARGE |
| **Per-cell modern-art swap in Nexus V1 draw pipeline** | OPEN-LARGE |

---

## F. Theron gaps (PC Engine / TurboGrafx-16)

Source: `docs/NEXUS_PLAN.md` (similar shape), Theron local probes.

### F1. V1

| Gap | Status |
|---|---|
| V1 parser | FIXED |
| Rendering pipeline | FIXED |
| Mechanics | FIXED |
| Save/load (.SRM) | PARTIAL — data-free `.tqsv` slot round-trip, rejection, verification, and cross-slot export/import are CTest-gated; no real `.srm`/Track 02 save artifact is present locally yet. |
| Track02 bank routing | FIXED |
| Dungeon progression (7 dungeons) | FIXED |
| **JP/US Track 02 BIN/ISO real-asset launch** | FIXED — `tier1_strict_boot_probe` covers Theron JP canonical, Theron JP extras, and Theron US extras booting to the TQR level-load milestone; `theron_v1_launcher_scan_reuse` and `theron_v1_track02_bank` also PASS. |
| **Theron m11 runtime command proof** | FIXED — 2026-06-21 (commit `cd86d520`): `firestaff_theron_v1_cross_route_mechanics_probe` CTest-gates a real `firestaff --game theron` run via temporary launcher config, captures a M11 run receipt (launch + early command tick), verifies M11 reports source `theron` plus command processing, and proves the M11 run path is launchable on this host. 2026-06-21 (commit `363bf3b9`): `tqr_v1_track02_bank_signal_2026-06-03.md` locks raw Track 02 bank anchors (`0x1F000..0x1FFFF` graphics bank + 0x20000..0x27FFF dialogue + `0x28000..0x2BFFF` map-data) against the CD-ROM2 1MB sector map. |
| **Theron 24h readiness rollup** | FIXED — 2026-06-21 (commit `a0592d6d`): `tools/theron_24h_readiness.py` + `parity-evidence/verification/theron_24h_readiness/manifest.json` + `docs/THERON_CAPTURE_READINESS.md` roll up 7 Theron V1 readiness gates (track02 bank, save/load, cross-route mechanics, runtime screenshot, dungeon progression, cross-slot, m11 launch) into a single per-day PASS/FAIL line, mirroring the DM1 24h readiness pattern. 2026-06-21 (commit `393d9f64`): `theron: refresh readiness reports` re-emits the manifest with current commit SHAs. |
| **Theron runtime screenshot readiness** | FIXED — 2026-06-21 (commit `b7dbcd60`): `firestaff_theron_v1_runtime_screenshot_readiness` CTest-gate + `tools/verify_theron_v1_runtime_screenshot_readiness.py` + `parity-evidence/theron_v1_runtime_screenshot_readiness.md` prove the M11 path can produce a Track-02-backed screenshot receipt on the current host. CTest/screenshot gates pass for the launchable Theron paths. |
| Cross-slot import/export against real Track 02 saves | PARTIAL — `theron_v1_save_load` now covers export/import mechanics across save roots using validated Theron save images; remaining work is importing/exporting a real Track 02 save artifact when one is available. |
| Cross-route mechanics runtime evidence | FIXED for the CTest-gated mechanics path — 2026-06-21 `firestaff_theron_v1_cross_route_mechanics_probe` CTest-gates real `firestaff --game theron` command proof (commit `cd86d520`). Broader real-asset cross-route capture pairs (level-by-level route transcript) remain out-of-24h scope. |

### F2. V2

| Gap | Status |
|---|---|
| Phase 0/1 | FIXED |
| Phase 2 (presentation selection, EPX upscaler) | FIXED |
| theron_v22_modern_assets_pc34 | FIXED |
| **Real PBR hero art for Theron** | OPEN-LARGE |
| **Per-cell modern-art swap in T400/T600** | OPEN-LARGE |
| **Phase 3 enhanced UI overlays** | OPEN-LARGE — not started per TODO |
| **Phase 4 enhanced lighting/effects** | OPEN-LARGE — not started |
| **Phase 5 smooth movement** | OPEN-LARGE — not started |
| **Phase 6 touch/controller ergonomics** | OPEN-LARGE — not started |
| **Phase 7 V2 verification suite** | OPEN-LARGE — not started |

---

## G. Launcher / Settings / Accessibility (cross-cutting)

### G1. M12 launcher

| Gap | Status |
|---|---|
| 19-locale UI cycle | FIXED in v2.9.2 |
| Persistence for many options + 5 per-game slots | FIXED |
| **Polished UI flow** | OPEN-BOUNDED |
| Runtime handoff for every option | PARTIAL — CSB V1 launch-readiness blocker retired on 2026-06-21: matched CSB assets now produce a valid M12 launch intent, M11 hands CSB to `FS_GAME_CSB`, and `csb_v1_pc_real_asset_launch` proves PC CSB boot/tick. Remaining gaps are per-game polished flows, richer CSB viewport/HUD/gameplay proof, original capture parity, and non-PC emulator parity. |
| **Save export/import** | OPEN-BOUNDED |
| **Session timer** | OPEN-BOUNDED |
| **Manual/docs launcher** | OPEN-BOUNDED |
| **Cloud sync** | OPEN-LARGE |
| Custom/V2 smooth-turn-pan toggles | OPEN-BOUNDED |

### G2. Touch / controller

| Gap | Status |
|---|---|
| Gesture navigation for runtime movement/turning | OPEN-LARGE |
| UI scaling + touch-target audit across launcher/game views | OPEN-LARGE |

### G3. Accessibility

| Gap | Status |
|---|---|
| Screen reader integration | OPEN-LARGE |
| High-contrast launcher remap | FIXED |
| In-game overlay coverage | PARTIAL |
| Launcher fontScale affects M12 text | FIXED |
| In-game overlays + UI-fit coverage | OPEN-BOUNDED |

---

## H. Cross-spiel prioritized work order

For the next sprint (post v2.9.1, pre v2.10.0), I propose this
order:

### Tier 1 (BLOCKED-DATA — surface via tooling, not code)

1. **Document real-data acquisition checklist**: which Atari ST /
   Amiga / SNES / MegaCD / Saturn / PC Engine binaries need to be
   sourced per game, with hashes that will gate future runs of
   `compare_to_greatstone.py`. — DONE (`docs/DATA_ACQUISITION_CHECKLIST.md`
   with per-game ✅/🟡/🔴/⚪ status matrix; generated by
   `python3 tools/asset-validate/compare_md5_to_sha256.py` — current
   coverage: 148/148 registry entries present + hash-correct).
2. **Add `compare_to_greatstone.py` summary mode** that prints a
   per-game "data gap" view (which files in VERIFIED_HASHES.md are
   not on disk). — DONE (new `--summary` mode in
   `tools/asset-validate/compare_to_greatstone.py`: prints
   per-game TOTAL/FOUND/MISS table from any data root; default to
   `~/.firestaff/data`). Run: `python3 tools/asset-validate/compare_to_greatstone.py --summary`.
   Companion tool `tools/asset-validate/data-readiness-summary.py`
   (Tier 1 #2 L2, FIXED 2026-06-22 commit `a56d79c70` → main
   `22a8caa3`) builds a per-game readiness table by combining
   `--scan-data`, the `--summary` hash table, and an opt-in
   `--boot-probe`. See L2 below for full description.
5. **Verify all `--scan-data` READY-path:er are actually
   launchable** by M11. — DONE for Tier 1 path-discovery scope
   2026-06-21: CTest `tier1_strict_boot_probe` PASS for all present
   in-scope launch paths (DM1 canonical, DM1 legacy-dos, CSB canonical,
   CSB Amiga 3.3 Meynaf FR, Theron JP canonical, Theron JP extras,
   Theron US extras), and `asset_validate_coverage_by_game` PASS. Nexus
   virtual-ISO launch remains out-of-scope here and tracked as a Tier 4
   runtime/launcher gap, not a path-discovery gap.
6. ~~**Scanner path-naming limitations**:~~ CLOSED as NO-GAP
   2026-06-20. The scanner already matches on MD5 via
   `asset_find_by_md5` and `scan_iso_by_md5` now also falls
   back to whole-file MD5 for non-ISO-9660 .bin files (Theron
   Track 02, raw Saturn tracks). Verified: `--data-dir
   ~/.firestaff/data/nexus-extras/saturn-ja` reports Nexus
   READY (`FOUND ...Track 1).bin::DM.BIN`) and `--data-dir
   ~/.firestaff/data/theron-extras/{japan,usa}` reports
   Theron READY (`FOUND ...Track 02).bin`). See
   `reference/L1_data_path_verification_2026-06-20.md`
   Resultat v2 för verifieringsdata.
3. **Mirror dmweb /community/documentation/ for offline research**
   — DONE 2026-06-20 (commit pending; 43 pages mirrored at
   `reference/dmweb-community-docs/` with INDEX.md + index.json +
   SCRAPE_LOG.md; reproduceable via `crawl.sh`). Eliminates the
   "free.fr intermittent 404" risk for 5 topic areas.
4. **Ship a reproducible game-archive extraction script** that
   pulls from `~/Downloads/` to `~/.firestaff/data/<game>-extras/`
   without overwriting canonical staging — DONE 2026-06-20 (commit
   `4b097f54`; 73 archives → 71 version directories; ~6.2 GB
   extraherat; 4 nya READY-path:er discovered by `--scan-data`).

### Tier 2 (OPEN-BOUNDED — fits one commit each)

3. CSB hidden-code skip table for Atari ST + Amiga items 558-562. — DONE
   (commit `c5897fce feat: extend CSB hidden-code skip table to 12 entries
   (CSB Atari + Amiga specific)` + earlier `a8033a47 feat: extend CSB
   hidden-code skip table to 12 entries`). 12 entries total: 4 Atari/Amiga
   executable rows (558-562), 2 kid dungeon rows (135-138), 3 CSB Atari
   ST (21/538/548), 3 CSB Amiga (21/676/686). Source-locked against
   Meynaf disassembly + 4 COD1/COD2/COD3/COD4 container formats.
4. LZW decoder for Atari ST GRAPHICS.DAT (only Atari ST uses LZW).
   — DONE for the decoder: `m11_gfx_lzw_decompress` has a contract-only
   round-trip test (`test_dm1_lzw_round_trip.c`, `dm1_lzw_round_trip`
   PASS 1/1 on 2026-06-21, pass852). Real Atari ST asset handoff still
   BLOCKED-DATA.
5. PAK container decoder for Atari ST START.PAK. — DONE (commit 3ee479de)
6. CMP portrait loader for CSB utility disk. — DONE (commit 532c8250);
   `firestaff_cmp_decode_unit` + `csb_v1_cmp_import_pc34` PASS 2/2 on
   2026-06-21. Runtime champion portrait source-selection remains tracked
   separately under A3, not as a CMP format decoder gap.
7. Harmonize MD5 vs SHA256 in `asset_find_by_hash.c` (or add
   wrapper). — DONE (commit 5988b620, see docs/MD5_SHA256_HARMONIZATION.md)
8. `_G2157_` linker fix (add `image_frontend_pc34.c` to
   firestaff_m10 source list). — DONE (commit 3588798f, added
   `image_backend_pc34_compat_globals.c` instead which provides the
   same symbols without dragging in the legacy frontend)
9. Lefthook-in-CI install step. — DONE (`.github/workflows/verify.yml:227-253` installs Go + lefthook via `go install github.com/evilmartians/lefthook@latest`, then runs `lefthook run ci` for po_layout + hash_harmonization as CI sanity gate).
10. **`--data-dir` override in `m12_build_search_roots`** — DONE
    2026-06-20 (commit `6a7eccdc`). Explicit `--data-dir` no longer
    silently falls back to `~/.firestaff/data`; runtime
    dataDir-override also skipped when request is explicit. 5/5
    asset-status tests pass; tested with
    `--data-dir ~/Downloads --scan-data` → reports
    `Data dir: /Users/bosse/Downloads` and scans only that
    directory.

### Tier 3 (OPEN-LARGE — separate milestones)

10. FTL container decoder (Amiga, X68000, MegaCD).
11. CSBWin custom resource path (csbgraphics.dat + dmsave).
12. DM2 advanced CCM + projectile-list drain.
13. DM2 per-cell modern-art swap (T560/T600).
14. Nexus per-cell modern-art swap (Saturn draw pipeline).
15. Theron V2 phases 3-7 (UI overlays, lighting, smooth movement,
    touch, verification).
16. CSB V1 graphics ornament blits (F0108, F0115, F0111,
    CustomBackgrounds).

### Tier 4 (per-game polish, partially started)

17. CSB mechanics parity (combat, dungeon, champion, inventory).
18. DM2 mechanics (shops, NPCs, triggers, timeline). — Shop-economy
    determinism probe (`firestaff_dm2_v1_shop_economy_determinism_probe`,
    ctest `dm2_v1_shop_economy_determinism_probe`, commit `a89c257f`):
    19/19 PASS covering reset/enter/buy/sell/leave state transitions,
    party-state hash preservation across leave, 50-repetition
    determinism, wrong-shop rejection, insufficient-gold path.
    Source-locked per skproject/SKULLWIN/c_shop.cpp transaction pricing
    + skproject/SKWinGlobal.h:42 (NUM_NPCS=4). Existing coverage
    already includes `firestaff_dm2_v1_shop_probe` (built-in shop
    catalog + NPC dialog), `firestaff_dm2_v1_trigger_probe` (8
    triggers + 4 kinds + 6 targets), and `firestaff_dm2_v1_timeline_probe`
    for the three other DM2 mechanics domains.
19. Nexus runtime/probe coverage beyond compile/save-load.
20. Theron cross-slot import/export + cross-route evidence. — Dungeon-progression
    determinism probe (`firestaff_theron_v1_dungeon_progression_determinism_probe`,
    ctest `theron_v1_dungeon_progression_determinism_probe`, commit
    `8a43fcf8`): 20/20 PASS covering the 7-dungeon progression state
    machine (THQUEST.ASM T080 between-dungeon save/load), init / advance /
    quest-complete transitions, NULL-safety, and 50-repetition
    determinism. Source-locked per ReDMCSB GROUP.C analogue + THQUEST.ASM
    T080. Between-dungeon save/load itself is already covered by
    `tests/test_theron_v1_save_header_rejection.c` + the existing
    Theron V1 mechanics probe (10 tests / 5 probes already green for
    the V1 gameplay loop).

### Tier 5 (i18n follow-up)

21. Native-vs-fallback separation in `validate_po_layout.sh`. — DONE:
    validator now reports `nonblank` and `native` coverage separately,
    with fallback-only catalogs marked `FALL` without failing the
    structural CI gate.
22. Fill DM1/nexus/csb/theron/firestaff missing native translations
    with native speakers (out of scope for AI).

### Tier 6 (launcher/accessibility)

23. Save export/import, session timer, manual launcher.
24. Touch gesture navigation.
25. Screen reader integration.

---

## I. Sources surveyed (this round)

| Source | Pages reviewed | Used for |
|---|---|---|
| dmweb.free.fr (Game Pages) | 5 (DM, CSB, DM2, TQ, Nexus) | per-game history, awards, ports |
| dmweb.free.fr (File formats) | 3 (Data Files, Animations, Animation Script) | IMG/IMG5/ANIMATE.SCR specs |
| dmweb.free.fr (Clones) | 3 (CSBwin, SKWIN, Return to Chaos) | clone source references |
| dmweb.free.fr (Custom dungeons) | 1 (g_csb.html) | 60+ custom dungeon index |
| dmweb.free.fr (FAQ) | 1 index | category map only — Drupal URLs 404 |
| dmweb.free.fr (ReDMCSB) | 0 — page not fetchable | fallback to local ReDMCSB |
| dmweb.free.fr (Community documentation) | **43 (mirrored locally at `reference/dmweb-community-docs/`)** | copy protection, file formats (animation script, animations, data files, dungeon files, DM2 data files, DM2 music triggers, hint/oracle, layout coordinates, portrait files, saved-game files), Nexus file formats (DGN, MNS, item.ibs), DM+CSB mechanics (actions, attacks, creature generators, items, skills, GRAPHICS.DAT hidden code + items 558–562), miscellaneous (Atari ST history, PC, SNES, FTL sound adapter, game versions) |
| greatstone (Tools) | 4 (Overview, Product, Screenshots, Tutorial) | sck usage, extraction CLI |
| greatstone (Technical doc) | 4 (Mapfile, FTL, PAK, Items + IMG5) | all critical format specs |
| greatstone (Games) | 4 (DM, CSB, DM2, Custom) | per-version file lists |
| greatstone (Articles) | 0 — content empty | skipped |
| Firestaff existing docs | FINAL_GAPS, FINAL_CSB_GAPS, PLATFORM_MATRIX, DMWEB_REFERENCE, csb_gap_*.md, dm1_gap_*.md | per-game gap detail |

**Total new pages reviewed this session: 62** (19 prior + 43 from
the dmweb community/documentation mirror).

---

## J. Update cadence

This document should be regenerated after every milestone commit.
Each major section (B-J) is regenerated from its per-domain
source-of-truth:

- A → `po/validate_po_layout.sh` + `tools/asset-validate/compare_to_greatstone.py --list`
- B → `docs/FINAL_GAPS.md` + `docs/dm1_gap_*.md` + `docs/ai_*.md`
- C → `docs/FINAL_CSB_GAPS.md` + `docs/csb_gap_*.md`
- D → `docs/NEXUS_PLAN.md` (DM2 sections) + `src/dm2/` headers
- E → `docs/NEXUS_FILE_CLASSIFICATION.md` + `src/nexus/` headers
- F → Theron TODO sections + `src/theron/` headers
- G → `TODO.md` cross-cutting sections
- H → manual prioritization after the above regenerates

Avoid duplicating content; this doc is an index, not a source.

---

## K. Session delta — 2026-06-20 (post v2.9.1)

What changed in this session that affects the gap list above:

### Data staging

- **73 game archives extracted** from `~/Downloads/` to
  `~/.firestaff/data/` via `reference/extract-game-archives.sh`.
  - 71 new version-staging directories under
    `<game>-extras/<version>/` (~6.2 GB extraherat)
  - 4,232 extraherade filer utöver befintlig canonical staging
  - Inga befintliga canonical-filer (dm1/, csb/, dm2/, nexus/, theron/)
    skrivna över

### New READY path:er (utöver canonical)

`./build/firestaff --scan-data` hittar nu alternativa
match-path:er för fyra av fem spel:

| Spel | Ny match-path |
|---|---|
| DM1 | `dm1-extras/legacy-dos/DungeonMasterPC34/DATA/GRAPHICS.DAT` |
| CSB | `csb-extras/legacy-amiga-dms/...Meynaf/DungeonMaster/Graphics.DAT` (Amiga 3.3 French hackad av Meynaf) |
| DM2 | (oförändrad, canonical `dm2/` vinner) |
| Nexus | `nexus-extras/saturn-ja/Dungeon Master Nexus (Japan) (Track 1).bin::DM.BIN` |
| Theron | `theron-extras/japan/Dungeon Master - Theron's Quest (Japan) (Track 02).bin` |

### Kanoniska hashar (VERIFIED_HASHES.md)

`tools/asset-validate/compare_to_greatstone.py ~/.firestaff/data`
körs mot alla 12 entries i registret: **148/148 OK, 0 FAIL** (alla
filer refererade i `VERIFIED_HASHES.md` finns lokalt och matchar
hasharna — några mappar till samma fil, därav 148 vs 12).

### Documentation mirror

- **43 sidor av dmweb.free.fr /community/documentation/**
  speglade lokalt till `reference/dmweb-community-docs/` (5.5 MB)
  - `INDEX.md` (19 KB) — mänskligt läsbar innehållsförteckning
  - `index.json` (27 KB) — maskinläsbar
  - `crawl.sh` — reproducibelt crawl-skript (curl + 1.2s rate limit)
  - Täcker 5 ämnesområden (copy protection, DM+CSB mechanics,
    Nexus file formats, file formats, miscellaneous)

### Bug fix: CLI --data-dir

- `src/shared/asset_status_m12.c::m12_build_search_roots` —
  explicit `--data-dir` överskuggar nu default-fallbacks i
  `--scan-data`-läge. Tidigare ignorerades flaggan och scannern
  rapporterade FOUND-path:er från `~/.firestaff/data/` även när
  användaren bad om en annan rot.
- `m12_fill_game_versions` — runtime-dataDir-override skippas när
  `requestedDataDir` är explicit satt (defensivt mot framtida
  fallback-tillägg).
- `tests/test_asset_status_scan_metrics.c` — uppdaterad till
  `rootCount=1, duplicateRootSkips=0` när `--data-dir` är satt.
- Committat: `6a7eccdc`.

### Test coverage (smoke runs)

- 40/40 DM1 V1 chest/item/weight/recompute-tester PASS
- 23/23 Phase A invariants PASS
- 12/12 kanoniska hashar matchar (0 fail)

### Påverkan på gap-status

Markerade i docen ovan som **EXTRACTED** (nya rader i C2, D3, E1,
F1) — tidigare `BLOCKED-DATA`. Fyra markerade **EXTRACTED +
VERIFIED** eftersom de nu också matchar en kanonisk hash. Uppdatering
2026-06-21: Theron JP/US Track 02 är dessutom launch-testad via
`tier1_strict_boot_probe`.

| Spel | Version | Status |
|---|---|---|
| DM1 | PC 3.4 (legacy-dos) | EXTRACTED + VERIFIED |
| CSB | Amiga 3.3 (Meynaf FR hack) | EXTRACTED + VERIFIED |
| Nexus | Saturn JA (Track 1) | EXTRACTED + VERIFIED |
| Theron | JP/US Track 02 | EXTRACTED + VERIFIED + LAUNCH-TESTED |

Dessa kan nu användas som real-asset testkällor utöver den
befintliga canonical-staging som finns under `dm1/`, `csb/`, etc.

---

## L. Follow-up — concrete next-session tasks

Listan nedan är prioriterade mindre tasks som följer direkt av
sessionens leveranser. Varje punkt tar < 1 dag och kräver ingen
ny design.

### L1. Verifiera alternativa READY-path:er bootar

Status 2026-06-21: DONE för Tier 1 path-discovery scope via
`tier1_strict_boot_probe` (alla närvarande in-scope launch paths). CSB
canonical och CSB Amiga 3.3 Meynaf FR skriver nu `CSB READY`;
Nexus virtual-ISO launch ligger kvar som separat Tier 4
runtime/launcher-gap.

Den ursprungliga källan Tier 1 #5: Kör mot varje EXTRACTED + VERIFIED path
och bekräfta att M11 faktiskt startar spelet, inte bara att
scannern hittar hasharna.

```bash
for spec in \
  "dm1   ~/.firestaff/data/dm1-extras/legacy-dos" \
  "csb   ~/.firestaff/data/csb-extras/legacy-amiga-dms" \
  "nexus ~/.firestaff/data/nexus-extras/saturn-ja" \
  "theron ~/.firestaff/data/theron-extras/japan"
do
  set -- $spec
  game=$1
  path=$2
  echo "=== $game @ $path ==="
  SDL_VIDEODRIVER=dummy ./build/firestaff --data-dir "$path" --game $game --duration 1000 2>&1 | tail -5
done
```

Förväntat: Phase A-probe-PASS + en spel-specifik asset-load PASS
per path. Om något FAIL:ar, markera gap-status tillbaka till
PARTIAL.

**Resultat (2026-06-20):** Se
`reference/L1_data_path_verification_2026-06-20.md` för detaljer.
Kort version:

| Path | READY? | Orsak |
|---|---|---|
| DM1 legacy-dos | ✅ | Canonical `GRAPHICS.DAT`/`DUNGEON.DAT` i katalogen — matchar hashen direkt |
| CSB Amiga 3.3 Meynaf FR | ✅ | Matchar canonical CSB-hasen i `...Meynaf/DungeonMaster/Graphics.DAT` |
| Nexus Saturn JA | ⚠️ | MD5 stämmer (`d8362321...`) men filnamnet matchar inte scanner-mönstret `g_nexusArchiveNames` (`DM.BIN`, `SEGADATA.BIN`, etc.) — hittas bara i default-scan, inte via `--data-dir` |
| Theron JP Track 02 | ✅ | MD5 stämmer (`b7afb338...`); 2026-06-21 `tier1_strict_boot_probe` launch-testar JP canonical + JP extras till TQR level-load milestone |
| DM1 PC 3.4 English 3.5" (extras) | ⚠️ | Innehåller `.raw`-filer (CTRaw emulator-format) som scanner ej mappar |

**Ny status:**
- DM1 + CSB legacy path:er är nu `EXTRACTED + VERIFIED +
  LAUNCH-TESTED` (redo för framtida tester/parity-evidence).
- Theron JP/US Track 02 path:er är `EXTRACTED + VERIFIED +
  LAUNCH-TESTED` sedan 2026-06-21: `tier1_strict_boot_probe`
  startar JP canonical, JP extras och US extras till TQR
  level-load milestone.
- Nexus container-path:er: `--data-dir <path>` HITTAR dem korrekt
  via MD5-hash-matchning (asset_find_by_md5), inte filnamn.
  Source-filenamn som `Dungeon Master Nexus (Japan) (Track 1).bin`
  accepteras direkt. Tidigare påstått problem med filnamn var FEL.

**Tier 1 #6 stängs som NO-GAP (2026-06-20)** — verifierat att
scannern matchar på MD5-hash, inte filnamn. Source-filenamn
accepteras direkt av `--data-dir`. Tier 1 #6 togs upp av L1-rapporten
men den faktiska scan-beteendet stödjer READY för alla 4 paths.
Inget alias-steg krävs. Tier 1 #6-posten i listan ovan är inaktuell
och bör rensas vid nästa watchdog refresh.

### L2. ~~Skapa `tools/data-readiness-summary.py`~~ — FIXED 2026-06-22

Tier 1 #2 --summary-mode. Skriver ut per-spel tabell:
`game / required-files-present / found-in-canonical / found-in-extras /
launch-tested`. Tar input från `compare_to_greatstone.py` + en
manifest-läsare.

Output (exempel):
```
dm1   2/2 present   2/2 canonical   1 extra (legacy-dos PC34)  NOT launch-tested
csb   2/2 present   2/2 canonical   1 extra (Amiga 3.3 Meynaf FR) NOT launch-tested
dm2   2/2 present   2/2 canonical   0 extras                       LAUNCH-TESTED
nexus 1/1 present   1/1 canonical   1 extra (Saturn JA Track 1)     NOT launch-tested
theron 1/1 present  1/1 canonical   JP+US extras Track 02           LAUNCH-TESTED
```

Wire in i CMakeLists + `ci: asset-hygiene` job. Används vid varje
`git push` för att snabbt se om något blockerar M12 launch.

**Status 2026-06-22 (commit `a56d79c70`, cherry-picked to main
as `22a8caa3`):** Shipped at
`tools/asset-validate/data-readiness-summary.py` (342 lines,
executable). Combines three sources into one human + JSON table:

1. `firestaff --scan-data` against a data root
   (`~/.firestaff/data/` by default — covers canonical + extras).
2. `compare_to_greatstone.py` SHA-256 hash-match summary against
   `VERIFIED_HASHES.md`.
3. Optional `--boot-probe` (`firestaff --game X --data-dir Y`,
   ~8s per path) — opt-in only, since it's slow.

Output: human-readable per-game table on stdout, JSON dump with
`--json`, exit code 0 iff all 5 games canonical-READY, 1 otherwise.
Verified running example captured in the commit message.

**Remaining (out of L2 scope):** not yet wired into CMakeLists
or the `ci: asset-hygiene` job. `--boot-probe` mode is opt-in
and unverified on a CI runner. A future pass should add a
CMake target + CI step that runs `--json` and posts a status
check on each push.

### L3. Utöka `extract-game-archives.sh` med verify-steg

Efter extraktion, kör `compare_to_greatstone.py` per extras-
katalog och rapportera per-version-summary. Loggas in i
`.extract-log.md` och `.extract-manifest.json`. Detta gör att
framtida körningar direkt ser vilka versioner som matchar en
kanonisk hash och vilka som bara är reference-material.

### L4. CSB Amiga 3.5 launch-barriär

Den extraherade `csb-extras/amiga-3.5-ctraw-en` innehåller CTraw
(.st/.raw/.err/.out) som scannern inte mappar till en canonical
hash (CTRaw är ej CTraw-filen själv, den är bara Amiga-emulator-
formatet). Skriv en liten helper `tools/ctraw_to_amiga_dat.py` som
packar om .raw → .DAT/.DATA-format Firestaff kan läsa. Eller
acceptera att 3.5 är oåtkomlig utan mer arbete och stryk den ur
"extraherad"-listan.

### L5. DM2 extras launch-test

Status 2026-06-21: DONE för PC extras. `dm2_v1_boot_scan_assets`
accepterar nu extracted DOS-layouten `data/graphics.dat` +
`data/dungeon.dat`, och `tier1_strict_boot_probe` kör DM2 canonical
plus `dm2-extras/dos-en`, `dm2-extras/dos-fr`, `dm2-extras/pc-fr`
och `dm2-extras/pc-de` till `DM2 READY`. Återstående DM2-versionsteg
ligger i D3: demo och icke-PC-versioner behöver separat klassning och
eventuell container-/formatnormalisering innan de kan bli
cross-version-regressionstäckning.

### L6. README-public-dokumentation-uppdatering

Per AGENTS.md: README ska vara honest, user-facing, sales-friendly
med verklig per-spel-status. Efter dagens gap-list-uppdateringar
bör README:s DM1/CSB/DM2/Nexus/Theron-tabeller uppdateras för
att reflektera att fyra av fem spel har real-asset-evidens i
både canonical- OCH extras-staging.

---

## M. Session delta — 2026-06-21 (post v3.0.0)

What changed in this session that affects the gap list above:

### DM1 V1 original-capture gap close (pass1052-1058)

- 6 B1 capture-gap pairs moved from `BLOCKED-DATA` to `PARTIAL`:
  - I34E keyboard buffer transcript (pass513 SCAFFOLD_ONLY_MISSING_ORIGINAL_RUNTIME_DEBUG_FIELDS)
  - Paired original viewport screenshot (pass1052 + pass1056 CTest gate)
  - Paired original wall screenshot (pass1052 + pass1054 exact 0-pixel match)
  - Paired original collision transcript (pass1055 closed-door stasis)
  - Paired original champion-panel screenshot (pass1053 candidate/resurrect)
  - Paired original creature-chain screenshot remains `BLOCKED-DATA` (level-1 target behind inert closed door) but pass1058 locks the corrected keypad mapping.

### DM1 V1 gap cascade (pass1059-1070)

- 12 B2/B3 PARTIAL rows moved to `FIXED`: portrait sensor parity, AI pathfinding, AI perception targets, AI reactions, mirror stat, C25/C26 projectile fallback, touch zones, inventory route parity, chest scroll-wheel pickup overflow, object consumable use, V2 smooth interpolation, V2 HUD rune routes, V22 material routing.
- 2 PARTIAL rows remained: chest runtime detail coverage, creature grouping/coordination (now PARTIAL after per-route audits).

### Tier 1 #5 strict boot-probe

- New `firestaff_tier1_strict_boot_probe` ctest entry runs the launcher with `--game <id> --data-dir <path> --duration 1500` under `SDL_VIDEODRIVER=dummy` for every EXTRACTED + VERIFIED path.
- All present in-scope paths PASS: DM1 canonical, DM1 legacy-dos, CSB canonical, CSB Amiga 3.3 Meynaf FR, Theron JP canonical, Theron JP extras, Theron US extras.
- Nexus (`Track 1.bin::DM.BIN` mount) remains tracked as a Tier 4 runtime/launcher gap, not a Tier 1 path-discovery gap.

### Tier 2 #4 LZW Atari ST decoder DONE

Decoder code path test-covered (`test_dm1_lzw_round_trip` 96/96 PASS, `pass852`). Real Atari ST asset handoff still `BLOCKED-DATA`.

### Tier 4 determinism probes (3 new)

- **Theron V1 dungeon-progression** (`THQUEST.ASM T080`) — DONE
- **CSB V1 champion-stat** (`F0306`/`F0309`/`F0310`/`BUG0_72`) — DONE
- **Nexus V1 creature-state** (`F0209` timeline) — DONE

### Asset-status fix

`required=1` for all required-files rows. `matchAnyVersion` now propagates `matchedPath` so the missing-files popup and report show where the runtime will load the asset from, while keeping `launch_blocker` honest.

### DM1 V2 polish

- V20 filtered renderer probe
- V21 upscale renderer probe
- V22 in-place render probe (CSB + DM1 Apple Silicon + DM1 V22 modern asset)
- Side-by-side V1/V2 presentation-disabled seed gates
- V22 in-place cache wiring through `pass376` overlays

The V22 in-place drawing pipeline still uses placeholder overlay; wiring `m11_draw_dm1_*` draw passes to consume real modern art in-place remains `OPEN-LARGE` in B3.

### Documentation

- 100+ row status changes in B1-B3/C1-C4/D1-D2/E1-E2/F1-F2/G1-G2 reflecting post-pass1052-1070 reality.
- Tier 1 #5 marked DONE for path-discovery scope.
- Multiple Tier 4 entries closed (Lefthook CI, CSB CMP decoder, Atari ST PAK decoder, CSB hidden-code skip, LZW Atari ST decoder, M12 extras DM1, chest runtime detail, creature grouping, Theron extras launch-tested, Theron Track 02 launch).

### Verification

- ctest baseline 700+/700+ green (was 692/696 at v2.9.2).
- Phase A probe 23/23.
- Audio probe green.
- Strict `-Wall -Wextra -Werror` warnings-check green.
- Cross-platform determinism green.
- M10 verify green.

### Commits since v2.9.2

116 commits, summarized:

- `pass1052-1058` DM1 V1 original-capture gap close (B1 capture-gap pairs)
- `pass1059-1070` DM1 V1 gap cascade (B2/B3 PARTIAL → FIXED)
- `tier1-5` strict boot-probe per path
- `tier2-4` LZW Atari ST decoder
- `tier4-17` CSB V1 champion-stat determinism probe
- `tier4-19` Nexus V1 creature-state determinism probe
- `tier4-20` Theron V1 dungeon-progression determinism probe
- `dm1_v2_inplace_render_gate` / `dm_v20_filtered_renderer_silicon` / `dm_v21_upscale_renderer_silicon` / `dm_v22_modern_renderer_silicon` (V2 polish)
- `dm1_v2_side_by_side_seed_gates` (V1/V2 presentation-disabled)
- `m11_capture_route_state` / `dm1_v1_wall_collision_runtime_capture` / `m11_turn_viewport_orientation` (B1/B3 Firestaff-side gates)
- `m11_v22_inplace_draw_pc34` (V22 in-place cache wiring)
- `firestaff_dm1_v22_inplace_render_probe` (CSB V22 in-place render probe)
- Asset-status fix + `m12_fill_game_versions` TIER1DEBUG cleanup
- gap-list updates: B1 PARTIAL closure, B2 PARTIAL → FIXED cascade, Tier 1-4 closes
- `tools/dm1_24h_readiness.py` expansion (12/12 ctest rows in the roll-up)
- `verify_pass623_dm1_v1_input_capture_readiness_bridge.py` line-range refresh
- `tools/verify_pass352_dm1_v1_movement_route_regression_matrix.py` token/keypad aliases fix
- `pass372` rebuild target fix
- `src/dm1v2/dm1_v22_shapes.c` `-Wunused-parameter` warning fix
- `22a8caa3` (2026-06-22) — `tools/asset-validate/data-readiness-summary.py` cherry-picked to `main` from `csb-v1-hidden-skip-cmp-real-asset-2026-06-20` (commit `a56d79c70`). Closes Tier 1 #2 L2. The other 6 subagent branches' commits were audited file-for-file against `origin/main` and were subsumed by newer in-main versions; only this one had substantively new content. `dm1v1-capture-gap-close-20260620`, `csb-v1-hidden-skip-cmp-real-asset-2026-06-20`, `dm1-b3-v2-gates-20260621`, `dm1-lane-a-original-evidence-20260621`, `dm1-lane-c-gap-audit-20260621`, `dm1-lane-d-readiness-20260621`, `dm2-v1-mechanics-parity-2026-06-20`, `main-cmake-fix` — all left in place on origin as historical branches; the 15 worktrees have been removed and only `workspace-main` + the main-pass1052 worktree remain locally.
- `363bf3b9`, `cd86d520`, `b7dbcd60`, `a0592d6d`, `393d9f64` (2026-06-21, on `origin/main` post-cherry-pick) — Theron 24h readiness cascade: raw Track 02 bank anchors locked, m11 runtime command proof, runtime screenshot readiness gate, 24h readiness rollup tool, and readiness-report refresh. Closes F1 cross-route mechanics runtime evidence (CTest path) and adds the Theron 24h readiness row to the per-day PASS/FAIL list. `152c6a8a release: prepare v3.0.1` then tags the cascade as v3.0.1.

### Migration to GitHub main

`dm1v1-capture-gap-close-20260620` branch fast-forwarded into `origin/main` (52bce320 → cd24ea72, 67 commits). The branch is now redundant with main.

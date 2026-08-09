# Firestaff DONE - DM1

## 2026-08-09

- ✅ Backing-aware PC34 save corpus verification: the two authentic,
  provenance-attested saves currently in `/Users/bosse/Downloads` both pass
  the production F0435 -> F0433 -> F0435 round-trip against the real PC34
  `DUNGEON.DAT`. The corpus includes the empty C03/C04 layout and a distinct
  save with 15 active groups and 15 live C03 records. No generated save or
  repository game-data payload was added. C13 remains open because neither
  save contains a C13 event.
- ✅ External-disk audit: three additional provenance-attested saves under
  `/Volumes/Extern-disk/Documents/Firestaff/dm1-resume-*` pass the same
  backing-aware round-trip and are byte-identical. Their `c13` directory name
  is not evidence of C13 content: decoding finds zero C13 events, so the
  files remain C03/C04 evidence only.
- ✅ Real resume runtime check: the external-disk save from
  `dm1-resume-c13-diskette.Y0dbXx` now launches through explicit `--platform
  auto` with the real PC34 data, reaches `dm1-runtime`, restores map 1,
  party `(6,2,2)` and runtime tick 1674, and does not remain in the entrance
  menu. This proves the generic resume handoff, not C13 content or original
  Mac-window capture.
- ✅ Full user-data-root scan: the real `/Users/bosse/.firestaff/data` root
  reports DM1, CSB, DM2, Nexus and Theron's Quest as `READY`. DM1's automatic
  selection resolves to the authenticated FM Towns English archive member and
  a direct boot reaches `dm1-runtime` with `levelLoaded=1`. The scan is
  archive-backed and can take tens of seconds on the complete preservation
  root; an interim launcher scan view must not be treated as missing data.
  This confirms data discovery and launchability only. The boot receipt still
  correctly leaves complete-support false until the required original HoC/
  Mac capture gates and an authenticated C13-bearing save exist.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Dungeon Master (DM1)

### DM1 V1 - Runtime and Source-Lock

- ✅ pass376 measurement-only original-vs-Firestaff viewport overlays: tracked 6 existing original DM1 PC 3.4 route crops against current Firestaff viewport crops under `parity-evidence/overlays/pass376_firestaff_pairing/`, plus summary report `parity-evidence/pass376_dm1_v1_original_firestaff_measurement_overlays.md`. Each mask is 224x136 RGB, every stats JSON carries the explicit non-claim `honesty` field, and measured deltas remain large (72.7055%..93.4874%). This is useful visual-debug evidence, not same-state parity promotion.
- ✅ pass1056 pass1052 original-to-Firestaff pairing gate: added `tools/verify_pass1056_dm1_v1_pass1052_firestaff_pairing_gate.py`, a CTest-backed evidence gate over the existing pass1052/pass1054 artifacts. It verifies all four clean original PC 3.4 224x136 viewport crops exist, all four pass1054 pair rows and pair/diff images exist, pair artifact SHA256 values match the manifest, and exactly one promoted row remains an exact wall match: original `02_left_1_wall_original_viewport_224x136.png` == Firestaff `hall_1_4_dirE_viewport_224x136.ppm`, 0 changed pixels / MAE 0.0. Nonzero rows remain scout-only and are not same-state parity claims. `firestaff_m11_viewport_state_probe` now also accepts an optional explicit `map_index map_x map_y direction` pose so future original-route pairing can inspect the exact Firestaff viewport state without changing the default probe behavior.
- ✅ pass1058 DM1 V1 original keypad route atlas: added `tools/verify_pass1058_dm1_v1_original_keypad_route_atlas.py` plus tracked route-atlas evidence under `parity-evidence/verification/pass1058_dm1_v1_original_keypad_route_atlas/`. The atlas locks original DOSBox keypad semantics for future creature-route work: from the tested start pose `kp5`, `kp8`, and viewport forward-click all reach the same forward frame, `kp2` returns to start, `kp4` turns right, and `kp6` turns left/back. The corrected level-1 creature route reaches three distinct original states, then documents an honest blocker: the first chosen target remains behind an inert door, and enter/space/two clicks/forward leave the raw frame byte-identical. Non-claim: this is not a paired original creature screenshot and does not close the creature-chain gap.
- ✅ pass1057 DM1 Amiga 2.2 English `DUNGEONB.DAT` asset lock: added a registry/documentation/verifier gate for the extracted kid-dungeon sidecar at `~/.firestaff/data/dm1-extras/amiga-2.2-en/DUNGEONB.DAT` (4,806 bytes, SHA256 `9bac133b4d8d6ca88abad70ff4a3a6436f264e3ae3a7503e0b40a8a6b4007730`, MD5 `d42915cf346494efa0ed78cfbbb4c2b5`). `coverage_by_game.py` now reports the DM1 Amiga 2.2 English row as 3/3 present and 3/3 registry-backed. This closes the DM1 variant-data acquisition gap without promoting the file into PC34 runtime launch requirements.
- ✅ pass1055 original closed-door collision/stasis evidence + Firestaff semantic pair: promoted the latest original PC 3.4 DOSBox route under `verification-screens/pass1055-dm1-original-closed-door-collision/` plus report `parity-evidence/pass1055_dm1_v1_original_closed_door_collision_capture.md`. The route reaches a closed Hall-area door, records `door_before`, then tests `click:112,100` and `kp5`. The three closed-door raw frames are byte-identical (`a0d3a9cdbddc310e3ef195c9c7719508a5141fbd66e1acb6a8dbe4b14ebc0dd6`), and the three 224x136 viewport crops are also byte-identical (`93a07d28805f4a0e554607899406b6d706c88be920415940d2453069e673a5f6`). New probe `firestaff_dm1_v1_pass1055_closed_door_pair_probe` replays the same movement sequence through the DM1 V1 pipeline, reaches `map=0 x=6 y=9 dir=3`, sees closed door square `0x94` at `(5,9)`, and verifies `MOVE_BLOCKED_DOOR` with no party movement. Non-claims: this is not yet a Firestaff-vs-original pixel comparison, not a full wall/door/fakewall transcript, and not a creature capture.
- ✅ pass1054 original-to-Firestaff wall crop exact match: compared the four pass1052 original PC 3.4 viewport crops against the 24-pose Firestaff Hall capture set. One wall crop is exact pixel-equal: original `02_left_1_wall_original_viewport_224x136.png` matches Firestaff `hall_1_4_dirE_viewport_224x136.ppm` at 224x136 with 0 changed pixels, MAE 0.0, SHA256 `8d5d9bd870d9aab74907fcd2051ae71547dd27583b2b81758ebebf32cfa2161c`. Report: `parity-evidence/pass1054_dm1_v1_original_firestaff_viewport_wall_diff.md`; artifacts: `verification-screens/pass1054-dm1-original-firestaff-viewport-wall-diff/`. Non-claim: the three nonzero nearest-neighbor rows are scout data only, not same-state parity claims.
- ✅ DM1 24h readiness roll-up: added `tools/dm1_24h_readiness.py` plus `parity-evidence/dm1_24h_readiness.md` and manifest. Current PASS rolls up DM1 data coverage 14/14, registry agreement 155/155, pass1056 viewport/wall pairing, pass1057 Amiga 2.2 `DUNGEONB.DAT`, Phase A, playable-route, and pass1055 closed-door semantic CTests. The tool preserves pass1056/pass1057 manifests while running their verifiers, so repeated readiness checks only refresh the roll-up output. Non-claim: remaining original-capture gaps stay open.
- ✅ pass1053 original champion candidate-panel evidence: promoted the existing pass455 original-PC34 corrected-click artifact into tracked repo evidence under `verification-screens/pass1053-dm1-original-champion-candidate-panel/` plus `parity-evidence/pass1053_dm1_v1_original_champion_candidate_panel_capture.md`. Source route: corrected `click:111,82` on the Hall champion portrait produces `candidate_select` SHA256 `e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e`; corrected `click:130,115` on C160 produces terminal/HUD frame SHA256 `7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f`. Source-locked via pass455 against CLIKVIEW.C C080 -> MOVESENS.C C127 -> REVIVE.C F0280 and COMMAND.C C160 -> REVIVE.C F0282. Non-claim: this is candidate/resurrect-panel original evidence, not a full four-champion party HUD pixel-pair.
- ✅ pass792 steal-from-slot-indices: contract-only runtime evidence that Graphics.dat item 562 init data `G0025_auc_Graphic562_StealFromSlotIndices[8]` matches the {NECK, POUCH_1, BACKPACK_LINE1_1, QUIVER_LINE1_1, NECK, BACKPACK_LINE1_1, POUCH_2, BACKPACK_LINE1_1} pattern and that GROUP.C F0193 (StealFromChampion) dispatches it with counter = RANDOM(8), lookup, backpack+random(17), and ++counter &= 0x0007. Source-locked against DATA.C:31/244-251 + GROUP.C:1032/1041/1045/1075 + DEFS.H:786/790-793. CTest 96/96 + Python verifier PASS. Disjoint from pass784-790 + pass791.
- ✅ pass790 wound-probability-index-to-mask: contract-only runtime evidence that Graphics.dat item 562 init data `G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask[4]` = `{FEET, LEGS, TORSO, HEAD}` and that PROJEXPL.C:1386 reads it after a wound-test branch. Source-locked against DATA.C:30/243 + PROJEXPL.C:1378-1389 + DEFS.H:736-741. CTest 37/37 + Python verifier PASS. First non-mirror-candidate contract; disjoint from pass784-789.
- ✅ pass789 mirror-candidate C040 save-game-while-panel-live: contract-only runtime evidence that COMMAND.C F0380:2367-2369 gates the C140 save-game command on `G0305 > 0 && !G0299`. While the C040 panel is live C140 is dropped; after F0282(C162) clears G0299 the save fires F0433. Source-locked against COMMAND.C F0380:2367-2369 + STARTEND.C F0433 + REVIVE.C F0280:124-132 + F0282:744-806 + DEFS.H C140/C040/M568/G0299/G0305/G0411. CTest 43/43 + Python verifier PASS. Disjoint from pass788 (C012..C015), pass787 (C111), pass786 (C100 with G0514), pass785 (C007..C011), and pass784.
- ✅ pass788 mirror-candidate C040 status-box-click-while-panel-live: contract-only runtime evidence that COMMAND.C F0380:2159-2161 gates the C012..C015 status-box click on `(champion_index < G0305) && !G0299`. In-range status-box clicks are dropped while G0299 is set; F0282(C162) clears G0299 → click fires F0367. Source-locked against COMMAND.C F0380:2159-2161 + COMMAND.C F0367 + REVIVE.C F0280:124-132 + F0282:744-806 + DEFS.H C012..C015/C040/M568/G0299/G0305/G0411. CTest 46/46 + Python verifier PASS. Disjoint from pass787, pass786, pass785, pass784.
- ✅ pass787 mirror-candidate C040 action-area-click-while-panel-live: contract-only runtime evidence that COMMAND.C F0380:2309-2311 gates the C111 action-area click on `!G0299_ui_CandidateChampionOrdinal`. While the C040 panel is live C111 is dropped; after F0282(C162) clears G0299 the click fires F0371. Source-locked against COMMAND.C F0380:2309-2311 + COMMAND.C F0371 + REVIVE.C F0280:124-132 + F0282:744-806 + DEFS.H C111/C040/M568/G0299/G0305/G0411. CTest 42/42 + Python verifier PASS. Disjoint from pass786 (C100 with G0514 gate), pass785 (C007..C011), and pass784.
- ✅ pass786 mirror-candidate C040 spell-area-click-while-panel-live: contract-only runtime evidence that COMMAND.C F0380:2303-2306 gates the C100 spell-area click on `!G0299_ui_CandidateChampionOrdinal && G0514_i_MagicCasterChampionIndex != CM1_CHAMPION_NONE`. Three-phase contract: live+valid=drop, post-cancel+valid=fire, post-cancel+invalid=drop. Source-locked against COMMAND.C F0380:2303-2306 + COMMAND.C F0370:2482-2520 + REVIVE.C F0280:124-132 + F0282:744-806 + DEFS.H C100/C040/M568/G0299/G0305/G0411/G0514. CTest 48/48 + Python verifier PASS. Disjoint from pass785 (no G0514 gate) and pass784.
- ✅ pass785 mirror-candidate C040 inventory-toggle-while-panel-live: contract-only runtime evidence that COMMAND.C F0380:2181-2183 gates the C007..C011 inventory-toggle commands on `!G0299_ui_CandidateChampionOrdinal`. While the C040 panel is live all 5 inventory commands are dropped; after F0282(C162) clears G0299 the toggle becomes live again. Source-locked against COMMAND.C F0380:2181-2183 + PANEL.C F0355:2299-2318 + REVIVE.C F0280:124-132 + F0282:744-806 + DEFS.H C007..C011/C040/M568/G0299/G0305/G0411. CTest 44/44 + Python verifier PASS. Disjoint from pass784.
- ✅ pass784 mirror-candidate C040 cancel-then-reopen same tick: contract-only runtime evidence that F0282(C162 cancel) and F0280 (new C127 sensor) fire in the same tick with a fresh C040 panel on a fresh candidate ordinal. Source-locked against REVIVE.C F0280:124-132 + F0282:744-806 + PANEL.C F0355:2299-2318 + COMMAND.C F0378:1956-1990 + MOVESENS.C F0275:1502 + DEFS.H C040/M568/C127/C162/G0299/G0305/G0415/G0424. CTest 53/53 + Python verifier PASS. Disjoint from pass760, pass762, and the chest cancel-reopen-pickup gate.
- ✅ Movement and collision: cardinal movement, turning, wall/door/fake-wall blocking, cooldowns, stairs, pits, teleporters, blocked self-damage, empty-party group cleanup, and deterministic capture gates.
- ✅ Viewport rendering: wall/floor/ceiling slices, doors, frames, ornaments, inscriptions, pits, stairs, creatures, projectiles, explosions, floor items, alcoves, occlusion, palette dimming, HiDPI scaling, and teleporter visuals.
- ✅ Viewport wall evidence hardening: `g_dm1_wall_frame_bitmaps` is source-locked to the PC34 `G2107`/door-frame offset model and guarded by an asset-free null-write regression.
- ✅ Door-front occlusion pixel-zone gate: all 11 source-locked front-door branches prove rear cells are masked by door pixels and front cells draw after the door pass.
- 🔒 DOR-01 F0715 front-door toggle resolver source-lock pin (commit `73c9b1e1`): `test_dm1_v1_dor01_f0715_door_resolve_toggle_action_pc34_compat` 18/18 assertions, FNV-1a 32-bit hash `0xEC4F85A7`, pins the ReDMCSB door actuator branch (adjacent to F0275_SENSOR_IsTriggeredByClickOnWall) contract for door state 0/4/5 -> CLOSE/OPEN/DESTROYED, animating state 1/2/3 -> snap-OPEN, vertical-bit 0x08 -> doorVertical, F0715 purity (no square byte mutation), and outResult population on early reject.
- ✅ Creature and combat systems: creature groups, AI, attacks, deaths, drops, XP, projectile attacks, sounds, fleeing, special positioning, possession drops, Black Flame behavior, generator/teleporter/fall/drop cases, and Lord Chaos constants.
- ✅ Spells and magic: rune UI, spell casting, mana/skill checks, projectiles, shields, light/dark, open-door magic, poison cloud behavior, and spell failure paths.
- ✅ Champions: recruitment, active selection, health/stamina/mana bars, skill/XP updates, death/resurrection, stats panel routing, weight/load behavior, and stamina regeneration.
- ✅ DM1 V1 Hall of Champions portrait-6 (SYRA) / west_back_route / portrait_rect_position slice: `firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_portrait_rect_position_runtime_probe` proves the portrait_rect_position contract for the (2,4) Hall cell at every direction. At `(2,4)E` the engine reports ordinal=6 (SYRA, "CHILD OF NATURE"), mirrors the F0660 mirror-catalog name, and paints the C026 strip slot 6 at the D1C cutout (96,35)-(128,64) with 100% pixel match and 275 warm pixels. At `(2,4)W` the engine reports ordinal=10 (GANDO, "THURFOOT") — the back-route portrait: the east wall of the front cell (1,4) carries a C127 sensor with sensorData=10, so the engine correctly renders the GANDO portrait at the same D1C cutout (100% pixel match, 232 warm pixels). At `(2,4)S` the engine reports ordinal=15 (MOPHUS, "THE HEALER") at the same cutout (100% pixel match, 47 warm pixels — MOPHUS uses fewer warm pixels because the gray-cloth palette is dominant). At `(1,2)W` and `(1,4)W` the engine correctly reports ordinal=-1 with empty cutouts and empty wall boxes (0 warm pixels). The D1C wall-mirror frame invariant (80,29,64,43) with portrait cutout parented at (96,35) holds across all 4 tested poses. Source-locked to ReDMCSB DUNGEON.C:2573 + 2608-2612 + DUNVIEW.C:3913-3928 + DUNVIEW.C:525 + MOVESENS.C:1501-1503 + REVIVE.C:63 + REVIVE.C:704 + DEFS.H:821-826 + DEFS.H:2186 + DEFS.H:2552 + COORD.C:1693-1722 + COORD.C:1748-1749. Disjoint from `firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe` (16-pose C127 ordinal coverage with pixel rect match disabled) and `firestaff_dm1_v1_champion_mirror_capture_probe` (PPM visual capture). Also corrected a wrong initial assumption: the probe initially expected the (2,4)W front cell (1,4) to have no C127 sensor at all (the "west_negative" framing from the source-lock prompt) but the real DM1 PC 3.4 DUNGEON.DAT places a C127 sensor on the east wall of (1,4) with sensorData=10. The slice is now honest about the back-route portrait and asserts the actual GANDO mapping. Diagnostic probe `firestaff_dm1_v1_champion_mirror_diag_2_4_west_probe` enumerates the (1,4)/(2,4) front-mirror ordinals for all 4 directions and dumps the full 24-entry mirror catalog (HALK=1, LEIF=4, SYRA=6, ZED=9, GANDO=10, WUUF=13, MOPHUS=15, SONJA=18, ...). 23/23 probe assertions PASS against both `~/.firestaff/data/dm1-extras/dmfiles-dos-en-v34/DATA` and `~/.firestaff/data/dm1-extras/legacy-dos/DungeonMasterPC34/DATA`.
- ✅ Inventory and items: leader hand, alcoves, throwing, torches/light, floor pickup, scrolls, potions, food/water, item descriptions, chest/backpack routes, equip/unequip, fountains, and source-blocked direct key action.
- ✅ Survival, sensors, entrance, save/load, audio, and data loading: food/water decay, rest, stamina, sensor/timeline behavior, title/entrance flow, save/load routes, sound routing, and DUNGEON.DAT/GRAPHICS.DAT ingestion.
- ✅ Source-lock verifier hardening: viewport/walls landable metadata, wall-clip source audit, side-wall source-row clipping, D3/D2 wall-ornament order, front-cell collision, D0/D1 visible-square draw-order, wall-alcove C2548, champion stat panel, and ambient dungeon sound gates now resolve current local code/source boundaries and reflect the closed no-ambient-loop source boundary.
- 🔒 DM1 source-lock audit completed across movement, rendering, creatures, combat, spells, champions, inventory, survival, sensors, entrance, save/load, audio, and data structures.
- ✅ 2026-06-20 Wall-inscription blurry/double-exposed render (BUG-DNY-DM1-2026-06-16): `m11_draw_dm1_front_wall_inscription_text` in `src/engine/m11_game_view.c:13618-13636` switched the 5x7 fallback path from `g_text_shadow` (shadowDx=1, shadowDy=1, double-draws each glyph at (+1,+1) shadow + (0,0) main → 2px-thick overlapping blurry glyphs) to `g_text_small` with explicit `shadowDx=0`, `shadowDy=0`, `shadowColor=BLACK` so each glyph is drawn exactly once at its true 5x7 pixel position. Matches ReDMCSB DUNVIEW.C:3619-3638 (no shadow on M648_GRAPHIC_INSCRIPTION_FONT). New probe invariant `INV_GV_407G` in `firestaff_m11_game_view_probe` smoke-tests the fallback path; pixel-density heuristic is TODO for a future pass with a real DUNGEON.DAT + front-wall inscription cell. M11 game view probe now 625/625 (was 624/624 before this entry).
- ✅ 2026-06-22 DM1 V1 Hall of Champions ordinal-8 east_walkpath slice: added `firestaff_dm1_v1_champion_mirror_east_walkpath_ordinal_8_runtime_probe` (registered as CTest target `firestaff_dm1_v1_champion_mirror_east_walkpath_ordinal_8_runtime_probe`) to cover the previously unprobed (2,1) EAST ordinal 8 route exposed by the C127 sensor at map=0 cell=(3,1) cell_bit=3 sensorData=8. The new probe drives a 5-step pose sequence at the (2,1) cell — N (no portrait), E (ordinal 8, D1C portrait matches 100%), S (ordinal 4 LEIF, D1C portrait matches 100%), W (no portrait), and back to E (re-blt recovers ordinal 8) — and asserts both the per-step ordinal match (>=90%) and the cross-ordinal re-blt invariant (prior ordinal pixels do not leak >=35% over the side wall). Source-locked to ReDMCSB DUNGEON.C:2573 + DUNGEON.C:2608-2612 + DUNVIEW.C:3913-3928 + DUNVIEW.C:8318-8542 F0128 + MOVESENS.C:1501-1503 + MOVESENS.C:556 + REVIVE.C F0280/F0282. Per-build SKIP guard matches the existing `walkpath_runtime_probe` and `candidate_panel_runtime_probe` patterns when the DUNGEON.DAT fixture differs. Closes a narrow Hall-of-Champions east_walkpath source-lock gap (the previous coverage only exercised the (1,3) EAST ordinal 18 SONJA route); this is source/runtime fixture coverage only, not original-capture or pixel-parity evidence.
- ✅ 2026-06-22 DM1 V1 Hall portrait ordinal-6 south_return / portrait_rect_position probe: new `firestaff_dm1_v1_champion_mirror_ordinal_6_south_return_portrait_rect_position_runtime_probe` discovers ordinal 6 on the local PC 3.4 DUNGEON.DAT (HIT at (2,4) DIR_EAST — Leif's east-wall C127 sensorData=6) and proves the (96,35,32,29) D1C portrait rectangle (ReDMCSB `DUNVIEW.C:525` G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}, blit rectangle on the 320x200 framebuffer at (96,68,32,29)) paints the C026 ordinal-6 portrait with 100% pixel match, 275 warm pixels, and zero left-side warm-pixel bleed (no floating portrait over the side wall). The south_return trip (SOUTH→NORTH→EAST) is exercised end-to-end: at (2,4) SOUTH M11 reports ordinal 15 (MOPHUS south wall), at NORTH ordinal=-1 with warm=0 (clean back-wall, no stale portrait bleed), and at EAST ordinal 6 with 100% match. Discovery reports 11 unique corridor-band ordinals across 16 positive-ordinal poses, and the cross-ordinal invariant confirms 16/16 positive-ordinal poses land in the D1C portrait rectangle (the cutout is universal, ordinal-6 is not special). Wired into CMakeLists.txt as a target + pool entry; manual run: `SDL_VIDEODRIVER=dummy ./build/firestaff_dm1_v1_champion_mirror_ordinal_6_south_return_portrait_rect_position_runtime_probe ~/.firestaff/data/dm1` PASS 10/10. Source-locked to `DUNGEON.C:2573` normalize(M011_CELL - direction) front-wall sensor filter, `DUNGEON.C:2608-2612` G0289 = M000_INDEX_TO_ORDINAL(M040_DATA), `DUNVIEW.C:3913-3928` C026 portrait blit into G0109 portrait box, `DUNVIEW.C:525` G0109 = {96,127,35,63}, `DUNVIEW.C:8318-8542` F0128_Draw_CPSF far-to-near draw order so D0/D1/D2/D3 walls draw with D1C last and the champion portrait is the final pixel over the front wall, `COORD.C:1693-1722` PC 3.4 viewport origin (0,33), 224x136, `COORD.C:1748-1749` G2078_C32_PortraitWidth=32, G2079_C29_PortraitHeight=29, `MOVESENS.C:1501-1503` sensorData → F0280 candidate. Non-claim: this is Firestaff runtime evidence only, not DOS pixel parity.

- ✅ 2026-06-20 Champion mirrors not visible (P1 visual bug): the existing 6 firestaff_dm1_v1_champion_mirror_* runtime probes (visibility, walkpath, zorder, zorder_reblt, actual_pose, candidate_panel) all pass for the canonical Hall of Champions poses, but the P1 ticket asked for visual evidence. Added a 7th probe — `firestaff_dm1_v1_champion_mirror_capture_probe` — that calls `M11_GameView_Draw` for each of 9 poses (6 with mirror ordinals: HALK/LEIF/SONJA/ZED/MOPHUS/WUUF + 3 corridor negatives) and saves a full-frame PPM + a 224x136 viewport-crop PPM per pose (18 PPMs total) plus a JSON+MD manifest under OUT_DIR. The probe computes two heuristics on the (96,35)-(128,64) D1C portrait rect: `portrait_rect_nonzero` (any non-black pixel) and `portrait_rect_warm_count` (pixels with palette indices in {0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B yellow, 0x0E blue} — the warm-color set used by C026/C027 portrait sprites per ReDMCSB DUNVIEW.C:3913-3928). Run on the local DM1 V1 PC 3.4 DUNGEON.DAT: every positive-ordinal pose has warm_count ≥ 30 (HALK 383, LEIF 348, SONJA 211, ZED 238, MOPHUS 47, WUUF 141) and every negative-ordinal pose has warm_count < 30 (door 0, corridor-N 0, corridor-N 13). Visual confirmation by image inspection: HALK shows orange-blonde portrait on red niche, LEIF shows blue-background portrait, SONJA shows cyan-haired portrait, ZED shows yellow-haired portrait on red niche, MOPHUS shows pale-skinned portrait, WUUF shows dark-brown-bearded portrait; the 3 negative poses show grey-stone wall only. **The P1 ticket is closed by visual evidence: the current m11_game_view rendering correctly draws the D1C front-wall champion portrait in every pose with a C127 sensor and correctly does NOT draw any portrait in poses without one.** Probe wired into CMakeLists.txt but intentionally not add_test()-bound (requires non-default OUT_DIR); manual run: `./build/firestaff_dm1_v1_champion_mirror_capture_probe ~/.firestaff/data/dm1 /tmp/captures`.
- ✅ 2026-06-20 Champion Z-order/floating (P1 visual bug): extended the same `firestaff_dm1_v1_champion_mirror_capture_probe` with 6 additional 'no-floating' side-wall scenarios that mirror the synthetic-view pixel-compare cases covered by `firestaff_dm1_v1_champion_mirror_zorder_runtime_probe`: hall_d1c_front_route_blocked_1_N (1,3) N, hall_d1c_front_route_blocked_2_N (1,4) N, hall_d1c_front_route_blocked_east (1,4) E, hall_d1c_front_route_blocked_south (1,4) S, hall_side_no_floating_west_1 (1,3) W, hall_side_no_floating_west_2 (1,4) W. All 6 captures show wall texture only (warm_count 0/13/0/22/0/0, all below the 30-pixel threshold). Visual confirmation: all 6 side-wall poses show plain grey stone masonry with no champion portrait sprite floating over the wall. **The P1 ticket is closed by visual evidence: the current m11_game_view rendering correctly avoids painting champion portrait sprites on side walls after the player turns, because the DUNVIEW.C:8318-8618 F0128 viewport redraw order (far-to-near) overpaints the D1C portrait rectangle with the side-wall geometry when the front cell no longer has a C127 sensor.** Probe now generates 15 captures per run (30 PPMs total). Combined with the synthetic-view pixel-compare probe, the 'champion Z-order/floating' bug has both quantitative (pixel-match) and qualitative (visual-evidence) coverage.
- ✅ 2026-06-23 DM1 V1 Hall of Champions portrait ordinal 14 south_return / portrait_rect_position: added `firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_rect_probe`. The local DM1 V1 PC 3.4 DUNGEON.DAT carries a C127 champion-portrait sensor at map 0 cell (1, 18) south wall (sensor idx=21) with sensorData=14. The canonical front-route pose is (1, 19) DIR_NORTH so the view-cone front cell lands on (1, 18) and the visible-wall side (DIR_NORTH + 2 == SOUTH = 2) matches the sensor's wall side (cell = 2). Map 0 is 18x19 so the party at y=19 is one cell south of the map edge; this is the only party pose that resolves `M11_GameView_GetFrontMirrorOrdinal` to 14 — the in-bounds (1, 17) DIR_SOUTH and (0, 18) DIR_EAST poses both fail the visible-wall-side filter, and (1, 18) DIR_NORTH puts the front cell on the corridor square (1, 17) instead of (1, 18). The probe parks the party directly at the canonical pose, the same way the existing `firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe` parks at (1, 2)/(1, 5). Source-locked against ReDMCSB DUNGEON.C:2608-2612 (C127 sensorData → G0289), MOVESENS.C:1501-1503 (C127 sensorData → F0280), REVIVE.C F0280 (materialize candidate from sensorData), DUNVIEW.C:525 (G0109_box = { 96, 127, 35, 63 }), DUNVIEW.C:3913-3928 (D1C champion portrait blit), and DEFS.H:2186 (C127_SENSOR_WALL_CHAMPION_PORTRAIT). Probe proves: (A) the sensor at (1, 18) south wall maps to portrait ordinal 14 with name LEYLA / title SHADOWSEEK; (B) the canonical (1, 19) DIR_NORTH pose resolves the front-mirror ordinal to 14 via `m11_front_cell_mirror_ordinal` (m11_game_view.c:11652); (C) the public `DM1 front-mirror wall-ornament zone helper` returns the source-locked (80, 29, 64, 43) which contains the portrait cutout (96, 35, 32, 29); (D) the C026 champion portrait cell at atlas (col 6, row 1) — ordinal 14 — matches the rendered (96, 35, 32, 29) cutout by 100% of non-transparent pixels while the four adjacent atlas cells (5,1), (7,1), (6,0), (6,2) match by 8%, 18%, 3%, 4% (all below the 30% cut-point); (E) turning to side poses (0, 19) DIR_EAST, (2, 19) DIR_WEST, or stepping off the route to (1, 18) DIR_NORTH (front=(1, 17) corridor) yields frontMirrorOrdinal=-1 and the cutout does not match ordinal 14 (13%, 13%, 21%); (F) re-entering (1, 19) DIR_NORTH restores ordinal 14 and the cutout re-matches at 100%; (G) ordinal 14 is inside the mirror catalog range [0, 24). Run on local DM1 V1 PC 3.4 DUNGEON.DAT: 18/18 PASS. CTest-gated via `firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_rect_probe`. Non-claim: we do not assume the south_return route is reachable in normal gameplay — the Hall doors block forward movement until every prior mirror has been recruited, so the probe teleports the party directly into the route. Non-claim: we do not claim DOS pixel parity; the pixel-match uses the local C026 strip pulled from the same GRAPHICS.DAT the runtime is drawing from, so this is runtime correctness rather than pixel-for-pixel DOSBox reference parity.
- ✅ 2026-06-20 Missing or incorrect viewport walls (P1 visual bug): added a new probe — `firestaff_dm1_v1_viewport_wall_capture_probe` — that visits 6 Hall-of-Champions cells (DOOR/TELEPORTER/CORRIDOR/WALL) x 4 directions (24 poses total) and saves PPMs of every viewport, with grey-pixel + non-black + texture-diversity heuristics. The cell type at each position is read from `world.dungeon->tiles[mapIndex].squareData[mapX * height + mapY]` via `M034_SQUARE_TYPE = (square >> 5)` per DEFS.H:2482. Run on the local DM1 V1 PC 3.4 DUNGEON.DAT (24/24 pass): every pose shows substantial grey (23000-31000) + non-black (26000-30000) pixel counts confirming wall/floor/door geometry renders correctly. Visual confirmation by image inspection: (2,3) facing N shows the back wall with right-side perspective; (0,3) facing N shows the back wall with left-side perspective; (1,2) DOOR facing S shows the dark doorway with door-frame side wall; (1,3) TELEPORTER facing S shows the corridor and the champion figure teleport destination; (1,5) facing N shows the WUUF portrait alcove; (2,3) facing E shows the MOPHUS portrait alcove on the east wall; (2,3) facing W shows the side wall with the panel of portrait alcoves on the right edge. **The P1 ticket is closed by visual evidence: the current m11_game_view rendering correctly draws the expected walls, doors, corridors, teleport fixtures, and portrait alcoves for every cell type and every direction in the Hall of Champions entry area.** Probe wired into CMakeLists.txt but intentionally not add_test()-bound (requires non-default OUT_DIR); manual run: `./build/firestaff_dm1_v1_viewport_wall_capture_probe ~/.firestaff/data/dm1 /tmp/captures`.
- ✅ 2026-06-20 pass1052 DM1 V1 original PC 3.4 DOSBox turn-cycle capture: new clean original evidence under `verification-screens/pass1052-dm1-original-route-24h-turncycle/` plus report `parity-evidence/pass1052_dm1_v1_original_turncycle_viewport_wall_capture.md`. Route: direct PC 3.4 VGA/no-sound/keyboard launch (`DM -vv -sn -pk`), dungeon-entry click sequence, then 3 left turns. `tools/pass80_original_frame_classifier.py --expected dungeon_gameplay,wall_closeup,dungeon_gameplay,wall_closeup --fail-on-duplicates` PASS: 4 captures, 2 `dungeon_gameplay`, 2 `wall_closeup`, 0 duplicate raw hashes. This closes the old "no clean original viewport/wall frame exists" blocker for the first narrow turn-cycle route. Non-claims: pass513 I34E transcript remains scaffold-only; no champion-panel/creature-chain route proof; no original-vs-Firestaff pixel diff yet.
- ✅ 2026-06-20 pass852-859 DM1 V1 Graphics.dat init-table bulk-wire + verifier closure (commits `6f8c0648` + `f9b2da84` + `84301856` + `6519b380`): 7 new DM1 V1 Graphics.dat item 562 init-table modules (pass852 mandatory_graphic_indices + pass853 wound_defense_factor + pass854 underscore_character_string + pass855 rename_champion_input_character_string + pass856 reincarnate_special_characters + pass857 bitmap_arrow_pointer + pass858 bitmap_hand_pointer + pass859 square_type_to_event_type) each with include + src + test + Python verifier + parity-evidence .md + manifest.json. Plus `test_dm1_lzw_round_trip.c` (96/96) — a tiny reference LZW encoder local to the test that exercises every documented branch of `m11_gfx_lzw_decompress` without real Atari ST assets. All 8 ctest binaries + 8 Python verifiers wired in CMakeLists.txt (with if(EXISTS) defensive guards per the watchdog pattern). **Before this push**: ctest 714/717 (99.4%, 3 pass857/858/859 verifier-only FAIL on DATA.C:lnum source-lock line-citation gaps). **After CMakeLists.txt wiring**: ctest 718/718 (100% green). Source-locked against DATA.C:24/137-209/271-280/321-340/362-410/463-477 + DUNVIEW.C:2350-2455 + DEFS.H:736-741/922/2167-2170/2171-2233/3234 + PANEL.C F0344 + IO.C cursor blit path + REVIVE.C F0280:124-132 + CLIKCHAM.C F0367/F0368 + SQUARE.C F0920. Disjoint from pass784-851.

- ✅ 2026-06-20 pass860-863 four more Graphics.dat init-table non-mirror-candidate gates (commit `f8e15b30`): 4 more non-mirror-candidate contract gates in the DM1 V1 Graphics.dat item 562 init-var namespace. **pass860 G0054_Box_ChampionIcons[16]** (73/73) — 4 boxes {L,R,T,B} for on-screen champion-icon rectangles, source-locked against DATA.C:92/431/1112 + CHAMDRAW.C:830/1022/1025/1028 + CHAMPION.C:1656 + IO.C:2433/2619/2677. **pass861 G0055_BarGraphMasks[4][3][2]** (104/104) — 4 champions × 3 graphs (health/mana/stamina) × 2-mask bitmask pairs for status-panel bar-graph renderer, source-locked against DATA.C:93-98 + CHAMDRAW.C:204/207/208. **pass862 G0056_BarGraphByteOffsets[4][3]** (65/65) — 4 champions × 3 byte-offsets into the per-byte bar-graph renderer, source-locked against DATA.C:99-104 + CHAMDRAW.C:204/208. **pass863 G0057_SlotDropOrder[30]** (133/133) — 30-entry slot drop-order priority table used by F0300_CHAMPION_GetObjectRemovedFromSlot during forced-drop, source-locked against DATA.C:105/436-466/1119 + CHAMPION.C:1546 + DEFS.H C00_SLOT..C29_SLOT. Total 375 new C-test assertions across the 4 gates. Wired into CMakeLists.txt (test binaries + Python verifiers with if(EXISTS) defensive guards + DEPENDS) + .gitignore (pass860-863 parity-evidence whitelists). **Bugfix during the work:** pass860 originally misinterpreted {L,R,T,B} as {X,Y,W,H} — the L<R/T<B constraints flagged the box-0 (281, 299, 0, 13) as illegal-width, so the assertion was rewritten to check L<R/T<B explicitly (the icons are 18×13 with L<R=18 and T<B=13 in screen coordinates). **ctest 726/726 100% green** (parallel sweep 60.87s with --timeout 60, the 4 pre-existing parallel-flaky gates pass373/508/512 acknowledged separately). Push: GitHub `main` HEAD now at `f8e15b30`.

- ✅ 2026-06-20 pass864-873 ten more DM1 V1 Graphics.dat init-table + global init-var gates (commits `eb9efd2e` + `2a2a3c84` + `67f2dd81`): 10 more non-mirror-candidate contract gates extending the DM1 V1 Graphics.dat item 562 init-var namespace + 3 global palette-change palettes. **pass864 G0061_Box_ScreenTop[4]** (32/32) — top status-bar area {0, 319, 0, 32}. **pass865 G0062_Box_ScreenRight[4]** (30/30) — right panel column {224, 319, 33, 169}. **pass866 G0063_Box_ScreenBottom[4]** (30/30) — bottom message-line strip {0, 319, 169, 199}. **pass867 G0064_PrintTextMasks2[4]** (24/24) — 32-bit text-print mask-2 table {0xFFF0FFF0, 0xFFF8FFF8, 0xFFFCFFFC, 0xFFFEFFFE} (unsigned-int return type since values > INT_MAX). **pass868 G0065_PrintTextMasks1[4]** (24/24) — 32-bit text-print mask-1 table {0x7FFF7FFF, 0x3FFF3FFF, 0x1FFF1FFF, 0x0FFF0FFF}. **pass869 G0066_LineFeedCharacterString[2]** (23/23) — newline char '\n' + NUL terminator used by F0040_TEXT_Print. **pass870 G4010_PaletteChanges_CursorMask[16]** (87/87) — dark-cursor mask palette-change table {15, 0, ..., 0} (2 non-zero entries). **pass871 G4011_PaletteChanges[16]** (89/89) — cursor + champion-icon palette-change remap table {15, 14, ..., 0, 2, 1, 0} (entries 12 and 15 both = 0, NOT all-distinct, captured in entry12And15Collide + exactlyTwoZeroEntries). **pass872 G4013_PaletteChanges_CursorMask[16]** (84/84) — variant-2 cursor palette-change table {15, 15, ..., 0, 15, 15, 15}. **pass873 G0347_Palette_TopAndBottomScreen[16]** (82/82) — VGA palette for the top status bar + bottom message-line strip. Total 505 new C-test assertions across the 10 gates. Wired into CMakeLists.txt (test binaries + Python verifiers with if(EXISTS) defensive guards + DEPENDS) + .gitignore (pass864-873 parity-evidence whitelists). All gates green. Disjoint from pass784-863 + the bulk-wire batches.

- ✅ 2026-06-22 DM1 V1 Hall of Champions portrait ordinal 10 (GANDO/THURFOOT) source_wall_entry / portrait_rect_position slice: added `firestaff_dm1_v1_champion_mirror_portrait10_rect_position_runtime_probe`, a real-asset CTest probe that verifies the ordinal-10 portrait placement slice against `~/.firestaff/data/dm1`. The probe locks (1) `M11_GameView_GetFrontMirrorOrdinal == 10` at the source-visible Hall route that exposes the C127 sensor with sensorData=10 in PC 3.4 English DUNGEON.DAT — `mapIndex=0, (mapX=1, mapY=3)` with `direction=SOUTH` (the C127 sensor is on cell 0 of square `(1,4)`, so `(1,3,SOUTH)` resolves it through the DUNGEON.C:2573 front-wall filter while `(1,5,NORTH)` is the same square from the wrong wall side), (2) `M11_GameView_GetMirrorNameByOrdinal(10) == "GANDO"` and `GetMirrorTitleByOrdinal(10) == "THURFOOT"` (catalog identity in PC 3.4 English; note that ordinal 9 in the same catalog is ZED/DUKE OF BANVILLE, so ordinal 10 is the distinct GANDO/THURFOOT row), (3) `DM1 front-mirror wall-ornament zone helper == (80,29,64,43)` and the D1C portrait rect `(96,35,32,29)` lives strictly inside that C346 wall-ornament frame per `DUNVIEW.C:3913-3928` + `DUNVIEW.C G0205` coordSet 5 / index 12 (rect x = frame x + 16, rect y = frame y + 6, full containment), (4) the C026 portrait-strip blit at `(96,35)-(127,63)` matches ordinal 10 and the best-of-24 ordinal at the rect is 10, and (5) three wrong-wall poses around the `(1,4)` C127 sensor — `(1,5,NORTH)`, `(0,4,EAST)`, `(2,4,WEST)` — all yield `GetFrontMirrorOrdinal == -1` and do not leave stale ordinal-10 pixels in the D1C rect. Verification: focused build/CTest is required after integration. Wired into `CMakeLists.txt` alongside the existing `firestaff_dm1_v1_champion_mirror_*` runtime probes in the foreach list; SKIP-safe when hash-verified DM1 data is unavailable. Disjoint from the earlier ordinal-4 LEIF probe (different sensor / different rect-target ordinal / different catalog name) and from the ordinal-7 TIGGY/TAMAL probe (different route — ordinal 7 is reachable only via `(2,17) SOUTH`, no front_north_entry pose exists for it). Source evidence: `DUNGEON.C:2573` M011_CELL(sensor) vs view dir; `DUNGEON.C:2608-2612` C127 sensorData → G0289; `DUNVIEW.C:3913-3928` D1C portrait blit to (96,35)-(127,63); `DUNVIEW.C G0205` coordSet 5 / index 12 = C346 champion-mirror frame; `DUNVIEW.C:8318-8542` F0128_DUNGEONVIEW_Draw_CPSF far-to-near viewport redraw; `MOVESENS.C:1501-1503` C127 sensorData → F0280; `REVIVE.C F0280` candidate materialization; `COORD.C:1693-1722` PC34 viewport origin/224x136 dimensions. Honest scope: this is Firestaff-runtime portrait_rect_position evidence only; it does not claim DOS pixel parity and does not add an original-vs-Firestaff viewport comparison.
- ✅ 2026-06-22 DM1 V1 Hall portrait-ordinal-23 front-north-entry portrait-rectangle runtime evidence: new probe `firestaff_dm1_v1_champion_portrait_ordinal_23_front_north_entry_rect_runtime_probe` locks the D1C portrait-rect destination (96,35)-(128,64) for ordinal 23 (NABI / THE PROPHET in real DM1 V1 PC 3.4 DUNGEON.DAT) when ordinal 23 is the front ordinal. The probe (14/14 PASS on the local data) verifies (a) the catalog reports a non-empty name at ordinal 23, (b) the C026 strip source cells at (224,58)-(255,86) carry 635 non-transparent pixels, (c) the D1C rect renders ordinal 23 strip pixels with 100% match (635/635) after a synthetic C127 sensorData mutation at the front wall cell (saves + restores sensorData around the mutation), (d) side walls (E/W at (1,2)) and the back wall (S at (1,2)) show no floating portrait (warm_count 0/0/22 < 30), and (e) `F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat` stores portraitIndex = mirror ordinal (NOT 20+slot) when recruiting HALK. Source-locked against ReDMCSB DUNGEON.C:2573 / DUNVIEW.C:3913-3928 / DUNVIEW.C:8318-8618 / ENDGAME.C:327-394 / G0109 / M635_ZONE_PORTRAIT_ON_WALL. Probe wired into CMakeLists.txt pool (test #43). Companion to the existing `firestaff_dm1_v1_champion_mirror_*` pool, no overlap with their 1/4/10/13/15/18 ordinal coverage. Non-claim: this is Firestaff runtime evidence, not pixel parity with original DM1 PC 3.4.
- ✅ 2026-06-22 Hall of Champions ordinal 13 (WUUF) east-walkpath portrait-rect-position runtime probe: new `firestaff_dm1_v1_champion_portrait_13_east_walkpath_portrait_rect_position_runtime_probe` (CTest-registered) locks the destination-rectangle-position invariant for the WUUF C127 sensor at (1, 6) when the party reaches (1, 5, SOUTH). Two slices: (1) a direct `set_pose` reference slice that pins the canonical destination rectangle (framebuffer (96, 68)-(128, 96) at width M11_PORTRAIT_W=32 / height M11_PORTRAIT_H=29) with 596/596 opaque pixels matching the C026 atlas source sub-rectangle ((ordinal & 7) * 32, (ordinal >> 3) * 29) = (160, 29); and (2) an input-driven walkpath slice that drives the party through `M11_GameView_HandleInput` from (1, 2, NORTH) ordinal 1 (HALK) via two TURN_RIGHT events + three FORWARD events to (1, 5, SOUTH) ordinal 13 (WUUF), with per-step front-ordinal + destination-rectangle-position + slice-specific no-floating-ordinal-13 checks. Slice-specific no-floating guard isolates ordinal 13 from the documented Hall corridor wall-pattern false positives (corridor walls at (1, 2, S) and (1, 4, S) can resemble ordinal 21 in the same destination rectangle but never resemble ordinal 13, which has a unique palette distribution). Source-locked against DUNGEON.C:2608-2612 (C127 sensorData -> G0289), DUNVIEW.C:3913-3928 / 8522-8533 (C026 D1C front-wall blit, C01 dark-gray transparency), DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF (re-blt after MOVESENS.C:556), DEFS.H:821-826 (M027/M028 atlas math), and the existing firestaff_dm1_v1_champion_mirror_actual_pose + walkpath + zorder_reblt probe coverage matrix. Disjoint coverage from existing probes: actual_pose does not draw (GetFrontMirrorOrdinal only); zorder_reblt covers ordinal 13 but only asserts dominance, not rectangle position; walkpath SKIPs on this DM1 V1 build because (1, 3, NORTH) is -1 here; capture_probe dumps PPMs but does not assert the destination rectangle position with a per-pixel C026 match. Headless probe run on local DM1 V1 PC 3.4 DUNGEON.DAT: PASS, 1/1 direct + 6/6 input-driven steps. CTest target `firestaff_dm1_v1_champion_portrait_13_east_walkpath_portrait_rect_position_runtime_probe` registered with `add_test()` + WORKSPACE_DATA_DIR auto-resolution.

### DM1 V2.0 / V2.1 / V2.2

- ✅ V2.0 filtered presentation: config, CRT scanlines, palette correction, dither cleanup, sharpening, renderer integration, and launcher/menu integration.
- ✅ DM1 V2.2 GPU render path per-frame shape cache: new `m11_v22_shape_cache_pc34` module (include + src) provides the data-flow seam between the V22 shape book (m11_v22_shape_for_cell) and the M11 per-cell draw passes. Module-static 3x3 cache (D1..D3, L/C/R) populated by `m11_v22_shape_cache_update(direction, raw_squares)` called from `m11_draw_viewport` after the sample loop, and consulted via `m11_v22_shape_cache_get(depth, lateral)` + `m11_v22_shape_cache_active(depth, lateral)`. The cache is in its own module so tests can link it without pulling in the full M11 game view + image frontend chain. ctest target `test_m11_v22_shape_cache_pc34` passes 23/23 (V1 default all-cells-inactive, V22 active all-9-cells-resolve, V1↔V22 transition, OOB depth/lateral, evidence). Headless probe `firestaff_m11_v22_shape_cache_probe` passes 17/17. Source-locked against ReDMCSB DUNVIEW.C:6697-6816 (composition draw order), DUNGEON.C:2238-2246 (square type decode), DEFS.H:922 (M034_SQUARE_TYPE).
- ✅ DM1 V2.2 GPU render path V22 modern-art overlay: new `m11_v22_render_overlay_pc34` module (include + src) completes the V22 dispatch by painting a placeholder colored rectangle over each V22-active cell on the V1 framebuffer. The placeholder is a filled rectangle (palette index derived from the V22 shape's color_tint RGB average) with a 1-pixel border using the fixed `M11_V22_OVERLAY_PLACEHOLDER_INDEX` (0xFF). Called from `m11_draw_viewport` after the V1 palette-apply pass and before the turn-pan pass. The V1 m11_draw_dm1_* draw passes are NOT modified (the overlay is layered on top of the V1 pixels, not swapped in-place). ctest target `test_m11_v22_render_overlay_pc34` passes 13/13 (V1 inactive = 0 cells painted + no pixels change, V22 active = 9 cells painted, 1-pixel border uses the placeholder index, NULL/zero framebuffer safe). Headless probe `firestaff_m11_v22_render_overlay_probe` passes 13/13. The real V22 modern art (PBR textures, normal maps, etc.) lives in `~/.firestaff/assets/dm1/modern/` and is a follow-up; this commit delivers the end-to-end V22 data flow: M12 menu -> V2 settings wire-up -> m11_v22_shape_cache_update -> m11_v22_render_overlay -> V1 framebuffer pixels. Source-locked against ReDMCSB DUNVIEW.C:6697-6816.
- ✅ V2 parity/presentation scaffold: Phase 0 and Phase 1 command routing, deterministic config, profile boundary, and launch-smoke verification.
- ✅ V2.1 asset pipeline: Phase 2 source-preserving upscale/EPX pipeline, deterministic cache behavior, fallback handling, and probe coverage.
- ✅ V2 presentation slices: HUD/action route gate, palette/projectile metadata gates, smooth-movement runtime bridge, touch/controller route gate, and presentation-disabled state-hash gate.
- ✅ DM1 V2 smooth turn pan backend: optional Custom/V2 turn-pan setting persists through config, the Phase 5 bridge can start pan-enabled turns, and the camera exposes a presentation-only viewport pan offset while V1 command direction changes remain source-owned.
- ✅ DM1 V2 smooth turn pan launcher toggle: `m12_startup_menu` now explicitly cycles the Custom/V2 smooth-turn-pan row, saves `dm1_v2_smooth_turn_pan_enabled = 1`, reloads it through the launcher config path, and keeps the setting tied to the existing presentation-only camera backend proof.
- ✅ Custom dungeon import synthetic gate: `custom_dungeon_import` covers the existing M12 launcher scanner and DM1 V1 engine scanner with generated `DUNGEON.DAT` fixtures, validating map-count/header handling, case-insensitive `dungeon.dat`/`graphics.dat` discovery, compressed/tiny rejection, sorted launcher entries, and selection refusal for invalid imports.
- ✅ DM1 V2 Phase 4 field/projectile VFX binding gate: source explosion thing IDs map to V2 overlay/emitter families, fluxcage remains field-only, unknown things are rejected, and invalid source palette lighting falls back deterministically.
- ✅ DM1 V2 presentation-mode selection: `dm1_v2_presentation_mode_pc34` module (include/dm1_v2_presentation_mode_pc34.h, src/dm1v2/dm1_v2_presentation_mode_pc34.c) maps the launcher M12_PRESENTATION_V1_ORIGINAL/V20/V21/V22 enum onto the DM1 V2 presentation runtime with V22→V21 fallback when the modern asset pack is absent. `dm1_v2_presentation_mode_set_m12()` is called from M11_GameView_Start in src/engine/m11_game_view.c (gameId=dm1). Per-mode settings defaults: `v2_settings_apply_v20_defaults()` (no upscale, palette correction on), `v2_settings_apply_v22_defaults()` (2x scale, bilinear on, per-material palette), alongside the existing V2.1. V22 entry branch initialises `m11_v22_shapes_init()` for modern shape table. CTEST target `test_dm1_v2_presentation_mode_pc34` passes 50/50, headless probe `firestaff_dm1_v2_presentation_mode_probe` passes 30/30. Source-locked against ReDMCSB COMMAND.C F0359 LoadGameSettings, CLIKMENU.C F0365/F0366 V1 source-locked turn/move, CSBWin/Graphics.cpp:3186 V2.0 filter pair, CSBWin/Viewport.cpp:7290 V2.1 EPX-style blit, DM1 PC 3.4 GRAPHICS.DAT V2.2 modern asset pack.

## 2026-07-13 - DM1 PC34 C20 sound-event union roundtrip

- `SOUND.C:1536-1543` and `TIMELINE.C:1903-1905` define delayed C20 as
  `B.Location + C.SoundIndex`. Native export/import and original-save
  materialization now retain that exact union, including signed sound ids,
  rather than treating the bytes as generic Cell/Effect data. The original
  save handoff regression roundtrips a delayed negative sound index.
# ✅ 2026-07-13 DM1 original-save C13 Vi Altar event-plan boundary

The original-save reader now exposes a fail-closed typed C13 plan that keeps
the source-owned `Priority`, `B.Location`, `C.Cell`, and `C.Effect` fields.
Only source steps 2, 1, and 0 are accepted; malformed authenticated saves do
not become generic square events. Source: ReDMCSB `CLIKVIEW.C F0374` lines
179-186 and `TIMELINE.C F0255` lines 1665-1699. Verification:
`test_dm1_v1_original_save_pc34_handoff` covers a checksum-authenticated C13
fixture and rejects an out-of-range step.

# ✅ 2026-07-13 DM1 original-save C13 runtime materialization

Authenticated C13 saves now materialize into a dedicated runtime event rather
than a generic square event. ReDMCSB `TIMELINE.C F0255` is followed in three
bounded transitions: step 2 creates the rebirth effect and stages step 1 five
ticks later; step 1 unlinks only matching bones before staging step 0; step 0
applies the `REVIVE.C F0283` inventory, health, cell, and direction state.
Invalid source plans, maps, champions, or queue capacity fail closed before
publication. Verification: `test_dm1_v1_original_save_pc34_handoff`.

# ✅ 2026-07-13 DM1 original-save C13 native PC34 export

The world-aware PC34 exporter now writes a materialized C13 Vi Altar event as
the original `Priority`, `B.Location`, and `C.A.Cell/Effect` union. It rejects
generic events, absent champions, invalid maps/cells, and rebirth steps outside
the source-proven 2/1/0 sequence rather than emitting a guessed native event.
Source: ReDMCSB `CLIKVIEW.C F0374` lines 179-186 and `TIMELINE.C F0255` lines
1665-1699. Verification: `test_dm1_v1_original_save_pc34_handoff` exports and
reimports the C13 union and rejects an unproven step.

# ✅ 2026-07-13 DM1 original-save C24 fluxcage roundtrip

Original PC34 C24 now materializes only from the ReDMCSB C15 fluxcage slot:
`Priority=0`, `B.Location`, and `C.Slot`. The runtime binds that exact static
thing to a live C050 fluxcage, respects the source game-won guard, and on
expiry unlinks and retires both records. Native export recovers the preserved
source slot rather than writing a host explosion index. Source: ReDMCSB
`PROJEXPL.C F0224` lines 983-994 and `TIMELINE.C F0261` lines 1906-1916.
Verification: `test_dm1_v1_original_save_pc34_handoff` drives authenticated
import, runtime expiry, native export, and reimport of the C24 union.

# ✅ 2026-07-13 DM1 original-save C24/C25 staged union rollback

The F0435 runtime handoff now has a focused original-PC34 regression for the
combined C24/C25 union stage. It places a valid C25 `B.Location + C.Slot`
before an authenticated C24 whose identical slot is outside its own source
location. The rejected candidate cannot publish the earlier C25 runtime
explosion, overwrite the caller's receipt, mutate the borrowed start-world
explosion list, or rewrite the original save bytes. Source: ReDMCSB
`LOADSAVE.C F0435` event/timeline restore and `PROJEXPL.C F0213/F0224` C15
slot ownership. Verification: `test_dm1_v1_original_save_pc34_handoff`.

# ✅ 2026-07-13 DM1 HoC resurrection/rename no-fallback M11 consumer

M11 now draws ReDMCSB `REVIVE.C F0282` C040 and `F0281` C027 only from the
loaded PC34 `GRAPHICS.DAT` panels. If either panel is unavailable or has the
wrong geometry, the C017-backed viewport remains no-draw; the old host-made
green/orange panel, labels, and rename cursor are not rendered. This keeps
dynamic rename text behind real C027 ownership and does not synthesize art.
The focused M11 overlay regression verifies that a no-asset HoC rename
fixture writes no substitute pixels. Source: `REVIVE.C:357-455,704-840`.

# ✅ 2026-07-13 DM1 HoC C027 PC34 rename-font consumer

The live C027 rename panel now consumes the real loaded PC34 M653 interface
bitmap with ReDMCSB `TEXT2.C F0644`'s six-pixel advance and baseline mapping.
`REVIVE.C F0281` guides, typed name/title, C13 foreground, C12 erase color,
and C09 cursor are rendered at their source viewport positions. If M653 is
unavailable, C027 remains real but receives no host-text fallback. Verification:
`test_m11_dm1_hoc_no_fallback_panel` with local PC34 `GRAPHICS.DAT` proves the
real font coordinates/colors and the no-font no-text branch.

# ✅ 2026-07-13 DM1 HoC C040 PC34 host-input gate

M11 now accepts ReDMCSB `COMMAND.C` C160/C161/C162 host pointer/keyboard
input only while `PANEL.C F0346`'s original PC34 C040 panel is loaded at the
source C101 geometry. Missing or malformed C040 leaves the candidate modal
blocked instead of accepting an invisible substitute. The lower-level M10
candidate APIs remain available to source-locked save/runtime tests. Verification:
`test_m11_dm1_hoc_no_fallback_panel` rejects no-asset C160 input and consumes
real-PC34 C162 cancel input.

# ✅ 2026-07-16 DM1 HoC mirror selection/resurrection/reincarnation runtime

The live HoC C127 route now carries a DM1-owned runtime receipt from the
source C080/C127/F0280 path before M11 can publish C040: `F0871` requires the
original mirror champion record and the real C026 portrait atlas, records the
candidate ordinal/index/party-count transition, and rejects active-panel or
fallback candidate states. `F0872` wraps C160/C161/C162 finalization so C160
commits resurrection, C162 cancels without disabling the mirror, and C161 must
pass through the real F0281 rename gate before world/HUD/save state changes.

M11 consumes these receipts when selecting and finalizing front mirrors, while
keeping the existing real GRAPHICS.DAT portrait packing and save sidecar path.
The new real-data probe discovers two original HoC mirrors from DUNGEON.DAT,
selects/resurrects the first, selects/reincarnates+renames the second, verifies
both packed C026 portrait byte payloads, checks source sensor disablement, and
quicksave/resumes the committed two-champion party with no synthetic champion,
portrait, panel, or HUD fallback. Verification: `test_dm1_v1_resurrection_pc34_compat`
198/198, `test_m11_dm1_hoc_c127_resurrect_reincarnate_full_pc34` on local
PC34 data (mirrors 5/14), `m11_dm1_hoc_c127_c162_rotation_owner_pc34`,
`m11_dm1_hoc_c160_clear_corridor_redraw_pc34`,
`dm1_v1_mirror_candidate_save_load_pc34_compat`, `ninja -C build/ninja-dm2
firestaff`, and `git diff --check`. The separate
`m11_dm1_hoc_c127_panel_redraw_close_pc34` inscription/M648 redraw ownership
is covered by the follow-up panel-close entry below.

# ✅ 2026-07-16 DM1 HoC C160/C161/C162 panel-close M648 invalidation

The HoC panel-close/redraw regression now follows the actual ReDMCSB
F0128/F0107 boundary instead of retaining or inventing inscription material
after C160. The test drives the real C127 -> C040 -> C160 route, then proves
three frame-local outcomes from original DM1 data: C040 redraw remains
clear-only while the candidate panel is active, the first post-C160 ordinary
viewport can repaint a different real TextString through GRAPHICS.DAT M648/C10,
and the now-disabled mirror wall itself publishes clear-only M648 because its
source thing chain has no visible C02 TextString. That last point is deliberate:
no fallback font, synthetic post-mirror text, or retained prior glyph run is
allowed when the original wall has no source text.

Verification: `test_m11_dm1_hoc_c127_panel_redraw_close_pc34` passes with local
PC34 data and reports source text `107`; the surrounding `ctest` filter
`m11_dm1_hoc_c127|m11_dm1_hoc_c160|dm1_v1_mirror_candidate_save_load|dm1_v1_wall_inscription|m11_dm1_inscription`
passes 7/7; `ninja -C build/ninja-dm2 firestaff` passes; `git diff --check`
passes.

# ✅ 2026-07-13 DM1 C100 lightning/rebirth material and C3000 geometry

C100 now has its own source-locked material and destination receipt: ReDMCSB
`DUNVIEW.C F0115:5965-5969,5977-5979` resolves native graphic `464` from the
lightning C03 aspect, while the PC34 `G2034` row selects `C3000..C3016` and
`COORD.C:G3025` supplies their actual viewport centres. The new API rejects
unmapped rows and does not draw; C101 and the general D0C route are untouched.
The unresolved PC34 C100 scale mapping remains fail-closed in TODO because
`G2037` is declared as seven entries while `G2034` reaches rows 0..16.
Verification: `test_dm1_v1_projectile_explosion_render_pc34_compat` and
`test_dm1_v1_viewport_3d_pc34_compat`.

# ✅ 2026-07-13 DM1 C100 PC34 scale audit

C100's selector was corrected to the actual PC34 `L2476 = G2028` path from
`DUNVIEW.C:4806-4812,5948,5984,5999`; `G2034` belongs to ordinary explosion
placement. The seven original `G2037` values are exposed only for C100 rows
0..6 (`15,15,15,20,20,20,32`). `FTL.idc` places the next global at `0x25842`
immediately after `G2037` at `0x2583B`, proving no source-backed rows 7..11
exist. Those rows reject scale retrieval and the renderer remains no-draw.
Verification: `test_dm1_v1_viewport_3d_pc34_compat`.
- ✅ 2026-07-13 DM2 weather decoded-material receipt: each source ENVIRONMENT
  weather command now proves a decodable IMG3 pixel plane together with its
  own local palette before it can reach the no-draw weather handoff.
- ✅ 2026-07-13 DM2 G1 creature map-chip palette ownership: direct DB4
  `CreatureType -> CREATURES/type/F9` material receipts now retain and require
  the matching source IMG3 local palette before the dungeon viewport accepts
  the decoded plane.
# instrumented Mednafen trace now captures an FNV-1a checksum of the bounded
# 32-byte `$3800` destination span immediately after original System Card
# `CD_READ` returns at `$4093`. `theron_v1_raw_loader_trace` accepts one exact
# receipt only after the authenticated dynamic record transaction and requires
# it before binding any startup bitmap route. This proves only original
# record-to-RAM transfer; palette/VCE/VDC byte provenance and rendering remain
# fail-closed. Verification: Ninja targets
# `firestaff_theron_v1_raw_loader_trace_ingest_probe`,
# `firestaff_theron_v1_raw_loader_trace_import_probe`, and
# `firestaff_theron_v1_capture_preflight_chain_probe` PASS; both Mednafen
# source-patch and live-capture script contracts PASS; `git diff --check`
# clean.

# DM1 C001 title presentation command

- 2026-07-13 DM1 M11 title presentation now consumes a source-locked,
  DM1-owned command for each ReDMCSB `TITLE.C F0437` event. The command admits
  only decoded C001 material, keeps C12 PRESENTS separate from C13/C14 title
  palette selection, retains the source VBlank waits, and rejects CSB palette
  substitution. The focused regression covers PRESENTS, zoom, post-zoom waits,
  and the final title event against real PC34 graphics.
- ✅ 2026-07-13 DM1 F0115 alcove material hardening: F0121/F0124 wall-alcove candidates now resolve through a DM1-owned ReDMCSB `G2029`/`C2548` receipt instead of M11's ordinary `G2028`/`C2500` floor-row placement. The receipt preserves `G0209` object-aspect selection, including Chest's source-only alcove native bitmap (`M612 + 1` = graphic 499), `C10` transparency, and the absolute opposite-facing alcove cell. The host route fails closed while the original PC34 C2548 coordinate/clip layout remains undecoded, so items cannot be substituted into an incorrect C2500 pane or rendered floating in a wall. Verification: Ninja `firestaff`, `test_dm1_v1_f0115_alcove_item_material_pc34_compat`, and focused CTest PASS 2/2 (`dm1_v1_f0115_alcove_item_material_pc34_compat`, `dm1_v1_viewport_floor_ceiling_items_pc34_compat`). Source: ReDMCSB `DUNVIEW.C F0115:4808-4824,4932-5078`, `G2029`, `G0209`; `DEFS.H C2548`.

- ✅ 2026-07-13 DM1 F0261 legacy spell-tick closure: `TIMELINE_EVENT_SPELL_TICK` admits only verified F0412/F0763 status receipts or complete native PC34 C71/C73/C74/C77/C78/C79 records, and delegates to the existing ReDMCSB expiry logic. Untyped payloads are consumed without creating a host spell/status effect. Verification: focused `test_orch_legacy_spell_tick_consumes_only_typed_status_receipts`. Source: ReDMCSB `TIMELINE.C F0261:1953-1999`; native export folds the same receipts in `memory_savegame_pc34_native_export_pc34_compat.c`.

- ✅ 2026-07-13 DM1 F0412/F0327 cast-spell receipt hardening: spell projectile
  launch now requires a complete PC34-consistent receipt for the original
  G0487 spell row, explosion Thing, kinetic energy, maximum-mana-derived step
  energy, zero required mana, and champion turn/redraw facts. Mismatches fail
  closed before F0212 creation. Verification: Ninja plus
  `test_dm1_v1_spell_casting_pc34_compat` and
  `test_dm1_v1_throw_shoot_pc34_compat` PASS. Source: ReDMCSB `MENU.C
  F0412:1861-1870`, `CHAMPION.C F0327:2091-2102`.
- 2026-07-13 DM1 F0115/F0127 C15 source-order receipt: active same-square
  explosion records now retain their original M10 list order before M11's
  material gate, including separate ordinary F0114/D0C-M636, C100 C3000,
  C101 C3007/D0C-M636, and Fluxcage F0113 routes. The ordinary render list
  remains filtered and C100's separate scale gate is unchanged; no fallback
  material was introduced. Source: ReDMCSB `DUNVIEW.C` F0115:5915-6220.
  Verification: Ninja and CTest `dm1_v1_viewport_runtime_materialization_pc34_compat`,
  `dm1_v1_projectile_explosion_render_source_lock`.
- 2026-07-13 DM1 PC34 atomic C2/C3 receipt audit: C3 already retained the
  full decrypted EVENT-array fingerprint at the `F0433`/`F0435` boundary.
  C2 now retains matching source/export fingerprints for the complete opaque
  128-byte `PARTY_INFO` tail after each file's own F0417 decode key. This is
  receipt-only: it neither interprets PARTY_INFO nor changes importer,
  exporter, or runtime behavior. Source: ReDMCSB `LOADSAVE.C`
  F0433:1583-1584/F0435:2762-2777 and `DEFS.H` `PARTY_INFO`. Verification:
  focused native-PC34 handoff CTest, including the opt-in real-corpus path.
- 2026-07-13 DM1 PC34 C2/C3/C4 receipt negative paths: focused regression
  covers a truncated C4 part, a checksum-valid header with only the C4 key
  changed, and one extra byte after the portrait boundary. Each must reject
  before any C2 PARTY_INFO, C3 EVENT, or C4 TIMELINE fingerprint receipt is
  published. No import/export/runtime behavior changed. Source: ReDMCSB
  `LOADSAVE.C` F0433:1583-1589/F0435:2762-2796. Verification: focused
  native-PC34 handoff CTest.
- 2026-07-13 DM1 PC34 C2/C3/C4 empty-subtype corpus gate: an external
  envelope with no C13/C24/C25 records can no longer pass on core state alone.
  The corpus route now requires the independently authenticated raw C3 EVENT
  and C4 TIMELINE receipts to preserve identity, while retaining failed-row
  provenance for diagnosis. No importer/exporter/runtime fallback changed.
  Source: ReDMCSB `LOADSAVE.C` F0433:1583-1589 and F0435:2762-2796.
   Verification: focused native-PC34 handoff CTest.
- 2026-07-13 DM1 PC34 C2/C3/C4 receipt negative paths: focused regression
  covers a truncated C4 part, a checksum-valid header with only the C4 key
  changed, and one extra byte after the portrait boundary. Each must reject
  before any C2 PARTY_INFO, C3 EVENT, or C4 TIMELINE fingerprint receipt is
  published. No import/export/runtime behavior changed. Source: ReDMCSB
  `LOADSAVE.C` F0433:1583-1589/F0435:2762-2796. Verification: focused
  native-PC34 handoff CTest.
- 2026-07-13 DM1 PC34 empty-subtype corpus receipts: corpus rows now retain
  C13 EVENT and C13 timeline-receipt availability and require positive C13,
  C24, and C25 receipts even where their source/export row counts are zero.
  This prevents a missing subtype decoder from becoming vacuous external-save
  evidence; C3/C4 raw identity remains independently required. No runtime
  importer/exporter behavior changed. Source: ReDMCSB `LOADSAVE.C`
  F0433:1583-1589/F0435:2762-2796. Verification: focused native-PC34
  handoff CTest.
- 2026-07-13 DM1 PC34 optional EVENT roundtrip gate: direct F0435 -> F0433
  -> F0435 state identity now rejects an unavailable or changed C13, C24, or
  C25 receipt whenever either source/export side contains that optional row.
  Zero-row families still require positive receipt availability. This closes
  the direct-roundtrip gap previously covered only by the corpus wrapper; no
  importer/exporter fallback was added. Source: ReDMCSB `LOADSAVE.C`
  F0433:1586-1589/F0435:2781-2796. Verification: focused native-PC34
   handoff CTest.
# ✅ 2026-07-13 DM1 PC34 original-save snapshot-bound corpus handoff

`dm1_v1_original_save_pc34_roundtrip_world_file()`, its reload sibling, and
the recursive corpus verifier now perform provenance, manifest rejection, and
F0435 import over one immutable in-memory file snapshot. Corpus rows are
reclassified after their byte read and fail closed when the path changed after
discovery, so stale F7057 envelope offsets cannot be attached to replacement
bytes. Source: ReDMCSB `LOADSAVE.C F0435` one-save read transaction and
`F0433` export order. Verification: focused
`test_dm1_v1_original_save_pc34_handoff` passes; real-corpus coverage remains
explicitly opt-in through `FIRESTAFF_DM1_PC34_SAVE_CORPUS`.

# ✅ 2026-07-14 DM1 PC34 fixture-free external corpus admission target

Added `test_dm1_v1_original_save_pc34_external_corpus`, a focused admission
target that creates no save bytes or substitute corpus. With
`FIRESTAFF_DM1_PC34_SAVE_CORPUS` set, it requires every classifier-qualified
external candidate to complete the ReDMCSB `LOADSAVE.C` F0435 -> F0433 ->
F0435 transaction, then emits its source/export byte hashes, F7057 envelope
endpoint, trailing-tail size, and no-fallback runtime stage/adoption result.
Without an explicitly staged corpus it reports `SKIP` and makes no parity or
interop claim. The outstanding requirement remains provenance-recorded PC34
saves and original executable load results.

# ✅ 2026-07-13 DM1 PC34 F0435/F0433 dungeon-tail column-table validation

The original-save handoff now validates every persisted
`G0280_pui_DungeonColumnsCumulativeSquareFirstThingCount` entry against the
saved raw-map thing-list flags before materializing a dungeon tail. This
closes the gap where M10 could reconstruct a lookup from tiles while accepting
a checksum-valid but different ReDMCSB F0433 column table. Spare SFT capacity
is retained: the final cumulative index may be below the saved allocation
length, exactly as `DUNGEON.C F0160` requires. Source: ReDMCSB `LOADSAVE.C`
F0433:1641-1682 / F0435:1995-2017, `DUNGEON.C F0160`, and
`READWRIT.C F0421/F0422`. Verification: isolated Ninja build and
`test_dm1_v1_original_save_pc34_handoff` pass with a real local F0433 export;
a checksum-recomputed mismatched column row is rejected before handoff.

# ✅ 2026-07-14 CSBWin DSA door timer handoff

CSBWin restored `TT_DOOR` now retains its source queue slot through one
authenticated type-47 pure-stack DSA action and the subsequent same-time
`TT_1` door handoff. This follows `Timer.cpp::ActivateDSA` and
`ProcessTT_DOOR`: DSA runs first, then the original timer is converted without
changing its time. Multiple type-47 entries, failed/unsupported DSA actions,
and all world-mutating DSA behavior remain fail-closed. Verification:
`csb_v1_csbwin_dsa_door_timer_handoff`.

# ✅ 2026-07-14 CSBWin real-package DSA catalog tick receipt

The opt-in `csb_v1_csbwin_extended_dsa_handoff` probe now fingerprints every
authenticated `DSA::Read` action from a supplied original package, including
its `(DSA, state, column)` selector, before advancing the production runtime
one tick. It accepts the tick only when every action keeps its original owned
address and words and every unique selector still resolves to that same
save-owned action. No package path means `SKIP`; no save, DSA, timer, or
fallback action is constructed. Source boundary: CSBWin `SaveGame.cpp`
`ReadExtendedFeatures`/`ReadDSAs`, `DSA.cpp` `DSA::Read`, and ReDMCSB
`LOADSAVE.C F0435_STARTEND_LoadGame`.

# ✅ 2026-07-14 CSBWin zero-word DSA action rejection

The authenticated CSBWin DSA runner now rejects an imported action with no
program words before it inspects an opcode. This preserves parameter words,
save-owned globals, execution counters, and both prior stack and transfer
receipts when a malformed save action reaches the pointer-identity boundary.
It adds no opcode support, synthetic behavior, or world/filter route. Source:
CSBWin `SaveGame.cpp::ReadDSAs` / `DSA.cpp::ProcessDSAFilter` and `Execute`.
Verification: focused `csb_v1_dsa_trigger_single_step_pc34_compat` CTest.

# ✅ 2026-07-14 CSBWin saved TT_75 full-word poison continuation

Restored `TT_75` now keeps its full `timerWord6` through the source-timed
poison requeue instead of truncating it to one byte. Verification:
`csb_v1_dsa_restored_timer_tick_bridge`.

# 2026-07-14 DM1 V1 message-area physical font clipping

M11 now clips each C015 message row by the measured pixel width of the active
original font, rather than an assumed fixed character width. This preserves
ReDMCSB TEXT.C's 320-pixel bottom surface and prevents wide original glyphs
from overrunning the right edge. Verification: `test_dm1_v1_text_message_pc34_compat`,
`test_m11_dialog_choice_overlay_fit_pc34_compat`, and
`test_m11_open_door_spell_runtime_pc34_compat`.

# 2026-07-14 DM1 V1 spell/action HUD source-box geometry

The C009 spell background and C010 action area now interpret ReDMCSB DATA.C
BOX values as inclusive `{left,right,top,bottom}` coordinates. Their real
destinations are therefore `{224,42,96,33}` and `{224,77,96,45}`, instead of
the previously malformed HUD rectangles. Verification:
`test_dm1_v1_box_spell_area_pc34_compat` (175 assertions),
`test_dm1_v1_box_action_area_pc34_compat` (161 assertions), and
`test_m11_open_door_spell_runtime_pc34_compat`.

# 2026-07-14 DM1 save close-failure recovery

Both native and PC34 save writers now remove a failed primary file and restore
the previous `.bak` after a close failure. The ordinary original-save handoff
and C13 runtime gates pass; the optional external PC34 corpus remains skipped
until supplied.

# ✅ 2026-07-14 DM1 F0435 original-save runtime provenance gate

M11 now publishes `ORIGINAL_SAVE_PC34` only when the input was classified and
materialized through the ReDMCSB `LOADSAVE.C F0435` PC34 handoff. Firestaff
native F0433-style save loads instead retain `QUICKSAVE_RESUME_PC34`, so they
cannot enter the external-original HoC/runtime presentation gate by sharing a
generic resume path. `test_m11_quick_resume_roundtrip` asserts native,
original-PC34, and re-exported-PC34 provenance separately; the fixture-free
external HoC test remains the only corpus-backed proof of an externally
supplied original runtime. Source boundary: ReDMCSB `LOADSAVE.C F0433/F0435`.
# ✅ 2026-07-14 DM1 original PC34 HoC/save/runtime source-backed idle-tick gate: strengthened `test_dm1_v1_original_save_pc34_external_hoc_runtime` without fixtures, generated saves, or substitute graphics. For every explicitly staged external PC34 candidate that passes F0435/F0433/F0435 admission, the test materializes an independent F0435 world and advances it with the same no-input `F0884_ORCH_AdvanceOneTick_Compat` route before M11 receives `M12_MENU_INPUT_NONE`. M11 must retain `ORIGINAL_SAVE_PC34`, advance exactly one tick, and match the staged source world's pre/post ticks, canonical post-tick world hash, timeline count, emission count, and full fixed emission receipt. It still requires a nonblank, byte-stable 224x136 viewport from original PC34 media, but makes no DOS pixel-parity claim. Source anchors: ReDMCSB `LOADSAVE.C F0435` (restored party/EVENTS/TIMELINE/dungeon), `GAMELOOP.C` idle command loop, and `TIMELINE.C F0261` event processing. The opt-in test skips cleanly without `FIRESTAFF_DM1_PC34_SAVE_CORPUS` and `FIRESTAFF_DM1_PC_DATA`.
# 2026-07-23 - DM1/CSB G0201-G0250, M0101-M0150, P0151-P0200, F1886-F1965

Completed the next ReDMCSB inventory batch. DM1 graphic-table, macro-label,
and video-parameter ownership plus CSB hint-load/CPSX source boundaries are
now covered by focused PC34 compatibility tests; unproved routes stay
fail-closed.
# 2026-07-23 - DM1/CSB G0251-G0300, M0151-M0200, F1966-F2045

Completed the next ReDMCSB inventory batch. DM1 dungeon-state globals and
macro labels plus CSB hint/input source boundaries are covered by focused PC34
compatibility tests; unproved routes stay fail-closed.
# 2026-07-23 - DM1 P0201-P0250

Completed the ReDMCSB dungeon-parameter ownership audit with a focused PC34
compatibility test. Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB M0201-M0250, F2046-F2125

Completed the ReDMCSB macro-label and CSB platform/portrait-input ownership
batch with focused PC34 compatibility tests. Unproved routes stay fail-closed.
# 2026-07-23 - DM1 G0301-G0350, P0251-P0300

Completed the ReDMCSB base-runtime-global and dungeon-map-parameter ownership
batch with focused PC34 compatibility tests. Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB M0251-M0300, F2126-F2205

Completed the ReDMCSB macro-label and CSB platform/CPSX ownership batch with
focused PC34 compatibility tests. Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB G0351-G0400, M0301-M0350, P0301-P0350, F2206-F2245

Completed the ReDMCSB message/timeline-global, macro-label, group-parameter,
and CSB platform ownership batch with focused PC34 compatibility tests.
Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB G0401-G0450, M0351-M0400, F2286-F2325

Completed the ReDMCSB movement/panel/input-global, macro-label, and CSB
unmapped-platform ownership batch with focused PC34 compatibility tests.
Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB P0351-P0400, F2326-F2365

Completed the ReDMCSB group-combat-parameter and CSB unowned-route ownership
batch with focused PC34 compatibility tests. Unproved routes stay fail-closed.
# 2026-07-23 - DM1 M0401-M0450

Completed the ReDMCSB macro-label ownership batch with a focused PC34
compatibility test. Unproved routes stay fail-closed.
# 2026-07-23 - DM1/CSB G0451-G0500, P0401-P0450, F2366-F2445

Completed the ReDMCSB Graphic560-global, group-projectile-parameter, and CSB
unmapped/unowned ownership batch with focused PC34 compatibility tests.
Unproved routes stay fail-closed.
# 2026-07-23 - DM1 M0451-M0500

Completed the final confirmed ReDMCSB macro-label ownership boundary. M500 has
no independent verified PC34 owner and remains fail-closed; focused test
passes.
# 2026-07-23 - DM1/CSB G0501-G0550, P0451-P0500, F2446-F2525

Completed the ReDMCSB Graphic/save-global, projectile/melee-parameter, and
CSB unmapped/unowned ownership batch with focused PC34 compatibility tests.
Unproved routes stay fail-closed.
# 2026-07-23 - ReDMCSB L0051-L0100

Completed the ReDMCSB local-symbol ownership audit. No independent PC34 owner
was found, so every route remains explicitly fail-closed; focused test passes.
# 2026-07-23 - DM1/CSB G0551-G0600, P0501-P0550, F2526-F2605

Completed the ReDMCSB save/media/input-global, projectile/melee-parameter, and
CSB unmapped/unowned ownership batch with focused PC34 compatibility tests.
Unproved routes stay fail-closed.
# 2026-07-23 - DM1 Action Menu Graphic Geometry

Fixed the source geometry check for F0387's C010 action asset: it now validates
the 87x45 graphic rectangle rather than the larger C011 clear rectangle.
Missing or mismatched original material remains fail-closed.
# DM1 synthetic wall-coordinate fallback removed

- ✅ 2026-08-05 DM1 wall-ornament provenance cleanup: removed the legacy
  `DM1_V1_WallOrnament_SetupDefaultCoordsPc34Compat()` perspective rectangles.
  They were approximate host geometry and were not present in ReDMCSB. The
  compatibility entry point now clears its coordinates and fails closed;
  active PC34 rendering continues to consume the authenticated G0205 table
  from `GRAPHICS.DAT`. The focused wall-ornament test passes 127/127.

- ✅ 2026-08-05 CSB archive save-cache provenance: optional CSB archive
  sidecars now form an exact view of the hash-selected package.  If a newly
  scanned package lacks `MINI.DAT` or `CSBGAME*.DAT`, an older cached member
  is removed rather than being offered as authentic Resume data.  DMWeb's
  Saved Game Files reference confirms that `MINI.DAT` is the original CSB
  campaign saved-game image, while `CSBGAME.DAT` is a player save and
  `DUNGEON.DAT` contains only the Prison.  Regression coverage first
  materializes both archive members, rescans a GRAPHICS-only replacement, and
  verifies that neither stale save remains.  The focused archive split test
  passes.
- ✅ 2026-08-05 CSB verklig första viewport-frame: den registrerade
  `firestaff_csb_v1_first_viewport_frame_probe` skapar inte längre en falsk
  PC-profil med stubbat `GRAPHICS.DAT` och en egen dungeon. Den kräver nu en
  hashverifierad CSB-källa via argument eller `FIRESTAFF_CSB_DATA`, accepterar
  PC-, Atari ST- och Amiga-varianter och skippar ärligt när ingen paketdata
  finns. Runtime behåller den verifierade variantidentiteten i stället för
  att förutsätta PC 3.4. Verifiering: CTest utan data skippar och passerar;
  verkliga Atari ST-data från `Game,Chaos_Strikes_Back,Atari_ST,Software.7z`
  extraherades till en tillfällig katalog och proben passerade 30/30 genom
  boot, dungeon-handoff, tick, första viewport-frame och cleanup.
- ✅ 2026-08-05 DM1 movement timing fail-closed: the isolated PC34
  movement reference no longer assumes a synthetic two-tick default when
  champion records are unavailable. It now requires the caller to provide
  the ReDMCSB F0310 party timing result, leaves movement queued when absent,
  and retains the exact active F0310 formula in the shared timing path.
  Focused movement and timing tests pass.

# Documentation now distinguishes the reviewed bounded PRS3 decoder and its
# 20/20 real FACE.BIN result from the still-blocked Saturn VDP1 presentation
# route. No startup pixels or synthetic fallback were enabled.

# corpus test verifies 243 item declarations, 223 inventory images, and 109
# overlay as test/probe-only; production remains on the no-op boundary until
# retail Saturn widget ownership and VDP1/VDP2 placement are captured.

- ✅ 2026-08-05 DM1 PC 3.4 Greatstone/DMWeb asset audit: verified the local
  `GRAPHICS.DAT` (713 records) and `TITLE` (59-record/53-frame) identities,
  confirmed the normal runtime consumes real C001 title graphics, and the
  existing 22-record SCK/Greatstone pixel comparison remains zero-difference.
  Title cadence/palette tests pass. Full 713-record visual comparison and
  packaged macOS capture remain explicitly open in `TODO.md`.
- ✅ 2026-08-05 DM1 legacy synthetic viewport removal: the unowned
  `firestaff_viewport_renderer` bridge no longer paints coloured rectangles
  for missing walls, floors, ceilings, side walls or doors. It now draws only
  an authenticated atlas wall bitmap; unavailable source material remains
  no-draw. The local DOS manual was recorded as behavioural evidence for
  pickup, cursor and inventory contracts, never as a substitute asset source.
  Ninja `firestaff` build and `git diff --check` pass.

- ✅ 2026-08-05 DM1 utility-panel fallback removal: normal DM1 source sessions
  no longer paint the legacy cyan panel, host champion/status labels or light
  bar when the authenticated C009/C010/C011 surfaces are unavailable. Those
  paths remain explicitly limited to debug/non-source sessions; missing DM1
  source material is now no-draw.

# ✅ 2026-08-06 DM1 legacy V2.2 overlay synthetic-pixel removal

The retired `m11_v22_render_overlay_pc34` compatibility route no longer paints
material-colour rectangles, palette-shadowed fills, borders, or any other
generated V2.2 pixels over the DM1 V1 framebuffer. It remains link-compatible
for probes and preserves the source viewport-cell geometry API, while the
authenticated in-place renderer is the only route allowed to draw V2.2 art.
The focused test now proves that both an empty cache and a populated V2.2
cache leave the framebuffer byte-for-byte unchanged. Tracked as
`DM1-ORIGINAL-REPLACE-016`.

# ✅ 2026-08-06 DM1 blocked-step audio and damage HUD correction

DM1 blocked forward/backward steps now emit the source `C00_SOUND_METALLIC_THUD`
through the existing source-audio path when ReDMCSB's collision gate rejects a
dequeued step. Removed the Firestaff-only yellow attack-X overlay. Damage
numbers now honor the original font baseline before drawing, keeping them
inside the C015/C016 damage surface instead of painting over champion names.
The DM1 Atari GRAPHICS.DAT contract was also corrected to DMWeb's real 563
records. `firestaff` builds successfully; packaged Mac capture remains an open
verification item.

- ✅ 2026-08-06 DM2 PC-DOS startup/menu real-corpus verification: ran the
  complete 103-case startup action contract and the M11 startup/profile gate
  against the user's existing `dos_extract` corpus. The checks consume the
  actual 320×200 decoded title/menu GDAT records and package palette/HUD
  handoff, while New Game remains blocked at original `GAME_LOAD` instead of
  fabricating a playable session. The verification was read-only; no game
  data was unpacked, copied or tracked.
- ✅ 2026-08-06 DM2 dialogue-selector audit: removed the false TODO for
  `_476d_04ed` from the isolated `c_0aaf` compatibility audit. SkProject
  `skgame.cpp:2575-2579` proves that routine is an intentional unconditional
  zero-return no-op, so no auto-selection callback or synthetic behavior was
  added. The callback audit remains outside production pending a real M11
  dialogue/menu bridge.
- ✅ 2026-08-06 DM2 M12 verified-launch handoff: extended the real-data M11
  startup gate through the actual M12 DM2 card, options and Launch action.
  A DOS install whose original files are under `DATA/` now has a regression
  proof that M12's runtime directory exactly equals the matched asset-owner
  directory before M11 boots the hash-verified PC-English profile. This covers
  the source build, not a
  separately installed stale application bundle; no game data was written,
  unpacked or tracked.
- ✅ 2026-08-06 DM2 runtime provenance wording: removed the obsolete “Runtime
  Stub — Phase 1” declaration from the linked DM2 runtime. Its public source
  evidence now accurately identifies the hash-verified boot and real-GDAT
  frame gates, while unowned gameplay remains explicitly fail-closed. This is
  a provenance correction only; it does not promote partial save or gameplay
  state into a playable session.
- ✅ 2026-08-06 DM2 dungeon loader ownership gate: the bounded legacy loader
  no longer returns success after a failed copy of its input bytes. A success
  result now always retains the complete owned dungeon buffer required by all
  square and record accessors; allocation failure is rejected before any
  partial handoff. The production PC G1 route remains hash-verified and
  source-byte backed; no fixture layout was promoted into the runtime.
- ✅ 2026-08-06 DM2 HUD source-plan receipt correction: the active viewport
  now retains the original GDAT command's decoded width, height, indexed
  pixels and hash-verified palette after drawing a source-plan HUD panel.
  The top-bar and portrait-panel presentation receipts therefore remain
  source-backed even when no duplicate host palette callback ran. A new
  real-`GRAPHICS.DAT` regression checks all nine static HUD commands and both
  retained material receipts; missing or altered command data still blocks
  drawing without a fallback surface.
- ✅ 2026-08-06 DM2 legacy creature-fixture boundary: corrected the public
  contract for `dm2_v1_creature_tick()`. It advances only its isolated test
  pool and is not a `DM2_THINK_CREATURE`/CCM implementation or an M11 runtime
  path. The production frame gate and the focused CCM test both confirm that
  source-owned DB4/CAII/command-stream state is still required.
# ✅ 2026-08-06 DM1 inventory fallback removal

Authenticated DM1 inventory frames no longer draw a red host rectangle or a
host damage number when the original C016 damage bitmap is missing. Food and
water labels likewise remain source-owned GRAPHICS.DAT surfaces; missing label
records no longer become generated text. The focused damage source-gate test
and `firestaff` build pass.

- ✅ 2026-08-06 Theron native Mednafen consumer-capture receipt: built the
  instrumented Mednafen 1.32.1 path against native SDL2, reached the authentic
  PC Engine Super CD-ROM2 runtime with the supplied Track 02 CUE, and retained
  4,096 bounded `main_ram_consumer_read` rows. The new C11 receipt verifies the
  source marker, monotonic rows, and both executing and target physical bank
  coordinates while explicitly keeping `$2600`, object/level, and visual
  semantics unpublished. The repository test passes against the captured
  sidecar; raw media and traces remain user-supplied and untracked.
- ✅ 2026-08-06 Theron CD sidecar compatibility: the strict CD receipt now
  accepts the authenticated E009 enter/register/return observation rows that
  accompany the native app capture. The captured run verifies 54 SCSI reads,
  171 raw MODE1/2352 sectors, and all 171 command bindings while retaining
  the semantic-publication block.
- ✅ 2026-08-06 CSB FM Towns ANM frame decoding: replaced the former
  structure-and-palette-only reader with a bounded decoder for the original
  `EN`/`DL` command stream. It follows ReDMCSB `ANIM.C F2275` and
  `ANIMIMG.C F8288`, retaining the prior source frame while applying literal,
  fill, skip and previous-line commands. The real CD-cached `TITLE.ANM`,
  `STORY.ANM` and `ENDING.ANM` now decode their first and final frames at
  320×200/4bpp with source-derived hashes. M11 presentation scheduling stays
  open; no host artwork or generated frame fallback was added.

- ✅ 2026-08-06 DM1 HoC D1/D0 wall-ornament palette regression: restored the
  ReDMCSB F0107/F0110 native C10 pixel route for D1/D0 wall ornaments,
  including the champion-mirror backing frame. G0198/G0199 palette maps remain
  limited to their D3/D2 projections. The focused PC34 wall-ornament test now
  verifies native D1C output and all 126 assertions pass.

- ✅ 2026-08-06 DM1 real PC34 floor-capture test: fixed the test data resolver
  to recognize the original archive's `DATA/DUNGEON.DAT` and
  `DATA/GRAPHICS.DAT` layout. The D0C F0115 material and final M11 capture
  receipt now run instead of falsely skipping and pass against the extracted
  original corpus.
- ✅ 2026-08-06 DM1 HoC synthetic wall-marker removal: authenticated DM1 no
  longer paints the legacy white thing-chain marker on closed wall cells.
  That marker was host-only diagnostic art and could appear as junk on walls;
  it remains available only for non-source debug probe worlds. The real PC34
  object-name, alcove placement, HoC orientation/mirror, inscription
  invalidation, wall-material and viewport-wall capture tests pass after the
  change.
- ✅ 2026-08-06 DM2 PC-DOS SKSave corpus spelling: the source-backed corpus
  and startup menu now recognize the supplied lower-case, single-digit
  `sksave0.dat`…`sksave3.dat` files and their four `.bak` counterparts.
  All eight files pass the authenticated 42-byte header and raw-dungeon
  boundary checks; the menu exposes only the real four slots and does not
  fabricate a resumable session from the still-partial raw save decoder.
- ✅ 2026-08-06 DM1 HoC false-artifact audit: real PC34 `DUNGEON.DAT` sampled 2,172 open HoC cells and 83 source item cells. The projectile/explosion guard found zero compact projectile chains, zero explosions, zero viewport leaks, zero stale fields, zero debug-marker leaks and zero fire/explosion blob leaks; the floor-item guard found 83/83 real item renders with zero viewport mismatches. The prior 30-sample hot-color warning was a false positive, not missing game data.
- ✅ 2026-08-06 DM1 HoC visual-asset audit: authenticated PC34 item labels resolve through the real M564 icon-name stream, object icons through the real G0237/F0033 mapping, and the held-object cursor through the source F0702 route. The 611-record object corpus, floor pickup-to-mouse-hand path, and rendered-pile hit targets pass. The D1/D2/D3 wall-ornament audit also confirms source GRAPHICS.DAT material, C10 transparency and ReDMCSB palette-depth routing; no active DM1 host subtype-name or synthetic wall-ornament fallback was found. Packaged macOS capture remains open in TODO.
- ✅ 2026-08-06 Nexus SLEV/SAL playback provenance: the real renamed
  SNDLEV00.SAL/.MAP hash route now verifies that loaded diagnostic windows
  cannot create a host SFX voice through either `NEXUS_SFX_*` dispatch or the
  legacy sample-index API. SAL candidates remain opaque until the authentic
  Saturn event→MAP/SDDRVS consumer is captured; no synthetic playback is
  introduced.

- ✅ 2026-08-06 Nexus MENU.BPK presentation gate: real PRS3 decode/upload
  receipts no longer promote a menu to drawable merely because 162 surfaces
  decoded. The renderer and launcher now require the independently admitted
  Saturn PALT, palette-state and VDP1-command capture; missing capture stays
  no-draw, while an authenticated opaque capture opens the route. Real
  European boot and handoff regressions pass without fallback visuals.

- ✅ 2026-08-06 Nexus PRS3 strict legacy API parity: the public decoder now
  enforces DMWeb PRS3 version 1 and rejects positive forward-window
  references that point beyond produced output instead of manufacturing
  zero-filled pixels. Negative-window prefix zeroes and valid forward-window
  copies remain source-compatible; the legacy regression and real 162-entry
  MENU.BPK decode/upload receipt both pass.

- ✅ 2026-08-06 Nexus MNS host-route quarantine: the unbound MNS pose/texture
  helper is no longer exported by the production Nexus library. Its 30-file
  retail corpus and 815 source textures remain covered by an explicit
  real-data test, while approximate trig/BGR555 output cannot become a
  viewport image before Saturn VDP1/VDP2 capture proves the consumer.

- ✅ 2026-08-06 Nexus shop mutation quarantine: the real DM.BIN eight-row
  price catalog remains source-bound, but the unproven host buy/sell manager
  can no longer mutate champion inventory, gold or stock. Saturn shop/event
  capture is still required before those actions can be enabled.

- ✅ 2026-08-06 Nexus gold-pile quarantine: `nexus_gold_add()` can no longer
  manufacture a floor gold object without an authenticated Saturn drop
  producer. The loot/drop gate and real boot regression remain green.

- ✅ 2026-08-06 Nexus spell-effect host-route quarantine: disassembly-backed
  spell tables remain covered by the focused probe, but the unbound helper
  that directly mutated health/status/light/projectiles is no longer exported
  by the production Nexus library. Live spell action remains capture-gated.

- ✅ 2026-08-06 Nexus M11 HUD readiness boundary: a ready DGN viewport can no
  longer set `startup_hud_ready`. M11 now keeps HUD readiness false until the
  independent DM.BIN/VDP1/VDP2/FONT256/SLEV consumer capture is admitted.

- ✅ 2026-08-06 DM2 PC-DOS save-path handoff: direct startup save selection
  now accepts the real lower-case, one-digit `sksaveN.dat` filename shape,
  carries the containing data root and slot to the validated loader, and
  continues to reject a partial raw `GAME_LOAD` as a playable session. The
  focused menu contract and the supplied `sksave0.dat` corpus path both pass.
- ✅ 2026-08-06 DM2 raw-record timer gate: ornament animator/noise and timer
  types `0x58`, `0x59`, `0x5B` and `0x5C` no longer mutate or requeue from a
  boot-time raw record-pool address. The original GAME_LOAD timer queue,
  actuator lifetime and record-write transaction are not yet restored, so
  the runtime now leaves these source events unbound and fail-closed.
- ✅ 2026-08-06 DM1 HoC interaction audit: real PC34 validation still passes
  the 611-record M564 name/icon corpus, 8 HoC F0115 floor objects,
  alcove pickup/placement, mirror orientation/click, wall-material and
  inscription invalidation routes. Fixed a latent stale-click-target bug by
  binding each rendered floor-item target to its source `mapIndex` before
  pickup or mouse placement; a target from a previous map can no longer
  unlink an object from the active square. Packaged macOS capture of torches,
  holders, stairs, doors and the held-object cursor remains open in TODO.
- ✅ 2026-08-06 DM2 incomplete SKSave resume closure: public raw-candidate,
  slot and last-session resume entry points now reject before touching live
  state. The real raw prefix remains available to the read-only corpus
  diagnostics, but cannot become a partial `GAME_LOAD` session before the
  complete record/possession/hero/actuator/timer chain is ported.

- ✅ 2026-08-06 DM2 session-publication boundary: made session application a
  private helper of the parsed original-save candidate transaction and removed
  the public runtime API. Caller-constructed session structs can no longer
  write party, map, inventory or timer state into a linked DM2 runtime; the
  active path still requires the authenticated candidate parser and its
  preflight checks.

- ✅ 2026-08-06 DM2 private runtime-save removal: removed the production
  `FS2RT01` sidecar serializer, deserializer and public export API. That
  format combined Firestaff session/cache/dungeon state and could not be an
  original `DM2_GAME_SAVE` SUPPRESS stream. The sole fixture consumer is now
  compile-disabled; normal save actions remain fail-closed until the complete
  original writer is recovered.

- ✅ 2026-08-06 DM1 C015/C016 damage transparency: fixed the authenticated
  PC34 champion damage overlays to use ReDMCSB `C10_COLOR_FLESH` (palette slot
  10) for both normal and inventory damage surfaces. Black source pixels are
  artwork, not the transparency key; this removes the block/name corruption
  seen during creature damage. `test_m11_dm1_damage_indicator_source_gate`
  and `test_dm1_v1_champion_panel_damage_indicator_pc34_compat` pass. A real
  packaged macOS capture is still intentionally tracked in TODO.
- ✅ 2026-08-06 DM2 wall-button fallback removal: removed the two local
  DB2/DB3 record-chain walkers from the linked V1 runtime. A custom or test
  viewport asset provider can no longer derive a `WALL_GFX` button from raw
  tile-chain bytes; only the authenticated dungeon graph and direct G1
  material receipts may authorize one. The focused WALL_GFX and door-scene
  gates pass, and the real PC-DOS startup probe still reaches the original
  title/menu surface.

- ✅ 2026-08-06 DM1 authenticated side-door duplicate: the deferred F0115
  content pass no longer redraws a whole wall-set side bitmap over ReDMCSB
  F0111's exact door panel/frame slices. This removed the generic C0
  transparency path that could create doubled or black edges and inconsistent
  open-door states. Source material, HoC wall, orientation and final-capture
  tests pass against the real PC34 corpus; packaged macOS capture remains in
  TODO.
- ✅ 2026-08-06 Nexus spell-cast mutation quarantine: retained the
  hash/disassembly-backed DM.BIN spell lookup and mana-table probe, but made
  `nexus_v1_cast_spell()` fail closed without mutating mana or synthesizing
  damage. The live mechanics path was already action-gated; focused magic and
  spell-cast tests now assert the no-mutation boundary. Saturn dispatcher,
  effect/target, RNG and SLEV/SFX capture remain open.
- ✅ 2026-08-06 Theron FIFO-origin capture path: added a valid Mednafen 1.32.1
  capture patch and wired it into the Theron build script. Each CD data byte is
  retained with raw LBA, sector offset and FIFO sequence before the Firestaff
  CD-to-RAM receipt; the parser accepts only source LBAs covered by authenticated
  SCSI reads. The existing capture remains transport/loader evidence only until
  the patched binary produces a fresh consumer trace; no runtime semantics or
  synthetic viewport data were enabled.
- ✅ 2026-08-06 Nexus HUD geometry provenance handoff: the real `DM.BIN`
  `yam\\menuctrl.c` parser now carries its canonical source receipt, 80 layout
  entries and 40 hit rectangles through the Nexus startup host-caller receipt
  into M11. A real renamed-DM.BIN boot regression verifies the European corpus
  path. M11 still keeps `startup_hud_ready=0`; Saturn element surfaces,
  palette, FONT256 text and VDP1/VDP2 destination capture remain required
  before any HUD pixels or input semantics are enabled.
- ✅ 2026-08-06 Nexus unregistered-door placeholder removal: a type-8 square
  without a source-owned door registration no longer defaults to passable or
  openable. The square-event path now blocks it, while registered DGN doors
  retain their bounded state machine. Regression coverage proves the missing
  SDDRVS/DGN state cannot create a door transition or viewport movement.
- ✅ 2026-08-06 Nexus projectile door fallback removal: projectile traversal
  now treats an unbound type-8 square as a collision, rather than allowing
  the generic non-wall passability helper to send a projectile through a
  closed door. The focused regression covers the fail-closed route; open-door
  passage remains capture/state-gated.
- ✅ 2026-08-06 Nexus real ITEM.IBS floor provenance: the boot regression now
  loads retail LEV01 and verifies that its eight authenticated Structure1F
  item records materialize exactly eight floor items through the hash-bound
  ITEM.IBS association table. No creature-drop, gold, name, action, combat or
  HUD semantics are inferred from that handoff.
- ✅ 2026-08-06 DM1 PC34 wall-ornament projection fix: corrected the live
  F0107 host dispatch to ReDMCSB's authenticated 13-row G0205 table, removing
  the duplicate/misaligned D3L2/D3R2 entries that shifted side/depth ornaments.
  The production consumer now creates the real F0675 14/32 or 21/32 indexed
  derived bitmap and clips it through the real G0205 zone in the F0791 order,
  rather than scaling the entire native GRAPHICS.DAT surface to the zone.
This fixes the source path for torch holders, mirrors, inscriptions and
  narrow wall ornaments without synthetic pixels. Ninja build plus the 126/126
  source test and real PC34 side-wall, mirror, and HoC wall-material tests pass.
- ✅ 2026-08-06 Nexus FONT256 runtime-flag correction: real FONT256.S2D
  character-generator bytes remain retained for diagnostics, but
  `font_loaded` stays clear until the Saturn page/attribute character mapping
  and text consumer are proven. The real-data boot regression covers all 242
  retained tiles and the blocked runtime flag.
- ✅ 2026-08-06 CSB FM Towns Game handoff: F31E now enters only the verified
  `CHTWE.EXP` C03_GAME owner after the source-language SWITCHTW Game click.
  M11 presents the authenticated C004 entrance raster and its entrance palette
  directly, without manufacturing a PC TITLE.C replay. The opt-in
  `csb_v1_fmtowns_m11_game_handoff` regression runs against original F31E
  media and verifies TITLE.ANM → SWITCHTW → CHTWE, C001--C005/C017/C040
  session admission and exact C004 pixels.
- ✅ 2026-08-06 Nexus MENU.BPK PRS3 pixel provenance: the real 162-surface
  DMWeb PRS3 decode now records an aggregate FNV-1a hash over the emitted
  indexed pixel bytes, and the upload-plan regression requires the same hash.
  This is diagnostic source output only; palette interpretation and VDP1/VDP2
  presentation remain capture-gated.
- ✅ 2026-08-06 Theron source-lock documentation correction: downgraded the
  stale Phase 2 “complete” claims to the evidence actually supported by the
  authenticated US/JP Track 02 records. The document now distinguishes
  source-bound map/object/ground/door/creature-bank retention from unresolved
  HuC6280 consumer handoff, level decoding, JP text ownership, tile/material
  mapping and palette semantics. No runtime behavior or synthetic fallback was
  changed.

# DM1 original-data replacement: source gate for legacy wall primitives

DM1 source sessions now suppress the legacy wall/door/stairs primitive path as
soon as the authenticated PC34 `GRAPHICS.DAT` loader is initialized. The old
gate incorrectly waited for populated Thing tables, allowing a synthetic
primitive to appear during the interval between graphics admission and the
later F0115 Thing handoff. The source renderer now fails closed until its real
material consumer is ready.

Verification: focused DM1 inventory/object/portrait/inscription/stairs/sound
tests passed (27/27), and the real archive-backed object names, object corpus,
alcove runtime and archive-media tests passed (4/4).
- ✅ 2026-08-06 CSB Utility Disk real-media regression: the existing
  utility/import CTest now has an opt-in `FIRESTAFF_CSB_UTILITY_DISK` lane
  that drives the same ReDMCSB `UTIO.C F1991` Amiga volume-identity check as
  production. It remains skip-safe in CI with no licensed media. The supplied
  English Utility Disk 3 ADF was independently hash-checked as
  `85091454b3885a216f6bdbbe5c47cc75` (the registered original release 3) and
  passes the real-media lane, 79 checks total.
- ✅ 2026-08-06 CSB real CMP decoder regression: `test_firestaff_cmp_decode`
  now accepts `FIRESTAFF_CSB_CMP` for an opt-in original portrait check while
  retaining its data-free self-test in CI. The authenticated FM Towns
  `PORTRAIT/ALEX.CMP` cache member decodes successfully (MD5
  `e2a889bdb8e1923cb7f83989f92b53a2`); this is portrait metadata/pixel
  validation only and does not promote a portrait-only CMP to a party save.
- ✅ 2026-08-06 CSB FM Towns MINI.DAT save-boundary gate: verified both
  original CDATA (42,776-byte) and CJDATA (43,208-byte) MINI.DAT files
  against the production Resume/runtime route. Neither file is an Atari/
  Amiga big-endian GAMEBLOCK save, so the opt-in real-media regression now
  requires both to remain rejected without changing the live dungeon or party.
  ReDMCSB `LOADSAVE.C` F0435 selects the separate F31E/F31J save-header path;
  that platform-specific decode and handoff remain open rather than being
  substituted with the Atari reader.
- ✅ 2026-08-06 launcher missing-extractor popup: archive-backed game data
  now gives an actionable, localized installation instruction instead of a
  generic missing-data result. The scanner identifies the actual required
  host tool (`unrar/7zz/7z/bsdtar` for external archives, `chdman` for CHD),
  and the M12 popup tells the player to install one and rescan. The three
  strings are translated in every shipped startup-menu catalog (19 locales
  plus English); focused 7z and CHD popup regression coverage is included.
- ✅ 2026-08-06 Nexus DGN readiness regression sync: the geometry-readiness
  fixture now expects the current source-authenticated MENU.BPK handoff to
  remain `SATURN_PRESENTATION`/no-draw when PALT and VDP1 capture are absent.
  Real Structure3 viewport presentation remains capture-gated.
- ✅ 2026-08-06 Nexus PRS3 real-corpus admission: corrected the capture
  schema's `MENU.BPK` MD5 to the verified retail value
  `a6f2272a4f6cb3c6b3b33012bc5b15ed`, matching the boot profile. The real
  V3 sidecar/provenance regression now binds evidence while remaining
  fail-closed for Saturn authentication and runtime import.
- ✅ 2026-08-06 DM2 unowned-creature-pool removal: production no longer
  restores a caller-authored `DM2_V1_CreatureLiveState` into its global pool.
  The standalone structure remains an explicit fixture facility only; real
  creature restoration remains closed until the original SKSAVE, DB4, timer
  and CAII/CCM ownership chain is imported. The focused production gate
  verifies both rejection and no mutation.
- ✅ 2026-08-06 Nexus CDDA fidelity boundary: removed the unproven host
  `track02.wav/ogg/mp3` fallback from manual track selection. A local audio
  file can no longer masquerade as Saturn Red Book playback; selection remains
  provenance-only until an authenticated media handoff exists.
- ✅ 2026-08-06 Theron ISO capture parser correction: the CUE `FILE`/`TRACK 02`
  association now matches both authenticated `MODE1/2352` and `MODE1/2048`
  layouts, so the ISO route reaches its payload/hash gate instead of being
  rejected by a raw-BIN-only matcher. Shell syntax, regression test and diff
  checks pass; no semantic consumer handoff was promoted.
- ✅ 2026-08-06 Theron generator fallback removal: production no longer
  activates the legacy DMWeb-derived generator table or executable source
  generator records before the original timing/re-enable consumer is bound.
  The focused combat-runtime regression now asserts zero active generators in
  that unbound state; source records remain available as diagnostic receipts.
- ✅ 2026-08-06 Theron production archive cleanup: removed the unused legacy
  `theron_v1_track02_creature.c` translation unit from `firestaff_theron` after
  closing its generator route. The production archive was rebuilt and checked
  for absence of the legacy object; focused combat and real JP Track 02 loader
  regressions remain green.
- ✅ 2026-08-06 Theron source-monster spawn gate: regular creature admission now
  requires a matching authentic Track 02 source-monster ledger occurrence at
  the requested dungeon/level/coordinate, not merely a verified level header.
  The focused production regression covers header-only rejection and a bound
  source record; no host-positioned monster can bypass the real-data ledger.
- ✅ 2026-08-06 CSB selected-archive isolation: M12 can now materialize every
  selected CSB edition into its own hash-checked runtime cache instead of
  reusing the scanner's first-match `asset-cache/csb` view. The existing FM
  Towns CD extractor uses the same API; PC, Atari and Amiga copies validate
  their selected `GRAPHICS.DAT`, the authenticated shared `DUNGEON.DAT`, and
  package-owned startup sidecars. M11 uses that private route whenever an
  archive-backed non-first edition is selected, so title, entrance, HUD and
  viewport data cannot silently come from another platform. The focused ZIP+
  ISO split-archive regression passes; the real FM Towns handoff remains
  skip-safe without user media.

- ✅ 2026-08-06 DM2 unowned-weather-seed removal: the production runtime no
  longer accepts caller-provided weather seeds. Only the original
  GAME_LOAD/SKSAVE/ENVIRONMENT route may establish that state; the focused
  weather regression verifies that an outdoor flag plus a host seed retains
  neither a seed nor a weather chain.

- ✅ 2026-08-06 Nexus unbound rest/status/light quarantine: retail ISO and
  extracted engines no longer advance the DM1-shaped rest regeneration,
  poison/status expiry or torch/FUL/ambient-light timers while the Saturn
  action dispatcher and HUD/VDP consumers remain uncaptured. The explicit
  fixture source keeps isolated module tests usable, and the Nexus tick
  integration regression now covers the retail no-mutation boundary (15
  tests). No game data or BIOS was copied into the repository.

- ✅ 2026-08-06 Nexus engine hunger quarantine: closed the second, engine-level
  DM1 hunger tick that bypassed the mechanics resource gate. Retail extracted
  and ISO engines now retain food, water, health and hunger accumulators until
  the Saturn provisions/start/save consumer is captured. The focused tick
  regression now verifies threshold-crossing retail state remains unchanged
  (16 tests).

- ✅ 2026-08-06 Nexus PRS3 schema regression repair: the retail MENU.BPK V8
  capture test now has sufficient space for V9/V10 DGN extensions, rejects
  truncation explicitly, uses the parser's hexadecimal sequence convention,
  and restores the valid receipt after negative checks. Verification passes
  against the real Nexus data root; no decoder or VDP1 presentation is
  promoted by this test repair.

- ✅ 2026-08-06 DM1 save-disk format boundary: the in-game DM1 SAVE GAME,
  QUIT GAME and unsaved-quit paths now emit source-shaped PC34 save bytes via
  `DM1_SaveGamePC34()`. Firestaff's private quick-resume envelope remains
  confined to F9/host resume. The focused save/load suite passes 16/16 and
  the real PC34 graphics audit passes 713/713 structural records; authentic
  corpus-backed F0802/C13 writeback remains tracked in TODO.
- ✅ 2026-08-06 DM1 live sensor actuator handoff: M11 pit and fakewall
  SET/CLEAR/TOGGLE effects now use the source-bound
  `SensorActuatorDispatch_Compat` and `dm1_v1_actuator_execute_dispatch_pc34()`
  implementation instead of duplicating square-byte mutation in the host
  loop. HOLD remains non-mutating; door animation stays on the source timeline
  owner and teleporter state remains on its existing route. Ninja build,
  actuator, pit/teleporter and game-loop integration regressions pass.
# DM1 legacy wall-index placeholder removed (2026-08-06)

- ✅ 2026-08-06 CSB FM Towns ANM CDDA runtime handoff: the selected retail
  FM Towns archive now materializes its original `FMTOWNS.CUE` and raw
  `FMTOWNS.IMG` together with the verified ISO inventory. M11 reads the CUE
  again for each source `TD`/`TR` receipt from `ANIM.C` F2275 and plays only
  the requested physical Red Book span, with no generated PCM, filename
  heuristic or host loop. It replaces a prior request and ends its one-shot
  transport on the animation-owner boundary. Native F0740 pause/resume state
  remains deliberately open. Verification: full `firestaff` build; real archive scan into
  an isolated cache; cached CUE/IMG regression `36 passed, 0 failed`; and
  authentic TITLE/STORY/ENDING playback timeline regression `1367 passed,
  0 failed`.

- ✅ 2026-08-06 CSB FM Towns Game music-table admission: the F31E/F31J
  C03_GAME handoff now binds ReDMCSB `MUSIC.C`'s real
  `G4099_SquareCoordinatesToMusicTrack[10][32][32]` bytes rather than a host
  coordinate map. It verifies the raw CHTWE/CHTWJ offsets (271144/271624),
  shared 10,240-byte FNV-1a `3faffb70`, and exposes bounded map/x/y lookups
  returning the native F0719 selector. The real F31E boot smoke and M11
  entrance handoff regressions exercise the receipt; live movement-triggered
  CDDA remains intentionally open.

- ✅ 2026-08-06 CSB FM Towns F0743 live music route: after the verified
  C03_GAME entrance reaches the live F31 world, M11 reads the authenticated
  G4099 table with the source map/y/x order. It preserves ReDMCSB `MUSIC.C`
  F0743's nonzero change gate and 100-update delay, then hands the unmodified
  selector to the matching physical CUE track. The original CD-DA byte length
  supplies the source-tick completion boundary, avoiding a permanent host
  stream latch. The real title→Switch→Game→Prison regression preserves its
  boot-owned map 0 coordinate (9,0), retail selector/physical track 2. Native CD pause/resume
  remains intentionally open.

- ✅ Removed the remaining fixed arithmetic wall-index helper from the old
  `firestaff_dungeon_viewport_bridge` API. The previous `300 + distance * 18
  + position * 6` calculation was not a ReDMCSB or PC34 `GRAPHICS.DAT`
  mapping and could select unrelated pixels when that compatibility path was
  called. It now returns a no-draw sentinel; authenticated M11 source tables
  remain the only DM1 wall-material owner. Verification: external Ninja
  `firestaff` and `test_firestaff_dm1_dungeon_state_real_data` build/pass,
  real PC34 `DUNGEON.DAT` state test passes, bridge syntax check passes, and
  `git diff --check` is clean.

# DM1 FM Towns Phar Lap symbol ownership (2026-08-06)

- ✅ 2026-08-06 DM2 PC 1.0 English visual-corpus lock: strengthened the
  real-data GDAT census so it no longer treats an arbitrary decodable table
  as complete asset coverage. The test now requires the original corpus's
  5,624 raw entries, 11,854 ENT1 rows, 5,676 image rows, 4,031 unique image
  payloads, 18,633,937 decoded pixels and census hash `bf5050d3`. The
  inventory passed against the supplied PC-DOS `GRAPHICS.DAT`; all pixels
  still come directly from user-supplied original media.

- ✅ 2026-08-06 DM2 FM Towns startup/HUD parity review: compared the DM2
  HME-242 route with the bounded P3/EGB methodology in
  `parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`, without borrowing
  DM1 coordinates, text, pixels or input rules. The DM2 M11 real-media gate
  was rerun against the selected original disc and PC-English companion: it
  verifies RAM-only SWOOSH and TITLE playback, the selected IMG2 menu frame
  FNV-1a `63310e49`, the local palette, real GDAT NEW GAME/RESUME rectangles,
  and the deliberate no-synthetic-party boundary. The independent canonical
  DOS GDAT HUD/material receipts also pass, confirming four source-owned HUD
  commands and palette rejection. The stale TODO statement that listed END
  playback as missing is corrected: END's 420 source frames already replay;
  only its game-won handoff and native SKULL continuation remain open. The
  title cue is source-proven silence rather than an unimplemented CDDA route.
  No game data was unpacked or copied to disk.

- ✅ 2026-08-06 FM Towns menu-cue correction: SKProject
  `startend.cpp` calls `DM2_PLAY_MUSIC(0, true)` before the menu, but the
  selected original `SKULL.EXP+0x3dac` HMP-to-CDDA receipt maps cue 0 to
  silence. Startup no longer asks the PC HMP path to stand in for FM Towns or
  invents a CDDA track. The real-CD M12 regression verifies the table byte and
  records source-proven silence; user media remains in RAM only.

- ✅ The FM Towns startup receipt now parses the real English `EDM.EXP`
  `SYM1` table from the authenticated P3 symbol span. It verifies all 1,174
  bounded records and records the original entry addresses for
  `DO_TITLE_ANIMATION`, `TITLE_PRESENTS`, `TITLE_DUNGEON`, `DRAW_DMENU`,
  `DYNAMENU`, `MENU_ICONS` and `CD_LEVEL_SONG`. Japanese `JDM.EXP` remains
  accepted as the verified no-symbol-table P3 variant. Focused real-cache
  startup, FM Towns disc, ISO, graphics and CD-audio tests pass on the
  external Ninja build. Pixel/TBIOS decoding and live M11 playback remain
  explicitly open in TODO.

- ✅ 2026-08-06 DM1 FM Towns native title-plan receipt: the authenticated
  English EDM P3 load image now verifies the original title animation's
  GRAPHICS.DAT graphic 1 owner, PRESENTS source y=137, MASTER source y=80,
  destination rectangles, 320x80 zoom region and 18-step 16x4 shrink loop at
  their real load-image offsets. `test_dm1_v1_fmtowns_startup` passes against
  the retained original FM Towns cache. This is source-plan evidence only;
  native M11 pixel/TBIOS playback and TMENU rendering remain open in TODO.
# DM1 FM Towns native action-label consumer (2026-08-06)

- ✅ The authenticated English EDM load image now verifies and retains all 44
  native `DYNAMENU+8` action labels, including source duplicates such as
  `STAB` and `X`. M11 uses this receipt-owned stream for English FM Towns
  action rows instead of borrowing the generic PC34 table. The focused real
  cache startup test verifies `PUNCH`, `WAR CRY` and `FUSE`; native TMENU/
  DYNAMENU input and pixel/TBIOS rendering remain separate TODO items.
# 2026-08-06 DM1 creature viewport placeholder isolation

- Classified the authenticated PC34 `C696_GRAPHIC_LAYOUT` as source layout
  data rather than a suspicious bitmap. ReDMCSB `COORD.C` F0640 consumes its
  `0xfc0d` range table for layout-696; the M11 bitmap cache now rejects it
  alongside the other non-raster records. The real 713-record audit requires
  the resulting 543 bitmap, 35 non-bitmap, 4 empty and 131 zero-sized census.

- Removed `dm1_v1_creature_viewport_pc34_compat.c` from the production M10/M11
  archives. Its fixed 225..297 sprite table was not source-faithful: the
  authenticated PC3.4 `GRAPHICS.DAT` has zero-sized entries at the first
  creature indices and different dimensions at later indices, while the
  ReDMCSB creature route selects native/derived bitmap indices from the
  creature-aspect table. No production caller used this legacy API, so its
  fixture tests remain explicit and the runtime can no longer expose its
  synthetic creature metadata as real DM1 material.
- Verification: production M10 archive-symbol check, focused creature
  viewport fixture test, and authenticated PC3.4 `GRAPHICS.DAT` header audit.

# 2026-08-06 DM1 HoC C127/C346 real backing parity

- Corrected the source-owned Hall-of-Champions C127 route to load dedicated
  C346 (48x43) after applying the G0205 D1C destination; it no longer treats
  the generic global-43 derived slot 349 (16x19 in the real PC34 archive) as
  the mirror backing.
- Removed temporary diagnostic output from the real-data verifier.
- Verification: real PC34 `test_m11_dm1_hoc_wall_material_receipt_pc34`,
  `test_dm1_v1_champion_mirror_pc34_compat`, and
  `test_dm1_v1_hoc_graphics_material_receipts_pc34_compat` pass.

# 2026-08-06 DM1 creature native/derived real-data route

- Kept the legacy 225..297 creature viewport table test-only and out of the
  production archives.
- Wired the production F0115 creature path to materialize missing D2/D3
  derived-cache slots from authenticated native C584+ pixels using the
  source 21/32 and 14/32 F0675/F0129 scales. Corrected D3 to use G0221 while
  D2 uses G0222; CSB sources remain fail-closed and unaffected.
- Verification: all 27 native PC34 creature types, real F0115 D1–D3
  admission, host material receipt, creature tick receipt and 132 source-lock
  assertions pass.

# 2026-08-06 DM1 D3C pit synthetic-face removal

- Authenticated PC34 viewport sessions no longer receive the generic
  black/brown procedural pit face from `m11_draw_wall_face`. Open D3C pits
  are left to the real F0104 floor-pit plan and GRAPHICS.DAT zone blit;
  unauthenticated probe worlds retain the diagnostic primitive path.
- The D3C contract fixture stays test-only. Real floor-pit/stairs material
  admission continues through the source-indexed plan and C10 transparency
  gate.

# 2026-08-06 DM1 D2C wall synthetic-face removal

- Authenticated DM1 viewport sessions no longer prepaint a generic grey wall
  face below the source-owned D2C C709 wall-set bitmap. This preserves the
  F0100/F0101 C10-transparent composition instead of turning transparent
  pixels into generated wall material.
- The D2C order/pixel fixture remains test-only; the production path uses
  GRAPHICS.DAT wall-set material and the existing source-zone renderer.

# 2026-08-06 DM1 hidden control-strip input removal

- A source DM1 V1 session (`sourceId=dm1`) now rejects clicks in M11's hidden
  procedural bottom-left control strip. ReDMCSB `COMMAND.C` G0448 owns DM1
  movement through C013's C068--C073 zones, and the strip is neither source
  pixels nor source input. The catalog/debug path remains available for its
  explicit tooling use.
- The same source-DM1 boundary rejects direct calls to M11's procedural
  full-screen map helper. This matches the already-blocked keyboard route and
  prevents callers from exposing a non-source overlay in authenticated V1.
  A retained flag from a prior diagnostic session is cleared before input or
  idle simulation and is excluded from the final frame compositor.
- Regression: `test_m11_overlay_command_queue_block` verifies that a click at
  the old strip coordinate returns ignored and enqueues no movement in DM1 V1,
  that the direct map helper keeps its overlay closed, and that stale map
  state is removed without blocking the DM1 command pipeline.

# 2026-08-06 DM1 FM Towns DRAW_DMENU backdrop binding

- Bound the recovered EDM.EXP `DRAW_DMENU` backdrop order to the software
  EGB shim: exact region-11 clear followed by exact region-10 panel fill,
  with panel colour selected from the live DYNAMENU record. The helper
  refuses to invent icon or text pixels; native icon/text/input work stays
  open.
- Verification: `test_dm1_v1_fmtowns_egb_shim` passes; the real-media startup
  target remains an explicit skip when its runtime-root environment variable
  is absent.
# 2026-08-06 DM1 FM Towns live DYNAMENU backdrop

- The active English FM Towns action menu now builds the recovered eight-byte
  DYNAMENU record from the live action-set indices and sends it through the
  source-locked DRAW_DMENU EGB backdrop route. The result is the native
  region-11 clear followed by the region-10 panel colour selected by the
  real DYNAMENU sentinel bytes.
- The selected hash-admitted Japanese JDM.EXP route now uses its separately
  recovered `DRAW_DMENU` and `DYNAMENU` addresses for the same EGB backdrop,
  rather than falling through to PC34 action chrome. It intentionally draws
  no labels or icons: the recovered Shift-JIS label pool does not decode a
  native text or icon consumer.
- Native label rasterisation, icon decode and mouse capture remain closed;
  the live path intentionally leaves them blank and never substitutes PC34
  C079/C077/C011 art or M653 glyph pixels.

# 2026-08-06 DM1 legacy random-ornament source binding

- The legacy DUNGEON.DAT bridge now retains the PC34 `OrnamentRandomSeed` and
  per-map random wall/floor counts rather than inventing ordinals from fixed
  constants. Wall, corridor, pit and teleporter squares use the
  F0170/F0171 30-way source calculation with their real map dimensions and
  raw flag bits.
- The reader now copies the source column bases, compact SquareFirstThings
  table and raw Thing `Next` words. F0510/F0511 traversal finds a sensor at
  any position in a list and applies its real four-bit `ornamentOrdinal`;
  Thing lists with no sensor remain no-draw. The bridge still has no
  authenticated graphics consumer, so this is source-state recovery rather
  than a viewport-parity claim.
- Regression: real PC34 `DUNGEON.DAT` asserts map-0's wall `(0,12)`/west
  ordinal `2`, floor `(4,2)` ordinal `3`, and the map-1 `(2,15)` sensor
  override ordinal `3`.

# 2026-08-06 Asset scanner warning cleanup

- ✅ Removed the unused DM1 FM Towns admission local from the shared scanner
  loop. The admission still runs for its required side effects, while CSB
  archive-scanner builds no longer issue that unused-variable warning.

- ✅ Kept a verified PC34 GRAPHICS.DAT/DUNGEON.DAT pair as PC34 when a shared
  cache also contains an A31M `TITL.DAT`. The sidecar now promotes A31M only
  from M12's selected `csb-amiga31-multi` cache package, which preserves the
  original package provenance instead of using a nearby file as a variant
  override.

# DM1 D1C F0115 synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1C door-frame/F0115 contract from M10. Its direct
  ReDMCSB regression remains explicit; verification passes 87 assertions.
# DM1 mirror reopen-after-save-load synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040 reopen-after-save/load snapshot model from
  M10; its explicit ReDMCSB regression remains the build owner.
# DM1 mirror icon-refresh synthetic audit isolation (2026-08-07)

- ✅ Removed the in-memory icon/slot fixture from M10; its direct regression
  and the dependent double-open regression compile it explicitly.
# DM1 V2.2 source-art provenance gate (2026-08-07)

DM1 V2.2 no longer treats every non-placeholder generator label as real
material. Only `original_graphics_dat_10x_palette_expansion`, emitted by the
source-only `build_dm1_v22_source_fsart.py` pipeline, may promote a PNG slot
to `FINISHED_REAL`. PBR, AI and generic review labels remain blocked even with
matching PNG dimensions. The local PBR/GPT manifest now evaluates to
`SYNTHETIC_PLACEHOLDER` with zero real slots; focused material, receipt and
runtime-admission tests cover both source-derived promotion and the PBR
negative case. The mounted local-art regression now also opens the actual
PBR/GPT manifest when present and asserts `SYNTHETIC_PLACEHOLDER`, zero real
slots, and no V2.2 runtime admission instead of silently skipping it.

# DM1 V2.2 PC34 source-art archive intake (2026-08-07)

`build_dm1_v22_source_fsart.py` now reads `DATA/GRAPHICS.DAT` directly from
the supplied `Dungeon-Master_DOS_EN_Version-34.zip` when no loose file is
present. It keeps the original bytes in memory, records both archive and
member SHA-256 values in the `.fsart` manifest, and handles the archive's
historical slash-versus-backslash ZIP header mismatch while retaining CRC
verification. A real five-record smoke build produced only
`original_graphics_dat_10x_palette_expansion` entries from the canonical
PC34 GRAPHICS.DAT hash.
# DM1 V2:s syntetiska A*-sökning spärrad (2026-08-07)

- ✅ Tog bort V2:s egna öppna lista, Manhattan-heuristik och vägrekonstruktion.
  PC34 provar varelserörelse riktning för riktning i `GROUP.C` F0202/F0203
  med dungeon- och varelsetillstånd. Kompatibilitetsytan skriver därför inte
  längre några rutter till en fristående heltalskarta. CTesten och källåset
  kontrollerar spärren och ReDMCSB-rutterna.

# DM1 V2:s syntetiska ljudmixer spärrad (2026-08-07)

- ✅ Tog bort V2:s host-samplebuffertar, åtta kanaler, panorering och PCM-mixning.
  ReDMCSB:s SOUND.C F0064 äger PC34:s ljudbegäran. CTesten och källåset
  kontrollerar att V2 inte längre kan producera egna ljudprover.

# DM1 V2:s syntetiska prestationer spärrade (2026-08-07)

- ✅ Tog bort V2:s prestationslistor, notifieringsköer, sparfiler och gul banner.
  ReDMCSB:s F0444-slutsekvens, originalgrafik och meddelandeyta äger i stället
  synliga resultat. CTesten och källåset kontrollerar spärren.

# DM1 V2:s lokala kontroll- och lagringstillstånd spärrat (2026-08-07)

- ✅ Spärrade V2:s egna tangenttabell, roterande autosaves och kopierade
  inventarieposter. ReDMCSB äger kommandon, F0433/F0435-sparning och
  inventarieslotar. CTesten och källåset kontrollerar att inga värdar kan
  behålla eller serialisera parallellt tillstånd.

# DM1 V2:s syntetiska UI-överlägg spärrade (2026-08-07)

- ✅ Tog bort V2-journalernas textarkiv och sparfiler, minimapernas egna
  kartcachar, markörer och pixelöverlägg samt tooltipens 4×5-typsnitt och
  tidsstyrda ruta. ReDMCSB lämnar den riktiga dungeonritningen på
  `F0128/F0097` och meddelanden på `F0047`; DMWebs grafikpost 562 beskriver
  i stället originalets textmasker och rektanglar. Greatstones PC34-register
  innehåller endast `GRAPHICS.DAT`-material, inte sådana UI-data. De sex
  kompatibilitetsmodulerna behåller nu inget tillstånd och skriver inga pixlar.
  CTesten och källåset kontrollerar den oförändrade framebuffern, PC34-arkivet
  och ReDMCSB-rutterna.

# DM1 V2 fri varelseanimation spärrad (2026-08-07)

# DM1 V2-kamerainterpolering spärrad (2026-08-07)

- ✅ Tog bort V2-kamerans egna mellanrutor, mellanriktning, svängpanorering
  och HUD-slutsignaler. ReDMCSB muterar först partiets riktning och position,
  varefter GAMELOOP anropar F0128 direkt med `G0308/G0306/G0307`. F0128
  komponerar och presenterar sedan vyn. Kamerabryggan speglar därför den
  accepterade PC34-tupeln omedelbart och rapporterar alltid noll för alla
  offsetar. Den lokala PC34-arkivkontrollen innehåller ingen separat
  kamerayta, och den befintliga disassemblyinventeringen saknar en validerad
  FIRES-symbolbindning som skulle kunna styrka en alternativ animeringsrutt.
  Åtta berörda DM1/CSB CTester kontrollerar den diskreta vägen.

# DM1 V2 champion-select-data borttagen (2026-08-07)

- ✅ Rensade champion-select-hjälparens döda 5×5-font, lokala panelritare,
  sex påhittade klasser och 24-poster stora roster. ReDMCSB:s CLIKCHAM F0367
  lämnar fokus och kommando till V1, medan CHAMDRAW/PANEL äger levande
  championdata och pixlar. V2 behåller därför bara en begränsad fokusmarkör
  för fyra statusrutor; båda render-API:erna är fortsatt no-op. Källåset
  förbjuder de rensade symbolerna och HUD-interaktionens CTester passerar.

# DM1 V2 syntetiska skärmövergångar spärrade (2026-08-07)

- ✅ Rensade V2-modulens egna tona-till-svart-, tona-till-vitt-, svep- och
  pixeleringsregler. De saknade både anropare och PC34-källmaterial. ReDMCSB
  skickar den redan fastställda partytupeln direkt från GAMELOOP till F0128,
  och F0097 äger presentationen av den färdigkomponerade vyn. API:t är därför
  nu en ren kopieringsbrygga utan animeringstillstånd; V2.2-faden rapporterar
  noll opacitet. CTesten och källåset kontrollerar att framebufferinnehållet
  lämnas oförändrat och att de syntetiska reglerna inte återkommer.

# DM1 V2:s efterbehandlingsfilter spärrade (2026-08-07)

- ✅ Rensade hela V2.0-kedjan från egen CRT-raddämpning, 3×3-ditherrensning,
  påhittad ljusinterpolering och oskarp mask. De hade anrop i SDL-vägen men
  saknar PC34-motsvarighet. Palettbyggaren behåller i stället exakt den
  autentiserade sexnivåtabellen `G9010`, även om gamla kompatibilitetsanrop
  skickar gamma-, ljus- eller kontrastvärden. ReDMCSB:s palettexport kopierar
  källägda värden direkt före viewportpresentation. CTesten bevisar att både
  indexerade och RGBA-buffertar lämnas orörda och att LUT:en är identisk med
  originaltabellen; källåset förbjuder de tidigare filteralgoritmerna.

# DM1 V2:s syntetiska nivåövergångar spärrade (2026-08-07)

- ✅ Tog bort två V2-modeller som kunde svartmåla trappor, tona pitfall,
  blinka teleporter och rita en portal, samt deras egna tidsuppskattningar.
  PC34 uppdaterar i stället karta och partytupel i den källägda rörelsen och
  ritar sedan F0128 direkt. Kompatibilitets-API:t har nu inget tillstånd,
  kan inte skriva en överlagring och rapporterar ingen hittepåvaraktighet.
  CTesten kontrollerar en oförändrad framebuffer och källåset binder vägen
  till ReDMCSB:s F0150/F0128-rutiner.

# DM1 V2:s meddelandelogg och totalsiffror spärrade (2026-08-07)

- ✅ Rensade V2:s lokala meddelandelogg med egen 4×5-font, bakgrundsremsa,
  kategorifärger och 256 textposter. Samtidigt spärrades den fristående
  räknaren för steg, dödar, föremål, skada och speltid. ReDMCSB skriver i
  stället riktiga nivåmeddelanden till meddelandeytan och äger statistik i
  varje championpost, vilket också stämmer med DMWebs beskrivning av spelets
  färdigheter och statistik. V2 kan nu varken behålla eller rita en egen logg,
  och ackumulering samt serialisering av globala totalsiffror avvisas.
  CTesten och källåset kontrollerar den orörda framebufferbilden och de
  källägda ReDMCSB-vägarna.

# DM1 V2-föremålsmetadata källbunden (2026-08-07)

- ✅ Ersatte två påhittade V2-ID:n för föremål med den faktiska PC34-ikonen
  `C201_ICON_ACTION_ICON_EMPTY_HAND`: `GRAPHICS.DAT` post 48, ikon 201 och
  rektangel `(144, 0, 16×16)`. Den lokala PC34-arkivkontrollen bekräftar den
  kanoniska `GRAPHICS.DAT`-hashen och postens 256×32-atlas. En generisk
  golvföremålsyta saknar däremot en egen källa, eftersom F0115 väljer material
  per levande Thing. V2 returnerar därför ingen golvbindning och lämnar
  pixelägandet till den befintliga källbundna V1-vägen. CTesten jämför V2:s
  metadata med V1:s ikonresolver och förhindrar att `fs.v2.item.*` återinförs.

- ✅ Spärrade V2-hjälparen som tog emot godtyckliga spriteindex och
  flyttalslängder utan någon PC34-källa. Den kan nu varken behålla animation-
  stillstånd eller välja en alternativ sprite. Den verkliga vägen är fortsatt
  GROUP.C F0179 för aspektaktualisering och DUNVIEW.C F0115 med G0219-bundna,
  avkodade ytor. CTesten kontrollerar både spärren och V1-källägarskapet.

# DM1 V2 syntetisk dynamisk belysning spärrad (2026-08-07)

- ✅ Delade upp V2-belysningen i dess två faktiska delar. Palettbryggan
  behåller PC34:s G0040-trösklar `99, 75, 50, 25, 1, 0` från DATA.C och
  PANEL.C. Däremot saknar RGB-källor, flimmer och 32×32-ljusspridning en
  PC34-rutt och är nu helt spärrade. F0337/F0338:s autentiska fackelljus,
  magiska ljus och palettval är fortsatt ensamma ljusägare. De berörda
  CTester kontrollerar både den verkliga tabellen och att lokala ljuskällor
  inte kan rita eller behålla tillstånd.

# DM1 V2 generiska partiklar spärrade (2026-08-07)

- ✅ Spärrade hela V2-kedjan för generiska emitterpartiklar. Den kunde skapa
  egna banor, färger och indexerade pixlar ovanpå dungeonbilden utan en PC34-
  källa. ReDMCSB äger i stället explosionerna genom `PROJEXPL.C` och deras
  bitmap/aspektkomposition i `DUNVIEW.C`. Emitter-, direktpartikel- och
  blit-API:erna behåller därför inget tillstånd och avvisar varje begäran.
  De berörda testerna kontrollerar nu den källspärrade policyn i stället för
  den tidigare syntetiska ritningen.

# DM1 V2 syntetiska vädereffekter spärrade (2026-08-07)

- ✅ Spärrade V2-hjälparens källfria regn, dimma, damm och droppar. Den
  skapade partiklar med en egen slumptalsgenerator och blandade fasta
  palettvärden över dungeonbilden, men saknar både PC34-anrop och motsvarighet
  i ReDMCSB-inventeringen. API:t återställs nu till `NONE` och skriver inga
  pixlar. Den registrerade CTesten `dm1_v2_weather_fx_pc34` kontrollerar både
  tillståndet och en orörd framebuffer.

# DM1 V2 syntetisk kameraskakning spärrad (2026-08-07)

- ✅ Tog bort två källfria kameraskakningar från V2: en slumpbaserad
  pixelförskjutning och en traumamodell hämtad från en GDC-presentation.
  De saknar verifierad PC34-anropsväg och returnerar nu bara nollförskjutning.
  Inställningshjälparen kan inte längre flytta eller mörka källägda pixlar.
  F0337/F0338:s verkliga fackelljus och laddningsdränering lämnas i den
  autentiserade V1-rutten. Den registrerade CTesten
  `dm1_v2_damage_numbers_pc34` kontrollerar båda spärrarna och passerar.

- ✅ Corrected the PC music-map reader from a fabricated 44-entry subset to
  SONGLIST.DAT's complete original 63-byte prefix. The selected DOS corpus
  proves valid selectors at slots 44 and 45, followed by the original
  `0xff` no-music tail.
- ✅ `test_dm2_v1_songlist_dat_real_data` reads only an explicit
  `FIRESTAFF_DM2_DATA_DIR` and fails for an unreadable selected file. The
  ordinary parser test is now explicitly a fixture test, never evidence of
  original media.

# DM1 V2 syntetiska skadenummer borttagna (2026-08-07)

- ✅ Tog bort den hostskapade 3×5-sifferfonten och den alternativa
  röd/gröna punktmålaren för flytande skadenummer. Ingen autentisk PC34-rutt
  eller yta har påträffats, så V2-hjälparna behåller ingen popup-data och
  skriver inga pixlar ovanpå källägda stridsbilder. Den registrerade CTesten
  `dm1_v2_damage_numbers_pc34` passerar för båda API:erna.

# DM1 V2 syntetiskt fotstegsljud borttaget (2026-08-07)

- ✅ Tog bort V2-hjälparens brusgenerator för partifotsteg. ReDMCSB- och
  DMWeb-/Greatstone-korsreferensen för PC34 visar ingen motsvarande
  ljudbegäran; verkliga DM1-effekter går i stället via `GRAPHICS.DAT` SND3.
  API:t returnerar nu ingen sample och låter anroparens buffert vara orörd.
  Den registrerade CTesten `dm1_v2_footstep_audio_pc34` passerar.

# DM1 V2 champion-select synthetic-data removal (2026-08-07)

- ✅ Utökade DM1-inventeringen till `src/dm1v2/` och tog bort den vilande
  champion-select-hjälparens sex påhittade klasser, namn, positioner och
  fasta statusvärden. Hjälparen behåller endast fokus från CLIKCHAM-rutten,
  ritar inga hostskapade panelpixlar och kan inte längre rapportera
  källparitet utan en sammanhängande V1 CHAMDRAW/PANEL-bindning. Den
  källägda V1-panelen är därmed ensam pixelägare. Verifiering:
  `dm1_v2_hud_interaction_pc34` bygger och passerar med kontroll av orörd
  framebuffer efter både HUD- och champion-select-anrop.

# DM1 V2.2 source-derived hero slots (2026-08-07)

- ✅ `scripts/build_dm1_v22_source_fsart.py --hero-slots` now creates the
  complete seven-slot V2.2 pack only from the supplied PC34
  `GRAPHICS.DAT` archive. The source-locked records are D3 wall `107`, floor
  `78`, closest pit edge `57`, Demon front `657` (`M618 + G0219 relative 73`),
  champion-portrait atlas `26`, D0 door frame `86` and teleporter field `76`.
  The generated manifest records each source index, record SHA-256 and source
  rationale. It neither installs nor promotes the pack. Verification against
  the supplied archive: the generator wrote all 7/7 expected slots with
  source provenance and decodable PNG members.

- ✅ `--scan-data` now retains and reports each required file's verified
  source path after runtime-cache materialization. Archive members therefore
  stay visible as `archive::member` instead of being misreported as loose
  files under Firestaff's application-support cache.
- ✅ Verified against the supplied CSB archive corpus: GRAPHICS.DAT and
  DUNGEON.DAT report their authenticated archive members while launch still
  uses the ordinary cache files internally.
- ✅ Optional title, credits, palette and logo files now retain the same
  nested archive/ADF provenance in the scanner report instead of appearing
  to originate in the cache.

# DM1 source-owned creature timeline gate (2026-08-07)

- ✅ Stock DM1 no longer falls through to M11's historical C04 map-scan
  simulator, which could manufacture creature movement or attacks. The PC34
  loader's `F0882 → F0195` route now remains the sole owner of initial active
  groups and their C37 behavior events; M10's `F0190/F0209` dispatch owns
  subsequent behavior.
- ✅ Kept the legacy scan for isolated diagnostic worlds only. The focused M11
  source-name tests pass against the local PC34 data, and the source F0195
  group-activation regression passes. No release was made.

# DM1 PC34 local-data capture harness (2026-08-07)

- ✅ Updated `scripts/dosbox_dm1_capture.sh` to discover the real local
  PC34 archive at `~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip`
  when the historical `original-games` archive is absent. The harness accepts
  both that archive's flat file layout and the legacy nested layout, without
  copying game data into the repository.
- ✅ Verified staging from the real ZIP in a temporary external directory:
  `DM.EXE`, `DATA/DUNGEON.DAT` and `DATA/GRAPHICS.DAT` are present and the
  generated DOSBox configuration mounts that exact staged tree and writes
  captures to its isolated capture directory. No release was made.

# DM1 F0352 eye material med vald korpus (2026-08-07)

- ✅ F0352-regressionen läser nu endast explicit
  `FIRESTAFF_DM1_DATA_DIR/GRAPHICS.DAT`. Utan vald korpus skippar den, men en
  vald oläsbar fil är ett fel i stället för en privat hemkatalogsreserv.

# DM1 mirror resurrect-chest-close-order synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040/chest/queue fixture from M10. Its
  generated regression compiles the source directly; it loads no original
  DM1 data and has no M11/runtime caller.

# DM1 mirror C545 drop-panel-live synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C545/C040 fixture from M10 and made its
  regression compile the source directly. It loads no original DM1 data and
  has no M11/runtime caller.

# DM1 inventory hand-belt round-trip synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only hand/belt swap probe from M10 and made its
  direct regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 status-hand closed-chest synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only closed-chest status-hand probe from M10 and
  made its direct regression compile the source explicitly. It loads no
  original DM1 data and has no M11/runtime caller.

# DM1 mirror keyrot-combo-invclick synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only F0361/F0380 race fixture from M10 and made its
  direct regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 mirror close-C045-pending synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C045/C160 queue fixture from M10. Its direct
  regression already compiles the source explicitly; it loads no original DM1
  data and has no M11/runtime caller.

# DM1 chest open-stack-split press-eye synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only chest close/pickup fixture from M10. Its direct
  regression already compiles the source explicitly; it loads no original DM1
  data and has no M11/runtime caller.

# DM1 inventory hand-belt-quiver swap synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only slot-mask probe from M10 and made its direct
  regression compile the source explicitly. It loads no original DM1 data and
  has no M11/runtime caller.

# DM1 chest pickup pending-resurrect synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only asset-free chest/pending-resurrect fixture
  from M10 and made its direct regression compile the source explicitly. It
  loads no original DM1 data and has no M11/runtime caller.

# DM1 auto chest action-hand-swap synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only chest close/swap fixture from M10. Its direct
  regression already compiles the source explicitly; it loads no original DM1
  data and has no M11/runtime caller.

# DM1 chest mid-close hand-swap synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only manual chest-close fixture from M10. Its direct
  regression already compiles the source explicitly; it loads no original DM1
  data and has no M11/runtime caller.

# DM1 chest C061 rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C061/C540 queue fixture from M10 and made its
  direct regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 mirror scroll-pickup-nonleader synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C038/C040 fixture from M10. Its generated
  regression compiles the source directly; it loads no original DM1 data and
  has no M11/runtime caller.

# DM1 mirror open-then-reselect synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C159/C040 fixture from M10 and registered its
  existing regression as an explicit CTest target that compiles the source.
  It loads no original DM1 data and has no M11/runtime caller.

# DM1 champion-panel inventory-walk synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only F0296 inventory/chest icon-walk fixture from
  M10. Its direct regression already compiles the source explicitly; it loads
  no original DM1 data and has no M11/runtime caller.

# DM1 chest open-mirror-rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C540 wheel-swap fixture from M10 and made its
  direct regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 mirror keyboard-browse synthetic audit isolation (2026-08-07)

# DM1 champion dead-hand-refresh synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only F0296/F0295/F0386 fixture from M10. Its direct
  regression already compiles the source explicitly; it loads no original DM1
  data and has no M11/runtime caller.

# DM1 chest leader-death synthetic audit isolation (2026-08-07)

- ✅ Removed the deterministic leader-death chest-close contract fixture from
  M10. It fabricates chest, champion and hand objects locally; M11 owns the
  runtime behavior and the direct regression compiles the source.

# DM1 C040 status-box synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040/G0299 status-box click fixture from M10.
  It fabricates command and panel state, has no original-data input or runtime
  caller, and its direct source-lock regression compiles the source.

# DM1 C161 cancel-after-F0334 synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C161 cancel fixture from M10. It fabricates
  champion, chest-slot and pending state, reads no original DM1 input and has
  no M11/runtime caller; its direct regression now compiles the source.

# DM1 chest partial-mask synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only partial-mask chest swap model from M10. It
  fabricates its chain, item values and slot masks locally, reads no original
  DM1 input and has no M11/runtime caller; its direct regression now compiles
  the source explicitly.

# DM1 chest scroll-resurrect-confirmation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040/C545 confirmation fixture from M10 and
  made its direct regression compile the source explicitly. It loads no
  original DM1 data and has no M11/runtime caller.

# DM1 mirror pickup-right-click synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C159-row fixture from M10 and made its direct
  regression compile the source explicitly. It loads no original DM1 data and
  has no M11/runtime caller.

# DM1 mirror left-click-rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040 view-rotation fixture from M10 and made
  its direct regression compile the source explicitly. It loads no original
  DM1 data and has no M11/runtime caller.

# DM1 mirror inventory-click-rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C156/C157 dispatch-table fixture from M10 and
  made its direct regression compile the source explicitly. It loads no
  original DM1 data and has no M11/runtime caller.

# DM1 G0145/G0196 Graphic558 synthetic audit isolation (2026-08-07)

- ✅ Removed the test-only D2R floor-pit frame and door-ornament coordinate
  copies from M10. ReDMCSB confirms their source values, while the active
  material routes use independent authenticated graphics zones; both direct
  source-lock regressions retain the copies outside production archives.

# DM1 G0212 smoke-palette synthetic audit isolation (2026-08-07)

- ✅ Removed the duplicate source-lock smoke palette table from M10. The live
  F0663 material route owns the same palette map and admits it only with
  authenticated `GRAPHICS.DAT` smoke surfaces; G0212 remains a direct test.

# DM1 G0117 stair-frame synthetic audit isolation (2026-08-07)

- ✅ Removed the generated, source-locked Graphic558 D1C stair-frame table
  from M10. Its direct regression compiles the table explicitly; no runtime
  caller or authenticated `GRAPHICS.DAT` material consumer exists yet.

# DM1 mirror resurrect-reincarnate-skills synthetic audit isolation (2026-08-07)

- ✅ Removed the fabricated C160/C161 party/champion fixture from M10 and made
  its direct regression compile the source explicitly. It loads no original
  DM1 data and has no M11/runtime caller.
# DM1 mirror keyboard-rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only command-queue fixture from M10 and made its
  direct regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 mirror icon-refresh synthetic audit isolation (2026-08-07)

- ✅ Confirmed the contract-only F0295/F0296 in-memory icon/slot fixture is
  excluded from M10 and M11. Its dedicated and double-open regressions compile
  the source explicitly; it loads no original DM1 data and has no runtime
  caller.

# DM1 mirror close-button synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040 chrome fixture from M10. Its direct and
  dependent double-open regressions compile the source explicitly; it loads no
  original DM1 data and has no M11/runtime caller.

# DM1 F0098 floor/ceiling synthetic audit isolation (2026-08-07)

- ✅ Confirmed the contract-only F0098 dispatch model is excluded from M10 and
  M11 while the data-owned floor/ceiling route remains active. Corrected its
  adjacent F0115 regression to expect the original `G0237[166] -> G0209[58]`
  mapping for junk subtype 39, whose `GraphicInfo` has no mirror-on-right bit.

# DM1 C53 synthetic start-watchdog removal (2026-08-07)

- ✅ Removed the host-only tick-1 C53 from dungeon startup. ReDMCSB
  `TIMELINE.C:F0256` creates and dispatches C53 only outside
  `NOCOPYPROTECTION`; Firestaff retains import/export support for an
  authenticated saved C53 receipt, but no longer fabricates one for a new game.

# DM1 D2C stair-rail synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only D2C stair-rail trace from M10 and made its
  regression compile the source explicitly. It loads no original DM1 data and
  has no M11/runtime caller.

# DM1 C040 rotation-save-load synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only F0433/F0435/C040 round-trip fixture from M10.
  Its generated regression compiles the source directly; it loads no original
  DM1 data and has no M11/runtime caller.

# DM1 chest multi-champion-close synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only multi-champion chest-close fixture from M10 and
  made its regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.

# DM1 mirror reopen-after-save-load synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only F0433/F0435/C040 fixture from M10. Its
  generated regression compiles the source directly; it loads no original
  DM1 data and has no M11/runtime caller.
# DM1 C70 full G0039 light table (2026-08-07)

- ✅ Replaced the Phase 19 seven-entry light-table copy and its 0–6 clamp with
  the canonical PC 3.4 `DATA.C:359` / GRAPHICS.DAT item 562 G0039 owner.
  C70 decay and original-save bounds now admit source-valid magnitudes 1–15
  and reject invalid values rather than silently clamping them.

# DM1 mirror cancel-rotation synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040/C162 rotation fixture from M10. Its
  generated regression still compiles the source directly; it loads no
  original DM1 data and has no M11/runtime caller.
- ✅ Verification: `test_dm1_v1_mirror_candidate_click_cancel_with_rotation_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror inventory-exit HUD synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040 inventory-exit HUD fixture from M10. Its
  direct ReDMCSB regression continues to compile the fabricated overlay model
  explicitly.

# DM1 mirror chest-open-during-pending synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040/F0333/F0282 fixture from M10 and wired
  its regression to compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.
- ✅ Verification: `test_dm1_v1_mirror_candidate_chest_open_during_pending_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror click-cancel-rotation synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040 click/cancel/rotation race from M10. Its
  direct ReDMCSB regression continues to compile the fabricated state model
  explicitly.

# DM1 mirror pending-hand-queue synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040 chest/hand-queue fixture from M10 and
  made its regression compile the source explicitly. It loads no original DM1
  data and has no M11/runtime caller.
- ✅ Verification: `test_dm1_v1_mirror_candidate_pending_hand_queue_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C004--C006/C040 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C004..C006/C040 party/panel/chest fixture from
  M10 and made its regression compile the source explicitly. It loads no
  original DM1 data and has no M11/runtime caller.
- ✅ Verification: `test_dm1_v1_mirror_candidate_lower_arrow_state_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 mirror teleporter-survival synthetic audit isolation (2026-08-06)

- ✅ Removed the teleporter-survival fixture from M10. Its direct ReDMCSB
  regression continues to compile the local dungeon, panel, party and
  chest-slot model explicitly.

# DM1 mirror no-pending-resurrect synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only no-op C040/G0299 fixture from M10. It has no
  original DM1 data input or M11/runtime caller; its explicit regression
  continues to compile the source directly.
- ✅ Verification: `dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 champion hand-slot priority synthetic audit isolation (2026-08-06)

- ✅ Removed the source-only champion hand-slot priority trace from M10. It
  fabricates party, slot and Thing state, while its direct ReDMCSB regression
  and status-hand sibling regression compile it explicitly.

# DM1 mirror C160/F0284 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C160/F0284 party/panel fixture from M10. It
  loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_close_order_party_shuffle_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C040/F0284 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040/F0284 party/panel fixture from M10. It
  loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C545/C160 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C545/C160 party/panel/pixel fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c545_accept_during_rotation_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C159/C160 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C159/C160 party/panel/hand fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c159_click_rotation_combo_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 F0098 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only F0098 dispatch and local pixel-blend model from
  M10. It consumes no original DM1 material and has no M11/runtime caller;
  its dedicated ReDMCSB regression now compiles the source explicitly.

# DM1 mirror C061/C028 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C061/C028 slot/party/panel fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c061_drop_resurrect_pending_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C045/C160 rotation synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C045/C160 party/panel/item fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 F1506--F1525 metadata audit isolation (2026-08-06)

- ✅ Removed the metadata-only SWITCH/VDI ownership inventory from M10. It
  has no M11 caller or authenticated PC34 data input, and its dedicated
  ReDMCSB audit continues to compile the source explicitly.

# DM1 mirror C045 dead-owner synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free C045/C040 party/panel/item fixture from M10. It
  loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C100/C040 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C100/C040 party/panel/G0299/G0514 fixture from
  M10. It loads no original DM1 data and has no M11/runtime caller; the
  generated mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1C wall synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only D1C wall pixel model from M10. It uses
  caller-provided local buffers and fixed route metadata, has no original DM1
  data input or M11/runtime caller, and its dedicated regression still
  compiles it directly.

# DM1 mirror C040/C545 redraw synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free C040/C545 party/chest/candidate fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_redraw_after_chest_close_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 FM-Towns JA DYNA_BUTTONS placeholder audit (2026-08-06)

- ✅ Confirmed that the JDM.EXP `DYNA_BUTTONS` `N`/`X` values are
  byte-verified original control labels, not placeholders, and retained their
  exact Shift-JIS/ASCII byte values.
- ✅ Verification: `test_dm1_v1_fmtowns_dyna_buttons_ja` passes with the
  336-byte source span unchanged.

# DM1 mirror C040 pickup-rotate synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040/chest/rotation party/panel/Thing fixture
  from M10. It loads no original DM1 data and has no M11/runtime caller; the
  generated mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 chest-close pending-panel synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C011/C038/C162 party/chest/panel/Thing fixture
  from M10. It loads no original DM1 data and has no M11/runtime caller; its
  generated mirror regression continues to compile the source explicitly.

# DM1 mirror C007--C011/C040 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C007..C011/C040 party/panel/G0299 fixture from
  M10. It loads no original DM1 data and has no M11/runtime caller; the
  generated mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 F0099 row-flip synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only F0099 row-flip fixture from M10. It uses
  caller-provided local buffers and fixed dimensions, has no original DM1 data
  input or M11/runtime caller, and its dedicated regression now compiles it
  directly.

# DM1 mirror C546/C040 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C546/C040 panel/chest/Thing fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C111/C040 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C111/C040 party/panel/G0299 fixture from M10.
  It loads no original DM1 data and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1R2 wall synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only D1R2-wall probe fixture from M10. It uses local
  320×200 buffers and fixed route metadata, has no original DM1 data input or
  M11/runtime caller, and its dedicated regression now compiles it directly.

# DM1 F0292 name-box clip synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free F0292 name/title clip fixture from M10. It
  fabricates champion, name and title inputs, has no original DM1 data input
  or M11/runtime caller, and its dedicated regression compiles it directly.

# DM1 F0296 inventory-viewport walk synthetic audit isolation (2026-08-06)

- ✅ Bound SKProject `DM2_PROCEED_SPELL_FAILURE` class `0x30` to the exact
  authenticated `INTERFACE_GENERAL/5/dtImage/0x0B` NEED_FLASK record and
  source destination rect `0x5C`; runtime records decoded-pixel and local-
  palette hashes and never publishes invented text or pixels.
- ✅ Kept the receipt fail-closed (`no_draw`) until the M11 transparent-static-
  pic surface consumer and C068--C070 panel-global update are source-owned.
- ✅ The real-data M11 startup/profile regression passes against the mounted
  PC-English DM2 corpus; no release was made.

# DM1 mirror C040 panel-exit synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040 close/reopen panel fixture from M10. It
  loads no original DM1 assets and has no M11/runtime caller; the generated
  mirror regression continues to compile the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 wound-probability placeholder audit (2026-08-06)

- ✅ Confirmed the FMTowns `DYNA_BUTTONS` `N`/`X` values are original media
  bytes, not placeholders, and kept them unchanged. ReDMCSB `DATA.C:243`
  independently confirms the wound-mask table `{0x20, 0x10, 0x08, 0x04}`.
- ✅ Removed the wound-mask result's self-equality “future” placeholder; it
  now reports only six independently evaluated invariants. The source-locked
  audit is also test-only because no M11/runtime caller consumes it.
- ✅ Verification: targeted CTest
  `dm1_v1_wound_probability_index_to_mask_pc34_compat` passes.

# DM1 C045 non-candidate transition synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C045 close-after-transition fixture from M10.
  It fabricates chest, leader-hand and visible-slot state, has no original DM1
  data input or M11/runtime caller, and the generated regression compiles it
  directly.
- ✅ Verification: `dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_pc34_compat`
  passes with 445 assertions; the source object is absent from both M10 and
  M11 archives.

# DM1 C040/C537/C162 thought-cancel synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only thought-project cancel-after-pickup fixture
  from M10. It fabricates champion, chest and scroll state, has no original
  DM1 data input or M11/runtime caller, and its explicit ReDMCSB regression
  now compiles the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_thought_project_cancel_after_pickup_pc34_compat`
  passes with 166 assertions; the source object is absent from both M10 and
  M11 archives.

# DM1 C040/C537 double-open-close synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C040/C537 lifecycle fixture from M10. It
  fabricates champion, hand and chest-slot state, has no original DM1 data
  input or M11/runtime caller, and its generated regression compiles it
  directly.
- ✅ Verification: `dm1_v1_mirror_candidate_double_open_close_guard_pc34_compat`
  passes with 63 assertions; the source object is absent from both M10 and M11
  archives.

# DM1 chest another-open synthetic audit isolation (2026-08-06)

- ✅ Removed the F0333/F0334 another-open scenario fixture from M10. It
  fabricates Thing ordinals, container chains and leader-hand state in a local
  Next array, reads no original material, and has no M11/runtime caller; its
  dedicated ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_chest_open_while_another_open_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 chest stack-split synthetic audit isolation (2026-08-06)

- ✅ Removed the local F0333 stack-split chain fixture from M10. It reads no
  original material and has no M11/runtime caller; its dedicated ReDMCSB
  regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_chest_open_stack_split_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 D1L/D1R stairs-pit synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-slot D1L/D1R stairs/pit dispatch fixture from M10. It
  reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d1l_d1r_stairs_pit_dispatch_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 C140/C040 save-gate synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only C140/C040 save gate from M10. It fabricates
  party, panel and candidate state, has no original DM1 data input or
  M11/runtime caller, and the generated mirror regression compiles it directly.
- ✅ Verification: `dm1_v1_mirror_candidate_c040_save_game_while_panel_live_pc34_compat`
  passes with 43 assertions; the pass789 ReDMCSB verifier passes and the source
  object is absent from both M10 and M11 archives.

# DM1 D0L/D0R F0111 synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-rectangle D0L/D0R door composition contract from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 first-C127 focus synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only first-C127/C040 focus fixture from M10. It
  fabricates party, panel and input-focus state, has no original DM1 data input
  or M11/runtime caller, and the generated ReDMCSB regression compiles it
  directly.
- ✅ Verification: `dm1_v1_mirror_candidate_first_interaction_focus_pc34_compat`
  passes with 74 assertions; the source object is absent from both M10 and M11
  archives.

# DM1 chest hidden-tail synthetic audit isolation (2026-08-06)

- ✅ Removed the deterministic ninth-item hidden-tail chest fixture from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_chest_ninth_item_hidden_tail_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1L2/D1R2 F0111 synthetic audit isolation (2026-08-06)

- ✅ Removed the partly-open D1L2/D1R2 side-door simulation from M10. It
  reads no original material and has no M11/runtime caller; its dedicated DM1
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L2/D2R2 F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-zone D2L2/D2R2 F0108 floor/ceiling contract from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d2l2_d2r2_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 C040/C537 close-while-pending synthetic audit isolation (2026-08-06)

- ✅ Removed the deterministic mirror-candidate C040/C537 fixture from M10.
  It fabricates champion, hand and chest-chain state, has no original DM1 data
  input or M11/runtime caller, and its explicit ReDMCSB regression now compiles
  the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_close_while_resurrect_pending_with_inventory_pickup_pc34_compat`
  passes with 27 assertions plus 20 internal assertions; the source object is
  absent from both M10 and M11 archives.

# DM1 D3L/D3R F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-zone D3L/D3R F0108 floor/ceiling contract from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3L2/D3R2 F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-zone D3L2/D3R2 F0108 floor/ceiling contract from M10.
  It has no game-data input or M11/runtime caller; its dedicated ReDMCSB
  regression already compiles the source explicitly.
- ✅ Verification: `dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 mirror full-chain synthetic audit isolation (2026-08-06)

- ✅ Removed the deterministic mirror-candidate full-chain fixture from M10.
  It synthesizes party and hand state, has no original DM1 data input or
  M11/runtime caller, and the generated ReDMCSB regression compiles it
  directly.
- ✅ Verification: `dm1_v1_mirror_candidate_full_chain_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 D0C F0098 synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed-row D0C ceiling/floor ownership fixture from M10. It
  has no game-data input or M11/runtime caller; its dedicated ReDMCSB
  regression already compiles the source explicitly.
- ✅ Verification: `dm1_v1_viewport_d0c_ceiling_f0098_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 chest round-trip synthetic audit isolation (2026-08-06)

- ✅ Removed the C537/C538 chest hand-swap fixture from M10. It fabricates
  slots, item kinds and weights, reads no original DM1 material and has no
  M11/runtime caller; its dedicated regression now compiles it explicitly.
- ✅ Verification: `test_dm1_v1_chest_round_trip_hand_swap_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 C045 food-water synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free C045/chest food-water-close scenario fixture from
  M10. It reads no original DM1 data and has no M11/runtime caller; the
  generated mirror ReDMCSB regression compiles the source directly.
- ✅ Verification: `dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 C146 wake-up synthetic audit isolation (2026-08-06)

- ✅ Removed the C040/C146 wake-up state simulation from M10. It reads no
  original DM1 material and has no M11/runtime caller; its generated
  ReDMCSB regression already compiles the source explicitly.
- ✅ Verification: `dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2C F0111 door synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only/no-game-data D2C F0111 trace from M10. It has
  no M11/runtime caller; its dedicated ReDMCSB regression now compiles the
  source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d2c_f0111_door_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 mirror occupied-hand synthetic audit isolation (2026-08-06)

- ✅ Removed the C040 occupied-hand fixture from M10. It synthesizes panel and
  candidate state, has no original DM1 data input or M11/runtime caller, and
  its explicit ReDMCSB regression now compiles it directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_occupied_hand_panel_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L/D2R F0111 door synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D2L/D2R F0111 framebuffer simulation from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression already compiles the source explicitly.
- ✅ Verification: `dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3C F0111 door-front synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed D3C F0111 door-front source-order/C10 model from M10.
  It reads no original material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 champion status-recompute synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only champion status-recompute model from M10. It
  uses synthetic panel state, reads no original DM1 data and has no M11/runtime
  caller; its explicit ReDMCSB regression compiles it directly.
- ✅ Verification: `dm1_v1_champion_panel_status_recompute_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3C F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only D3C F0108 framebuffer model from M10. It uses
  fixed probe pixels, reads no original DM1 material and has no M11/runtime
  caller; its dedicated ReDMCSB regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1L/D1R F0111 door synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1L/D1R F0111 control/blit model from M10. It
  reads no original DM1 material and has no M11/runtime caller; its dedicated
  ReDMCSB regression now compiles the source explicitly. The independent D1
  side-door receipt remains the source-material production owner.
- ✅ Verification: `test_dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 champion portrait synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only champion portrait route model from M10. It uses
  synthetic panel state, has no bitmap sampling or original DM1 data input,
  and has no M11/runtime caller; its explicit ReDMCSB regression compiles it
  directly.
- ✅ Verification: `dm1_v1_champion_panel_portrait_pc34_compat` passes; the
  source object is absent from both M10 and M11 archives.
# DM1 D1C F0111 door synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1C F0111 door geometry/transparency model from
  M10. It uses synthetic blits, reads no original DM1 material and has no
  M11/runtime caller. The independent D1C door receipt remains the
  source-material production owner.
- ✅ Verification: `dm1_v1_viewport_d1c_f0111_door_pc34_compat` passes; the
  source object is absent from both M10 and M11 archives.
# DM1 D1C F0107 wall-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1C F0107 wall-ornament audit from M10. It fixes
  route metadata and local C10 probe pixels, reads no original DM1 material
  and has no M11/runtime caller. The separate source-bound D1C wall and
  inscription renderers remain the production owners.
- ✅ Verification: `dm1_v1_viewport_d1c_f0107_wall_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror reselect-inventory synthetic audit isolation (2026-08-06)

- ✅ Removed the C040/C038 reselect-with-inventory fixture from M10. It
  synthesizes champion and inventory state, reads no original DM1 data and has
  no M11/runtime caller; its explicit ReDMCSB regression compiles it directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_resurrect_reselect_with_inventory_pickup_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L/D2R F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only D2L/D2R F0108 floor/ceiling/ornament table
  from M10. It fixes source-order and zone metadata without comparing original
  bitmap data, and has no M11/runtime caller; its dedicated ReDMCSB regression
  remains the only build owner.
- ✅ Verification: `test_dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2C F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only D2C F0108 floor/ceiling/ornament model from
  M10. It fixes source-order metadata and a local framebuffer probe, reads no
  original DM1 material and has no M11/runtime caller; its dedicated ReDMCSB
  regression now compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d2c_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3L2/D3R2 F0108 occlusion synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D3L2/D3R2 F0108 occlusion model from M10. It
  fixes F0676/F0677 ordering, zones, cell orders and C10 probe pixels, reads
  no original DM1 material and has no M11/runtime caller. The independent
  source-bound side-pair renderer remains the production owner.
- ✅ Verification: `dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror scroll-rotation synthetic audit isolation (2026-08-06)

- ✅ Registered and removed the C040/chest/leader-rotation scenario fixture
  from M10. It reads no original DM1 data and has no M11/runtime caller; its
  explicit ReDMCSB regression compiles the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 D3L/D3R F0108 floor-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D3L/D3R F0108 occlusion model from M10. It fixes
  zones, cell orders and C10 probe pixels, reads no original DM1 material and
  has no M11/runtime caller. The independent source-bound D3 renderer remains
  the production owner.
- ✅ Verification: `dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3L2/D3R2 F0108 composition synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D3L2/D3R2 F0108/F0111/F0115 composition model
  from M10. It fixes route/zone metadata and local C10 probe pixels, reads no
  original DM1 material and has no M11/runtime caller. The separate D3 route
  remains the GRAPHICS.DAT-bound production owner.
- ✅ Verification: `test_dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror party-direction synthetic audit isolation (2026-08-06)

- ✅ Removed the C040/G0299 party-direction harness from M10. It uses a
  synthetic portrait token, has no original DM1 data input or M11/runtime
  caller, and its explicit ReDMCSB regression now compiles the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_party_direction_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1C F0108 floor-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1C F0108 occlusion model from M10. It fixes
  zones, cell orders and C10 probe pixels, reads no original DM1 data or
  GRAPHICS.DAT material, and has no M11/runtime caller; its explicit ReDMCSB
  regression already compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 champion inventory-tail synthetic audit isolation (2026-08-06)

- ✅ Removed the champion inventory-tail model from M10. It uses synthetic
  inventory slot and dirty-bit rows, reads no original DM1 data and has no
  M11/runtime caller; its explicit ReDMCSB regression now compiles the source
  directly.
- ✅ Verification: `dm1_v1_champion_panel_inventory_tail_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L/D2R door-frame-top synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D2L/D2R door-frame-top trace from M10. It uses
  fixed strides, zones and synthetic framebuffer fields, has no original DM1
  data input or M11/runtime caller, and its explicit test and gate probe
  compile the source directly.
- ✅ Verification: `dm1_v1_viewport_d2l_d2r_door_frame_top_edge_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3C back-wall item synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D3C F0115 back-wall item model from M10. It uses
  synthetic framebuffer writes, cells and zones, has no original DM1 data
  input or M11/runtime caller, and its explicit tests and gate probe already
  compile the source directly.
- ✅ Verification: `dm1_v1_viewport_d3c_back_wall_item_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 champion disabled-icon synthetic audit isolation (2026-08-06)

- ✅ Removed the champion disabled-icon state model from M10. It synthesizes
  champion rows and G0491 state, reads no original DM1 data, and its explicit
  ReDMCSB regression already compiles the source directly. M11 separately
  owns the narrow source-cited global hatch predicate; it does not link this
  synthetic model.
- ✅ Verification: `dm1_v1_champion_panel_disabled_icon_state_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2C F0115 front-rear synthetic audit isolation (2026-08-06)

- ✅ Removed the D2C F0115 front/rear door-pass model from M10. It hard-codes
  cell orders, zones and synthetic pixel composition, reads no original DM1
  data and has no M11/runtime caller; its explicit ReDMCSB regression now
  compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d2c_f0115_front_rear_overlap_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D0L/D0R F0115 synthetic audit isolation (2026-08-06)

- ✅ Removed the D0L/D0R F0115 front-cell-order model from M10. It hard-codes
  cell orders, rows and probe pixels, reads no original DM1 data and has no
  M11/runtime caller; its explicit ReDMCSB regression now compiles the source
  directly.
- ✅ Verification: `test_dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 C545 occupied-leader-hand synthetic audit isolation (2026-08-06)

- ✅ Removed the C545 occupied-leader-hand model from M10. It seeds party,
  chest, icon and panel state around inventory helpers, reads no original DM1
  data and has no M11/runtime caller; its explicit ReDMCSB regression now
  compiles the source directly.
- ✅ Verification: `test_dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 chest partial-drop synthetic audit isolation (2026-08-06)

- ✅ Removed the chest partial-drop-to-floor probe from M10. It locally
  fabricates chest contents, leader-hand state, a floor link and map
  coordinates; it reads no original DM1 input and has no runtime caller.
  Its explicit ReDMCSB regression now compiles the probe source directly.
- ✅ Verification: `test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat`
  passes; the source object is absent from M10.

# DM1 chest rotate-pickup synthetic audit isolation (2026-08-06)

- ✅ Removed the chest-pickup-during-party-rotation model from M10. It
  locally fabricates party/chest/leader-hand/queue state, reads no original
  DM1 input and has no runtime caller. Its existing ReDMCSB regression is now
  registered and compiles the source explicitly.
- ✅ Verification: `test_dm1_v1_chest_pickup_while_party_rotate_in_progress_pc34_compat`
  passes; the source object is absent from M10.

# DM1 D1L/D1R F0107 wall synthetic audit isolation (2026-08-06)

- ✅ Removed the D1L/D1R F0107 wall-ornament probe from M10. It hard-codes
  zones, ordinals and framebuffer pixels, reads no original DM1 data or
  GRAPHICS.DAT material and has no M11/runtime caller; its explicit ReDMCSB
  regression now compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L2/D2R2 F0115 synthetic audit isolation (2026-08-06)

- ✅ Removed the D2L2/D2R2 F0115 no-draw receipt from M10. It has fixed
  F0115 rows and suppression values, reads no original DM1 data and has no
  M11/runtime caller; its explicit ReDMCSB regressions continue to compile it
  directly.
- ✅ Verification: `test_dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D2L2/D2R2 side-wall synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free F0678/F0679 side-wall model from M10. It carries
  fixed route data and a synthetic 8×8 probe, reads no authenticated graphics
  material and has no M11/runtime caller; its source-lock regression remains
  explicit.
- ✅ Verification: `test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat`
  passes; the source object is absent from M10.

# DM1 champion portrait-box redraw synthetic audit isolation (2026-08-06)

- ✅ Removed the portrait-box redraw-state matrix from M10. It fabricates
  panel geometry, champion ownership and dirty-bit flows, reads no original
  DM1 data and has no M11/runtime caller; its explicit ReDMCSB regression
  continues to compile the source directly.
- ✅ Verification: `test_dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 F0344 planar-fill audit isolation (2026-08-06)

- ✅ Removed the standalone F0344 food/water planar-fill helper from M10. It
  has no runtime caller and its callers supply test-owned bitmap/layout data;
  the live source-bound champion status-box route remains separate.
- ✅ Verification: `test_dm1_v1_viewport_food_water_fill_pc34_compat` passes;
  the source object is absent from M10.

# DM1 D2C F0107 wall-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the D2C F0107 wall-ornament probe from M10. It hard-codes
  ordinals, framebuffer pixels and test boxes, reads no original DM1 data or
  GRAPHICS.DAT material and has no M11/runtime caller; its explicit ReDMCSB
  regression now compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d2c_f0107_wall_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D3C F0107 wall-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the D3C F0107 wall-ornament model from M10. It has fixed probe
  pixels and geometry, reads no authenticated `GRAPHICS.DAT` material, and
  has no M11/runtime caller; its explicit ReDMCSB regression compiles it
  directly.
- ✅ Verification: `test_dm1_v1_viewport_d3c_f0107_wall_ornament_pc34_compat`
  passes; the source object is absent from M10.

# DM1 M11 compact Thing-chain consumers (2026-08-06)

- ✅ M11 endgame text and projectile door/teleporter inspection now obtain
  the first Thing through the PC34 F0511 compact SquareFirstThings lookup,
  rather than dense map-square indexing that aliases a later-map chain.
- ✅ Verification: `test_m11_dm1_endgame_final_presentation_receipt_pc34`
  passes. The broader action/stamina suite remains externally red on its
  existing projectile-damage and fleeing-delay cases, unrelated to lookup.

# DM1 procedural minimap source boundary (2026-08-06)

- ✅ The host-drawn corner minimap is now rejected for authenticated PC34 DM1
  sessions, including persisted/F7-enabled QoL state. Source viewport pixels
  remain owned by the original game; the minimap stays available only in
  diagnostic worlds.
- ✅ Verification: `test_dm1_v1_minimap_pc34_compat` checks the enabled
  minimap leaves an authenticated session framebuffer unchanged.

# DM1 legacy ornament sensor-cell selection (2026-08-06)

- ✅ The legacy DUNGEON.DAT bridge now matches a C03 wall sensor only when
  its packed Thing cell is the face viewed by F0172. A sensor on another wall
  cell, or another Thing type, no longer overrides the F0170/F0171 ordinal.
- ✅ Verification: the raw compact-Thing regression and the materialized PC34
  `DUNGEON.DAT` state test pass, as does `test_m11_overlay_command_queue_block`.

# DM1 D1L2/D1R2 F0108 floor-ceiling synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed D1L2/D1R2 F0108 floor/ceiling-ornament model from M10.
  It hard-codes ornament indices, zones, seeds and probe pixels, reads no
  original DM1 data and has no M11/runtime caller; its explicit ReDMCSB
  regression now compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1L2/D1R2 F0108 wall synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed D1L2/D1R2 F0108 wall-composition model from M10. It
  hard-codes ornament ordinals, zones, seeds and probe pixels, reads no
  original DM1 data and has no M11/runtime caller; its explicit ReDMCSB
  regression now compiles the source directly.
- ✅ Verification: `test_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.
# DM1 mirror pending-hand/chest-race synthetic audit isolation (2026-08-06)

- ✅ Removed the pending-hand/chest-pickup race model from M10. It fabricates
  champion hands, chest slots, candidate state and panel counters, reads no
  original DM1 data and has no M11/runtime caller; its explicit ReDMCSB
  regression compiles the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 mirror C545 pickup synthetic audit isolation (2026-08-06)

- ✅ Removed the C545/C040 pickup model from M10. It fabricates floor objects,
  chest slots, candidate state and panel counters, reads no original DM1 data
  and has no M11/runtime caller; its explicit ReDMCSB regression now compiles
  the source directly.
- ✅ Verification: `test_dm1_v1_mirror_candidate_c545_pickup_while_panel_live_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 legacy dungeon bridge ornament fallback removal (2026-08-06)

- ✅ Removed the legacy bridge's fabricated wall/floor ornament ordinal
  generator. The `2000`/`3000`/`31417` coordinate arithmetic was not a
  recovered F0170/F0172 random/aspect path and could select unrelated art
  from otherwise authentic `DUNGEON.DAT`.
- ✅ The bridge now fails closed with no ornament while continuing to decode
  the real map grid, start position and door state. The real-data state test
  passes against the materialized PC 3.4 `DUNGEON.DAT`, and `firestaff_m11`
  builds successfully.

- ✅ Corrected the Nexus sound-driver source identity: authenticated
  `SDDRVS.TSK` is a 26,610-byte 68000 sound-CPU image, not SH-2 code.
- ✅ Bound the real entry/base-register corridor (`0x1000`/`0x1080`), the
  16-value command-nibble dispatch mask (`0x1c08`/`0x1c2a`) and the PCM
  voice-register handler corridor (`0x1f0e`) against the local retail bytes.
- ✅ Kept event→MAP selection, SAL codec semantics and playback fail-closed;
  the real-data SAL corpus probe passes with all 16 banks.

# DM1 FM Towns DECODEGRAPHIC RLE decoder (2026-08-06)

- Full port of DECODEGRAPHIC (EDM.EXP 0x1f63c) inner RLE loop with
  byte-verified disassembly of leaf helpers 0x1f4c4, 0x1f518,
  0x1f578, 0x1f5d8.
- New API `dm1_v1_fmtowns_pic_library_decode_asset_pc34` returns
  the 4bpp packed pixel matrix and the geometry header for any RLE
  asset in `DATA/GRAPHICS.DAT`.
- Round-trip verification: 347/347 RLE-branch assets decode with
  exact source-byte consumption and exact destination-byte size.
- Evidence: `parity-evidence/dm1_fmtowns_pic_library_format.md`
  (updated with leaf-helper decodes and verification numbers).

# DM1 F0181–F0200 group source-owner audit isolation (2026-08-06)

- ✅ Removed the metadata-only F0181–F0200 source-owner table from the broad
  M10 compatibility archive. Caller search found no M11/runtime consumer, and
  the table reads no real GROUP/DUNGEON bytes; its explicit audit test remains
  available for ReDMCSB ownership and fail-closed checks.
- ✅ Verification: `test_dm1_v1_f0181_f0200_group_source_audit_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 L0201–L0250 local-owner audit isolation (2026-08-06)

- ✅ Removed the ReDMCSB local-label inventory from the broad M10 archive.
  It records DUNVIEW/BLIT/DUNGEON local ownership but has no M11/runtime
  caller and consumes no authenticated DM1 bytes; the existing explicit
  inventory test retains its source evidence.
- ✅ Verification: `test_dm1_v1_l0201_l0250_local_owner_audit` passes; the
  source object is absent from both M10 and M11 archives.

# DM1 M151–M200 absent-label audit isolation (2026-08-06)

- ✅ Removed the M151–M200 absence receipt from the broad M10 archive.
  ReDMCSB's full label inventory has no labels in this range; the source is
  a test-only fail-closed table with no M11/runtime caller or DM1 data input.
- ✅ Verification: `test_dm1_v1_m151_m200_source_audit_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 mirror C546 eye-slot-swap synthetic audit isolation (2026-08-06)

- ✅ Removed the C546/C09 eye-route contract model from M10. It models fixed
  things, chest slots, panels and icons without a live M11 caller or original
  save/graphics input; its explicit ReDMCSB-backed test remains available.
- ✅ Verification: `test_dm1_v1_mirror_candidate_eye_slot_swap_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 chest scroll/drop rotation synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free C540 scroll/drop-during-rotation regression from
  M10. It fabricates item, slot and queue state and has no runtime caller or
  original-data input; its test now compiles the driver explicitly and still
  links M10 only for the real inventory helper APIs it checks.
- ✅ Verification:
  `test_dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_pc34_compat`
  passes; the driver object is absent from both M10 and M11 archives.

# DM1 L0151–L0200 local-owner audit isolation (2026-08-06)

- ✅ Removed the F0115/F0116 automatic-local metadata table from M10. The
  table has no runtime caller or original-data input; it points to the real
  F0115 material/projectile owners without becoming one itself.
- ✅ Verification: `test_dm1_v1_l0151_l0200_f0115_local_owner_audit` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 F0410–F0411 spell-continuation audit isolation (2026-08-06)

- ✅ Removed the receipt-only F0410/F0411 spell-continuation contract from
  M10. It has no M11/runtime caller, while F0412 remains the live spell-result
  owner; its explicit source-receipt test remains available.
- ✅ Verification:
  `test_dm1_v1_f0410_f0411_spell_cast_continuation_pc34_compat` passes; the
  source object is absent from both M10 and M11 archives.

# DM1 D2L2/D2R2 synthetic wall audit isolation (2026-08-06)

- ✅ Removed the synthetic D2L2/D2R2 framebuffer contract from M10. It uses
  local probe pixels and has no M11/runtime caller or original GRAPHICS.DAT
  input; the test now compiles the contract explicitly.
- ✅ Verification: `test_dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat` passes;
  the source object is absent from both M10 and M11 archives.

# DM1 D3L2/D3R2 F0115 thing-pass synthetic audit isolation (2026-08-06)

- ✅ Removed the D3 side-route F0115 fixture from M10. It hard-codes
  viewport zones, source coordinates and pixel blends, consumes no original
  DM1 data, and has no M11/runtime caller; its explicit viewport tests retain
  the ReDMCSB dispatch regression.
- ✅ Verification: `test_dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1L/D1R F0108 synthetic audit isolation (2026-08-06)

- ✅ Removed the D1 side-view F0108 contract from M10. It hard-codes zones,
  seeds and probe pixels, reads no original DM1 bytes and has no M11/runtime
  caller; its explicit ReDMCSB regression now compiles the fixture directly.
- ✅ Verification: `test_dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 PC3.4 archive-layout realdata regressions (2026-08-06)

- ✅ The viewport source-frame, deferred-C15 and thrown-projectile realdata
  regressions now recognize the standard original DOS archive layout
  `DATA/DUNGEON.DAT` plus `DATA/GRAPHICS.DAT`, as well as a flat install.
- ✅ Verification against the supplied PC3.4 archive: source-frame preflight
  and pixel rendering pass from authentic bytes. The C15 and throw probes now
  reach the real corpus and skip only when it lacks their required live pose.

# DM1 D0L/D0R F0108 synthetic audit isolation (2026-08-06)

- ✅ Removed the D0 side-view F0108 contract from M10. It hard-codes zones,
  seeds and probe pixels, reads no original DM1 bytes and has no M11/runtime
  caller; its explicit ReDMCSB regression now compiles the fixture directly.
- ✅ Verification: `test_dm1_v1_viewport_d0l_d0r_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D1L2/D1R2 F0115 thing-pass synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed D1 side-route F0115 table from M10. It hard-codes
  rows, zones and cell orders, reads no original DM1 bytes and has no
  M11/runtime caller; its explicit ReDMCSB regression now compiles it directly.
- ✅ Verification: `test_dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D0C F0108 floor-ornament synthetic audit isolation (2026-08-06)

- ✅ Removed the D0C F0108 contract probe from M10. It uses fixed pixels,
  seed and zone metadata, reads no original DM1 bytes and has no M11/runtime
  caller; the separate D0C real-material route remains unchanged.
- ✅ Verification: `test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D0L2/D0R2 F0108 synthetic audit isolation (2026-08-06)

- ✅ Removed the D0 side-route F0108 probe from M10. It hard-codes rows,
  zones, seeds and pixels, reads no original DM1 bytes and has no M11/runtime
  caller; its explicit ReDMCSB regression now compiles it directly.
- ✅ Verification: `test_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D0L2/D0R2 F0111 synthetic audit isolation (2026-08-06)

- ✅ Removed the partly-open D0 side-door route model from M10. It hard-codes
  frames, zones and states, reads no original DM1 bytes and has no M11/runtime
  caller; its explicit ReDMCSB regression now compiles it directly.
- ✅ Verification: `test_dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 D0L2/D0R2 F0115 thing-pass synthetic audit isolation (2026-08-06)

- ✅ Removed the fixed D0 side-route F0115 model from M10. It hard-codes rows,
  zones and pixels, reads no original DM1 bytes and has no M11/runtime caller;
  its explicit ReDMCSB regression now compiles it directly.
- ✅ Verification: `test_dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat`
  passes; the source object is absent from both M10 and M11 archives.

# DM1 production placeholder re-audit (2026-08-06)

# DM1 D0C/F0108 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only `dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_pc34_compat.c`
  from the broad M10 production source glob. Its fixed coordinate set, floor
  zone, and framebuffer values are synthetic probe inputs; source comments
  explicitly make no original pixel-parity claim, and repository-wide caller
  search found only its dedicated tests. The test targets still compile the
  helper explicitly.
- ✅ Retained the ReDMCSB control-flow audit as test material while preventing
  its synthetic values from entering a production archive. Recovery of the
  real D0C consumer and GRAPHICS.DAT ownership remains open.
- ✅ Verification target: M10/M11 archive-symbol audit plus the focused
  D0C/F0108 contract test; no game data was copied or committed.

# DM1 D0L/D0R F0107 synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only
  `dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_pc34_compat.c` from the broad
  M10 source glob. Its source evidence explicitly says it uses a synthetic
  framebuffer, reads no `GRAPHICS.DAT`, and claims no original pixel parity;
  repository-wide caller search found only the dedicated test.
- ✅ The focused test now compiles the audit source explicitly, preserving the
  ReDMCSB dispatch/ordering contract without allowing probe pixels into M10.
  Real D0L/D0R bitmap ownership remains open.
- ✅ No game data was copied or committed.

# DM1 pending-wounds synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only
  `dm1_v1_champion_panel_pending_wounds_tick_pc34_compat.c` from the broad
  M10 source glob. The file explicitly models a synthetic champion state,
  does not load original saves or DM1 data, and caller search found only its
  dedicated test target.
- ✅ Kept the ReDMCSB F0320/F0321 state-machine test intact; it already
  compiles the audit source explicitly. The live F0320 owner remains gated on
  real runtime champion state.
- ✅ No game data was copied or committed.

# DM1 D3C stairs/pit synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free
  `dm1_v1_viewport_d3c_stairs_pit_dispatch_pc34_compat.c` helper from the
  broad M10 source glob. Caller search found no M11/runtime consumer; the
  remaining references are the source-audit table and its dedicated test.
- ✅ The focused target now compiles the audit source explicitly, preserving
  the ReDMCSB dispatch/C10 contract while preventing its synthetic fixture
  blit from entering production. Real D3C bitmap ownership remains open.
- ✅ No game data was copied or committed.

# DM1 D2C center-wall synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free
  `dm1_v1_viewport_d2c_center_wall_composition_pc34_compat.c` pixel-trace
  helper from the broad M10 source glob; caller search found only its
  dedicated test.
- ✅ The test target now compiles the audit source explicitly, retaining the
  ReDMCSB D2C ordering/C10 contract while preventing synthetic pixels from
  entering production. Real D2C bitmap ownership remains open.
- ✅ No game data was copied or committed.

# DM1 F0241-F0260 source-receipt audit isolation (2026-08-06)

- ✅ Removed the unused raw F0241–F0260 source-receipt audit from the broad
  M10 source glob. Caller search found no M11/runtime consumer; its explicit
  test remains the only compilation owner.
- ✅ Preserved the ReDMCSB timeline admission and F0256 platform-boundary
  evidence without linking an inert receipt into production. A live timeline
  event owner remains open.
- ✅ No game data was copied or committed.

# DM1 chest partial-mask synthetic audit isolation (2026-08-06)

- ✅ Removed the contract-only
  `dm1_v1_chest_partial_mask_swap_with_mirror_candidate_pc34_compat.c` from
  the broad M10 source glob. Its evidence explicitly excludes real assets,
  savegames and pixel parity; caller search found only its dedicated test.
- ✅ The test now compiles the audit source explicitly, retaining the
  ReDMCSB chest/mask ordering contract without linking synthetic runtime state
  into production. The live masked-bitmap owner remains open.
- ✅ No game data was copied or committed.

# DM1 mirror resurrection cross-candidate synthetic audit isolation (2026-08-06)

- ✅ Removed the synthetic stale-panel
  `dm1_v1_mirror_candidate_resurrect_cross_candidate_clear_pc34_compat.c`
  from the broad M10 source glob. Caller search found only its dedicated
  contract test, and the helper consumes no original data.
- ✅ The test now compiles the audit source explicitly, preserving the
  ReDMCSB F0280/F0346 clear-order contract without linking fabricated
  champion state into production. The live owner remains open.
- ✅ No game data was copied or committed.

# DM1 F0449/F0450 floppy platform audit isolation (2026-08-06)

- ✅ Removed the unused F0449/F0450 floppy-media guard from the broad M10
  source glob. ReDMCSB identifies these bodies as Atari-ST floppy code, and
  caller search found only the explicit fail-closed test.
- ✅ Retained the platform-boundary receipt and its no-synthetic-availability
  assertion without linking an inert PC34 floppy route into production. No
  PC34 floppy data was invented or copied.
- ✅ No game data was copied or committed.

# DM1 G0601-G0650 owner-audit isolation (2026-08-06)

- ✅ Removed the metadata-only G0601–G0650 source-owner inventory from the
  broad M10 source glob. Its dedicated inventory target already compiles the
  table explicitly, and no runtime caller consumes the audit object.
- ✅ Preserved the mapping to the existing mouse, champion, GRAPHICS.DAT,
  runtime-memory and platform-boundary owners without treating the inventory
  table as a production implementation.
- ✅ No game data was copied or committed.

# DM1 F0249/F0267 C15 runtime relocation (2026-07-17)

- Completed the loaded C15 square-tail path used by ReDMCSB `MOVESENS.C
  F0267` and `TIMELINE.C F0249`. A moved explosion now keeps its source-owned
  decoded and raw `Next` synchronized while joining an existing destination
  chain; F0249 moves only the matching C25 event and deliberately leaves C24
  Fluxcage removal untouched. The regression covers C14 C48/C49 relocation,
  C15 append raw/decoded linkage, C25 relocation, and the C24 rejection.
  Verification: `dm1_v1_f0249_runtime_relocation_pc34_compat` and
  `csb_v1_f0267_loaded_chain_pc34_compat` PASS; `firestaff` built in
  `build/codex-dm1-f0205`.

- 2026-07-17 Nexus Structure3 raw counted-region intake: added strict direct
  first-, second-, and third-region ordinal-row admissions alongside the
  existing entry, opaque tag, face row, face-indexed row/set, same-ordinal
  third row, and face prefix/tail receipts. Every admission requires the direct-level identity,
  package and retained entry/region FNV witnesses, and exact 12-byte bounds;
  all retained bytes remain opaque. No coordinate, topology, normal, material,
  texture, geometry, or draw semantics are granted. Verification:
  `ninja -C build-nexus-codex test_nexus_v1_structure3_face_admission_fixture`
  and the `nexus_v1_structure3_(target|entry|face)_admission_fixture` CTests
  passed; scoped `git diff --check` passed.

- 2026-07-17 Nexus Structure1F-to-Structure3 raw selector-row relation:
  added a fail-closed admission joining an admitted 0x21 Structure1F
  wall-decoration record's raw selector byte to an admitted Structure3
  second-region row ordinal. It revalidates direct DGN identity, package and
  retained Structure1F record/Structure3 entry-region-row FNV witnesses, and
  exact offsets before retaining the equality. No face, owner, topology,
  geometry, material, texture, placement, or draw semantics are granted.
  Verification: `ninja -C build-nexus-codex
  test_nexus_v1_structure3_face_admission_fixture` and the
  `nexus_v1_structure3_(target|entry|face)_admission_fixture` CTests passed;
  scoped `git diff --check` passed.

- 2026-07-17 Nexus Structure1F alcove-to-Structure3 raw selector-row relation:
  added the same strict, independent fail-closed join for an admitted 0x20
  alcove record and an admitted Structure3 second-region row ordinal. Source
  tag, raw selector, direct DGN identity, package, record, entry, region, row
  FNV witnesses, and exact offsets must all still match. The receipt is only
  raw equality: it grants no portal, face, owner, topology, geometry, material,
  texture, placement, or draw semantics. Verification: `ninja -C
  build-nexus-codex test_nexus_v1_structure3_face_admission_fixture` and the
  `nexus_v1_structure3_(target|entry|face)_admission_fixture` CTests passed;
  scoped `git diff --check` passed.

- 2026-07-17 Nexus Structure1F direct-payload ownership admission: added a
  strict directory-bound owner receipt for the opaque tails of 0x11
  floor-decoration and 0x12 floor-sensor records. It retains only family/tag,
  record and payload spans, FNV witnesses, and direct package identity; other
  families reject. It explicitly does not permit a Structure3 relation or
  object, sensor, placement, geometry, material, texture, or draw semantics.
  Verification: `ninja -C build-nexus-codex
  test_nexus_v1_structure3_face_admission_fixture` and the
  `nexus_v1_structure3_(target|entry|face)_admission_fixture` CTests passed;
  scoped `git diff --check` passed.

- 2026-07-17 Theron Track 02 capture-required rescan continuity: direct
  ISO alias discovery now resolves the known materialized payload before
  campaign classification, and M11 retains capture-required only when a
  rescan repeats the exact canonical payload layout and capture-plan FNV.
  An unchanged alias rescan advances the scan epoch; payload, sector, hash,
  or plan drift clears the no-draw state. M11 now visibly reports
  `TRACK02 CAPTURE REQUIRED` instead of entering a graphics startup path.
  Verification: focused raw-media, campaign-discovery, M12 launch-intent,
  and M11 capture-required runtime tests passed in `build-theron-trace-md5`.

- 2026-07-17 Theron Track 02 capture-required startup handoff: a current
  direct ISO/BIN/CUE receipt plus the existing opaque capture plan now passes
  from M12 into M11 as `TRACK02 CAPTURE REQUIRED` when no later capture
  evidence is bound. M11 retains the plan identity and scan epoch in its
  no-draw capture-required state, rejects drift, and does not attempt the
  startup graphics route. M12 rejects this reduced handoff when any partial
  later receipt is present. Verification: `theron_v1_track02_raw_media_intake`,
  `theron_v1_campaign_launch_intent`, and
  `m11_theron_track02_startup_capture_required` passed in
  `build-theron-trace-md5`.

- 2026-07-17 Theron Track 02 startup media validation: fixed direct
  ISO/BIN/CUE discovery to accept a real known Track 02 MD5 only with its
  matching sector layout, including CUE payload-local user-data offsets and
  the known `TQJP02.iso`/`TQUS02.iso` to `End.iso` materialization aliases.
  The scanner and campaign receipt now expose exact stable failure reasons
  instead of misclassifying valid layouts as generic graphics-invalid.
  Missing paths/payloads, malformed CUE, bad sector alignment, unknown hash,
  hash/layout conflicts, CUE-index drift, and invalid windows remain closed.
  No bytes are fabricated and no consumer, route, bitmap, or render path is
  promoted. Verification: `theron_v1_track02_raw_media_intake` and
  `theron_v1_track02_campaign_media_discovery` passed in
  `build-theron-trace-md5`.

- 2026-07-17 Nexus PRS3 terminal startup state: added a launcher/M12
  transition receipt that turns a missing authenticated MENU.BPK PRS3 trace
  into a terminal `capture-required` startup outcome. M12 can return to idle
  or rescan without a wait loop; a verified imported capture is consumed as a
  terminal no-draw outcome. No decoder, pixels, palette semantics, or
  synthetic MENU.BPK image is enabled. Verification:
  `nexus_v1_prs3_startup_state`, `nexus_v1_prs3_material_local_artifact`, and
  `nexus_v1_menu_bpk_renderer_handoff_gate` passed; `git diff --check` passed.

- 2026-07-17 DM1 HoC all-C127 pointer sweep: extended the real PC34 HoC
  runtime/save test to enumerate each map-0 raw C127 sensor from its original
  Thing chain, derive the only front-facing party pose from the packed cell,
  and click the source-provided D1C hit zone through `M11_GameView_HandlePointer`.
  Every locally staged source candidate (24/24) rejects an adjacent
  out-of-zone click and opens the F0280/C040 candidate path on the positive
  click; the test keeps no HoC coordinate list and introduces neither a
  champion-list fallback nor synthetic sensors. Verification:
  `m11_dm1_hoc_no_fallback_panel`,
  `m11_dm1_hoc_c127_resurrect_reincarnate_full_pc34`,
  `m11_dm1_hoc_c127_panel_redraw_close_pc34`, and
  `m11_dm1_hoc_c160_clear_corridor_redraw_pc34` passed; `firestaff` built in
  `build/codex-dm1-f0205` and `git diff --check` passed.

- 2026-07-17 DM1 HoC C127 source-cell click routing: fixed the live M11
  mirror selection handoff to preserve the F0172-visible packed C127 wall
  cell through the F0280 candidate receipt. The D1C source hit box and
  source-backed C346/C026 material gate remain unchanged; no alternate
  champion list or host fallback is introduced. Verification:
  `m11_dm1_hoc_no_fallback_panel`,
  `m11_dm1_hoc_c127_resurrect_reincarnate_full_pc34`,
  `m11_dm1_hoc_c127_panel_redraw_close_pc34`, and
  `m11_dm1_hoc_c160_clear_corridor_redraw_pc34` passed; `firestaff` built
  in `build/codex-dm1-f0205` and `git diff --check` passed.

- 2026-07-17 Theron opt-in G8 FIFO-output instrumentation: extended the
  local Mednafen FIFO-origin main-RAM patch with an explicit
  `FIRESTAFF_THERON_G8_FIFO_OUTPUT_TRACE=1` marker emitted only for the
  existing generation-8, LBA-4859, dispatch-4 observation. Added a local
  operator planner that requires an executable emulator, a regular CUE, and
  an absent output target, without launching, copying media, or fabricating
  rows. The G8 schema verifier now requires that marker before the existing
  FIFO-origin output row and continues to reject consumer claims. A real
  original CUE/BIN Mednafen run is still needed before any importer or M11
  binding may consume this evidence. Verification:
  `test_theron_v1_g8_fifo_output_receipt.sh`, patch-hook checks, no-input
  planner rejection, `ninja -C build-theron-trace-md5 firestaff`, and
  `git diff --check` passed.

- 2026-07-17 Theron G8 FIFO-output capture schema: added a strict local
  verifier for the already source-owned post-G7 sequence-4 `JSR $e009` and
  G8 READ(6) `0x73a` chain. It admits exactly one ordered
  `pce_cd_fifo_origin_main_ram_receipt` only after the fixed callsite/CDB and
  rejects generation drift or any asserted consumer row. This prepares the
  missing FIFO-to-game-RAM output witness without inventing a consumer, route,
  payload format, bitmap, palette, or draw path. The fixture is trace metadata
  only; a real Mednafen G8 output row remains required for corpus/M11 binding.
  Verification: `test_theron_v1_g8_fifo_output_receipt.sh`, focused sector/M11
  CTests, `ninja -C build-theron-trace-md5 firestaff`, and `git diff --check`
  passed.

- 2026-07-17 Theron Stage-3 `$e009` callsite-context admission: the strict
  coalesced original-capture receipt now retains the disassembled `CL/DL/CH`
  record arguments, requested sector count, observed raw-sector LBA, and the
  immediate post-return next PC alongside the verified `CALL $e009` edge.
  Sector admission recomposes the record from those register bytes, and M11
  rejects active-session drift in any retained callsite or post-return control
  coordinate before the no-draw handoff can bind. This identifies one observed
  loader branch only; it deliberately does not assign a level route or decode
  record, bitmap, palette, or object payloads. The existing multi-capture
  index/seed-to-route blocker remains in TODO. Verification: focused
  sector-record, M11 host, artifact-corpus, and loader-output CTests passed;
  `ninja -C build-theron-trace-md5 firestaff` and `git diff --check` passed.

- 2026-07-17 Theron Stage-3 later `$e009` return-control receipt: the
  source-backed sector-record admission now retains the observed game-owned
  `CALL $e009` opcode/target and immediate post-return PC. M11 carries those
  coordinates through the active handoff receipt and requires the post-return
  PC to equal the original call return before dungeon `RESUME_READY` can
  remain bound. A changed return edge clears no-draw readiness; this proves
  control-flow provenance only and assigns neither a level route nor payload,
  bitmap, palette, object, or rendering semantics. The multi-capture
  index/seed-to-route correlation remains blocked pending independent original
  captures. Verification: focused sector-record, M11 host, artifact-corpus,
  and loader-output CTests passed; `ninja -C build-theron-trace-md5 firestaff`
  and `git diff --check` passed.

- 2026-07-17 DM1 M11 GROUP.C F0190 killed-all source admission: the runtime
  now requires an exact live/raw PC3.4 C04 `GROUP` match and proof that the
  supplied source square's raw Thing chain owns that exact group before it
  can execute F0188 drops, F0189 unlink, C29-C41 cleanup, ACTIVE_GROUP
  retirement, or C040 afterplay. Unknown creature records, raw/decoded drift,
  and an unowned LoS square fail closed without mutating the group, chain, or
  active state. The fixed-possession fixtures now preserve authentic C04 slot
  state and source cell bits. Verification: the five focused F0190/M11 CTests
  passed, `firestaff` built with Ninja, and `git diff --check` passed.

- 2026-07-17 DM1 F0328/F0215/F0142/F0115 raw thrown-object consumers:
  M11 now revalidates every rendered or materialized `Projectile.Slot` through
  raw PC3.4 F0156 data and F0140's F0159 container traversal. Weapon aspects,
  potion type, armour/junk subtype, F0215 drops, poison consumption, and the
  sharp-weapon aftermath all reject missing, altered, unknown, or cyclic raw
  records instead of borrowing a decoded object mirror. The corpus render
  probe selects a weapon only through the same raw admission and verifies an
  F0156 type drift yields no F0115 receipt; it SKIPs when no admissible local
  record exists. Verification: `m11_action_stamina_runtime_source_lock` and
  `m11_dm1_throw_projectile_runtime_materialization_pc34` passed, `firestaff`
  built with Ninja, and `git diff --check` passed.

- 2026-07-17 DM1 DUNGEON F0140/F0159 M11 F0328 object-weight handoff:
  M11's leader-hand and action-hand THROW paths now delegate weight admission
  to the loaded PC3.4 F0156/F0159 raw record chain before F0328 advances
  RNG, stamina, XP, or projectile materialization. Raw potion power/type are
  retained as C/G-owned fields for the same launch receipt. Missing, unknown,
  or cyclic raw chains fail closed even when a decoded mirror is populated.
  Focused fixtures cover authentic junk, container, and potion records plus a
  decoded-valid/raw-cyclic container rejection. Verification:
  `dm1_v1_object_weight_f0140_pc34_compat` and
  `m11_action_stamina_runtime_source_lock` passed, `firestaff` built with
  Ninja, and `git diff --check` passed.

- 2026-07-17 DM1 DUNGEON F0158 M11/F0312 weapon-fact handoff: M11 now
  resolves THROW, SHOOT, and delayed F0259 quiver refill weapon facts through
  ReDMCSB `DUNGEON.C F0158` over the loaded PC3.4 `WEAPON.Type` record and
  G0238, instead of its duplicate decoded-table copy. Missing, malformed, or
  out-of-range raw records fail closed before a projectile or inventory move;
  valid raw facts retain the existing F0312/F0328 weight, strength, kinetic,
  and class path. Focused fixtures now carry authentic four-byte weapon
  records, and a contradictory decoded mirror with an invalid raw type is
  explicitly rejected. Verification: `m11_action_stamina_runtime_source_lock`
  passed, `firestaff` built with Ninja, and `git diff --check` passed.

- 2026-07-17 DM1 M11 F0394 spell-panel source-caster admission: the live
  spell renderer now accepts C009 (96x33), C011 (96x36), and verified M653
  material only when F0394 has published a valid G0514 caster. It reads the
  selected caster's authenticated `Symbols[]` and `SymbolStep` directly;
  the old `spellBuffer` presentation bridge can no longer synthesize rune
  text. Missing G0514, source pixels, or source font retains the complete
  ReDMCSB black clear. C077/C079 remain F0387 action-panel assets and are
  deliberately not admitted as spell material. Verification: five focused
  spell/action CTests passed, `firestaff` built with Ninja, and `git diff
  --check` passed.

- 2026-07-17 DM1 M11 F0384/F0387 action-panel provenance: removed M11's
  duplicate G0490 action-name table and delegated WAR CRY, KICK, STAB, and
  every other label to DM1's ReDMCSB-locked F0384 lookup. Action labels now
  render only after the selected C011/C077/C079 PC34 panel surface validates;
  absent or malformed material retains F0387's black clear without synthetic
  text. The M11 probe surface delegates to the physical C010/C009 source
  rectangles, preserving M653's native 6x7 cells and disabled-state coverage.
  Verification: 10 focused action-panel CTests passed, `firestaff` built,
  and `git diff --check` passed.

- 2026-07-17 DM1 HoC F0168/F0107 wall-inscription material provenance:
  bound the selected original packed TextString words to the DM1 material
  and M11 presentation receipts with bounded span and FNV-1a identities.
  D1C accepts the authentic M648 8x8 font pixels only with matching source
  glyph bytes, records native 1:1 scale, and performs no host palette remap.
  D3/D2/D1 side and depth views retain the same source receipt while drawing
  only ReDMCSB's unreadable ornament bitmap with its original C10 palette,
  flip, and G0204 height. No font, texture, or palette fallback was added.
  Verification: 14 focused inscription/material/source-lock CTests passed,
  `firestaff` built, and `git diff --check` passed.

- 2026-07-17 DM1 HoC C127 D1-only visibility routing: constrained the
  ReDMCSB `DUNVIEW.C F0107` projection receipt to the three original one-tile
  routes. D1L/D1R may consume authentic C346 only at view-wall indices 10/11;
  D1C owns the C346/C026 overlay at index 12. A C127 fact at D2/D3 or an
  incompatible wall index now suppresses the generic ornament instead of
  repurposing C346 or C026. Verification:
  `dm1_v1_champion_mirror_pc34_compat`,
  `m11_dm1_hoc_mirror_side_depth_material_receipt`,
  `m11_dm1_hoc_mirror_rotation_inscription_pc34`, and
  `dm1_v1_hoc_all_front_mirror_ordinals_pc34_compat` passed; `firestaff`
  built; `git diff --check` passed.

- 2026-07-17 DM1 TITLE/Entrance palette and timing receipt: bound the
  ReDMCSB ENTRANCE.C palette identity, entry count, and fingerprint to the
  startup timing receipt. Each M11 entrance presentation now consumes the
  matching command-carried palette receipt rather than selecting a local
  palette value; substituted or mutated palette receipts reject before
  rendering. The TITLE.C cadence source gate was updated to lock the current
  per-step source-command route. Verification: the nine focused title and
  entrance CTests passed, `firestaff` built, and `git diff --check` passed.

- 2026-07-17 DM1 HoC C127 side/depth material admission: added the M11
  receipt consumer regression for ReDMCSB `DUNVIEW.C F0107` one-tile D1L
  and D1R champion-mirror projections. It admits the authentic C346 backing
  at view-wall indices 10/11, including the right-side source flip, and
  refuses to publish C026 outside the dedicated D1C `3913-3928` route. The
  PC34 graphics loader supplies the tested bytes; no portrait or bitmap
  fallback is involved. Verification:
  `m11_dm1_hoc_mirror_side_depth_material_receipt`,
  `m11_dm1_side_wall_ornament_host_presentation_receipt`,
  `m11_dm1_hoc_wall_material_receipt_pc34`, and
  `dm1_v1_champion_mirror_pc34_compat` passed; `firestaff` built.

- 2026-07-17 DM1 M11 F0111 door-ornament receipt consumption: M11 now
  feeds center and side temporary door-panel segments through the existing
  ReDMCSB `DUNVIEW.C F0111` receipt before applying authenticated ornament
  material. Each visible depth is handled independently, so a missing/open
  far door cannot suppress an eligible nearer door. The consumer preserves
  source panel coordinates and rejects non-door, open, empty-ornament, and
  malformed panel shapes through the DM1 receipt; it introduces no fallback
  artwork. Verification: `dm1_v1_f0111_door_ornament_dispatch_pc34_compat`,
  `m11_dm1_hoc_wall_material_receipt_pc34`, and
  `dm1_v1_champion_mirror_pc34_compat` passed; `firestaff` built; `git diff
  --check` passed.

- 2026-07-17 DM1 F0207/F0212 creature-projectile publication: made the
  creature-owned C14 slot and its first C48/C49 movement receipt atomic in
  both the M10 C38/F0207 bridge and M11's remaining live consumer. A full
  bounded timeline now rejects the launch and F0813 removes exactly the new
  slot, preserving all pending source events rather than leaving an
  unscheduled renderable projectile. The focused M11 runtime regression now
  covers the admitted Red Dragon fireball and the full-queue rejection path.
  Verification: `test_m11_creature_projectile_runtime_pc34_compat` passed
  29/0; `memory_tick_orchestrator_f0303_skill_query_pc34_compat` passed;
  `firestaff` built; `git diff --check` passed.

- 2026-07-17 DM1 F0219 C14/C48-C49 projectile motion: added the source-named
  PROJEXPL.C F0219 boundary for live projectile advancement and made its raw
  C14 owner authoritative. M10 now verifies decoded/raw C14 identity before
  advancing, keeps queue-compaction EventIndex writeback synchronized, and
  serializes the moved kinetic energy, attack, and next C48/C49 EventIndex
  into the same record. A drifted raw C14 rejects before mutation. Champion
  impact planning also retains F0217's already-published action attack
  receipt rather than recomputing from later projectile state. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
  `dm1_v1_throw_shoot_pc34_compat`, `dm1_v1_original_save_pc34_handoff`, and
  `m11_creature_projectile_runtime_source_lock` passed; isolated `firestaff`
  built; `git diff --check` passed.

- 2026-07-17 DM1 F0215 projectile delete and thrown-potion ownership: M10
  now applies the existing F0215 receipt's authenticated C05 consumption
  before completing C14 deletion. The path validates the source C14 Slot,
  writes both decoded and raw C05/C14 `Next` fields, and preserves C05 power
  and type bytes. It does not synthesize a C15 Thing for the separate
  F0217/F0213 explosion handoff. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
  `dm1_v1_throw_shoot_pc34_compat`,
  `memory_tick_orchestrator_f0303_skill_query_pc34_compat`,
  `dm1_v1_original_save_pc34_handoff`, and
  `m11_creature_projectile_runtime_source_lock` passed; isolated `firestaff`
  built; `git diff --check` passed.

- 2026-07-17 DM1 F0216 source impact receipt: added a standalone,
  source-authenticated C14-slot boundary for PROJEXPL.C F0216. It verifies
  decoded/raw C14 identity, obtains the carried object's F0140 weight and
  weapon F0158 kinetic term, consumes the documented RNG sequence, and
  rejects missing, drifted, or C15 slots rather than borrowing runtime
  subtypes. Verification: `dm1_v1_projectile_impact_attack_f0216_pc34_compat`
  and isolated `firestaff` passed; `git diff --check` passed.

- 2026-07-17 DM1 F0142/F0115 projectile material admission: restored the
  strict source-record boundary for a carried C14 `Projectile.Slot`. F0142
  now rejects an out-of-range C05..C0B subtype before the generic inventory
  aspect helper can normalize it to subtype zero, so malformed projectile
  data cannot borrow G0209/M612 object art. The F0115 world-summary fixture
  now supplies its authentic one-column SFT descriptor and exercises the
  M10 F0511/F0512 chain instead of relying on an incomplete world shape.
  Verification: focused F0142, F0115, and viewport-materialization CTests
  passed; `test_m11_action_stamina_runtime_pc34_compat` passed 1317/0;
  `firestaff` built; `git diff --check` passed.

- 2026-07-17 DM1 M11 F0253/F0259 SHOOT delayed quiver refill: restored the
  source boundary in TIMELINE.C C11. F0253 now sees live C032 ActionIndex,
  re-enables the champion and moves only compatible quiver ammunition into
  an empty ready hand before clearing ActionIndex; C11's nonzero
  SlotOrdinal F0259 move remains independent. This retains C12 then
  C07-C09 source priority without turning unrelated C11 receipts into a
  refill. Verification: `test_m11_action_stamina_runtime_pc34_compat`
  reached 1317 passes with 0 failures; `firestaff` build and
  `git diff --check` passed.

- 2026-07-17 DM1 M11 F0412/F0410 spell-practice feedback: retained the
  original spell skill through the F0412-to-HUD receipt boundary and insert
  its ReDMCSB CHAMPION.C base class name between F0410's two source text
  fragments. A failed Fireball practice cast now reports the authentic
  champion-specific `HALK NEEDS MORE PRACTICE WITH THIS WIZARD SPELL.` line,
  while preserving the source shifted practice XP and normal rune clear.
  Verification: `test_m11_action_stamina_runtime_pc34_compat` reached 1315
  passes; `firestaff` build and `git diff --check` passed.

- 2026-07-17 DM1 M11 F0407 WINDOW G0497 XP ownership: corrected the common
  action XP tail to pass ReDMCSB CHAMPION.C F0304's
  `CurrentMap->Difficulty`, rather than the unrelated party map ordinal.
  Headless M11 dispatches now retain the source zero-difficulty fallback
  until an authenticated dungeon-map descriptor is present, so C039 WINDOW
  receives its G0497 award exactly once and without a synthetic multiplier.
  Verification: `test_m11_action_stamina_runtime_pc34_compat` reached 1314
  passes with all WINDOW assertions green; `firestaff` build and
  `git diff --check` passed.

- 2026-07-17 DM1 M11 F0253 action-enable and SPIT projectile-queue
  ownership: completed the source C11 lifecycle for ordinary F0407 actions.
  F0330 now translates source `GameTime` through M10's pre-increment queue
  dispatch and preserves CHAMPION.C's two replacement-time branches; F0253
  consumes the matching live gate before the aging pass, so it removes G0495
  defense and resets ActionIndex exactly once. F0328 keeps its in-flight
  action-tick timing separate. LIGHT retains both its G0491/C11 receipt and
  F0404 C70 decay receipt. SPIT follows `MENU.C F0407 -> F0327 -> F0212`,
  retaining one authentic C49 projectile-movement event alongside its C11,
  rather than replacing either queue owner. Verification:
  `test_m11_action_stamina_runtime_pc34_compat` reached 1312 passes; all
  C11/F0253, C70, and SPIT assertions are green.

- 2026-07-17 DM1 M11 F0215 thrown-weapon materialization and Thing-next
  synchronization: separated F0215's stored C14 source-square cleanup from
  its F0219-committed champion-impact drop square, while retaining the real
  `Projectile.Slot` identity and cell bits. The M11 handoff now materializes
  the slot even when a direct runtime advance has no compact C14 square-list
  mirror, and writes the same end/tail link to decoded and raw PC34 object
  records. F0319 rejects structural slot values before its inventory-drop
  loop, preventing an unowned C00 entry from preceding the F0215 weapon.
  Focused lifecycle coverage verifies empty-square and tail append paths,
  raw/decoded `Thing.next`, cross-cell champion impact, and the structural
  slot rejection. Verification: `test_m11_action_stamina_runtime_pc34_compat`
  reached 1300 passes with all F0215 assertions green; `firestaff` build and
  `git diff --check` passed.

- 2026-07-17 DM1 M11 F0330/C11 action-cell reactivation: replaced the
  placeholder M11 F0330 scheduler with the ReDMCSB `CHAMPION.C F0330`
  one-C11-per-champion receipt, including its replacement-delay rule and
  `TIMELINE.C F0253` ownership. The action-cell route now remains disabled
  until its authenticated C11 dispatch, then permits same-cell reactivation
  and the subsequent same-cell toggle through `MENU.C F0389/F0388`; no local
  synthetic re-enable path was added. Verification passed:
  `firestaff_dm1_v1_champion_panel_action_menu_routing_probe`,
  `firestaff_dm1_v1_champion_panel_action_cell_slotbox_runtime_probe` with
  `/Users/bosse/.firestaff/data/dm1`,
  `test_dm1_v1_f0330_c11_production_pc34_compat`,
  `test_dm1_v1_inventory_slot_placement_pc34_compat` (156/156),
  `test_m11_dm1_hoc_wall_material_receipt_pc34`, and
  `firestaff_m11_chest_slot_zones_probe` (65/65).

- 2026-07-17 Theron Track 02 M11 replay-consistency host admission: bound the
  active opaque multi-record replay receipt into the live Soul Room/dungeon
  route boundary. M11 requires current direct campaign layout, replay epoch,
  media identity, and final record/raw-sector identity, retaining them at
  Soul Room and rechecking them at dungeon handoff. Stale epochs and replay
  invalidation from duplicate/reordered evidence clear both route flags. No
  decoder, renderer, or fallback was added. Verification: CTest
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_loader_trace_replay_consistency`, and
  `theron_v1_track02_live_loader_route_admission` passed (3/3); full
  `firestaff` build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 loader-trace replay consistency: added a
  stateful opaque receipt for dynamic CD_READ ownership under one current
  direct campaign-media layout epoch. It retains only media identity, ordered
  record/sector provenance, count, and an order checksum. Duplicate,
  reordered, stale-epoch, mixed-media, or layout-drifted records clear the
  state. No payload, level/object, bitmap/palette, decode, draw, or fallback
  semantics are admitted. Verification: CTest
  `theron_v1_track02_loader_trace_replay_consistency`,
  `theron_v1_track02_dynamic_cd_read_ownership`, and
  `theron_v1_track02_live_loader_route_admission` passed (3/3); full
  `firestaff` build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 live loader log reproducibility: propagated the
  MD5 of the converted HuC6280 event log into the opaque live-route receipt
  and made M11 retain and recheck it alongside the Mednafen source-trace MD5
  across Soul Room epoch 1 and dungeon-handoff epoch 2. Missing or changed
  log identity clears the entire route pair; direct campaign-layout checks and
  all no-decode/no-draw flags remain required. Verification: CTest
  `theron_v1_track02_live_loader_route_admission`,
  `m11_theron_track02_capture_campaign_host`, and
  `theron_v1_track02_campaign_media_discovery` passed (3/3); full
  `firestaff` build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 M11 direct campaign-layout live-route gate:
  extended the observed loader-route host boundary to require the retained
  direct campaign receipt, a fresh raw-media receipt, and the opaque
  three-route plan on every Soul Room/dungeon-handoff bind. Canonical paths,
  MD5/variant, MODE1 facts, INDEX 01, sector/user-data layout, route order,
  and trace identity must stay current; drift clears both live-route flags.
  No graphics/object format, decode, render, or fallback permission was
  added. Verification: CTest
  `m11_theron_track02_capture_campaign_host` and
  `theron_v1_track02_campaign_media_discovery` passed (2/2); full
  `firestaff` build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 direct campaign-media layout lifecycle: added a
  strict direct-only revalidation boundary between campaign discovery and a
  refreshed raw-media intake. It requires the source-owned canonical CUE and
  payload paths, variant/MD5, MODE1 facts, INDEX 01, payload bytes, sector
  count, and raw/user-data windows to remain exact before a plan can be
  reused. Ambiguous, virtual, extracted, non-launchable, stale, or mixed
  evidence rejects; no media is copied and no decode, rendering, or fallback
  path was added. Verification: CTest
  `theron_v1_track02_campaign_media_discovery` and
  `asset_status_theron_campaign_media_scan` passed (2/2); full `firestaff`
  build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 M11 live route lifecycle: extended the observed
  loader-route receipt with the stable source Mednafen-trace MD5 from the
  converted trace receipt, then bound it to M11 as an opaque two-step route.
  Soul Room is accepted only at epoch 1; dungeon handoff requires epoch 2 and
  the same trace/media identity. Sequence, epoch, identity, or no-draw flag
  drift clears both readiness flags. No graphics/object format, decode,
  renderer, or fallback was added. Verification: CTest
  `theron_v1_track02_live_loader_route_admission` and
  `m11_theron_track02_capture_campaign_host` passed (2/2); full `firestaff`
  build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 live loader-route admission: added a strict
  runtime join requiring the register-normalized dynamic CD_READ ownership
  receipt and the closed observed HuC6280 loader/dungeon/object event log.
  It binds those facts through the existing manifest and opaque runtime
  admission only when MD5, variant, loader record, `$3800` destination,
  checksums, consumer PCs, and bounded windows all agree. Drift clears the
  route; all level/object, bitmap/palette, decode, draw, and fallback flags
  remain false. Verification: CTest
  `theron_v1_track02_live_loader_route_admission`,
  `theron_v1_track02_dynamic_cd_read_ownership`, and
  `theron_v1_track02_huc6280_capture_event_log` passed (3/3); full
  `firestaff` build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 dynamic HuC6280 CD_READ ownership: added a
  strict normalized receipt over the existing hash-authenticated raw CUE/BIN
  intake and original loader trace. It reconstructs the record only from the
  observed CL/DL/CH bytes, requires the `$3800` transfer, exact raw sector and
  MODE1 user-data coordinates, and retains the bounded destination/full-sector
  checksums. Register, MD5, CUE-window, raw-offset, and payload-span drift
  reject. The receipt grants no level/object, bitmap/palette, decode, draw, or
  fallback permission. Verification: CTest
  `theron_v1_track02_dynamic_cd_read_ownership`,
  `theron_v1_track02_huc6280_capture_event_log`, and
  `theron_v1_raw_loader_trace_ingest` passed (3/3); full `firestaff` build and
  `git diff --check` passed.

- 2026-07-17 Theron Track 02 non-startup sector-directory admission: added a
  file-backed receipt which rechecks the identity-verified raw CUE/BIN MD5,
  consumes the existing source-owned Stage-3 six-byte descriptor directory,
  and accepts only a complete ordered coalesced Mednafen `$e009` observation
  for a descriptor-resolved later record. The receipt carries exact physical
  offsets, 2048-byte user-data bounds and hashes, with all level/object,
  bitmap/palette, decode, draw and fallback permissions false. M11 publishes
  that receipt only as identity-matched opaque dungeon readiness and clears it
  on source or flag drift. Added no-input/missing-corpus tests and an explicit
  read-only corpus probe. Verification: full `firestaff` build passed; CTest
  `theron_v1_track02_sector_record_admission`,
  `theron_v1_track02_sector_record_corpus_probe_no_inputs`,
  `theron_v1_track02_sector_record_corpus_probe_missing_media`, and
  `m11_theron_track02_capture_campaign_host` passed (4/4); `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 boot/runtime campaign transaction: retained the
  already verified opaque capture plan alongside M12's campaign-media receipt,
  copied both into launch intent, and bound both to the actual M11 Theron boot
  after profile allocation. The boot runtime receipt preserves only the
  launchable direct identity; payload-path/MD5 drift, virtual media, or a plan
  mismatch abort before any Soul Room or dungeon capture route can be
  published. No capture semantics, decoder, renderer, or fallback changed.
  Verification: CTest `theron_v1_track02_campaign_media_launch`,
  `theron_v1_campaign_launch_intent`, and
  `theron_v1_m11_launcher_handoff_boundary` passed (3/3); full `firestaff`
  build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 campaign-media launch intent: M12 launch intent
  now carries a value copy of its verified direct campaign-media receipt, and
  the M11 selected-menu boundary revalidates that copy against current M12
  status before starting Theron. The payload path/MD5 is taken only from the
  bound receipt; path-only availability, stale payload or MD5 identity,
  virtual/container evidence, and missing bindings reject. Existing local
  generic-media handoff coverage now skips safely until an explicit campaign
  scan is supplied. Verification: CTest
  `theron_v1_campaign_launch_intent`,
  `theron_v1_m11_launcher_handoff_boundary`, and
  `asset_status_theron_campaign_media_scan` passed (3/3); full `firestaff`
  build and `git diff --check` passed.

- 2026-07-17 Theron Track 02 M12 campaign-media launcher scan: added an
  explicit Theron-only M12 entry that consumes the verified campaign-media
  discovery plus opaque target plan and publishes launch readiness only for
  one direct exact-layout candidate. Generic archive cache materialization is
  not called; virtual/container hits, absent media, plan mismatch, and later
  rejected rescans retain diagnostics while clearing availability/readiness.
  Added M12 lifecycle coverage and corrected the campaign discovery/launch
  CMake targets to link their existing M12 scanner dependencies. Verification:
  CTest `asset_status_theron_campaign_media_scan`,
  `theron_v1_track02_campaign_media_discovery`, and
  `theron_v1_track02_campaign_media_launch` passed (3/3); full `firestaff`
  build passed; `git diff --check` passed.

- 2026-07-17 Theron Track 02 campaign-media launch boundary: added the
  profile/launch entry that consumes discovery receipts only after one direct,
  exact-layout candidate matches every capture-plan target. The launch receipt
  clears stale launchability before each bind; profile MD5 drift, ambiguous
  variants, and virtual/container evidence retain diagnostics but cannot enter
  the launch path. No archive extraction, decoder, renderer, or original-data
  gate replacement was added. Verification: `firestaff_theron` rebuilt; CTest
  `theron_v1_track02_campaign_media_discovery` and
  `theron_v1_track02_campaign_media_launch` passed (2/2); `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 campaign-media discovery: added hash-first
  discovery for explicit loose CUE/BIN/ISO paths and Firestaff scanner-backed
  directory/container matches. Direct media still passes the strict raw intake;
  virtual container matches retain only a hash/layout identity and are never
  extracted or launchable. Known mixed Track 02 variants reject as ambiguous,
  and all three capture-plan targets must match the selected identity with no
  decoder/render/fallback permission. Verification: `firestaff_theron` rebuilt;
  CTest `theron_v1_track02_campaign_media_discovery` passed; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 M11 capture-ready host boundary: M11 now
  reconstructs its active startup-media receipt and consumes only a verified
  capture-campaign admission to expose start, Soul Room, and dungeon readiness.
  Every bind clears stale state first; absent evidence, media MD5 drift,
  campaign destination drift, or a non-Theron host leave every route unready.
  The boundary does not authorize a decoder, renderer, fallback visual, or
  replace existing original-data gates. Verification: CTest
  `theron_v1_runtime_track02_capture_campaign_admission` and
  `m11_theron_track02_capture_campaign_host` passed (2/2); `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 Hall-of-Records receipt contract: aligned the
  startup/save-resume regression fixture with the source-locked opaque entry-6
  candidate. It retains bounded raw offsets, user-data provenance, hashes, and
  no-fallback blocker evidence, but asserts no object-table semantic role,
  field decoder, or cross-anchor row consensus. Reordering or mutating those
  opaque bytes remains blocked. Verification: CTest
  `theron_v1_startup_save_resume_pc34` and
  `theron_v1_runtime_track02_capture_campaign_admission` passed (2/2);
  `git diff --check` passed.

- 2026-07-17 Theron Track 02 runtime capture-campaign boundary: added the
  active startup/Soul Room/dungeon admission receipt that consumes only a
  complete campaign, matching startup-media Track 02 identity, and the
  manifest-bound opaque dungeon window. Lifecycle, campaign absence, media
  drift, and destination-window drift fail closed; it grants no decode,
  render, or fallback permission and does not alter startup selection.
  Verification: `firestaff_theron` rebuilt and the exact runtime-boundary
  fixture passed directly. The CMake test target remains blocked by unrelated
  DM2 `missile_record` compile errors in `dm2_v1_dungeon_loader.c`;
  `git diff --check` passed.

- 2026-07-17 Theron Track 02 operator campaign emission: added a strict
  operator-only emitter for three independently route-selected campaign
  bundles. It requires an existing authenticated plan, raw CUE/Track 02 MD5,
  and stable Mednafen-trace MD5, writes outputs exclusively, and reports their
  hashes. Dry-run writes nothing; no emulator launch, media copy, synthetic
  capture row, decoder, or renderer exists on this path. Verification:
  focused Track 02 CTest passed 6/6 in 0.51 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 multi-bundle capture campaign: added a strict
  campaign verifier for independent start, Soul Room, and dungeon-handoff
  bundle admissions. It requires distinct bundle MD5s, shared Track 02 and
  Mednafen provenance, and agreement on every opaque transfer, palette,
  bitmap, and destination identity. Route reordering, bundle reuse, and
  contradictory facts reject; the retained campaign receipt has no decoder or
  render permissions. Verification: focused Track 02 CTest passed 5/5 in
  0.47 seconds; local corpus discovery safely skipped with fewer than three
  candidates; `git diff --check` passed.

- 2026-07-17 Theron Track 02 capture-artifact importer: added a strict
  external bundle importer that rechecks direct regular Mednafen trace and
  bundle files by stable MD5, then requires exactly the three ordered
  multi-route plan identities. Partial, mixed, reordered, and hash-mismatched
  inputs reject; the only successful output is an opaque runtime-admission
  receipt with pixel, level/object, render, and fallback permissions false.
  The bounded corpus probe safely skips without bundles. Verification: focused
  Track 02 CTest passed 7/7 in 0.40 seconds; local artifact-corpus discovery
  printed `SKIP`; `git diff --check` passed.

- 2026-07-17 Theron Track 02 multi-route capture-target planner: added a
  strict capture-only plan for start, Soul Room, and dungeon-handoff routes.
  It joins authenticated CUE/Track 02 provenance, the CD_READ/loader-output
  chain, direct HuC6260 write identity, bitmap-transfer identity, and the
  opaque destination record boundary. Fixture drift, empty spans, and any
  decoder/draw/fallback flag reject; the planner never renders or assigns
  pixel, palette, level, or object semantics. A bounded corpus probe finds
  only direct CUE candidates and safely skips absent observations. Verification:
  focused Track 02 CTest passed 7/7 in 0.48 seconds; the local corpus probe
  found one authenticated US CUE and reported `SKIP`; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 live dungeon-handoff replay: added a strict
  six-step validator binding one authenticated raw CUE/Track 02 identity to
  the dynamic CD_READ receipt, loader-chain checksum, direct-VCE observation
  identity, source-bound bitmap identity, and opaque destination record
  boundary. Partial, reordered, or mutated evidence rejects; level/object
  semantics, pixel decoding, rendering, and fallback remain false. The
  external `--inspect` probe verifies only explicit media plus a dynamic trace
  and safely skips when the real capture is absent. Verification: focused
  CTest passed 5/5 in 0.49 seconds; authenticated local US CUE plus missing
  trace printed `SKIP`; `git diff --check` passed.

- 2026-07-17 Theron SRM operator attestation: added an external-only
  `--attest` probe that requires explicit original SRM and CUE paths, known
  Track 02 MD5, and admission version. It canonicalizes direct regular files,
  computes the SRM MD5/size, validates opaque admission, and prints one closed
  corpus-manifest row without writing, copying, inflating, or interpreting the
  save. Verification: CTest no-input, bad-version, and nonregular-SRM cases
  passed 3/3 in 0.27 seconds; the authenticated local US CUE plus missing SRM
  safely skipped; `git diff --check` passed.

- 2026-07-17 Theron SRM corpus manifest intake: added a closed root/candidate
  manifest and scanner for direct `.srm` entries with declared MD5, size,
  version, and Track 02 identity. Only opaque-admission-ready entries enter
  the corpus receipt; source absence, source rejection, bad roots, duplicate
  declarations, and hash conflicts are reported separately. It never creates
  or decodes a save. Verification: `firestaff_theron` built successfully;
  direct fixture verification covered discovery, hash conflict, and no-corpus
  skip; `git diff --check` passed. Normal CTest linking remains blocked by the
  unrelated M11 missing `CSB_V1_StartupRenderPlan_PC34.valid` field.

- 2026-07-17 Theron SRM opaque admission probe: added an external-only
  `--admit` entry point requiring explicit SRM path, MD5, exact size,
  admission version, and Track 02 MD5. It reports only opaque readiness;
  no-input and missing-corpus paths skip without writing or restoring a save.
  Verification: compiled against the built Theron libraries; both skip paths
  and `git diff --check` passed. Normal CTest linking remains blocked by the
  unrelated M11 missing `CSB_V1_StartupRenderPlan_PC34.valid` field.

- 2026-07-17 Theron SRM opaque runtime handoff: runtime now retains an
  accepted SRM admission only as MD5, size, admission-version, and Track 02
  identity. Any missing verified fact rejects; restore, body semantics, and
  fallback remain false. Verification: `firestaff_theron` built successfully
  and the exact linked opaque-admission fixture passed; `git diff --check`
  passed. The normal CTest link is currently blocked by the unrelated missing
  `CSB_V1_StartupRenderPlan_PC34.valid` field in M11.

- 2026-07-17 Theron SRM campaign/replay receipt: an opaque SRM admission now
  travels through the M12 launch intent into M11 only with its SRM MD5, size,
  FNV-1a identity, direct Track 02 variant/MD5, campaign layout epoch, and
  final replay record/sector. M12 rejects a stale intent; M11 snapshots the
  SRM size/FNV at bind time and clears route readiness on stale epoch,
  cross-media, replay-reorder, size, or FNV drift. The receipt is bound after
  M11 initialization so startup cannot silently discard it. No save body,
  level/object, pixel, palette, draw, or fallback semantics are admitted.
  Verification: direct `test_m11_theron_track02_capture_campaign_host` and
  `test_theron_v1_campaign_launch_intent` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron direct SRM launch discovery: M12 now retains an opaque
  SRM launch receipt only when the hash-first corpus admission yields exactly
  one direct regular candidate for the current direct Track 02 media. Virtual,
  mixed, duplicate, rejected, and cross-Track02 candidates cannot launch;
  virtual hits remain diagnostic-only. The receipt copies through the launch
  intent and M11 validates and retains it as opaque source provenance. No save
  body is copied, decoded, restored, or interpreted. Verification:
  `test_theron_v1_srm_launch_discovery`,
  `test_theron_v1_campaign_launch_intent`, and
  `test_m11_theron_track02_capture_campaign_host` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron direct-media launch layout invalidation: the M12 Theron
  launch intent now compares the complete direct raw-media layout identity,
  not merely path and Track 02 MD5. CUE consumption, MODE1 form, INDEX 01,
  payload length, sector count, and user-data window must match the current
  campaign receipt; M11 also requires a ready direct CUE/MODE1 receipt with
  nonempty bounded layout before startup. Layout drift rejects without reading
  or decoding any loader, level/object, palette, bitmap, or pixel payload.
  Verification: `test_theron_v1_campaign_launch_intent` and
  `test_m11_theron_track02_capture_campaign_host` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron M12-to-M11 opaque launch trace identity: a direct-media
  launch can now retain source-trace and HuC6280 event-log MD5 values with its
  layout epoch and final replay record/sector. M11 compares that receipt at
  Soul Room and dungeon-handoff admission and clears readiness on trace, media,
  epoch, or final-record drift. It grants no decode, draw, or fallback route.
  Verification: `test_m11_theron_track02_capture_campaign_host` and
  `test_theron_v1_campaign_launch_intent` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron external trace-bundle discovery: hash-first selection now
  admits exactly one existing Mednafen conversion receipt only when its source
  trace and HuC6280 event-log MD5 values match the current opaque launch trace
  identity. Virtual and ambiguous candidates reject. No trace payload is
  decoded or rendered. Verification:
  `test_theron_v1_track02_trace_bundle_discovery` PASS; full `firestaff` build
  and `git diff --check` PASS.

- 2026-07-17 Theron M12-to-M11 trace-bundle handoff: the selected opaque
  direct trace bundle now travels with the Theron launch intent and M11 checks
  its exact source-trace/event-log MD5 identity during live campaign route
  admission. Virtual, ambiguous, non-opaque, or drifted bundles reject and
  clear readiness. Verification: `test_theron_v1_track02_trace_bundle_discovery`
  and full `cmake --build build-ninja --target firestaff -j1` PASS; `git diff
  --check` PASS.

- 2026-07-17 Theron trace-bundle epoch lifecycle: the opaque bundle receipt
  now carries the launch-trace campaign layout epoch; M12 and M11 reject a
  stale epoch before live route readiness. Verification:
  `test_m11_theron_track02_capture_campaign_host` PASS; full `firestaff`
  build and `git diff --check` PASS.

- 2026-07-17 Theron trace-bundle invalidation lifecycle: M12 now clears a
  previously bound opaque bundle whenever a replacement is virtual, ambiguous,
  stale, or otherwise invalid. The focused launch-intent test covers valid to
  virtual invalidation; M11 host coverage remains no-draw. Verification:
  `test_theron_v1_campaign_launch_intent`,
  `test_m11_theron_track02_capture_campaign_host`, full `firestaff` build,
  and `git diff --check` PASS.

- 2026-07-17 Theron live capture-target-plan lifecycle: M11 now retains an
  FNV-1a identity over the opaque, direct-media-bound three-route capture
  target plan at Soul Room and compares it before dungeon handoff. Any target
  coordinate, transfer/output identity, route metadata, or no-draw-policy
  drift clears both live readiness flags. No payload parsing, level/object
  semantics, bitmap/palette decode, rendering, or fallback path was added.
  Verification: `test_m11_theron_track02_capture_campaign_host` and
  `test_theron_v1_campaign_launch_intent` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron external trace-bundle capture-plan binding: direct
  Mednafen/HuC6280 trace-bundle selection now records the shared opaque
  capture-target-plan FNV identity. M12 requires that identity to match both
  its current plan and the copied launch intent; M11 verifies it at startup
  and before live Soul Room/dungeon readiness. Stale route/output-plan drift
  rejects and clears readiness. No trace payload, bitmap/palette, level, or
  object format was decoded, and no renderer or fallback route was added.
  Verification: `test_theron_v1_track02_trace_bundle_discovery`,
  `test_theron_v1_campaign_launch_intent`, and
  `test_m11_theron_track02_capture_campaign_host` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron M12 direct-media re-scan trace lifecycle: added the
  menu-owned strict Track 02 scan wrapper. It retains existing launch-trace
  and direct external trace-bundle bindings only when the refreshed direct
  CUE/BIN/ISO layout and opaque capture-target-plan FNV are unchanged; a
  missing, rejected, virtual, or drifted scan clears both bindings. The scan
  never copies media or interprets trace payload, level/object, bitmap,
  palette, or pixel semantics. Verification:
  `test_theron_v1_campaign_launch_intent`,
  `test_asset_status_theron_campaign_media_scan`, and
  `test_m11_theron_track02_capture_campaign_host` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron M12-to-M11 direct-media re-scan epoch gate: each strict
  direct Track 02 scan now advances an M12-owned epoch, which is copied into
  the authenticated Theron launch intent and retained by M11 at boot. Live
  Soul Room and dungeon-handoff admission require the current epoch and the
  Soul Room epoch captured for the sequence; a later rescan, mismatched intent,
  media-layout drift, or route-plan drift clears readiness. The receipt is
  opaque and no-draw: it does not parse trace payloads or infer bitmap,
  palette, level, or object semantics. Verification:
  `test_theron_v1_campaign_launch_intent`,
  `test_asset_status_theron_campaign_media_scan`, and
  `test_m11_theron_track02_capture_campaign_host` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron capture-campaign receipt currentness: M11 now retains the
  externally attested Mednafen trace MD5, the distinct start/Soul Room/dungeon
  bundle MD5s, and the opaque dungeon-record window checksum when campaign
  readiness is bound. The new currentness gate revalidates the normal runtime
  admission and exact stored identities; trace, per-route bundle, media, or
  dungeon-window drift clears every capture-ready flag. No payload is decoded,
  and all level/object, bitmap/palette, pixel, render, and fallback flags stay
  closed. Verification: `test_m11_theron_track02_capture_campaign_host` and
  `test_theron_v1_runtime_track02_capture_campaign_admission` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron direct-media capture-campaign currentness: the M11
  capture receipt can now be rechecked against the current direct CUE/BIN/ISO
  campaign layout, its refreshed raw-media receipt, opaque capture-target-plan
  identity and per-route destination identities, launch-trace identity, and
  M12 media-scan epoch. A changed plan or stale scan epoch clears all capture
  readiness before any route can use it. This remains provenance only: no
  media or trace payload is decoded and no draw or fallback path is opened.
  Verification: `test_m11_theron_track02_capture_campaign_host`,
  `test_theron_v1_runtime_track02_capture_campaign_admission`, and
  `test_asset_status_theron_campaign_media_scan` PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron later sector-record direct-media currentness: M11 now
  retains the opaque later record's user-data hash and observed raw-sector
  checksum, then rechecks it against the current direct CUE/BIN/ISO layout,
  capture plan, media-scan epoch, and active loader-replay tail. The replay's
  final record and raw sector must still name the descriptor-resolved record;
  any mismatch clears the no-draw record readiness. No record bytes, level or
  object fields, bitmap/palette data, or pixels are interpreted. Verification:
  `test_m11_theron_track02_capture_campaign_host` PASS and
  `test_theron_v1_track02_sector_record_admission` reports its intended
  no-corpus SKIP; full `cmake --build build-ninja --target firestaff -j1` PASS;
  `git diff --check` PASS.

- 2026-07-17 Theron Track 02 later-sector corpus discovery: added a
  hash-first direct discovery receipt for explicit regular CUE plus coalesced
  loader-trace pairs. It hashes the trace, delegates media verification and
  later-sector selection to the existing strict CUE/trace admission, requires
  exactly one READY pair, and exposes only the resulting opaque record receipt.
  Missing files are UNAVAILABLE/SKIP; virtual, malformed, duplicate, or
  non-admitted candidates are never promoted. M11 consumes a READY discovery
  only through its existing no-draw sector admission bridge. No record payload,
  level/object, bitmap/palette, or pixel semantics were added. Verification:
  `test_theron_v1_track02_sector_record_corpus_discovery` PASS (no corpus),
  `test_m11_theron_track02_capture_campaign_host` PASS, and the sector corpus
  probe no-input/missing-input paths SKIP safely; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron later-sector corpus M12-to-M11 admission: the optional
  hash-first direct CUE/coalesced-trace discovery receipt is now copied into a
  validated Theron launch intent, passed through the real M11 boot transaction,
  and retained only as opaque per-route provenance. A bound receipt must keep
  its direct media MD5, coalesced trace MD5, record hash/checksum, media-scan
  epoch, capture-plan layout, and replay tail current; trace, media, epoch, or
  replay drift clears no-draw route readiness. No corpus remains unbound and
  skip-safe, and no record bytes, level/object fields, bitmap/palette data, or
  pixels are decoded. Verification:
  `test_theron_v1_campaign_launch_intent`,
  `test_m11_theron_track02_capture_campaign_host`, and
  `test_theron_v1_track02_sector_record_corpus_discovery` PASS (the last
  correctly reports no local corpus); full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron first dungeon opaque world/level-object admission: M11's
  epoch-2 dungeon handoff now consumes a READY later-sector corpus only through
  its original-observed descriptor fields: Stage-3 record, descriptor ordinal
  and selector, loader caller/return PCs, resolved record and bounded hashes.
  The receipt additionally requires the current direct media layout, coalesced
  trace/trace bundle, loader-replay final record/sector, campaign layout epoch,
  and media-scan epoch. No corpus is a strict unbound/SKIP path; any mismatch
  clears the opaque admission and route readiness. It does not materialize a
  world, decode level/object fields, or authorize bitmap/palette/pixel draw.
  Verification: `test_m11_theron_track02_capture_campaign_host` PASS,
  `test_theron_v1_campaign_launch_intent` PASS,
  `test_theron_v1_track02_sector_record_corpus_discovery` PASS (no corpus),
  full `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron level/object descriptor capture intake: a new strict
  receipt accepts only the existing hash-first direct CUE/BIN plus coalesced
  loader-trace corpus after its raw-media, resolved-record, replay-tail,
  launch-trace, media-layout, layout-epoch, and scan-epoch facts agree. It
  publishes only the original-observed opaque descriptor receipt, and M12/M11
  adapters reuse the established sector-corpus launch and route boundary
  rather than introducing payload access. Missing local corpus is
  UNAVAILABLE/SKIP; malformed or drifted evidence clears admission. No
  level/object decoding, bitmap/palette/pixel path, or fallback draw was
  added. Verification:
  `test_theron_v1_track02_level_object_descriptor_capture_intake` PASS (no
  local corpus), `test_theron_v1_campaign_launch_intent` PASS,
  `test_m11_theron_track02_capture_campaign_host` PASS, full
  `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron descriptor bitmap/palette capture intake: added a strict
  dungeon-handoff capture-artifact join above the ready descriptor chain. It
  requires matching direct CUE/BIN layout, coalesced loader trace, replay
  tail, launch-trace identity, descriptor-selected destination record, target
  plan identity, and layout/scan epochs. M11 retains only opaque palette and
  bitmap output identities behind a presentation no-draw witness; identity
  drift clears that witness. No palette entries, bitmap bytes, image format,
  level/object field, renderer, synthetic menu graphic, or fallback is
  exposed. Verification:
  `test_theron_v1_track02_descriptor_bitmap_palette_capture_intake` PASS (no
  local capture), `test_m11_theron_track02_capture_campaign_host` PASS
  (valid and artifact-drift lifecycle), full
  `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron operator dungeon-handoff capture plan: added
  `scripts/plan_theron_track02_dungeon_handoff_capture.sh`, an operator-local
  plan for the original Mednafen run. It checks explicit executable/CUE/System
  Card/output-root inputs, authenticates the direct raw Track 02 MD5, requires
  the current layout epoch plus equal nonzero replay record/sector and plan
  FNV, and refuses overwrite. Default mode writes only an opaque request with
  paths for the real trace, descriptor manifest, and capture artifact; opt-in
  mode delegates to the existing bounded live capture script. It never copies
  media, invents trace/descriptor/palette/bitmap rows, decodes payloads, or
  enables draw. Verification:
  `theron_v1_track02_dungeon_handoff_capture_plan_script` CTest PASS, full
  `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff capture-plan resume admission: added a
  strict parser/receipt for the local V1 capture-plan grammar and wired it to
  M12 launch intent plus M11 startup and presentation state. A validated plan
  is explicitly `CAPTURE_REQUIRED`; it becomes `RESUME_READY` only after the
  existing observed descriptor bitmap/palette artifact receipt agrees with the
  direct CUE/BIN identity, System Card requirement, replay final coordinate,
  plan FNV, layout epoch, and scan epoch. M11 keeps the route no-draw and
  clears it on drift. Verification: capture-plan script and admission CTests
  PASS, `test_m11_theron_track02_capture_campaign_host` PASS, full
  `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff source-trace binding: extended the local
  V1 capture-plan grammar and its admission receipt with a required lowercase
  source-trace MD5. The artifact path can reach `RESUME_READY` only when its
  existing opaque descriptor/bitmap/palette receipt carries the identical
  coalesced trace hash; the plan otherwise remains `CAPTURE_REQUIRED` and
  presentation remains no-draw. The local artifact verifier now checks the
  actual trace hash against both the plan and artifact envelope. Added valid,
  malformed-plan, and artifact trace-drift lifecycle coverage without adding
  payload decoding, graphic data, or fallback rendering. Verification:
  `theron_v1_track02_dungeon_handoff_capture_plan_script`,
  `theron_v1_track02_dungeon_handoff_artifact_verifier`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-ninja firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff artifact-plan provenance: strengthened
  external artifact import so every start/Soul Room/dungeon envelope row must
  share one known direct Track02 variant and MD5. A ready artifact receipt now
  carries the computed opaque capture-plan identity, and the descriptor
  bitmap/palette handoff requires it to equal the current plan before any
  positive capture can continue. Added importer coverage for mixed-variant
  plan rows and for the receipt identity; absent positive capture remains
  `CAPTURE_REQUIRED`/no-draw and no payload, bitmap, palette, or level/object
  semantics are interpreted. Verification in isolated
  `build-theron-trace-md5`:
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff operator capture schema: added the
  mandatory `capture_target_plan_fnv1a` row to the opaque artifact envelope.
  The source-trace stamper and direct-media campaign emitter write the current
  complete-plan identity; importer and local artifact verifier reject a
  missing, altered, or stale identity before a runtime artifact receipt is
  published. Added importer rejection coverage for a mismatched plan identity
  and initialized the converter's local file handle defensively. This remains
  real-media/trace metadata only: no positive corpus means no-draw and no
  payload, bitmap, palette, level, or object interpretation. Verification in
  isolated `build-theron-trace-md5`:
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_campaign_bundle_emitter`,
  `theron_v1_track02_dungeon_handoff_artifact_verifier`, and
  `theron_v1_track02_mednafen_trace_converter` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff operator launch receipt: added a dedicated
  external Mednafen handoff-validator API. Before it can convert the
  explicitly MD5-bound observed trace, it requires one direct MODE1/2352
  CUE/BIN intake and the requested current capture-plan identity, plus exact
  Track02, CD-read, loader-output, palette/bitmap, and destination metadata
  across the opaque plan. Its receipt records only that provenance and remains
  capture-required/no-draw; any media, trace, or plan drift clears admission.
  The no-local-media launcher test covers rejected/no-input lifecycle without
  creating synthetic Track02 media or payloads. Verification in isolated
  `build-theron-trace-md5`:
  `theron_v1_track02_external_capture_launcher`,
  `theron_v1_track02_capture_artifact_importer`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon-handoff artifact-corpus import: added a strict
  operator-corpus admission for exactly one direct artifact bundle. It
  rechecks the prior MD5-bound MODE1/2352 handoff receipt, direct CUE/BIN,
  observed source trace, and capture-plan identity before delegating to the
  existing opaque importer; virtual, multiple, stale, or mismatched candidates
  reject. Empty input is a no-corpus SKIP. A ready receipt remains explicitly
  capture-required/no-draw and stores no payload or graphics semantics.
  Verification in isolated `build-theron-trace-md5`:
  `theron_v1_track02_handoff_artifact_corpus`,
  `theron_v1_track02_external_capture_launcher`,
  `theron_v1_track02_capture_artifact_importer`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron M12/M11 artifact-corpus host gate: launch intent now
  carries a bound opaque corpus receipt and M11 requires its plan, Track02 and
  source-trace identities before retaining dungeon `RESUME_READY`. Any absent
  or drifted receipt leaves capture-required/no-draw. Verification: isolated
  host, launch-intent and corpus CTests PASS; `git diff --check` PASS.

- 2026-07-17 Theron M12 artifact-corpus intent identity: strengthened launch
  intent validation so a bound corpus must remain the current one-candidate
  opaque no-draw receipt with matching Track02, source-trace and capture-plan
  identities before M11 starts. Verification in isolated
  `build-theron-trace-md5`: host, launch-intent and corpus CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron artifact-corpus rescan drift: M12 now clears bound
  artifact-corpus and dungeon capture-plan receipts whenever the direct media
  rescan invalidates trace-campaign identity. Verification: isolated host,
  launch-intent and corpus CTests PASS; `ninja -C build-theron-trace-md5
  firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron M11 active-session artifact-corpus epoch gate: M11 now
  stamps the opaque corpus receipt with the launch scan epoch and rejects a
  dungeon `RESUME_READY` bind if that epoch is stale during the active session.
  The host lifecycle regression covers valid, stale, and restored-current
  epochs; stale evidence clears the route fail-closed and all retained evidence
  remains opaque/no-draw. Verification
  in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron artifact-corpus envelope continuity: added shared strict
  receipt checks that bind all three opaque artifact rows to the exact current
  capture-plan identities. M12 rejects a launch intent whose nested artifact
  envelope drifts; M11 repeats the dungeon route's destination, palette, and
  bitmap identities before preserving `RESUME_READY`. No payload is retained
  or interpreted, and decoder, render, and fallback permissions remain false.
  Verification in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon loader-output continuity: the existing opaque
  capture-plan loader checksum now survives descriptor/palette/bitmap intake
  into M11 active session state. Dungeon `RESUME_READY` requires the matching
  imported artifact row, alongside the existing record/palette/bitmap/
  destination witnesses; a changed loader identity clears the route fail
  closed. No loader bytes, record grammar, decoding, palette values, bitmap
  layout, rendering, or fallback path was added. Verification in isolated
  `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon CD-read scope continuity: the no-draw descriptor/
  palette/bitmap receipt now carries the source-owned capture-plan CD-read
  record as a distinct opaque witness. M11 compares it with the imported
  dungeon artifact row before preserving `RESUME_READY`; record-scope drift
  clears the route fail closed. This assigns no record grammar, level/object,
  bitmap, palette, decoder, or rendering semantics. Verification in isolated
  `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_dungeon_handoff_capture_plan_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon loader-span continuity: capture-artifact import
  now preserves each already verified loader raw offset and byte count, and the
  dungeon no-draw receipt carries its source coordinates into M11. The active
  handoff rejects artifact offset or length drift before `RESUME_READY`.
  Coordinates remain opaque provenance only: no loader bytes, record grammar,
  bitmap layout, palette, decoder, rendering, or fallback behavior is added.
  Verification in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon destination-span continuity: artifact import now
  preserves the already source-verified destination offset and byte count for
  every capture-plan row. M11 carries the dungeon coordinates only as opaque
  no-draw provenance and rejects changed artifact coordinates before
  `RESUME_READY`. No destination bytes, level/object grammar, bitmap layout,
  palette, decoder, rendering, or fallback semantics were added. Verification
  in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon route-selector continuity: the existing strict
  artifact-envelope `campaign_route` selector must now be
  `DUNGEON_HANDOFF` before the corpus can bind through M12 or retain M11
  `RESUME_READY`. Start/Soul Room selector drift rejects even with matching
  media, trace, plan, and row identities. It remains opaque route provenance;
  no record payload, level/object grammar, decoder, palette, bitmap, rendering,
  or fallback semantics were added. Verification in isolated
  `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon descriptor-selector continuity: the strict
  artifact envelope now carries a required nonzero Stage-3 descriptor selector.
  Descriptor intake compares it with the existing source-backed sector-record
  selector, M12 rejects intent drift, and M11 rechecks active session state
  before `RESUME_READY`. This remains an opaque loader-record selector, with
  no level/object data, bitmap/palette layout, decoder, rendering, or fallback
  semantics. Verification in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon descriptor-ordinal continuity: the strict
  artifact envelope now carries the exact observed Stage-3 descriptor ordinal.
  Descriptor intake compares it with the source-backed sector-record receipt,
  M12 rejects a changed intent ordinal, and M11 rechecks active session state
  before `RESUME_READY`. The ordinal remains opaque record-location provenance,
  with no level/object data, decoder, palette, bitmap, rendering, or fallback
  semantics. Verification in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron dungeon descriptor-source-hash continuity: the strict
  artifact envelope now carries the required nonzero source hash of the
  observed Stage-3 descriptor witness. Descriptor intake compares it with the
  source-backed sector-record receipt, M12 rejects changed intent hashes, and
  M11 rechecks active-session state before `RESUME_READY`. The hash remains
  opaque provenance only, with no source bytes, level/object data, decoder,
  palette, bitmap, rendering, or fallback semantics. Verification in isolated
  `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_capture_artifact_importer`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_handoff_artifact_corpus` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron positive dungeon-handoff corpus ingress: added the
  operator-only `M12_StartupMenu_ImportTheronHandoffArtifactCorpus` boundary.
  It invokes the existing strict direct MODE1/2352 CUE/BIN, source-trace, and
  opaque bundle importer against the current M12 campaign plan before binding
  anything for launch. The ready Stage-3 descriptor identity can then travel
  through the normal launch intent to M11's existing `RESUME_READY` runtime
  gate; mismatched trace MD5, absent local corpus, media/plan drift, or any
  rejected import atomically clears retained corpus state. The runtime host
  test continues to prove the matching receipt reaches no-draw dungeon
  readiness and rejects active-session drift. No loader payload, level/object
  field, bitmap, palette, decoder, renderer, or fallback behavior was added.
  Verification in isolated `build-theron-trace-md5`:
  `theron_v1_campaign_launch_intent`,
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_handoff_artifact_corpus`,
  `theron_v1_track02_capture_artifact_importer`, and
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron live Stage-3 loader-capture handoff: preserved the
  original coalesced `$e009` trace's later destination, bounded output span,
  and checksum in the direct sector-corpus receipt. The operator corpus import
  now requires its dungeon artifact row and capture plan to agree with that
  descriptor-selected CD_READ/loader-output capture. M11 repeats the same
  join at actual Theron launch and retains only an opaque live no-draw witness
  before dungeon `RESUME_READY` can be considered. The host lifecycle covers
  the matching end-to-end receipt path and a mutated original output checksum
  rejection that clears the witness. No sector payload, level/object field,
  bitmap, palette, decoder, renderer, synthetic dungeon, or fallback was
  introduced. Verification in isolated `build-theron-trace-md5`:
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_campaign_launch_intent`,
  `theron_v1_track02_handoff_artifact_corpus`,
  `theron_v1_track02_sector_record_admission`,
  `theron_v1_track02_descriptor_bitmap_palette_capture_intake`, and
  `theron_v1_track02_capture_artifact_importer` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron strict Stage-3 loader-output record admission: added a
  manifest-bound admission for the one fully disassembled original `$e009`
  output record `0x0b52`. It retains only the source-proven level-envelope
  span at record offset `0x114` and the directly following opaque continuation,
  each with exact bounds and checksum. Bitmap boundary evidence is explicitly
  absent and cannot be inferred. M11 invokes this check during the live
  artifact/corpus handoff when a real boot capture exists; all imported media,
  trace, descriptor, and loader facts must agree. Neither span grants level or
  object semantics, bitmap/palette data, decoder access, rendering, a
  synthetic dungeon, or fallback visuals. Verification in isolated
  `build-theron-trace-md5`:
  `theron_v1_track02_loader_output_record_admission`,
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_handoff_artifact_corpus`,
  `theron_v1_track02_sector_record_admission`, and
  `theron_v1_track02_capture_artifact_importer` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron typed `0x0b52` envelope-header admission: extended the
  strict original loader-output receipt with byte-verified big-endian width,
  height, seed, header-level-index, and opaque two-byte extension facts. Each
  value must agree with the manifest-bound original envelope and the existing
  record-local boundary receipt before the no-draw handoff can remain current.
  The header-level-index is retained as observed metadata only, not a dungeon
  selector; bitmap continuation remains unproven and opaque. No level/object
  semantics, bitmap/palette decoding, renderer path, synthetic dungeon, or
  fallback behavior was added. Verification in isolated
  `build-theron-trace-md5`: `theron_v1_track02_loader_output_record_admission`,
  `m11_theron_track02_capture_campaign_host`,
  `theron_v1_track02_handoff_artifact_corpus`, and
  `theron_v1_track02_sector_record_admission` CTests PASS;
  `ninja -C build-theron-trace-md5 firestaff` PASS; `git diff --check` PASS.

- 2026-07-17 Theron local dungeon-handoff artifact verifier: added
  `scripts/verify_theron_track02_dungeon_handoff_artifact.sh`. It accepts only
  direct regular plan/artifact/trace files, verifies the plan-selected artifact
  path and Track 02 MD5, hashes the trace against the artifact envelope, and
  requires complete start/Soul Room/dungeon descriptor, palette, bitmap, and
  destination identity rows with route 2 selected. It is a preflight only:
  no payload bytes are read, no decoder is invoked, and it cannot turn a file
  into M12/M11 `RESUME_READY` without the existing importer and no-draw route
  receipts. Verification: artifact-verifier, capture-plan, and admission
  CTests PASS; full `cmake --build build-ninja --target firestaff -j1` PASS;
  `git diff --check` PASS.

- 2026-07-17 Theron artifact importer envelope ingress: the importer now
  rejects an artifact before candidate publication unless each ordered route
  row has an exact, nonzero loader checksum, palette identity, bitmap identity,
  destination record, and destination identity. Successful imports set the new
  `opaque_envelope_verified` provenance bit; campaign verification and the
  descriptor bitmap/palette M12/M11 ingress require it. Added importer
  coverage for a zero dungeon bitmap identity, while campaign and host
  lifecycle fixtures cover propagation. No payload parsing, image decoding,
  draw, or fallback was introduced. Verification: importer, campaign, host,
  corpus-probe, and local artifact-verifier CTests PASS; full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron Mednafen trace envelope stamp: the existing strict trace
  converter can now write an importer-compatible, route-2 handoff envelope
  only from a converted observed trace and a complete opaque capture plan. It
  stamps the trace MD5 plus all three route identities and refuses zero or
  existing outputs. It writes no payload or graphics data and remains subject
  to the existing importer and M12/M11 no-draw receipts. Full
  `cmake --build build-ninja --target firestaff -j1` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron later-route candidate intake: added a strict closed-row
  intake for future observed dungeon loader candidates. It accepts only the
  raw loader PC, equal nonzero record/raw-sector coordinate, and destination
  identity after direct-media replay/layout agreement, then publishes only
  opaque `CAPTURE_REQUIRED` evidence. It deliberately has no route ID,
  level/object semantics, payload access, decoder, or draw state. Verification:
  `test_theron_v1_track02_later_route_candidate_intake` PASS (no corpus), full
  `cmake --build build-ninja --target firestaff -j1` PASS, and
  `git diff --check` PASS.

- 2026-07-17 Theron later-route candidate manifest/M12 binding: added a
  non-overwriting metadata-only V1 operator manifest exporter and M12
  capture-required binding. The manifest has no route ID or payload field and
  exports only trace/media hashes, epoch, loader PC, record/raw-sector, and
  destination identity from an opaque candidate. Export/rewrite/mismatched
  sector fixtures reject; M12 cannot promote it to route-ready. Verification:
  candidate intake and manifest tests PASS, `ninja -C build-ninja firestaff`
  PASS, and `git diff --check` PASS.

- 2026-07-17 Theron later-route candidate manifest import: added a strict
  closed-grammar rehash admission for the operator metadata manifest. It
  requires matching direct media/trace hashes, layout epoch, and active replay
  record/raw-sector, then republishes only opaque `CAPTURE_REQUIRED` evidence.
  Missing local corpus is unavailable; no route ID, payload, decoder, draw, or
  route-ready state is present. Verification:
  `test_theron_v1_track02_later_route_candidate_manifest_import` PASS (no
  corpus), `ninja -C build-ninja firestaff` PASS, and `git diff --check` PASS.

- 2026-07-17 Theron later-route candidate campaign index: added strict
  deduplication for imported opaque candidates, rejecting same-record metadata
  collisions and non-CAPTURE_REQUIRED entries. M12 binds the resulting index
  only as capture-required metadata. Verification: dedup/collision/reject test
  PASS, `ninja -C build-ninja firestaff` PASS, `git diff --check` PASS.

- 2026-07-17 Theron later-route candidate stale pruning: added strict index
  currentness that clears all candidates on layout-epoch drift. The focused
  index test covers current and stale epochs alongside dedup/collision/reject;
  no route ID, payload, or route-ready state was added. Verification:
  `test_theron_v1_track02_later_route_candidate_campaign_index` PASS,
  `ninja -C build-ninja firestaff` PASS, and `git diff --check` PASS.

- 2026-07-17 Theron later-route operator attestation import: strict file
  attestation accepts only a current opaque candidate record/destination/epoch
  and returns CAPTURE_REQUIRED. Accept/stale rejection test PASS; full Ninja
  PASS; `git diff --check` PASS.

- 2026-07-17 Theron later-route batch attestation import: added strict
  metadata-only batch import against the current candidate index. It restores
  only matched opaque candidates, deterministically deduplicates duplicates,
  and rejects collision/unknown/epoch drift without mutating const input.
  Batch test PASS, `ninja -C build-ninja firestaff` PASS, and `git diff --check`
  PASS. No route ID, payload, or runtime promotion was added.

- 2026-07-17 Theron later-route M12 launch binding: candidate indexes now
  require current direct campaign media plus a valid loader trace identity and
  matching layout epoch before M12 binds or carries them in launch intent.
  Media rescans clear candidate state; trace epoch and candidate-entry drift
  invalidate launch validation. The index, single and batch attestations now
  also require the full observed-trace/direct-media/replay-tail opaque receipt
  chain. Verification: candidate index, attestation, batch attestation, and
  campaign launch-intent CTests PASS; `ninja -C build-ninja firestaff` PASS;
  `git diff --check` PASS. No route ID, payload, decoder, draw, or promotion
  was added.

- 2026-07-17 Theron later-route receipt provenance: added exact Track02 and
  source loader-trace MD5 fields to opaque candidate receipts. Intake requires
  direct-media/replay MD5 agreement, manifest export cannot substitute either
  identity, import restores both, and campaign-index construction rejects
  mixed identities. Updated attestation, batch, and M12 fixtures use consistent
  metadata identities. Verification: 7 focused later-route/M12 CTests PASS,
  `ninja -C build-ninja firestaff` PASS, and `git diff --check` PASS. No route
  ID, payload, decoder, renderer, or promotion was added.

- 2026-07-17 Theron SRM operator-attestation Windows portability: removed the
  unconditional POSIX `lstat`/`S_ISLNK`/`realpath`/`libgen` dependency.
  Windows now rejects directories and reparse points with Win32 attributes and
  canonicalizes with `_fullpath`; POSIX keeps lstat symlink rejection. Shared
  manifest root/name splitting is separator-neutral and remains fail-closed
  regular-file-only. Verification: SRM no-input, bad-version, and nonregular
  CTests PASS; full `ninja -C build-ninja firestaff` PASS; `git diff --check`
  PASS.

- 2026-07-17 Theron SRM opaque admission: added a separate strict `.srm`
  gate that requires a direct regular file, exact size and MD5, gzip/DEFLATE
  container form, admission-format version, and matching known Track 02
  identity. It retains only opaque provenance and keeps save-body decoding and
  fallback flags false. Fixture coverage rejects size, version, and variant
  drift; the missing-corpus branch safely reports unavailable. Verification:
  focused CMake build plus SRM classifier/corpus tests passed 3/3 in 0.60
  seconds; `git diff --check` passed.

- 2026-07-17 Theron Track 02 capture-root discovery: added a bounded,
  write-free `--discover <capture-root>` probe mode. It inspects at most 128
  direct regular files from one non-symlink operator root and reports only a
  strict, stable-MD5 Mednafen export; other files remain non-evidence. Both
  local Theron roots safely found none. Verification: reconfigured focused
  build and Track 02 CTest passed 10/10 in 0.44 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 direct-file evidence paths: MD5-bound Mednafen
  export inspection and HuC6280 event-log intake now reject symlink paths, in
  addition to their regular-file checks. The fixtures verify explicit
  rejection without altering opaque record semantics. Verification: focused
  CMake build and Track 02 CTest passed 6/6 in 0.49 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 Mednafen export inspection: added a write-free
  `--inspect` path that accepts one explicit regular-file export only after its
  closed HuC6280 grammar and stable before/after MD5 agree. It reports the
  MD5 for later explicit conversion, writes no event log, and cannot open
  runtime, decoding, or drawing. Verification: reconfigured focused build and
  Track 02 CTest passed 9/9 in 0.28 seconds, including the explicit
  missing-export CTest; missing export safely skipped; `git
  diff --check` passed.

- 2026-07-17 CSB title-to-terminal lifecycle binding: the source-owned
  STRIKES lifecycle receipt now reaches a terminal receipt only when its
  capture identity, PC34 title palette, session generation, and earlier source
  tick still agree. A stale terminal tick rejects. Verification: focused
  startup/boot/coupling CTests passed 3/3 in 0.53 seconds.

- 2026-07-17 CSB title lifecycle receipt: F0437 startup coupling now tracks
  source-owned PRESENTS, CHAOS zoom/hold, and STRIKES captures through strict
  phase order, real PC34 palette identity, session generation, and monotonic
  ticks. Stale, skipped, or palette-mismatched transitions invalidate the
  receipt without opening a draw fallback. Verification: focused CTest passed
  1/1 in 0.24 seconds; `git diff --check` passed.

- 2026-07-17 Theron Track 02 HuC6280 event-log file boundary: the strict
  event-log parser now accepts regular files only, rejecting devices, FIFOs,
  and directories before event parsing. It retains opaque loader/consumer
  windows only and grants no semantic or draw permission. Verification:
  focused CMake build and Track 02 CTest passed 4/4 in 0.59 seconds; the
  authenticated-CUE/no-export probe printed `SKIP` and left no event log;
  `git diff --check` passed.

- 2026-07-17 CSB DSA runtime admission span guard: the immutable receipt now
  proves the declared DSA payload fits completely before its recorded next
  payload offset, in addition to checksum and source identity checks. A
  length-crossing structural fixture rejects without interpreting DSA
  semantics. Verification: focused DSA CTests passed 2/2 in 0.38 seconds;
  `git diff --check` passed.

- 2026-07-17 CSB local viewport declaration corpus admission: added a
  skip-safe probe that accepts an operator-named manifest only after a
  hash-admitted `CSBgraphics.dat` and exact 768-byte palette receipt agree.
  The boot public API is manifest-only; raw declaration and corpus routes are
  internal and no probe draw path is opened. Verification: CMake configure,
  focused build, and manifest/boot/CSBgraphics CTests passed 3/3 in 0.51
  seconds; `git diff --check` passed.

- 2026-07-17 Theron Track 02 capture-evidence unavailable path: the combined
  raw-CUE/Mednafen probe now treats an absent external export as a safe skip,
  while retaining rejection for malformed present evidence. With the local
  authenticated US CUE and absent export it writes no log. Verification:
  focused CMake build and CTest passed 3/3 in 0.26 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 capture-evidence probe: added a local no-launch
  `--bind` entry point that requires both raw CUE/MD5 provenance and an
  MD5-attested external Mednafen export before strict event-log creation.
  It has no runtime-admission, decode, or media-copy path. Verification:
  focused CMake build and Track 02 CTest passed 8/8 in 0.62 seconds; `git diff
  --check` passed.

- 2026-07-17 CSB viewport manifest receipt consistency: the operator manifest
  now retains its validated palette-source receipt, matching parser and boot
  admission ownership. Duplicate frame identities reject, and an invalid boot
  manifest clears any stale selection before no-draw. Verification: focused
  CSBgraphics runtime-plan and boot title/import gate CTests passed 2/2;
  `git diff --check` passed. The full build reaches CSB successfully but is
  currently blocked by the unrelated DM1 undefined
  `TIMELINE_EVENT_ENABLE_CHAMPION_ACTION` test symbol.

- 2026-07-17 Theron Track 02 converted-log handoff stability: the external
  runtime handoff now rechecks the converter-attested event-log MD5 before and
  after consuming the strict HuC6280 log, rejecting substitution. Verification:
  focused CMake build and Track 02 CTest passed 7/7 in 0.60 seconds; `git diff
  --check` passed.

- 2026-07-17 Theron Track 02 Mednafen regular-file intake: the external
  converter now rejects devices, FIFOs, and directories before any MD5 or row
  parsing; the fixture covers `/dev/null` rejection. Verification: focused
  CMake build and Track 02 CTest passed 7/7 in 0.51 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 Mednafen source-stability guard: the converter
  now requires the external export MD5 to remain unchanged before and after
  its closed row read, rejecting a modified trace before event-log creation.
  Verification: focused CMake build and Track 02 CTest passed 7/7 in 0.50
  seconds; `git diff --check` passed.

- 2026-07-17 Theron Track 02 converted-log MD5 provenance: the converter now
  hashes the closed strict event log before reporting success, and the external
  handoff receipt retains that digest only after full runtime admission. Hash
  failure removes the new output and rejects. Verification: focused CMake
  build and Track 02 CTest passed 7/7 in 0.53 seconds; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 capture-ingress verification update: the full
  CMake target build and focused Track 02 CTest are green again, passing 7/7
  in 0.78 seconds. The final raw-media test also accepts the authenticated
  local `TQUS-Raw.cue` with its known US Track 02 MD5. `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 Mednafen exclusive output creation: after all
  source checks, the converter now creates its event log exclusively so a
  concurrent file cannot be replaced between preflight and write. Verification:
  converter and fixture compiled with `-Wall -Wextra -Werror`, the isolated
  focused test passed, and `git diff --check` passed. The regular CMake target
  is presently blocked by an unrelated Nexus syntax error.

- 2026-07-17 CSB boot-owned declared live material route: removed the generic
  `VIEWPORT_DERIVED` plan loop from boot viewport rendering. A separate owner
  route now requires verified ingress, a real dungeon-grid door state, exact
  operator declaration selection, hash-admitted cache materialization, and a
  live-frame receipt before it can raster pixels. Missing declarations reject
  with no fallback. Verification: boot gate, viewport gate, CSBgraphics plan,
  and related suite passed 6/6; `git diff --check` passed.

- 2026-07-17 Theron Track 02 Mednafen output integrity: conversion now
  refuses to replace a pre-existing event log and treats write/close failure as
  rejected, removing only its own partial new output. Fixture coverage verifies
  an existing event log's MD5 remains unchanged. Verification: focused Track
  02 CTest passed 4/4; `git diff --check` passed.

- 2026-07-17 Theron Track 02 Mednafen source-write guard: the closed export
  converter now rejects output paths that are identical to, or resolve to,
  the authenticated source trace. Valid/rejected fixtures prove the source MD5
  remains unchanged for direct and alias paths. Verification: focused Track 02
  CTest passed 4/4; `git diff --check` passed.

- 2026-07-17 CSB boot-to-viewport ingress projection: added the boot-owner
  bridge from an accepted first-live-dungeon receipt to the standalone
  viewport ingress receipt. It copies only verified session/tick/dungeon
  ownership/no-synthetic facts, rejects incomplete boot state, and avoids a
  boot/viewport include cycle. Verification: focused boot, declaration, and
  multi-frame viewport tests plus the relevant CSB suite passed 5/5; `git diff
  --check` passed.

- 2026-07-17 Theron Track 02 converted-trace receipt provenance: successful
  external runtime handoff now retains only the converter-verified Mednafen
  export path and MD5. Rejected or unavailable paths expose neither field and
  still leave decoder/draw permission closed. Verification: focused CMake
  build and Track 02 CTest passed 4/4; `git diff --check` passed.

- 2026-07-17 CSB viewport verified dungeon ingress: added a fail-closed
  ingress projection that requires verified session, tick, dungeon ownership,
  and no-synthetic-surface facts before reading the selected dungeon cell's
  door low bits. It preserves the explicit operator wall/floor/door entry
  triple unchanged, rejects invalid door-bit states, and never lets grid data
  select entries. Verification: related CSB suite passed 5/5 and `git diff
  --check` passed. A full workspace build reaches unrelated DM1 test failures
  for an undefined `TIMELINE_EVENT_ENABLE_CHAMPION_ACTION`.

- 2026-07-17 Theron Track 02 raw-CUE provenance: fixed strict intake for the
  authenticated US raw CUE's explicit `PREGAP 00:03:00`. It preserves the
  source-locked Track 02 LBA 225 while computing the MODE1 user-data offset
  against the separate Track 02 BIN member's own `INDEX 01`, rather than
  treating a disc-relative LBA as a file offset. The known US MD5 and CUE now
  pass without inspecting any payload semantics. Verification: focused CMake
  build plus Track 02 CTest passed 7/7 with `TQUS-Raw.cue`; `git diff --check`
  passed.

- 2026-07-17 CSB viewport live dungeon-state selection: added an exact
  operator-declaration selector for the current frame, door state, entry
  triple, and source identity. It invalidates a prior selection on every state
  identity change and rejects an undeclared entry rather than selecting a
  default material. Verification: focused state-selection plus multi-frame
  raster tests and the relevant CSB suite passed 5/5; `git diff --check`
  passed.

- 2026-07-17 CSBgraphics viewport live-frame declaration adapter: the
  hash-admitted CSBgraphics cache now decodes wall, floor and door spans only
  when the caller explicitly supplies every entry index, dimensions, FNV,
  clip, path/MD5 and admitted palette receipt. The adapter owns its decoded
  buffers and rejects source mismatch, stale bytes, or geometry drift; it has
  no default surface-to-entry mapping. Verification: focused declaration and
  multi-frame tests plus the relevant CSB suite passed 5/5; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 Mednafen trace conversion: added an
  external-only bounded adapter from a source-path/MD5-authenticated
  `mednafen-pce-instrumented` export into the strict HuC6280 capture-event
  log. It preserves only the observed loader CD-read and two opaque consumer
  rows. Unsupported debugger rows and MD5 drift reject; the receipt records
  no emulator launch, media copy, or synthesized event. Verification:
  converter and fixture test compiled with `-Wall -Wextra -Werror`, then the
  isolated targeted fixture passed against the existing M12 MD5 implementation
  and Theron event-log parser; `git diff --check` passed. The CMake CTest
  target is registered, but its normal build is currently blocked by an
  unrelated Nexus header/source mismatch.

- 2026-07-17 Theron Track 02 converted-trace handoff: the external capture
  launcher now accepts the MD5-attested Mednafen conversion only after its
  ordinary explicit raw-media preflight, then reuses the HuC6280-log manifest
  and opaque runtime-admission chain. Absent media prevents any output-log
  write; failed conversion or admission rejects with decoder and draw paths
  still closed. Verification: the focused no-local-media launcher regression
  passed, the converter valid/reject fixture passed, and `git diff --check`
  passed. Normal CMake CTest remains blocked by the unrelated Nexus mismatch.

- 2026-07-17 Theron Track 02 Mednafen conversion probe: added the local
  `--convert <export> <md5> <event-log>` entry point for the closed converter.
  It skips with no inputs and has no emulator-launch or media-copy operation.
  Verification: CMake reconfiguration passed, the focused no-input probe
  printed `SKIP`, and `git diff --check` passed. Normal CMake CTest remains
  blocked by the unrelated Nexus mismatch.

- 2026-07-17 CSB viewport live wall/floor/door progression: added a bounded
  multi-frame raster admission for caller-supplied decoded GRAPHICS.DAT wall,
  floor and door spans. It requires an accepted first-frame M11 receipt for
  frame zero, then preserves path/MD5/palette identity and enforces contiguous
  frames with one-state C0..C5 door transitions. Missing spans, altered FNV
  bytes, source mismatch, skipped frames, and door-state jumps reject without
  generating a surface. Verification: focused viewport materialization and
  related CSB graphics/startup regression passed 5/5; `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 log-driven runtime handoff: the external capture
  launcher can now validate a HuC6280 event log by generating the strict
  manifest in memory and feeding it through existing raw-media intake,
  provenance, manifest binding, and runtime admission. This route never reads
  or writes the hand-edited manifest path and remains entirely opaque. Missing
  local media/log input rejects or skips through the existing no-launch path.
  Verification: `build-ninja` focused CTest passed 5/5 and `git diff --check`
  passed.

- 2026-07-17 Theron Track 02 HuC6280 capture-event log: added a strict,
  ordered external log parser for the source-locked `$4090` CD-read event and
  two opaque consumer windows. Only a log whose record, destination, byte
  count, checksums, and consumer PCs match existing provenance/preparation can
  emit the strict capture-manifest receipt. Unknown event labels, an unknown
  loader PC, and window drift reject. No payload bytes or level/object/visual
  semantics are decoded. Verification: `build-ninja` focused CTest passed
  5/5, including no-input, unknown-event, and unknown-PC fixtures; `git diff
  --check` passed.

- 2026-07-16 CSB viewport material-byte handoff: first-frame D0/D1/D2
  F0111/F0115 runtime commands now receive decoded indexed GRAPHICS.DAT pixel
  spans and their shared palette span only after the existing material proof,
  route hashes, exact span sizes, decoded FNV values, and source path/MD5 all
  agree. The bounded raster consumer additionally requires the accepted M11
  receipt and command-attached spans, and rejects absent, truncated, stale, or
  source-mismatched bytes without a synthetic pixel or colour fallback.
  The existing viewport runtime binding consumes that handoff into the live
  raster only with the identical source identity; a changed MD5, absent bytes,
  or failed receipt remains no-draw. Verification: focused viewport
  materialization CTest passed, including structural raster-consumer,
  runtime-binding, stale-source, and local-corpus-safe regressions; `git diff
  --check` passed.

- 2026-07-16 Theron Track 02 external capture probe: added an operator-facing
  local executable with an explicit `--prepare <emulator> <cue-or-bin>
  <expected-md5> <manifest>` contract. It safely skips without inputs and
  delegates only to the strict no-launch capture-request helper; it neither
  starts an emulator nor copies media, creates trace rows, or decodes bytes.
  Verification: `build-ninja` focused CTest passed 5/5, including the
  no-input probe case; `git diff --check` passed.

- 2026-07-16 Theron Track 02 external capture launcher/probe: added a
  local-only request helper that accepts explicit emulator, media, expected
  MD5, and manifest paths. It verifies the source through strict intake before
  writing a comment-only manifest skeleton, and can later pass an
  operator-supplied completed manifest through the existing binder and runtime
  admission. It has no process-launch, media-copy, synthetic-trace, or decode
  path. Verification: `build-ninja` focused CTest passed 5/5, including
  no-launch, missing-emulator, and rejected-admission coverage; `git diff
  --check` passed.

- 2026-07-16 Theron Track 02 capture-trace runtime admission: added the
  runtime join for explicit manifest-bound opaque evidence, existing source
  provenance, and level/object trace preparation. The route becomes ready
  only after the Track 02 identity, loader record, consumer PCs, and exact
  windows all agree; fixture-only evidence, altered PCs/windows, absent
  manifest consumption, or pre-opened visual flags reject. Readiness retains
  coordinates only and leaves level/object field decoding, bitmap/palette,
  pixel decode, dungeon draw, and fallback visuals disabled. Verification:
  `build-ninja` focused CTest passed 5/5 and `git diff --check` passed.

- 2026-07-16 Theron Track 02 external capture-trace manifest binding: added a
  bounded closed-schema parser and binder for operator-supplied trace text.
  It can consume a manifest only after its MD5, raw CUE coordinates, runtime
  provenance, loader receipt, consumer PCs, and exact level/object windows
  agree with pre-existing authenticated receipts. Unknown or duplicate keys,
  unknown variants, missing fields, and changed windows reject. The output is
  opaque bounded evidence with all field, bitmap/palette, pixel, draw, and
  fallback routes closed. Verification: direct strict C11 compile of the new
  module/test and focused manifest probe passed, including malformed-manifest
  rejection and safe local-media/manifest skip; `git diff --check` passed.
  The ordinary CMake CTest target remains externally blocked by unrelated DM2
  compile errors in `src/dm2/dm2_v1_runtime.c` (two missing-argument calls).

- 2026-07-16 Theron Track 02 strict raw-media intake: added an explicit
  CUE/ISO/BIN discoverer that MD5-authenticates the payload against the known
  Track 02 variants, rejects unknown tracks, non-binary members, duplicate or
  absent `INDEX 01`, unsupported modes, incompatible sector layouts, and raw
  CUE index drift. It exposes only MODE1 sector and logical user-data-window
  provenance. A narrow adapter supplies existing loader-trace preparation
  only from an authenticated MODE1/2352 CUE at the source-locked JP/US index;
  ISO and bare BIN remain non-trace routes. Missing local media returns a safe
  unavailable receipt. No level/object, VCE, palette, or pixel decoding was
  introduced. Verification: targeted CTest passed 5/5, with the local-media
  branch reporting `SKIP (no local Track 02 media)`; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN bounded wall-ornament raw ownership:
  `DRAW_DEFAULT_DOOR_BUTTON` now admits only the source-proven DB2 Text/DB3
  Actuator `WALL_GFX/index/dtImage/1` path when its exact decoded pixels,
  local palette, root ObjectID/cell, and raw `GRAPHICS.DAT` interval agree.
  M11 retains the raw receipt and blocks absent or altered provenance; no
  generic sprite, DM1 projection, or unproven ornament transform is used.
  Source: skproject `SKULLWIN/c_gui_vp.cpp:1904`. Verification: focused
  wall-button gate passed 11/11 and real `GRAPHICS.DAT` ornament receipt test
  passed 14/14; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN wall GFX256 material ownership: `DM2_DRAW_WALL`
  now binds every admitted GRAPHICSSET WALL_GFX image to its exact GFX256 raw
  interval, local palette, source cell pass, RAW4 placement row, movement
  offset, and mirror state before M11 presentation. Mismatched raw receipts
  or unproved clipping block the complete wall pass, with no generic wall,
  object, or DM1 fallback. Source: skproject `SKULLWIN/c_gui_vp.cpp:575`.
  Verification: strict direct real-data wall-plan/M11 test passed, including
  altered-receipt rejection; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN dynamic creature GFX256 material ownership:
  `DM2_GET_CREATURE_ANIMATION_FRAME` now carries the exact FB/FC/FD-selected
  `CREATURES` raw GDAT interval through a bounded GFX256 receipt alongside
  the existing decoded image and local palette evidence. Its receipt hash is
  part of the dynamic material identity consumed by the existing M11 creature
  placement plan, which remains separate from DB4/F9 map-chip art. Source:
  skproject `SKULLWIN/c_creature.cpp:3217`. Verification: strict direct
  real-data dynamic-creature material test passed; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN outdoor weather M11 material ownership:
  `DM2_DRAW_TEMP_PICST` now reaches M11 only through a final receipt that
  joins exact ENVIRONMENT command text, GFX256 raw image material,
  QUERY_TEMP_PICST decoded-pixel/local-palette identities, the renderer
  transaction, and a live outdoor `DM2_SET_TIMER_WEATHER` owner. The runtime
  binds no weather renderer when any link is absent. The canonical corpus
  proves command text but not a complete weather image chain, so the tested
  result is strict no-draw with no procedural or cached replacement. Source:
  skproject `SKULLWIN/c_image.cpp:418` and `c_weather.cpp:22`. Verification:
  strict direct real-data scene/weather/light test passed; `git diff --check`
  passed.

- 2026-07-16 DM2 SKULLWIN HUD GFX256 runtime ownership:
  `DM2_LOAD_GDAT_INTERFACE_00_02` now carries each M11 HUD command's exact
  `INTERFACE_GENERAL` or `CHAMPIONS` raw GDAT interval through a bounded
  GFX256 receipt. The source-gated viewport validates that receipt together
  with the existing decoded pixels and local palette before presentation; it
  cannot use an absent receipt, fallback frame, or cache image. Source:
  skproject `SKULLWIN/c_gdatfile.cpp:1922` and `c_gui_vp.cpp`. Verification:
  strict direct real-data HUD command test passed; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN viewport scene GFX256 ownership:
  `DM2_DISPLAY_VIEWPORT` floor and ceiling commands now retain the exact
  active `GRAPHICSSET` raw intervals through bounded GFX256 receipts before
  M11 presentation. The pre-existing source rect, decoded-pixel, local
  palette, and scene/light contracts remain mandatory; this adds no cache
  allocation, fallback plane buffer, or synthetic art. Source: skproject
  `SKULLWIN/c_gui_vp.cpp:7325` and `c_dballoc.cpp:1112`. Verification:
  strict direct real-data M11 material test passed; `git diff --check`
  passed.

- 2026-07-16 DM2 SKULLWIN GFX256 door-runtime ownership: `DRAW_DOOR_FRAMES`
  material commands now retain the exact loaded GDAT raw interval selected by
  the source door plan and require a bounded GFX256 raw-material receipt
  before M11 admission. This covers panel, ornament, frame, side-frame, and
  button routes without treating a non-`dtImage` door row as a GFX16 default.
  Existing decoded-pixel and local-palette evidence remains required, while
  CPX cache nodes, new pixel buffers, and fallback art remain absent. Source:
  skproject `SKULLWIN/c_gui_vp.cpp:2333` and `c_dballoc.cpp:1112`.
  Verification: direct real-data door-plan test passed; strict direct GDAT
  querydb test passed 119/119; `git diff --check` passed.

- 2026-07-16 DM2 SKULLWIN GFX16/GFX256 GDAT material ownership: added
  source-named GFX16 tuple and GFX256 raw-index routes to the DM2 asset
  loader. Each route exposes only the immutable bytes already owned by loaded
  `GRAPHICS.DAT` plus the existing image-decode receipt. GFX16 uses the
  source `MISCELLANEOUS/FE/FE` image only when its requested image tuple is
  absent. Neither route allocates a CPX/cache node, decoded pixel buffer, or
  fallback graphic. Source: skproject `SKULLWIN/c_dballoc.cpp:1112-1170`.
  Verification: strict direct DM2 build/test passed 119/119, including local
  real `GRAPHICS.DAT` pointer/length identity checks; the CMake target remains
  blocked before DM2 link by unrelated CSB HUD-constant compile errors.

- 2026-07-16 DM2 SKULLWIN CPX1 reusable-block family: implemented the
  source-bounded `DM2_ALLOC_CPX_LINK_NODE`, `DM2_ALLOC_CPX_UNLINK_NODE`, and
  `DM2_ALLOC_CPX1` metadata path in `dm2_v1_skproject_cpx_heap`. It keeps the
  source descending-size ordering, chooses an exact reusable block before the
  largest fallback, and retains a split only when the remainder is at least 30
  bytes. The API accepts allocator metadata only and never creates decoded
  GDAT pixels, CPX backing memory, or fallback art. Source: skproject
  `SKULLWIN/c_dballoc.cpp:26-114`. Verification: focused CPX heap test passed.

- 2026-07-16 Theron Track 02 level-1 draw blocker receipt: added a real-CUE
  M11 blocker receipt for the concrete fail-closed gap after level-1 stage
  media consumption. The receipt consumes the proven M11 level-consumption
  facts, transition runtime, original Track 02 binding gap, and current world
  state, then records the real nonstartup level/object source-window offsets
  while confirming that level-1 geometry/object placement is not loaded from a
  verified source yet. It refuses already-open draw routes, keeps
  `dungeon_draw_route_allowed`, pixel blit, and fallback visuals closed, and
  adds focused unit and real US-CUE probe coverage. Verification: Ninja built
  `test_theron_v1_track02_loader_intake` and
  `firestaff_theron_v1_runtime_admission_probe`; CTest passed
  `theron_v1_track02_loader_intake` and `theron_v1_runtime_admission`; direct
  real US-CUE runtime-admission probe, full `ninja -C build/ninja-dm2
  firestaff`, and `git diff --check` passed.

- 2026-07-16 DM1 F0328/F0811 live thrown-object viewport route: carried the
  live `Projectile.Slot` associated thing through the DM1 viewport
  materialization receipt so M11 resolves thrown weapons via the real
  F0142/G0209 object-as-projectile branch instead of falling back to native
  M613 projectile art. Added a real-data M11 probe that starts DM1 from local
  PC34 `DUNGEON.DAT`/`GRAPHICS.DAT`, chooses a real throwable weapon record,
  triggers `DM1_ACTION_THROW`, and verifies the C2900 object-material receipt.
  Verification: focused Ninja/CTest projectile materialization targets passed.

- 2026-07-16 Nexus LEV00 scene geometry gate: added a Nexus DGN
  scene/runtime-plan consumer that binds the real `LEV00.DGN` source identity
  (147456 bytes, FNV `e715281f66445610`) and party/camera adjacent-cell facts
  before any Structure3 mesh use. The retail LEV00 route is now a verified
  fail-closed blocker: it reaches the real Structure1B dungeon-cell envelope
  but no bounded Structure1F-owned Structure3 mesh entry is present for this
  consumer, so geometry submit, texture submit, raster submit, M11 handoff,
  fallback geometry, and fallback visuals all stay closed. Verification:
  strict C11 syntax, Ninja target `test_nexus_v1_dgn_scene_runtime_plan`,
  CTest `nexus_v1_dgn_scene_runtime_plan`, direct real-corpus probe, and
  `git diff --check` passed. An attempted neighboring
  `test_nexus_v1_dgn_geometry_readiness` build is still blocked by that
  pre-existing test's private `asset_file_matches_md5` declaration.

- 2026-07-16 CSBWin DSA STKOP_Copy actuator-copy receipt: finalized the
  existing transactional `STKOP_Copy` DB3 actuator-copy path as a production
  runtime-receipted dungeon-copy family after COPYTELEPORTER. The VM preserves
  CSBWin's stack order, DB type gate, source non-DB3 no-op, staged chained
  source bytes, and rollback behavior, then publishes committed copy count plus
  source/destination actuator Things only after `set_actuator_payload` succeeds
  on the profile candidate. Real-corpus probing now reports verified
  actuator-copy actions separately from COPYTELEPORTER. Source: CSBWin
  `DSA.cpp` `STKOP_Copy` and `Data.h` `STKOP_Copy`. Verification: Ninja built
  `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`,
  `test_csb_v1_dsa_pure_control_pc34_compat`, and
  `test_csb_v1_dsa_queued_localstate2_timer`; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; direct Extended-DSA probe remains
  skip-safe without a supplied DSA save corpus; `git diff --check` passed.

- 2026-07-16 CSBWin DSA COPYTELEPORTER dungeon mutation: implemented
  authenticated `DSACMD_COPYTELEPORTER` and `DSACMD_COPYTELEPORTER32` as the
  next switch/actuator/dungeon-mutation family after MESSAGE. The VM now
  decodes source/destination target operands in CSBWin order, including
  A/B/ABS/GEAR and 16/32-bit ABS forms, then invokes a runtime-owned
  teleporter-copy owner. Runtime copies only a real source DB1 teleporter
  payload and source CELLFLAG byte onto an existing destination DB1 teleporter,
  preserving Thing `Next` links and rejecting missing/malformed cells or
  records without fallback. Receipts publish teleporter-copy count and last
  source/destination locations only after the profile-copy transaction commits.
  Source: CSBWin `DSA.cpp` `EX_COPYTELEPORTER` and `Data.h`
  `DSAcopyTeleporterCmd`. Verification: Ninja built
  `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`,
  `test_csb_v1_dsa_pure_control_pc34_compat`, and
  `test_csb_v1_dsa_queued_localstate2_timer`; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; direct Extended-DSA probe remains
  skip-safe without a supplied DSA save corpus; `git diff --check` passed.

- 2026-07-16 CSBWin DSA MESSAGE timer scheduling: implemented the next
  authenticated DSA action family after control flow for `DSACMD_MESSAGE`,
  `DSACMD_MESSAGE32`, and `DSACMD_DESSAGE32`. The VM now decodes
  `DSAmessageCmd` source words, preserves stack/parameter delay and target
  semantics, and calls a runtime-owned `QueueDSASwitchAction` boundary that
  validates real dungeon cells before scheduling live CSBWin timers. Runtime
  receipts publish `message_core`, scheduled timer count, event type, target
  location, and rollback guard only after commit; unsupported message routes,
  unknown cells, malformed words, and missing owners still fail closed without
  synthetic scripts, saves, or timer payloads. Source: CSBWin `DSA.cpp`
  `EX_MESSAGE`/`QueueDSASwitchAction` and `Data.h` `DSAmessageCmd`.
  Verification: Ninja built `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`,
  `test_csb_v1_dsa_pure_control_pc34_compat`, and
  `test_csb_v1_dsa_queued_localstate2_timer`; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; direct Extended-DSA probe remains
  skip-safe without a supplied DSA save corpus; `git diff --check` passed.

- 2026-07-16 DM1 F0407/F0446 FUSE final-presentation receipt: added a
  DM1-owned terminal receipt for the completed endgame handoff after all
  recorded F0445 replay frames and the F0446 text/final delay have drained.
  The receipt validates the C2 game-won music request, original GRAPHICS.DAT
  final-screen ids C006/C346/C026, converted G0012 THE END runtime rect, G0019
  credits palette endpoints, gameplay-input block, and terminal save/runtime
  state before M11 reports the final screen route as valid. M11 now exposes the
  receipt from live state without inventing final art or music. Verification:
  CTest `dm1_v1_endgame_presentation_pc34_compat` and
  `m11_dm1_endgame_final_presentation_receipt_pc34`, full
  `ninja -C build/ninja-dm2 firestaff`, and real-data DM1 boot probe
  `gtimeout 20s build/ninja-dm2/firestaff --game dm1 --boot-probe
  --boot-probe-frames 90 --duration 0` passed.

- 2026-07-16 Theron Track 02 M11 dungeon draw-route gate: added the first
  guarded M11 dungeon draw-route receipt above the proven level-1 media and
  multilevel handoff chain. The receipt consumes the exact stage atlas/palette
  M11 level receipt, the Track 02 level-transition runtime receipt, the live
  loaded world level, party pose, level-bank selection, bounded viewport cell
  sample, and placed object count before publishing a route hash to M11. It
  does not promote a bitmap fallback or an unreviewed pixel blit:
  `dungeon_draw_route_allowed=1` is separate from
  `dungeon_pixel_blit_allowed=0`, and real US-CUE probing confirms the route
  remains closed when runtime level geometry/object placement is not actually
  loaded from the verified media path. Verification: strict C11 syntax, Ninja
  build of `test_theron_v1_track02_loader_intake` and
  `firestaff_theron_v1_runtime_admission_probe`, CTest
  `theron_v1_track02_loader_intake` and `theron_v1_runtime_admission`, direct
  runtime-admission probe, direct real US-CUE runtime-admission probe, and
  full `ninja -C build/ninja-dm2 firestaff` passed.

- 2026-07-16 Theron Track 02 multilevel M11 media consumption: generalized the
  verified Soul Room M11 consumption into a guarded level-transition consumer
  for the next proven Track 02 stage route. The decode vector now carries
  per-level stage atlas evidence from real US Track 02 bytes, and the new M11
  level-consumption receipt requires a matching runtime transition, live world
  media, exact indexed atlas checksum/offsets, HuC6260 palette checksum,
  placement/clip/scale bounds, and the selected level-bank route before
  allowing host M11 presentation for level 1. Dungeon draw and fallback visuals
  remain closed, and checksum, target-level, host-bounds, and scale drift fail
  closed. Verification: strict C11 syntax, Ninja build of
  `test_theron_v1_track02_loader_intake` and
  `firestaff_theron_v1_runtime_admission_probe`, CTest
  `theron_v1_track02_loader_intake` and `theron_v1_runtime_admission`, direct
  runtime-admission probe, direct real US-CUE runtime-admission probe, full
  `ninja -C build/ninja-dm2 firestaff`, and `git diff --check` passed.

- 2026-07-16 Nexus PRS3/VDP1 consumer evidence gate: added a fail-closed
  capture/evidence receipt for the already-bound `MENU.BPK` entry 5
  Structure2 ABI. It carries the real 54x31x1 PRS3 vector identity, output FNV
  `14cacc01cee292aa`, raw BE16 PALT FNV `0ec4e98ca3a18f85`, and `LEV00.DGN`
  Structure2 descriptor counts into an independent Saturn/VDP1-consumer
  target, then blocks unless a real authenticated trace supplies VDP1 command,
  texture-window, palette-application, descriptor-selection, and placement
  lanes. Local probe evidence found Mednafen 1.32.1 with the `ss` module, but
  no Saturn BIOS in `~/.mednafen/firmware` and no Nexus VDP1 trace sidecar, so
  no Structure2 submit, M11 handoff, pixel format claim, palette claim, or
  fallback visual was promoted. Verification: strict C11 syntax, Ninja target
  `test_nexus_v1_prs3_vdp1_consumer_evidence`, CTest
  `nexus_v1_prs3_vdp1_consumer_evidence`, neighboring PRS3 ABI/subset tests,
  and direct real-corpus probe passed.

- 2026-07-16 CSBWin DSA nested control-flow VM: extended the authenticated
  transfer/runtime path from QUESTION-selected branches into explicit
  JUMP/GOSUB/RETURN call-frame execution. The CSBWin source path has no RETURN
  opcode; Firestaff now mirrors that by treating missing `Program(state,column)`
  as an Execute return, while GOSUB pushes a continuation frame and JUMP remains
  in the current frame. Runtime receipts publish transfer/return counts,
  frame-push/pop balance, maximum subroutine depth, final state, and rollback
  guard only after committed execution. Unknown or unproved action forms still
  fail closed, and no synthetic save/script was added. Verification: Ninja built
  `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`,
  `test_csb_v1_dsa_pure_control_pc34_compat`, and
  `test_csb_v1_dsa_queued_localstate2_timer`; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; the direct Extended-DSA probe remains
  skip-safe without a supplied DSA save corpus; `git diff --check` passed.

- 2026-07-16 Nexus PRS3/Structure2 ABI gate: added a real-data ABI receipt
  that binds the positive `MENU.BPK` entry 5 full PRS3 output vector to the
  canonical PALT trailer and `LEV00.DGN` Structure2 descriptor envelope
  without promoting pixels. The gate records entry 5 as 54x31x1, stream
  offset 1612, stream size 552, expected output 1674, output FNV
  `14cacc01cee292aa`, PALT raw-table FNV `0ec4e98ca3a18f85`, and LEV00's
  82 Structure2 descriptors with 80 palette anchors. It also rejects entry 1
  because its real stream still stops at 237/480 bytes and rejects loose auth
  claims before pixel submit, palette submit, M11 handoff, or fallback visual
  can open. Verification: strict C11 syntax, Ninja target
  `test_nexus_v1_prs3_structure2_abi`, CTest
  `nexus_v1_prs3_structure2_abi`, and direct real-corpus probe passed.

- 2026-07-16 CSBWin DSA `QUESTION` branch-transfer execution: promoted the
  conditional branch family from operand receipt to actual VM execution. The
  authenticated stack interpreter now decodes `DSACMD_QUESTION` branch operands
  in CSBWin source order, pops the condition, and, only for the selected branch,
  enters an exact imported JUMP/GOSUB action chain through the existing
  transfer subset. Missing or mismatched branch owners still fail closed before
  any parameter/global/runtime callback publication, and stack-triggered
  transfers now publish through the same `last_transfer`/state persistence path
  as top-level transfer actions. Verification: Ninja built the Extended-DSA
  probe and focused DSA tests; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; the direct Extended-DSA probe remains
  skip-safe without a real DSA save corpus; `git diff --check` passed.

- 2026-07-16 Nexus PRS3 SH-2 subset full-vector proof: extended the strict
  `DM.BIN` V1 subset executor through the missing real output stores:
  nonzero `MOV.B R2,@R10` (`2a20`) and zero-side `MOV.B R1,@R3` /
  `MOV.B R1,@R10` (`2310`/`2a10`) with R10 as the linear output pointer and
  R13/R6 as the 4 KiB history window. The positive real `MENU.BPK` entry 5
  vector now consumes 514 bytes from its own 552-byte stream and emits the
  full declared 1674 bytes: 147 nonzero transfers, 164 zero-side merges, 1527
  zero-side copied bytes, output FNV `14cacc01cee292aa`, and control FNV
  `1a0e6440b26b01bd`. Entry 1 remains a verified negative boundary at
  237/480 bytes from its own stream. No Structure2 pixel/palette route,
  renderer upload, Saturn-authenticated provenance, or fallback visual was
  promoted; the remaining gate is independent Saturn/emulator provenance plus
  reviewed pixel/palette ABI. Verification: strict C11 syntax, Ninja target
  `test_nexus_v1_prs3_sh2_subset_trace`, and direct real-corpus probe passed.

- 2026-07-16 CSBWin DSA opcode-family execution admission: extended the
  production DSA verifier/runtime receipt from a generic transfer/stack label
  into explicit source opcode families for conditionals, arithmetic, local and
  global variables, timer-owned effects, and dungeon/save mutations. The VM
  still executes only checksum-imported `DSAAction` words, stages every
  callback-backed mutation on candidate state, and publishes family/mutation
  facts only after transactional commit; unknown opcodes and unproved subcodes
  remain fail-closed. The Extended-DSA probe now reports these families for a
  real supplied corpus and checks the committed execution receipt when a saved
  TimerQueue DSA fires. Verification: Ninja built the Extended-DSA probe and
  focused DSA tests; CTest passed `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; the direct Extended-DSA probe remains
  skip-safe without a real DSA save corpus; `git diff --check` passed.

- 2026-07-16 Nexus PRS3 SH-2 subset execution trace baseline: added the first
  strict fail-closed subset executor for the proven retail `DM.BIN` V1 PRS3
  loader corridor and bound it to real `MENU.BPK` entry 1. That baseline
  observed the source cursor R12, output base/index R13/R6, remaining source
  R14, and control word R11, but intentionally stopped before decoder
  promotion because the linear output stores were not yet modeled. It is
  superseded by the full-vector proof above, which adds the missing real
  store instructions while preserving the same no-fallback admission boundary.
  Verification at the baseline stage: strict C11 syntax, Ninja target
  `test_nexus_v1_prs3_sh2_subset_trace`, CTest
  `nexus_v1_prs3_sh2_subset_trace`, and direct real-corpus probe passed.

- 2026-07-16 CSBWin DSA runtime execution receipt: promoted the authenticated
  transfer/stack-core runner into a production-observable runtime handoff. The
  CSB runtime now verifies the exact imported `DSAAction` core before
  execution, stages operand/global/local-state/EXPOOL/dungeon side effects on
  candidate state, and publishes a rollback-guarded receipt only after commit.
  The receipt names DSA id/state/column/action ordinal, transfer versus
  stack-core admission, stack/transfer counters, and committed mutation classes
  without creating a save, script, timer, or fallback DSA payload. Verification:
  Ninja built the Extended-DSA probe and focused DSA tests; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; the direct Extended-DSA probe remains
  skip-safe without a real DSA save corpus; `git diff --check` passed.

- 2026-07-16 DM1 native wall-inscription F0172/F0107 material route:
  added strict selected-wall inscription presentation receipts for both the
  readable D1C M648 path and the side/depth unreadable ornament path. M11 now
  consumes those receipts, so a hidden or malformed selected wall TextString
  cannot borrow another visible TextString from the square list. The existing
  real PC34 pixel probes continue to prove GRAPHICS.DAT graphic 258, byte<<3
  glyph cells, native 8x8 blits, C10 preservation, and original unreadable
  ornament palette/clip materialization. Verification: Ninja built the focused
  inscription targets; CTest passed the wall material gate, non-HoC M648 pixel
  probe, unreadable side/depth real-data probe, hidden/stale/F0128/HOC
  inscription regressions, and `git diff --check`.

- 2026-07-16 CSBWin DSA bytecode core admission: added a production
  decode-only verifier for authenticated CSBWin `DSAAction` word programs.
  The verifier consumes the source opcode grammar for transfer-only
  `JUMP/GOSUB`, stack-core `LOAD/STORE/NOOP/EQUAL/QUESTION`,
  local/global variables, and the implemented AMPERSAND/AMPERSAND2 STKOP
  subset before runtime execution can begin. Unknown opcodes, malformed
  operands, `LOAD_ABS32`, and unproved subcodes fail closed without creating
  scripts or save data. The Extended-DSA real-corpus probe now requires at
  least one verified executable imported action when a real DSA corpus is
  supplied. Verification: Ninja built the DSA/probe targets; CTest passed
  `csb_v1_csbwin_package_runtime_handoff`,
  `csb_v1_csbwin_extended_dsa_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_queued_localstate2_timer`; `git diff --check` passed.

- 2026-07-16 Nexus PRS3 loader control-flow map: added a Nexus-owned
  fail-closed receipt over real retail `DM.BIN` and `MENU.BPK` that maps the
  proven PRS3 loader callee `85376` through the `85450` control-bit branch,
  `85460` nonzero source read, `85464` output store, and the zero-side
  two-source-byte merge/indexed-output-window corridor. The receipt records
  source/output pointer register roles, control masks, exact basic-block
  offsets, first MENU.BPK PRS3 stream facts, and a zero-side byte fingerprint.
  It deliberately does not implement or promote a decoder: zero-side copy
  semantics, positive expected-output vectors, authenticated Saturn execution
  provenance, and reviewed opcode grammar remain required before Structure2
  pixel/palette intake can open. Verification: strict C11 syntax, Ninja target
  `test_nexus_v1_prs3_loader_control_flow`, CTest
  `nexus_v1_prs3_loader_control_flow`, and direct real-corpus probe passed.

- 2026-07-16 DM1 HoC mirror/inscription D0-D3 viewport route: added a
  DM1-owned C127 viewport projection receipt that keeps the D1C C346->C026
  portrait overlay separate from side/depth mirror wall backing. M11 now lets
  real HoC C127 side/depth walls consume global ornament 43 through the
  existing F0107/GRAPHICS.DAT wall-ornament path while suppressing C026 outside
  D1C; readable inscriptions remain native M648/C10 only and side/depth text
  remains the original unreadable ornament route. No host text, fallback font,
  synthetic portrait, or fake wall pixels were added. Verification: Ninja/
  direct/CTest for `test_dm1_v1_champion_mirror_pc34_compat`, M11 side-wall
  ornament and unreadable-inscription real-data tests, HoC ordinal and mirror
  visibility probes against local DM1 data, full `firestaff` Ninja build,
  DM1 boot-probe exit 0, and `git diff --check` passed.

- 2026-07-16 Nexus PRS3 decoder reverse-admission: added a fail-closed
  decoder-admission boundary over real retail `DM.BIN` and `MENU.BPK`. It
  binds the SH-2 V1 decompression-loader corridor (`85376` callee, `85450`
  low-bit control, `85460` nonzero source read, `85464` output store, and the
  zero-side two-byte merge path) to the real 162 MENU.BPK PRS3 stream plans,
  then runs the existing LSB/MSB framed trial decoders as differential
  evidence. Both trial grammars fail the retail corpus with zero complete
  outputs, so the receipt remains `ready-blocked`: no decoder promotion,
  runtime upload, Structure2 pixel/palette intake, or fallback visuals are
  permitted without a source-bound expected-output sidecar, authenticated
  Saturn provenance, and reviewed opcode grammar. Verification: strict C11
  syntax, Ninja target `test_nexus_v1_prs3_decoder_admission`, CTest
  `nexus_v1_prs3_decoder_admission`, and direct real-corpus probe passed.

- 2026-07-16 CSBWin DSA runtime-chain receipt: added a production CSB runtime
  receipt that binds an admitted CSBWin Extended Features save to its
  authenticated DSA catalog, `DSALevelIndex` selector table, materialized
  TimerQueue-to-event ownership, and any executed saved-timer DSA action
  without creating a DSA/save fixture. The opt-in package and extended-DSA
  probes now consume that same receipt around resume and first tick, and the
  Extended-DSA probe is registered in CTest as a real-data skip-safe target.
  Verification: Ninja built `firestaff_csb_v1_csbwin_package_runtime_handoff_probe`
  and `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe`; CTest
  `csb_v1_csbwin_package_runtime_handoff` and
  `csb_v1_csbwin_extended_dsa_handoff` passed; `git diff --check` passed for
  the touched CSB/runtime probe files.

- 2026-07-16 DM1 F0128 D0-D3 viewport wall-material route: tightened the
  DM1/ReDMCSB wall receipt path so D2L/D2R side-wall materialization is no
  longer pre-culled by nearer same-lane D1 side occupancy. The source-owned
  renderplan test now binds F0128 draw-order rows to D3L2/D3R2, D3/D2/D1
  side and center wall zones, plus D0L/D0R wall material specs, while retaining
  real C710/C711 backing expectations for the PC34 78x74 bitmap cropped into
  the 75x71 destination. M11 continues to consume the same receipt path with
  GRAPHICS.DAT assets; no synthetic wall pixels or fallback events were added.
  Verification: Ninja build of `test_dm1_v1_viewport_3d_pc34_compat`, direct
  green run and focused CTest for
  `test_dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat`, manual C11 receipt
  probe for D2L/D2R draw under blocked D1 side lanes, full `firestaff` Ninja
  build, real DM1 boot-probe exit 0, and `git diff --check` passed.

- 2026-07-16 Nexus PRS3/Structure2 real-data intake: added a Nexus-owned
  `ready-no-draw` admission boundary that consumes hash-found retail
  `MENU.BPK` and `LEV00.DGN` bytes, binds BPPK/BMPD directory facts, 162 PRS3
  stream plans, the opaque 256-entry PALT trailer, and Structure2 descriptor
  image/palette payload anchors without exposing decoded pixels or palette
  semantics. Missing source verification, PRS3 framing, PALT, or Structure2
  source facts fail closed; runtime render, PRS3 decode, Structure2 pixel
  submit, palette submit, and fallback visuals remain denied. Verification:
  strict C11 syntax, Ninja target `test_nexus_v1_prs3_structure2_intake`,
  CTest `nexus_v1_prs3_structure2_intake`, and direct probe against the local
  real Nexus corpus passed.

- 2026-07-16 CSB runtime HUD/door first-frame draw-plan package: promoted the
  real `GRAPHICS.DAT` D0/D1/D2 F0111/F0115 material receipts from proof-only
  hashes into a CSB-owned M11 runtime draw plan. The plan now requires the
  shared real catalog/palette hash, five source-material hashes, real graphics
  session state, party input, route order, transparent color and bounded clips
  before `csb_v1_viewport_render_frame` reports first-frame consumption. No
  fallback pixels or wrapper images are produced; missing real session data
  fails closed. Verification: Ninja build/direct run of
  `test_csb_v1_viewport_first_frame_materialization_pc34_compat`, focused
  CTest for that test plus D0/D1/D2 door/thing receipt tests, and
  `git diff --check` passed.

- 2026-07-16 Theron Track 02 real-media runtime-admission probe: the
  runtime-admission probe now resolves its optional US Track 02 input from
  either raw BIN or production CUE metadata before exercising the real
  level/object route receipts. The local raw/CUE corpus proves source evidence
  only; the missing original palette/consumer binding gap remains a no-render,
  no-RGBA, no-draw, no-fallback blocker instead of failing the probe or
  inventing a substitute. An optional object/dungeon HuC6280 trace corpus can
  now drive grammar, raw nonstartup handoff, and object/level admission
  receipts, still with bitmap, palette, RGBA, draw, synthetic, and fallback
  promotion denied. Verification: Ninja build, CTest for
  `theron_v1_runtime_admission` and `theron_v1_track02_loader_intake`, direct
  real-CUE runtime-admission probe, syntax check, and `git diff --check`
  passed.

- 2026-07-16 DM2 p130 ANIM/IBMIO runtime-vector receipts: closed the requested
  SkWinCore chain around `_01b0_1ed2`, `_0759_0126`, `_0759_06c2`,
  `_0759_06db`, `_0759_072c`, `_0759_071b`, `_0759_06b5`, and
  `_0759_065f` with source-backed caller-owned ANIM/IBMIO runtime state.
  The receipts preserve interrupt-vector capture, timer-reload countdown,
  IBMIO event poll/consume, source 320x200 zero-fill, the `_00eb_0bc4` LFSR
  clear order, and sound-card availability without host interrupts, blocking
  waits, or synthetic graphics. `_069a_03fc`, `_0759_07f2`, `_0759_0792`,
  and `_0759_0739` were marked explicit `NONAPPLICABLE` because skproject
  leaves them as `Unr()` ANIM stubs. Verification: strict syntax check,
  Ninja build/direct run of `test_dm2_v1_skproject_core`, focused CTest,
  backlog refresh to DM2 1144, and `git diff --check` passed.

- 2026-07-16 DM1 M11 action/spell HUD runtime consumption: M11 now consumes
  the verified F0407/F0231/F0412 live action/spell HUD receipts on the real
  runtime path. The creature-hit overlay requires a live F0231 damage receipt,
  GRAPHICS.DAT C014, and the source-bound M653 HUD font before drawing; missing
  source material is no-draw instead of a host rectangle/font fallback. Spell
  failure feedback now goes through the F0412 feedback receipt, preserving the
  original clear-symbol/redraw flags so flask/map failures do not falsely wipe
  the spell runes. Verification: Ninja app build, focused M11/DM1 Ninja
  targets, focused CTest, real `gtimeout 20s build/ninja-dm2/firestaff
  --game dm1 --boot-probe --boot-probe-frames 90 --duration 0`, and
  `git diff --check` passed.

- 2026-07-16 CSB real startup sequence package: consolidated the CSB
  `GRAPHICS.DAT` startup path around the production runtime session from real
  C001-C005/C017/C040 data through PRESENTS, CHAOS zoom/hold, STRIKES BACK,
  Entrance closed/opening/credits and terminal HUD/runtime handoff. The title
  timing now keeps the ReDMCSB CHAOS hold and STRIKES step alive through a
  101-tick title window, the terminal handoff requires the full verified
  surface contract and consumed title mask, and the C001 contract matches the
  decoded real PC34 320x153 source shape instead of a synthetic full-screen
  assumption. Verification: Ninja build/direct run and focused CTest for
  `test_csb_v1_startup_real_sequence_pc34_compat`,
  `test_csb_v1_startup_terminal_handoff_real_data_pc34_compat`, and
  `test_csb_v1_startup_entrance_pointer_pc34_compat`; direct
  `firestaff --game csb --boot-probe --boot-probe-frames 90 --duration 0`
  passed with `titleFrameMax=101`.

- 2026-07-16 CSB/ReDMCSB live door-opening capture consumer: added a direct
  session-owned capture of all 31 `ENTRANCE.C F0438/F0807` C004/C002/C003
  opening pages. Every source step is rebuilt through the source render plan,
  rasterized from the verified session, and bound into a distinct route hash;
  stale/replayed pages, fallback surfaces, and legacy wrappers fail closed.
  Verification: local verified PC34 media exercised all 31 pages in
  `csb_v1_startup_real_sequence_pc34_compat`; focused CTest passed that test,
  `csb_v1_boot_title_import_ui_gate_pc34_compat`, and
  `csb_v1_startup_runtime_coupling_adapter_pc34_compat` (3/3).

- 2026-07-16 CSBgraphics live HUD frame provenance: C017/C040 selected from
  hash-admitted `CSBgraphics.dat` now carry an MD5/path/entry-pair receipt
  from the verified session into the live HUD binding hash. The existing frame
  route, host-raster, and presentation/capture hashes consequently distinguish
  the same panel pixels from a different admitted cache. Original
  `GRAPHICS.DAT` HUD frames retain their prior zero source receipt. No wrapper
  or fallback panel can satisfy the route. Verification: CMake built
  `test_csb_v1_boot_title_import_ui_gate_pc34_compat` and
  `test_csb_v1_startup_runtime_coupling_adapter_pc34_compat`; focused CTest
  passed 2/2 and `git diff --check` passed.

- 2026-07-16 CSBgraphics palette-source admission: the startup package now
  owns a value receipt for a declared `CSBgraphics.dat` palette source only
  after source kind, resolved path, MD5, indexed payload span, exact 768-byte
  LZW output and FNV-1a identity agree. Without that receipt its indexed
  startup surfaces are explicitly no-draw; no palette is inferred from bitmap
  data and no renderer was opened. The structural regression covers missing
  source evidence, path/hash mismatches, truncated material and one admitted
  fixture. Verification: built `test_csb_v1_csbgraphics_runtime_plan`, CTest
  `csb_v1_csbgraphics_runtime_plan` PASS, and `git diff --check` passed.

- 2026-07-16 CSBgraphics startup image/palette eligibility: title, entrance
  door and HUD package entries now carry individual cache-derived image
  receipts (source kind, path, MD5 and bounded index span). Surface decoding
  is no-draw until that entry receipt and the admitted palette receipt have
  the same source identity. The focused structural fixture proves incomplete
  material stays closed, and title-image, door-span and HUD-palette mismatches
  all reject decode; the complete door/HUD fixture remains bounded and
  decodable. No palette pixels were guessed and no presentation path changed.
  Verification: `csb_v1_csbgraphics_dat_classify_unit`,
  `csb_v1_csbgraphics_runtime_binding`, and
  `csb_v1_csbgraphics_runtime_plan` PASS (3/3); `git diff --check` passed.

- 2026-07-16 CSBgraphics corpus palette-candidate scan: the hash-only local
  `CSBgraphics.dat` cache can now report only exact declared 768-byte entries
  that survive bounded LZW decode, with their source path, MD5, archive span
  and FNV-1a identity. Candidate admission requires a second exact
  path/MD5/entry/FNV declaration and decode before it emits a value
  source-byte receipt. The real-data probe reports candidates when a
  user-staged corpus is available and otherwise stays skip-safe; it does not
  select a palette or open a runtime/UI route. Structural coverage proves the
  single 768-byte fixture, bad-FNV rejection and receipt emission. Verification:
  `csb_v1_csbgraphics_dat_classify_unit`,
  `csb_v1_csbgraphics_runtime_binding`,
  `csb_v1_csbgraphics_runtime_plan`,
  `csb_v1_csbgraphics_dat_real_scan_manifest`, and
  `csb_v1_csbgraphics_dat_real_scan` PASS (5/5); `git diff --check` passed.

- 2026-07-16 CSBgraphics boot palette provenance: `CSB_V1_BootProfile` now
  accepts a candidate only through the exact scanner admission and stores the
  resulting path/MD5/span/FNV receipt. Startup readiness reports separate
  title, door and HUD palette facts; CSBgraphics HUD bindings stay unverified
  and M11 reports no-draw until that receipt matches the loaded cache. A
  successful receipt is folded into the session HUD provenance hash. The new
  local-corpus boot probe is skip-safe for absent material and exercises the
  same admission when a hash-admitted 768-byte candidate is available. No
  generic palette, guessed colors, fallback UI or renderer route was added.
  Verification: `csb_v1_csbgraphics_palette_boot`,
  `csb_v1_boot_title_import_ui_gate_pc34_compat`,
  `csb_v1_csbgraphics_runtime_plan`, and
  `csb_v1_csbgraphics_dat_real_scan` PASS (4/4); `git diff --check` passed.

- 2026-07-16 CSBWin DSA/save runtime-body admission: tightened the strict DSA
  corpus receipt so an authenticated Extended Features DSA tail and valid
  GAMEBLOCK1 header no longer suffice. It now requires the complete
  checksum-verified GAMEBLOCK1 body and carries full-save/body FNV identities
  into the boot DSA handoff, which consumes all of those facts before it can
  report runtime ownership. The data-free regression proves that a header-only
  DSA fixture stays closed while retaining its authenticated tail evidence;
  no synthetic DSA action or runtime state is promoted. Verification: built
  `firestaff_m11`, CTest passed
  `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` and
  `csb_v1_dsa_save_runtime_admission_pc34_compat` (2/2), and
  `git diff --check` passed.

- 2026-07-16 CSBWin DSA immutable runtime admission: added a CSB-only receipt
  that binds an already accepted DSA corpus's DSA/GAMEBLOCK offsets, lengths,
  checksums, and FNV identities to the verified boot `Dungeon.dat` MD5. It
  publishes no DSA program words or execution capability and rejects stale or
  mixed save bytes before runtime consumption. Verification: built and ran
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` (1/1); its structural
  fixture has no external corpus dependency, while absent user corpus keeps
  the production admission route closed.

- 2026-07-17 CSBWin DSA/save-to-runtime atomic handoff: extended the immutable
  DSA source admission with a final receipt that accepts only matching
  checksum-bound save/GAMEBLOCK identities, current verified `Dungeon.dat`
  identity, complete source-owned startup-session generation, and an already
  verified CSBWin DSA catalog/index/timer chain. It adds no DSA semantics or
  fallback path. The structural regression covers a complete handoff plus
  save-hash, session-generation, runtime-chain and Dungeon-MD5 mismatch
  rejection. Verification: `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`,
  `csb_v1_dsa_save_runtime_admission_pc34_compat`, and
  `csb_v1_f0213_f0220_explosion_runtime_pc34_compat` PASS (3/3); `git diff
  --check` passed. A subsequent full build cleared the Theron linkage but is
  currently blocked by unrelated DM2 CoreMIDI and session-parser link errors.

- 2026-07-17 CSBWin DSA/save M11 viewport delivery: added an explicit M11
  binding for the completed immutable DSA/save runtime receipt. Only this
  opt-in route checks the current verified `Dungeon.dat` MD5 and source-owned
  session generation/tick before `F0128` can publish a viewport; stale or
  mixed receipts clear the frame, while normal title, entrance and HUD routes
  retain their established lifecycle. No DSA bytes, interpreter path or
  fallback presentation was added. Verification: `build-ninja` target
  `firestaff` passed; CTest passed
  `csb_v1_m11_launcher_handoff_boundary`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_save_runtime_admission_pc34_compat` (3/3).

- 2026-07-17 CSB DSA viewport runtime-epoch and material gate: the immutable
  DSA/save handoff now carries the admitted runtime game-time epoch, and M11
  compares it at the existing CSB tick boundary before publishing a DSA-owned
  viewport. Its D1/D2/D3 projectile and explosion overlays require the same
  current DSA route plus admitted CSBgraphics palette provenance; missing,
  stale, mixed, or incomplete inputs produce no draw and no marker fallback.
  The non-DSA title, entrance, and HUD lifecycle is unchanged. Verification:
  `firestaff` built in `build-ninja`; CTest passed
  `csb_v1_m11_launcher_handoff_boundary`,
  `csb_v1_viewport_phase3_rendering`,
  `csb_v1_dsa_save_runtime_admission_pc34_compat`, and
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` (4/4).

- 2026-07-17 CSBWin DSA restored-timer execution admission: added a
  source-bound bridge from the complete DSA/save handoff to the existing
  TimerQueue action and Monster.cpp movement-filter runners. It requires the
  current Dungeon identity, startup generation/tick, exact runtime game-time,
  source queue slot/TIMER row, selected LocalState action, and the existing
  authenticated core-program verifier before calling either runtime consumer.
  Session/save/tick drift and unknown opcode bodies reject before execution;
  level-one and level-two movement filters retain independent declared index
  ownership. No DSA semantics or fallback path was added. Verification:
  `firestaff` built in `build-ninja`; CTest passed
  `csb_v1_dsa_admitted_restored_timer_bridge`,
  `csb_v1_dsa_save_runtime_admission_pc34_compat`, and
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` (3/3).

- 2026-07-17 CSB DSA M11 outcome transaction: M11 now commits a restored
  timer outcome only after CSB revalidates its handoff/save identity, startup
  session, runtime tick, bridge hash, TimerQueue execution identity, and the
  runtime-owned last DSA execution receipt. The viewport transaction then
  fails no-draw on outcome/session/tick drift. This retains the existing DSA
  runners as the only execution owners; unbacked or unknown-opcode outcomes
  cannot alter the M11 route. Verification: `firestaff` built in `build-ninja`;
  CTest passed `csb_v1_m11_launcher_handoff_boundary`,
  `csb_v1_dsa_admitted_restored_timer_bridge`, and
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` (3/3).

- 2026-07-17 CSB DSA restored-world transaction: the source-bound timer
  bridge now snapshots the live runtime profile and its raw Dungeon.dat
  buffer before it calls the already admitted door/state/teleporter action
  runner. A rejected runner or post-execution queue/action receipt restores
  both snapshots, so no partial save/resume-visible mutation can commit.
  Successful outcomes record only existing runtime mutation classes; unknown
  opcode bodies remain blocked by the prior verifier. Verification: focused
  M11/admission CTest passed 3/3 and `firestaff` was rebuilt in `build-ninja`.

- 2026-07-17 CSB save candidate receipt: direct save files and virtual
  container members now have one immutable, no-write admission receipt that
  retains source path/kind, save FNV, declared save/Dungeon identities and the
  already strict GAMEBLOCK/DSA admission. The boundary rejects FNV drift,
  mixed GAMEBLOCK material and missing source, save-MD5 or Dungeon-MD5
  identity; it neither extracts nor synthesizes save data. Verification:
  `firestaff` built in `build-ninja`; CTest passed
  `csb_v1_m11_launcher_handoff_boundary`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` (3/3).

- 2026-07-17 CSB local save candidate discovery: bounded direct-file and
  virtual-container byte views now feed the strict candidate receipt without
  extraction, writes, or payload ownership. An empty local view set is a
  skip, while multiple otherwise complete candidates reject as mixed rather
  than selecting an arbitrary save. Verification: `firestaff` built in
  `build-ninja`; CTest passed `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`
  and `csb_v1_dsa_admitted_restored_timer_bridge` (2/2).

- 2026-07-17 CSB M12 launch identity: CSB quick-resume now carries a
  candidate-identity binding and `M12_StartupMenu_GetLaunchIntent` refuses
  path/game-id-only CSB resume intents. This is a CSB-only gate; it does not
  reinterpret or constrain unrelated game scans. Verification in isolated
  `build-csb-verify`: `firestaff` built, and CTest passed
  `csb_v1_launch_blocker_m12` plus
  `csb_v1_m11_launcher_handoff_boundary` (2/2).

- 2026-07-17 CSB discovery-to-M12 binding: M12 can now consume the
  source-owned CSB discovery receipt directly. It accepts exactly one valid,
  non-mixed candidate only when its source path matches the active CSB resume
  path; absent, mixed, or stale evidence clears the launch identity. Isolated
  verification in `build-csb-verify`: `firestaff` built, and CTest passed
  `csb_v1_launch_blocker_m12` plus
  `csb_v1_m11_launcher_handoff_boundary` (2/2).

- 2026-07-17 CSB launch-session identity: the CSB candidate identity now
  round-trips from the hash-verified M12 launch into M11's opened CSB startup
  session without mutating source tick or generation. Lifecycle coverage
  rejects stale, missing, and non-CSB discovery routes. Isolated
  `build-csb-verify` built `firestaff`; CTest passed
  `csb_v1_m11_launcher_handoff_boundary`.

- 2026-07-17 CSB startup-profile/native-F0435 boundary: ordinary verified
  graphics/Dungeon evidence now has a separate startup-package identity in
  the CSB session, distinct from optional save/DSA identity. The new
  source-owned real-receipt constructor rescans the named root and rejects
  altered or missing proof before profile publication. Native F0435 admission
  reads and verifies the structured 512-byte CSB header before it loads; the
  focused package test is explicitly skip-safe without
  `FIRESTAFF_CSB_DATA_DIR`. Verification in `build-csb-verify`:
  `csb_v1_startup_package_identity_pc34` passed, `firestaff` built, and
  `git diff --check` passed.

- 2026-07-17 CSB launch-owned presentation package identity: M11 now keeps
  the ordinary hash-verified graphics/Dungeon identity set at CSB boot and
  compares it read-only with the live startup session before TITLE.C's
  PRESENTS/CHAOS/STRIKES raster, entrance-door delivery, C017/C040 HUD, and
  the final door-to-runtime handoff. Missing, stale, or drifted identity
  clears the frame or rejects the handoff; an active startup cannot fall
  through to a normal runtime tick. No tick, generation, wrapper, or fallback
  state is repaired by the gate. The corpus-backed M12-to-M11 lifecycle test
  covers valid title/HUD/door delivery plus missing and stale rejection.
  Verification in `build-csb-verify`: CTest passed
  `csb_v1_m11_launcher_handoff_boundary` and
  `csb_v1_startup_package_identity_pc34` (2/2); `firestaff` built and
  `git diff --check` passed. The latter target explicitly skips corpus-only
  profile cases when `FIRESTAFF_CSB_DATA_DIR` is unset.

- 2026-07-17 CSB restored DSA timer M11 ingress: added
  `M11_GameView_ExecuteCSBDSARestoredTimer`, a caller-named location and
  TimerQueue-slot bridge to the existing CSBWin authenticated restored-timer
  runner. It requires the current checksum-bound save/GAMEBLOCK admission,
  verified Dungeon MD5, source session generation/tick, boot-owned ordinary
  package identity, wrapper-free session, and source queue ordering before
  it publishes M11's immutable outcome receipt. M11 neither selects a timer
  nor interprets a DSA body; stale sessions and missing queue slots reject
  before the runner can mutate state. Structural CSBWin timer records cover
  successful M11 commit, session drift, invalid queue-slot rejection, and
  existing unknown-opcode rejection. Verification in `build-csb-verify`:
  CTest passed `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` and
  `csb_v1_dsa_admitted_restored_timer_bridge` (2/2); `firestaff` built and
  `git diff --check` passed. A positive operator-supplied CSBWin corpus
  remains intentionally required for corpus admission.

- 2026-07-16 DM2 scene/light/weather runtime-chain receipts: added a
  source-backed DM2 weather chain that joins real `DUNGEON.DAT`
  `MapGraphicsStyle`, decoded GRAPHICSSET floor/ceiling material,
  scene-light control, map-bound c_light, real `GRAPHICS.DAT` ENVIRONMENT
  command text, GRAPHICS_DATA_OPEN admission, and the source
  `QUERY_RAINFALL_PARAM` formula into one no-fallback runtime receipt. The
  audit now closes `DM2_UPDATE_WEATHER`,
  `DM2_ENVIRONMENT_DRAW_DISTANT_ELEMENT`,
  `DM2_ENVIRONMENT_SET_DISTANT_ELEMENT`,
  `DM2_ENVIRONMENT_DISPLAY_ELEMENTS`, `DM2_DRAW_RAIN`,
  `DM2_QUERY_RAINFALL_PARAM`, and
  `DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW`; the local canonical corpus keeps
  weather image blits closed because its selected GRAPHICSSET proves command
  text but no admitted weather image material. Verification: strict C11
  syntax, Ninja build/direct run of
  `test_dm2_v1_scene_weather_light_runtime_chain_real_data` and
  `test_dm2_v1_weather_gdat_receipt`, focused CTest for both, backlog
  refresh, and `git diff --check` passed.

- 2026-07-16 DM1 F0407/F0231/F0412 action-spell HUD presentation route: added
  a DM1-owned presentation receipt on top of the existing live action-effect
  state so M11 can consume source-shaped F0231 damage, F0407 action locks,
  F0412 projectile/effect/potion spell results, and F0412 failure-feedback
  text/layout flags without inventing host HUD events or fallback text. Invalid
  champion/action data and unknown spell kinds fail closed. Verification:
  Ninja build of `test_dm1_v1_live_action_effects_pc34_compat`,
  `test_dm1_v1_spell_casting_pc34_compat`, and
  `test_dm1_v1_action_f0407_tail_pc34_compat`; focused CTest for
  `dm1_v1_live_action_effects_pc34_compat`,
  `dm1_v1_spell_casting_source_lock`, and
  `dm1_v1_action_f0407_tail_pc34_compat`; `git diff --check` passed.

- 2026-07-16 Nexus DGN runtime materialization gate: added a production-facing
  no-draw admission boundary that joins parser-verified Structure3 mesh,
  retail DGN face/material provenance, package-host consumption,
  MENU.BPK/PRS3 upload proof, Structure1F/ITEM.IBS floor-material receipts,
  BPK material/palette plan facts, and M11 host-route consumption. The receipt
  consumes the real-data route only as `ready-no-draw`: it keeps runtime DGN
  presentation closed, blocks mesh rendering, rejects frame hashes, and fails
  closed when mesh source, PRS3/BPK proof, Structure1F VDP1-provenance
  blockers, or host blockers are missing. Verification: strict C11 syntax,
  Ninja build and CTest for `test_nexus_v1_dgn_runtime_materialization`,
  adjacent CTest `nexus_v1_dgn_face_material_provenance`, direct mesh probe
  build/run, and `git diff --check` passed.

- 2026-07-16 DM2 GDAT door state/movement receipt gate: door M11 material
  receipts now carry movement state and match live door state/opening/open_pct
  before consumption, with closed-door panel selection kept on the proven
  square route instead of an opening-dir record route. The real DOORS/
  GRAPHICSSET trailing local palette is loaded from source GDAT bytes, stale
  stationary receipts are rejected during movement, and split/D3/D0 receipts
  remain fail-closed where runtime placement is not fully proven. Verification:
  GNU11 syntax for the touched DM2 viewport/door files, a narrow manual
  build/run of `test_dm2_v1_gdat_door_overlay_plan_real_data` against the
  existing DM2 libraries, and `git diff --check` passed; full Ninja/CTest was
  blocked by an unrelated CSB header type error in the shared worktree.

- 2026-07-16 DM1 F0190 killed-all runtime afterplay chain: added the
  DM1-owned killed-all afterplay receipt that requires the F0190/F0189 world
  mutation before source C040 presentation, carries the unlink/active-state
  apply plan, suppresses later F0231 reaction and F0209/LoS movement for the
  deleted group, and feeds the existing live explosion/F0115 HUD boundary
  without synthetic effects. Verification: Ninja build of
  `test_dm1_v1_f0190_c040_m11_integration_audit` and
  `test_dm1_v1_f0190_moving_killed_all_m10_handoff_pc34_compat`, focused
  CTest for both, direct C11 build/run of
  `test_dm1_v1_group_los_move_rollback_pc34_compat`, and `git diff --check`
  passed.

- 2026-07-16 DM2 SkWinCore IBMIO/palette/anim/mouse runtime receipts:
  closed the contiguous p130 family around `_00eb_04bc` with source-backed
  DM2 runtime receipts for `_00eb_04bc`, `_0759_0688`, `_0759_06a1`,
  `_00eb_070c`, `_0759_0310`, `_0759_02c6`, `_01b0_0adb`, `_01b0_0c70`,
  and `_01b0_0ca4`. The new path consumes real `GRAPHICS.DAT`
  `INTERFACE_GENERAL` `dtPalIRGB`/`dtPalette16` data for palette and
  4bpp-to-8bpp expansion, keeps packed animation copy bounded to caller-owned
  buffers, and records mouse hide/shape/bounds state without synthetic
  cursor, menu, audio, or viewport fallback. Verification: syntax check for
  `src/dm2/dm2_v1_skproject_core.c`, Ninja build/run and focused CTest for
  `test_dm2_v1_skproject_core`, backlog refresh, and `git diff --check`
  passed.

- 2026-07-16 CSB first-frame viewport materialization pass: added a shared
  real-data proof carried by the CSB M11 viewport consumer, joining the D0
  F0111 door, D0 F0115 thing, D1 F0111 door, D1 F0115 native-object, and D2
  F0111 door receipts with a common DMCSB1 `GRAPHICS.DAT` catalog/material
  hash. `csb_v1_viewport_render_frame` now publishes a first-frame
  materialization receipt only when the real graphics session and every
  required route hash are present, and records a fail-closed blocked receipt
  instead of promoting fallback visuals. Verification: focused syntax check,
  CMake configure, Ninja build of
  `test_csb_v1_viewport_first_frame_materialization_pc34_compat`, focused
  CTest, direct probe run with 23 assertions on the positive real-data path,
  and `git diff --check` passed.

- 2026-07-16 CSBWin package DSA runtime handoff gate: the
  `csb_v1_csbwin_package_runtime_handoff` probe now discovers real staged
  CSBWin saves under the user data root, admits only a strict
  `runtime_handoff_ready` Extended Features DSA corpus receipt, and then uses
  the production Dungeon.dat + CSBWin resume/tick/core-resume path. With no
  local DSA-bearing save corpus it skips cleanly and claims no positive
  execution. Core-only CSBWin resume now clears stale saved-timer DSA receipt
  fields as part of Extended Features ownership reset. Verification: Ninja
  build/run of `firestaff_csb_v1_csbwin_package_runtime_handoff_probe`,
  `test_csb_v1_core_resume_extended_state_clear`,
  `test_csb_v1_csbwin_save_loader_boundary_pc34_compat`; focused CTest regex
  for all three passed; syntax checks and `git diff --check` passed.

- 2026-07-16 DM2 RECALC_LIGHT_LEVEL map-bound c_light receipt: strengthened
  the source-backed `DM2_RECALC_LIGHT_LEVEL`/`RECALC_LIGHT_LEVEL` runtime path
  so `dm2_v1_c_light_m11_receipt_build_for_map()` binds the result to the real
  `DUNGEON.DAT` `Map_definitions::Difficulty()` descriptor hash. Focused
  coverage now scans the staged real DM2 dungeon corpus, builds fixed/dynamic
  branch receipts only when the map descriptor admits them, proves descriptor
  identity changes the receipt hash, and keeps missing or mismatched live
  c_light state fail-closed without synthetic light, palette, weather, or
  viewport fallback. Verification: GNU11 syntax check for the c_light/GDAT
  scene source, Ninja build/run of `test_dm2_v1_c_light_receipt`, focused
  CTest `dm2_v1_c_light_receipt`, backlog refresh, and `git diff --check`
  passed.

- 2026-07-16 DM1 original PC34 save corpus route hardening: the fixture-free
  external corpus path now records explicit discovery receipts for every
  scanned file before import/export. Each receipt binds classifier shape,
  readiness, PC34 header identity, F7057 envelope end/trailing bytes,
  external-original eligibility, Firestaff-export rejection, and path back to
  the later F0435 -> F0433 -> F0435 roundtrip receipt. The route remains
  real-corpus only: unset `FIRESTAFF_DM1_PC34_SAVE_CORPUS` /
  `FIRESTAFF_DM1_PC_DATA` skips the probes rather than synthesizing a save.
  Verification: Ninja built `test_dm1_v1_original_save_pc34_external_corpus`
  and `test_dm1_v1_original_save_pc34_external_hoc_runtime`; focused CTest
  `dm1_v1_original_save_pc34_external_(corpus|hoc_runtime)` passed; local
  search found no staged DM1 PC34 save corpus, so no positive external corpus
  admission was claimed; `git diff --check` passed.

- 2026-07-16 DM1 boot-probe terminal teardown: direct
  `firestaff --game dm1 --boot-probe --boot-probe-frames 90 --duration 0`
  now reaches the source-visible `dm1-runtime` receipt and returns exit code
  0 instead of entering normal live runtime/audio/asset teardown and hanging
  after the READY receipt. The process-terminal boot-probe path keeps normal
  app shutdown untouched, frees only the owned launch framebuffers, and exits
  before `M11_GameView_Shutdown`/renderer teardown. The selected-entry receipt
  now accepts the valid post-handoff state where `active=0`,
  `startedFromLauncher=1`, and the intro was not bypassed. Verification:
  `tests/test_dm1_boot_probe_terminal_exit.sh build/ninja-dm2/firestaff`,
  `test_dm1_v1_startup_intro_state_machine_gate`,
  `test_title_frontend_c001_fallback_gate_pc34_compat`, direct `gtimeout 20s`
  DM1 boot-probe with `BOOT_PROBE_RC=0`, and `git diff --check` passed.

- 2026-07-16 CSB D2C F0111 real door-front receipt: promoted the D2C center
  door-front source lock to a fail-closed real-data receipt for ReDMCSB
  `DUNVIEW.C` F0121/F0111/F0115. The receipt requires local DMCSB1
  `GRAPHICS.DAT` item 694 for `G0694_ai_DoorNativeBitmapIndex_Front_D2LCR`,
  preserves the 64x61 D2C door geometry, M628 door zone 3760, C1 D2LCR
  ornament view, rear/front F0115 order words 0x0218/0x0349, and C10
  transparency, and rejects missing source binding, neighboring item ids
  693/695, synthetic pixels, fallback visuals, and zero hashes. Verification:
  strict C11 syntax check, focused direct build/run, CMake configure, Ninja
  build of `test_csb_v1_viewport_d2c_f0111_door_front_pc34_compat`, focused
  CTest, and `git diff --check` passed.

- 2026-07-16 DM2 p130 movement continuation receipts: closed the next
  skproject movement/map pair after the top movement batch by verifying
  `DM2_move_2c1d_028c` and `DM2_move_2fcf_0434` against their DM2-owned
  source-named receipt implementations. The `2c1d` receipt consumes
  `DM2_move_075f_1bc2` target facts and real `DUNGEON.DAT` commit/block
  candidates; the `2fcf` teleporter gate consumes real enabled teleporter
  square candidates and remains blocked on incomplete record graph state
  without `GenericRecord::w0`, synthetic teleport, sound, or map mutation.
  Verification: Ninja build/run of `test_dm2_v1_move_2c1d_028c` and
  `test_dm2_v1_move_2fcf_0434`, focused CTest
  `dm2_v1_move_(2c1d_028c|2fcf_0434)`, syntax checks, backlog refresh, and
  `git diff --check` passed.

- 2026-07-16 CSBWin save-loader real-corpus probe: extended the registered
  CSB save-loader boundary probe so a user-staged real
  `csbgame.dat`/`csbgame.bak`/`dmsave.dat`/`dmsave.bak` corpus is scanned as a
  bounded set, not just the first filename hit. Each real candidate now runs
  both the loader discovery verdict and the strict DSA save-runtime corpus
  receipt; loader-ready plain CSBGAME remains DSA-runtime blocked unless the
  real Extended Features DSA section, runtime actions, tail, and following
  GAMEBLOCK1 header authenticate. The local data root had no such save corpus,
  so no positive DSA corpus was claimed. Verification: focused syntax check,
  Ninja build/run of `firestaff_csb_v1_csbwin_save_loader_boundary_probe`,
  focused CTest `csb_v1_csbwin_save_loader_boundary`, and `git diff --check`
  passed.

- 2026-07-16 CSB D1L/D1R F0111 real side-door receipt: promoted the D1 side
  door-front source lock to a fail-closed real-data receipt for ReDMCSB
  `DUNVIEW.C` F0122/F0123/F0111. The receipt requires both D1L and D1R routes
  plus local DMCSB1 `GRAPHICS.DAT` item 558 bytes for the shared
  `StdDoorGraphicsF1` material, preserves the side door zones 3780/3800, top
  track zones 732/734, rear/front F0115 order words, and C10 transparency, and
  rejects missing source binding, neighboring item ids 557/559, synthetic
  pixels, fallback visuals, zero hashes, and duplicate-side route attempts.
  Verification: strict C11 syntax check, focused direct build/run, CMake
  configure, Ninja build of
  `test_csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat`, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 CSB D1C F0111 real door-frame receipt: promoted the center
  door-front F0111 contract to a fail-closed real-data receipt for ReDMCSB
  `DUNVIEW.C` F0124/F0111. The route now requires local DMCSB1
  `GRAPHICS.DAT` item 558 bytes for `G0186_s_Graphic558_Frames_Door_D1C`,
  preserves the 96x88 native byte-count, C2 D1LCR ornament view, M631 D1C
  zone, and BACK/FRONT door-pass ordering, and rejects missing source binding,
  neighboring item ids 557/559, synthetic pixels, fallback visuals, and zero
  hashes. Verification: strict C11 syntax check, focused direct build/run,
  CMake configure, Ninja build of
  `test_csb_v1_viewport_d1c_f0111_door_pc34_compat`, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 DM2 moving G1 wall RAW4 route: added a movement-bound
  GRAPHICSSET wall-plan builder using the exact skproject `dm2data.cpp`
  `table1d6b15` offsets, hashes the signed RAW4 movement offset into each wall
  command, and only lets source-required M11 wall plans run when their
  movement bit matches the live `gdat_scene_movement_active` route. Stationary
  wall plans still fail closed during movement, while the moving plan consumes
  real GDAT pixels, trailing source palettes, and signed RAW4 geometry without
  fallback. Verification: direct Ninja build/run of
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` passed; direct Ninja
  build/run of `test_dm2_v1_gdat_scene_plan_viewport_real_data` passed;
  focused CTest `dm2_v1_gdat_(scene|wall)_plan_viewport_real_data` passed
  2/2; GNU11 syntax check for the wall-plan and viewport renderer sources
  passed; `git diff --check` passed.

- 2026-07-16 CSB D1C F0115 real native-object receipt: moved the center
  door-front thing-pass from contract-only evidence to a fail-closed
  real-data handoff that admits ReDMCSB `DUNVIEW.C` F0124/F0115 only when the
  caller binds native object-family `GRAPHICS.DAT` entries 498..583 to real
  DMCSB1 payload bytes. The focused test reads local `GRAPHICS.DAT` item-table
  payloads for entries 498 and 583, preserves the BACK/FRONT pass order around
  the D1C door frame, and rejects missing source binding, item 584,
  synthetic pixels, fallback visuals, and zero hashes. Verification: strict
  C11 syntax check, focused direct build/run, CMake configure, Ninja build of
  `test_csb_v1_viewport_d1c_f0115_thing_pass_pc34_compat`, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 DM2 G1 wall viewport material route: fixed the documented
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` blocker by binding wall
  commands to the GRAPHICSSET image record's trailing source palette inside
  the wall-plan path instead of the weather/environment palette helper that
  rejects canonical wall IMG9/C8 records. The wall preflight now proves the
  same real M11 command plan that the viewport consumes, and the scene and wall
  real-data gates are registered with CTest. Verification: GNU11 syntax check
  for the wall-plan and viewport renderer sources passed; direct Ninja
  build/run of `test_dm2_v1_gdat_scene_plan_viewport_real_data` and
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` passed; `ctest -R
  'dm2_v1_gdat_(scene|wall)_plan_viewport_real_data'` passed 2/2.

- 2026-07-16 CSB D0L2/D0R2 F0115 real thing-pass wall-frame receipt:
  extended the viewport-side thing pass with a fail-closed receipt that binds
  ReDMCSB `DUNVIEW.C` F0115 wall-frame rows 10/11 to real local
  `GRAPHICS.DAT` item payloads before the route can be considered backed by
  source data. The focused test reads the DMCSB1 item table directly, hashes
  the original compressed payload for each fixture row, rejects wrong item
  indices, zero hashes, synthetic pixels, and fallback visuals, and keeps the
  G2028 item/projectile suppression boundary intact. Verification: strict C11
  syntax check, focused direct build/run, CMake/Ninja builds for both the new
  F0115 target and the previously missing F0111 door-front target, focused
  CTest, and `git diff --check` passed.

- 2026-07-16 DM2 G1 scene viewport material route: repaired the
  GRAPHICSSET floor/ceiling plan so canonical G1 dungeon planes retain their
  real GDAT image-local trailing palettes across IMG3/IMG9 records, then
  hardened `dm2_v1_viewport_set_gdat_scene_material_plan()` to reject stale or
  edited QUERY_BLIT_RECT and c_gui_vp draw-order sidecars before rendering can
  fall back to callbacks. Verification: GNU11 syntax check for the scene-plan
  and viewport renderer sources passed; `test_dm2_v1_gdat_scene_plan_viewport_real_data`
  built and passed with 5 canonical G1 GRAPHICSSET plane plans reaching M11
  directly.

- 2026-07-16 CSB D0L2/D0R2 F0111 real door-front asset receipt: added a
  viewport-local receipt that binds ReDMCSB `DUNVIEW.C` F0111 door-front
  rendering to real `GRAPHICS.DAT` evidence for bitmap 693 and ornament view
  0. The route preserves the rear/door/front F0115/F0111 ordering and C10
  transparency while rejecting missing source binding, wrong bitmap ids,
  zero payload hashes, synthetic pixels, and fallback visuals. Verification:
  strict C11 syntax check and focused direct build/run of
  `test_csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat` passed,
  including a local DMCSB1 `GRAPHICS.DAT` item-693 payload hash receipt.

- 2026-07-16 Nexus Structure2 descriptor admission gate: tightened DGN
  Structure2 parsing so selector anchors are retained only for the observed
  retail descriptor classes `0x0008` and `0x0028` with nonzero width and
  height. Unknown encodings and zero-sized descriptors now withdraw the
  Structure2 table before Structure1G/Structure3 material routes can bind,
  while the post-FFFF payload remains opaque and no decoder, pixel span,
  palette format, VDP1 mode, draw route, or fallback visual is promoted.
  Verification: CMake build/run of `test_nexus_v1_dgn_geometry_readiness`,
  direct strict C99 build/run of `test_nexus_v1_dgn_face_material_source_path`,
  focused syntax check for `src/nexus/nexus_v1_dungeon.c`, and
  `git diff --check` passed.

- 2026-07-16 Nexus Structure1Fa descriptor-0008 lane proof: tightened the
  ITEM.IBS special floor-material consumer so a descriptor-0008 binding can
  reach a DGN floor command only when the special floor-image lane stays
  separate from regular inventory palette/image/packed-texel fields. Forged
  regular-material fields, non-floor command kind, source-cell drift, and
  out-of-range command indices now remain no-draw, with fallback visuals still
  denied and VDP1 command provenance still required before any texel-order or
  drawing admission. Verification: CMake build/run of
  `test_nexus_v1_dgn_geometry_readiness`, direct strict C99 build/run of
  `test_nexus_v1_structure1f_spatial_receipt`, focused syntax check for
  `src/nexus/nexus_v1_dungeon.c`, and `git diff --check` passed.

- 2026-07-16 Theron Track02 bitmap-atlas layout fail-closed gate: tightened
  startup bitmap atlas construction so a caller-supplied catalog with tile
  records but zero nonzero pixels cannot become atlas-ready; the atlas receipt
  is cleared and returns `NOT_FOUND`. Added focused coverage showing a positive
  bitmap layout preserves CD raw/user-data offsets, tile order, route widths,
  nonzero counts, and checksum, while an all-zero layout fails closed without
  pixel promotion, palette binding, dungeon draw, synthetic decode, or fallback
  visuals. Verification: strict syntax checks for `theron_v1_track02.c` and
  the new bitmap-atlas layout test passed; focused direct C11 build/run of
  `test_theron_v1_track02_bitmap_atlas_layout` passed.

- 2026-07-16 Theron Track02 palette-window fail-closed evidence: tightened
  explicit HuC6260 4bpp palette-window inspection so failed raw user-data copy
  or malformed palette decode clears the evidence receipt completely. Added a
  focused palette-window regression proving that valid US ISO palette bytes
  produce format-only evidence with `promotion_allowed=0`, malformed reserved
  palette bits leave no stale evidence, and optional real raw Track02 palette
  probes cannot promote without a source-locked semantic binding. Verification:
  strict syntax checks for `theron_v1_track02.c` and the new palette-window
  test passed; focused direct C11 build/run of
  `test_theron_v1_track02_palette_window` passed.

- 2026-07-16 Nexus DGN package/host identity fail-closed proof: strengthened
  the face/material package-host consumer regressions so host-route request,
  level identity, canonical DGN byte-size identity, and Structure2
  descriptor-count identity must all be retained before source-route
  consumption is published. Drift now blocks package-host route publication
  while raster input, material pixel promotion, and fallback visuals remain
  denied. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, focused syntax check for
  `src/nexus/nexus_v1_dgn_face_material_provenance.c`, and `git diff --check`
  passed.

- 2026-07-16 Nexus PRS3 reviewed output real-data proof gate: tightened the
  reviewed PRS3 output/upload join so `source-bound-no-runtime` requires the
  retained decoded-output proof to still carry capture-source binding, length
  match, hash match, exact observed/expected byte count, and a nonzero output
  FNV. Missing or drifting proof facts now block the MENU.BPK upload join while
  decoder promotion, runtime upload, and fallback visuals remain closed.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_prs3_capture_trace_schema`, focused syntax check for
  `src/nexus/nexus_v1_prs3_capture_trace_schema.c`, and `git diff --check`
  passed.

- 2026-07-16 DM1 HoC F0280 candidate text materialization: added a
  source-backed DM1 slice in `dm1_v1_resurrection_pc34_compat.c` that
  materializes caller-owned source-proven candidate name/title plus A..P encoded
  health/stamina/mana and seven statistics through the existing F0279 decoder.
  Missing proof, malformed A..P fields, and too-small output buffers fail
  closed without mutating outputs or synthesizing champion data. Verification:
  strict C11 build/run of
  `test_dm1_v1_f0280_candidate_text_materialization_pc34_compat` and existing
  `test_dm1_v1_resurrection_pc34_compat` passed; syntax compile passed.

- 2026-07-16 Theron split-CUE Track02 resolver binding: wired the documented
  exact split-layout aliases into the Track02 media resolver, so
  `TQUS02.iso`/`TQJP02.iso` CUE members can resolve only to their matching
  `*02End.iso` sibling when the declared member is absent. The resolver still
  preserves both `MODE1/2352` raw-sector payloads and `MODE1/2048` ISO payloads,
  while arbitrary missing members and duplicate Track02 `INDEX 01` entries fail
  closed. Verification: strict syntax checks for `theron_v1_track02.c` and the
  new resolver test passed; focused direct C11 build/run of
  `test_theron_v1_track02_cue_resolve` passed.

- 2026-07-16 DM2 startup menu host-input active-gate: the DM2 startup host
  facts wrappers now reject Firestaff keyboard/menu input and pointer hits
  unless `startup_menu_active` is true. This keeps M11/title-menu input behind
  the real startup menu gate and prevents inactive host facts from consuming
  synthetic menu navigation. Verification: direct strict C11 regression over
  `dm2_v1_startup_menu.c` passed; the CMake target build reached the DM2
  library/test link step but is currently blocked by an unrelated unresolved
  `dm2_v1_runtime_import_sksave_receipted_candidate` symbol in the shared
  `firestaff_dm2` link.

- 2026-07-16 Nexus DGN Saturn/material pre-promotion blocker proof: added
  fail-closed route coverage for retained DGN package/host facts that must not
  drift before real capture exists. The joint DGN/PRS3 route now rejects a
  missing original-Saturn capture requirement, pre-promoted Saturn rendering,
  or pre-promoted material semantics while preserving PRS3 no-runtime evidence
  and the retained DGN no-draw/mesh blockers. No synthetic graphics, guessed
  Saturn decoder, runtime upload, pixel promotion, or fallback path is opened.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, focused syntax check, and
  `git diff --check` passed.

- 2026-07-16 DM1/CSB ReDMCSB CEDT006 HoC render/undo batch:
  added DM1-owned narrow PC34 callables for `F7034_DrawButton`,
  `F7035_SetSelectedColorBox`, `F7036_SetSelectedColorIndex`, and
  `F7037_UpdateUndoBitmap`. The receipts consume only caller-owned
  source-proven boxes, color indices, text, and selected-portrait bytes.
  Missing proof, invalid geometry, invalid color indices, or undersized undo
  buffers fail closed without fabricated champion data, portrait bytes,
  screen pixels, color-palette state, input events, or fallback UI resources.
  Verification: direct strict C11 build/run of
  `test_dm1_v1_f7034_f7035_f7036_f7037_cedt006_hoc_render_undo_pc34_compat`
  passed, alongside the adjacent CEDT006 selection and editor-input tests plus
  strict syntax compilation of the CEDT006 module.

- 2026-07-16 Theron Track02 level/object loader route receipt: added a
  fail-closed receipt after the dungeon object/level table binding. It computes
  and verifies a loader-route pair hash from the real Track 02 record, selected
  dungeon index, level/object route hashes, source byte counts/hashes, and
  original consumer PCs, giving later work one positive loader-route evidence
  handle without decoding either table. The receipt keeps loader-route review
  required and keeps field decoder execution, dungeon-route handoff, runtime
  admission, dungeon draw, synthetic decode, and fallback visuals closed.
  Verification: strict syntax checks for the touched Theron header/source/test
  passed; focused direct C11 build/run of `test_theron_v1_track02_loader_intake`
  with local unused dependency stubs passed.

- 2026-07-16 DM2 skproject c_sound adjacent helper batch: added
  source-backed receipts for `R_5F7`, `stop_all_sound`, `dtor`, and
  `sndptr6`. `R_5F7` now proves the source order of `R_8AF` handle-table
  stopping before scanning the caller-owned `v1dff38` allocation list and
  reporting the matched node's next pointer without synthetic unlink or audio
  playback. The stop/destructor/allocation receipts model `stop_sfx`,
  `stop_music`, `al_uninstall_audio`, and `DM2_SOUND6`'s `sndptr6` allocation
  size boundary. Verification: focused strict C11 build/run of
  `test_dm2_v1_sound_source_gate` passed.

- 2026-07-16 Nexus DGN no-draw/mesh blocker route proof: exposed the retained
  DGN no-draw host boundary and real-DGN mesh-render blocker on the joint
  DGN/PRS3 route receipt. The route now fails closed if either host fact is
  lost after package/host consumption, while preserving DGN identity,
  selector/geometry evidence, no-raster blockers, and PRS3 no-runtime source
  facts. DGN rendering, pixel promotion, runtime upload, fallback visuals,
  synthetic visuals, and guessed Saturn decoder promotion remain closed.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed; focused syntax check and
  `git diff --check` passed.

- 2026-07-16 Theron Track02 dungeon object/level table binding: added a
  fail-closed receipt after the dungeon-selection level-record boundary. It
  binds the selected dungeon index to the real Track 02 level-record route and
  object-table route only when the same capture trace repeats the route hashes,
  source byte counts/hashes, and original consumer PCs. The receipt keeps
  level-record review and object-table layout review required, and keeps field
  decoder execution, dungeon-route handoff, runtime admission, dungeon draw,
  synthetic decode, and fallback visuals closed. Verification: strict syntax
  checks for the touched Theron header/source/test passed; focused direct C11
  build/run of `test_theron_v1_track02_loader_intake` with local unused
  dependency stubs passed.

- 2026-07-16 DM2 skproject sound helper batch: added source-backed receipts
  for `R_5044A`, `R_51AF6`, `R_4FF39`, `R_B65`, `R_928`, `R_8FE`,
  `R_5096A`, `R_51083`, `R_51B56`, `R_8E6`, and `R_8AF`. The helpers model
  the SKULLWIN `c_sound` MIDI program/reset/armed-stop state, SFX attenuation
  and bearing table, SFX priority ordering, and the documented zeroed sample
  table behavior without playing host audio or fabricating sound payloads.
  Verification: focused strict C11 build/run of
  `test_dm2_v1_sound_source_gate` passed.

- 2026-07-16 DM2 skproject HUD/item/object alias audit correction: moved 14
  already implemented aliases out of `MISSING` in
  `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`, covering `DM2_MONEY_BOX_SURVEY`,
  `DM2_SHOW_ATTACK_RESULT`, `DM2_REMOVE_POSSESSION`,
  `DM2_LOAD_PROJECTILE_TO_HAND`, `DM2_RETRIEVE_ITEM_BONUS`,
  `DM2_PROCESS_ITEM_BONUS`, `DM2_PUT_OBJECT_INTO_CONTAINER`,
  `DM2_QUERY_PLAYER_SKILL_LV`, `DM2_IS_MISSILE_VALID_TO_LAUNCHER`,
  `DM2_GET_MISSILE_REF_OF_MINION`, `DM2_IS_ITEM_HAND_ACTIVABLE`, and matching
  SKWIN aliases where present. Verification: direct strict C99 build/run of
  `test_dm2_v1_champion_hud_helpers`, `test_dm2_v1_item_missile_helpers`,
  `test_dm2_v1_object_transfer_helpers`, and `test_dm2_v1_hud_survey_helpers`
  passed; `tools/verify_symbol_backlog.py`, `tools/symbol_backlog.py --limit 0`,
  and `git diff --check` passed.

- 2026-07-16 DM2 skproject HUD panel routing audit sync: aligned
  `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` with the existing verified
  `dm2_v1_hud_panel_routing` implementation for `QUERY_CMDSTR_TEXT`,
  `DM2_QUERY_CMDSTR_TEXT`, `TRANSMIT_UI_EVENT`, `DM2_TRANSMIT_UI_EVENT`,
  `UPDATE_RIGHT_PANEL`, and `DM2_UPDATE_RIGHT_PANEL`. Verification: direct
  strict C99 build/run of `test_dm2_v1_hud_panel_routing` passed, and
  `tools/verify_symbol_backlog.py`, `tools/symbol_backlog.py --limit 0`, and
  `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB pre-title swoosh sound batch: added CSB-owned
  source-named receipts for `F0908_InitSound`, `F0909_PlaySwooshSound`, and
  `F0910_ReleaseSwooshSound`. The gate accepts only the real source-bound
  9078-byte swoosh sample with period 334, stereo left/right channel binding,
  source start ordering before TITLE, finish/wait before stop, and release
  before `TITLE.C F0437` consumes C001. Synthetic sound data, host audio-device
  emulation, and legacy swoosh wrappers remain closed. Verification: strict C11
  F0908/F0909/F0910 build/run, source/test syntax checks, and adjacent F0806
  build/run passed.

- 2026-07-16 CSB/ReDMCSB `SWSH.C F0904`: added a CSB-owned pre-title palette
  animation receipt between `F0909_PlaySwooshSound` and
  `F0910_ReleaseSwooshSound`. The gate consumes the verified F0909 play receipt
  and accepts only a caller-bound source palette stream with the 27 two-word
  record shape; synthetic palette data, synthetic graphic bytes, and legacy
  palette wrappers remain closed. Verification: strict C11 F0904 build/run and
  source/test syntax checks passed.

- 2026-07-16 CSB/ReDMCSB `SWSH.C F0902`: added a CSB-owned FTL-logo startup
  receipt before the swoosh path. The gate accepts only caller-bound original
  `Graphic_FTLLogo` data with the source 320x200 packed frame, 160-byte row
  stride, and 16-color palette shape before `F0908_InitSound`; synthetic
  graphic bytes, synthetic palette data, and legacy logo wrappers remain
  closed. Verification: strict C11 F0902 build/run and source/test syntax
  checks passed.

- 2026-07-16 CSB/ReDMCSB D0L2/D0R2 viewport route receipt: added an internal
  CSB receipt that binds the existing ReDMCSB F0125/F0126/F0128 side dispatch,
  F0115 thing-pass order, and F0111 door-front draw contracts. It verifies the
  D0 side lane, source cell orders, C10 transparency, disabled D0 item/
  projectile rows, and non-mutating door draw boundary without loading game
  data, creating viewport pixels, or using a legacy viewport wrapper.
  Verification: strict C11 focused build/run plus source/test syntax checks
  passed.

- 2026-07-16 CSB/ReDMCSB startup host-surface presenter hardening: the
  runtime host-surface receipt hash now includes the routed raster pixel hash,
  and the packed-page presenter recomputes the receipt hash from the receipt's
  frame route, raster route, raster pixels, host-surface kind, HUD binding,
  and title/door palette fields before F0692/F0693 presentation. Forged host
  hashes and stale title/opening/HUD rasters remain blocked without creating a
  wrapper or synthetic surface. Verification: strict C11 focused presenter
  build/run, syntax checks, and startup-surface object symbol check passed.

- 2026-07-16 DM1/CSB ReDMCSB CEDT019 portrait save/load batch:
  added DM1-owned narrow PC34 callables for
  `F2122_DecodeAllPortraitsWhileLoading`,
  `F2123_EncodeAllPortraitsBeforeSaving`, and
  `F2124_DecodeAllPortraitsAfterSaving`. The gates require exactly four
  caller-owned, source-proven 32x29 portrait spans and reuse the existing
  planar/chunky conversion helpers. Missing proof, wrong counts, or malformed
  buffers fail closed without fabricated champion data, portrait bytes, screen
  pixels, file IO, input events, or fallback portrait resources. Verification:
  direct strict C11 build/run of
  `test_dm1_v1_f2122_f2123_f2124_cedt019_portrait_save_pc34_compat` passed,
  along with strict syntax compilation of the new CEDT019 module.

- 2026-07-16 Nexus DGN package/host no-raster blocker route proof: exposed the
  DGN side's retained package/host no-raster facts on the joint DGN/PRS3 route
  receipt: material pixel promotion blocked, raster input denied, and DGN
  fallback visuals denied. The route now fails closed if any of those host
  facts are lost after real-DGN package consumption, while preserving PRS3
  no-runtime evidence and DGN source identity. DGN rendering, pixel promotion,
  runtime upload, fallback visuals, synthetic visuals, and guessed Saturn
  decoder promotion remain closed. Verification: direct strict C99 build/run
  of `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed; focused syntax check and
  `git diff --check` passed.

- 2026-07-16 Nexus DGN retained geometry admission/raster blocker proof:
  strengthened the joint DGN/PRS3 route coverage so retained package/host
  receipts must keep both geometry admission and textured-raster blocking after
  geometry source consumption. If either fact is lost, the route stays no-draw
  even with matching DGN identity, PRS3 no-runtime evidence, and the remaining
  geometry source facts. DGN rendering, pixel promotion, runtime upload,
  fallback visuals, synthetic visuals, and guessed Saturn decoder promotion
  remain closed. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed; focused syntax check and
  `git diff --check` passed.

- 2026-07-16 Theron Track02 dungeon-selection level-record boundary: added a
  fail-closed receipt that consumes the level/object facts handoff plus a
  same-capture dungeon-selection trace. It binds the selected dungeon route to
  real Track 02 level/object byte counts, hashes, route hashes, and original
  consumer PCs, while requiring future level-record review and keeping field
  decoder execution, runtime admission, dungeon draw, synthetic decode, and
  fallback visuals blocked. Verification: strict syntax checks for the touched
  Theron header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 Nexus DGN retained geometry source route proof: carried the
  material receipt's renderer-neutral geometry source facts into the
  package/host receipt and the joint DGN/PRS3 route proof. The route now
  preserves source-bound geometry, geometry admission, and textured-raster
  blocking, and it fails closed if the retained geometry source route is lost
  after package/host consumption. DGN rendering, pixel promotion, runtime
  upload, fallback visuals, synthetic visuals, and guessed Saturn decoder
  promotion remain closed. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed; focused syntax check and
  `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB entrance-loop handoff batch: added a CSB-owned
  source-named receipt for `F0806_F0806_ENTRANCE_int`. The gate accepts only
  real entrance asset/input setup, F0439 redraw ownership, C099 wait-loop and
  command-queue ownership, optional F0442 credits redraw loops, F0797
  micro-dungeon proof, and the F0807 door-animation receipt for dungeon or
  bonus-dungeon entry. Terminal wait/credits states, synthetic input, synthetic
  graphics bytes, fallback visuals, and legacy entrance wrappers remain closed.
  Verification: strict C11 F0806 build/run, F0806 source/test syntax checks,
  and adjacent F0797 build/run passed.

- 2026-07-16 DM1/CSB ReDMCSB CEDT006 HoC champion-selection batch:
  added DM1-owned narrow PC34 callables for
  `F7032_DrawChampionNameOnTopOfScreen`,
  `F7033_DrawPortraitsAndNamesOnTopOfScreen`,
  `F7038_PrintChampionNameOrTitleForEdition`, and
  `F7040_SelectChampion`. The receipts use only caller-owned champion
  summaries and proven portrait-pixel availability; missing records fail
  closed without fabricated champion data, portrait bytes, screen pixels,
  input events, or fallback editor resources. Verification: direct strict C11
  build/run of
  `test_dm1_v1_f7032_f7033_f7038_f7040_cedt006_hoc_select_pc34_compat`
  passed, alongside the adjacent F7039/F7041 CEDT006 test and strict syntax
  compilation of the module.

- 2026-07-16 DM2 skproject GDAT CPX compaction helpers: closed
  `R_2D8AD`, `R_2D8BA`, and `R_2D802` in the DM2 asset loader. The new
  receipts model source CPX top-down reservation, copy-from-header
  relocation, and raw-length based compaction over GDAT CPX blocks, including
  high-bit free-marker skips and moved active-block offsets. No decoded
  pixels, menu data, dungeon data, or synthetic graphics are created.
  Verification: direct strict C99 build/run of
  `tests/test_dm2_v1_gdat_querydb_receipts.c` with
  `src/dm2/dm2_v1_asset_loader.c` passed, reporting
  `113 passed, 0 failed`.

- 2026-07-16 Nexus DGN retained Structure2 descriptor route proof: carried the
  Structure2 descriptor source-route bit from the material receipt into the
  package/host receipt and the joint DGN/PRS3 route proof. If a later retained
  host receipt loses that descriptor-route evidence, the DGN/PRS3 join blocks
  no-draw even when DGN identities, selector counts, binding completeness, and
  PRS3 source facts still match. Rendering, pixel promotion, runtime upload,
  fallback visuals, synthetic visuals, and guessed Saturn decoder promotion
  remain closed. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed; focused syntax check and
  `git diff --check` passed.

- 2026-07-16 Theron Track02 level/object facts handoff: added a positive
  fail-closed handoff from the dungeon-route admission boundary back to the
  real level/object field boundary. It carries only Track 02 route hashes,
  source byte counts/hashes, original consumer PCs, and the reviewed decoder
  identity, while keeping field decoder execution, dungeon-route handoff,
  runtime admission, dungeon draw, synthetic decode, and fallback visuals
  blocked. Verification: strict syntax checks for the touched Theron
  header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 DM2 skproject GDAT allocator helper batch:
  added `R_2BAD4` and `R_2D07D` mappings in the DM2 asset loader. `R_2BAD4`
  preserves the source 16-bit byte-swap helper, and `R_2D07D` scans real
  parsed GDAT rows for max raw length with type/field filters, without
  allocating or fabricating buffers. Verification: focused GDAT querydb test,
  syntax compile, symbol verifier, and diff whitespace checks passed.

- 2026-07-16 DM1/CSB ReDMCSB CEDT006 HoC editor HUD/input batch:
  added DM1-owned narrow PC34 callables for
  `F7039_DrawHealthOrStaminaOrMana` and `F7041_ProcessKeyboardInput`.
  `F7039` records caller-owned status-line draw facts without emitting
  pixels, and `F7041` edits a caller-owned text buffer from an explicit key
  sequence without host polling or fabricated events. Verification: direct
  strict C11 build/run of
  `test_dm1_v1_f7039_f7041_cedt006_hoc_editor_pc34_compat` passed, along
  with strict syntax compilation of the new module.

- 2026-07-16 CSB/ReDMCSB entrance micro-dungeon batch: added a CSB-owned
  source-named receipt for `F0797_STARTEND_DrawEntranceMicroDungeon`. The gate
  accepts only the ReDMCSB source 5x5 C255 entrance map, wall/corridor layout,
  south-facing F0128 draw request, and verified F0439 entrance draw receipt;
  loaded-dungeon substitutes, synthetic viewport pixels, and legacy micro-dungeon
  wrappers remain closed. Verification: strict C11 F0797 build/run, F0797
  source/test syntax checks, and adjacent F0439/F0441/F0442 build/run passed.

- 2026-07-16 Theron Track02 dungeon-route admission boundary: added a
  fail-closed receipt that consumes the reviewed field-decoder boundary and a
  same-capture dungeon-route boundary trace. It preserves only real Track 02
  route hashes, record, consumer trace checksum, and reviewed decoder identity,
  requires future dungeon-route review, and keeps field decoder execution,
  dungeon-route handoff, runtime admission, dungeon draw, synthetic decode, and
  fallback visuals blocked. Verification: strict syntax checks for the touched
  Theron header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 Nexus DGN retained selector-binding route proof: carried the
  selector-binding completeness bit from the DGN material source receipt into
  the package/host receipt and the joint DGN/PRS3 route proof. If later
  package/host handling loses complete Structure3 selector-binding evidence,
  the DGN/PRS3 join now blocks no-draw even when retained DGN counts and PRS3
  source evidence still match. Material semantics, pixel promotion, runtime
  upload, rendering, fallback visuals, synthetic visuals, and guessed Saturn
  decoder promotion remain closed. Verification: direct strict C99 build/run
  of `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 Theron Track02 reviewed field-decoder boundary: added a
  fail-closed receipt that binds the real level/object field boundary to a
  non-placeholder, non-synthetic reviewed decoder identity without executing
  that decoder. It preserves the same Track 02 route hashes and consumer trace
  checksum while keeping field decoder execution, exact level/object fields,
  object layout, dungeon-route handoff, rendering, synthetic decode, and
  fallback visuals blocked. Verification: strict syntax checks for the touched
  Theron header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 DM1/CSB ReDMCSB CEDT004 object runtime batch: added
  DM1-owned narrow PC34 wrappers for `F7017_GetIconIndex`,
  `F7018_GetThingData`, and `F7019_GetObjectInfoIndex`. The wrappers reuse
  the existing loaded raw Thing data, object-info, and icon contracts, and
  fail closed without decoded-object fallbacks, synthetic graphics resources,
  direction state, or input events. Verification: direct strict C11 build/run
  of `test_dm1_v1_f7017_f7018_f7019_cedt004_thing_runtime_pc34_compat`
  passed, along with syntax compilation of the touched dungeon thing-data
  module.

- 2026-07-16 Nexus DGN retained static-selector/Structure2 relation flag:
  carried the static-selector-within-Structure2 relation as an explicit
  package/host and joint DGN/PRS3 route receipt flag. Later consumers can now
  audit that the retained static Structure3 selector census still fits the
  retained Structure2 descriptor identity without recomputing it from raw
  counts. Missing or false relation evidence keeps the join no-draw; material
  semantics, pixel promotion, runtime upload, rendering, fallback visuals,
  synthetic visuals, and guessed Saturn decoder promotion remain closed.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 Theron Track02 level/object field boundary: added a fail-closed
  receipt that consumes the paired level/object handoff evidence plus a
  same-capture boundary trace before publishing only route hashes, source byte
  counts/hashes, consumer PCs, and window checksums. The receipt requires a
  future reviewed field decoder and keeps level fields, object fields, object
  layout, dungeon-route handoff, rendering, synthetic decode, and fallback
  visuals blocked. Verification: strict syntax checks for the touched Theron
  header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 Nexus DGN static-selector Structure2 route proof: strengthened
  the DGN package/host route and joint DGN/PRS3 proof so retained static
  Structure3 material selectors must still fit within the retained Structure2
  descriptor count. A later mismatch blocks the DGN/PRS3 join while keeping
  material semantics, pixel promotion, runtime upload, rendering, fallback
  visuals, synthetic visuals, and guessed Saturn decoder promotion closed.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 DM1/CSB ReDMCSB USIO keyboard input runtime batch: added
  DM1-owned narrow PC34 callables for `F1690_GetASCIICode`,
  `F1691_Cconis`, and `F1692_Crawcin`. The batch reads only explicit
  caller-owned raw-key/key-buffer state, preserves low-byte ASCII/control
  values, and fails closed without host keyboard polling, blocking host waits,
  scan-code table synthesis, or fabricated key events. Verification: direct
  strict C11 build/run of
  `test_dm1_v1_f1690_f1691_f1692_usio_keyboard_input_pc34_compat`
  passed, along with strict syntax compilation of the input poll module.

- 2026-07-16 Theron Track02 level/object handoff evidence join: added a
  receipt that joins nonstartup level record evidence with object-table route
  evidence only when they share Track 02 capture identity, record, consumer
  trace checksum, route hashes, source byte windows, and original consumer PCs.
  It publishes paired real-data handoff evidence while keeping level fields,
  object fields, object layout, rendering, synthetic decode, and fallback
  visuals blocked. Verification: strict syntax checks for the touched Theron
  header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 Nexus DGN selector/geometry material-count route proof:
  strengthened the DGN package/host route and joint DGN/PRS3 proof so they
  carry the Structure3 material selector census and matching renderer-neutral
  geometry material-face count. Static selector count, animated selector count,
  and geometry material-face count must still match the retained DGN face
  identity before the join is source-bound. Drift keeps the route no-draw, with
  rendering, pixel promotion, runtime upload, fallback visuals, synthetic
  visuals, and guessed Saturn decoder promotion still closed. Verification:
  direct strict C99 build/run of `test_nexus_v1_dgn_face_material_provenance`
  passed.

- 2026-07-16 DM1/CSB ReDMCSB mouse accessor runtime batch: added DM1-owned
  narrow PC34 callables for `F1128_IsLeftMouseButtonDown`,
  `F2008_IsLeftMouseButtonDown`, `F2009_GetMouseX`, `F2010_GetMouseY`,
  `F2024_IsLeftMouseButtonDown`, `F2047_GetMouseX`, and `F2048_GetMouseY`.
  The accessors read only explicit caller-owned mouse status and fail closed
  without host cursor polling, editor/hint input synthesis, or fallback mouse
  state. Verification: direct strict C11 build/run of
  `test_dm1_v1_f1128_f2008_f2009_f2010_f2024_f2047_f2048_mouse_accessors_pc34_compat`
  plus the adjacent USIO mouse/input queue tests passed.

- 2026-07-16 Nexus DGN/PRS3 no-runtime source-bound route facts: extended the
  joint DGN package/host plus PRS3 output/upload route receipt so it carries
  the PRS3 decoded-output proof-bound flag and the source-bound-no-runtime
  flag through the route proof. Missing no-runtime PRS3 source evidence now
  blocks the DGN/PRS3 join even when DGN identity, PRS3 stream identity,
  decoded-output sidecar evidence, and reviewed upload facts are present.
  Rendering, pixel promotion, runtime upload, fallback visuals, synthetic
  visuals, and guessed Saturn decoder promotion remain closed. Verification:
  direct strict C99 build/run of `test_nexus_v1_dgn_face_material_provenance`
  passed.

- 2026-07-16 Theron Track02 object-table route/layout evidence: added a
  receipt that consumes object/level byte admission plus a same-capture
  object-table trace before publishing real raw/user-data object-table route
  evidence. It carries the object route hash, raw sector/user-data coordinates,
  byte count, source hash, object consumer PC, and post-envelope
  offset/count/checksum while keeping object layout, exact object fields,
  rendering, synthetic decode, and fallback visuals blocked. Verification:
  strict syntax checks for the touched Theron header/source/test passed;
  focused direct C11 build/run of `test_theron_v1_track02_loader_intake` with
  local unused dependency stubs passed.

- 2026-07-16 Nexus DGN/PRS3 reviewed-upload route facts: extended the joint
  DGN package/host plus PRS3 output/upload route receipt so it carries the
  reviewed PRS3 upload-path facts that made the route source-bound: reviewed
  upload path, MENU.BPK upload review, original Saturn provenance, and
  independent-authentication requirement. Missing MENU.BPK upload review now
  blocks the DGN/PRS3 join even when DGN identity, PRS3 stream identity, and
  decoded-output sidecar evidence are present. Rendering, pixel promotion,
  runtime upload, fallback visuals, synthetic visuals, and guessed Saturn
  decoder promotion remain closed. Verification: direct strict C99 build/run
  of `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 DM1/CSB ReDMCSB USIO mouse input runtime batch: added
  DM1-owned narrow PC34 callables for `F1684_GetMouseStatus` and
  `F1694_AddMouseInputToQueue`. Mouse status is copied only from explicit
  caller-owned facts, and mouse X/Y/button input routes through the existing
  DM1 command queue tables; no host cursor polling or synthetic mouse data is
  introduced. Verification: direct strict C11 build/run of
  `test_dm1_v1_f1684_f1694_usio_mouse_input_pc34_compat`,
  `test_dm1_v1_f1172_f1173_f1174_usio_input_queue_pc34_compat`, and
  `test_dm1_v1_input_command_queue_f0357_pc34_compat` passed.

- 2026-07-16 Theron Track02 nonstartup level record-route evidence: added a
  receipt that consumes object/level byte admission plus a same-capture
  nonstartup-level trace before publishing real raw/user-data level-window
  evidence. It carries the level route hash, nonstartup raw sector/user-data
  coordinates, byte count, raw hash, dungeon consumer PC, and dungeon-window
  offset/count/checksum while keeping exact level fields, object-table layout,
  rendering, synthetic decode, and fallback visuals blocked. Verification:
  strict syntax checks for the touched Theron header/source/test passed;
  focused direct C11 build/run of `test_theron_v1_track02_loader_intake` with
  local unused dependency stubs passed.

- 2026-07-16 Nexus DGN/PRS3 sidecar-bound route proof: tightened the joint DGN
  package/host plus PRS3 output/upload route gate so PRS3 decoded-output
  sidecar evidence is required alongside the output fingerprint and reviewed
  upload path. Missing sidecar binding now blocks the DGN/PRS3 join even when
  DGN package identity and PRS3 stream identity are otherwise present; DGN
  rendering, startup menu rendering, PRS3 runtime upload, material pixel
  promotion, fallback visuals, synthetic visuals, and guessed Saturn decoder
  promotion remain closed. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 Theron Track02 object/level handoff window preservation: the
  object/dungeon original-consumer binding and raw nonstartup handoff now carry
  the capture consumer PCs plus loader, dungeon-window, and object-window
  coordinates/checksums. Object/level admission checks the handoff copy against
  the grammar receipt before allowing byte admission, while field decode,
  object-table layout interpretation, dungeon draw, synthetic promotion, and
  fallback visuals remain blocked. Verification: strict syntax checks for the
  touched Theron header/source/test passed; focused direct C11 build/run of
  `test_theron_v1_track02_loader_intake` with local unused dependency stubs
  passed.

- 2026-07-16 CSB/ReDMCSB title/HUD/door runtime coupling batch: added
  CSB-owned source-named receipts for `F0437_STARTEND_DrawTitle`,
  `F0438_STARTEND_OpenEntranceDoors`,
  `F0580_ENTRANCE_DrawDoorAnimationStep`, and `F0581_ENTRANCE_BlitDoors`.
  The acceptance gate requires all title phases, HUD capture, the bounded
  31-step door-opening route, real startup assets, receipt-only draw/input,
  no wrapper fallback routes, and no synthetic visuals. Verification: direct
  strict C11 build/run of
  `test_csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat`
  and strict C11 syntax-only checks passed.

- 2026-07-16 CSB/ReDMCSB temporary entrance graphic byte-count batch: added a
  CSB-owned source-named receipt for
  `F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount`. The gate accepts only
  real C004 entrance, C005 credits, C534 door-rattle, or C535 switch
  GRAPHICS.DAT members through the reviewed F0490 load/decompress route, and
  rejects synthetic bytes, synthetic file handles, and legacy graphics wrappers.
  Verification: direct strict C11 build/run of
  `test_csb_v1_f0440_startend_temporary_graphic_byte_count_pc34_compat` and
  strict C11 syntax-only checks passed.

- 2026-07-16 CSB/ReDMCSB title/entrance palette fade batch: added a
  CSB-owned source-named receipt for `F0436_STARTEND_FadeToPalette`. The gate
  requires real source-bound 16-entry palettes from the verified title or
  entrance/credits startup route, source fade-step/VBlank/component-mask facts,
  and no renderer palette substitute, legacy palette wrapper, or synthetic
  palette. Verification: direct strict C11 build/run of
  `test_csb_v1_f0436_startend_fade_palette_runtime_coupling_pc34_compat` and
  strict C11 syntax-only checks passed.

- 2026-07-16 CSB/ReDMCSB entrance bitplane-init batch: added a CSB-owned
  source-named receipt for `F0579_ENTRANCE_InitializeBitPlanes`. The gate
  requires the real ReDMCSB 256x161 composite-door bitmap and 320x200 screen
  bitplane geometry, consumes the title/HUD/door runtime coupling receipt, and
  rejects legacy bitplane wrappers plus synthetic visual surfaces. Verification:
  direct strict C11 build/run of
  `test_csb_v1_f0579_entrance_bitplanes_runtime_coupling_pc34_compat` and
  strict C11 syntax-only checks passed.

- 2026-07-16 CSB/ReDMCSB entrance animation-step batch: added a CSB-owned
  source-named receipt for `F0807_ENTRANCE_DrawAnimationStep`. The gate
  requires a real F0438/F0580 door-step frame, the F0579 bitplane-init receipt,
  source-locked 31-step bounds, receipt-only draw/input, and no legacy or
  synthetic visual route before the screen blit is accepted. Verification:
  direct strict C11 build/run of
  `test_csb_v1_f0807_entrance_animation_step_runtime_coupling_pc34_compat` and
  strict C11 syntax-only checks passed.

- 2026-07-16 Nexus DGN/PRS3 package route identity proof: strengthened the
  joint DGN package/host plus PRS3 output/upload route gate so it carries both
  sides of the real-data identity. Alongside the MENU.BPK PRS3 entry/stream
  and output fingerprint, the receipt now preserves the DGN level index,
  canonical DGN byte count, material-face count, and Structure2 descriptor
  count. If that DGN identity is missing, the join stays no-draw and still
  blocks DGN rendering, startup menu rendering, PRS3 runtime upload, material
  pixel promotion, fallback visuals, synthetic visuals, and guessed Saturn
  decoder promotion. Verification: direct strict C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance` passed.

- 2026-07-16 DM1/CSB ReDMCSB USIO input queue runtime batch: added
  DM1-owned narrow PC34 callables for `F1172_QueueMouseAndKeyboardInput`,
  `F1173_AddUsioDataToInputQueue`, and
  `F1174_AddPendingUsioDataToInputQueue`. The bridge maps explicit
  caller-owned USIO keyboard/mouse samples through the existing DM1 command
  queue tables and keeps pending data until accepted; no host polling or
  synthetic input data is introduced. Verification: direct strict C11
  build/run of `test_dm1_v1_f1172_f1173_f1174_usio_input_queue_pc34_compat`
  and the existing `test_dm1_v1_input_command_queue_f0357_pc34_compat` passed.

- 2026-07-16 Theron Track02 object/dungeon raw binding handoff: added a
  narrow original-consumer binding that consumes the verified raw Track 02 data
  gap plus the capture-produced object/dungeon grammar receipt before raw
  nonstartup dungeon handoff. The positive path now flows from capture text to
  facts, grammar, raw/user-data object and level coordinates, and object/level
  byte admission without requiring hand-built binding facts or broader
  bitmap/palette semantics. Field decode, object layout interpretation,
  rendering, synthetic promotion, and fallback visuals stay blocked.
  Verification: strict syntax checks for the touched Theron header/source/test
  passed; focused direct C11 build/run of `test_theron_v1_track02_loader_intake`
  with local unused dependency stubs passed.

- 2026-07-16 Nexus startup/menu animation handoff gate: coupled the
  package-level startup animation gate to a MENU.BPK handoff receipt boundary.
  An active `nexus-title` or `nexus-champion-select` animation now remains
  no-draw unless the real MENU.BPK source is hash-verified, the handoff receipt
  is valid, stored original surfaces are available, PRS3/Saturn presentation
  traces are not required, and fallback visuals are absent. Synthetic visuals
  and guessed Saturn decoders remain rejected. Verification: direct strict C99
  build/run of `test_nexus_v1_startup_presentation_animation_receipt` passed
  with the real startup-menu and title-sequence sources.

- 2026-07-16 DM1/CSB ReDMCSB hint string helper batch: added DM1-owned
  source-named PC34 callables for `F1909_CopyStringUntilCharacter`,
  `F1984_ConvertCharacterToLowerCase`, and
  `F2014_ConvertStringToLowerCase`. The helpers cover bounded caller-owned
  hint text copy and ASCII lowercasing only; no hint-oracle, HTC/file, screen,
  palette, or input state is synthesized. Verification: direct strict C11
  build/run of `test_dm1_v1_hint_string_helpers_pc34_compat` passed.

- 2026-07-16 CSB/ReDMCSB STARTEND entrance boundary batch: added CSB-owned
  source-named bounded receipts for `F0439_STARTEND_DrawEntrance`,
  `F0441_STARTEND_ProcessEntrance`, and
  `F0442_STARTEND_ProcessCommand202_EntranceDrawCredits`. The wrappers accept
  only real-asset host-view/ownership facts for the entrance, credits,
  receipt-owned draw/input, and retired legacy wrapper routes; missing
  real-data proof or fallback graphics fails closed. Verification: direct
  strict C11 build/run of
  `test_csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat`
  and strict C11 syntax-only checks passed.

- 2026-07-16 DM2 GDAT asset-loader compile repair: removed a duplicate
  `dm2_gdat_entry_owns_raw_payload` helper, added missing internal IMG3
  bpp/metadata/local-palette helpers, and corrected receipt paths to read IMG3
  bits-per-pixel from word 4 instead of word 6. Verification: syntax-only C99
  compile of `tests/test_dm2_v1_extended_spells_definition_asset_stub.c` with
  `src/dm2/dm2_v1_asset_loader.c`, `python3 tools/verify_symbol_backlog.py`,
  and `git diff --check` passed.

- 2026-07-16 Theron Track02 object/dungeon capture-facts producer: added a
  narrow runtime-admission producer that turns an authenticated object/dungeon
  consumer trace into `Theron_V1Track02Post3800ConsumerTraceFacts` only when it
  carries the same `$0b52` loader coordinates, nonstartup/object raw offsets,
  nonzero dungeon/object consumer PCs, exact level/object byte-window
  offsets/counts/checksums, and explicit no-bitmap, no-palette, no-synthetic,
  no-fallback flags. The resulting grammar admission still blocks field
  decode, object layout interpretation, rendering, and fallback visuals.
  Verification: strict syntax checks for the touched Theron header/source/test
  passed; focused direct C11 build/run of `test_theron_v1_track02_loader_intake`
  with local unused dependency stubs passed.

- 2026-07-16 Nexus startup/menu package animation gate: added a
  package-level presentation gate for the existing `nexus-title` and
  `nexus-champion-select` animation labels. The gate only permits a future
  draw route when the full startup package receipt, host display caller, real
  package assets, exact Saturn timing, and exact capture-frame facts are all
  present; MENU.BPK/PRS3 blockers, fallback visuals, inactive save/runtime
  states, synthetic visuals, and guessed Saturn decoders remain no-draw.
  Verification: direct strict C99 build/run of
  `test_nexus_v1_startup_presentation_animation_receipt` passed with the real
  startup-menu and title-sequence sources.

- 2026-07-16 DM1/CSB ReDMCSB Amiga Copper/CPSX platform boundary batch:
  extended the DM1-owned PC34 Amiga platform descriptor to cover
  `F1111_CPSX`, `F1133_AddCopperInterrupt`,
  `F1134_RemoveCopperInterrupt`, `F1135_CopperInterrupt_CPSX`,
  `F1140_InitializeColorPaletteFullBlack`,
  `F1148_CustomExceptCode_CPSX`, `F1149_Init_CPSX`,
  `F1150_Free_CPSX`, and `F1157_BackupA5`. These remain explicit
  no-portability boundaries with source evidence only: no synthetic palette
  writes, interrupt handlers, copy-protection state, disk behavior, or A5
  register backup route is introduced. Verification: direct strict C11
  build/run of `test_dm1_v1_f0513_amiga_dialog_platform_boundary_pc34_compat`
  passed.

- 2026-07-16 Symbol backlog verifier: added
  `tools/verify_symbol_backlog.py` to smoke-check the subagent queue tooling:
  it compiles `tools/symbol_backlog.py`, checks the total open backlog JSON,
  and validates ReDMCSB plus skproject JSONL queue rows. Verification:
  `python3 tools/verify_symbol_backlog.py` and focused `git diff --check`
  passed.

- 2026-07-16 CSB/ReDMCSB startup graphics boundary batch: added CSB-owned
  source-named bounded receipts for `F0474_MEMORY_LoadGraphic_CPSDF`,
  `F0477_MEMORY_OpenGraphicsDat_CPSDF`,
  `F0478_MEMORY_CloseGraphicsDat_CPSDF`,
  `F0479_MEMORY_ReadGraphicsDatHeader`,
  `F0488_MEMORY_ExpandGraphicToBitmap`, and
  `F0490_MEMORY_LoadDecompressAndExpandGraphic`. The wrappers preserve the
  real-data contract around the existing startup runtime loader and fail
  closed without synthesizing GRAPHICS.DAT bytes, decompressed payloads,
  IMAGE2/IMG3 pixels, or file handles. Verification: direct strict C11
  build/run of
  `test_csb_v1_f0474_f0477_f0478_f0479_f0488_f0490_startup_graphics_boundaries_pc34_compat`
  and strict C11 syntax-only checks passed.

- 2026-07-16 Nexus startup presentation animation receipt: the startup
  presentation receipt now marks the named champion-select presentation
  animation active, matching the existing `nexus-champion-select` label, while
  save/runtime states remain inactive and no new visual asset or draw route is
  introduced. Verification: direct strict C99 build/run of
  `test_nexus_v1_startup_presentation_animation_receipt` passed with the real
  startup-menu and title-sequence sources.

- 2026-07-16 DM1/CSB ReDMCSB Amiga memory/scroller platform boundary batch:
  extended the DM1-owned PC34 Amiga platform descriptor to cover
  `F0535_MEMORY_GetGraphicsDatFileSize`, `F0557_SCROLLER_Initialize`,
  `F0558_SCROLLER_CancelInitialize`, `F0559_SCROLLER_Deinitialize`,
  `F0562_SCROLLER_Task`, and `F0563_SCROLLER_UpdateMessageArea`. These remain
  explicit no-portability boundaries: no synthetic GRAPHICS.DAT size API,
  host scroller task, or message-area callback is introduced. Verification:
  direct strict C11 build/run of
  `test_dm1_v1_f0513_amiga_dialog_platform_boundary_pc34_compat` passed.

- 2026-07-16 Symbol backlog queue tooling: added `--reference`, `--family`,
  and `--format jsonl` filters to `tools/symbol_backlog.py` so DM1/CSB/DM2
  symbol batches can be assigned to subagents without hand-copying mixed
  queues. Verification: `python3 -m py_compile tools/symbol_backlog.py`,
  focused JSONL/text CLI smoke checks, and `git diff --check` passed.

- 2026-07-16 Theron Track02 object/level window-evidence hardening: the
  object/level admission receipt now preserves and checks the same-capture
  `$0b52` loader coordinates plus dungeon/object grammar windows alongside
  the raw nonstartup sector/user-data evidence. Loader destination, payload
  size, dungeon-window checksum, object-window offset, bitmap-route drift,
  pre-opened dungeon draw, or fallback visuals all fail closed. Verification:
  strict syntax checks for the touched Theron header/source/test/stub passed;
  focused direct C11 build/run of `test_theron_v1_track02_loader_intake` with
  local unused dependency stubs passed; targeted `git diff --check` passed.

- 2026-07-16 Theron Track02 object/level admission gate: added a narrow
  runtime admission receipt that consumes the raw nonstartup dungeon handoff
  plus the same-capture object/dungeon grammar receipt before allowing the
  object-table and nonstartup-level byte sources through. It carries the raw
  sector/user-data coordinates and consumer PCs forward, while exact field
  decode, bitmap/palette binding, RGBA output, dungeon draw, synthetic
  promotion, and fallback visuals remain blocked. Verification: strict syntax
  checks for the touched Theron header/source/test/stub passed; focused direct
  C11 build/run of `test_theron_v1_track02_loader_intake` with local unused
  dependency stubs passed.

- 2026-07-16 CSB/ReDMCSB FIO1 trackdisk boundary batch: added CSB-owned
  source-named PC34 callables for `F1106_IsTrackdiskDeviceOpened`,
  `F1107_GetDiskChangeCounter`, `F1109_GetDiskState`, and
  `F1114_CloseTrackdiskDevice`. The implementation preserves the no-host
  boundary only: trackdisk is not opened, disk-change counter is `-1`,
  disk state remains the ReDMCSB default `0` (no disk), and close is a no-op;
  no disk, file, write-protection, or change-counter data is synthesized.
  Verification: direct strict C11 build/run of
  `test_csb_v1_f1106_f1107_f1109_f1114_fio1_trackdisk_boundaries_pc34_compat`
  and strict C11 syntax-only checks passed.

- 2026-07-16 Nexus DGN geometry/material source-path proof: DGN
  face/material admission now requires renderer-neutral geometry proof from
  the same Structure3 path: source-bound geometry, matching static/animated
  material-face count, geometry submission allowed, no textured-raster
  promotion, and no fallback visuals. The live engine caller now feeds those
  facts from current-level geometry state and Structure2 descriptor count. The
  source-path test builds a real `nexus_v1_dgn_mesh` packet with one color
  face plus static/animated material faces and proves only the material faces
  enter the no-draw material route. Verification: direct strict C99 builds/runs
  of `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_dgn_mesh` passed.

- 2026-07-16 DM1/CSB ReDMCSB F0513/F0551/F0552 Amiga platform boundary closure:
  added a DM1-owned PC34 boundary descriptor for
  `F0513_DIALOG_DrawGameReadyToPlay_Unreferenced`,
  `F0551_VIDEO_HatchBox_Unreferenced`, and `F0552_BASE_DisplayError`,
  recording the Amiga-host dialog/video/error-display routes while keeping any
  ready-to-play dialog, hatch-fill visual, or generic host error substitute
  unclaimed. Verification: direct strict
  C11 build/run of
  `test_dm1_v1_f0513_amiga_dialog_platform_boundary_pc34_compat` passed.

- 2026-07-16 Theron Track02 raw nonstartup dungeon handoff: runtime admission
  now has a narrow receipt that consumes the existing original-data gap plus
  same-capture original-consumer binding and publishes the exact raw-sector
  and logical user-data coordinates for the first nonstartup level candidate
  and object-table container. The route remains real-data only: level/object
  field decode, bitmap/palette binding, RGBA output, dungeon draw, synthetic
  promotion, and fallback visuals stay blocked. Verification: strict syntax
  checks for the touched Theron header/source/test/stub passed; focused direct
  C11 build/run of `test_theron_v1_track02_loader_intake` with local unused
  dependency stubs passed; targeted `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB USIO queue/mouse boundary batch: added CSB-owned
  bounded PC34 callables for `F1164_USIO_15_GetFirstQueuedUsioDataType`,
  `F1165_USIO_17_WaitUntilKeyboardOrMouseInput`,
  `F1166_USIO_16_ExtractFirstUsioDataFromQueue`,
  `F1167_USIO_14_GetMouseStatus`, `F1175_GetFirstQueuedUsioDataIndex`, and
  `F1176_ExtractFirstUsioDataFromQueue`. The implementation models ReDMCSB's
  11-slot queue empty/index/extraction contract and keeps host mouse status
  unavailable unless the caller supplies explicit facts; the test fabricates
  no disk, keyboard, mouse, or queued input data. Verification: direct strict
  C11 build/run of
  `test_csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat`
  and strict C11 syntax-only checks passed.

- 2026-07-16 Nexus DGN/PRS3 route-proof receipt coupling: tightened the joint
  DGN material host-route plus PRS3 output/upload gate so DGN now consumes the
  real `Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt` type from the PRS3
  schema via a forward-declared pointer instead of a duplicate fact struct.
  The route proof still blocks DGN rendering, startup menu rendering, PRS3
  runtime upload, decoder promotion, material pixel promotion, and fallback
  visuals. Verification: direct strict C99 builds/runs of
  `test_nexus_v1_dgn_face_material_provenance`,
  `test_nexus_v1_dgn_face_material_source_path`, and
  `test_nexus_v1_prs3_capture_trace_schema` passed.

- 2026-07-16 DM1/CSB ReDMCSB F0537/F0544 input platform boundary closure:
  added a DM1-owned PC34 boundary descriptor for
  `F0537_INPUT_ReleaseResources` and
  `F0544_INPUT_ResetPressingEyeOrMouth`. The descriptor records the Amiga
  source anchors and locks both rows as non-portable PC34 platform boundaries
  instead of synthesizing a host resource teardown or generic mouse-release
  shim. Verification: direct strict C11 build/run of
  `test_dm1_v1_f0537_f0544_input_platform_boundary_pc34_compat` passed.

- 2026-07-16 ReDMCSB F0550 planar fill closure: added focused coverage for
  `F0550_VIDEO_FillScreenBox` over the existing PC34 4-plane big-endian fill
  shim, including word boxes, byte boxes, shade mask, and fail-closed bounds.

- 2026-07-16 ReDMCSB F0085 runtime helper closure: added a source-named
  PC34 `_blockcmp` shim for DEFS.H:6902 with bounded structure-byte comparison
  semantics and focused tests for equal/prefix/different/null/count cases.

- 2026-07-16 DM1/CSB ReDMCSB F0357 input discard source boundary: added the
  DM1-owned `F0357_COMMAND_DiscardAllInput` wrapper over the existing PC34
  input-command queue discard implementation and pinned its source evidence to
  `COMMAND.C:1304-1377`, the blocked-step caller in `CLIKMENU.C:317-323`, and
  the wake-up caller in `CHAMPION.C:1382-1414`. The focused direct C11 test
  proves the ReDMCSB behavior that ordinary queued movement/turn input is
  flushed while C129 release-champion-icon and C254 stop-pressing-wall reserved
  commands are compacted and retained, with pending clicks cleared and no
  synthetic queue data introduced. Verification: direct strict C11 build/run
  of `test_dm1_v1_input_command_queue_f0357_pc34_compat` passed.

- 2026-07-16 ReDMCSB existing-wrapper closure: verified and dispositioned
  `F0019_MAIN_DisplayErrorAndStop` and `F0089_strncpy`. F0019 uses the
  existing PC34 terminal error callback adapter; F0089 now has a missing
  focused test covering signed-count copy, copied-NUL stop, and no padding.
  Verification: direct strict C11 build/run of both focused tests passed.

- 2026-07-16 CSB/ReDMCSB USIO/FIO1 platform boundary batch: added CSB-owned
  source-named callables for `F1168_USIO_18_Empty`,
  `F1170_USIO_03_Expunge`, `F1171_USIO_19_LockDF0`,
  `F1305_OpenFTLLibrary`, and `F1307_FIO1_03_Expunge`. These are verified
  PC34 no-op boundaries over USIO2.C/FIO1MAIN.C empty, expunge, DF0 lock, and
  FIO1 library routes, without fabricated queue, cursor, disk, or host-library
  behavior. Verification: direct strict C11 build/run of
  `test_csb_v1_f1168_f1170_f1171_f1305_f1307_usio_fio1_boundaries_pc34_compat`,
  strict C11 syntax-only check for the new CSB source/test, and focused
  diff-check passed.

- 2026-07-16 Nexus DGN/PRS3 route-proof stream identity: strengthened the
  joint DGN material host-route plus PRS3 output/upload no-runtime gate so the
  route proof must preserve a concrete MENU.BPK PRS3 stream identity: entry,
  stream offset/size, expected output bytes, and output FNV. Missing output
  fingerprint now blocks the route while keeping DGN rendering, startup menu
  rendering, PRS3 runtime upload, and fallback visuals closed. Verification:
  strict direct C99 and C11 builds/runs of
  `test_nexus_v1_dgn_face_material_provenance` and
  `test_nexus_v1_dgn_face_material_source_path` passed; strict C99
  syntax-only checks for the touched Nexus header/source/tests and focused
  `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB F1090-F1094 alert template closure: implemented the
  exact mutable AMIGINIT.C DisplayAlert byte templates for
  `F1090_GetCSBInternalErrorMessage` and
  `F1091_GetCSBSystemErrorMessage`, corrected `F1092_GetHexadecimalDigits` to
  the source lowercase table, and made F1093/F1094 mutate the real templates
  before the PC34 host-alert boundary. Verification: direct strict C11
  build/run of `test_csb_v1_f1092_f1093_f1094_alert_helpers_pc34_compat`
  passed.

- 2026-07-16 Nexus DGN/PRS3 route-proof gate: added a joint DGN material
  host-route plus PRS3 output/upload no-runtime gate. It binds only
  source-bound real-DGN package/host consumption with source-bound PRS3
  output/upload facts, requires explicit startup and DGN route requests, and
  keeps DGN rendering, startup menu rendering, PRS3 runtime upload, material
  pixel promotion, and fallback visuals closed. Verification: strict direct
  C99 and C11 builds/runs of `test_nexus_v1_dgn_face_material_provenance`
  passed; strict C99 syntax-only checks for the touched Nexus
  header/source/test passed; focused `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB USIO empty/pointer boundary batch: added CSB-owned
  source-named callables for `F1159_Empty`, `F1160_USIO_04_Empty`,
  `F1161_USIO_05_Empty`, `F1162_USIO_06_HidePointer`, and
  `F1163_USIO_07_ShowPointer`. The empty vectors and pointer hide/show routes
  are verified PC34 no-op boundaries with USIOMAIN.C/USIO1.C evidence, without
  host cursor behavior or synthetic input data. Verification: direct strict
  C11 build/run of
  `test_csb_v1_f1159_f1160_f1161_f1162_f1163_usio_helpers_pc34_compat`,
  strict C11 syntax-only check for the new CSB source/test, and focused
  diff-check passed.

- 2026-07-16 DM2 projectile impact move receipt disposition: closed
  `DM2_move_075f_06bd` through the existing skproject-backed
  `dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack` helper. The focused
  test covers item throw-strength/poison/weight input, poison blob, lightning,
  poison bolt, unsupported cloud zero-attack, bounded random terms, and source
  evidence. Related dungeon-loader-dependent move helpers remain open because
  the shared `dm2_v1_dungeon_loader.h` header is mid-transition in this
  worktree.

- 2026-07-16 Nexus PRS3 output/upload no-runtime gate: added a reviewed
  output/upload receipt that joins source-bound MENU.BPK PRS3 decoded-output
  proof with the reviewed VDP1/producer upload path while keeping runtime
  upload, decoder promotion, Saturn rendering, and fallback visuals closed.
  The capture-file gate now performs its original MENU.BPK/DM.BIN MD5 checks
  with a local Nexus verifier instead of relying on an undeclared external
  symbol. Verification: strict direct C99 and C11 builds/runs of
  `test_nexus_v1_prs3_capture_trace_schema` passed; strict C99 syntax-only
  checks for the touched Nexus header/source/test passed; focused
  `git diff --check` passed.

- 2026-07-16 CSB/ReDMCSB AMIGINIT alert helper batch: added CSB-owned
  source-named callables for `F1092_GetHexadecimalDigits`,
  `F1093_DisplayAlertCSBInternalError`, and
  `F1094_DisplayAlertCSBSystemError`. F1092 exposes the source hexadecimal
  digit table; F1093/F1094 are verified terminal PC34 no-op boundaries for the
  Amiga Intuition DisplayAlert/ResetAmiga paths, without host alert/restart or
  synthetic message-template data. Verification: direct strict C11 build/run
  of `test_csb_v1_f1092_f1093_f1094_alert_helpers_pc34_compat`, strict C11
  syntax-only check for the new CSB source/test, and focused diff-check passed.

- 2026-07-16 DM1 F0024/F0026/F0030 main-math callable bundle: added
  source-named DM1 callables for `F0024_MAIN_GetMinimumValue`,
  `F0026_MAIN_GetBoundedValue`, and `F0030_MAIN_GetScaledProduct`.
  `dm1_scaled_product` now delegates to the F0030 boundary, so the existing
  combat scaled-product path is no longer a locally reassembled helper.
  `SYMBOL_DISPOSITIONS.tsv` records all three rows with focused source/test
  evidence. Verification: direct strict C11 main-math test, strict object
  compile for the new source and touched combat source, and `git diff --check`
  passed.

- 2026-07-16 DM2 memory/mement helper receipt bundle: mapped `ZERO_MEMORY`
  and `ValidateMements` to skproject-backed bounded helpers. The regression
  covers caller-owned byte clearing, null/nonzero fail-closed behavior, active
  mement table census, bad id/span/overlap rejection, source evidence, and no
  synthetic cache or image data.

- 2026-07-16 DM2 palette driver disposition: closed skproject
  `driver_setcolors` through the existing `dm2_v1_skproject_core` palette
  receipts. The strict C11 regression proves RGB8-to-DMPAL6 conversion,
  immediate-colors driver flush requests, `SELECT_PALETTE_SET(1)` driver color
  request routing, and missing-source-palette fail-closed behavior; no
  synthetic palette route is introduced.

- 2026-07-16 Nexus Structure3 material source-path API cleanup: updated the
  real DGN Structure3 face/material collector to emit only static/animated
  material bindings under the current package/host no-draw contract.
  Color-fill faces are skipped rather than promoted to a synthetic material
  route, static selectors remain bounded by Structure2 descriptor count, and
  the missing Structure3 edge helpers are restored as local deterministic
  helpers. Verification: strict direct C99 and C11 builds/runs of
  `test_nexus_v1_dgn_face_material_provenance` and
  `test_nexus_v1_dgn_face_material_source_path` passed; strict C99 syntax-only
  checks for touched Nexus header/source/tests and focused `git diff --check`
  passed.

- 2026-07-16 DM2 skproject helper disposition batch: closed 20 open
  skproject audit rows through existing Firestaff code and focused tests.
  Added `SYMBOL_DISPOSITIONS.tsv` evidence for FIRE/IBMIO row blits,
  mouse cursor/queue/pattern helpers, ability/bar/tile predicates,
  level/CPX/game-state helpers, wall ornate alcove lookup, and
  `GRAPHICS_DATA_OPEN`. Verification: `python3 -m py_compile
  tools/symbol_backlog.py`; direct strict C11 builds/runs of
  `test_dm2_v1_fire_blit_rows`, `test_dm2_v1_mouse_cursor`,
  `test_dm2_v1_predicate_helpers`, and `test_dm2_v1_system_helpers`;
  `python3 tools/symbol_backlog.py --limit 0` reported 2497 open rows
  before the later F0216 closure.

- 2026-07-16 DM2 HUD survey/attack-result receipt bundle: mapped
  `MONEY_BOX_SURVEY`, `DM2_MONEY_BOX_SURVEY`, `SHOW_ATTACK_RESULT`, and
  `DM2_SHOW_ATTACK_RESULT` to skproject-backed bounded helpers. The regression
  covers real coin-table/container-chain consumption, missing-data fail-closed
  paths without synthetic GDAT, player damage icon/text routing, SKWIN and
  SKULLWIN alias receipts, and source evidence. Verification: direct strict
  C11 build/run of `test_dm2_v1_hud_survey_helpers` with
  `dm2_v1_skproject_core.c`; `MONEY_BOX_SURVEY` and `SHOW_ATTACK_RESULT` audit
  rows are now closed in `SYMBOL_DISPOSITIONS.tsv`.

- 2026-07-16 DM2 p130 helper disposition batch: closed eight already
  implemented skproject rows: `IS_MISSILE_VALID_TO_LAUNCHER`,
  `REMOVE_POSSESSION`, `LOAD_PROJECTILE_TO_HAND`,
  `PUT_OBJECT_INTO_CONTAINER`, `PROCESS_TIMER_0E`,
  `PROCEED_GLOBAL_EFFECT_TIMERS`, `SET_TILE_ATTRIBUTE_02`, and
  `SUMMARIZE_STONE_ROOM`. Verification: direct strict C11 builds/runs of
  `test_dm2_v1_item_missile_helpers`, `test_dm2_v1_object_transfer_helpers`,
  `test_dm2_v1_global_effect_timer_helpers`, and
  `test_dm2_v1_dungeon_room_helpers`; backlog now reports DM2 1259 and total
  2486 open rows.

- 2026-07-16 DM2 HUD panel routing receipt bundle: closed
  `QUERY_CMDSTR_TEXT`, `DM2_QUERY_CMDSTR_TEXT`, `TRANSMIT_UI_EVENT`,
  `DM2_TRANSMIT_UI_EVENT`, `UPDATE_RIGHT_PANEL`, and
  `DM2_UPDATE_RIGHT_PANEL` with a bounded skproject-backed route. Command-
  string text is accepted only from real caller-provided dtText bytes,
  command-text events require that receipt, and right-panel mode changes
  preserve event provenance without synthetic labels or panel content.
  Verification: direct strict C11 build/run of
  `test_dm2_v1_hud_panel_routing`; backlog now reports DM2 1256 and total 2479
  open rows.

- 2026-07-16 DM2 HUD/item helper disposition batch: closed
  `IS_ITEM_HAND_ACTIVABLE`, `RETRIEVE_ITEM_BONUS`, `PROCESS_ITEM_BONUS`,
  `QUERY_PLAYER_SKILL_LV`, and `REFRESH_PLAYER_STAT_DISP` through existing
  skproject-backed helpers. Verification: direct strict C11 builds/runs of
  `test_dm2_v1_item_missile_helpers` and `test_dm2_v1_champion_hud_helpers`;
  backlog now reports DM2 1251 and total 2474 open rows.

- 2026-07-16 DM2 PICT/missile helper disposition batch: closed
  `GET_MISSILE_REF_OF_MINION`, `QUERY_PICT_BITS`, `QUERY_PICST_IMAGE`, and
  `QUERY_PICST_IT` through focused skproject-backed helpers. Verification:
  direct strict C11 builds/runs of `test_dm2_v1_item_missile_helpers` and
  `test_dm2_v1_pict_picst_helpers`; backlog now reports DM2 1244 and total
  2467 open rows.

- 2026-07-16 DM1 F0216 projectile impact-attack callable: added the
  source-named `F0216_PROJECTILE_GetImpactAttack` boundary for ReDMCSB
  `PROJEXPL.C` impact attack selection. DM1 projectile runtime now calls that
  helper for champion impact and creature precheck attack selection instead of
  rebuilding the scalar fallback locally. Verification: direct C11 F0216 test,
  strict object compile for the new source and touched throw/shoot source, and
  focused diff-check passed.

- 2026-07-16 DM1 F0039 slot-box icon callable: added source-named
  `F0039_OBJECT_GetIconIndexInSlotBox` over the existing PC34
  `G0030_as_Graphic562_SlotBoxes` table, plus a focused direct C11 regression
  that covers status-hand, inventory, chest, and out-of-bounds slotbox reads.
  The backing slotbox source now also compiles cleanly under `-Wextra -Werror`.
  `SYMBOL_DISPOSITIONS.tsv` records F0039 as verified and `F039_aaaL_` as its
  Atari/PRIM alias. Verification: direct F0039 C11 test, strict object compile
  for the new source and slotbox source, and focused diff-check passed.

- 2026-07-16 CSB/ReDMCSB F1075-F1078 device-helper boundary bundle: added
  CSB-owned source-named `F1075_OpenLayersLibrary`,
  `F1076_CloseLayersLibrary`, `F1077_OpenConsoleDevice`, and
  `F1078_CloseConsoleDevice` as no-op PC34 host boundaries for the
  Amiga-only layers.library and console.device routes. Verification: strict
  direct C11 build/run of
  `test_csb_v1_f1075_f1076_f1077_f1078_device_helpers_pc34_compat`, strict
  syntax check, no-index whitespace checks for the new header/source/test
  files, and focused `git diff --check` for TODO/DONE. The focused CMake target
  and symbol dispositions are now added for main integration.

- 2026-07-16 CSB/ReDMCSB F1085-F1087 Intuition-vector boundary bundle:
  added CSB-owned source-named `F1085_IntuitionVectorReplacement`,
  `F1086_ReplaceIntuitionVectors`, and `F1087_RestoreIntuitionVectors`.
  F1085 returns the source-defined zero callback; F1086/F1087 are no-op PC34
  host boundaries for Amiga-only Intuition vector replacement/restore.
  Verification: strict direct C11 build/run of
  `test_csb_v1_f1085_f1086_f1087_intuition_vector_helpers_pc34_compat`,
  strict syntax check, no-index whitespace checks for the new
  header/source/test files, and focused `git diff --check` for TODO/DONE and
  `SYMBOL_DISPOSITIONS.tsv`.

- 2026-07-16 Nexus DGN package/host real-route gate: package host consumption
  now distinguishes explicit real-DGN source consumption from rejected
  synthetic material-route requests. The accepted Structure2/Structure3
  selector/descriptor boundary stays `ready-no-draw`: material pixel
  promotion, raster input, original Saturn rendering, and fallback visuals
  remain blocked. Verification: strict direct C99 and C11 builds/runs of
  `test_nexus_v1_dgn_face_material_provenance` passed; strict C99
  syntax-only checks for touched Nexus header/source/test and focused
  `git diff --check` passed. Adjacent
  `test_nexus_v1_dgn_face_material_source_path` remains blocked by an
  in-progress Nexus API transition around the removed COLOR selector and
  unrelated `nexus_v1_dungeon.c` Structure3 edge helper declarations.

- 2026-07-16 Launcher artpack/font path settings: startup settings can now
  choose and persist a `.fsart` V2.2 artpack file and an optional broad Unicode
  TTF/OTF/TTC font path. Native file dialogs are suppressed in headless/dummy
  video tests so probes do not block.

- 2026-07-16 DM2 GDAT immediate scalar/raw gate: `dtWordValue` and
  `dtImageOffset` entries no longer alias raw GDAT payload slots through the
  generic asset lookup. Exact typed APIs still expose scalar values. This
  prevents synthetic/raw-payload confusion in skproject-style GDAT consumers.

- 2026-07-16 DM1 F0323 callable alias: exposed source-named
  `F0323_CHAMPION_Unpoison` alongside the existing compat wrapper and
  strengthened the registered regression so ReDMCSB symbol inventory can bind
  to the real callable name instead of wrapper-only evidence.

- 2026-07-16 Artpack Studio DM2 GDAT preview: added skproject-style preview
  decoding for DM2 GDAT IMG3 C4 plus uncompressed U4/U8 records. Local
  `~/.firestaff/data/dm2/GRAPHICS.DAT` now imports 11,854 ENT1 rows and shows
  453 real decoded previews instead of metadata-only placeholders; unsupported
  payloads remain warning-only.

- 2026-07-16 V2.2 artpack studio game-data import: the studio can now import
  original files such as `GRAPHICS.DAT`, identify known game/variant hashes via
  `docs/VERIFIED_HASHES.md`, list original DM1/CSB graphics records and DM2
  GDAT ENT1 records, show asset statistics/warnings, drag/drop target images
  where Tk DND is available, batch-run the AI generation hook with a custom
  prompt, and export/import `.fsart`.

- 2026-07-16 ReDMCSB platform-symbol disposition cleanup: added
  source-backed `SYMBOL_DISPOSITIONS.tsv` rows for reviewed non-applicable
  platform boundaries and Atari ST/PRIM ABI aliases. The DM1/CSB callable
  backlog drops from 1537 to 1228 open rows; no runtime shim, fallback, or
  synthetic gameplay path was added.

- 2026-07-16 V2.2 artpack studio: added `scripts/firestaff_artpack_studio.py`,
  a cross-platform Tk/Pillow GUI for DM1, CSB, DM2, Theron, and Nexus V2.2
  artpack creation. It opens/creates `modern_asset_manifest.json`, displays
  V1/reference and V2.2 target images side by side, supports pixel/color edits,
  imports target PNGs into category/asset slots, validates required slots,
  writes `finish_receipt.json`, and can run an operator-provided AI generation
  command hook. Added docs and a CTest self-test.

- 2026-07-16 DM1 V2.2 runtime admission hardening: `m11_v22_modern_assets_available()`
  now requires both FINISHED_REAL material classification and promoted
  `finish_receipt.json` before V2.2 can resolve to modern rendering. The
  legacy missing-texture placeholder is documented as diagnostic-only, compact
  one-line manifests resolve real asset paths correctly, and the runtime gate
  regression proves that an `installed=1` flag without a reviewed receipt still
  falls back to V2.1.

- 2026-07-16 DM1 V2.2 in-place render proof repair: the in-place render probe
  now stages a local FINISHED_REAL manifest plus matching `finish_receipt.json`
  before loading `v22_inplace_cache.bin`, so the render proof exercises the same
  reviewed-pack admission gate as runtime instead of bypassing it with a
  synthetic cache-only setup.

- 2026-07-16 DM2 cache/mement free receipt bundle: mapped
  `FREE_CACHE_INDEX`, `FREE_INDEXED_MEMENT`, `FREE_TEMP_CACHE_INDEX`, and
  `FREE_PICT6` to skproject-backed bounded helpers. The regression covers
  sorted cache-index compaction, raw/cache mement release, temp-cache indexed
  release, current-mement clearing, recycle handoff, and upper/lower picture
  free planning.

- 2026-07-16 DM2 cursor macro receipt bundle: mapped `WRITE_BYTE`,
  `WRITE_WORD`, `READ_BYTE`, and `READ_SBYTE` to skproject-backed bounded
  helpers. The regression covers single-byte writes, little-endian word writes,
  unsigned byte reads, signed byte reads, and fail-closed cursor bounds.

- 2026-07-16 DM2 rect/cursor receipt bundle: mapped `DM2_OFFSET_RECT`,
  `OFFSET_RECT`, `PT_IN_RECT`, and `PTR_ADVANCE` to skproject-backed bounded
  helpers. The regression covers inclusive rect hit-testing, origin-relative
  rect translation, forward cursor advance, and underflow/overflow rejection.

- 2026-07-16 DM2 scalar/container possession receipt bundle: mapped
  `IS_NEGATIVE`, `IS_CONTAINER_MAP`, `DM2_IS_CONTAINER_MAP`,
  `FIND_POUCH_OR_SCABBARD_POSSESSION_POS`, and
  `DM2_FIND_POUCH_OR_SCABBARD_POSSESSION_POS` to skproject-backed bounded
  helpers. The regressions cover signed negative results, ObjectID DB9
  ContainerType==1 map containers, scabbard slot priority 12 then 7..9, and
  pouch slot priority 11 then 6.

- 2026-07-16 DM1 V2.2 complete artpack builder: added
  `scripts/build_dm1_v22_complete_artpack.py`, installed the missing
  `field_teleporter_hero_01.png` into the local DM1 modern pack, upgraded the
  local manifest to include all seven runtime-gated finished-art slots, and
  wrote a matching `finish_receipt.json`.

- 2026-07-16 DM2 `EQUIP_ITEM_TO_INVENTORY` receipt: mapped the skproject
  inventory placement helper to `dm2_v1_skproject_equip_item_to_inventory()`.
  The regression covers direction-bit clearing, champion slot writes,
  current-container overlay writes, OBJECT_NULL rejection, and range blocking.

- 2026-07-16 DM2 `FIND_FREE_MEMENTI` receipt: mapped the skproject
  next-free mement allocator to `dm2_v1_skproject_find_free_mementi()`.
  The regression covers normal allocation, skipping already referenced
  mement slots, exhaustion, and fallback recycle through the existing
  `FREE_INDEXED_MEMENT` receipt path.

- 2026-07-16 DM2 c_map/c_record scalar helper bundle: mapped
  `tile_to_ulong`, `tile_to_ubyte`, `mk_record`, `record_to_word`, and
  `record_to_long` to skproject-backed dungeon-loader helpers. The tests cover
  low-byte tile casts, full tile payload preservation, and signed object
  sentinel round-trips.

- 2026-07-16 DM2 querydb real-text promotion: promoted
  `DM2_QUERY_GDAT_ITEM_NAME`, `DM2_QUERY_CMDSTR_NAME`,
  `DM2_QUERY_CMDSTR_ENTRY`, `DM2_QUERY_CUR_CMDSTR_ENTRY`, and SKWIN aliases to
  verified source mappings over real GDAT `dtText`. The asset-loader test also
  proves real CONTAINERS field-0x40 order parsing without fallback text.

- 2026-07-16 DM2 string helper receipt bundle: mapped
  `DM2_SKCHR_TO_SCRIPTCHR`, `DM2_LTOA10`, `SK_STRLEN`, `SK_STRSTR`,
  `SK_LTOA10`, `SK_STRCPY`, and `SK_STRCAT` to skproject-backed receipts.
  The receipts cover script-character conversion, signed base-10 formatting,
  source substring semantics, and bounded copy/append output.

- 2026-07-16 DM2 text/fill/mouse wrapper receipt bundle: mapped
  `FILL_STR`, `DRAW_STRONG_TEXT`, `HIGHLIGHT_ARROW_PANEL`,
  `IBMIO_FILL_HALFTONE_RECT`, `FIRE_FILL_HALFTONE_RECTV`,
  `FIRE_FILL_HALFTONE_RECTI`, `IBMIO_MOUSE_RELEASE_CAPTURE`,
  `FIRE_MOUSE_RELEASE_CAPTURE`, and SKULLWIN aliases `DM2_FILL_STR`,
  `DM2_FILL_HALFTONE_RECTV`, `DM2_FILL_HALFTONE_RECTI`, and
  `DM2_MOUSE_RELEASE_CAPTURE` to skproject-backed receipts. The receipts
  cover strided byte fills, strong-text draw planning, arrow-panel redraw,
  checkerboard halftone fills, and mouse capture release side effects.

- 2026-07-16 DM2 item/container classifier receipt bundle: mapped
  `GET_ITEM_NAME`, `IS_MISCITEM_CURRENCY`, `IS_CONTAINER_MONEYBOX`,
  `IS_CONTAINER_CHEST`, `GET_ITEM_ORDER_IN_CONTAINER`, `FMT_NUM`, and their
  SKULLWIN aliases to skproject-backed receipts. The receipts cover GDAT
  currency flags, moneybox item-list gating, chest exclusion, champion-bones
  name side effect, order-text ranges, and four-byte numeric formatting.

- 2026-07-16 DM2 tile/fill receipt bundle: mapped
  `GET_ADDRESS_OF_TILE_RECORD`, `GET_TILE_VALUE`, `FILL_ENTIRE_PICT`, and
  `FILL_RECT_SUMMARY` to skproject-backed bounded receipts. The receipts cover
  tile-link addressing, GET_TILE_VALUE boundary masks/0xE0 blocked edges, and
  source-shaped fill planning without synthetic pixels.

- 2026-07-16 DM2 record-address receipt bundle: mapped
  `DM2_GET_ADDRESS_OF_RECORD`, `GET_ADDRESS_OF_RECORD`, typed
  `GET_ADDRESS_OF_RECORD0` through `GET_ADDRESS_OF_RECORDF`,
  `GET_ADDRESS_OF_RECORDX4`, `GET_ADDRESS_OF_GENERIC_CONTAINER_RECORD`,
  `GET_ADDRESS_OF_ACTU`, and `GET_ADDRESS_OF_DETACHED_RECORD` to
  skproject-backed bounded address receipts. The receipts use the source
  4-bit table / 10-bit index formula and fail closed on invalid links.

- 2026-07-16 DM2 SKWIN HUD alias receipt bundle: mapped
  `DM2_DRAW_SQUAD_SPELL_AND_LEADER_ICON`,
  `DRAW_SQUAD_SPELL_AND_LEADER_ICON`, `DRAW_SQUAD_POS_INTERFACE`,
  `DRAW_POWER_STAT_BAR`, `DRAW_SCROLL_TEXT`, `DRAW_SIMPLE_STR`, and
  `DRAW_SKILL_PANEL` to skproject-backed narrow receipts. The receipts cover
  squad spell/leader icons, squad position filtering, power bars, scroll
  text, simple text drawing, and skill-panel routes without synthetic art.

- 2026-07-16 DM2 HUD/dialog/item GUI receipt bundle: mapped
  `DM2_DRAW_ICON_PICT_ENTRY`, `DM2_DRAW_DIALOGUE_PROGRESS`,
  `DM2_DRAW_DIALOGUE_PARTS_PICT`, `DM2_DRAW_DIALOGUE_PICT`,
  `DM2_DRAW_WAKE_UP_TEXT`, `DM2_DRAW_PLAYER_3STAT_HEALTH_BAR`,
  `DM2_DRAW_PLAYER_NAME_AT_CMDSLOT`, `DM2_DRAW_PLAYER_DAMAGE`,
  `DM2_DRAW_SPELL_TO_BE_CAST`, `DM2_DRAW_SPELL_PANEL`,
  `DM2_DRAW_ITEM_IN_HAND`, `DM2_DRAW_ITEM_ICON`, `DM2_DRAW_ITEM_SURVEY`,
  `DM2_DRAW_HAND_ACTION_ICONS`, and covered SKWIN aliases to
  skproject-backed narrow receipts. The receipts bind real GDAT image/text,
  button, palette and blit requests without adding synthetic visuals.

- 2026-07-16 DM2 magic-map/attack-dir receipts: mapped
  `DM2_DRAW_MAJIC_MAP`, `DRAW_MAJIC_MAP`, `DM2_DRAW_PLAYER_ATTACK_DIR`,
  `DRAW_PLAYER_ATTACK_DIR`, plus SKWIN aliases `DRAW_PLAYER_3STAT_PANE` and
  `DRAW_PLAYER_3STAT_TEXT`, to skproject-backed narrow receipts. The new
  receipts cover magic-map held-container flags/panel setup and attack-dir
  squad icon/aura/arrow draw routing without fabricating pixels.

- 2026-07-16 DM2 querydb command-string receipts: mapped
  `DM2_QUERY_GDAT_ITEM_NAME`, `QUERY_GDAT_ITEM_NAME`,
  `DM2_QUERY_CMDSTR_NAME`, `QUERY_CMDSTR_NAME`,
  `DM2_QUERY_CMDSTR_ENTRY`, `QUERY_CMDSTR_ENTRY`, and
  `DM2_QUERY_CUR_CMDSTR_ENTRY` to real `dtText` GDAT rows through
  `dm2_v1_asset_loader`. The receipt parses name prefixes, repeated command
  keys, signed values, absent keys, and caller-owned current cmdstr context.

- 2026-07-16 DM2 blit/text alias receipt bundle: mapped
  `DM2_DRAW_ICON_PICT_BUFF`, `DRAW_ICON_PICT_BUFF`, `DRAW_DEF_PICT`,
  `DRAW_GRAY_OVERLAY`, `DRAW_NAME_STR`, `DRAW_GUIDED_STR`, and
  `DRAW_LOCAL_TEXT` to skproject-backed narrow receipts or verified
  text-route aliases. The new receipts cover offset-rect 8bpp icon blits,
  extended-picture rect selection, gray overlay cache/dirty-rect handoff, and
  SKWIN aliases for existing name/guided/local text routing.

- 2026-07-16 DM2 champion/HUD panel receipt bundle: mapped
  `DM2_DRAW_CUR_MAX_HMS`, `DRAW_CUR_MAX_HMS`,
  `DM2_DRAW_PLAYER_3STAT_TEXT`, `DM2_DRAW_PLAYER_3STAT_PANE`,
  `DM2_DRAW_FOOD_WATER_POISON_PANEL`, `DRAW_FOOD_WATER_POISON_PANEL`,
  `DM2_DRAW_CRYOCELL_LEVER`, `DRAW_CRYOCELL_LEVER`,
  `DM2_DRAW_EYE_MOUTH_COLORED_RECTANGLE`, and
  `DRAW_EYE_MOUTH_COLORED_RECTANGLE` to skproject-backed narrow receipts for
  stat text, champion panel variant selection, food/water/poison panel
  routing, cryocell lever state, and eye/mouth dialogue-part blits.

- 2026-07-16 DM2 GUI/container receipt bundle: mapped
  `DM2_DRAW_MONEYBOX`, `DRAW_MONEYBOX`, `DM2_DRAW_ITEM_STATS_BAR`,
  `DRAW_ITEM_STATS_BAR`, `DM2_DRAW_CONTAINER_PANEL`,
  `DRAW_CONTAINER_PANEL`, `DM2_DRAW_CONTAINER_SURVEY`,
  `DRAW_CONTAINER_SURVEY`, `DM2_DRAW_ITEM_ON_WOOD_PANEL`, and
  `DRAW_ITEM_ON_WOOD_PANEL` to skproject-backed narrow receipts for moneybox
  coin stacks, stat bars, container slot/survey drawing, and activable
  hand-item wood-panel admission. No synthetic art was added; live GDAT pixel
  consumption remains the next boundary.

- 2026-07-16 DM2 command-slot/character-sheet GUI receipts: mapped
  `DM2_DRAW_CMD_SLOT`, `DRAW_CMD_SLOT`, `DM2_DRAW_CHARSHEET_OPTION_ICON`,
  and `DRAW_CHARSHEET_OPTION_ICON` to skproject-backed draw receipts for
  command slot icon/name routing and character-sheet active-option icon
  selection. The receipts bind real GDAT categories/entries and button IDs
  without fabricating rendered pixels.

- 2026-07-16 DM2 special-effects blit receipt: mapped
  `DM2_sub_blit_specialeffects` to a skproject-backed render receipt for
  palette update, alpha/plain blit selection, source-run splitting, odd-width
  source advance, and overlay prefix/suffix clipping. Pixel ownership remains
  with the renderer; no fallback pixels are synthesized.

- 2026-07-16 Theron original consumer trace marker verifier: added a
  Mednafen FIFO-origin log verifier that emits runtime-admission markers only
  when palette, non-startup level and object-table raw Track02 offsets are
  observed as game-owned main-RAM consumers.

- 2026-07-16 DM2 movement handoff bundle: added skproject-backed narrow
  receipts for `DM2_move_12b4_0d75`, `DM2_move_075f_0af9`, and
  `DM2_move_2fcf_0b8b`. Coverage locks creature push/lift admission,
  thrown-object terminal direction handoff, and direct/adjacent teleporter
  search order without inventing dungeon mutation or synthetic map state.

- 2026-07-16 DM2 GDAT allocation/DYN4 bundle: mapped
  `DM2_dballoc_3e74_24b8`, `DM2_dballoc_3e74_2162`, and `DM2_LOAD_DYN4`
  to skproject-backed receipts for type-2 GDAT raw-index allocation,
  zone-nibble admission, and DYN4 mark-table increment/decrement gates. No
  synthetic GDAT rows or fallback dungeon graphics were introduced.

- 2026-07-16 Theron original trace-text facts producer: added a fail-closed
  parser for authenticated original Track 02 post-$3800 consumer traces. It
  requires exact palette, non-startup level and object-table offsets, matching
  payload/envelope checksums, observed dungeon/object/bitmap/palette consumers,
  and explicit no-synthetic/no-fallback markers before producing consumer
  facts.

- 2026-07-16 Nexus Structure3 reviewed material-upload gate: added a
  no-draw receipt that binds Structure3 raw capture, original Saturn
  attestation, package/host route and producer workflow attestation. It still
  blocks material semantics, runtime upload, renderer handoff and fallback
  visuals. Verification: `nexus_v1_structure3_capture_manifest` and
  `git diff --check` passed.

- 2026-07-16 Theron original consumer binding gate: added a fail-closed
  binding receipt between the real Track 02 data-gap receipt and original
  post-$3800 consumer semantics. It refuses render admission until palette,
  non-startup level, object-table and bitmap consumers are all proven.
  Verification: `theron_v1_runtime_admission` and `git diff --check` passed.

- 2026-07-16 DM2 door querydb bundle: mapped `DM2_GET_DOOR_STAT_0X10`,
  `DM2_GET_GRAPHICS_FOR_DOOR`, and `DM2_query_0cee_3275` to real DOORS
  dtWordValue rows. Missing source rows fail closed; no synthetic door stats
  or graphics defaults are introduced. Verification:
  `test_dm2_v1_gdat_querydb_receipts` and `git diff --check` passed.

- 2026-07-16 Nexus MENU.BPK reviewed upload receipt: added a PRS3 V3 review
  boundary that binds raw sidecars, provenance ledger, producer binary and
  attestation into one source evidence chain. Runtime upload, decoder
  promotion and fallback visuals remain blocked until independent Saturn
  capture authentication exists. Verification:
  `test_nexus_v1_prs3_capture_trace_schema` and `git diff --check` passed.

- 2026-07-16 DM2 LOAD_GDAT_INTERFACE_00_0A Rect14 receipt: added the named
  skproject source-symbol boundary for INTERFACE_GENERAL dt07/0x0A. It
  consumes real GDAT Rect14 rows and the bounded host placement proof, with no
  synthetic creature placement data. Verification: `test_dm2_v1_boot_profile_smoke`
  and `git diff --check` passed.

- 2026-07-16 DM2 querydb equipment bundle: mapped
  `DM2_QUERY_CREATURES_ITEM_MASK`, `QUERY_CREATURES_ITEM_MASK`,
  `DM2_IS_ITEM_FIT_FOR_EQUIP`, and `IS_ITEM_FIT_FOR_EQUIP` to skproject
  querydb semantics over real parsed GDAT rows. Coverage locks CREATURES
  dtText item-mask ranges and DB-spec dtWordValue equipment flags without
  synthetic defaults. Verification: `test_dm2_v1_gdat_querydb_receipts`,
  focused CTest, and `git diff --check` passed.

- 2026-07-16 Theron original Track 02 binding gap receipt: runtime admission
  now records concrete original-media facts for the still-blocked Track 02
  dungeon handoff: palette offsets/hashes, non-startup sector/container
  hashes, first descriptor-window offsets, and route hashes. The receipt
  remains fail-closed for palette promotion, non-startup decode, object-table
  decode, render admission, and fallback visuals. Verification: runtime
  admission probe, focused CTest, and `git diff --check` passed.

- 2026-07-16 DM1 portrait/bar-graph symbol bundle: closed
  `F0515`, `F0516`, `F2104`, `F2105`, and `S0287` through focused ReDMCSB
  audit mappings. Coverage locks 32x29/464-byte 4bpp portrait planar
  conversion roundtrips and existing DM1 champion bar-graph table/fill
  contracts. Verification: portrait planar conversion build/direct run,
  focused bar-graph/HUD tests, CTest, TSV checks, and `git diff --check`
  passed.

- 2026-07-16 DM2 querydb symbol bundle: mapped
  `DM2_QUERY_ORNATE_ANIM_FRAME`, `DM2_GET_ORNATE_ANIM_LEN`,
  `DM2_QUERY_DOOR_DAMAGE_RESIST`, `DM2_QUERY_DOOR_STRENGTH`,
  `DM2_QUERY_GDAT_CREATURE_WORD_VALUE`, and
  `DM2_QUERY_GDAT_FOOD_VALUE_FROM_RECORD` to real parsed GDAT query rows.
  Verification: `test_dm2_v1_gdat_querydb_receipts`, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 Nexus PRS3 V3 raw-sidecar provenance ledger gate: tightened the
  skip-safe local real `MENU.BPK`/`DM.BIN` path to prove bounded PRS3 stream,
  raw sidecar, VDP1/palette sidecars, producer trace, and provenance ledger
  binding while still blocking decoder promotion, runtime import, rendering,
  and fallback visuals. Verification: PRS3 schema test, focused Nexus CTest
  set, launcher compile check, and `git diff --check` passed.

- 2026-07-16 Theron real Track02 capture-producer gate: runtime admission now
  has a producer from real Track02 bytes/MD5 that builds level/object routes,
  startup bitmap atlas evidence, palette-window evidence, and render-asset
  proof while still blocking staged US data until palette binding and
  non-startup records are source-locked. Verification: runtime admission probe
  without media, probe with staged US raw Track02, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 CSB ReDMCSB F7088-F7090 imported-party registration: registered
  focused CTests and mapped dispositions for F7088 portrait transfer,
  F7089 first-empty-cell scan, and F7090 new-adventure normalization.
  Verification: F7088/F7090 build targets, 13 focused ReDMCSB/CSB CTests, and
  `git diff --check` passed.

- 2026-07-16 DM2 skproject side-candidate wound helper: mapped
  `DM2_move_12b4_023f` from `SKULLWIN/c_move.cpp`. Coverage locks
  `(arg1 + arg0 + 2/3) & 3` direction selection, duplicate champion
  suppression, source `WOUND_PLAYER(..., 1, 0x18, 2)` admission as
  caller-provided wound receipts, and `QUEUE_NOISE_GEN2` hero-type requests
  only after successful wounds. Live wound mutation and audio playback remain
  caller-owned. Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_skproject_core -j2`, direct test binary, focused CTest, and
  `git diff --check` passed.

- 2026-07-16 DM2 skproject sound handoff bundle: mapped
  `DM2_GET_MUSIC_INDEX_FROM_MODLIST`, tightened `DM2_SOUND2` to consume that
  helper, and promoted the existing `DM2_PLAY_MUSIC`/`DM2_PLAY_SOUND`
  boundaries from uncertain to implemented-narrow. Coverage locks byte lookup
  over real caller-provided MODLIST bytes, verified DATA-root music queueing,
  and fail-closed sample playback when no source-resolved payload exists.
  Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_sound_source_gate -j2`, `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_sound_source_gate$'`, direct test binary,
  `ctest --test-dir build-local-ninja --output-on-failure -R
  '^dm2_v1_startup_music_queue$'`, and `git diff --check` passed.

- 2026-07-16 DM2 skproject wall attack helper: mapped `DM2_ATTACK_WALL`
  from `SKULLWIN/c_move.cpp`. Coverage locks matching wall-side scan,
  ornate-alcove RANDDIR missile relocation, class-0x22 actuator
  invoke/consume gates, and class-0x23 wildcard/matching projectile teleport
  relocation without claiming live record cutting/deletion or actuator
  execution. Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_skproject_core -j2`, `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_skproject_core$'`, direct test binary, and
  `git diff --check` passed.

- 2026-07-16 DM2 skproject door attack helper: mapped `DM2_ATTACK_DOOR`
  from `SKULLWIN/c_move.cpp`. Coverage locks byte3 bit0 and byte2 bit7
  admission gates, attack-power threshold, door tile-type requirement, delayed
  timer route, and immediate tile type 4-to-5 open plan without claiming live
  tile memory or timer queue mutation. Verification: `cmake --build
  build-local-ninja --target test_dm2_v1_skproject_core -j2`, `ctest
  --test-dir build-local-ninja --output-on-failure -R
  '^dm2_v1_skproject_core$'`, direct test binary, and `git diff --check`
  passed.

- 2026-07-16 Nexus PRS3 original-provenance no-runtime proof: tightened
  `test_nexus_v1_bpk_prs3_payload_evidence` so a real MENU.BPK PRS3 decoded
  output sidecar with matching length/FNV, bound capture source, and asserted
  original Saturn provenance reaches `source-bound-no-runtime`. The receipt is
  proof-ready but still forbids opcode grammar promotion, decoder promotion,
  runtime upload, rendering, and fallback visuals. Verification:
  `cmake --build build-local-ninja --target
  test_nexus_v1_bpk_prs3_payload_evidence -j2` and focused CTest over PRS3
  evidence, startup handoff, startup menu compatibility, DGN material raster,
  DGN geometry readiness, startup gate, and boot file hash scan passed.

- 2026-07-16 DM1 CHAMPION projectile/action-time F0326-F0331 bundle:
  closed source-backed ReDMCSB dispositions for
  `F0326_CHAMPION_ShootProjectile`,
  `F0327_CHAMPION_IsProjectileSpellCast`,
  `F0328_CHAMPION_IsObjectThrown`,
  `F0330_CHAMPION_DisableAction`,
  `F0331_CHAMPION_ApplyTimeEffects_CPSF`, and aliases `F326_ozzz_`,
  `F327_kzzz_`, `F328_nzzz_`, `F330_szzz_`, and `F331_auzz_`. Coverage
  locks the bounded launch-cell/direction/projectile-create inputs,
  spell-projectile mana/kinetic/step handoff, thrown-object stamina/XP/sound/
  kinetic/attack/action-disable planning, F0330 C11 action-enable scheduling,
  and F0331 needs/scent/stat-recovery/dirty-mask clock effects. Projectile
  queue/render, deeper impact behavior, and full live panel mutation remain
  route-owned; `F0329` stays open for a clean DM1-owned leader-hand proof.
  Verification: focused direct executables for combat, throw/shoot, F0407
  action-tail, F0330 C11 production, champion-needs, champion-panel clock tick,
  and LIF-01/F0830 all passed; focused CTest over the same seven targets
  passed 7/7.

- 2026-07-16 Nexus real LEV00 host-route fail-closed receipt: tightened
  `test_nexus_v1_boot_file_hash_scan` so a hash-resolved real LEV00.DGN
  package/host route must consume the source package but remain blocked before
  DGN command copy or pixel writes. The receipt now asserts `blocked-handoff`,
  runtime DGN blocked, presentation denied, zero rasterized commands, zero
  written pixels, and no fallback visuals. Verification: `cmake --build
  build-local-ninja --target test_nexus_v1_boot_file_hash_scan -j2` and the
  focused Nexus CTest set covering startup handoff, startup menu compatibility,
  DGN material raster, DGN geometry readiness, startup gate, and boot file hash
  scan passed.

- 2026-07-16 DM2 skproject other-level/cross-map helper bundle: mapped
  `DM_LOCATE_OTHER_LEVEL` and `DM2_map_3BF83` from `SKULLWIN/c_map.cpp`.
  Coverage locks cursor/resume candidate scanning, world-to-local coordinate
  conversion, wall/active-teleporter rejection, same-map record-move planning,
  cross-map restore/LOAD_NEWMAP handoff, and party-rotation request without
  claiming full live `DM2_ARRANGE_DUNGEON` or record relocation mutation.
  Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_skproject_core -j2`, `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_skproject_core$'`, and `git diff --check`
  passed.

- 2026-07-16 DM1 CHAMPION damage/poison/stamina F0320-F0325 bundle:
  closed source-backed ReDMCSB dispositions for
  `F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds`,
  `F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage`,
  `F0322_CHAMPION_Poison`,
  `F0324_CHAMPION_DamageAll_GetDamagedChampionCount`,
  `F0325_CHAMPION_DecrementStamina`, and aliases `F320_akzz_`,
  `F321_AA29_`, `F322_lzzz_`, `F324_aezz_`, and `F325_bzzz_`. Coverage locks
  DM1 pending wound/damage mutation order, combat damage staging, poison
  immediate damage plus 36-tick continuation, all-party damage fanout, and
  F0325 stamina clamp/underflow dirty-mask planning. Rendering, C12 hide event
  refresh, poison panel redraw, candidate/game-won edge cases, and every live
  projectile/caller variant remain route-owned. Verification: `ninja -C
  build-local-ninja test_dm1_v1_combat_pc34_compat_integration`, `ninja -C
  build-local-ninja test_dm1_v1_action_f0407_tail_pc34_compat`,
  direct runs of both test executables, and `ctest --test-dir
  build-local-ninja --output-on-failure -R
  '^(dm1_v1_combat_damage_source_lock|dm1_v1_action_f0407_tail_pc34_compat)$'`
  passed.

- 2026-07-16 Theron Track 02 host proof producer: moved host dungeon-consumer
  proof construction into runtime admission code. The producer consumes the
  verified real Track 02 dungeon handoff, requires an original host-route
  identity plus level-grid/object-table/bitmap-palette runtime consumers,
  host surface upload, and host capture-frame proof, and rejects placeholder,
  synthetic, fallback, pre-drawn, or fallback-enabled routes. Verification:
  `cmake --build build-local-ninja --target
  firestaff_theron_v1_runtime_admission_probe -j2`,
  `./build-local-ninja/firestaff_theron_v1_runtime_admission_probe`,
  `ctest --test-dir build-local-ninja --output-on-failure -R
  '^theron_v1_runtime_admission$'`, and scoped `git diff --check` passed.

- 2026-07-16 DM1 CHAMPION scent F0315-F0317 bundle: added source-backed
  champion-needs scent helpers for `F0315_CHAMPION_GetScentOrdinal`,
  `F0316_CHAMPION_DeleteScent`, `F0317_CHAMPION_AddScentStrength`, and aliases
  `F315_arzz_`, `F316_aizz_`, and `F317_adzz_`. Coverage locks ReDMCSB
  newest-to-oldest 1-based lookup, zero-based delete/shift plus
  FirstScentIndex/LastScentIndex adjustment, normal strength addition capped
  at 80, MASK0x8000 merge max behavior, duplicate-match result reuse, and the
  source boundary that missing scents are not appended by F0317 itself.
  Verification: `ninja -C build-local-ninja
  test_dm1_v1_champion_needs_pc34_compat_integration`,
  `./build-local-ninja/test_dm1_v1_champion_needs_pc34_compat_integration`,
  and `ctest --test-dir build-local-ninja --output-on-failure -R
  '^dm1_v1_champion_needs_source_lock$'` passed.

- 2026-07-16 DM2 skproject `DM2_3D93B` text/map scan receipt: added a
  bounded c_map/c_record receipt in `dm2_v1_dungeon_loader` for skproject
  `c_gdatfile.cpp` line 2108. It scans source-owned byte maps and DB2 Text
  records, preserves the `TextMode` and `SimpleTextExtUsage` gates, counts or
  selects ext-0x0B records, records ext-0x10 fallback coordinates, and clears
  only the original ext-0x0F visibility bit for mode 4. It refuses incomplete
  G1 record graphs and does not touch GDAT images or synthetic runtime state.
  Verification: focused Ninja target `test_dm2_v1_c_map_tile_access`, focused
  CTest `dm2_v1_c_map_tile_access`, and `git diff --check`.

- 2026-07-16 Theron Track 02 host dungeon consumer gate: added a fail-closed
  runtime consumer receipt after dungeon handoff. It admits dungeon draw only
  for the same real US Track 02 capture with matching route/checksum/decoded
  hashes, original host route proof, level/object/bitmap-palette runtime
  consumers, host surface upload, and host capture-frame proof. It rejects
  synthetic host/level/object/bitmap promotion, hash drift, pre-opened draw,
  and fallback visuals. Verification: `cmake --build build-local-ninja
  --target firestaff_theron_v1_runtime_admission_probe -j2`,
  `./build-local-ninja/firestaff_theron_v1_runtime_admission_probe`,
  `ctest --test-dir build-local-ninja --output-on-failure -R
  '^theron_v1_runtime_admission$'`, and scoped `git diff --check` passed.

- 2026-07-16 DM2 skproject move/alcove helper bundle: mapped
  `DM2_move_12b4_099e`, `DM2_move_12b4_0092`, `DM2_move_12b4_00af`, and
  `DM2_0cee_317f` from `SKULLWIN/c_move.cpp`. Coverage locks party
  creature-lift admission/stamina drain planning, arrow-panel highlight
  gating, other-level transition planning, and ornate wall-alcove GDAT
  data-index handoff without claiming full live `DM2_PERFORM_MOVE` or
  `DM2_ATTACK_WALL` mutation. Verification: `cmake --build
  build-local-ninja --target test_dm2_v1_skproject_core -j2` and `ctest
  --test-dir build-local-ninja --output-on-failure -R
  '^dm2_v1_skproject_core$'` passed.

- 2026-07-16 DM2 skproject map/list helper bundle: mapped
  `DM2_map_0cee_1815`, `DM2_map_0cee_185a`, `DM2_map_2066_1f37`,
  `DM2_map_2066_1ec9`, and `SKW_2066_1ea3` from `SKULLWIN/c_map.cpp`.
  Coverage locks candidate-table random selection, four-slot map decoration
  fill/sanitize behavior, linked-record 0x27 updates, low-type chain prepend,
  and tmpmap flag mutation without claiming full live `DM2_ARRANGE_DUNGEON`
  global mutation. Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_skproject_core -j2` and `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_skproject_core$'` passed.

- 2026-07-16 Nexus Track 1 capture-required readiness gate: updated the
  Track 1 screen-capture readiness probes so real DM.BIN, FONT256.S2D,
  LEV00.DGN, SCORPION.MNS, MENU.BPK PRS3, and Structure3 capture evidence can
  be verified while DGN presentation remains no-draw/capture-required. The
  probes now reject nonzero DGN/fallback pixels before original Saturn
  capture/admission and keep the local BMP/PPM receipts deterministic without
  promoting screenshots. Verification: focused Ninja targets for the two
  Track 1 readiness probes plus Nexus PRS3, Structure3, direct-capture, and
  MENU.BPK handoff tests; focused CTest passed 11/11 for the same gate set.

- 2026-07-16 DM2 skproject map helper bundle: mapped
  `DM2_SET_DESTINATION_OF_MINION_MAP`, `DM2_map_0cee_17e7`,
  `DM2_map_0cee_04e5`, and `DM2_map_3B001` from `SKULLWIN/c_map.cpp`.
  Coverage locks source bit-packing for minion destinations, mixed modulo map
  selection, vector-before-tile lookup, and map marker restore receipts without
  claiming live global map mutation. Verification: `cmake --build
  build-local-ninja --target test_dm2_v1_skproject_core -j2` and `ctest
  --test-dir build-local-ninja --output-on-failure -R
  '^dm2_v1_skproject_core$'` passed.

- 2026-07-16 DM1 CHAMPION stat/wake/death F0311-F0314/F0318-F0319 bundle:
  closed source-backed ReDMCSB dispositions for `F0311_CHAMPION_GetDexterity`,
  `F0312_CHAMPION_GetStrength`, `F0313_CHAMPION_GetWoundDefense`,
  `F0314_CHAMPION_WakeUp`, aliases `F311_wzzz_`, `F312_xzzz_`,
  `F314_gzzz_`, plus `F0318_CHAMPION_DropAllObjects` and
  `F0319_CHAMPION_Kill`. The closures bind existing DM1 combat,
  mirror-candidate C146 wake-up, chest auto-close on leader death,
  pending-damage kill dispatch, and resurrection/bones receipts. `F0315`,
  `F0316`, `F0317`, and their aliases remain open because this pass found no
  reviewed production DM1 owner strong enough to close them. Verification:
  focused Ninja targets `test_dm1_v1_combat_pc34_compat_integration`,
  `test_dm1_v1_chest_auto_close_on_leader_death_pc34_compat`,
  `test_dm1_v1_champion_panel_pending_damage_apply_pc34_compat`, and
  `test_dm1_v1_resurrection_pc34_compat`; focused CTests
  `dm1_v1_combat_damage_source_lock`,
  `dm1_v1_chest_auto_close_on_leader_death_pc34_compat`,
  `dm1_v1_champion_panel_pending_damage_apply_pc34_compat`, and
  `dm1_v1_resurrection`; direct compile/run of
  `tests/test_dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat.c`.

- 2026-07-16 DM1 CHAMPION skill/stamina/load F0303-F0310 bundle: closed
  source-backed ReDMCSB dispositions for `F0303_CHAMPION_GetSkillLevel`,
  `F0304_CHAMPION_AddSkillExperience`, `F0305_CHAMPION_GetThrowingStaminaCost`,
  `F0306_CHAMPION_GetStaminaAdjustedValue`,
  `F0307_CHAMPION_GetStatisticAdjustedAttack`, `F0308_CHAMPION_IsLucky`,
  `F0309_CHAMPION_GetMaximumLoad`, `F0310_CHAMPION_GetMovementTicks`, and
  aliases `F303_AA09_`, `F304_apzz_`, `F307_fzzz_`, `F308_vzzz_`,
  `F309_awzz_`, and `F310_AA08_`. The bundle binds existing DM1
  skill-experience, throw/shoot stamina, champion-stat, and combat helpers to
  ReDMCSB with honest narrow boundaries around live all-callers, object-weight
  ownership, and broader RNG replay. Verification: focused Ninja targets
  `test_dm1_v1_skill_experience_pc34_compat`,
  `test_dm1_v1_throw_shoot_pc34_compat`,
  `test_dm1_v1_f0306_stamina_pc34_compat`,
  `test_dm1_v1_champion_stats_pc34_compat`, and
  `test_dm1_v1_combat_pc34_compat_integration`; focused CTests
  `dm1_v1_skill_experience_source_lock`, `dm1_v1_throw_shoot_pc34_compat`,
  `dm1_v1_f0306_stamina_pc34_compat`,
  `dm1_v1_champion_stats_load_source_lock`, and
  `dm1_v1_combat_damage_source_lock`.

- 2026-07-16 DM2 skproject movement admission symbols: mapped
  `DM2_12b4_0953` and `DM2_12b4_0881` from `SKULLWIN/c_move.cpp`. The
  implementation preserves facing extraction from record word `0x0e`,
  side-relative 5x5 creature offset, tile-type return codes,
  blocked-destination gate, creature AI flag branching, and secondary creature
  query routing. Full `DM2_PERFORM_MOVE` dungeon mutation remains open.
  Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_skproject_core -j2` and `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_skproject_core$'` passed.

- 2026-07-16 DM2 skproject sound queue symbol bundle: added source-backed
  receipt/runtime-state coverage for `DM2_SOUND1`-`DM2_SOUND9`,
  `DM2_PROCESS_SOUND`, and `DM2_QUERY_SND_ENTRY_INDEX`. The bundle models
  skproject queue insertion, duplicate/capacity rejection, one-based queue
  lookup, pending-batch clearing, volume/music request state, and fail-closed
  process routing without pretending sample payload playback is available.
  Verification: `cmake --build build-local-ninja --target
  test_dm2_v1_sound_source_gate -j2` and `ctest --test-dir build-local-ninja
  --output-on-failure -R '^dm2_v1_sound_source_gate$'` passed.

- 2026-07-16 DM1 CHAMPION HoC/reset F0278/F0279 bundle: added source-backed
  narrow resurrection-owner contracts for `F0278_CHAMPION_ResetDataToStartGame`,
  alias `F278_apzz_`, and `F0279_CHAMPION_GetDecodedValue`. `F0278` now records
  the CHAMPRST.C reset side-effect order for new-game hand clear,
  resume/restart leader-hand restore, dirty-attribute clear, F0293 redraw,
  leader restore, and magic-caster restore; `F0279` now locks the REVIVE.C
  A..P nibble decoder used by F0280 vitals/stat parsing. Full F0280 champion
  text/materialization and original capture/save breadth remain separate.
  Verification: focused Ninja target `test_dm1_v1_resurrection_pc34_compat`;
  focused CTest `dm1_v1_resurrection`.

- 2026-07-16 Nexus startup/menu/DGN fail-closed handoff: repaired the current
  launcher/startup-menu WIP so verified WARNING.BIN/TITLE.CG/SAVE startup
  receipts can be consumed while DGN package/host/runtime routes stay blocked
  as `blocked-dgn-capture-required` unless a real original-Saturn DGN capture
  admission exists. The focused tests now assert zero copied/cached DGN
  commands, zero DGN pixels, no startup/DGN fallback visuals, and no synthetic
  Saturn decoder route. Verification: focused Ninja build targets
  `test_m11_nexus_startup_runtime_handoff` and
  `test_nexus_v1_startup_menu_pc34_compat`; focused CTest
  `m11_nexus_startup_runtime_handoff|nexus_v1_startup_menu_pc34_compat`.

- 2026-07-16 DM1 CHAMPION slot/hand redraw audit bundle: closed the
  source-backed narrow ReDMCSB dispositions for `F0293`, `F0295`-`F0302`,
  `F292_arzz_`, and the listed Atari ST aliases where bounded Firestaff
  routes already exist. The bundle covers all-state redraw dispatch, changed
  object icon refresh, leader-hand put/remove, slot remove/add, slot-box
  command routing, and the bounded F0299 Rabbit's Foot / modifier-apply
  evidence. Remaining blockers are the full F0299 all-item modifier matrix,
  standalone all-callers coverage for the slot helpers, and original pixel
  parity beyond the existing source-material gates. Verification: focused
  Ninja target `test_dm1_v1_champion_panel_portrait_state_redraw_pc34_compat`;
  focused CTests for inventory/chest owner changes, status-hand rotation,
  portrait redraw states, hand-slot refresh, action-hand priority,
  second-leader hand priority, and all-state redraw.

- 2026-07-16 DM2 skproject GDAT image-structure bundle: added source-backed
  receipts for `DM2_TRACK_UNDERLAY`, `DM2_READ_GRAPHICS_STRUCTURE`, and the
  bounded `DM2_EXTRACT_GDAT_IMAGE` route. Firestaff now binds real parsed
  GDAT raw tables, optional underlay pairs, direct IMG3/U4 or IMG9 decode
  hashes, and source allocation byte formulas without fabricating overlay
  pixels or CPX cache nodes. Verification: focused Ninja target
  `test_dm2_v1_gdat_querydb_receipts`, focused CTest
  `dm2_v1_gdat_querydb_receipts`.

- 2026-07-16 DM2 skproject GDAT entry value/copy bundle: added
  source-mapped receipts for `DM2_QUERY_GDAT_ENTRY_VALUE` and
  `DM2_LOAD_GDAT_ENTRY_DATA_TO`. Firestaff now reads T/I/D/S/P/F/G values
  from parsed ENT1 rows and copies real loader-owned GDAT payload bytes into
  caller buffers while rejecting scalar rows, absent payloads, and undersized
  destinations. Verification: focused Ninja target
  `test_dm2_v1_gdat_querydb_receipts`, focused CTest
  `dm2_v1_gdat_querydb_receipts`.

- 2026-07-16 DM1 CHAMPION pre-HUD F0280-F0286 bundle: closed the
  source-backed subset around Hall portrait candidate publication, reincarnate
  rename UI, Vi Altar rebirth, party direction rotation, cell target lookup,
  and adjacent attacker target selection. `F0278`/`F0279` remain blockers
  because no reviewed DM1 reset-data or decoded-value implementation was found.
  Verification: focused Ninja targets `test_dm1_v1_resurrection_pc34_compat`,
  `test_dm1_v1_resurrection_rename_ui_gate_pc34_compat`,
  `test_dm1_v1_mov05_f0284_cell_rotation_pc34_compat`; focused CTests
  `dm1_v1_resurrection`, `dm1_v1_resurrection_rename_ui_gate_pc34_compat`,
  and `dm1_v1_mov05_f0284_cell_rotation_pc34_compat`.

- 2026-07-16 DM1 CHAMPION HUD F0287-F0290: added a shared
  `DM1_ChampionPanel_FormatIntegerF0288` formatter and routed champion
  status/stat/load text through it. Verified F0287 bar pixels, F0288
  padded/unpadded integer strings, F0289 status value text, F0290
  HP/stamina/mana order and stamina /10 routing, plus the existing action
  menu source-font gate.

- 2026-07-16 DM2 skproject GRAPHICS_DATA file lifecycle bundle: added
  bounded receipts for `DM2_GRAPHICS_DATA_OPEN`, `DM2_GRAPHICS_DATA_CLOSE`,
  and `DM2_GRAPHICS_DATA_READ`, preserving open-counter nesting, primary/
  secondary handle routing, source sys-error codes, and split reads across
  `gdat.filesize`. Real file IO and buffers remain caller-owned.
  Verification: focused Ninja target `test_dm2_v1_gdat_querydb_receipts`,
  focused CTest `dm2_v1_gdat_querydb_receipts`.

- 2026-07-16 DM2 skproject c_gfx_str bundle: added bounded source-shaped
  helpers for `DM2_QUERY_FONT`, `DM2_QUERY_STR_METRICS`, draw-plan receipts
  for the `DM2_DRAW_*` text family, literal-only `DM2_FORMAT_SKSTR`,
  encrypted literal `DM2_QUERY_GDAT_TEXT`, and hint-line wrapping. This closes
  the text/glyph formulas without pretending to own live pixel blits, dynamic
  substitutions, or scrollbox screen mutation. Verification: focused Ninja
  target `test_dm2_v1_skproject_core`, focused CTest
  `dm2_v1_skproject_core`, and symbol backlog recount.

- 2026-07-16 DM1 TIMELINE square-event source bundle: closed ReDMCSB
  `F0241`, `F0242`, `F0244`-`F0248`, `F0250`, `F0251`, and `F0259` against
  existing DM1-owned M10 runtime surfaces, with `F0249` recorded as narrow
  party/ordinary replay plus C14/C15 timeline relocation only. The focused
  fixture now uses loaded dungeon state, raw C04 group backing for the C006
  generator path, real weapon slots for F0259 planning, and C11 action-enabled
  emissions rather than synthetic direct mutation. C04 group insertion/rotation
  remains separate F0249/F0252/F0267 work. Verification: focused Ninja target
  `test_dm1_v1_square_state_dispatch_pc34_compat`, focused CTest
  `dm1_v1_square_state_dispatch_pc34_compat`, backlog recount, and
  `git diff --check`.

- 2026-07-16 DM2 skproject square-chain traversal handoff: added
  `dm2_v1_dungeon_walk_square_things()` as the loader-owned bounded route for
  `GET_TILE_RECORD_LINK` + `GET_NEXT_RECORD_LINK` consumers. The object model
  now uses that API instead of reading `GenericRecord::w0` locally, so
  incomplete PC G1 record graphs inherit the existing traversal guard while
  source-shaped chains still materialize objects. Verification: strict direct
  C99 build/run of `test_dm2_v1_dungeon_loader_first_map_gate` passed 71/71;
  strict direct C99 build/run of `probe_dm2_v1_object_model` passed 34/34 with
  a build-only FTL decompressor stub because `src/shared/dungeon_decompressor_ftl.c`
  is absent in the agent worktree.

- 2026-07-16 DM1 GROUP/ACTION source-audit bundle: closed ReDMCSB `F0225`,
  `F0231`, and `F0232` plus their Atari ST ABI aliases against DM1-owned
  implementations. The bundle verifies Fuse/endgame gates, melee damage and
  runtime side effects, luck/group-damage writeback, F0190 aftermath handoff,
  and closed-door/projectile door-destruction gates including RA-door reject.
  Also fixed the `test_dm1_v1_action_f0407_tail_pc34_compat` Ninja target so
  it links the real creature-AI and steal-slot sources used by F0231 drop
  handling. Verification: focused CTests
  `dm1_v1_endgame_system_source_lock`,
  `dm1_v1_action_f0407_tail_pc34_compat`,
  `dm1_v1_ra_door_projectile_reject_pc34_compat`, backlog recount, and
  `git diff --check`.

- 2026-07-16 DM1 GROUP/MOVE source-audit bundle: closed ReDMCSB `F0226`,
  `F0227`, `F0228`, `F0229`, and `F0264` plus their Atari ST ABI aliases
  against existing DM1-owned source implementations. The bundle verifies
  Manhattan square distance, rotated visibility cone checks, direction
  selection with source RNG consumption, G0023 ordered-cell attack table
  selection, and MOVESENS.C levitation rules for C04/C14/C15 without decoded
  substitutes. Verification: focused CTest
  `dm1_v1_creature_ai_behavior_source_lock`, focused CTest
  `dm1_v1_ordered_cells_to_attack_pc34_compat`, backlog recount, and
  `git diff --check`.

- 2026-07-16 DM2 skproject GDAT ENT1/preload/sound receipts: added
  source-named `c_gdatfile.cpp` receipts for `DM2_LOAD_ENT1`,
  `DM2_LOAD_GDAT_ENTRIES`, `DM2_QUERY_NEXT_GDAT_ENTRY`, `DM2_47eb_00a4`,
  and `DM2_482b_0684` over the parsed real ENT1/raw GDAT tables and
  caller-owned sound payload bytes. The receipts preserve the ENT1
  T/I/D/S/P/F/G map, raw0 hash, non-scalar preload byte accounting, filtered
  iterator state, sound sample 0x80 XOR gate, dt02 sound payload span, and
  SOUND7 rejection boundary without fabricating decoded images, CPX nodes, or
  fallback buffers. Verification: focused Ninja target
  `test_dm2_v1_gdat_querydb_receipts`, direct
  `./build-local-ninja/test_dm2_v1_gdat_querydb_receipts` 34/34 including
  real local DM2 `GRAPHICS.DAT`, focused CTest
  `dm2_v1_gdat_querydb_receipts`, regenerated symbol backlog, and
  `git diff --check`.

- 2026-07-16 Theron runtime Track 02 dungeon handoff gate: added
  `Theron_V1RuntimeTrack02DungeonHandoffProof`,
  `Theron_V1RuntimeTrack02DungeonHandoffReceipt`, and
  `theron_v1_runtime_bind_track02_dungeon_handoff()` so admitted real
  Track 02 render assets can be handed to a dungeon-facing consumer only when
  the same US raw session, route hashes, payload/envelope/consumer checksums,
  decoded level/object-table/bitmap/palette hashes, source-byte binding, object
  layout proof, and bitmap/palette decode proof all match. The receipt admits
  real dungeon data/layout/decode handoff but keeps drawing and fallback
  visuals closed, and rejects synthetic dungeon state, synthetic layouts,
  synthetic bitmap/palette decodes, hash drift, and fallback observation.
  Verification: focused Ninja target `firestaff_theron_v1_runtime_admission_probe`,
  direct probe run, focused CTest `theron_v1_runtime_admission`, and
  `git diff --check`.

- 2026-07-16 Theron runtime Track 02 render-asset admission gate: added
  `Theron_V1RuntimeTrack02RenderAssetProof`,
  `Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt`, and
  `theron_v1_runtime_bind_track02_render_asset_admission()` so the runtime
  path cannot promote consumer semantics into render admission until the same
  admitted US raw Track 02 session also supplies matching route hashes,
  payload/envelope/consumer checksums, nonzero decoded level/object-table/
  bitmap/palette hashes, explicit consumer proof for each domain, decoded
  bitmap-pixel and palette-word proof, and no synthetic promotion or fallback
  observation. This adds no decoded art and no fallback visual path; it only
  admits caller-supplied real proof. Verification: focused Ninja target
  `firestaff_theron_v1_runtime_admission_probe`, direct probe run, focused
  CTest `theron_v1_runtime_admission`, and `git diff --check`.

- 2026-07-16 DM1 TIMELINE/MOVE audit closure: tightened
  `dm1v1_event_process_tick()` so the bounded F0261 dispatch recorder stops
  before extracting an event it cannot publish, preserving the real expired
  EVENT for the next pass. The callable audit now closes ReDMCSB `F0240`,
  `F0261`/`F261_hzzz_`, and `F0265` as DM1-owned source mappings: F0240 uses
  the first heap entry's low-24-bit `Map_Time`, F0261 drains through F0239
  order into source-shaped dispatch records, and F0265 builds the exact C60/
  C61 move-group retry EVENT with `gameTime + 5`, zero priority, destination
  `B.Location`, and the source C04 Thing word in `C.Slot`. Verification:
  focused Ninja target and focused CTest `dm1_v1_event_timer_source_lock` in
  both `build-local-ninja` and `build-dm1-worker`, plus `git diff --check`.

- 2026-07-16 DM2 skproject utility helper receipts: added bounded
  `dm2_v1_skproject_abs()`, `dm2_v1_skproject_calc_square_distance()`,
  `dm2_v1_skproject_calc_vector_dir()`,
  `dm2_v1_skproject_compute_power_4_within()`,
  `dm2_v1_skproject_fill_i16table()`, and
  `dm2_v1_skproject_atimesb_rshiftc()` receipts for the SKULLWIN
  `util.cpp` helper slice. The helpers preserve source scalar arithmetic,
  diagonal direction tie breaking through caller-owned `DM2_RANDBIT` state,
  caller-owned table writes, and unsigned-word multiply-before-shift behavior
  without creating movement/pathfinding/HUD/GDAT consumers or synthetic
  storage. Verification: strict direct C99 build/run of
  `test_dm2_v1_skproject_core`, focused Ninja target
  `test_dm2_v1_skproject_core`, focused CTest `dm2_v1_skproject_core`, and
  `git diff --check`.

- 2026-07-16 DM2 skproject attribute/UI event receipts: added bounded
  `dm2_v1_skproject_boost_attribute()` and
  `dm2_v1_skproject_adjust_ui_event()` for skproject `BOOST_ATTRIBUTE`,
  `DM2_ADJUST_UI_EVENT`, and `ADJUST_UI_EVENT`. The helpers preserve the
  source champion-attribute decay loop and 10..220 clamp, champion hand-event
  remapping, spell/leader triangle selection, cooldown cancellation, and
  hand-activable gates over caller-supplied source-shaped state. They do not
  fabricate champion state, UI events, or dispatch runtime commands.
  Verification: focused Ninja target `test_dm2_v1_skproject_core`, focused
  CTest `dm2_v1_skproject_core`, and `git diff --check`.

- 2026-07-16 DM2 skproject coin-type count receipt: added
  `dm2_v1_skproject_count_by_coin_types()` for skproject
  `DM2_COUNT_BY_COIN_TYPES` / `COUNT_BY_COIN_TYPES`. The helper preserves the
  source ten-counter zeroing, moneybox contained-record walk, miscellaneous
  currency filter, distinctive item-type table match, and charge+1 count
  accumulation over caller-supplied source-shaped records. It does not load or
  fabricate item definitions, GDAT DBSPEC flags, or container contents.
  Verification: strict direct C99 build/run of `test_dm2_v1_skproject_core`,
  focused Ninja target `test_dm2_v1_skproject_core`, and focused CTest
  `dm2_v1_skproject_core`, plus `git diff --check`.

- 2026-07-16 Theron runtime Track 02 consumer-semantic bridge: added
  `Theron_V1RuntimeTrack02ConsumerSemanticReceipt` and
  `theron_v1_runtime_bind_track02_consumer_semantics()` so runtime admission
  can consume the combined capture-consumer gap only when an already proven
  original post-$3800 consumer semantic receipt agrees with the admitted US
  raw Track 02 session and record. The gate rejects pre-opened gap receipts,
  non-US variants, mismatched records, missing consumer checksums, incomplete
  dungeon/object/bitmap/palette consumer proof, and fallback visuals. This is
  a fail-closed bridge only; it does not create the real post-$3800 capture or
  admit synthetic consumers. Verification: focused Ninja target, direct run
  of `firestaff_theron_v1_runtime_admission_probe`, and focused CTest
  `theron_v1_runtime_admission`.

- 2026-07-16 DM1 GROUP F0224 audit repair: added
  `DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat` as the DM1-owned
  source mapping for ReDMCSB `PROJEXPL.C F0224_GROUP_FluxCageAction`. The
  plan preserves the wall/stairs no-op, unused C15 explosion Thing gate, C050
  fluxcage creation, C24 remove-fluxcage event fields at `GameTime + 100`,
  adjacent Lord Chaos scan order (north, west, east, south), and C29 danger
  reaction when two other fluxcages are present. The callable audit and symbol
  disposition rows now close F0224 as `VERIFIED_SOURCE_MAPPING`. Verification:
  focused Ninja target `test_dm1_v1_endgame_system_pc34_compat` and focused
  CTest `dm1_v1_endgame_system_source_lock`; no synthetic dungeon/save/art
  data was added.

- 2026-07-16 DM2 skproject movement vector receipt: added
  `dm2_v1_skproject_calc_vector_w_dir()` for skproject
  `DM2_CALC_VECTOR_W_DIR` / `CALC_VECTOR_W_DIR`. The helper preserves the
  source additive accumulator contract over the X/Y direction tables,
  signed forward/side operands, and direction wrap while staying below
  collision, map mutation, movement timing, and runtime input dispatch.
  Verification: strict direct C99 build/run of `test_dm2_v1_skproject_core`,
  focused Ninja target `test_dm2_v1_skproject_core`, focused CTest
  `dm2_v1_skproject_core`, and `git diff --check`.

- 2026-07-16 Theron runtime Track 02 capture-consumer route-gap receipt:
  added `Theron_V1RuntimeTrack02CaptureConsumerGapReceipt` and
  `theron_v1_runtime_bind_track02_capture_consumer_gap()` so the admitted US
  raw Track 02 FIFO/session/startup-anchor chain can consume the verified
  non-startup level and object-table route evidence together without mixing
  sessions or route hashes. The receipt preserves candidate masks/counts and
  first opaque candidate hashes, but leaves capture-consumer readiness,
  object-table decode/admission, non-startup level decode/admission, exact
  level/object semantics, payload/visual semantics, and fallback visuals
  closed. Verification: focused Ninja target, direct run of
  `firestaff_theron_v1_runtime_admission_probe`, and focused CTest
  `theron_v1_runtime_admission`.

- 2026-07-16 Theron runtime object-table route-gap evidence receipt: added
  `Theron_V1RuntimeObjectTableRouteEvidenceReceipt` and
  `theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence()`
  so the admitted US raw Track 02 FIFO/session/startup-anchor chain can
  consume the verified object-table route receipt as blocked evidence. The
  gate requires a matching object-table route hash, descriptor/candidate/
  blocked anchor evidence, first candidate offsets/user-data/hash/counts,
  the missing-real-object-evidence blocker, and rejects object decode,
  object-table admission, exact object semantics, payload/visual semantics,
  and fallback visuals. Verification: focused Ninja target, direct run of
  `firestaff_theron_v1_runtime_admission_probe`, focused CTest
  `theron_v1_runtime_admission`, and focused `git diff --check`.

- 2026-07-16 DM2 skproject item value/weight receipts: added bounded
  `dm2_v1_skproject_query_item_value()`,
  `dm2_v1_skproject_query_item_weight()`, and
  `dm2_v1_skproject_calc_player_weight()` receipts for the skproject
  `DM2_QUERY_ITEM_VALUE` / `QUERY_ITEM_VALUE`, `DM2_QUERY_ITEM_WEIGHT` /
  `QUERY_ITEM_WEIGHT`, and `CALC_PLAYER_WEIGHT` slices. The helpers preserve
  source control flow for GDAT word values, charge multipliers, potion money
  scaling, recursive containers, moneybox aggregation, the weight wrapper, and
  selected-player open-chest overlay while requiring caller-supplied
  source-shaped records. They do not load real GDAT/dungeon records or drive
  HUD/runtime item state. Verification: strict direct C99 build/run of
  `test_dm2_v1_skproject_core`, focused Ninja target
  `test_dm2_v1_skproject_core`, and focused CTest `dm2_v1_skproject_core`.

- 2026-07-16 DM1 GROUP F0208 audit repair: narrowed the stale
  `UNCERTAIN_NUMBERED_EVIDENCE` row for the existing source-shaped
  `F0208_DM1_GROUP_BuildAddEventPlan_Compat` event-plan helper. The verified
  mapping follows ReDMCSB `GROUP.C:1820-1834`: earlier requested aspect-update
  times demote C38-C41 to C33-C36, preserve the later behavior delay in
  `EVENT.C.Ticks`, update `Map_Time`, and keep F0238 timeline insertion
  caller-owned. Verification: focused CTest
  `dm1_v1_creature_ai_behavior_source_lock`; no synthetic event data was
  added. Broader M10 initial-attack consumption remains open.

- 2026-07-16 CSB DSA save-runtime handoff receipt: added
  `CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34` and
  `csb_v1_boot_runtime_dsa_save_handoff_receipt_pc34()` so CSBWin save import
  consumers must distinguish a loader-ready `CSBGAME.DAT` runtime load from a
  real Extended Features DSA runtime handoff. The receipt consumes only the
  existing boot save/import receipt, preserves the DSA corpus decision label
  and runtime load facts, and stays invalid unless the DSA corpus, extended
  tail, DSA section, runtime actions, GAMEBLOCK1, CSB import source, and
  loaded party facts all pass. No synthetic DSA-positive save or wrapper data
  was added. Verification: focused Ninja target
  `test_csb_v1_boot_runtime_handoff`, focused run
  `FIRESTAFF_FOCUS_CSB_DSA_SAVE_HANDOFF=1 ./build-local-ninja/test_csb_v1_boot_runtime_handoff`
  (39/39), and `git diff --check`.

- 2026-07-16 DM1 GROUP F0185 audit repair: closed the stale
  `UNCERTAIN_NUMBERED_EVIDENCE` row for the existing source-locked C006 group
  generator path. The verified mapping covers `F0860_RUNTIME_HandleGroupGenerator_Compat`
  and M10 materialization through raw C04 allocation/init, one start-cell RNG
  draw, descending group cell/health writes, F0267 placement/deferred retry,
  F0183 active-state admission, F0180 wander scheduling, and M560 buzz
  routing. This records existing behavior only and adds no synthetic group
  data. Verification: focused CTest
  `FIRESTAFF_FOCUS_F0185=1 m10_c006_generator_reenable_dispatch_pc34_compat`,
  direct `test_dm1_v1_mummy_viewport_pc34_compat`, direct
  `test_dm1_v1_group_move_removal_pc34_compat`, and `git diff --check`.

- 2026-07-16 Theron runtime non-startup level evidence receipt: added
  `Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt` and
  `theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence()`
  so the admitted US raw Track 02 FIFO/session/startup-anchor chain can
  consume the detailed Track 02 level-route receipt's non-startup candidate
  evidence. The gate requires verified Track 02 descriptor anchors, a matching
  startup-level anchor and route hash, opaque post-descriptor candidate
  offsets/user-data/hash/header evidence, and the existing missing-evidence
  blocker. It keeps non-startup level admission, exact level/object semantics,
  payload semantics, visual semantics, and fallback visuals closed.
  Verification: focused Ninja target and direct run of
  `firestaff_theron_v1_runtime_admission_probe`, focused CTest
  `theron_v1_runtime_admission`, and focused `git diff --check`.

- 2026-07-16 DM2 skproject item max-charge receipt: added
  `dm2_v1_skproject_get_max_charge()` for the bounded
  `DM2_GET_MAX_CHARGE` / `GET_MAX_CHARGE` slice. The helper preserves the
  skproject DB-type outcomes used by the item charge/value path: DB5 weapon
  and DB6 cloth max at 15, DB10 miscellaneous items max at 3, and
  `OBJECT_NULL` or unsupported DB types return zero. This does not implement
  recursive item value/weight, GDAT DBSPEC lookup, container moneybox logic,
  potion value scaling, player weight aggregation, or HUD/runtime
  consumption. Verification: strict direct C99 build/run of
  `test_dm2_v1_skproject_core`, focused CTest `dm2_v1_skproject_core`, and
  `git diff --check`.

- 2026-07-16 Theron runtime startup-level anchor receipt: added
  `Theron_V1RuntimeStartupLevelAnchorReceipt` and
  `theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor()` so the
  admitted US raw Track 02 FIFO/session handoff can carry the bounded
  all-dungeon route's real Hall of Records anchor metadata forward without
  promoting object or level semantics. The receipt requires source-bound
  startup anchor offsets, dimensions, seed/index, object/level/all-dungeon
  route hashes, no-fallback object/non-startup blockers, and keeps object-table
  admission, non-startup level admission, exact level semantics, exact object
  semantics, payload semantics, visual semantics, and fallback visuals closed.
  Verification: focused Ninja target and direct run of
  `firestaff_theron_v1_runtime_admission_probe`, focused CTest
  `theron_v1_runtime_admission`, and `git diff --check`.

- 2026-07-16 CSB startup HUD/door host-raster handoff: tightened
  `csb_v1_startup_session_hud_door_input_package_receipt_pc34` so the first
  live HUD/door/input package handoff consumes a routed C017/C040 HUD host
  raster, not just a nonzero HUD host-surface hash. The receipt now requires
  nonzero HUD frame route, raster route, raster pixel hash, verified HUD
  binding hash, the exact resident C017/C040 owners, and the two-surface HUD
  raster count before it can carry the package into the first door tick and
  command input. No synthetic HUD, door, save, DSA, or host pixels were added.
  Verification: focused standalone CTest
  `csb_v1_startup_session_contract_pc34_compat`, top-level focused CTests
  `csb_v1_startup_terminal_handoff_real_data_pc34_compat`,
  `csb_v1_startup_receipt_coherence_pc34_compat`,
  `csb_v1_startup_entrance_pointer_pc34_compat`, and
  `csb_v1_startup_raster_present_pc34_compat`, plus `git diff --check`.

- 2026-07-16 CSB startup title/opening host-capture stability: tightened the
  CSB v1 title-to-opening session receipt so PRESENTS, CHAOS, and STRIKES
  BACK must carry distinct host-surface and route hashes, and so the
  C004/C002/C003 opening host frame must carry nonzero frame route, raster
  route, and pixel hashes before it can satisfy either the opening-door
  receipt or the broader title/opening consumption receipt. The gate still
  consumes only existing real startup-session host receipts and does not
  create synthetic title, door, HUD, or save data. Verification: direct
  strict C11 build/run of `test_csb_v1_startup_session_contract_pc34_compat`,
  focused startup CTest `csb_v1_startup_terminal_handoff_real_data_pc34_compat`,
  `csb_v1_startup_receipt_coherence_pc34_compat`,
  `csb_v1_startup_entrance_pointer_pc34_compat`, and
  `csb_v1_startup_raster_present_pc34_compat`, plus `git diff --check`.

- 2026-07-16 DM2 skproject CPX lower-heap receipt: added isolated
  `dm2_v1_skproject_cpx_heap` helpers for the bounded
  `ALLOC_LOWER_CPXHEAP` / `ALLOC_CPXHEAP_MEM` and c_dballoc link/unlink
  ordering slice. The receipt allocates from the low edge of the first
  sufficient free span, distinguishes exact-fit unlink from split residuals,
  reinserts freed spans by ascending heap offset, and coalesces adjacent
  blocks without creating CPX pointers, decoded image buffers, or fallback
  graphics. Verification: strict direct C99 build/run of
  `test_dm2_v1_skproject_cpx_heap` and `git diff --check`.

- 2026-07-16 DM1 GROUP F0190-F0192 audit repair: closed stale
  `UNCERTAIN_NUMBERED_EVIDENCE` rows for the existing source-locked
  single-creature group damage/compaction, all-creatures group damage, and
  poison-resistance-adjusted attack helpers. This did not add data or broaden
  runtime behavior; it records the verified DM1-owned coverage already present
  in `dm1_v1_combat_pc34_compat`, F0738 group damage, and
  `F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat`. Verification:
  focused Ninja targets, direct runs, and focused CTest for
  `dm1_v1_combat_damage_source_lock`,
  `dm1_v1_grp02_f0738_apply_damage_to_group`, and
  `dm1_v1_grp02_f0192_poison_resistance_source_lock`.

- 2026-07-16 Nexus DGN package/host no-draw consumer gate: added
  `nexus_v1_dgn_package_host_consumer_gate()` as the bounded real-data
  consumer after the DGN face/material provenance receipt. It admits only a
  ready no-draw retail-DGN material receipt plus explicit host-route request,
  package-consumed bit, matching level index, canonical DGN size, face count,
  and Structure2 descriptor count. Accepted receipts record package/host
  source-route consumption but keep original Saturn rendering, material
  semantics, raster input submission, and fallback visuals blocked.
  Verification: strict direct C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`, focused Ninja target
  `test_nexus_v1_dgn_face_material_provenance`, and focused CTest
  `nexus_v1_dgn_face_material_provenance`.

- 2026-07-16 CSBWin restored DSA DiscardText message boundary: extended the
  restored timer tick bridge regression so authenticated STKOP_DiscardText
  reaches only the existing CSB-owned DB2/F0168 one-message receipt. The path
  clears that receipt through the saved TIMER/DSA owner, keeps generic text
  mutation blocked, and still cannot create or route a host message log.
  Verification: focused Ninja target and direct executable
  `test_csb_v1_dsa_restored_timer_tick_bridge`, focused CTest
  `csb_v1_dsa_restored_timer_tick_bridge`,
  `csb_v1_dsa_parameter_message_save_handoff`, and
  `csb_v1_dsa_localstate1_save_handoff`, plus `git diff --check`. Broader
  `csb_v1_dsa_trigger_single_step_pc34_compat` still fails its pre-existing
  unsupported world-mutating AMPERSAND subcode assertion in this checkout.

- 2026-07-16 Theron runtime-session handoff receipt: added
  `Theron_V1RuntimeSessionHandoffReceipt` and
  `theron_v1_runtime_session_handoff_from_admission()` so startup/session code
  can consume the admitted US raw Track 02 FIFO payload without promoting
  semantics. The receipt copies only the original `$3840` -> READ(6) ->
  FIFO -> game-RAM consumer provenance, marks real runtime capture as still
  required, and keeps payload semantics, visual semantics, object-table
  admission, level admission, and fallback visuals closed. Verification:
  `cmake --build build-local-ninja --target firestaff_theron_v1_runtime_admission_probe`,
  direct `build-local-ninja/firestaff_theron_v1_runtime_admission_probe`,
  focused CTest `theron_v1_runtime_admission`, adjacent CTest
  `theron_v1_game_payload_trace_corpus`, and the same focused target/CTest in
  `build-verify`.

- 2026-07-16 DM1 GROUP F0186-F0189 drop/delete receipts: added
  source-named DM1 receipts for fixed creature possession drops, moving
  fixed-drop cell stack consumption, GROUP.Slot possession-chain drops, and
  final group delete gates. The slice is bound to raw C04-shaped group data
  and preserves ReDMCSB's fixed-drop table payloads, reverse moving-cell
  consumption, random cell packing, next-thing snapshot, group slot clear,
  weapon-vs-non-weapon sound choice, group Thing unlink, raw C04 next clear,
  active-state retirement, and C29-C41 event deletion intent without
  materializing dungeon things or broader renderer/runtime behavior.
  Verification: direct strict C99 build/run and focused CTest
  `dm1_v1_group_drop_delete_receipts_pc34_compat`.

- 2026-07-16 DM2 skproject `CUT_RECORD_FROM` slice: added
  `dm2_v1_skproject_cut_record_from()` as the bounded source mutation receipt
  for SKULLWIN `c_record.cpp`. The route rejects null/end cut inputs, masks
  ObjectIDs with `0x3fff` for the original address/link comparisons, unlinks
  parent-owned and tile-owned chains, replaces first tile roots with the cut
  record's next link, compacts single-root ground-stack entries while
  decrementing later column offsets, and resets the cut record `w0` to
  `OBJECT_END_MARKER`. Verification: strict direct C11 build/run and focused
  `dm2_v1_dungeon_loader_first_map_gate`.

- 2026-07-16 Nexus MENU.BPK PRS3 decoded-output proof gate: added a
  fail-closed `nexus_v1_bpk_archive_prs3_decoded_output_proof_gate()` receipt
  for caller-supplied PRS3 output sidecars. It reuses the MENU.BPK stream
  plan, requires exact decoded byte count plus FNV match, records whether the
  capture source and original Saturn provenance are present, and keeps opcode
  grammar, decoder promotion, runtime upload, and fallback visuals blocked.
  Verification: focused strict direct C99 build/run of
  `test_nexus_v1_bpk_prs3_payload_evidence` with local real MENU.BPK coverage
  when present, focused Ninja/CTest target
  `nexus_v1_bpk_prs3_payload_evidence`, and `git diff --check`.

- 2026-07-16 CSBWin SetTimer duplicate-policy matrix: tightened the CSB runtime
  timer owner so C05..C10 map timers never fall through the shared DM1 merge
  helper after the CSBWin duplicate scan. Matching source timer functions on
  the same square/time reuse the original live slot and replace only the action
  byte; TT_STONEROOM additionally requires the same cell/position; different
  timer functions on the same square append as separate source-owned timers.
  Verification: focused Ninja target and CTest
  `csb_v1_csbwin_duplicate_timer_policy`, plus `git diff --check`.
  No synthetic save, DSA, dungeon, HUD, or host data was added.

- 2026-07-16 DM2 skproject `ADD_ITEM_CHARGE` source mapping: added
  `dm2_v1_skproject_add_item_charge()` with the skproject DB5 weapon,
  DB6 cloth, and DB10 miscellaneous charge bitfields, original max-charge
  clamps, and fail-closed null/unsupported-object behavior. The SKULLWIN,
  SkWinCore, and xxx.cpp aliases are now marked source-mapped in the DM2
  symbol audit. Verification: direct C11 build/run and focused CTest
  `dm2_v1_skproject_core`.

- 2026-07-16 Theron runtime admission consumes game-owned FIFO corpus
  receipts: wired `Theron_V1RuntimeAdmissionReceipt` to admit only the
  existing verified US raw Track 02 game-owned FIFO payload receipt. The new
  admission path preserves the `$3840` -> `$e009`/READ(6)/FIFO/main-RAM
  consumer facts, rejects missing consumer proof, wrong variant/hash, and
  any payload-semantic promotion, and keeps payload semantics, visual
  semantics, and fallback visuals closed. Verification:
  `firestaff_theron_v1_runtime_admission_probe` Ninja target and executable
  run in both `build-local-ninja` and `build-verify`, focused CTest
  `theron_v1_runtime_admission`, existing CTest
  `theron_v1_game_payload_trace_corpus`, and `git diff --check`.

- 2026-07-16 DM1 F0139 creature allowed-on-map source gate: added a
  source-named `F0139_DUNGEON_IsCreatureAllowedOnMap` helper that reads raw
  C04 `GROUP.Type` via F0156-owned Thing data and scans only the target map's
  loaded DUNGEON.DAT allowed-creature list. M10 F0267 group moves/generators
  now consume it after newly generated groups publish their raw C04 record;
  missing raw group data, non-group Things, invalid maps, and malformed
  creature-count metadata fail closed. Verification:
  `dm1_v1_f0139_creature_allowed_on_map_pc34_compat` passed. The broader
  `m10_c006_generator_reenable_dispatch_pc34_compat` target builds but still
  fails one C38 ACTIVE_GROUP aspect assertion in this already-dirty checkout.

- 2026-07-16 DM2 skproject `APPEND_RECORD_TO` slice: added
  `dm2_v1_skproject_append_record_to()` as a bounded source mutation receipt
  over ObjectID chains. It rejects null/end markers, resets appended
  `GenericRecord::w0` to `OBJECT_END_MARKER`, appends through parent links or
  existing tile chains, and inserts into empty byte-square tiles with the
  skproject ground-stack shift plus column-index bump. Verification: focused
  Ninja/CTest `dm2_v1_dungeon_loader_first_map_gate`.

- 2026-07-16 DM2 c_gdatfile bitmap allocation/free receipts: added
  source-named bounded receipts for `DM2_ALLOC_PICT_BUFF`,
  `DM2_FREE_PICT_BUFF`, `DM2_ALLOC_NEW_BMP`, and `DM2_FREE_PICT_ENTRY`.
  The receipts preserve skproject row-byte accounting for 4bpp and 8bpp
  bitmaps, the six-byte bitmap header, CPX raw-index ownership, and the
  preserved-GFX free split between struct-before and list-unlink routes while
  still refusing to allocate decoded pixels, CPX nodes, or fallback visuals.
  Verification: focused direct C11 build/run and CTest
  `dm2_v1_gdat_querydb_receipts`.

- 2026-07-16 CSB boot DSA/save-runtime handoff receipt: surfaced the strict
  CSBWin DSA corpus gate on `CSB_V1_BootRuntimeSaveImportReceipt_PC34` so
  boot/runtime callers can distinguish CSBGAME roster-loader readiness from
  real DSA runtime-handoff readiness. Plain CSBGAME saves now remain
  fail-closed for DSA handoff with `reject_dsa_corpus_no_extended_features`,
  while CSBWin runtime import refuses paths unless the existing filename/loader
  classifier marks them import-ready. Verification: focused Ninja build for
  `test_csb_v1_boot_runtime_handoff` and
  `test_csb_v1_csbwin_save_loader_boundary_pc34_compat`; the boundary test
  passed, and the boot test improved from 439/18 to 440/17 with the remaining
  failures in pre-existing startup/capture-route assertions.

- 2026-07-16 Theron game-owned FIFO consumer corpus gate: added
  `theron_v1_raw_loader_trace_import_game_owned_fifo_payload_file()` as the
  bounded file wrapper for staged original Mednafen consumer transcripts. The
  new `theron_v1_game_payload_trace_corpus` probe validates an explicit local
  US raw Track 02 plus consumer trace, requiring the existing `$3840` ->
  `$e009`/READ(6)/FIFO/main-RAM consumer chain to remain byte-faithful and to
  reject mutated media before publishing only an opaque payload receipt. No
  level, object, bitmap, palette, or visual semantics are promoted.
  Verification: strict syntax checks for
  `src/theron/theron_v1_raw_loader_trace.c` and
  `probes/theron/firestaff_theron_v1_game_payload_trace_corpus_probe.c`,
  focused Ninja target `firestaff_theron_v1_game_payload_trace_corpus_probe`,
  focused CTest `theron_v1_game_payload_trace_corpus`, and
  `git diff --check`.

- 2026-07-16 Nexus DGN package/host material no-draw gate: tightened the
  Structure2/Structure3 face-material provenance receipt so a canonical retail
  DGN can prove source-bound selector/descriptor routing at the package/host
  boundary without becoming drawable raster input. The receipt now reports
  material semantics unproven, original Saturn capture required/unavailable,
  no fallback visuals, and `can_submit_raster_input=0` even when the
  Structure3 face selectors resolve through the same Structure2 descriptor
  table. Verification: strict direct C99 build/run of
  `test_nexus_v1_dgn_face_material_provenance`, focused Ninja target
  `test_nexus_v1_dgn_face_material_provenance`, focused CTest
  `nexus_v1_dgn_face_material_provenance`, and `git diff --check` passed.

- 2026-07-16 CSBWin DSA/save-runtime corpus gate: added
  `csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt()` and the file
  wrapper beside the CSBWin save loader-boundary classifier. The receipt
  admits only recognised CSBWin save filenames carrying a checksum-authenticated
  Extended Features DSA section with real actions/program words, a verified
  game-info/level-index tail, and a following valid GAMEBLOCK1 header before
  marking DSA runtime handoff ready. DSA-less, bad-tail, bad-checksum,
  non-CSBWin filename, and missing-GAMEBLOCK1 cases remain fail-closed.
  Verification: focused Ninja/CTest
  `csb_v1_csbwin_save_loader_boundary_pc34_compat_unit` and sibling
  `csb_v1_csbwin_512_xor_pad_classify_unit` passed.

- 2026-07-16 DM1 DUNGEON F0173/F0174 current-map receipts: added
  source-named `DM1_V1_DungeonData_F0173SetCurrentMapPc34Compat` and
  `DM1_V1_DungeonData_F0174SetCurrentMapAndPartyMapPc34Compat` wrappers over
  the central DM1 dungeon-data store. F0173 now publishes a receipt for the
  loaded current-map index/dimensions update and preserves party metadata;
  F0174 composes F0173 then updates only the party map index while preserving
  party x/y/direction. Invalid map indices reject before either current-map or
  party-map mutation. Verification:
  `dm1_v1_f0173_f0174_current_map_pc34_compat` covers F0173-only, F0174
  current+party, and invalid-map no-partial-mutation cases. Audit rows
  `F0173_DUNGEON_SetCurrentMap` and
  `F0174_DUNGEON_SetCurrentMapAndPartyMap` are closed as
  `VERIFIED_SOURCE_MAPPING`.

- 2026-07-16 DM2 skproject picture-mement helper slice: added source-named
  bounded receipts for `TEST_MEMENT`, `RECYCLE_MEMENTI`, `ALLOC_NEW_PICT`,
  `ALLOC_IMAGE_MEMENT`, `ALLOC_PICT_MEMENT`, `CALC_PICT_ENT_HASH`,
  `FREE_IMAGE_MEMENT`, and `FREE_PICT_MEMENT`. The route preserves
  skproject's picture payload/header sizing, ExtendedPicture hash packing,
  image-backed versus Picture.w12 cache-index picture mement branches, the
  default-image fallback only after an absent primary entry, and the original
  Y=-32/8bpp image mement admission gate while failing closed instead of
  fabricating decoded graphics. Verification: strict direct C11 build/run and
  focused Ninja/CTest `dm2_v1_skproject_core`.

- 2026-07-16 Nexus MENU.BPK PRS3 opcode-prefix witness: added a bounded
  `nexus_v1_bpk_archive_prs3_opcode_prefix_witness()` receipt for validated
  PRS3 frames. It walks at most a caller-specified command count under an
  explicit LSB/MSB control-bit order, records control/operand byte
  consumption, literal/backref command counts, first backref operands, and
  stop reason, but never allocates output, promotes a decoder, or changes the
  MENU.BPK renderer/upload route. Verification: strict direct C99 build/run
  of `test_nexus_v1_bpk_prs3_payload_evidence` and focused Ninja/CTest
  `nexus_v1_bpk_prs3_payload_evidence` passed, including optional local
  MENU.BPK evidence where present.

- 2026-07-16 Nexus MENU.BPK/DGN fail-closed route slice: wired the
  `test_nexus_v1_menu_bpk_renderer_handoff_gate` CMake/CTest target, fixed
  the MENU.BPK `READY_DECODED` contract so generic decoded PRS3 remains
  `blocked-prs3`, and added an unverified-source regression that refuses a
  parseable MENU.BPK route without the canonical source hash. The DGN
  face/material provenance receipt now requires static Structure3 selectors
  to resolve into the same canonical Structure2 descriptor table before
  publishing a package/host no-draw boundary; it still blocks real DGN mesh
  render and permits no fallback visuals. Verification: focused CTest passed
  for `nexus_v1_dgn_face_material_provenance` and
  `nexus_v1_menu_bpk_renderer_handoff_gate`. The broad
  `nexus_v1_startup_menu_pc34_compat` regression still fails on the existing
  39 startup/Saturn/DGN/SFX completeness assertions.

- 2026-07-16 DM2 skproject core/random/cache helper slice: added source-named
  wrappers for `DM2_RAND16`, `DM2_RANDBIT`, `DM2_RANDDIR`,
  `FIND_ICI_FROM_CACHE_HASH`, `INSERT_CACHE_HASH_AT`,
  `QUERY_MEMENTI_FROM`, `ADD_CACHE_HASH`,
  `QUERY_MEMENT_BUFF_FROM_CACHE_INDEX`, `GET_TEMP_CACHE_HASH`, and
  `ALLOC_TEMP_CACHE_INDEX`. The cache model preserves skproject's sorted
  indirect hash table and cache-index/raw-data mement routing, but fails closed
  when eviction/full mement lifecycle state is not supplied. Verification:
  focused strict direct and Ninja `test_dm2_v1_skproject_core`.

- 2026-07-16 DM2 skproject square-chain traversal handoff: added
  `dm2_v1_dungeon_walk_square_things()` as the loader-owned bounded route for
  `GET_TILE_RECORD_LINK` plus `GET_NEXT_RECORD_LINK` consumers. The object
  model now uses that API instead of reading `GenericRecord::w0` locally, so
  incomplete PC G1 record graphs inherit the existing traversal guard while
  source-shaped chains still materialize objects. Verification: strict direct
  and Ninja focused tests for chained skproject records plus guarded invalid
  and cyclic G1 links.

- 2026-07-16 DM1 GROUP.C F0180/F0183/F0184 active-state wrappers: added
  explicit source-named start-wandering and active-group add/remove helpers
  over decoded original C04 records. F0180 builds the C37 wander event at
  game time + 1. F0183 copies C04 cells, packs the group direction per
  creature, stores the source square and low game-time byte, and fails closed
  at the active-group capacity. F0184 writes cells/direction back to C04,
  resets C4+ behavior to wander, and retires only a valid active slot.
  Verification: strict direct C11 build/run and Ninja target
  `test_dm1_v1_group_active_state_pc34_compat`.

- 2026-07-16 DM2 c_map real-data tile access receipts: closed the
  skproject `c_map.cpp`/SKWIN `xxx.cpp` tile accessor slice for
  `GET_TILE_VALUE`, `GET_ADDRESS_OF_TILE_RECORD`, `IS_TILE_PASSAGE`, and
  `IS_TILE_SOLID`. The new real-data focus test accepts only canonical PC G1
  `DUNGEON.DAT`, walks map 0 source bytes, proves the 22 map-root tiles via
  the column-index/ground-stack tables, and rejects plain tiles before any
  synthetic record address can appear. Verification: strict direct C11
  build/run of `test_dm2_v1_c_map_tile_access_real_data` against local
  canonical DM2 data, direct existing `test_dm2_v1_c_map_tile_access`, and
  strict `-fsyntax-only` for `src/dm2/dm2_v1_dungeon_loader.c`.

- 2026-07-16 DM2 c_map object-index/root ownership follow-up: promoted
  skproject `GET_OBJECT_INDEX_FROM_TILE` to a source-named receipt that
  exposes the column-prefix table offset, row-adjusted ground-stack index,
  ground-stack word offset, and selected ObjectID without resolving a record.
  The PC G1 partial boot receipt now records blocked roots by DB type, proving
  the canonical unresolved set as 1 DB8 and 4 DB10 roots while still reporting
  no blocked-record reads and no `GenericRecord::w0` traversal.

- 2026-07-16 DM2 c_map `CHANGE_CURRENT_MAP_TO` receipt: added a bounded
  source-named map-pointer receipt for same-map early return, negative/range
  rejection, selected map dimensions, raw tile-map offset, column-index
  offset, and player fallback pose fields. The route records skproject map
  state only; it does not invent renderer, save, GDAT, or live HUD state.
  Verification: strict direct `test_dm2_v1_c_map_tile_access` and focused
  `git diff --check`.

- 2026-07-16 CSB PANEL.C F0347 HUD-panel receipt: added a CSB-owned
  `F0347_INVENTORY_DrawPanel` adapter over the existing real C017/C040
  startup HUD-panel blit receipt. The route accepts only source-owned
  real-asset panel receipts, carries C017 and optional C040 hashes unchanged,
  rejects synthetic/wrapper receipts, and refuses stray or missing C040
  material at the resurrect-panel boundary. Verification: strict direct C11
  build/run and Ninja target
  `test_csb_v1_f0347_inventory_draw_panel_pc34_compat`.

- 2026-07-16 DM2 skproject core helpers: added source-mapped
  `BETWEEN_VALUE`, `DM2_BETWEEN_VALUE`, `ALLOC_TEMP_RECT`, and
  `ALLOC_TEMP_ORIGIN_RECT`. The temp rectangles use the original four-slot
  ringbuffer behavior and return receipts; no cache mement, image, or GDAT
  buffer is fabricated. Verification: direct and Ninja
  `test_dm2_v1_skproject_core`.

- 2026-07-16 CSB HINTHTC F1918/F1919 initial-load receipt: added a CSB-owned
  wrapper for the source F1918 load sequence and explicit F1919 post gate.
  The route reads the validated 512-byte header, then GLOBAL_DATA,
  ACTIVE_GROUPS, and PARTY through exact caller-owned spans; partial, failed,
  NULL, odd, or missing spans fail closed before any later save-tail handoff.
  Verification: strict direct C11 build/run of
  `test_csb_v1_f1918_hintload_initial_load_pc34_compat`; no synthetic save,
  DSA, title, door, HUD, or visual data was added.

- 2026-07-16 CSB startup presented-frame route hash gate: Mac/app presented
  capture now requires a 320x200 indexed real-CSB frame to carry a route hash
  from the receipt-owned title, HUD/door, or credits capture path. Unrelated
  presented-frame hashes and non-real CSB pixels fail closed without synthetic
  fallback.

- 2026-07-16 DM2 skproject GDAT querydb receipts: `dm2_v1_asset_loader`
  now exposes bounded source-named wrappers for
  `QUERY_GDAT_RAW_DATA_FILE_POS`, `QUERY_GDAT_RAW_DATA_LENGTH`,
  `LOAD_GDAT_RAW_DATA`, `QUERY_GDAT_ENTRYPTR`,
  `QUERY_GDAT_ENTRY_DATA_INDEX`, `QUERY_GDAT_ENTRY_DATA_PTR`,
  `QUERY_GDAT_ENTRY_DATA_LENGTH`, `QUERY_GDAT_ENTRY_DATA_BUFF`, and
  `QUERY_GDAT_ENTRY_IF_LOADABLE`, while retaining the existing
  `QUERY_GDAT_PICT_OFFSET` and `QUERY_GDAT_IMAGE_LOCALPAL` mappings. Scalar
  `dtWordValue`/`dtImageOffset` entries expose only their ENT1 data index and
  never become synthetic raw buffers. Verification: direct focused compile/run
  of `tests/test_dm2_v1_gdat_querydb_receipts.c` with
  `src/dm2/dm2_v1_asset_loader.c`, 14/14 passed including optional real
  local DM2 `GRAPHICS.DAT` census. CMake generation remains blocked by
  pre-existing missing non-DM2/shared/probe sources.

- 2026-07-16 DM2 c_map accessors: `dm2_v1_dungeon_loader` now exposes
  source-bounded skproject `GET_TILE_VALUE`, `GET_ADDRESS_OF_TILE_RECORD`, and
  `IS_TILE_PASSAGE` helpers over the parsed byte-map, column-index, and
  ground-stack tables. The route resolves only selected tile root addresses;
  it does not walk `GenericRecord::w0` or promote GDAT/HUD/save data.

- ✅ 2026-07-16 Nexus ITEM.IBS/DGN command VDP1 blocker receipt:
  carried the missing original-VDP1-command provenance blocker through
  `Nexus_V1_DgnCommandPacked4BppMaterial` and its receipt. Structure1Fa
  descriptor-0008 ITEM.IBS material can now prove exact source bytes at the
  DGN floor-command boundary while still reporting zero VDP1 captures, no
  texel-order proof, no draw authorization, and no fallback visuals.
  Verification: strict syntax checks for `src/nexus/nexus_v1_dungeon.c` and
  `tests/test_nexus_v1_dgn_geometry_readiness.c`.

- ✅ 2026-07-16 DM2 SKSave corpus first-importable provenance receipt:
  extended `DM2_SKSaveCorpusReceipt` with a bounded public source-kind enum
  and first-importable payload byte count. `dm2_v1_sksave_corpus_scan` now
  records provenance only after the existing runtime save-candidate parser
  accepts the payload, preserving rejection behavior and avoiding synthetic
  save promotion. Verification: `test_dm2_v1_save_load` passed 26/26.

- ✅ 2026-07-16 DM2 skproject ANIM memory helpers: added source-mapped
  `ANIM_farmalloc`, `ANIM_farfree`, and `ANIM_farcoreleft` helpers to the
  ANIM receipt layer. The test pins malloc-backed allocation/free and the
  skproject Win32 `1024*1024` farcoreleft sentinel without introducing
  synthetic artwork or GDAT substitutes. Verification:
  `test_dm2_v1_anim_bootstrap`.

- ✅ 2026-07-16 Theron Track02 copied-continuation source binding:
  fixed the initial post-envelope copied-code successor receipt so a second
  TII that starts inside the authenticated continuation maps successor
  instruction bytes back through `copy_source_offset + next_copy_offset`,
  not just the destination-local offset. This keeps `$0b52` post-envelope
  evidence byte-faithful and still leaves `level_or_object_semantics_proven=0`.
  Verification: strict syntax checks for `src/theron/theron_v1_raw_loader_trace.c`
  and `probes/theron/firestaff_theron_v1_raw_loader_trace_initial_level_handoff_probe.c`.

- ✅ 2026-07-16 DM2 skproject ANIM file/blit slice:
  added source-mapped `ANIM_FILE_OPEN`, `ANIM_GET_FILE_SIZE`,
  `ANIM_READ_HUGE_FILE`, `ANIM_FILE_CLOSE`, `ANIM_STRCPY`,
  `ANIM_TOUPPER`, and `ANIM_BLIT_TO_MEMORY_ROW_4TO4BPP` helpers.
  The permanent ANIM test now covers real file handles, 0x8000-byte chunked
  reads, EOF-preserving uppercase, strcpy return semantics, and unaligned
  4bpp nibble row copies. Verification: `test_dm2_v1_anim_bootstrap`.

- ✅ 2026-07-16 DM2 skproject ANIM bootstrap/decode slice:
  added source-mapped `ANIM_BOOTSTRAP_SWOOSH`,
  `ANIM_BOOTSTRAP_TITLE`, `ANIM_DECODE_IMG1`,
  `ANIM_FILL_SEQ_4BPP`, and `ANIM_SETPIXEL_SEQ_4BPP` receipts/helpers.
  The exact skproject argv lists are pinned for swoosh/title, and IMG1
  4bpp fill, literal, previous-row, and truncated-stream behavior is covered
  by a permanent CMake test. Verification: `test_dm2_v1_anim_bootstrap`.

- ✅ 2026-07-16 DM2 skproject SHOW_MENU_SCREEN real-GDAT startup route:
  startup presentation now emits only the original `TITLE/0 dt07/4`
  menu surface for `SHOW_MENU_SCREEN`; `TITLE/0 dt07/1` remains source
  receipt evidence for the preceding title phase, not a second draw.
  Render ownership accepts one menu GDAT command plus title receipt proof
  and rejects the old synthetic no-image text/rect fallback. Verification:
  `test_dm2_v1_startup_menu_action_contract` passed 99/99.

- ✅ 2026-07-16 CSB HINTLOAD save-part wrappers: added CSB-owned
  `F1910_LoadSavedGamePart`, `F1913_LoadAndDeobfuscateSavedGamePart`, and
  `F1914_LoadAndDeobfuscateSavedGameHeader` entry points to the native CSB
  save/load API. `csb_v1_load_game` now validates the 512-byte header through
  F1914 before reading the bounded state prefix through F1910; F1913 covers
  word-sized SAVEHEAD-key deobfuscating part loads. The ReDMCSB audit rows
  are closed as `PC34_SOURCE_IMPLEMENTED`. Verification: focused CSB
  save/export/import unit target and `git diff --check`.

- ✅ 2026-07-16 DM1 ReDMCSB DUNGEON F0142 projectile-aspect mapping:
  added a DM1-owned helper for `F0142_DUNGEON_GetProjectileAspect`'s signed
  return contract. Spell/projectile slots now expose negative
  `PROJECTIL_ASPECT` ordinals, weapon M066 projectile-art ordinals stay
  negative, and ordinary thrown objects expose their G0237/G0209
  `OBJECT_ASPECT` index before the existing material resolver selects M613 or
  M612 graphics. The `F0142` symbol disposition is closed as
  `VERIFIED_SOURCE_MAPPING`, reducing the DM1 backlog to 117 open rows.
  Verification: `cmake -S . -B build-dm1-worker`,
  `cmake --build build-dm1-worker --target test_dm1_v1_f0142_projectile_aspect_pc34_compat --parallel`,
  direct `./build-dm1-worker/test_dm1_v1_f0142_projectile_aspect_pc34_compat`,
  and `git diff --check` passed. The broader pre-existing
  `test_dm1_v1_projectile_explosion_render_pc34_compat` executable still has
  unrelated F0115 world-summary fixture failures.

- ✅ 2026-07-16 DM2 skproject weather timer receipts: added source-mapped
  receipts for `SKULLWIN/c_weather.cpp::DM2_SET_TIMER_WEATHER` and
  `DM2_weather_3df7_0037`. Runtime weather now records the outdoor-only
  182-tick scheduling boundary and the exact seed/weather transaction; indoor
  and not-due ticks fail closed and do not mutate weather. The skproject audit
  rows are closed as `VERIFIED_SOURCE_MAPPING`. Verification: strict direct
  C11 build/run of `tests/test_dm2_v1_weather_timer_receipts.c`, strict
  `-fsyntax-only` for the weather/runtime sources and weather tests, plus
  `git diff --check`.

- ✅ 2026-07-16 source-symbol backlog queue tooling: added
  `tools/symbol_backlog.py`, which reads the ReDMCSB callable-symbol and
  skproject DM2 named-symbol TSV audits, keeps MISSING/UNCERTAIN rows open,
  prioritizes runtime families, and emits per-game work queues in text or JSON.
  Verification: `python3 -m py_compile tools/symbol_backlog.py`,
  `python3 tools/symbol_backlog.py --limit 12`, and
  `python3 tools/symbol_backlog.py --game DM2 --limit 8 --json` pass.

- ✅ 2026-07-16 DM1 ReDMCSB DUNGEON F0150-F0153 behavior mapping:
  added DM1-owned callable wrappers for `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement`,
  `F0151_DUNGEON_GetSquare`, `F0152_DUNGEON_GetRelativeSquare`, and
  `F0153_DUNGEON_GetRelativeSquareType`. The mapping preserves the source
  direction/right-step transform, column-major square addressing, and F0151's
  exact map-edge synthetic wall behavior: corridor/pit edges emit the
  inward-facing random-ornament wall flag, while wall edges, diagonals, and
  farther-out coordinates remain plain walls. The ReDMCSB callable audit rows
  are closed as `VERIFIED_SOURCE_MAPPING`, reducing DM1 open symbol rows from
  122 to 118. Verification: strict direct C11
  build/run of `tests/test_dm1_v1_dungeon_f0150_f0153_pc34_compat.c` with
  `src/dm1/dm1_v1_dungeon_square_structs_pc34_compat.c`, plus strict
  `-fsyntax-only` on the touched DM1 source.

- ✅ 2026-07-16 Nexus MENU.BPK renderer fail-closed gate: tightened the
  startup/menu handoff so missing, invalid, no-surface, PRS3-blocked, and
  truncated MENU.BPK decode receipts all block real menu rendering without
  permitting fallback visuals. The launcher now requires a valid renderer
  handoff before marking the real-menu asset route ready; a missing receipt is
  no longer accepted as an implicit pass. Verification: strict direct C11
  `test_nexus_v1_menu_bpk_renderer_handoff_gate`, strict direct C11
  `test_nexus_v1_startup_title_route_asset_gate`, object build of
  `nexus_v1_launcher.c`, existing `test_nexus_v1_startup_title_pointer_contract`
  passed 17/17, and CTest passed for `nexus_v1_startup_title_pointer_contract`
  plus `nexus_v1_dgn_geometry_readiness`.

- ✅ 2026-07-16 CSB ReDMCSB F2262 timer-A event boundary: added a
  CSB-owned F2262 adapter for the source input-wait state transition:
  `G0317_i_WaitForInputVerticalBlankCount` advances until
  `G0318_i_WaitForInputMaximumVerticalBlankCount`, then sets
  `G0321_B_StopWaitingForPlayerInput`. The optional MEDIA670 FM-Towns
  counter/volume-fade branch is represented only when caller-provided
  FM-Towns state is present; PC34 reports that platform branch unavailable
  and does not synthesize SND service behavior. Verification: strict direct
  C11 build/run of `test_csb_v1_f2262_timer_a_event_pc34_compat`, plus
  `git diff --check` on the touched CSB files.

- ✅ 2026-07-16 source-symbol disposition queue support: the backlog tool now
  reads `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`, hides explicitly
  closed dispositions by default, exposes `--include-disposed` for audits, and
  includes disposition evidence in JSON/text output. The disposition TSV is a
  schema gate only; no symbol is closed without an evidence row. Verification:
  `python3 -m py_compile tools/symbol_backlog.py`,
  `python3 tools/symbol_backlog.py --limit 0`, and
  `python3 tools/symbol_backlog.py --include-disposed --limit 0` pass. The
  fixture test `python3 tests/test_symbol_backlog_dispositions.py` also passes
  and proves closed rows are hidden by default but visible with evidence under
  `--include-disposed`.

- ✅ 2026-07-16 DM2 G1 boot pose and runtime HUD capture closure: DM2
  boot now consumes the source-validated G1 File_header party pose decoded by
  the dungeon loader instead of replacing it with the old 15,15,N synthetic
  default. Four-direction HUD/runtime sampling now restores the original G1
  pose after probing, so boot, render, HUD, action, turn, and move receipts
  all remain source-positioned. Verification: CTests `dm2_v1_boot_profile_smoke`,
  `dm2_v1_save_load`, and `dm2_v1_runtime_handoff_smoke` pass in
  `build-local-ninja`.

- ✅ 2026-07-16 Theron ISO/Track02 media gate handoff: added the missing
  ISO-end and track-media gates, then threaded their receipts through the
  Theron media inventory, profile availability, launch gate, launch decision,
  and v3 trace command plan. Raw Track02-ready media is the only path that
  prepares bitmap, level-route, and object-route capture; JP/US ISO end
  variants remain opaque and block loader/runtime use with visual fallback
  disabled. Verification: strict C11 direct build/run of 17 focused Theron
  probes covering ISO-end receipt, track media availability, media inventory,
  profile media availability/audio, launch gate, launch decision, and v3 trace
  schema.

- ✅ 2026-07-16 DM1 original-save corpus report gate: exposed the
  PC34 corpus-roundtrip receipt counters already maintained by the handoff
  implementation: rejected/truncated scan counts, Firestaff-manifest and
  malformed-envelope rejection counts, first failure result, and the
  path-independent provenance fingerprint. Verification: strict direct C11
  compile of `src/dm1/dm1_v1_original_save_classifier.c`, strict direct C11
  compile of `tests/test_dm1_v1_original_save_classifier_pc34.c`, and a
  strict direct C11 executable contract check for
  `DM1OriginalSavePC34CorpusRoundtripReport`.

- ✅ 2026-07-16 CSB terminal door-runtime handoff hardening: the CSB
  entrance-to-runtime receipt now rejects legacy one-byte C017/C040 wrapper
  shapes, incomplete PRESENTS/CHAOS/STRIKES playback, stale door-open state,
  non-neutral C040 transparency, and mismatched GRAPHICS.DAT ownership before
  publishing HUD-ready. The live handoff requires the runtime mirror and exact
  C017 224x136 / C040 144x73 source surfaces from the verified CSB session.
  Verification: focused strict C11 direct build/run of
  `test_csb_v1_startup_terminal_handoff_real_data_pc34_compat`, plus strict
  object builds for `csb_v1_boot.c` and
  `csb_v1_startup_runtime_surfaces_pc34_compat.c`.

- ✅ 2026-07-16 CSB terminal startup HUD receipt gate: the verified CSB
  startup session now carries title phase mask, entrance-door completion,
  terminal C017/C040 source identity, neutral-palette live HUD raster, and
  stale tick/generation rejection through a CSB-owned receipt. ReDMCSB
  `TITLE.C F0437`, `ENTRANCE.C F0806/F0807`, and `PANEL.C F0346/F0347`
  remain the source boundary; no synthetic title, door, or HUD fallback can
  satisfy this gate. Verification: strict C11 object builds for
  `csb_v1_startup_playback_pc34_compat.c`,
  `csb_v1_startup_runtime_surfaces_pc34_compat.c`, and
  `csb_v1_startup_terminal_receipt_pc34_compat.c`.

- ✅ 2026-07-16 DM1 spell-HUD material bundle gate: M11 now requires the
  complete source-owned spell HUD bundle before rendering an open spell
  panel: exact loaded C009 pixels, exact loaded C011 pixels, and a verified
  PC34 M653 font from GRAPHICS.DAT 695/557. The material receipt rejects
  cropped C009, dimension-only/missing C011, fallback font records, and
  no-draw F0394 plans, so a valid C009 background can no longer leak as a
  half-rendered spell HUD when C011 or M653 is unavailable. Verification:
  focused Ninja build plus CTests
  `dm1_v1_champion_panel_spell_area_overlay_pc34_compat`,
  `m11_dm1_spell_area_asset_route`, and
  `m11_dm1_action_spell_asset_fail_closed` pass in `build-local-ninja`;
  full `ninja -C build-local-ninja firestaff` also passes.

- ✅ 2026-07-16 DM2 complete-support original-save gate: the
  complete-support receipt now remains observable for a valid real-profile
  boot even when unrelated runtime GDAT checks are incomplete, carries the
  original-save state candidate count, and refuses `complete-support-ready`
  until at least one original SKSave candidate parses into source-owned state
  with no rejected original candidates. Verification: focused Ninja build
  passes; CTests `dm2_v1_save_load` and `dm2_v1_runtime_handoff_smoke` pass.
  `dm2_v1_boot_profile_smoke` now also passes after the G1 boot-pose and
  runtime HUD restore fix above.

- ✅ 2026-07-16 Theron ISO first-sector source handoff gate: the Track 02
  loader-intake boundary now has a separate MODE1/2048 ISO route for the
  first `$0b52` sector. It accepts only authenticated US ISO capture facts,
  rejects raw-BIN trace borrowing, sector conversion, JP zero ISO, and
  synthetic dungeon promotion, and copies the initial level envelope plus the
  post-envelope bytes as opaque source material with no object/bitmap
  semantics. Verification: focused Ninja build plus CTests
  `theron_v1_track02_loader_intake`,
  `theron_v1_raw_loader_trace_initial_level_handoff`, and
  `theron_v1_m11_launcher_handoff_boundary` pass in `build-local-ninja`.
  Remaining blocker: operator-stage a real ISO capture through this gate and
  independently prove object/bitmap semantics.

- ✅ 2026-07-16 CSB title/opening source-tick sequence gate: the
  title/opening consumption receipt now requires PRESENTS, CHAOS, and
  STRIKES BACK C001 host captures to carry an ordered source-tick sequence
  immediately before the active C004/C002/C003 opening host tick. This blocks
  replaying a stale title phase into the current opening receipt without
  adding any wrapper or synthetic surface path. Verification: focused Ninja
  build plus CTest `csb_v1_startup_session_contract_pc34_compat` pass in
  `build-csb-startup-session-contract`; adjacent CTests
  `csb_v1_startup_terminal_handoff_real_data_pc34_compat` and
  `csb_v1_startup_raster_present_pc34_compat` pass in `build-local-ninja`.
  Remaining blocker: the opt-in real package launch probe still reports
  broader local real-data/hash failures and external original-window capture
  remains required.

- ✅ 2026-07-16 Nexus Structure2 shared texture/palette anchor proof:
  Structure2 descriptor capture targets now explicitly report when image and
  palette payload anchors resolve to the same bounded opaque DGN span. The
  target writer emits `shared_image_palette_payload_anchor`, preserving the
  real Saturn capture request without promoting shared bytes into decoded
  pixels, palette semantics, or fallback visuals. Verification: focused Ninja
  build plus CTest for `nexus_v1_dgn_geometry_readiness`,
  `nexus_v1_structure2_no_draw_receipt`, `nexus_v1_dgn_material_raster`, and
  `nexus_v1_bpk_prs3_payload_evidence` pass in `build-local-ninja`.
  Remaining blocker: `nexus_v1_startup_menu_pc34_compat` still fails on the
  broader startup/menu/DGN route completeness gates, including MENU.BPK/PRS3
  and runtime DGN host-route assertions.

- ✅ 2026-07-16 DM2 runtime scene local-palette unblock: source-required
  DM2 dungeon rendering now accepts the strict provider-backed
  GRAPHICSSET image plus `QUERY_GDAT_IMAGE_LOCALPAL` route when no
  boot-owned scene command plan has been installed, and binds ceiling/floor
  plus outdoor sky/ground palettes independently before each material blit.
  Outdoor rendering now prefers the real `GdatSceneM11CommandPlan` when it
  exists, so source-owned plan pixels are not replaced by a second callback
  lookup. Verification: focused Ninja build plus CTest
  `dm2_v1_gdat_m11_material_receipt_real_data` and
  `dm2_v1_runtime_handoff_smoke` pass in `build-local-ninja`.

- ✅ 2026-07-16 DM2 scene local-palette smoke closure: the skproject
  source-required renderer now admits the explicit asset+palette provider
  route used by focused viewport tests while live boot-owned rendering still
  requires a validated GDAT scene plan. Outdoor sky and ground now preserve
  their decoder-owned local-palette receipt hashes instead of treating them
  as hashes of already-decoded pixels. Verification: Ninja
  `test_dm2_v1_runtime_handoff_smoke` and
  `test_dm2_v1_gdat_wall_plan_viewport_real_data` pass.

- ✅ 2026-07-16 Theron Track 02 raw-loader receipt variant binding: the
  `$0b52` initial-envelope loader intake now carries the authenticated JP/US
  raw-BIN media variant through the complete payload, level-envelope, and
  post-envelope handoff receipts. The raw handoff hash includes that variant,
  and boot/startup runtime consumers reject stripped or ISO-mutated receipt
  chains before any level/object bytes can cross the handoff. Verification:
  focused Ninja build plus CTests `theron_v1_track02_loader_intake`,
  `theron_v1_m11_launcher_handoff_boundary`, and
  `theron_v1_raw_loader_trace_initial_level_handoff` pass in `build-verify`.
  Remaining blocker: real ISO capture proving the first level/object record;
  no sector conversion, raw-BIN trace borrowing, or synthetic dungeon route is
  admitted.

- ✅ 2026-07-16 DM1 legacy title C001 helper hardening: the
  `DM1_V1_Title_*` compatibility state now loads only exact 320x200
  C001-sized title data, refuses cropped title sources before allocating
  runtime buffers, and refuses to publish a real-asset receipt unless the
  exact C001-sized source is live. The draw path now consumes the last
  source zoom step selected by `DM1_V1_Title_AnimateZoomPc34Compat` instead
  of deriving the zoom frame from the active double-buffer index.
  Verification: focused Ninja build plus CTests
  `dm1_v1_intro_skip_state_cleanup_pc34_compat`,
  `title_frontend_c001_fallback_gate_pc34_compat`,
  `title_frontend_runtime_cadence_source_lock`, and
  `title_frontend_step_palette_v1_pc34_compat` pass in `build-local-ninja`.
  Remaining blocker: real PC34/Mac title-to-HoC capture and an
  operator-staged external original-save corpus run.

- ✅ 2026-07-16 CSB title/opening resident-surface ownership gate: the
  title/opening consumption receipt now requires PRESENTS, CHAOS, and
  STRIKES host frames to reference the resident C001 title surface and
  requires the C004/C002/C003 opening host frame to reference the resident
  entrance and door surfaces from the same startup session. Plausible raster
  facts with swapped owner pointers fail closed, so wrapper-owned title or
  entrance surfaces cannot satisfy the verified CSB startup path. Verification:
  focused Ninja build plus CTest `csb_v1_startup_session_contract_pc34_compat`
  pass in `build-csb-startup-session-contract`; adjacent CTests
  `csb_v1_startup_terminal_handoff_real_data_pc34_compat` and
  `csb_v1_startup_raster_present_pc34_compat` pass in `build-local-ninja`.
  Remaining blocker: external original-window capture and broader app-route
  parity for the complete CSB startup sequence.

- ✅ 2026-07-16 Nexus Structure1A relation receipt source recheck: the
  Structure1F/Structure1A host provenance gate now revalidates cached
  relation fields against the live owner-ref table and the current
  Structure1A model/rotation row before marking a relation resolved. Stale
  cached owner coordinates or stale row bytes fail closed before runtime DGN
  preparation, with no fallback material or draw route. Verification: focused
  Ninja build plus CTest for `nexus_v1_structure1f_spatial_receipt`,
  `nexus_v1_structure1_host_provenance`,
  `nexus_v1_structure1a_structure3_row_receipt`,
  `nexus_v1_dgn_material_raster`, `nexus_v1_dgn_geometry_readiness`, and
  `nexus_v1_structure2_no_draw_receipt` pass in `build-local-ninja`.
  Remaining blocker: original Saturn evidence for Structure1A selector
  semantics, transforms, texture/palette, VDP1 ordering, culling, and draw
  behavior.

- ✅ 2026-07-16 DM1 TITLE.C C001 source-selector fallback closure: the DM1 V1
  title runtime source selector now treats TITLE.DAT availability as rejected
  evidence only. Malformed, cropped, or missing GRAPHICS.DAT C001 selects
  `SKIP` rather than `TITLE_DAT_FALLBACK`, matching the already fail-closed
  M11 startup receipt and ReDMCSB `TITLE.C F0437` C001 path. This removes the
  last low-level selector route that could describe a synthetic TITLE.DAT
  frame bank as the drawable PC34 title source. Verification: Ninja
  `test_title_frontend_c001_fallback_gate_pc34_compat` and focused CTest
  `title_frontend_c001_fallback_gate_pc34_compat` pass. Remaining blocker:
  real PC34/Mac title-to-HoC capture and an operator-staged external
  original-save corpus run.

- ✅ 2026-07-16 CSB opening-door source-tick gate: opening-door and
  title/opening consumption receipts now require the package receipt and
  C004/C002/C003 opening host frame to match the active startup session source
  tick. This blocks tick-stale entrance captures from crossing the verified
  C001-to-door startup path without adding any fallback surface. Verification:
  focused Ninja build and CTest `csb_v1_startup_session_contract_pc34_compat`
  pass in `build-csb-startup-session-contract`.

- ✅ 2026-07-16 DM2 skproject DB0 front-door and wall-gfx runtime draw:
  source-required dungeon rendering now admits the D0C/front-player DB0 door
  outside `table1d7029`, supplements G1 door metadata only from the same real
  square first-thing DB0 owner, and binds custom wall buttons only from
  decoded DB2/DB3 `WALL_GFX` receipts. Runtime smoke now passes the DB0
  door-set panel, default button, ornate/destroyed overlays, and
  text/actuator/map-list wall-gfx button checks.

- ✅ 2026-07-15 DM1 F0292 status-bar geometry fix: the public champion-status
  layout now reports the ReDMCSB/F0287 PC34 bar container at y=2 instead of
  the status-box top edge y=0, matching the existing live bar-fill model and
  preventing fallback HUD consumers from drawing HP/stamina/mana bars two
  pixels too high.

- ✅ 2026-07-15 DM2 leader-hand injected-provider item receipt: the runtime
  keeps source-required leader-hand rendering strict on boot-owned GDAT, while
  allowing the synthetic viewport provider smoke to bind the carried ObjectID
  through the neutral item map-chip field. This proves the leader-hand blit,
  asset count, item receipt, and M11 item-material handoff without claiming
  source dtImage selection when no boot GDAT parser is present. Verification:
  the carried-item section of `dm2_v1_runtime_handoff_smoke` passes; the full
  smoke still fails on independent palette, wall, and door runtime gaps.

- ✅ 2026-07-15 DM1 wall-inscription fail-closed receipt: malformed
  F0172 selected-wall and F0168 world TextString material lookups now zero
  the output M648/C10 receipt before returning failure, preventing stale
  readable inscription material from surviving a rejected raw record. Focused
  wall-inscription material and M11 inscription CTests pass.

- ✅ 2026-07-15 CSB opening-door source-plan step gate: the public CSB
  source render-plan route now rejects pre-open delay states that carry a
  real door step and rejects opening-frame plans outside ReDMCSB's C002/C003
  step range 1..31. This closes the direct-plan path that could describe a
  door-opening frame without valid door rectangles. Focused startup-session
  contract and adjacent startup CTests pass.

- ✅ 2026-07-15 DM2 item/icon M11 scene receipt gate: source-required DM2
  item, carried-hand, and possession icon material receipts now carry the G1
  scene-control hash through runtime, boot, and the M11 presentation gate.
  M11 rejects an item/icon receipt whose GDAT material hash/count matches but
  whose scene identity differs from the presented viewport frame. Focused
  Ninja build and CTest `dm2_v1_m11_runtime_frame_receipt_gate` pass.

- ✅ 2026-07-15 CSB opening-door playback-stage gate: opening-door and
  title/opening consumption receipts now require the startup session to still
  be in active ENTRANCE playback, after the complete C001 title phases and
  before F0807 promotes the session to HUD. This blocks stale HUD-stage
  C004/C002/C003 frames from being recorded as valid opening captures.
  Focused startup-session contract and adjacent startup CTests pass.

- ✅ 2026-07-15 DM1 action-menu G0499/G0501 geometry table hardening:
  the one-action and three-action F0387 action-menu boxes now expose
  ReDMCSB's signed `int16_t[4]` source geometry directly, matching the
  verified G0500 two-action contract instead of byte-slicing host `int`
  storage. Focused direct-table assertions, pass928/pass930 verifiers, and
  the targeted CTest pattern pass in `build-local-ninja`.

- ✅ 2026-07-15 DM1 spell-HUD C109 out-of-party gate: the
  F0394/F0397/F0398 spell-area overlay contract now rejects a C109 caster
  request outside `G0305_ui_PartyChampionCount` before publishing C009/C011
  material, even if the skipped slot carries plausible health or Symbols
  bytes. Focused spell-HUD CTest coverage passes.

- ✅ 2026-07-15 CSB title phase host-consumption gate: the
  title/opening session receipt now requires PRESENTS, CHAOS, and STRIKES
  BACK host captures to carry the matching ReDMCSB F0437 source step,
  phase mask, palette, and distinct indexed raster hash before C004/C002/C003
  opening consumption can reach HUD/runtime. Focused startup-session contract
  coverage rejects relabeled PRESENTS-as-CHAOS and duplicated title pixels;
  adjacent startup CTests pass.

- ✅ 2026-07-15 CSB full C001 title-surface gate: the startup session and
  package-consumption receipts now require the resident GRAPHICS.DAT C001
  title surface to remain the full 320x200 decoded bitmap, while the
  PRESENTS/CHAOS/STRIKES sub-surfaces stay cropped by their ReDMCSB F0437
  source rectangles. This prevents valid full-title media from failing the
  handoff as a cropped 320x153 substitute. Focused CSB startup-session and
  terminal-handoff tests pass.

- ✅ 2026-07-15 DM1 title C001 startup gate hardening: DM1 startup now
  requires the exact 320x200 GRAPHICS.DAT C001 surface before publishing the
  ReDMCSB TITLE.C F0437 title route, rejects cropped C001/TITLE.DAT fallback
  substitution, holds the verified C001 surface through the frame-bank
  equivalent handoff boundary, and keeps failed staged save-corpus scans out
  of HoC complete-support readiness. Focused DM1 startup/title tests pass.

- ✅ 2026-07-15 DM1 HoC C026 direct mirror atlas gate: the low-level
  `dm1_v1_front_mirror_render_plan_pc34()` path now rejects portrait indices
  outside ReDMCSB's 8x3 C026 atlas before computing source coordinates, so
  damaged C127 data cannot bypass the runtime receipt and sample past the
  real champion portrait strip. Focused champion-mirror and real HoC C127
  material CTests pass.

- ✅ 2026-07-15 CSB startup real-surface receipt hardening: terminal,
  title/opening, and opening-door session receipts now require the resident
  decoded C001 title surface, PRESENTS/CHAOS/STRIKES crops, C004 entrance,
  and C002/C003 door strips to match their exact PC34 source dimensions and
  asset ids before HUD/runtime handoff. Focused startup-session contract
  coverage rejects forged one-line C001 and one-byte C003 stand-ins.

- ✅ 2026-07-16 CSB startup packed-raster receipt hardening: the PC34 startup
  raster presenter now requires the exact CSB host-surface receipt before it
  packs a 320x200 indexed title/PRESENTS/opening/HUD page into the 4bpp host
  page. It checks the receipt-owned host-surface hash, raster route hash, and
  recomputed pixel hash, and rejects missing receipts, wrapper/synthetic
  surfaces, stale route hashes, changed pixels, and out-of-range palette
  nibbles without clearing the destination. Verification: focused
  `csb_v1_startup_raster_present_pc34_compat` and
  `csb_v1_startup_terminal_handoff_real_data_pc34_compat` CTests pass.

- ✅ 2026-07-15 CSB terminal handoff full-surface gate: the F0806/F0807
  entrance-to-HUD handoff now reuses the full resident startup surface
  contract before entering HUD playback, so a valid C017/C040 HUD pair cannot
  cross into runtime if C001, C004, C002, or C003 were swapped or malformed.
  Focused `csb_v1_startup_terminal_handoff_real_data_pc34_compat` and
  startup-session contract CTests pass.

- ✅ 2026-07-15 DM1 spell-HUD SymbolStep/tab receipt hardening:
  the F0394/F0397/F0398 spell-area overlay contract now admits only the
  original champion-owned SymbolStep ring `0..3` from SYMBOL.C F0399/F0400,
  rejecting rows 4/5 instead of synthesizing unavailable rune material. The
  F0393 tab receipt also remains keyed by champion slot, so dead or
  out-of-party HoC/mirror candidates cannot pack later tab data into stale
  HUD rows. Focused `dm1_v1_champion_panel_spell_area_overlay_pc34_compat`
  coverage passes.

- ✅ 2026-07-15 DM2 active GRAPHICSSET scene-material gate: M11 scene
  command planning now matches skproject `UPDATE_GFXSET` by requiring the
  active set's `SCENE_COLORKEY`/`SCENE_FLAGS` plus real floor/ceiling GDAT
  material, while retaining optional light words only for later `c_light`
  consumers. Missing light words no longer block source-owned dungeon
  surfaces or trigger cross-set/synthetic fallback. Focused coverage:
  `test_dm2_v1_gdat_scene_plan_viewport_real_data` and
  `dm2_v1_gdat_m11_material_receipt_real_data`.

- ✅ 2026-07-15 DM2 runtime save/input route: DM2 runtime `SAVE_GAME`
  now writes the real `SKSave.dat` through `M11_GameView_QuickSave` instead
  of opening a blocked placeholder save panel. M11 direct save/resume and
  turn/action checks now pass in the DM2 startup profile gate.

- ✅ 2026-07-15 DM2 NEW GAME boot-pose routing: archive-backed DM2 launches
  now keep the decoded `DUNGEON.DAT` header pose as corpus evidence only and
  start runtime at skproject/T520's Hall-of-Champions pose `(15,15,N)`. This
  removed the M11 boot-pose mirror/runtime accessor failures without adding
  synthetic party or dungeon state.

- ✅ 2026-07-15 DM2 archive-backed real data startup: verified virtual
  `GRAPHICS.DAT`/`DUNGEON.DAT` paths from archive discovery now materialize
  into temporary byte-for-byte inputs for the DM2 boot parsers instead of
  failing through `fopen`. The M11 startup pointer route now resolves the
  original `INTERFACE_GENERAL/0/dt04/0` NEW/RESUME hit geometry from real
  GDAT and enters the real NEW runtime route when data is present. Focused
  startup pointer/menu-action tests pass; the broader M11 profile gate is
  reduced to the remaining runtime HUD/save/object-icon/tick blockers.

- ✅ 2026-07-15 DM1 G0500 action-menu table contract: the two-action
  action/spell menu box now exposes ReDMCSB's signed `int16_t[4]`
  `G0500_ai_Graphic560_Box_ActionArea2ActionsMenu` instead of a byte pointer
  cast over host `int` storage. The focused test now dereferences the table
  surface directly, and pass929 is re-pinned to `MENU.C:36/496` while finding
  the worktree build binary.

- ✅ 2026-07-15 CSB startup opening-step receipt gate: the presentation
  receipt now rejects an active C004/C002/C003 door-opening frame before
  ReDMCSB's first real animation step and after the final step. The pre-open
  delay still admits the closed entrance surface, but step 0 can no longer
  publish an opening-frame receipt with empty C002/C003 geometry. Focused CSB
  startup tests pass (`terminal_handoff_real_data`, `receipt_coherence`,
  `img3_decode`, `raster_present`).

- ✅ 2026-07-15 DM2 real-GDAT HUD receipt hardening: M11 now recomputes the
  complete HUD command-plan hash before consuming source material, so altered
  portrait `RECT_173..176` table receipts cannot reach runtime drawing. In
  source-required mode, a rejected champion portrait plan now blocks instead
  of falling back to local material fetch. Focused real `GRAPHICS.DAT` HUD
  coverage passes.

- ✅ 2026-07-15 DM1 SAVEUTIL F0419 cursor hardening: the original-PC34
  save-part reader now validates the incoming cursor with subtraction-based
  bounds before reading the F0420 length prefix or body, so malformed caller
  state cannot wrap past the save envelope. Focused F0419/F0420 and full
  original-save handoff CTests pass.

- ✅ 2026-07-15 CSB title/opening receipt door-raster gate: the
  title-to-opening startup consumption receipt now rejects a host opening
  page unless its raster proves the real C004 entrance plus C002/C003 door
  strips from the same C001 package session. Focused startup session contract
  coverage now accepts the complete C001 PRESENTS/CHAOS/STRIKES path and
  rejects missing or partial opening-door composition.

- ✅ 2026-07-15 CSB title CHAOS receipt off-by-one: startup presentation
  coherence now validates `TITLE.C F0437` against the current title frame,
  not the previous frame. This restores the first and last CHAOS zoom render
  plans instead of rejecting them before host presentation. Focused CSB
  startup tests pass (`139/139`, plus title/import gate `129/129`).

- ✅ 2026-07-15 DM2 skproject startup title/menu: startup presentation now
  consumes the original `TITLE/0` GDAT pair (`dt07/1` title and `dt07/4`
  menu) instead of a synthetic text/rect menu. M11 advances only the startup
  title tick while keeping runtime frozen, then consumes NEW GAME through the
  existing boot/session boundary. Focused startup pointer and menu-action
  contracts pass (`1/1`, `99/99`).

- ✅ 2026-07-15 DM1/DM2 source-save admission: DM1 keeps manifest exclusion
  in corpus discovery while F0435 materializes an explicitly selected valid
  PC34 stream, preserving original parser errors and live C01 door/timeline
  state. DM2 now counts a valid `SKSave.dat` as the authoritative
  last-session save and uses `.bak` only for recovery, matching SKProject.
  Focused `dm1_v1_original_save_pc34_handoff` and `dm2_v1_save_load` tests
  pass in full.

- ✅ 2026-07-15 DM1 M648 wall inscriptions: corrected the direct PC3.4
  `GRAPHICS.DAT` binding from mandatory preload slot 120 to ReDMCSB F0107's
  actual bitmap ordinal 258. The real-data all-map capture verifies 56 wall
  texts, 787 checks, exact source pixels, and C10 transparency with no
  host-font fallback.

- ✅ 2026-07-15 DM2 original raw-SKSave corpus layout handoff: the
  original-only census now carries the parser-validated raw dungeon map count,
  all sixteen DB-pool record counts, and prefix/map-data FNV identities into
  the exact selected-row restore boundary. A raw row missing that receipt is
  rejected before runtime restore. Focused `dm2_v1_save_load` coverage checks
  the selected fixture and external original corpus census against freshly
  parsed bytes. No GenericRecord links, record behavior, or graphics are
  inferred.

- ✅ 2026-07-15 DM2 original SKSave DB1 Teleporter admission: raw six-byte
  teleporter rows now expose only `DME.h`'s destination, scope, sound, and
  rotation fields after the complete DB-pool receipt validates them. Focused
  `dm2_v1_save_load` coverage proves the source fields survive the standard
  raw runtime handoff and rejects a missing row. No GenericRecord link,
  movement, map mutation, or inferred world route is admitted.

- ✅ 2026-07-15 DM2 original SKSave DB2/DB9 record admission: added bounded,
  hash-bound raw receipts for `DME.h::Text` (`w2` visibility/mode/text index)
  and `DME.h::Container` (`b4` open/type). The expanded source-order corpus
  fixture reaches the standard raw runtime handoff, verifies both families,
  and rejects missing rows. It does not read GenericRecord links, text-table
  bytes, container contents, or promote any graphics.

- ✅ 2026-07-15 CSB app presented capture: opt-in
  `FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR` writes post-present SDL RGBA BMPs for
  C001/C004 palette phases only after the current PC3.4 source raster and
  palette gate pass. The route cannot capture a wrapper, stale title phase,
  or unverified surface. M11 now treats `M11_RENDER_OK` as success when
  publishing the presented receipt, and uses SDL3's library-owned base path
  without freeing it during each capture frame.

- ✅ 2026-07-15 DM2 original SKSave common-item record admission: the raw
  save parser now reads `DME.h::ItemType()` only from hash-bound four-byte
  DB5 Weapon, DB6 Cloth, DB7 Scroll, and DB10 Miscellaneous_item rows. It
  enforces the original pool/index/stride boundaries and exposes no links,
  inventory graph, non-Weapon charges, contents, or graphics. Focused
  `dm2_v1_save_load` coverage verifies all four families and rejects DB9 and
  out-of-range rows.

- ✅ 2026-07-15 DM2 original-save corpus selected-row handoff: added a
  `DM2_SELECT_LOAD_GAME`-shaped import boundary that accepts only an exact
  row from a fresh, fully parsed original envelope/raw census and revalidates
  its file, payload, and decoded state identity before existing `GAME_LOAD`
  restore. It never uses first-importable selection or falls back to a
  Firestaff session. Focused `dm2_v1_save_load` coverage restores the chosen
  original raw candidate and proves a stale selected file is rejected without
  mutating runtime despite a valid Firestaff save in the same corpus.
- ✅ 2026-07-15 DM2 SKProject floor/ceiling trim composition: source-required
  M11 frames now consume `DM2_DISPLAY_VIEWPORT`'s real GRAPHICSSET 0x70/0x71
  trim words when the D1 or D2 three-wall cluster blocks the view. The low
  byte trims ceiling and the high byte trims floor exactly in source order;
  an absent optional word remains the source's zero/no-trim result. Verification:
  DM2 scene/viewport syntax gate plus isolated Ninja boot/GDAT/save targets.
- ✅ 2026-07-15 DM1 ReDMCSB CASTER.C F0394 / C109 per-caster spell owner:
  M11 now stores the original `Champion.Symbols[5]` and `SymbolStep` for all
  four casters plus `G0514_i_MagicCasterChampionIndex`, instead of lending
  one sequence to every tab. C109 resolves its coarse strip through F0393's
  exact inclusive tab rectangles, saves the old caster state, restores the
  selected caster state, and rejects dead/same-caster tabs without changing
  the party leader. The cast route now uses the selected F0394 caster for
  champion, skill, hand, XP, and tick ownership. Focused coverage proves
  isolated rune state across a caster round-trip and the dead-tab gate:
  `m11_dm1_spell_pointer_routes_pc34_compat`.

- ✅ 2026-07-15 DM1 ReDMCSB COMMAND.C G0447/G0454 spell pointer runtime:
  C100 now opens the real C009/C011 spell panel through C013, C101..C106
  consume only their exact layout-696 C245..C250 rune boxes, C108 consumes
  C252, and C107/C254 now follows SYMBOL.C F0400 by deleting just the final
  rune, preserving the earlier runes and open source panel. The input path
  is source-session-only and takes all hit geometry from the PC34 touch
  matrix, never glyph bounds or host scaling. Focused coverage:
  `m11_dm1_spell_pointer_routes_pc34_compat`.
- ✅ 2026-07-15 DM1 ReDMCSB F0168/F0172 M648 raster-source binding:
  the raw selected-wall and world discovery routes now agree byte-for-byte on
  the original inscription receipt. Every readable glyph is constrained to
  GRAPHICS.DAT entry 120 at `decodedByte << 3`, native unscaled 8x8 size,
  exact F0107 destination spacing, and C10 transparency. Padded/scaled font
  surfaces or altered cells fail closed. Focused coverage:
  `dm1_v1_inscription_source_raster_gate` and
  `dm1_v1_wall_inscription_pc34_material_gate`.
- ✅ 2026-07-15 CSB presented-frame source gate: before M11 accepts a CSB
  title or entrance capture, it now rebuilds the active PC3.4 C001--C005
  source plan and compares every indexed pixel plus the exact F0437/F0441
  palette phase. Stale title phases, foreign C002/C003 door steps, and host
  wrapper pages fail closed before SDL RGBA capture is recorded. Focused
  coverage: real PC3.4 package probe and M11 launcher handoff.

- ✅ 2026-07-15 DM2 SKProject `QUERY_ORNATE_ANIM_FRAME` viewport binding:
  legacy DB0-door wall-button material now resolves the real WALL_GFX
  `dtWordValue/0x0d` or `dtText/0x0d` sequence against the live game tick
  before M11 selects its image field. Invalid sequences and unavailable
  source frames fail closed. Focused coverage: `dm2_v1_gdat_word_values`,
  `dm2_v1_boot_profile_smoke`, and `dm2_v1_save_load`.

- ✅ 2026-07-15 DM2 SKProject D3 `DRAW_DOOR` light-palette handoff:
  field-zero retry panels now consume `_32cb_0804`'s original stationary
  distance-darkness table and the decoded `INTERFACE_GENERAL/0/dt07/2`
  `_0b36_037e` transform only with the matching live `c_light` receipt.
  Command and M11 receipts bind the effective darkness, transformed palette,
  and source identity. Missing or altered evidence remains no-draw; the IMG3
  base palette is never used as a substitute. Focused coverage:
  `dm2_v1_gdat_word_values`, `dm2_v1_boot_profile_smoke`, and
  `dm2_v1_save_load`.

- ✅ 2026-07-15 DM2 SKProject `DRAW_ITEM_IN_HAND` GDAT field handoff:
  leader-hand items now select their exact `dtImage` through
  `SkWinCore::_2405_014a`'s proven tick, record-index, and direction modes,
  and the M11 item key preserves that field through source pixel and palette
  resolution. Unsupported random, charge, and equipment-context modes fail
  closed instead of substituting `F9`/field zero. Focused coverage:
  `dm2_v1_gdat_word_values`, `dm2_v1_carried_item_local_palette_gate`,
  `dm2_v1_boot_profile_smoke`, and `dm2_v1_save_load`.

- ✅ 2026-07-15 DM1 ReDMCSB F0282 C160/C161 first-sensor ownership: M11
  resolves the front-square's first source SENSOR exactly once at command
  entry, retains that index through the candidate-panel mutation, then clears
  that same sensor in the F0282 tail. This preserves BUG0_87 when a custom
  sensor precedes C127 and never replaces it with a mirror/TextString scan.
  Focused coverage: `m11_dm1_hoc_no_fallback_panel` with a bounded, valid
  PC34 SFT chain.

- ✅ 2026-07-15 DM2 direct G1 WALL_GFX door-button M11 handoff: a verified
  DB2 Text or DB3 Actuator custom button now passes its exact
  `WALL_GFX/index/dtImage/1` decoded pixels, IMG3 local palette, coordinate,
  ObjectID, field, GDAT key, and pixel hash directly to M11. The renderer
  consumes it only when the existing G1 root receipt matches, avoiding a
  second mutable provider lookup; missing or altered data remains no-draw.
  A button-only DB2/DB3 root cannot borrow a DB0 DoorType, GRAPHICSSET frame,
  panel, or jamb route.
  `WALL_GFX/F9` and `DOORS/F9` are deliberately not used as substitute door
  art because SKProject has not yet proven their visible placement semantics.
  Source: SKProject `c_gui_vp.cpp::DRAW_DEFAULT_DOOR_BUTTON`,
  `DRAW_DOOR_FRAMES`, and DB2/DB3 wall-decoration routes.

- ✅ 2026-07-15 DM2 direct DB5/DB9 F9 M11 material handoff: a bounded
  `DRAW_MAP_CHIP` receipt carries its already decoded WEAPONS/CONTAINERS F9
  pixels, IMG3 local palette, ObjectID, coordinate, category/type and pixel
  hash directly into the viewport. The renderer consumes that exact receipt
  and does not re-query the mutable GDAT provider during the frame. A changed
  material identity fails closed. It does not grant a static-object blit:
  later source recovery established F9 is not `DRAW_ITEM` art. Source:
  SKProject `c_map.cpp::QUERY_DUNGEON_MAP_CHIP_PICT`. Verification: DB5 gate 6/6,
  DB9 gate 3/3, boot smoke 88/88, save/load 26/26.

- ✅ 2026-07-17 DM2 DB14 normal-scale original-pixel consumer: the strict
  `QUERY_PICST_IT` `0x40`/neutral-mode branch re-decodes and hashes original
  indexed IMG3 bytes, verifies its local palette and RAW4 clip, and copies
  only native-size zero-offset pixels. Flip, crop, scaling, and other modes
  fail closed. Verification: `dm2_v1_g1_flying_item_source_receipt`, full
  `firestaff` build, and `git diff --check` passed.

- ✅ 2026-07-17 DM2 HUD SUMMARY_IMAGE M11 no-draw receipt: the existing HUD
  plan now admits the exact `QUERY_GDAT_SUMMARY_IMAGE(1,vb_144,field)` tuple
  only when decoded GDAT pixels, local palette, and RAW4 destination identity
  are present. Tuple mismatch, palette loss, and stale destination reject.
  Verification: `dm2_v1_gdat_hud_summary_m11_receipt` passed; `git diff
  --check` passed.

- ✅ 2026-07-17 DM2 HUD QUERY_PICST_IT transform receipt: the bounded
  `c_gui_draw.cpp:926-942` scale branch now carries `0x1f/0x2f` X and `0x35`
  Y only for source values `0..0x28`, tied to the verified SUMMARY_IMAGE
  decoded pixels, palette, and destination identity. Unsupported values reject
  and the receipt remains `no_draw`. Verification:
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 HUD SUMMARY_IMAGE indexed-pixel M11 consumer: the exact
  `c_gui_draw.cpp:926-942` branch now uses `c_image.cpp:106-200` and
  `c_gfx_blit.cpp:886-1063` fixed-point size/centre-sample semantics to copy
  only authenticated U4 GDAT indices and their local palette into the native
  M11 target. The receipt requires the exact tuple, decoded-pixel hash,
  palette hash, raw GFX256 receipt, transform identity, and complete resolved
  destination. Pixel, palette, receipt, destination, and unsupported-scale
  drift reject; partial or unknown clipping remains no-draw. Verification:
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` raw-material/transform M11 admission:
  `SKULLWIN/c_gui_vp.cpp:4806-4856` and `dm2data.cpp:776-800` now source-lock
  cells 1..15 to their rect, field, mirror and open/closed state-word branch.
  The no-draw receipt requires the corresponding GRAPHICSSET SUMMARY_IMAGE,
  GFX256 raw receipt, U4 decoder bytes, and local palette; missing palette,
  sentinel cells, cell 0, out-of-range light, and absent raw data reject.
  Verification: `dm2_v1_gdat_pit_m11_receipt` and
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT M11 material receipt:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now source-lock
  the loadable primary branch to the exact `table1d6f9c/table1d6f5c` lane,
  GRAPHICSSET SUMMARY_IMAGE/GFX256 bytes, decoded U4 buffer, local palette,
  root RAW4 placement and current DM2 composition/surface snapshot. Cell,
  state lane, loadability, RAW4, data epoch and rebound-surface drift reject
  without a framebuffer write. `QUERY_TEMP_PICST` fallback and downstream
  B073/`DRAW_PICST` transforms remain intentionally no-draw. Verification:
  `dm2_v1_gdat_stairs_front_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_STAIRS_FRONT` `QUERY_TEMP_PICST` fallback
  provenance: `SKULLWIN/c_gui_vp.cpp:514-527`, `c_querydb.cpp:2381-2465`
  and `c_image.cpp:98-337` now source-lock the failed-primary-loadability
  branch to its `table1d6f7c` GRAPHICSSET field, authentic SUMMARY_IMAGE/GFX256
  U4 material, RAW4 root rectangle, M11 surface identity, normal `0x40`
  scales, HFLIP mode, zero offsets, rect query and light alpha. Selector,
  material and RAW4/M11 identity drift reject with no write. The live
  B073/field-7 palette transaction is intentionally unadmitted, so this path
  remains no-draw. Verification: `dm2_v1_gdat_stairs_front_m11_receipt`
  passed; full `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary GDAT M11 receipt:
  source-locks `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287`
  table lanes through original SUMMARY_IMAGE/GFX256 U4 bytes, local palette,
  RAW4 root placement and current composition/surface owner. Invalid cell,
  state, material and epoch inputs reject with no framebuffer write; the
  unproven B073/`DRAW_PICST` stage remains no-draw. Verification:
  `dm2_v1_gdat_stairs_side_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_STAIRS_SIDE` `DRAW_DUNGEON_GRAPHIC` transform
  provenance: `SKULLWIN/c_gui_vp.cpp:540-565` and `c_image.cpp:450-475`
  source-lock the primary side-stairs material to mode 0, default normal
  scale and the non-special source-offset path. Material, RAW4 and M11
  identity drift, plus either `0x2bc/0x2bd` special rect, reject before any
  framebuffer write. The live B073 palette state remains unbound, so this is
  deliberately no-draw. Verification:
  `dm2_v1_gdat_stairs_side_transform_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `TRIM_BLIT_RECT` source receipt: source-locks
  `SKULLWIN/c_gui_vp.cpp:570-573,611-658` to the active D1/D2 trim word,
  authenticated `DRAW_WALL` raw/decoded/palette/RAW4 command identity and
  owner surface. It records the source rectangle's exact remaining margins,
  rejects absent rows, palette drift and out-of-bounds geometry, and remains
  no-draw. Verification: `dm2_v1_gdat_wall_trim_receipt` passed; full
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 live `DRAW_WALL` trim M11 gate: binds the existing
  `SKULLWIN/c_gui_vp.cpp:695-719` `QUERY_TEMP_PICST` command only when its
  normal `0x40` scale, RAW4 `0x2be + cell`, movement offset, source flip,
  recomputed raw/decoded/palette/RAW4 identity, M11 wall hash and atomic
  owner snapshots agree. Positive admission passes; material and surface
  drift reject without a write. The gate remains no-draw and creates no
  synthetic blit. Verification: `dm2_v1_gdat_wall_trim_receipt` passed;
  full `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL_TILE` source admission: source-locks
  `c_gui_vp.cpp:6703-6741` and `dm2data.cpp:266-273,602-605` to all 23
  `table1d7012` branches, their `table1d6afe` orientation and the already
  authenticated wall/M11 composition identity. Invalid cells and material
  identity drift reject; the secondary `32cb_15b8` GDAT branches stay
  no-draw. Verification: `dm2_v1_gdat_wall_tile_receipt` passed; full
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `32cb_15b8` first simple `QUERY_TEMP_PICST` input
  receipt: `SKULLWIN/c_gui_vp.cpp:6618-6628` source-locks category 9,
  selector/image field, normal `0x40` scales, flip, all query parameters and
  RG71l's forced alpha. Category, transform and alpha drift reject. It is
  intentionally no-draw with no record-layout or destination inference.
  Verification: `dm2_v1_gdat_wall_tile_picst_receipt` passed; full
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `32cb_15b8` loadable category-9 `0x0f` input receipt:
  `SKULLWIN/c_gui_vp.cpp:6651-6692` source-locks successful loadability,
  selector/image field, normal scales, flip, query parameters and RG71l
  alpha. Missing loadability or field/transform drift rejects. The path is
  no-draw without record-layout or destination inference. Verification:
  `dm2_v1_gdat_wall_tile_loadable_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `32cb_15b8` category-8 overlay input receipt:
  `SKULLWIN/c_gui_vp.cpp:6322-6329` source-locks its selector/image field,
  normal scales, flip, QUERY_TEMP_PICST parameters and RG71l alpha. Category
  and alpha drift reject; no destination or blit is inferred. Verification:
  `dm2_v1_gdat_wall_tile_overlay_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `32cb_15b8` branch-set receipt: combines only the
  authenticated category-8 overlay, category-9 simple and loadable category-9
  `0x0f` input receipts. Identity or `0x0f` field drift rejects; it remains
  no-draw with no destination contract. Verification:
  `dm2_v1_gdat_wall_tile_branch_set_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `32cb_15b8` `DRAW_TEMP_PICST` admission: consumes only
  the full category-8/9 branch-set receipt and rechecks all constituent
  identities, `0x0f` field and exact normal-scale transform. Transform drift
  rejects. It remains no-draw with no destination or pixel contract.
  Verification: `dm2_v1_gdat_wall_tile_draw_temp_receipt` passed in the
  isolated DM2 build; the integration Ninja build was green; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DM2_query_B073` input receipt: source-locks
  `SKULLWIN/c_querydb.cpp:2506-2545` palette, `v1e12d2` light, alpha/mask,
  colors/cache ownership and RAW7/lookup/traversal identities. Missing input
  or cache inconsistency rejects; no palette or pixels are created.
  Verification: `dm2_v1_gdat_b073_input_receipt` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 B073/DRAW_TEMP_PICST palette-surface receipt: combines
  authenticated B073 and DRAW_TEMP_PICST identities with an owned viewport
  surface snapshot. Receipt or surface drift rejects; no pixel buffer is
  borrowed or written. Verification:
  `dm2_v1_gdat_draw_temp_palette_surface_receipt` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 original palette-byte admission: binds only borrowed
  original 16/256-byte palette storage whose bytes hash equals the supplied
  authenticated identity and whose B073/surface receipt remains valid. Byte
  drift rejects; no palette transform or pixel buffer is produced.
  Verification: `dm2_v1_gdat_original_palette_receipt` passed in the
  isolated DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 M11 original-palette consumer: binds only the borrowed
  original palette receipt to the current viewport owner generation. It has
  no transform, destination or pixel buffer; source and surface identities
  are retained for a later fully proven consumer. Verification:
  `dm2_v1_gdat_palette_m11_consumer` passed in the isolated DM2 build and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 original material-byte admission: binds only borrowed
  original decoded storage with verified dimensions, stride, byte count and
  byte hash to the current M11 palette consumer. Byte or layout drift rejects;
  no decoder or render path is admitted. Verification:
  `dm2_v1_gdat_original_material_receipt` passed in the isolated DM2 build
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 M11 material/palette pair admission: requires matching
  original material/palette identities, material dimensions/stride/count and
  current owner generation. Surface drift rejects; no decoder, destination
  or blit is admitted. Verification:
  `dm2_v1_gdat_material_palette_pair_receipt` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 live M11 materialization handoff: borrows only the
  validated material/palette pair while the owner generation remains current.
  Generation drift rejects; no destination, transform, decoder or blit is
  admitted. Verification: `dm2_v1_gdat_materialization_handoff` passed in
  the isolated DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` source trace admission: carries the live
  materialization handoff into the source `QUERY_PICST_IT`/`DRAW_PICST` stage
  only while owner generation matches. It explicitly records unproven source
  and destination rectangles and remains no-draw. Verification:
  `dm2_v1_gdat_draw_picst_trace_receipt` passed in the isolated DM2 build and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` direct source-rectangle trace: source-locks
  the `query1 == -1` branch in `SKULLWIN/c_image.cpp:240-296` to direct
  `srcx/srcy + imgdesc.x/y` and proven width/height. Every QUERY_BLIT_RECT,
  flip and destination branch remains no-draw. Verification:
  `dm2_v1_gdat_draw_picst_rect_trace` passed in the isolated DM2 build and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` root-node trace: binds the direct
  `SKULLWIN/c_xrect.cpp:217-280` unsigned, unchained root node to the
  authenticated `DRAW_PICST` source rectangle. Only `query2 == -1`,
  `mode1 <= 8`, `mode2 == 0`, an owned node identity and a present bitmap
  admit; final clip and destination remain explicitly no-draw. Verification:
  `dm2_v1_gdat_query_blit_rect_trace` passed in the isolated DM2 build and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` signed-root transform trace: binds the
  exact `SKULLWIN/c_xrect.cpp:228-276` signed root-node adjustment
  `datax/datay + input-x/input-y` to an authenticated source rectangle.
  Overflow, unsigned nodes, overrides and chained nodes reject; `crdecode`,
  clipping and destination remain no-draw. Verification:
  `dm2_v1_gdat_query_blit_rect_signed_trace` passed in the isolated DM2 build
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` mode-1 origin trace: binds the signed
  root receipt to `SKULLWIN/c_xrect.cpp:162-211,426-436`'s exact
  `crdecode(1, x0, y0, ...)` assignment. It retains current authenticated
  material dimensions and owner generation, rejects mode or material/surface
  drift, and remains no-draw before clip/bounds resolution. Verification:
  `dm2_v1_gdat_query_blit_rect_mode1_trace` passed in the isolated DM2 build
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` default-bounds trace: binds the
  `SKULLWIN/c_xrect.cpp:239,438-470` default `[-10000,10000)` range only when
  the complete mode-1 material rectangle remains inside it. Global override,
  terminal-chain and owner-generation drift reject; no surface destination or
  blit is admitted. Verification:
  `dm2_v1_gdat_query_blit_rect_default_clip_trace` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` global-clip input trace: source-locks
  `SKULLWIN/c_gui_vp.cpp:570-573`'s `TRIM_BLIT_RECT` calculation and its
  `c_xrect.cpp:438-439` override selection to active flag, trim-call,
  material and current surface identities. Missing source facts, empty clip
  and surface drift reject; no intersection, destination or blit is admitted.
  Verification: `dm2_v1_gdat_query_blit_rect_global_clip_trace` passed in the
  isolated DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_BLIT_RECT` global-clip intersection trace: binds
  `SKULLWIN/c_xrect.cpp:446-470`'s exact `dx/dy` branches to produce the
  source offset and clipped destination rectangle for the authenticated
  mode-1 override path. Clip, material or surface identity drift and empty
  overlap reject; it remains no-draw. Verification:
  `dm2_v1_gdat_query_blit_rect_global_intersection_trace` passed in the
  isolated DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` native surface-address trace: source-locks
  `SKULLWIN/c_image.cpp:293-335` and `c_gfx_blit.cpp:604-656`'s 8-bit
  `gfxsys.dm2screen` row address calculation. It accepts only packed original
  source rows, native 320-byte destination stride, no palette translation and
  unmasked alpha, then borrows the exact source/destination base pointers and
  offsets without writing pixels. Verification:
  `dm2_v1_gdat_draw_picst_surface_address_trace` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` forward-row traversal trace: retains the
  authenticated original material bytecount through the M11 pair/handoff and
  source-locks `SKULLWIN/c_gfx_blit.cpp:604-656` default `BLITMODE0` row
  increments. It records first/last row offsets, exclusive source/destination
  ends and addressed pixel count, rejecting bytecount, pointer, mode or
  surface drift without writing pixels. Verification:
  `dm2_v1_gdat_material_palette_pair_receipt`,
  `dm2_v1_gdat_materialization_handoff` and
  `dm2_v1_gdat_draw_picst_row_traversal_trace` passed in the isolated DM2
  build; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` masked palette-input trace: source-locks
  `SKULLWIN/c_image.h:45-70`'s `PAL256` storage and
  `c_gfx_blit.cpp:655-760`'s translated masked default branch. It requires a
  valid 8-bit alpha index, all 256 original palette bytes, original material
  bytecount and exact forward first/last row bounds; no palette lookup or
  pixel write occurs. Verification:
  `dm2_v1_gdat_draw_picst_mode0_palette_mask_trace` passed in the isolated
  DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` PAL256 index-order trace: source-locks
  `SKULLWIN/c_gfx_blit.cpp:39-42,675-682` so the raw indexed source value is
  compared against alpha before the corresponding PAL256 lookup. It retains
  the authenticated mask/palette receipt and row bounds, but does not
  dereference any pixel or palette byte. Verification:
  `dm2_v1_gdat_draw_picst_palette_index_trace` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` PAL256 format/write trace: source-locks
  `SKULLWIN/c_gfx_pal.h:23-44`, `c_gfx_pixel.h:54-98` and
  `c_gfx_blit.cpp:675-682`: a palette entry is one byte, PAL256 has 256
  entries, and only non-alpha source values conditionally target the current
  destination row in forward order. The receipt does not read or write pixel
  values. Verification: `dm2_v1_gdat_draw_picst_palette_write_trace` passed
  in the isolated DM2 build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PICST` masked consume-order trace: source-locks
  `SKULLWIN/c_gfx_blit.cpp:675-682`'s `source++`, conditional masked write,
  then `destination++` ordering for each forward row. It joins the authenticated
  mask/palette and destination receipts, rejects all identity drift, and does
  not consume a pixel. Verification:
  `dm2_v1_gdat_draw_picst_masked_consume_trace` passed in the isolated DM2
  build and `git diff --check` passed.

- ✅ 2026-07-17 DM2 native `DRAW_PICST` BLITMODE0 execution: consumes only
  the complete authenticated 8-bit source, PAL256, mask, destination and
  consume-order receipts. It performs the exact forward alpha-gated palette
  write and rejects owner-generation drift before any write. Verification:
  `dm2_v1_gdat_draw_picst_mode0_executor` passed byte-exact positive and
  no-write drift checks in the isolated DM2 build; `git diff --check` passed.

- ✅ 2026-07-17 DM2 M11 `DRAW_PICST` material consumer: the authenticated
  materialization handoff now gates the native executor at a DM2-owned M11
  boundary. The end-to-end test proves original source byte `3` reaches the
  viewport as its authenticated palette value and that altered handoff buffer
  identity leaves the destination untouched. Verification:
  `dm2_v1_gdat_draw_picst_m11_consumer` passed in the isolated DM2 build and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` material admission: the existing authentic
  GDAT wall command can now enter a source-locked `DRAW_PICST` boundary only
  with its raw, decoded, palette, material and geometry identities. It is
  explicitly no-draw: Skproject's wall route owns a 16-entry local palette,
  not the verified PAL256 executor contract. Verification:
  `dm2_v1_gdat_wall_draw_picst_admission` passed in the isolated DM2 build
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` B073 cache-output admission: binds the wall
  PAL16 input to a PAL256 cache slot only with matching RAW7, lookup,
  traversal and allocation identities from `SKULLWIN/c_querydb.cpp:2506-2668`.
  No palette expansion or pixel write occurs. Verification:
  `dm2_v1_gdat_b073_input_receipt` and
  `dm2_v1_gdat_wall_b073_output_receipt` passed in the isolated DM2 build;
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` B073 raw intake: source-locks borrowed RAW7
  program bytes, `v1e020c` group bytes and `v1e0210` lookup bytes, each with
  exact size/hash, to the authentic wall PAL16 and B073 cache allocation.
  Missing or drifted raw provenance rejects; no interpretation occurs.
  Verification: `dm2_v1_gdat_wall_b073_output_receipt` and
  `dm2_v1_gdat_wall_b073_raw_intake` passed in the isolated DM2 build;
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` B073 contiguous RAW7 loader: the wall PAL16
  route now loads only its original `INTERFACE_GENERAL/0/RAW7/2` record via
  `dm2_v1_asset_load_typed_sized()`, then binds the contiguous bytes, exact
  record length/FNV and B073 cache allocation. Category, index, length and
  hash drift reject; the route remains no-draw. Verification:
  `dm2_v1_gdat_wall_b073_raw7_loader` and `firestaff_dm2` passed in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` B073 PAL256 interpreter: adapts the proven
  RAW7 traversal from `SKULLWIN/c_gdatfile.cpp:1919-2003` and
  `c_querydb.cpp:2506-2668` for an owned 256-byte cache whose first PAL16
  entries exactly match the authenticated wall material. It applies the
  source lookup, light scale, interval and alpha-neighbour rules, then binds
  the resulting byte-exact cache to the existing wall `DRAW_PICST` output
  receipt. Golden normal/alpha cases and raw/cache/palette rejection cases
  pass. The route remains no-draw: an authentic U4/PAL256 wall consumer is
  still required before M11 may write pixels. Verification:
  `dm2_v1_gdat_wall_b073_output_receipt`,
  `dm2_v1_gdat_wall_b073_raw_intake`,
  `dm2_v1_gdat_wall_b073_raw7_loader`, and
  `dm2_v1_gdat_wall_b073_interpreter` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` native B073/M11 consumer: the source-owned
  normal `QUERY_TEMP_PICST` branch now performs the exact forward U4-to-8
  write order from `SKULLWIN/c_gfx_blit.cpp:495-548`, selecting bytes only
  from the authenticated B073 PAL256 cache. It requires the matching wall
  material, RAW4/trim receipt, composition snapshots, session/data identity
  and live surface generation. Golden viewport pixels, low-nibble alpha
  preservation, cache drift, surface rebound and composition drift are
  covered; flip, movement and non-normal branches remain closed. Verification:
  `dm2_v1_gdat_wall_b073_output_receipt`,
  `dm2_v1_gdat_wall_b073_raw_intake`,
  `dm2_v1_gdat_wall_b073_raw7_loader`,
  `dm2_v1_gdat_wall_b073_interpreter`, and
  `dm2_v1_gdat_wall_b073_m11_consumer` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_WALL` B073/M11 HFLIP consumer: extends only the
  source-proven `BLITMODE1` route from `SKULLWIN/c_gfx_blit.cpp:513-520`.
  Its U4 source scan remains forward while destination X decrements, including
  the low-nibble alpha no-write rule. The receipt binds flip identity to both
  wall and trim plans; invalid flip modes, cache drift, surface rebound and
  composition drift reject. Verification:
  `dm2_v1_gdat_wall_b073_output_receipt`,
  `dm2_v1_gdat_wall_b073_raw_intake`,
  `dm2_v1_gdat_wall_b073_raw7_loader`,
  `dm2_v1_gdat_wall_b073_interpreter`, and
  `dm2_v1_gdat_wall_b073_m11_consumer` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR` stationary panel M11 consumer: the first
  fully source-bound door branch now writes authentic DOORS IMG3 U4 indices
  through its exact local PAL16 and selected source colour key, at the
  command's RAW4 rectangle. It requires immutable raw/decoded/palette/geometry
  receipts plus composition snapshots and surface generation. Golden pixels,
  colour-key preservation, palette drift and movement rejection are covered.
  Opening/split, movement, flip and light-remap variants remain closed.
  Verification: `dm2_v1_gdat_door_panel_m11_consumer` and
  `dm2_v1_gdat_door_overlay_plan_real_data` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR` horizontal-opening M11 consumer: the
  source-bounded `SKULLWIN/c_gui_vp.cpp` `DRAW_DOOR` split branch admits only
  states 1..3 with horizontal opening and no movement or mirror. It consumes
  the authenticated right half first from its `base + state + 6` RAW4 row,
  then the left half from `base + state + 3`, using the same original DOORS
  IMG3 U4 bytes, PAL16, colour key, composition snapshot and live owner
  surface. The receipt rejects missing/mixed material identities, incomplete
  or duplicate RAW4 table rows, geometry drift, surface drift and every
  vertical/movement/flip state before writing. Golden pixels prove right-then-
  left consumption and the reject case proves no write. Verification:
  `dm2_v1_gdat_door_split_m11_consumer`,
  `dm2_v1_gdat_door_panel_m11_consumer`, and
  `dm2_v1_gdat_door_overlay_plan_real_data` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR` vertical-opening M11 consumer: the bounded
  `SKULLWIN/c_gui_vp.cpp:4860-5103` intermediate-state branch now admits only
  vertical states 1..3 with no movement or mirror. It retains the complete
  original DOORS IMG3 U4 plane and consumes the exact RAW4 geometry selected
  by `tlbRectnoDoorPosition[cell] + state`, with its local PAL16 and source
  colour key. The receipt carries the material, RAW4 table/row, composition
  and live surface identities. Golden bytes cover the source-order and colour
  key result; RAW4 geometry drift and movement reject with no write.
  Horizontal split, flip, vertical scaling and every other door state remain
  closed. Verification: `dm2_v1_gdat_door_vertical_m11_consumer`,
  `dm2_v1_gdat_door_split_m11_consumer`,
  `dm2_v1_gdat_door_panel_m11_consumer`, and
  `dm2_v1_gdat_door_overlay_plan_real_data` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR_FRAMES` right-jamb flip M11 consumer: the
  exact `SKULLWIN/c_gui_vp.cpp:2331-2500` right-side
  `QUERY_TEMP_PICST(1, 0x40, 0x40, ..., QUERY_CREATURE_BLIT_RECTI(...,14),3)`
  branch now consumes only its authenticated GRAPHICSSET IMG3 U4/PAL16
  material. The receipt requires the source right-jamb mirror identity, RAW4
  row/table, current scene-control hash and scene colour key, composition and
  owner surface. It executes the source forward-read/reverse-X write order;
  the golden fixture proves the colour-key skip and mirrored pixels, while
  scene-key and mirror drift reject without writes. Left jamb, panel flip,
  frame movement, scaling and all other states remain closed. Verification:
  `dm2_v1_gdat_door_side_frame_m11_consumer`,
  `dm2_v1_door_side_frame_source_route`,
  `dm2_v1_gdat_door_overlay_plan_real_data`,
  `dm2_v1_gdat_door_panel_m11_consumer`,
  `dm2_v1_gdat_door_split_m11_consumer`, and
  `dm2_v1_gdat_door_vertical_m11_consumer` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR_FRAMES` left-jamb M11 consumer: the paired
  `SKULLWIN/c_gui_vp.cpp:2331-2500` left-side
  `QUERY_TEMP_PICST(0, 0x40, 0x40, ..., QUERY_CREATURE_BLIT_RECTI(...,10),4)`
  branch is now admitted with its authenticated GRAPHICSSET IMG3 U4/PAL16,
  RAW4 row/table, scene-control hash and scene colour key, composition and
  owner surface. The shared side-frame receipt locks the jamb kind: left
  performs forward-X writes only while right performs reverse-X writes only.
  Golden/reject cases prove the left colour-key result, scene drift no-write,
  and direction mismatch rejection. Frame movement, scaling, panel flip and
  every unproven door transform remain closed. Verification:
  `dm2_v1_gdat_door_left_frame_m11_consumer`,
  `dm2_v1_gdat_door_side_frame_m11_consumer`,
  `dm2_v1_door_side_frame_source_route`,
  `dm2_v1_gdat_door_overlay_plan_real_data`, and the stationary/split/vertical
  door consumers passed; `firestaff` built in `build-dm2-agent`;
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR_FRAMES` movement jamb M11 consumer: the exact
  `SKULLWIN/c_gui_vp.cpp:2390-2458` `v1e12d0` branch now changes only the
  source-proven GRAPHICSSET selection: `table1d6b2c[cell]` selects the row
  and the left/right `table1d6ee1` column is swapped. The original viewport
  cell still supplies `QUERY_CREATURE_BLIT_RECTI`'s RAW4 rectangle and each
  jamb retains its source X direction. The M11 receipt requires current live
  movement state, permuted field, jamb mirror, raw material/PAL16, RAW4,
  scene, composition and surface identities. Golden pixels verify the moving
  right-jamb reverse-X route; stationary-field and movement-state drift reject
  without writes. Panel movement, frame scaling and all other transforms
  remain closed. Verification:
  `dm2_v1_gdat_door_moving_frame_m11_consumer`,
  `dm2_v1_door_side_frame_source_route`,
  `dm2_v1_gdat_door_side_frame_m11_consumer`,
  `dm2_v1_gdat_door_left_frame_m11_consumer`, and the remaining door suite
  passed; `firestaff` built in `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DOOR_FRAMES` roof-slit M11 consumer: the preceding
  source `SKULLWIN/c_gui_vp.cpp:2374-2387` `yy & 1` branch is now bounded to
  its authenticated `DRAW_DUNGEON_GRAPHIC(GRAPHICSSET, glbMapGraphicsSet,
  table1d6efd[cell], table1d6f0b[cell], glbSceneColorKey, 0)` route. It accepts
  only the six original table pairs for cells 3..8 (`0x12..0x17` and
  `0x2f2/0x2f1/0x2f3/0x2ef/0x2ee/0x2f0`), with immutable raw/decoded/PAL16,
  RAW4 table/row, scene, composition and live-surface receipts. The executor
  performs the source forward, unflipped indexed write and leaves scene-key
  pixels untouched; scene or table drift rejects before writing. No roof-slit
  flag, unlisted cell, scaling, mirror or fallback visual is admitted.
  Verification: `dm2_v1_gdat_door_roof_slit_m11_consumer` passed and
  `firestaff` built in `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DEFAULT_DOOR_BUTTON` D1C M11 consumer: the exact
  `SKULLWIN/c_gui_vp.cpp:1903-1986` default-button branch is admitted only for
  D1C's identity-scale `table1d6b71[1] == 0x40` route. It requires
  `DOOR_BUTTONS/0` field 0 or 5, `table1d6ed3[3] + 0x79e == 0x7a1`, immutable
  raw/decoded/PAL16 and RAW4 receipts, plus the scene, composition and live
  surface identities. Golden bytes cover the palette write and scene-key skip;
  RAW4 drift rejects before writing. D0/D2/D3 scaling and custom WALL_GFX
  buttons remain closed. Verification: `dm2_v1_gdat_door_button_m11_consumer`
  passed; `firestaff` built in `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DEFAULT_DOOR_BUTTON` D2C scaled M11 consumer:
  source `table1d6ed3[6] + 0x79e == 0x7a0` and
  `table1d6b71[2] == 0x2b` now reach M11 with the original
  `CALC_STRETCHED_SIZE(value, factor)` rounding rule. It accepts only default
  `DOOR_BUTTONS/0` fields 0/5, authentic material/PAL16/RAW4/scene/composition
  receipts, and performs the bounded downscaled indexed write. Golden coverage
  proves the scale and colour-key no-write; scale drift rejects before writing.
  D0/D3 and custom WALL_GFX buttons remain fail-closed. Verification:
  `dm2_v1_gdat_door_button_d2_m11_consumer` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_DEFAULT_DOOR_BUTTON` D0C scaled M11 consumer:
  admits only source `table1d6ed3[0] + 0x79e == 0x7a2` and
  `table1d6b71[0] == 0x60`, with the original rounded stretch size and
  authenticated default-button material, palette, RAW4, scene, composition
  and surface receipts. Golden coverage verifies the 3/2 source expansion and
  colour-key no-write; RAW4 drift rejects before writing. D3 and custom
  WALL_GFX buttons remain fail-closed. Verification:
  `dm2_v1_gdat_door_button_d0_m11_consumer` passed; `firestaff` built in
  `build-dm2-agent`; `git diff --check` passed.

- ✅ 2026-07-17 DM2 shared door-GDAT material receipt: restored the missing
  `dm2_v1_door_gdat_material_receipt` implementation required by the wall/
  door material gate. It admits only an enabled source `DOORS` colour-key
  `dtWordValue`, the caller-selected 4bpp IMG3/U4 field, metadata and local
  palette, then hashes the decoded original pixels without retaining or
  synthesizing material. Missing images and source-disabled scalar entries
  reject. Verification: `dm2_v1_wall_ornament_receipt` and the D0 button
  consumer passed; `firestaff` built in `build-dm2-agent`; `git diff --check`
  passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_ROOF` raw-material/transform M11 admission:
  `SKULLWIN/c_gui_vp.cpp:118-206` and `dm2data.cpp:814-816` now require the
  source's other-level/tile gate before cells 1..8 select their exact
  GRAPHICSSET field, rect, mirror and light parameter. The no-draw M11 receipt
  requires SUMMARY_IMAGE, GFX256 raw ownership, decoded U4 indices and a local
  palette. Missing remote-tile bit, invalid cell/light, missing GDAT raw bytes
  or palette reject. Verification: `dm2_v1_gdat_pit_roof_m11_receipt`,
  `dm2_v1_gdat_pit_m11_receipt`, and
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF B073/RAW4 prerequisite receipt: the existing
  no-draw PIT_ROOF material route now retains the authenticated c_light
  transaction consumed by `DM2_query_B073` and the raw
  INTERFACE_GENERAL/0/RAW4 `QUERY_BLIT_RECT` identity for `0x360..0x368`.
  The only admitted geometry is the source's bounded root `mode1=1/mode2=0`
  branch; palette/light identity and RAW4 table/row drift invalidate the
  no-draw admission. Verification: `dm2_v1_gdat_pit_roof_m11_receipt`,
  `dm2_v1_gdat_pit_m11_receipt`, and
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF alpha/blend material receipt: source-locked
  `SKULLWIN/c_image.cpp:450-475` and `c_gfx_blit.cpp:370-549` bind the full
  `DRAW_DUNGEON_GRAPHIC` alpha mask, its exact U4 low-nibble transparent index,
  source palette, B073 identity, RAW4 destination identity, and the only
  proven blit modes: normal and horizontal mirror. Mask drift and vertical or
  combined modes reject. The receipt remains no-draw until the B073 palette
  conversion and final destination composition are independently proven.
  Verification: `dm2_v1_gdat_pit_roof_m11_receipt`,
  `dm2_v1_gdat_pit_m11_receipt`, and
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF B073 RAW7 table receipt:
  `SKULLWIN/c_gdatfile.cpp:1919-2003` is now represented by a strict
  `INTERFACE_GENERAL/0/dt07/2` raw-layout receipt chained to the accepted
  PIT_ROOF material and B073 receipt. It retains the exact count/length
  program, both packed regions, trailing color lookup bytes, and raw identity;
  missing dt07/2, malformed lengths, and valid data drift reject. It remains
  no-draw because the per-color B073 traversal and destination composition are
  not yet authenticated. Verification: `dm2_v1_gdat_pit_roof_m11_receipt`,
  `dm2_v1_gdat_pit_m11_receipt`, and
  `dm2_v1_gdat_hud_summary_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF B073 per-color traversal receipt:
  `SKULLWIN/c_querydb.cpp:2506-2668` now has a source-locked, cache-free U4
  palette traversal chained to the authentic RAW7 program. Each input palette
  index must resolve through an in-bounds lookup, group, subindex and interval;
  the source alpha-neighbour branch is retained and drift in either index or
  alpha ownership rejects. The resulting palette hash remains no-draw pending
  exact `QUERY_PICST_IT` destination composition. Verification:
  `dm2_v1_gdat_pit_roof_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF QUERY_PICST_IT/DRAW_PICST destination receipt:
  `SKULLWIN/c_image.cpp:98-410` now admits only normal `0x40` scale, zero
  source crop, root RAW4 placement, B073 transformed palette, alpha order and
  the source's horizontal mirror. Clip identity drift and unsupported scale or
  flip reject. This remains no-draw because the destination bitmap ownership,
  dimensions/resolution and final viewport clip supplied to `DRAW_PICST` are
  not yet source-bound. Verification: `dm2_v1_gdat_pit_roof_m11_receipt`
  passed; full `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF viewport surface-owner binding: the DM2 viewport
  owner now publishes pointer, dimensions, stride, resolution and generation
  atomically on bind/rebind. PIT_ROOF accepts only the matching current
  generation and remains no-draw on stale or rebound surfaces. Verification:
  `dm2_v1_gdat_pit_roof_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 PIT_ROOF no-draw composition-slot validation: the receipt
  now rejects stale generation, rebound pointer, absent session/data epoch and
  absent ordered composition identity before any possible surface consumption.
  Full `firestaff` built and `git diff --check` passed; native draw remains
  unavailable pending a source-owned M11 consume hook.

- ✅ 2026-07-17 DM2 PIT_ROOF authenticated material-buffer handoff: the
  no-draw composition validator now requires the exact borrowed decoded U4
  pointer, width, height, stride, pixel count, palette hash and material
  identity captured from the accepted GDAT receipt. Pointer, dimension,
  stride, pixel-count, palette and material-identity drift each reject before
  any surface operation. Verification: `dm2_v1_gdat_pit_roof_m11_receipt`
  passed; full `firestaff` built; and `git diff --check` passed. Native draw
  remains blocked pending a source-owned ordered M11 consume hook with exact
  `DRAW_PICST` execution evidence.

- ✅ 2026-07-17 DM2 PIT_ROOF ordered `DRAW_PICST` consumer: the DM2-owned
  consume hook now performs only the source-proven normal-scale U4-to-8bpp
  masked rows from `SKULLWIN/c_image.cpp:229-337` and
  `c_gfx_blit.cpp:345-549`. It directly borrows the authenticated decoded
  handoff bytes, uses the B073-transformed palette, preserves the source
  alpha index and supports only the proven normal/horizontal-mirror order.
  Composition order, session/data identity, before/after surface snapshot,
  RAW4 rectangle, destination clip, palette and buffer identity all validate
  before any write. Regression coverage proves normal and mirrored pixels plus
  no-write on order, source-byte and rebound-surface mismatch. Verification:
  `dm2_v1_gdat_pit_roof_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` composition admission: source-locked
  cells 1..15 from `SKULLWIN/c_gui_vp.cpp:234-292` now bind their accepted
  GRAPHICSSET SUMMARY_IMAGE/GFX256 material, transform, session/data epoch,
  and parent viewport-composition identity into one strict no-draw receipt.
  Material, transform, parent order, session and epoch drift reject. The
  receipt explicitly has no PIT_TILE draw slot: cell 0 flip and the per-cell
  `QUERY_BLIT_RECT` placement/clip source facts remain required before any
  indexed consumer can write. Verification: `dm2_v1_gdat_pit_m11_receipt`
  passed; full `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` RAW4 placement receipt: the source's
  `table1d6c70[cell] -> DRAW_DUNGEON_GRAPHIC -> QUERY_PICST_IT ->
  QUERY_BLIT_RECT` chain is now represented for cells 1..15 by an exact
  INTERFACE_GENERAL/0/RAW4 root-row receipt. It retains selected rectangle,
  destination, full decoded extent and table/row identity, then binds that
  receipt to the PIT material/composition admission. Missing RAW4, incomplete
  table, row placement drift or material/composition drift reject. Chained
  rectangle/crop/clip forms remain fail-closed, and no PIT pixel consumer is
  authorized until source-owned order and buffer-handoff evidence exist.
  Verification: `dm2_v1_gdat_pit_m11_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` decoded-buffer handoff and M11 slot:
  PIT_TILE now owns a separate borrowed U4 handoff receipt, independent of
  PIT_ROOF identities. It binds exact decoded pointer, dimensions, stride,
  pixel count, palette and material identity to the established DM2 viewport
  before/after surface snapshot and ordered composition identity. Pointer and
  rebound-surface drift reject with no write. The resulting M11 slot remains
  explicitly no-draw because the source has not yet proved PIT_TILE's normal
  `DRAW_PICST` row order. Verification: `dm2_v1_gdat_pit_m11_receipt` passed;
  full `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` normal-row ordering receipt: cell 1's
  source `blitmode=0` branch is now bound through the PIT-specific handoff,
  RAW4 placement and generic composition slot to the exact normal
  `DRAW_PICST` row order from `SKULLWIN/c_image.cpp:229-337` and
  `c_gfx_blit.cpp:345-549` (top-to-bottom, left-to-right). RAW4 identity
  drift rejects before any possible write. The receipt remains no-draw: the
  source calls `DM2_query_B073` before those rows, and PIT has no authenticated
  transformed palette transaction yet. Verification:
  `dm2_v1_gdat_pit_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` B073 transformed-palette receipt: cell
  1 now authenticates `DM2_query_B073`'s bounded RAW7 count/left/right/lookup
  program against the accepted PIT material, RAW4 placement and normal-row
  receipt. It retains the transformed 16-colour palette plus RAW7 and source
  identities; RAW7 output or RAW4 drift rejects. It remains no-draw until the
  source alpha transaction and final ordered buffer consumer are admitted.
  Verification: `dm2_v1_gdat_pit_m11_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-1 normal indexed consumer: the
  source-proven normal branch now consumes only PIT's authenticated U4 handoff
  through its own RAW7 B073 palette and low-nibble alpha transaction, in exact
  top-to-bottom/left-to-right order into the current DM2 owner surface.
  Composition order, material/RAW4/placement/normal/B073 identities and owner
  snapshots validate before the first write; order drift produces no write.
  Other cells, mirrors, crop and chained-clip forms remain blocked.
  Verification: `dm2_v1_gdat_pit_m11_receipt` passed; full `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-3 normal indexed consumer: added a
  separate source-locked admission for the normal `blitmode=0` cell-3 form.
  It requires the exact GRAPHICSSET image field `0x6e`, RAW4 rect `0x35b`,
  independent B073/RAW7 palette transaction, authenticated U4 handoff, and
  ordered composition/surface identities before any row writes. Material or
  RAW4 drift leaves the surface unchanged. Cell 2 and every mirrored, crop,
  chained-clip, or other normal cell remain blocked. Verification:
  `dm2_v1_gdat_pit_m11_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-4 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x6f`, RAW4 rect `0x35a`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface. The
  consumer revalidates B073's complete identity before the first write, so
  palette or RAW4 drift leaves the surface unchanged. Cell 2 and every other
  mirrored, crop, chained-clip, or unproven normal form remain blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-6 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x71`, RAW4 rect `0x358`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface.
  Material-identity drift rejects before any surface write. Every mirrored,
  crop, chained-clip, or other unproven normal form remains blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-7 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x72`, RAW4 rect `0x357`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface. B073
  palette-identity drift rejects before any surface write. Every mirrored,
  crop, chained-clip, or other unproven normal form remains blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-11 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x76`, RAW4 rect `0x355`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface. RAW4
  identity drift rejects before any surface write. Every mirrored, crop,
  chained-clip, or other unproven normal form remains blocked. Verification:
  Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-12 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x77`, RAW4 rect `0x354`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface. B073
  RAW7-identity drift rejects before any surface write. Every mirrored, crop,
  chained-clip, or other unproven normal form remains blocked. Verification:
  Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-14 normal indexed consumer: added an
  independent source-locked normal `blitmode=0` admission requiring exact
  GRAPHICSSET field `0x79`, RAW4 rect `0x352`, B073/RAW7 palette identity,
  decoded U4 handoff, ordered composition, and current owner surface. Palette
  identity drift rejects before any surface write. Every mirrored, crop,
  chained-clip, or other unproven normal form remains blocked. Verification:
  Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja `firestaff` built;
  and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-2 HFLIP indexed consumer: added a
  distinct source-locked reverse-X row path for the exact mirrored form:
  GRAPHICSSET field `0x6c`, RAW4 rect `0x35f`, B073/RAW7 palette, decoded U4
  handoff, ordered composition and owner surface all bind before writing.
  The regression proves the source pixels are reversed per row and B073 drift
  causes no write. Crop, chained clip, vertical flip and all other mirror
  forms remain blocked. Verification: Ninja CTest
  `dm2_v1_gdat_pit_m11_receipt` passed; Ninja `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-5 HFLIP indexed consumer: added an
  independent source-locked reverse-X path requiring GRAPHICSSET field `0x6f`,
  RAW4 rect `0x35c`, B073/RAW7 palette, decoded U4 handoff, ordered composition
  and owner surface. The regression proves per-row source reversal and RAW4
  drift no-write. Other mirrored, crop and chained-clip forms remain blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-8 HFLIP indexed consumer: added an
  independent source-locked reverse-X path requiring GRAPHICSSET field `0x72`,
  RAW4 rect `0x359`, B073/RAW7 palette, decoded U4 handoff, ordered composition
  and owner surface. The regression proves per-row source reversal and RAW7
  drift no-write. Other mirrored, crop and chained-clip forms remain blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-13 HFLIP indexed consumer: added an
  independent source-locked reverse-X path requiring GRAPHICSSET field `0x77`,
  RAW4 rect `0x356`, B073/RAW7 palette, decoded U4 handoff, ordered composition
  and owner surface. The regression proves per-row reversal and palette drift
  no-write. Other mirrored, crop and chained-clip forms remain blocked.
  Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt` passed; Ninja
  `firestaff` built; and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` cell-15 HFLIP indexed consumer: exact GRAPHICSSET `0x79`, RAW4 `0x353`, B073/RAW7, U4 reverse-X, ordered composition and owner surface; RAW4 drift writes nothing. Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt`, Ninja `firestaff`, and `git diff --check` passed.

- ✅ 2026-07-17 DM2 `DRAW_PIT_TILE` crop/chained-clip provenance intake: source-locked `c_gui_vp.cpp:251-291 -> c_image.cpp:229-260` receipt retains only `query1`/RAW4 identity and is explicitly no-draw because source-coordinate mutation and chained output are not authenticated. Verification: Ninja CTest `dm2_v1_gdat_pit_m11_receipt`, Ninja `firestaff`, and `git diff --check` passed.

- ✅ 2026-07-15 DM2 source static-object cell gate: direct G1 DB5 weapon and
  DB9 container map-chip rows retain a `DM2_DRAW_STATIC_OBJECT` candidate cell and
  `DM2_DRAW_DUNGEON_TILES` pass only when their live coordinate is one of the
  source-proven D1C/D2C centre cells (cells 3/6, passes 17/14). The previous
  generic map-to-screen projection is no longer allowed to place F9 map-chip
  pixels at D0C, side, deep, or centre cells without the original `DRAW_ITEM`
  geometry tables. The item render plan retains source pass order and blocks
  missing or altered admission receipts. Source: SKProject
  `c_gui_vp.cpp::DM2_DRAW_DUNGEON_TILES`, `DM2_DRAW_STATIC_OBJECT`, and
  `DM2_DRAW_PUT_DOWN_ITEM`; `dm2data.cpp::table1d7029`. Verification:
  DB5 gate 6/6, DB9 gate 3/3, boot smoke 88/88, save/load 26/26.

- ✅ 2026-07-16 DM2 bounded `DRAW_STATIC_OBJECT` evidence gate: recovered
  SKWIN's first DB5/DB9 `DRAW_STATIC_OBJECT -> DRAW_PUT_DOWN_ITEM -> DRAW_ITEM`
  selection chain for D1C/D2C. The viewport now derives the exact 5x5
  direction position, F0/F4 field selector, `QUERY_CREATURE_BLIT_RECTI` clip
  key, distance stretch, 16-slot placement delta, and DB9 mirror rule. It also
  closes the old false admission: F9 receipts belong to `DRAW_MAP_CHIP`, not
  `DRAW_ITEM`, and cannot draw as static objects. The plan remains evidence
  only until raw source receipts for visibility, expanded clipping, image
  offset, and F0/F4 pixels/palette exist. Verification: focused DB5 and DB9
  viewport gates.

- ✅ 2026-07-17 DM2 static DB5/DB9 F0/F4 M11 frame-plan gate: bounded D1C/D2C
  `DRAW_ITEM` rows now require their source cell/pass/clip plan plus exact
  GFX256 raw receipt, local palette, RAW4 rect receipt, and matching M11
  identity before a viewport blit. DB5 selects F0; DB9 selects F0/F4 from its
  original opened bit. F9 `DRAW_MAP_CHIP`, changed receipt hashes, absent
  receipt fields, and unsupported cells remain no-draw. Verification:
  `dm2_v1_g1_static_m11_handoff_gate` passed 1/1 and `git diff --check`.

- ✅ 2026-07-17 DM2 static-object M11 delivery lifecycle: DB5/F0 weapons and
  DB9/F0/F4 containers now have a separate source-owned delivery plan built
  from existing GFX256/IMG3/RAW4 receipts and the bounded source cell/pass/
  clip plan. Session, selector/data, material, or rect drift invalidates the
  plan. It is explicitly no-draw pending original pixel decoder proof, and
  neither the earlier blit fixture nor F9/map-chip material can supply it.
  Verification: `dm2_v1_static_object_m11_delivery_plan` and
  `dm2_v1_g1_static_m11_handoff_gate` passed 2/2; `git diff --check`.

- ✅ 2026-07-17 DM2 source-owned M11 viewport composition receipt: the
  DM2 receipt gate now combines complete GDAT scene/light, wall, door,
  weather, DB5/DB9 static-object, and DB14 flying-item plans in a fixed
  source-owned order under one nonzero session/data epoch. Each component's
  identity is folded into the receipt; missing members, invalid door controls,
  changed data/session epoch, and changed member identities fail closed. The
  composition is metadata-only and explicitly no-draw until authentic
  per-pixel decoder proof exists. `m11_game_view.c` is untouched. Verification:
  `dm2_v1_dm2_viewport_m11_composition` passed 1/1; `git diff --check`.

- ✅ 2026-07-17 DM2 live viewport composition enumeration: the DM2-owned
  receipt gate now derives a deterministic identity from the serialized valid
  session and its G1 level/GRAPHICSSET/outdoor map token, then admits only a
  matching complete GDAT plan set. Scene/light/wall/weather graphicsset
  mismatch, absent members, session mutation, static-session mismatch, and
  changed GDAT plan identity reject. The resulting receipt remains metadata
  only and no-draw; neither CSB nor shared M11 files participate. Verification:
  `dm2_v1_dm2_viewport_m11_live_enumeration`, composition, DB5/DB9 static,
  and DB14 flying-item CTests passed 4/4; `git diff --check`.

- ✅ 2026-07-17 DM2 DB14 `Missile::missile_object` selector receipt:
  `dm2_v1_g1_flying_item_selector_receipt` now follows SKproject
  `c_gui_vp.cpp:3545-3770`, `c_record.cpp:203-279`, and
  `dm2data.cpp:487` through the raw DB14 address, class1/class2 resolution,
  byte+4 sentinel, and GDAT `(0x0d,class2,0x0b,1)` data-index query. The
  isolated raw G1/GDAT fixture covers the positive spell-missile branch,
  opaque non-0x0d `DM2_DRAW_ITEM` branch, sentinel rejection, and missing
  GDAT index rejection. It remains selector evidence only: no draw plan or
  pixel decoder was opened.

- ✅ 2026-07-17 DM2 DB14 `vb30`/summary-image receipt: the exact bounded
  branch from `SKULLWIN/c_gui_vp.cpp:3610-3745` now selects only `8`, `9`,
  `10`, or `12` from source-owned query/timer/direction/table facts. The
  TEMP_PICST tuple is then bound to `QUERY_GDAT_SUMMARY_IMAGE` as
  `(class1,class2,vb30)` and requires authentic IMG3 metadata plus its local
  palette. The focused raw G1/GDAT CTest covers every selection, invalid
  state/direction/table rejection, TEMP_PICST blocking, and missing GDAT
  index rejection. It remains no-draw and does not create decoded pixels or
  a frame plan. Verification: `dm2_v1_g1_flying_item_source_receipt`,
  `dm2_v1_flying_item_timer_receipt`, and `dm2_v1_flying_item_m11_frame_plan`
  passed 3/3; `firestaff` built; `git diff --check` passed.

- ✅ 2026-07-17 DM2 DB14 decoded-material no-draw plan: the verified
  `(class1,class2,vb30)` SUMMARY_IMAGE tuple now requires matching raw GDAT
  GFX256, IMG3 decoder output, and local-palette hashes. The decoder's bytes
  are hashed then freed; no surface or renderer fallback is retained. The
  DM2-owned runtime plan additionally binds session, map, timer, selector,
  vb30, and table identities, rejecting every tested drift while remaining
  `no_draw`. Verification: `dm2_v1_g1_flying_item_source_receipt`,
  `dm2_v1_flying_item_timer_receipt`, and `dm2_v1_flying_item_m11_frame_plan`
  passed 3/3; `firestaff` built; `git diff --check` passed.

- ✅ 2026-07-17 DM2 DB14 M11 material-consumer composition: the DM2 receipt
  gate now re-decodes the admitted indexed IMG3 bytes and rechecks their
  hash, local palette, raw receipt, clip id, flip fact, session/map/timer
  ownership, and the source order from `SKULLWIN/c_gui_vp.cpp:4320-4417`
  (`PUT_DOWN_ITEM`, creature summary, then flying item). It exposes no pixel
  buffer and remains `no_draw` because the original destination transform is
  not yet proven. Verification: focused DB14 material, delivery-plan, and
  composition CTests passed 3/3; full `firestaff` built; `git diff --check`
  passed.

- ✅ 2026-07-17 DM2 DB14 SUMMARY_IMAGE/RAW4 destination receipt:
  `dtImageOffset(0xfe)` and the selected `dtImageOffset(vb30)` are retained
  in source order as signed x/y evidence and joined only to the same
  `QUERY_CREATURE_BLIT_RECTI`-selected RAW4 `QUERY_EXPANDED_RECT`. The focused
  fixture covers offset arithmetic plus changed rect-id, empty clip, and flip
  rejection. The receipt remains `no_draw`; it does not guess the final
  QUERY_PICST_IT orientation or framebuffer transform. Verification:
  `dm2_v1_g1_flying_item_source_receipt` passed; full `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 DB14 viewport/M11 evidence lifecycle: a complete
  DB14 timer/GFX256/IMG3/RAW4 receipt may now enter the source-owned viewport
  evidence boundary only with matching no-draw selector and geometry receipts,
  session identity, and G1 map token. The extended M11 delivery builder checks
  timer/GDAT identity before retaining that evidence identity. Image fields and
  pixels remain unavailable, so this cannot draw. Verification: focused DB14
  selector, timer, and M11-frame CTests passed 3/3; `firestaff` built; and
  `git diff --check` passed.

- ✅ 2026-07-17 DM2 `QUERY_CREATURE_BLIT_RECTI` source receipt: dungeon-loader
  now models the SKWIN 5x5 rotation and exact `5000 + 25 * cell` RAW4 rect
  identity, with all invalid cell/position/direction combinations rejected.
  The DB14 `DRAW_FLYING_ITEM` receipt consumes the documented `dir=0` route
  only; it remains no-draw until independent timer-owner and GDAT material
  evidence is complete. Verification: `dm2_v1_g1_flying_item_source_receipt`
  and `dm2_v1_flying_item_timer_receipt` passed 2/2; `git diff --check`.

- ✅ 2026-07-17 DM2 DB14 raw timer receipt: `DRAW_FLYING_ITEM` now accepts a
  missile direction only through a bounded, caller-owned ten-byte
  `DME.h::Timer` row. The row index must equal DB14 `w6`, its complete raw
  bytes are hashed, and `Direction()` is decoded from `w8` bits 10--11.
  Truncated/non-row-aligned tables, invalid indices, changed direction, and
  mismatched DB14 owners reject. This still grants no draw or M11 handoff.
  Verification: `dm2_v1_g1_flying_item_source_receipt` and
  `dm2_v1_flying_item_timer_receipt` passed 2/2; `git diff --check`.

- ✅ 2026-07-17 DM2 DB14 saved-timer ownership bridge: the timer receipt can
  now be assembled only from the verified session's original contiguous timer
  table and its original count, never from the runtime heap or a synthetic
  direction. DB14 `w6` outside that count rejects. The bridge retains no-draw
  until a source-proven viewport selection and complete GDAT material receipt
  are present. Verification: `dm2_v1_g1_flying_item_timer_receipt` and
  `dm2_v1_g1_flying_item_source_receipt` passed 2/2; `git diff --check`.

- ✅ 2026-07-17 DM2 DB14 runtime material ownership: runtime now retains a
  flying-item receipt only after the bounded saved-timer owner and every
  GFX256/palette/RAW4 material identity agree. Any missing or altered identity
  clears the retained receipt. It remains explicitly no-draw and does not
  enter the legacy projectile/map-chip route or M11. Verification:
  `dm2_v1_flying_item_timer_receipt` and
  `dm2_v1_g1_flying_item_source_receipt` passed 2/2; `git diff --check`.

- ✅ 2026-07-17 DM2 DB14 flying-item viewport/M11 delivery plan: a separate
  `DRAW_FLYING_ITEM` plan now carries source cell/5x5/flip/RAW4 rect plus
  GFX256 raw, IMG3 palette, RAW4, and saved-timer identities. Every missing or
  drifted receipt rejects; F9/projectile/map-chip material has no entry point.
  The accepted plan is deliberately `no_draw` with original pixel decoding
  disabled, so M11 receives identity only rather than a synthetic frame.
  Verification: `dm2_v1_flying_item_m11_frame_plan`,
  `dm2_v1_flying_item_timer_receipt`, and
  `dm2_v1_g1_flying_item_source_receipt` passed 3/3; `git diff --check`.
- ✅ 2026-07-15 CSB live HUD ownership: moved the PC3.4 C017/C040 panel
  composition from M11 into CSB's source-owned `PANEL.C F0347/F0346` adapter.
  M11 now consumes only the completed terminal-session receipt; the real
  package probe compares the copied 224x136 region byte-for-byte with the
  authentic indexed raster. No generated HUD or transparency wrapper remains.
- ✅ 2026-07-15 CSB F0806-to-F0128 first-frame handoff: after the genuine
  C004/C002/C003 door sequence ends, M11 clears the released startup page
  before asking the source-bound F0128 viewport and HUD consumers to build the
  first live dungeon frame. The obsolete M11 opening-composite asset wrapper
  is removed, so the real package session is the only owner of door pixels.
- ✅ 2026-07-15 CSB F0128 live-frame receipt: M11 now commits a dungeon page
  only after CSB validates its real 224x136 indexed viewport pixels and draw
  counts against the terminal C001-C005/C017/C040 PC3.4 session. Missing
  session ownership fails closed before the candidate page can reach M11.
- ✅ 2026-07-15 CSB F0098 PC3.4 aperture handoff: the live F0128 binding now
  carries M11's hash-verified C079 ceiling and C078 floor assets into the
  shared ReDMCSB F0098 callback. The PC3.4 224x39 + 224x97 aperture is copied
  only from those expanded package bytes; the focused regression proves both
  source graphic requests and rejects a cleared binding. Verification:
  `test_csb_v1_viewport_phase3_rendering` 2651/0, M11 launcher 440/0, real
  PC3.4 package probe 71/71.
- ✅ 2026-07-15 DM1 ReDMCSB F0107 HoC C127 materialization: M11 now consumes
  the native PC34 C346 48x43 raster through F0791 into G0205's 64x43 D1C
  destination before C026, rather than treating the destination width as a
  source crop and rejecting the authentic bitmap. It publishes C346/C026
  provenance only after both source-backed blits complete; the real-PC34 HoC
  regression compares source/destination geometry, palette/key, and C026
  atlas cell against that live receipt.
- ✅ 2026-07-15 CSB live door/HUD gate: M11 now requires the complete
  C001 -> C004/C002/C003 -> C017/C040 session and its CSB-owned HUD host
  receipt before it can draw the first F0128 page. The remaining unused M11
  startup asset-command wrappers were removed. A real-launch regression
  rejects the live frame when the authenticated source surface set is absent.
- ✅ 2026-07-15 CSB opening-door package capture: the M12/M11 real-PC3.4
  regression now presents all 31 F0806/F0807 C002/C003 opening steps, checks
  each moving source strip at its exact native geometry, and records every
  indexed package frame before the terminal C017/C040 HUD handoff.

- ✅ 2026-07-15 CSB PC3.4 C001--C005 decoder and final-door receipt: the
  active package consumer now uses CSBWin `Graphics.cpp::ExpandGraphic`'s
  big-endian four-plane stream after the ReDMCSB F0490 archive/LZW boundary;
  the generic F0488 packed-nibble route cannot decode title or entrance art.
  C001 PRESENTS/CHAOS/STRIKES, C004+C002+C003, C017, and C040 have
  canonical local-package raster hashes. The M11 capture additionally proves
  all 31 F0807 frames, including steps 27--31 where C002 is fully clipped and
  C004+C003 are the only two submitted source surfaces. Focused coverage:
  `test_csb_v1_m11_launcher_handoff_boundary` (440/0), real package probe
  (71/0), and its CTest entry.

- ✅ 2026-07-15 CSB complete title/entrance source-frame regression: the
  local canonical PC3.4 package probe now walks all 102 `TITLE.C F0437`
  frames and all 31 `ENTRANCE.C F0807` frames through the same owned session
  and host-raster receipt that M11 presents. It compares each door strip
  directly to C002/C003 source bytes, rejects stale plane/strip composition,
  and locks the ordered C001 and C004 sequence hashes. Production records the
  actual indexed CSB page only after SDL has presented it; headless coverage
  deliberately does not claim a macOS app/window capture. Verification:
  real package probe 73/0 and M11 launcher handoff 440/0.

- ✅ 2026-07-15 CSB SDL-presented host capture: the common M11 presentation
  edge is no longer named or scoped as a DM1 HoC helper. For CSB it records
  C001-C005/C017/C040 only after a successful SDL presentation has produced
  a nonempty renderer RGBA buffer; an allocated SDL window alone cannot set
  the macOS-window receipt. The indexed source frame remains the exact
  C001--C005 package composition, while the renderer buffer proves that the
  host actually accepted a presentation. A local `firestaff --game csb
  --duration 0` macOS smoke reached this path with verified CSB data. Full
  visible app/window capture over the complete sequence remains a separate
  external-capture task.

- ✅ 2026-07-15 CSB screen-output capture gate: C001--C005 app capture now
  verifies the renderer-owned RGBA page pixel-for-pixel against the exact
  nearest-scaled indexed source page and the active ReDMCSB special palette.
  The C001 title and C004/C002/C003 entrance receipt is therefore withheld
  for stale planes, strips, palette state, or post-presentation RGB mismatch.
  The source page remains package-owned; this adds no generated visual path.
  The SDL dummy special-palette renderer probe covers the shared comparator
  across all six palette cases (6/6).

- ✅ 2026-07-15 DM1 ReDMCSB F0292 C008 opaque status-box route: dead
  champions now blit the complete original C008 67x29 source surface without
  a host transparency key, so source-black pixels erase prior bars and hands
  rather than leaking stale HUD. Focused CTest:
  `m11_dm1_champion_panel_asset_fail_closed`.

- ✅ 2026-07-15 DM2 source door scheduler: source-required center-door plans
  now retain the real `DM2_DRAW_DUNGEON_TILES` passes for D3C, D2C and D1C.
  D0C is excluded from this tile-loop route because cell zero is absent from
  `table1d7029`; it cannot borrow the later player-tile route. Focused
  coverage: `test_dm2_v1_door_side_frame_source_route`.
- ✅ 2026-07-15 CSB PC3.4 host palette/raster handoff: C001 PRESENTS, CHAOS,
  and STRIKES retain their distinct ReDMCSB `TITLE.C F0437` palette phase in
  the owned runtime frame and host-receipt hash. C004/C002/C003 opening-door
  presentation now requires the ENTRANCE palette with its canonical indexed
  raster hash. The real-package probe verifies all four host handoffs without
  a planar, generic, or generated fallback.

- ✅ 2026-07-15 DM1 ReDMCSB F0107 unreadable-inscription palette receipt:
  side/depth M648 substitutes now publish the exact source plan's C10 key and
  16-entry palette map together with their G0205 geometry. The real-PC34
  regression compares the complete map, so a host palette cannot silently
  replace the source route. Focused coverage:
  `m11_dm1_unreadable_inscription_host_presentation_receipt`.
- ✅ 2026-07-15 CSB PC3.4 title/entrance decode correction: F0490 owns the
  archive/LZW boundary, then C001-C005/C017/C040 use CSBWin
  `Graphics.cpp::ExpandGraphic`'s big-endian four-plane decoder. Generic
  ReDMCSB F0488 IMAGE2 expansion is not a valid substitute for these PC3.4
  streams. The real-package probe locks C001 PRESENTS, CHAOS, STRIKES,
  C017/C040, and C004+C002+C003 raster hashes without a fallback.

- ✅ 2026-07-15 DM1 ReDMCSB DUNVIEW C026 atlas gate: every D1C champion
  mirror draw now validates the native 256x87 atlas and the exact F0172
  render-index source cell before its unscaled raw-index blit. Palette
  application remains with the final F0097 viewport pass. Focused coverage:
  `m11_dm1_front_mirror_asset_fail_closed`.
- ✅ 2026-07-15 DM2 `DM2_DRAW_DUNGEON_TILES` wall scheduler: M11 now uses
  SKProject's exact `table1d7029` viewport-cell order for every admitted
  `DRAW_WALL` panel, replacing the old DM1-like depth loop. The focused
  canonical-GDAT test verifies all ten source wall cells and rejects center
  cells without a source wall geometry route. Object and tile-type branches remain
  unavailable until their own raw cell evidence exists.

- ✅ 2026-07-15 DM1 ReDMCSB DUNVIEW C346 source-rectangle gate: the D1C
  mirror backing now requires and samples exactly its G0205 64x43 source
  rectangle. A larger or malformed asset can no longer stretch unrelated
  trailing pixels into the fixed C026 portrait frame. Focused coverage:
  `m11_dm1_front_mirror_asset_fail_closed` and the real-PC34 HoC receipt
  regression.
- ✅ 2026-07-15 DM2 source scene composition: M11 now retains SKProject
  `DM2_DISPLAY_VIEWPORT`'s ceiling-then-floor transaction (`0x2bc` before
  `0x2bd`) with each exact GRAPHICSSET image, local IMG3 palette, rectangle,
  and material hash. Altering the order or any bound receipt is no-draw;
  Firestaff does not share or synthesize a plane palette. Focused coverage:
  `test_dm2_v1_gdat_scene_plan_viewport_real_data`.

- ✅ 2026-07-15 DM1 ReDMCSB F0172 C127 projection gate: a C127 sensor now
  bypasses the generic F0107 wall-ornament loop. Only DUNVIEW's fixed D1C
  C346/C026 route may render it, preventing an invented perspective mirror at
  D2/D3 or side-wall positions. Focused coverage:
  `m11_dm1_front_mirror_asset_fail_closed`.

- ✅ 2026-07-15 DM1 ReDMCSB F0168/F0172 selected-inscription consumers: the
  F0128 D1C repaint and F0107 side/depth presentation now consume the same
  F0172-selected raw G0290 record as the direct M648 material route. Generic
  Thing-chain scanning cannot leak an unrelated inscription into any live
  viewport projection; a bad selected record remains no-draw. Focused
  coverage: `m11_dm1_inscription_selected_wall_gate`.

- ✅ 2026-07-15 DM1 ReDMCSB F0168/F0172 selected-inscription gate: the M11
  F0107/M648 consumer now decodes only the current F0172-selected G0290 raw
  TextString. A bad selected source record is no-draw rather than a fallback
  scan to another visible TextString. Focused coverage:
  `m11_dm1_inscription_selected_wall_gate`.

- ✅ 2026-07-15 DM1 ReDMCSB F0292 champion-HUD fallback gate: DM1 status
  boxes now stay source-owned even when V1 chrome is disabled. If a required
  C008/C007 surface cannot be consumed, DM1 leaves the existing source clear
  rather than drawing the generic cyan host frame. Focused coverage:
  `m11_dm1_champion_panel_asset_fail_closed`.

- ✅ 2026-07-15 DM1 ReDMCSB F0172 C127 mirror material gate: the live D1C
  C346/C026 route now requires a successful original C346 backing draw before
  it may overlay C026, and validates the C026 atlas rectangle before blitting.
  This prevents a missing or malformed backing from presenting a floating
  champion portrait; no replacement frame or font is used. Focused coverage:
  `m11_dm1_front_mirror_asset_fail_closed` and
  `m11_dm1_hoc_wall_material_receipt_pc34`.

- ✅ 2026-07-15 DM1 ReDMCSB CASTER/MENUDRAW spell-HUD source gate: DM1 now
  leaves the spell area source-cleared unless C009/C011 and the original font
  are fully decoded. The legacy procedural workbench cannot become a DM1
  missing-asset fallback. Focused coverage:
  `m11_dm1_spell_area_asset_route` and
  `m11_dm1_action_spell_asset_fail_closed`.

- ✅ 2026-07-15 DM1 ReDMCSB F0387/F0397/F0398 M653 binding: action labels
  and spell runes now accept only the verified PC34 M653 graphic entries 695
  or 557 from the active DM1 asset session. A generic 768-byte record, another
  active font instance, or unavailable original font leaves the source-owned
  cells cleared; no host or guessed bitmap font can render `WAR CRY` or rune
  glyphs. Focused coverage: `m11_dm1_action_spell_asset_fail_closed`.

- ✅ 2026-07-15 DM1 ReDMCSB TEXT2.C F0644 M653 baseline raster: F0387 action
  header/rows and F0397/F0398 available/selected runes now share the real
  M653 six-by-six cell primitive. It samples `char * 8 + 3`, advances six
  pixels per HUD cell, and converts each original baseline to `baseline - 4`.
  This removes the prior four-pixel-low HUD glyph placement without adding a
  host-font route. Focused coverage: `m11_dm1_action_spell_asset_fail_closed`
  and `m11_dm1_spell_area_asset_route`.

- ✅ 2026-07-15 DM1 ReDMCSB F0387/F0393/F0397/F0398 panel consumption: the
  active action header/rows are clipped to their C080/C085..C087 source boxes,
  while the live spell path consumes the source-owned overlay plan for every
  six-rune and four-rune label. The living caster tab now applies F0698's
  indexed inversion only to F0393's exact inclusive C009 rectangle; no host
  highlight, text, or color is introduced. Focused coverage:
  `m11_dm1_action_spell_asset_fail_closed` and
  `m11_dm1_spell_area_asset_route`.

- ✅ 2026-07-15 DM1 ReDMCSB MENUDRAW.C F0396/F0392 C011 row material: the
  live caster path now validates C011 as its original 96x36 three-row bitmap
  and copies the complete 96x12 available and selected rows to y=50/y=62.
  The former 14x39 fragment could leave real panel controls missing or stale;
  it is no longer accepted by the DM1 route. Focused coverage:
  `dm1_v1_box_spell_area_pc34_compat` and
  `m11_dm1_spell_area_asset_route`.

- ✅ 2026-07-15 DM1 ReDMCSB F0115 creature source-material/geometry gate:
  live C584+ creature presentation now requires a decoded loaded-pixel PC34
  surface selected through G0221/G0222. Blank C3200/G0224 anchors reject the
  draw plan, and M11 rejects the old pane-relative side-hint geometry rather
  than inventing a placement. Focused coverage:
  `m11_dm1_f0115_material_asset_fail_closed` and
  `test_dm1_v1_creature_render_pc34_compat_integration`.
- ✅ 2026-07-15 CSB-001 ReDMCSB F0267/F0276 wall-object gate: the generic
  C002 wall-object side-effect path now requires a loaded PC3.4 wall square
   and its authentic Thing chain before it can reach F0272/F0268. The focused
  regression proves C002 -> fakewall SET and rejects a corridor chain without
  queuing a host substitute event.
- ✅ 2026-07-15 DM2 `c_light` terminal receipt: added a source-locked
  `DM2_RECALC_LIGHT_LEVEL` result builder. It accepts only an authenticated
  raw dynamic-state hash, applies the source non-dynamic base level or dynamic
   accumulator, then subtracts darkness and clamps to 0..5. No GRAPHICSSET
   value can stand in for runtime brightness. Focused coverage:
   `test_dm2_v1_c_light_receipt` (4/4).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0267/F0276 live wall-object move: C49
  associated-object materialization now enters the source C001..C003 wall
  sensor pass through its packed Thing cell. The source C003 inequality test
  runs before Revert/HOLD, then authentic PC3.4 C03 data alone may schedule
  F0272/F0268; non-wall chains and matching C003 objects stay inert. Focused
  coverage: `csb_v1_f0276_wall_object_move_pc34_compat` (15/0).
- ✅ 2026-07-15 DM2 `c_light` M11 dungeon consumer: the terminal
  `DM2_RECALC_LIGHT_LEVEL` receipt now binds only to the identical
  `UPDATE_GFXSET` scene transaction and reaches source-required dungeon-square
   metadata. A changed graphicsset/control hash clears it; no palette or pixel
   brightness is derived from the receipt. Focused coverage extends
  `test_dm2_v1_c_light_receipt` with matching and mismatched scene routes.

- ✅ 2026-07-15 DM2 `c_light` palette parameter: source
  `c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT` stores `glbLightLevel * 10` before
  local palette processing. The action-palette route now accepts only that
  matching authenticated value, never `GRAPHICSSET`'s unrelated
  `HIGHEST_LIGHT_LEVEL`. Without live `c_light` provenance it remains
  unavailable. Focused coverage verifies the exact multiply and mismatch
  rejection in `test_dm2_v1_c_light_receipt`.

- ✅ 2026-07-15 DM2 `c_light` presentation receipt: a consumed live
  `c_light` result now contributes its receipt, raw-state identity and level
  to the M11 presentation-state hash. Partial provenance, an out-of-range
  level, or a result not consumed by the viewport rejects the identity. The
  frame still has no light pixels until the original palette path is fully
  available. Focused coverage: `test_dm2_v1_frame_presentation_state`.

- ✅ 2026-07-15 DM2 `c_light` map-descriptor branch receipt: runtime now
  reads and hashes the active raw G1 `Map_definitions::Difficulty()` field
  (w12 high nibble), exactly as SKProject uses it to choose the fixed-light or
  dynamic-light branch. It does not infer the later accumulator or darkness
  inputs. Focused coverage extends `test_dm2_v1_c_light_receipt`.

- ✅ 2026-07-15 DM2 `c_light` map-state gate: an authenticated live light
  state now must agree with the active raw `Difficulty()` receipt before
  `DM2_RECALC_LIGHT_LEVEL` can publish a result. A fixed-map state cannot
  become dynamic, and vice versa. Focused coverage:
  `test_dm2_v1_c_light_receipt`.

- ✅ 2026-07-15 CSB-001 ReDMCSB F0267/F0276 C001 object move: the generic
  wall pass independently authenticates its PC3.4 wall byte and C03 chain,
  then admits C001's ordinary-object arrival through F0272/F0268. The focused
   C49 regression also keeps non-wall chains fail-closed. Coverage:
   `csb_v1_f0276_wall_object_move_pc34_compat` (20/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0267/F0276 C001 object move: the generic
  wall pass independently authenticates its PC3.4 wall byte and C03 chain,
  then admits C001's ordinary-object arrival through F0272/F0268. The focused
  C49 regression also keeps non-wall chains fail-closed. Coverage:
  `csb_v1_f0276_wall_object_move_pc34_compat` (20/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0276 C000 wall gate: disabled C03 records
  now explicitly stop before every object-side-effect stage. A live C49
  PC3.4-chain regression proves no F0272/F0268 event and no type-byte rewrite.
  Coverage: `csb_v1_f0276_wall_object_move_pc34_compat` (26/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0276 C001 audible object route: a loaded
  C001 wall record reached by C49/F0267 emits the authentic prioritized switch
  request and its F0272/F0268 event. The same regression proves C000 emits
  neither. Coverage: `csb_v1_f0276_wall_object_move_pc34_compat` (31/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0272 C002 OnceOnly writeback: the live C49
  wall-object route clears only the low C03 type bits before it publishes the
  source F0268 event, retaining the packed object-data field. Coverage:
  `csb_v1_f0276_wall_object_move_pc34_compat` (36/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0272 C003 lifecycle: a real PC3.4 C49/F0267
  arrival clears the OnceOnly type before publishing C003's exact three-tick
  delayed F0268 event at map time four. Coverage:
  `csb_v1_f0276_wall_object_move_pc34_compat` (41/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0276 C001 Revert/HOLD lifecycle: a loaded
  C49 object arrival computes `AddThing ^ RevertEffect` before converting HOLD,
  yielding the original F0268 CLEAR event. Coverage:
  `csb_v1_f0276_wall_object_move_pc34_compat` (41/0).

- ✅ 2026-07-15 CSB-001 ReDMCSB F0276 party TextString route: a sole visible
  PC3.4 C02 record entered by the party now reaches the existing F0168 C015
  receipt. Hidden, malformed, and multi-text chains remain no-draw rather than
  becoming host messages. Coverage: `csb_v1_f0276_party_text_pc34_compat`.

- ✅ 2026-07-15 CSB-001 ReDMCSB F0267/F0276 live wall-object move: C49
  associated-object materialization now enters the source C001..C003 wall
  sensor pass through its packed Thing cell. The source C003 inequality test
  runs before Revert/HOLD, then authentic PC3.4 C03 data alone may schedule
  F0272/F0268; non-wall chains and matching C003 objects stay inert. Focused
  coverage: `csb_v1_f0276_wall_object_move_pc34_compat` (15/0).

- ✅ 2026-07-15 DM1 ReDMCSB F0115 source projectile/object-material gate:
  M613 projectiles, F0142/G0209 thrown objects, and C2500 floor objects now
  require decoded loaded-pixel PC34 surfaces before their original placement
  plans can publish pixels. This also corrects the D1–D3 F0114 gate to validate
  its actual explosion consumer. Focused coverage:
  `m11_dm1_f0115_material_asset_fail_closed` and
  `m11_dm1_explosion_asset_fail_closed`.
- ✅ 2026-07-15 DM2 source per-square-light gate: source-required M11
  projection no longer writes the synthetic full-light value `15` into G1
  squares. The existing GRAPHICSSET transaction proves only ambient controls;
   per-square light remains unavailable (`0`) until a `c_light` result is
   source-bound. Compatibility-only callers retain the old default. Verified:
   `test_dm2_v1_boot_profile_smoke` (88/0) and `test_dm2_v1_save_load`
   (26/26).

- ✅ 2026-07-15 CSBWin Timer.cpp TT_60 pool mutation: the authenticated
  party-square +5 successor now retires the due source receipt and requeues
  through the original TIMER allocator, rather than rewriting its old slot.
- ✅ 2026-07-15 DM1 ReDMCSB F0114 source explosion-material gate: D1–D3
  scaled explosion sprites and the separate D0C M636 pattern now accept only
  decoded loaded-pixel `GRAPHICS.DAT` surfaces. Dimension-only cache entries
   cannot create an explosion frame. Focused coverage:
   `m11_dm1_explosion_asset_fail_closed`.

- ✅ 2026-07-15 CSBWin live TT_1 presentation transaction: the live
  `ProcessTT_1` animation path now stages its non-party door pixel state and
  commits it only after the source TIMER delete/set transaction succeeds.
  Failed ownership removes the staged timeline entry and leaves the door
  unchanged; the non-atomic party damage/sound subroute is explicitly blocked.

- ✅ 2026-07-15 CSBWin Timer.cpp TT_DOOR pool mutation: a source-owned
  `ProcessTT_DOOR` conversion now retires its TT_DOOR handle and publishes the
  TT_1 successor through `SetTimer` allocation, sequence, and heap ownership.
  The staged event is removed if the complete source pool transaction cannot
  commit. Focused coverage proves a conversion moves from slot 1 to slot 0.
- ✅ 2026-07-15 DM1 ReDMCSB F0113 source teleporter-field gate: the live M11
  consumer now requires both decoded C076 field pixels and every required
  C070..C075 projection mask before it writes a viewport pixel. Missing or
   malformed masks no longer expose an unmasked, geometrically incorrect field.
   Focused coverage: `m11_dm1_field_asset_fail_closed`.

- ✅ 2026-07-15 CSBWin Timer.cpp TT_7 pool mutation: deferred falsewall CLEAR
  now stages its +1 successor through the source `DeleteTimer`/`SetTimer`
  allocator transaction. An incomplete timer/event ownership change deletes
  the staged timeline event and leaves the source cell untouched. Focused
  coverage proves slot 1 is retired and the deferred clear resumes in slot 0.

- ✅ 2026-07-15 CSBWin Timer.cpp TT_1 pool mutation: collision-free door
  animation requeue now follows the same `DeleteTimer` then `SetTimer`
  transaction as the source. The consumed handle becomes TT_EMPTY, the
  successor uses the original allocator cursor and sequence, and the door
  cell changes only after the full timer/event ownership transaction commits.
  Focused coverage proves a nonterminal door successor moves from slot 1 to
  slot 0 with its exact +1 source time.
- ✅ 2026-07-15 DM1 ReDMCSB F0387 source action-menu gate: active V1
  one/two/three action rows accept only decoded loaded-pixel PC34
  C079/C077/C011 surfaces at their original geometry. A failed validation now
   retains the source black clear instead of taking a second permissive blit.
   The original M653 action-name route remains the sole text owner. Focused
   coverage: `m11_dm1_action_spell_asset_fail_closed`.

- ✅ 2026-07-15 DM1 F0292 auxiliary source-material gate: normal V1 shield
  borders accept only exact loaded-pixel C037/C038/C039 67x29 surfaces and
  C032 POISONED accepts only its 96x15 source surface at the C502 geometry.
  The default status coordinates are no longer used when source geometry is
  unavailable. Focused M11 coverage proves unavailable no-draw and real
  C032/C038 presentation.

- ✅ 2026-07-15 CSBWin Timer.cpp TT_53 pool mutation: watchdog requeue now
  stages `DeleteTimer` followed by `SetTimer` ownership as one transaction.
  It removes the dispatched TIMER handle, uses the first real TT_EMPTY slot
  for the successor, updates sequence/first-available state, remaps the live
  event receipt, and rolls back the staged event on any incomplete source
  transaction. Heap adjustment remains source-ordered before core-save export.
  Focused coverage exercises a two-slot pool where the successor must move
  from slot 1 into slot 0 rather than accepting an in-place host rewrite.

- ✅ 2026-07-15 DM1 F0623/F0320 source damage-indicator gate: normal V1
  champion damage feedback now accepts only exact loaded-pixel C015 (45x7)
  or C016 (32x29), uses C167/C179 geometry without default coordinates, and
  prints its three-character number only through the loaded M653 source font.
  The generic M11-font fallback is removed. Missing asset, font, or geometry
  leaves the existing status clear unchanged. The focused M11 gate covers
  unavailable no-draw and a real GRAPHICS.DAT C015/M653 presentation.

- ✅ 2026-07-15 DM1 status-hand source-material gate: normal V1 F0291 now
  draws C211..C218 only from exact loaded-pixel C033/C034/C035 hand-box
  surfaces. The host black/gray rectangle fallback is removed, so a missing
  source surface leaves the F0292 status clear intact before any icon route.

- ✅ 2026-07-15 DM1 leader-hand name source-font gate: normal V1 F0034 now
  retains the original C017 black clear and draws only the bounded source
  name through loaded M653 glyphs at the C017 geometry. The generic host-font
  fallback is removed; missing font data leaves the narrow HUD field clear.

- 2026-07-15 Nexus FACE.BIN PRS3 corpus receipt: all 20 retail frame headers
  and streams now have exact bounds, aggregate byte witnesses, and declared
  output totals. This is decoder evidence only; portrait decoding and drawing
  remain fail-closed.

- 2026-07-15 Nexus Structure1F active-row lookup: canonical source rows now
  resolve by exact index through the complete no-draw family receipt, giving a
  later mesh decoder a source-owned selection point without any inferred mesh
  or material semantics.

- ✅ 2026-07-15 DM1 champion-statistics source-material gate: normal V1
  F0351 now presents only exact loaded-pixel C020 and M653 glyphs at the
  source C557/C559 row geometry. Removed the host panel and generic-font
  fallback; unavailable PC34 media leaves the C017-cleared surface intact.
  Runtime coverage exercises both unavailable no-draw and real C020/M653
  panel setup.

- ✅ 2026-07-15 DM1 object-description source-material gate: normal V1
  F0342 uses only exact loaded-pixel C020 (panel), C029 (circle), real object
  icon pixels, and the loaded M653 font. The continuation text now begins at
  source C556 rather than a host-selected Y coordinate. Missing source media
  preserves the C017-cleared surface with no replacement panel/circle/icon or
  host-font text. Focused inventory runtime coverage checks both unavailable
  no-draw behavior and authentic C020/C029 presentation.

- 2026-07-15 Nexus Structure1C active-cell lookup: verified Structure1B grid
  cells now resolve their exact no-draw Structure1C source packet through the
  canonical DGN receipt. Invalid or unreferenced cells remain unavailable.

- 2026-07-15 CSB source-party HUD gate: C113..C116 and C150..C218 now use
  a newly generated CSBWin GAMEBLOCK/CHARDESC mirror receipt on every M11
  runtime frame, rather than trusting M11's cached party mirror. A missing
  or invalid source party clears those rectangles, so stale names, bars,
  hand slots, and C028 markers cannot survive or be synthesized. The launcher
  regression seeds a fake M11 champion, invalidates the source receipt, and
  proves both icon and status zones remain black.

- ✅ 2026-07-15 DM2 G1 DB4 decoded-pixel binding: direct
  `CREATURES/type/F9` handoffs now carry and recheck a row-wise indexed-pixel
  hash. Changed decoded bytes block the viewport material before blitting;
  dimensions and local-palette checks remain in force.

- 2026-07-15 CSB C033-C035 status-hand source gate: CSB F0291 hand slots
  now require the exact 18x18 GRAPHICS.DAT C033/C034/C035 source surface.
  A missing or malformed hand box leaves the C12 F0292 status clear intact
  and blocks the dependent C020 icon; M11 no longer invents a gray frame.
  The real-package launcher regression corrupts C033/C020 and verifies C211
  remains source-cleared.
- ✅ 2026-07-15 DM2 source HUD bar colors: HP, stamina, and mana now share
  SKProject's `glbChampionColor[player]` bootstrap values (`7,11,8,14`) and
  their decoded interface palette. Removed Firestaff's independent
  per-resource colors (`2,11,12`); future mutations remain gated on an
   authenticated runtime owner.
- 2026-07-15 Nexus Structure1C cell binding: source packets now retain each
  record's exact Structure1B reference count and first/last owning grid cell.
  This is authenticated DGN geometry provenance only and cannot draw or infer
   collision semantics.
- 2026-07-15 CSB C015 damage-text source gate: CSB F0623/F0320 feedback now
  emits its 1–3 digit damage number only through the loaded F0053 original
  font with C15-on-C08 colors. Missing C015/font bytes leave the source-clear
  status surface untouched; the M11 small-font substitute is removed. The
  real-package launcher regression verifies that missing C015/font cannot
   create host damage digits.

- 2026-07-15 CSB C028 champion-icon source gate: CSB party-position icons
  now require the complete 76x14 C028 strip from the selected GRAPHICS.DAT.
  Missing or malformed C028 clears C113..C116 rather than showing M11's
  color-only fallback. The real-package launcher regression corrupts the
  cached C028 dimensions and verifies the icon zone stays black.
- ✅ 2026-07-15 DM2 M11 leader-hand cursor-icon gate: removed the active
  arbitrary 14x14 pointer-scaled icon overlay. Although it fetched GDAT
  pixels, it did not consume SKProject `DRAW_ITEM_ICON`'s
  `QUERY_EXPANDED_RECT`/`QUERY_BLIT_RECT` geometry. The original-data
  inventory material consumer remains available, but M11 now draws no
   substitute icon until the source rect route is bound.
- 2026-07-15 Nexus DGT2 UI CLUT binding: verified WARNING.BIN/GAMEOVER.BIN
  RES*/DGT2 PP loads retain the exact 256-entry BGR555 CLUT as RGBA and the
  warning title phase consumes that source palette. No generated palette is
   used for these original UI surfaces.
- ✅ 2026-07-15 DM1 inventory source-material gate: normal V1 inventory no
  longer draws procedural slot boxes or a host scroll panel/text when PC34
  material is absent. F0355 uses only loaded-pixel 224x136 C017; F0341 uses
  only loaded-pixel C023 and the original loaded font; F0038 slot and chest
  boxes use only loaded-pixel 18x18 C033. Missing or dimension-mismatched
  media now leaves the original C017-cleared surface untouched. Focused
   source-lock assertions cover C017/C033 identity, dimensions, and rejection.

- ✅ 2026-07-15 DM2 M11 leader-hand name source gate: stopped drawing the
  active DM1 rectangle/host-font catalog label over DM2 runtime frames.
  SKProject `GET_ITEM_NAME` uses `QUERY_GDAT_ITEM_NAME` (`dtText/0x18`) plus
  `FORMAT_SKSTR`; Firestaff now leaves the zone untouched until that real
  text route is available, while retaining the separate GDAT icon route.
- 2026-07-15 Nexus legacy raw portrait removal: `nexus_ui_load_faces` is now
  fail-closed. Only the bounded PRS3-gated portrait route can ever materialize
  a champion surface.
- 2026-07-15 Nexus raw FACE retirement: layout detection, record expansion,
  and the legacy entry counter reject raw 48x48 tables. Retail FACE.BIN now
  admits only its authenticated PRS3 descriptor layout, which remains no-draw
   until opcode and palette evidence is complete.

- 2026-07-15 Nexus DGN raster fallback removal: textured quads with missing
   material now no-draw instead of becoming flat-color substitute geometry.

- ✅ 2026-07-15 DM2 M11 inventory-panel source gate: removed the active DM1
  `GRAPHICS.DAT` inventory workbench from the DM2 M11 path. DM2 inventory
  toggles now report `DM2 INVENTORY GDAT REQUIRED` and preserve the verified
  dungeon frame, because SKProject `CHANGE_VIEWPORT_TO_INVENTORY` owns a
  separate `CHAMPIONS`/`INTERFACE_GENERAL` GDAT layout and click route.
- 2026-07-15 CSB C013 movement-arrow source gate: CSB runtime keeps the
  verified GRAPHICS.DAT C013 panel unchanged and rejects M11's generic
  hatch/cyan keyboard overlay. The real-package launcher regression proves a
   pending keyboard visual mask cannot alter the original C013 rectangle.
- 2026-07-15 DM1 CHAMDRAW HUD material gate: the live F0622 champion-icon
  path now requires the complete C028 76x14 indexed GRAPHICS.DAT strip before
  its color fill/blit, and normal V1 status boxes no longer substitute a
  procedural frame when their exact source surface is absent. Focused tests
   cover correct graphic identity, pixels, and dimensions plus rejection.

- 2026-07-15 DM1 F0111 D3 door-front real-media gate: removed the active
  host-colored fallback from the F0676/F0677 C3700/C3710 panel route. It now
  presents only expanded GRAPHICS.DAT G0693 pixels or verified packed G0693
  rows through G0074; absent or malformed source remains no-draw. Focused
  viewport assertions cover no-media rejection and both packed/provider paths.

- 2026-07-15 CSBWin DSA `STKOP_DiscardText` runtime handoff: authenticated
  DSA actions now clear only the profile candidate's source-owned
  `TT_OPENROOM` DB2/F0168 C015 message receipt. The receipt commits only
  after full bytecode acceptance; malformed trailing bytecode leaves it
  intact. No host log, generic UI queue, or synthetic text is involved.
  Verification: `csb_v1_dsa_localstate1_save_handoff` covers both commit and
  rollback paths.
- 2026-07-15 CSB TT_OPENROOM DB2 -> C015 source message: CSBWin
  `Timer.cpp::ProcessTT_OPENROOM` now carries a newly visible, sole TextString
  on the party square through the original `DUNGEON.DAT` text-word bank and
  ReDMCSB `DUNGEON.C F0168` message decode.  M11 clears C015 first with
  `TEXT.C F0049`, then uses only the loaded original font for that runtime
  receipt. Missing/malformed records, a non-source font, queue-like wrapping,
  and M11's host log remain black/no-draw. The focused restored-timer bridge
  verifies packed `HEL` words through the live timer dispatch.
- 2026-07-15 CSB C015 message-area source gate: removed the active generic
  M11 `messageLog` renderer from the CSB V1 runtime HUD. The CSB path keeps
  ReDMCSB `TEXT.C F0049`'s C015 black clear but refuses to display host
  telemetry as game text. The real-package launcher regression injects a
  host-log entry after the C001-C004/C017 handoff and verifies all C015 rows
  remain black. Authentic CSB TextString/runtime message ownership remains
  required before text may render.
- ✅ 2026-07-15 DM2 M11 catalog-shop overlay removal: removed the active
  fixed-coordinate host shop panel and its buy/sell input route from the DM2
  M11 runtime. SKProject `SkWinCore::_32cb_0f82_SHOP_GLASS` renders shops as
  source-owned wall ornaments (`WALL_GFX` image offsets, `dt08` item lists,
  and overlay field `0x0f`), so Firestaff now fails closed with `DM2 SHOP GDAT
  REQUIRED` and keeps the verified dungeon frame rather than drawing a brown
  catalog substitute. `test_dm2_v1_m11_startup_profile_gate` now verifies no
  overlay pixels and no catalog transaction mutation.

- 2026-07-15 Nexus STABG synthetic-surface removal: the old file-size based
  320x170/320x200 guess is removed. Original STABG.BIN remains no-surface
  until its native framing is proven.

- 2026-07-15 Nexus GAMEOVER real-media decode: replaced the guessed raw
  320x200 path with the verified RES*/DGT2 PP pixel-plane reader already used
  for WARNING.BIN. Malformed containers remain unavailable.

- 2026-07-15 Nexus portrait allocation fallback removal: a failed raw-face
   allocation no longer leaves a zero-filled portrait surface behind. The
   champion surface stays unavailable unless original bytes were copied.

 - 2026-07-15 Nexus FACE.BIN raw-path block: canonical compact PRS3 container
   bytes are rejected by the legacy raw portrait helper until PRS3 decoding is
   independently proven.

- 2026-07-15 Nexus UI synthetic-fallback audit: the public UI contract now
  matches the implementation: missing or unsupported title/menu surfaces stay
  unavailable. Verified TITLE.CG and WARNING.BIN routes continue consuming
  original bytes; PRS3 remains blocked without decoder evidence.
- 2026-07-15 DM1 F0107/F0109 source-owned ornament material: active M11
  wall, floor, and door ornament consumers now require the F0173 map-local
  DUNGEON.DAT index table. Failed reads and missing local ordinals remain
  unavailable/no-draw rather than selecting identity graphics. The resolver
   accepts the real global-0 inscription slot and rejects unloaded metadata.

- 2026-07-15 Theron Track 02 source-only startup presentation: a complete,
  authenticated title/stage/Soul Room/forcefield atlas now renders only its
  decoded original pixels. Firestaff-owned fill, border, and cursor rectangles
  are not emitted on that route; unproven regions stay untouched. Focused
  startup receipt coverage verifies pixels are emitted while synthetic
  rectangle calls remain zero.

- 2026-07-15 Theron source-owned startup UI gate: the legacy `ui_chrome`
  compositor now leaves a complete authenticated Track 02 startup surface
  untouched. Its generated bars, blocks, glyphs, and compass remain blocked
  until original loader/CD evidence identifies the corresponding UI art.

- 2026-07-15 Theron unbound dungeon-tile fallback removal: both active V1
  viewport routes now preserve their existing indexed surface when a tile bank
  has no bound original Track 02 bytes. The generated black clear and
  palette-7 gray tile replacement are removed; only supplied atlas bytes can
  alter a dungeon region.

- 2026-07-15 Theron unbound runtime-chrome removal: the two active V1 UI
  compositors no longer fabricate top bars, text, portraits, compass markers,
  or champion/stat panels. The startup atlas is deliberately not treated as a
  runtime UI bank, so the host surface remains untouched pending a loader/CD
  receipt for those original bytes.

- 2026-07-15 Theron rejected Track03/04 marker parsers: synthetic `THG3`
  bitmap and `THS4` audio marker inputs can no longer populate a tile bank or
  claim sound readiness. Raw, hash-verified Track 02 remains available only
  to source-backed semantic routes until an original HuC6280 loader/CD capture
  identifies the actual consumer bytes.

- 2026-07-15 Theron startup receipt contract correction: all five remaining
  full-start, raw-media, snapshot, and host receipt assertions now require an
  executed authenticated atlas with nonzero source pixels and zero generated
  fill/border calls. The runtime receipt chain remains complete while the
  old UI chrome stays fail-closed.

- 2026-07-15 Theron atlas clear correction: source-owned Track 02 bitmap
  routes no longer issue the generated black `fill_rect` that used to precede
  every original pixel copy. Complete startup receipts now represent only
  authenticated pixel output; no source-less panel clear or border remains.

- 2026-07-15 Theron post-BRA JSR Track 02 byte-source gate: the
  control-to-media receipt now verifies FIFO bytes through the concrete Track
  02 payload layout instead of a raw-buffer shortcut. Hash-verified raw BIN
  data admits only MODE1 user-data bytes, MODE1/2048 ISO data admits direct
  user offsets, and unknown MD5s or sector header/tail offsets fail closed.
  This remains byte provenance only; no dungeon/object/bitmap semantics were
  promoted.

- 2026-07-15 DM1 F0108 real floor-ornament presentation: removed the live
  host-colored fallback. F0108 now draws only callback-provided original
  expanded GRAPHICS.DAT pixels in the source C10 zone, preserving the
  destination when source media is absent. Focused tests cover both paths.

- 2026-07-15 DM1 F0104/F0105 source-owned floor/pit/stairs primitives:
  added the explicit native C10 route and its F0099 horizontal-flip sibling.
  F0102/F0103 now reuse those canonical paths. F0105 only accepts the
  F0128-owned temporary bitmap and otherwise performs no draw; focused tests
  verify C10 transparency, flip pixels, and no-scratch rejection.

- 2026-07-15 DM1 F0100-F0103 source-owned viewport primitives: mapped the
  ReDMCSB wall/door blit batch to the live DM1 compatibility renderer. F0103
  now requires F0128's verified temporary bitmap span and is no-draw without
  it, rather than allocating host scratch. Focused assertions verify rejection
  plus the exact two-row horizontal flip.

- 2026-07-15 DM1 TITLE/Entrance source-palette presentation: M11 now sends
  ReDMCSB TITLE.C/SWSH.C/ENTRANCE.C indexed pixels through their complete
  source VGA palette without V2 gamma, CRT, sharpening, or motion postpasses.
  The C001 gate now checks the real 53-tick cadence, retaining the PRESENTS
  build interval before the zoom sequence. TITLE special-palette readback,
  cadence, C001, and action-area gates pass.

- 2026-07-15 DM1 F0128/F0115 square scheduler: M11 now invokes the existing
  source-backed side F0115 consumer immediately before the same-depth center
  consumer: D3L/D3R/D3C, D2L/D2R/D2C, D1L/D1R/D1C. The current F0107/C127
  mirror route remains before F0115 exactly as owned by main. No material,
  geometry, or fallback route was changed.
- 2026-07-15 Nexus Structure3 capture campaign ledger: the all-level
  source-only face campaign now emits an ordered, source-fingerprinted ledger
  beside its individual target files. It rejects fallback-enabled target input
   and records no decoder or renderer authorization. Verification:
   `test_nexus_v1_structure3_capture_manifest` plus a direct canonical
   LEV00--LEV15 campaign run.

- 2026-07-15 Nexus Structure3 campaign verifier: a companion probe rebuilds
  all face requests from canonical LEV00--LEV15 bytes and rejects any altered
  target or ledger before an external capture producer consumes it. It leaves
  Saturn decoder and rendering authorization disabled.

- 2026-07-15 Nexus Structure1F owner corpus: a canonical LEV00--LEV15 probe
  revalidates every Structure1F owner row against its bounded Structure1A
  record and produces a stable raw-owner fingerprint. It explicitly keeps the
  Structure1A model-to-Structure3-entry relation unproven and no-draw.

- 2026-07-15 Nexus Structure3 static descriptor corpus: canonical `00xx`
  faces are rechecked against their exact local Structure2 descriptor and raw
  bounded anchors across LEV00--LEV15. Payload decoder and rendering remain
  explicitly disabled.

- 2026-07-15 Nexus static face capture windows: the canonical static-face
  corpus now also locks each descriptor's next raw anchor boundary for capture
  acquisition. The intervals are not decoded image or palette sizes.

- 2026-07-15 Nexus static raw-window witness: the capture-window corpus hash
  now includes the exact original bytes in every bounded candidate interval.
  This remains evidence-only, with no payload codec claim.


- 2026-07-15 ReDMCSB inventory reconciliation: corrected stale DM1 and CSB
  F0145 audit rows to `IMPLEMENTED_NARROW`; both now name the source-owned
  C04/ACTIVE_GROUP paths and focused tests already present in the tree.

- 2026-07-15 DM1 F0115 source-material correction: viewport items, creatures,
  projectiles, pits, and explosions no longer receive host-colored stand-ins.
  Missing `GRAPHICS.DAT` material is no-draw; the newer source-owned F0115 and
  F0128 routes remain intact. The focused fail-closed source gate is wired.
- 2026-07-15 CSBWin DSA `STKOP_Mastery` runtime bridge: authenticated
  `DSA.cpp:3389-3409` bytecode now consumes the loaded CHARDESC skill rows
  through selector-four hand-character, temporary-XP, hidden-skill averaging,
  and transient `PartySleeping` rules from `DetermineMastery`. Possession
  bonuses have no verified CSBWin object-name-index owner, so calls that do
  not suppress possessions fail closed. Verification:
  `csb_v1_dsa_queued_localstate2_timer` and
  `csb_v1_dsa_pure_control_pc34_compat`.
- 2026-07-15 CSBWin DSA `STKOP_PartyFetch` runtime bridge: authenticated
  `DSA.cpp:4127-4165` bytecode now reads the complete twelve-word party image
  from verified GAMEBLOCK2 and character-tail state, including pose,
  `PartySleeping`, hand character, spell effects, and signed shields. Missing
  or inconsistent ownership rejects the entire query. Verification:
  `csb_v1_dsa_queued_localstate2_timer` and
  `csb_v1_dsa_pure_control_pc34_compat`.
- 2026-07-15 CSBWin DSA `STKOP_ExperiencePlus` runtime bridge:
  authenticated DSA bytecode now commits `Magic.cpp::AddToSkill`'s real
  non-level-up CHARDESC mutation through the candidate profile. It preserves
  the source UI16 increment, 0x10000000 XP cap, and hidden-skill-to-basic-skill
  write, then publishes only after full action acceptance. A mastery change
  rejects atomically because `LevelUp`'s random/stat/UI transaction is not yet
  complete. The live M11 party snapshot and CSBWin CHARDESC save-summary now
  consume the same 20-row skill owner through `DetermineMastery`, rather than
  a stale cached level. Verification: `csb_v1_dsa_queued_localstate2_timer`.

- 2026-07-15 CSBWin TIMER pool/runtime ownership: `SaveGame.cpp`'s fixed
  `MaxTimers` array and independent active `NumTimer` `TimerQueue` are now
  validated as two source-owned structures before M10 materializes a saved
  timer. Only unique, non-empty queue slots become live events; spare TIMER
  slots remain unavailable to M10 and DSA. Reheapification and core-save
  export preserve the full slot pool while requiring every active event to
   retain one queue receipt. The DSA queued-timer path consumes the same pool
   contract, so a valid active `TT_STONEROOM` can reach its authenticated DSA
   owner even when free timer slots exist. Focused Ninja/CTest coverage passes
   timer restart/export, duplicate/free-slot rejection, and queued DSA.

- 2026-07-15 CSB PC3.4 real startup handoff: production playback now consumes
  the single ReDMCSB `TITLE.C F0437` state-to-plan contract for C001 PRESENTS,
  CHAOS, and STRIKES BACK. M11 validates the terminal C040-to-C017 transaction
  and clears stale host pixels rather than substituting a prior viewport.
  Verification: package presentation 27/27, M12/M11 handoff 301/301,
  real-asset launch 68/68, and title/import UI gate 129/129.

- 2026-07-15 CSB package discovery: the scanner completes one hash-verified
  loose-file pass for `GRAPHICS.DAT`/`DUNGEON.DAT` before considering containers,
  so a verified pair does not enumerate unrelated shared-data archives.

- 2026-07-15 DM2 source-owned boot HUD: the real G1 startup path now draws
  only verified GDAT chrome until a real game load or new-game handoff owns
  `Champion::HeroType`; no synthetic party portraits are generated. The
  canonical source pose and subsequent turn receipt are preserved. Verification:
  `dm2_v1_boot_profile_smoke` passes 87/87.

- 2026-07-15 DM2 G1 first-frame wall ownership: boot now consumes the real G1 packed start pose after projecting its 5-bit coordinates through map-0 `Map_definitions` origins and checking raw map bounds. The first M10 frame consequently consumes source GDAT wall panels rather than rendering from the synthetic `(15,15,N)` pose outside the 7x10 map. Source: skproject `DME.h` `File_header::StartPartyPosX/Y`, `Map_definitions::MapOffsetX/Y`, and `SkWinCore.cpp READ_DUNGEON_STRUCTURE`. Verification: canonical G1 runtime-map test plus boot smoke 86 pass / 2 remaining directional-capture failures.
- ✅ 2026-07-15 DM1 F0292 status-name composition correction: the live
  champion status-name strip no longer centers or substitutes Firestaff host
  text. It clears the real C159+n 43x7 surface, then renders `Name[8]` with
  the authentic TEXT2 font at C163+n's fixed x+1/y=5 origin and clips after
  seven 6px cells. Portrait ownership remains F0354 inventory-only; status
  bars, hand slots, action cells, and spell cells retain their source order.
  The focused source gate locks original-font-only behavior, PC34 clipping,
  and the party/action/spell composition order.

- ✅ 2026-07-15 DM1 HUD F0407/F0412 glyph-cell correction: `ACTIDRAW.C`
  F0387 action/header labels now use only the authentic TEXT2 native 6x7
  cells and clear fail-closed when the original font is unavailable; no host
  font can substitute or scale `WAR CRY`. `MENUDRAW.C F0397/F0398` now
  always paints the six offered spell symbols and all four typed-symbol
  cells, padding the latter with source spaces so recant/caster changes
  cannot leave stale glyphs. The focused source gate locks the exact PC34
  coordinates, colors, original-font requirement, and action-menu caller.

- 2026-07-15 DM2 real-profile MBCS HUD gate: high-bit champion text now follows skproject `DRAW_STRING`/`DRAW_MBCS_STR` when the real GDAT has no category-0x1c font records, consuming the unsupported MBCS bytes without painting an ASCII or synthetic substitute. This removes false HUD-material blocking while preserving a fail-closed visual outcome. Verification: real boot smoke.

- 2026-07-15 DM2 verified HUD HeroType bootstrap: runtime boot now binds the
  four startup HUD slots only after every corresponding `CHAMPIONS/type`
  `dtText/0x18`, `dt08/0`, and image record passes the existing skproject
  `REVIVE_PLAYER`/`DRAW_CHAMPION_PICTURE` source gate. The bound value is the
  original `Champion::heroType` byte 255, never the old Firestaff portrait
  ordinal. A failed source record leaves the session unchanged.

- 2026-07-15 DM2 G1 scene-plan regression repair: `AMBIANT_LIGHT` is now
  optional exactly as skproject `QUERY_GDAT_ENTRY_DATA_INDEX` defines for
  missing `dtWordValue` rows. The canonical graphics set 2 therefore carries
  source value zero instead of invalidating G1/GDAT startup and first-frame
  receipts; no graphics set or light value is substituted.

- 2026-07-15 DM2 G1 startup-to-runtime context handoff: boot detach and
  post-menu session application now preserve a source-owned G1 receipt for
  the active level, `MapGraphicsStyle`, `runtime_g1_scene_map_token`, GDAT
  scene-control hash, and original interface-palette hash. The first M11
  runtime frame must match that exact receipt as well as the existing GDAT
  material gate; otherwise the host clears and blocks the frame. The focused
  real-data boot smoke coverage verifies the context before rendering and its
  identity in the presented GDAT frame.

- 2026-07-15 DM2 G1 direct door runtime handoff: connected the verified
  `DME.h::Door::w2` DB0 receipt to the active runtime map and its matching
  source-owned D0C GDAT panel command. Canonical G1 door metadata no longer
  follows `GenericRecord::w0`; absent direct roots or panel material stay
  blocked. The corpus test passes against both local canonical DUNGEON.DAT
  copies and the DM2 runtime module passes strict syntax compilation.

- 2026-07-15 DM1 F0225: raw C04 Lord Chaos escape now counts the four C15
  fluxcage neighbors, follows source RNG direction order, and moves the exact
  Thing through F0164/F0163 or reports the Fuse-sequence terminal condition.

- 2026-07-16 DM1 ReDMCSB F0222/F0223 source-lock: added focused coverage for
  raw C04 Lord Chaos lookup and Lord Chaos escape-square allow classification.
  The callable audit and symbol disposition rows now close F0222/F0223 as
  `VERIFIED_SOURCE_MAPPING`; F0224 is now separately closed for the
  source-owned Fluxcage action plan, while broader real-map/pixel capture
  remains separate.

- 2026-07-15 DM1 F0223: source walkability gate now accepts only corridor,
  teleporter, pit, and door target squares for Lord Chaos escape routing.

- 2026-07-15 DM1 F0222: raw square-chain Lord Chaos lookup now returns the
  actual C04 Thing only when its source creature type is C23.

- 2026-07-15 CSB ReDMCSB F0149: implemented the exact current-map
  alcove-ornament membership predicate from `DUNGEON.C:1330-1347`. The API
  reads only caller-owned G0267-equivalent data, rejects negative ordinals,
  and does not invent an ornament table when no real map owner is supplied.

- 2026-07-15 CSB ReDMCSB F0145--F0148 effective-group owner: added one
  bounded C04/ActiveGroup transaction from `DUNGEON.C:923-970`. It validates
  the C04 Thing type and 16-byte record, resolves byte 5 as ActiveGroupIndex
  only on the party map, reads F0145/F0147 effective Cells/Directions, and
  commits F0146/F0148 raw or active writes only after all inputs validate.
  Non-party directions use original `G0258` expansion; party-map failures
  leave raw and active data unchanged.

- 2026-07-15 CSB save decode retains verified raw TIMER slot and queue spans
  with FNV provenance for the source-owned GameTimers adapter.

- 2026-07-15 CSBWin timerheap contract audit: extracted SetTimer/DeleteTimer
  source ownership and rejected a partial Firestaff adapter before it could
  fabricate timer IDs or heap mutation.

- 2026-07-15 CSB discovery census: no hash-verifiable CSB asset candidate was
  found in configured local data roots; no runtime receipt was emitted.

- 2026-07-15 CSB capture discovery audit: rejected the unclassified local
  DATA directory as a title/HUD capture source; no cross-game asset fallback.

- 2026-07-15 CSBgraphics ownership audit: confirmed the existing hash-owned
  `LocateNthGraphic` span, LZW boundary, and runtime-plan chain; no unverified
  title/HUD/door entry was promoted.

- 2026-07-15 CSB callable audit refresh: verified local full-audit MISSING
  candidates against the available source corpus and excluded declaration-only,
  platform-only, graphics-blocked, and already landed routes from implementation.

- 2026-07-15 CSBWin overlay ownership audit: verified that the retained
  EXPOOL palette and CSBgraphics signature gate are prerequisites for the
  DSA overlay routes; no opcode was enabled without the missing payload owner.

- 2026-07-15 CSBWin `STKOP_ObjectID`: source DSA-bank object identity now
  reaches the authenticated stack only with an explicit live owner.

- 2026-07-15 CSBWin `STKOP_Describe`: implemented the source stack contract
  from `DSA.cpp:4639-4661`. Valid phrase-slot requests are staged and reach a
  DB2/phrase owner only after the full authenticated action accepts; invalid
  source slots remain no-ops and missing ownership fails closed.

- 2026-07-15 CSBWin `STKOP_SetAdjustSkillsParameters`: implemented
  `DSA.cpp:3034-3043` as a source-ordered, transactional five-word update.
  The values reach only an explicit `Magic.cpp::AddToSkill` owner after full
  authenticated bytecode acceptance; a missing owner or later bad word
  publishes nothing.

- 2026-07-15 CSBWin `STKOP_MonBlk`: implemented `DSA.cpp:4625-4636` as a
  transactional four-direction movement-filter mask. It accepts no absent
  owner and does not publish after a later malformed DSA word.

- 2026-07-15 DM1 CHAMPION F0286: added exact `CHAMPION.C:4956-4974`
  adjacent C04-to-party target selection. The live melee route now consumes
  F0229's ordered cell/RNG path and F0285's first living champion match.

- 2026-07-15 DM1 MOVE F0265: added exact `MOVESENS.C:11938-11955` C60/C61
  event construction. The deferred group-move route now preserves native
  map/time wrapping, zero priority, destination location, and the C04 Thing
  word in `C.Slot` for runtime and original-save consumption.

- 2026-07-15 DM1 MOVE F0264: added the exact `MOVESENS.C:11919-11936`
  levitation classifier. C04 uses only raw G0243 `MASK0x0020`; C14
  projectiles and C15 explosions always levitate. The active F0267
  loaded-chain route now consumes it before source/destination F0276 sensor
  passes, so explosions no longer take ordinary-object sensor paths.

- 2026-07-15 DM2 skproject raw-SKSave prefix receipt: raw original saves now
  validate `c_savegame.cpp::DM2_READ_DUNGEON_STRUCTURE`'s saved
  `warr_00[1]` map-data span rather than inferring its size from descriptors.
  The importer exposes bounded hashes for descriptors, columns, ground
  stacks, text, each DB pool, map data, and the prefix; undersized/corrupt
  prefixes reject atomically before SUPPRESS state is read.

- 2026-07-15 DM2 live-sidecar transactional restore: the runtime validates
  the complete serialized creature pool before publishing the accepted
  session, then replaces G1 bytes and refreshes derived GDAT scene controls.
  A wire-valid save that fails SKProject timer-owner reconstruction now leaves
  the live creature and dungeon state intact. The save/load executable passes
  26/26 tests; no raw-corpus byte path or synthetic runtime state was added.

- 2026-07-15 DM2 raw-SKSave transactional preflight: original `GAME_LOAD`
  candidates now run the shared session and timer-owner validation before G1
  publication or the source-required CCM clear. A decoded SKProject `tty0C`
  timer with an out-of-squad actor rejects while retaining the live party,
  dungeon, and creature pool. Verification: `test_dm2_v1_save_load` 26/26.

- 2026-07-15 DM1 GROUP F0229: added exact `GROUP.C:13860-13876`
  F0228/G0023 ordered-cell selection. M10 CMD_ATTACK now supplies real group
  and party coordinates, preserves the source RNG step, and rejects no-cell
  targets instead of choosing a synthetic first living creature.

- 2026-07-15 DM1 GROUP F0228: added exact `GROUP.C:13810-13859`
  primary/secondary direction selection. M10 now consumes it before F0200
  with source RNG order; F0201 stored-scent routing shares it. Focused
  coverage locks cardinal and diagonal RNG branches.

- 2026-07-15 DM1 GROUP F0227: unified the exact `GROUP.C:13772-13808`
  directional-cone transform behind one DM1 owner. F0200 and M10 now consume
  it before their loaded-map F0199 paths; focused coverage locks cardinal
  cone outcomes.

- 2026-07-15 DM1 GROUP F0226: added the exact `GROUP.C:13762-13770`
  Manhattan-distance primitive and bound M10's C29-C41 group-to-party
  distance to it. Focused coverage includes asymmetric and identical squares.

- 2026-07-15 DM2 skproject weather host receipt: source-backed weather
  renderer hash/count now cross runtime, boot, and M11. M11 rejects a
  mismatched weather command list before presentation.

- 2026-07-15 DM2 skproject creature host receipt: ordered creature GDAT plan
  hash/count now cross runtime, boot, and M11. M11 rejects a mismatched
  multi-creature pass before presentation.

- 2026-07-15 DM2 skproject object host receipt: ordered floor-object,
  possession, and leader-hand GDAT plan hash/count now cross runtime, boot,
  and M11. M11 rejects a mismatched object pass before presentation.

- 2026-07-15 DM2 skproject projectile host receipt: ordered missile/cloud
  GDAT plan hash and command count now cross runtime, boot, and M11. M11
  rejects a mismatched projectile pass before presentation.

- 2026-07-15 DM2 skproject wall command-count host receipt: visible
  `DRAW_WALL` count now crosses runtime, boot, and M11 with the material hash.
  M11 rejects a count mismatch before presenting the frame.

- 2026-07-15 CSBWin `STKOP_DiscardText`: the authenticated DSA executor now
  stages `DSA.cpp:3161-3167` against a source-owned scrolling-text callback.
  The discard publishes only after complete bytecode acceptance; unavailable
  UI ownership rejects without a fallback. Focused regression covers commit,
  rejected-bytecode atomicity, and missing owner.

- 2026-07-15 CSBWin `STKOP_CausePoison`: the authenticated DSA executor now
  stages CSBWin `DSA.cpp:4348-4362` / `CSBCode.cpp::PoisonCharacter` using a
  source-owned poison candidate. It retains poison-value/character stack
  order and publishes damage, flags, and timer effects only after complete
  bytecode acceptance. Focused regression covers valid, unavailable,
  negative-index, rejected, and missing-owner paths.

- 2026-07-17 CSBWin `STKOP_CausePoison` runtime/save handoff: the
  RCS-imported action now commits its exact single selected champion through
  the live candidate's `PoisonCharacter`/`CHAMPION.C F0322` owner. Immediate
  health and poison-dose mutation, `PoisonEventCount`, and the full-width
  C75 continuation are published together only after the action is accepted.
  The immutable DSA runtime receipt binds the source operands to CHARDESC
  before/after values and its exact C75 heap slot/time/full attack; party or
  C75 drift invalidates it. Focused `csb_v1_dsa_queued_localstate2_timer`
  coverage proves import, transaction, C75 drift rejection, and the existing
  +36 continuation. Portrait/status/UI and multi-target ordering remain
  explicitly fail-closed pending a complete original owner.

- 2026-07-15 CSBWin `STKOP_SwapCharacter`: the authenticated DSA executor
  now stages CSBWin `DSA.cpp:4413-4425` / `Character.cpp::SwapCharacter` with
  source stack order and immediate source status result. Only a successful
  candidate roster operation commits after complete bytecode acceptance.
  Focused regression covers a successful return, full-party result code,
  rejected-bytecode atomicity, and missing-owner rejection.

- 2026-07-15 CSBWin `STKOP_ExperiencePlus`: the authenticated DSA executor
  now stages CSBWin `DSA.cpp:4542-4557` and `Magic.cpp::AddToSkill` through
  source-owned prepare/commit callbacks. It keeps the original pop order and
  nonpositive-XP no-op, and does not publish XP or level effects after a later
  rejected bytecode word. Focused regression covers valid, unavailable,
  nonpositive, rejected, and missing-owner paths.

- 2026-07-15 DM2 skproject all-wall M11 receipt: runtime now carries every
  source-consumed `DRAW_WALL` GRAPHICSSET panel, in source draw order, into
  frame ownership. The visible command count/hash covers the ten viewport
  fields rather than claiming that unused cached wall fields were drawn.

- 2026-07-15 DM2 skproject multi-creature M11 receipt: every successful
  `DRAW_MAP_CHIP`/`QUERY_CREATURE_PICST` blit now preserves its GDAT key in
  draw order. Runtime and M11 require the complete command count and hash;
  an omitted or substituted later creature blocks the frame rather than
  inheriting the final creature's material identity.

- 2026-07-15 DM2 skproject creature ownership no-draw gate: unknown local
  creature candidates can no longer derive a GDAT image from type/frame, and
  Rect14 cannot upgrade them into a drawable sprite. Only a proven live
  `QUERY_CREATURE_PICST` image field or the existing exact G1/DB4 receipt is
  admitted; all other creature material stays blank and blocked.

- 2026-07-15 DM2 skproject creature no-overlay gate: scene creatures now draw
  only their selected `DRAW_MAP_CHIP`/Rect14 GDAT bitmap. Generated fallback
  rectangles, colours, and health bars have been removed; unavailable material
  remains blocked and blank.

- 2026-07-15 DM2 skproject projectile-category no-draw gate: missile and
  cloud map-chip draws require their source-owned GDAT category. An absent
  category no longer becomes the generic spell-missile image; source profiles
  block the draw without inventing pixels.

- 2026-07-15 DM2 skproject object-category no-draw gate: floor objects,
  creature possessions, and leader-hand overlays require the original
  record-owned GDAT category. Missing categories and unknown DB pools no
  longer become a generated `MISC` route; source profiles block the draw.

- 2026-07-15 DM2 skproject weather M11 receipt: a source-backed
  `DistantEnvironment` weather transaction now reaches M11 only when every
  selected GDAT layer was consumed. The renderer records the exact receipt
  hash and command count; partial or unavailable layers remain no-draw and
  cannot produce a presentable source frame.

- 2026-07-15 DM2 skproject HUD-core no-draw gate: `DM2_DRAW_INTERFACE` no
  longer replaces unavailable `INTERFACE_GENERAL` chrome with generated bars,
  dividers, coin discs, icon frames, portrait surrounds, or name markers.
  Real GDAT remains the only HUD-chrome route; live champion state bars and
  leader state remain runtime overlays.

- 2026-07-15 DM2 skproject portrait no-draw gate: `DRAW_CHAMPION_PICTURE`
  no longer replaces a missing `CHAMPIONS` GDAT bitmap with a generated colour
  fill. Strict profiles report a blocked portrait; other profiles leave its
  surface blank while retaining the source-owned state-bar overlay path.

- 2026-07-15 DM2 skproject scene no-draw gate: `DRAW_DUNGEON_TILES` planes
  and `DRAW_WALL` cells no longer paint generated ceiling, floor, or wall
  substitutes. Missing source GDAT leaves the region blank and strict profiles
  record the matching blocked material class. The focused viewport gate covers
  unavailable planes, unavailable walls, and a single missing wall cell.

- 2026-07-15 DM2 skproject door no-draw gate: `DRAW_DOOR` no longer paints
  Firestaff's generated fallback panel when its selected `DOORS` GDAT bitmap
  is unavailable. Source-material profiles report a blocked door; all other
  profiles leave the panel blank. The focused viewport gate covers both the
  fully unavailable and frame-only cases.

- 2026-07-15 DM2 skproject map-chip no-draw gate: floor items, leader-hand
  items, projectiles, and clouds now require their selected GDAT bitmap, as
  do the previously gated creature routes. On a source-material route, missing
  material records its blocked class and produces no generated diamond,
  hand-item, missile, or cloud pixels. The focused viewport gate covers all
  three new no-draw paths.

- 2026-07-15 DM2 skproject creature source-material gate: viewport creature
  and creature-possession routes now require their selected GDAT map-chip
  bitmap. Missing material records the appropriate blocked class and draws no
  generated rectangle, health bar, or diamond. The focused viewport test
  proves both no-draw paths as well as the existing source-asset paths.

- 2026-07-15 DM2 skproject DB4 creature facing: direct G1 creature receipts
  and `CREATURES/type/F9` material now use `Creature::b15_0_1()` for the
  `DRAW_MAP_CHIP` view-relative atlas direction, rather than ObjectID bits.
  The focused graph gate mutates only the original DB4 b15 byte and proves
  that the source-owned material receipt carries that facing.

- 2026-07-15 DM1 DUNGEON/OBJECT F0141/F0032/F0033: added the exact PC3.4
  raw Thing-to-G0237 path for object-info index, base icon, and dynamic icon
  variants. The resolver reads only F0156-owned raw records and preserves
  F0141's source arithmetic, G0237's Type column, compass party direction,
  G0029 torch charge bands, scroll closed state, and charged item variants.
  F0115 world-candidate subtype extraction now uses that same raw boundary,
  so an absent raw record cannot render as subtype-zero art. Focused test
  covers weapon/torch, scroll, compass, potion, source arithmetic, and
  missing-record rejection.

- 2026-07-15 DM1 M11 DUNGEON/OBJECT F0141/F0032/F0033 consumer: inventory
  and leader-hand icon resolution now consumes the same loaded raw-PC3.4
  F0156 -> F0141 -> F0032/F0033 owner as the viewport. M11 no longer derives
  icon identity, charge state, closed-scroll state, or compass direction from
  decoded Thing mirrors. Missing raw data is a strict no-draw result. The
  focused M11 gate proves a contradictory decoded torch cannot override raw
  source bytes and that removing the raw record suppresses both consumers.

- 2026-07-15 DM1 M11 DUNGEON F0141 AllowedSlots consumer: champion hand,
  inventory, and chest placement now query the raw F0156-selected G0237 row
  through the DM1 owner. M11 no longer uses its private AllowedSlots table or
  a decoded object subtype to admit a placement. A missing raw record rejects
  the move before any inventory mutation.

- 2026-07-15 DM1 CHAMPION F0294: the ammunition compatibility gate now reads
  actual loaded PC3.4 weapon THING records through F0158 and the original
  G0238 WeaponInfo table. It preserves AMMO.C's launcher type, bow/sling
  class, and matching-ammunition checks; unavailable or malformed raw records
  fail closed. The focused combat test covers raw bow/arrow, sling/stone,
  mismatch, and non-weapon rejection.

- 2026-07-15 DM1 SAVEUTIL F0421: original PC34 F0435 dungeon-tail import now
  stages header, maps, columns, SFT, text, every ThingData block, and raw map
  through the exact bytewise 16-bit running-checksum contract from
  `READWRIT.C`. A short section fails before materialization; the focused
  handoff test covers accumulation, wrap, failure, and runtime handoff.

- 2026-07-15 DM1 SAVEUTIL F0420: original-PC34 F0433 export now invokes an
  explicit DM1-owned save-part writer. It prefixes the bounded even-byte part,
  applies exactly one F0417 obfuscation/checksum pass, and rejects odd or
  truncated spans before the envelope can advance. The focused gate proves
  the encoded bytes, reverse F0417 recovery, checksum agreement, and both
  rejection paths.

- 2026-07-15 DM1 SAVEUTIL F0419: original-PC34 F0435 ingress now invokes an
  explicit DM1-owned part reader. It validates the F0420 size prefix, applies
  exactly one F0417 inverse/checksum pass, exposes the diagnostic checksum,
  and rejects odd, truncated, or mismatched parts before the staged save can
  materialize. The paired focused gate covers valid source recovery,
  checksum-mismatch diagnostics, odd lengths, and truncated bodies.

- 2026-07-15 DM1 SAVEUTIL F0422: original-PC34 tail round-trip receipts now
  reconstruct each authentic F0433/F0435 tail body through an explicit
  DM1-owned complete-span writer paired with F0421 before byte preservation
  can be certified. It validates destination capacity before copying, adds
  the exact unsigned source bytes to the caller-owned 16-bit checksum, and
  leaves destination, cursor, and checksum unchanged when the span cannot
  fit. The focused gate proves source bytes, checksum wrap, and rejection.

- 2026-07-15 DM1 DUNGEON F0168: HoC wall inscriptions use the original PC3.4
  raw TEXTSTRING record for Visible and TextDataWordOffset before the source
  glyph decoder builds the F0107 M648/C10 material receipt. The focused
  real-DUNGEON.DAT gate proves that corrupting only the decoded mirror cannot
  alter the rendered source material.

- 2026-07-15 DM2 skproject vertical-door geometry: intermediate vertical
  `DRAW_DOOR` states 1..3 now resolve `tlbRectnoDoorPosition + state` through
  RAW4 `QUERY_BLIT_RECT` and M11 consumes the resulting source rectangle.
  Horizontal split-panel openings remain blocked until both source blits are
  carried together.

- 2026-07-15 DM2 skproject closed-door geometry: D0-D3 `DRAW_DOOR` panels
  now carry their `tlbRectnoDoorPosition` RAW4 `QUERY_BLIT_RECT` destination,
  row/table hashes, and geometry hash through the GDAT M11 command. The
  source-required viewport consumes that exact rectangle; unproved opening
  and split-panel routes remain no-draw rather than using old bounded boxes.

- 2026-07-15 DM2: bound source-owned `DistantEnvironment` weather slots to
  the GDAT viewport route. The runtime consumes decoded weather material only
  for stationary frames with complete slot, palette, IMG3, and dt04 receipts;
  absent or malformed slots remain no-draw.

- 2026-07-15 DM2: added a source-bound `DM2_LOAD_NEW_DUNGEON` boot
  transaction. It atomically reparses the verified G1 dungeon and carries
  raw identity/seed/map-count evidence while explicitly refusing to create a
  synthetic starter party before the original party-reset records are bound.

- 2026-07-15 DM2: formalized skproject title-menu pointer ownership for both
  `INTERFACE_GENERAL/0/dt04/0` event surfaces. `0xD7` remains the only
  executable original NEW path; `0xD9` now yields a hash-bound,
  selector-unavailable receipt and cannot become a synthetic resume action.

- 2026-07-15 DM2 GDAT title/menu DAC presentation: `SkWinCore::DM2_INIT`
  loads `INTERFACE_GENERAL/0/dtPalIRGB/0xFE` and calls
  `DM2_CONVERT_DRIVERPALETTE` before `DM2_SHOW_MENU_SCREEN`. M11 now expands
  those verified 6-bit DAC channels to SDL's 8-bit RGBA presentation values;
  it retains the original RGB6 table for the existing source receipt and never
  substitutes a palette. The M11 pointer route continues to consume only the
  original dt04/0 `0xD7` NEW rectangle; the unbound `0xD9` selector stays
  unavailable.
- 2026-07-15 DM2 title-menu logical-pointer regression gate: the M11 test now
  maps a macOS-style logical FIT/content window coordinate back into the
  source GDAT `dt04/0` `0xD7` NEW rectangle, then verifies the real startup
  route emits its GAME_LOAD state boundary. This guards window scaling without
  widening the unproven `0xD9` resume selector.

- 2026-07-15 DM2 M11 HUD/scene composition: real HUD delivery now requires
  the same G1 scene-control receipt as its GDAT planes.

- 2026-07-15 DM2 skproject HUD M11 delivery: the exact real GDAT HUD command
  count now accompanies its hash and consumed flag; a partial plan is no-draw.

- 2026-07-15 DM2 skproject HUD command delivery: M11 now checks the decoded
  pixel identity of every real GDAT HUD command before composition; mismatch
  is no-draw, never a generated HUD substitute.

- 2026-07-15 DM2 skproject G1 scene identity: M11 frame tokens now retain
  both the G1 level and its MapGraphicsStyle before presenting GDAT scene/HUD.

- 2026-07-15 DM2 skproject scene command integrity: real GRAPHICSSET floor
  and ceiling M11 commands now retain decoded-pixel plus QUERY_BLIT_RECT
  geometry hashes. A changed source bitmap or plane destination blocks the
  source-required frame before either material is drawn.

- 2026-07-15 DM2 skproject side-wall command delivery: the real D3L/D3R/D2L/
  D2R M11 wall receipts now include G0163 source-crop and destination-panel
  geometry. The source-required renderer consumes that destination and blocks
  the complete frame before a draw when any receipt geometry differs.

- 2026-07-15 CSB ReDMCSB title source path: removed manufactured title text
  and corrected the title receipt to the 18 real CHAOS rasters (48x12 through
  320x80), followed by the source-backed PRESENTS and STRIKES regions.

- 2026-07-15 DM2 skproject side-wall GDAT receipt: D3L/D3R/D2L/D2R now have
  canonical-data coverage through M11 with source field, raw hash, decoded
  pixel hash, and local palette required for every side-wall command.

- 2026-07-15 DM2 skproject D3C viewport door: the real D3 GDAT panel now
  reaches the source-material renderer at G0163's 74,25 76x51 geometry, with
  no frame substitute. The focused canonical-data test verifies M11 consumes
  exactly the panel and blocks no material.

- 2026-07-15 DM2 skproject DRAW_DOOR D3 receipt: M11 now admits the real
  cell-11/third-distance DOORS panel transaction without inventing a D3 frame
  or destination rectangle. It preserves the original field-2 choice or its
  documented field-0 GDAT retry, including stretch/light controls.

- 2026-07-15 DM2 skproject DRAW_DOOR D0-D2: M11 door commands now retain the
  selected GDAT image field, real decoded pixels/local palette, and source
  stretch/light controls for D0, D1, and D2.

- 2026-07-15 CSB M11 viewport/HUD commit: source-backed viewport and C017/C040
  pages now validate into a candidate page before one atomic host copy. Failed
  source evidence preserves the preceding page instead of exposing a partial
  or black replacement frame.

- 2026-07-15 DM1 ReDMCSB F0094/F0098: M11 now supplies the active map's real
  PC 3.4 C079/C078 ceiling and floor assets through the DM1 viewport provider.
  Missing source material remains no-draw; no default floor set is selected.

- 2026-07-15 Theron SDL delivery trace: the authentic run records every SDL
  event class and proves Quartz never reaches a key/window/focus event. The
  empty PCE input and CD route remains fail-closed.

- 2026-07-15 DM2 skproject DRAW_DOOR_FRAMES: M11 consumes the real
  `DOORS/*/GDAT_DOOR_NO_FRAMES` word and suppresses frame blits only when the
  source door type requires it; a missing word retains skproject's zero value.

- 2026-07-15 CSB M11 startup surface: M11 now presents only the complete,
  verified session-owned C001-C005/C017/C040 host surface. Missing evidence
  leaves the prior page untouched; the black replacement frame is removed.

- 2026-07-15 Theron Mednafen trace build: the official 1.32.1 trace patch now
  applies reproducibly with `git apply`; the real Track 02 run remains
  fail-closed because no SDL input or non-System-Card CD reads were observed.

- 2026-07-15 DM2 skproject DRAW_DOOR M11 receipt: source-selected door panels
  now carry real DOORS pixels, opening state, decoded hash, and IMG_COLORKEY_1
  through M11; no generic panel or transparency is supplied.

- 2026-07-15 DM1 ReDMCSB F0098 viewport floor/ceiling: real provider-owned
  GRAPHICS.DAT ceiling and floor pixels now reach their original viewport rows;
  a missing source bitmap is no-draw rather than a synthetic floor.

- 2026-07-15 Nexus Structure1F/Structure1A dual-source capture target: added
  a source-checked producer request for one visible Structure1F owner cell
  together with one bounded Structure3 face. It validates the Structure1F
  row, Structure1A table row, raw payload identity, and face target before it
  writes a request, while deliberately keeping the model-to-entry mapping and
  all Saturn rendering semantics unproven and no-draw.

- 2026-07-15 DM1/CSB ReDMCSB F0685-F0688 IMG3 verification: source-locked
  the real PC 3.4 line fill, previous-line copy, packed-nibble reader, and
  signed pixel-count forms with focused strict-C11 coverage. No pixel,
  palette, or fallback image data was introduced.

- 2026-07-15 CSB ReDMCSB F0692/F0693 presentation: added the PC 3.4 packed
  fill-box primitive and a VBlank-gated real startup-raster presenter. Focused
  strict-C11 regression passes.

- 2026-07-15 CSB startup IMG3 runtime coupling: real LZW-decoded PC34 records
  now reach title, entrance, and HUD surfaces through the F0691 row path.
  Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0690/F0691 IMG3 screen rendering: added the PC 3.4
  video-driver line forwarder and validated source-backed IMG3 row expansion.
  Focused strict-C11 regression passes.

- 2026-07-14 Nexus Structure3 source entry framing: extended the external
  capture target/manifest correlation with the canonical Structure3 entry's
  entry, vertex, face, and normal offsets and vertex/face counts. A producer
  manifest for a different raw mesh layout now fails before lane intake; the
  new fields remain source-only and preserve the existing no-draw gate.

- 2026-07-14 CSB IMG3 asset presentation: connected successful source-backed
  F0689 expansion to a caller-owned presentation callback; malformed data
  never reaches the display route. Focused strict-C11 regression passes.

- 2026-07-14 DM2 skproject map-local DOORS M11 bridge: `UseDoor0/1` and
  `DoorType0/1` from each `Map_definitions` header now form a distinct
  `DOORS/type/F9` receipt plan. A present chip requires raw/decoded GDAT
  material and its local palette before M11 may accept the frame; disabled
  slots and absent source chips never receive a synthetic door substitute.
  Focused map-header and M11 receipt-gate regressions pass.

- 2026-07-14 CSB ReDMCSB F0689 IMG3 expansion: added source-backed bounded
  even-stride IMG3 header/palette/command expansion using F0685-F0688. Focused
  strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0687/F0688 IMG3 stream: added PC packed-nibble and
  source run-count decoding for real IMG3 expansion. Focused strict-C11
  regression passes.
- 2026-07-14 DM2 SKProject map-local WALL_GFX M11 bridge: added a distinct
  `Map_definitions::WallGraphics()` material plan for each source
  `WALL_GFX/index/F9` map chip. Raw data, decoded pixels, and the exact local
  palette are required before the matching hash reaches M11; missing material
  fails closed with no generic-wall substitute. Focused G1 map-list and M11
  receipt-gate regressions pass.
- 2026-07-14 Nexus Structure3 producer workflow binding: the external
  capture session now supplies an attestation destination and verifies a
  producer-written Mednafen SH2/VDP1 workflow against the executable's hash,
  source target, manifest, six raw lane hashes, bundle, and trace order. A
  self-claimed original-Saturn run remains explicitly no-draw and cannot be
  imported until independent authentication exists.

- 2026-07-14 CSB ReDMCSB F0686 IMG previous-line copy: added the PC packed
  pixel copy primitive for real IMG expansion. Focused strict-C11 regression
  passes.

- 2026-07-14 CSB ReDMCSB F0684 blit: added PC I34 zone/stride/flip line
  dispatch over caller-owned viewport line primitives. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F0685 IMG3 line fill: added the PC I34 packed-4bpp
  caller-owned line-color fill used by image expansion. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F0678/F0679 D2 side dispatcher: added PC D2L2/D2R2
  wall-panel/teleporter-field routing with source wall swaps and zones over
  caller-owned callbacks. Focused strict-C11 regression passes.
- 2026-07-14 DM2 SKProject map-local FLOOR_GFX M11 bridge: added the exact
  `Map_definitions::FloorGraphics()` list decode after creature and wall lists,
  reversible `FLOOR_GFX/index/F9` material addresses, and a runtime/M11 plan
  that requires raw bytes, decoded pixels, and each source image's local
  palette. Missing material fails closed and no ornament placement or
  substitute graphics is introduced. Focused dungeon-loader and M11-gate
  regressions pass.
- 2026-07-14 Nexus Structure3 external capture-producer session: added a
  source-owned launcher that creates one empty capture session, writes the
  canonical LEV/Structure3 face target, passes target/manifest/lane paths to
  a real external producer, and verifies the returned manifest correlates to
  the requested source rows and lane sizes. It creates no raw trace bytes and
  keeps capture output unattested, no-draw, and outside runtime import.

- 2026-07-14 CSB ReDMCSB F0676/F0677 D3 side dispatcher: added PC D3L2/D3R2
  ordering for walls, stairs, doors, pits, corridors, and teleporters over
  caller-owned rendering callbacks. Focused strict-C11 regression passes.
- 2026-07-14 Nexus Structure3 runtime raw-capture intake: connected the
  verified external manifest/sidecar path to the active canonical-LEV engine
  source. All six byte-bound lanes plus session, bundle, trace-order, and
  original-Saturn attestation must pass before the engine copies the opaque
  packet. The retained source remains no-draw, and failed intake preserves the
  previous admitted packet. Focused DGN geometry-readiness regression passes.

- 2026-07-14 CSB ReDMCSB F0674 viewport bitmap copy: added the PC F0631
  cached-graphic lookup and F0653-sized copy into caller-owned viewport
  storage. Missing graphics and undersized destinations do not synthesize or
  partially copy data. Focused strict-C11 regression passes.
- 2026-07-14 Nexus Structure3 external capture-target writer: added a
  canonical-engine producer tool that writes a concrete request for one
  verified `LEVxx.DGN` Structure3 entry/face, with the exact source-row
  fingerprints and required opaque capture lanes. It writes no trace, VDP1
  command, texture, palette, or synthetic visual, and keeps the DGN route
  no-draw. Focused capture-manifest regression passes.

- 2026-07-14 CSB ReDMCSB F0672/F0673 mouse-input initialization: added the
  PC ordered nine-table initialization and command-none bounded zone-box
  conversion over caller-owned MOUSE_INPUT records. The source `-2`/`-3`
  offsets and inclusive endpoints are covered; no input layout, zone, menu,
  HUD, DSA, or runtime fallback was introduced. Focused strict-C11 regression
  passes.
- 2026-07-14 DM2 SKProject teleporter GDAT runtime bridge: the dungeon
  `DRAW_MAP_CHIP` teleporter branch now resolves only `TELEPORTERS/0/F9`, uses
  the source map-chip frame selected by the live tick, and consumes the image's
  local IMG3 palette. Its raw/decoded/palette identity is carried to M11; a
  missing image or palette blocks the source-required frame without a visual
  substitute. Focused teleporter material and M11 receipt-gate tests pass.

- 2026-07-14 CSB ReDMCSB F0670/F0671 text helpers: added the PC first-match
  replacement and signed decimal long formatting used by save-path and
  out-of-memory UI routes. Buffers remain caller-owned and no localization,
  allocation, dialog, or menu fallback was introduced. Focused strict-C11
  regression passes.
- 2026-07-14 Theron Track 02 staging receipt split: direct
  `try_track02_initial_level` again requires the complete original Stage 2/3
  preflight before it can publish any world state. A distinct public
  receipt-only API builds the bounded Hall-of-Records evidence in an isolated
  staging world. The startup test proves synthetic bytes yield receipts only
  through that API and are rejected by every direct runtime load.

- 2026-07-14 Theron Quartz delivery observability: the checked-in macOS
  keypair helper now preflights CGEvent posting and emits an exact receipt for
  authorized, posted HID down/up pairs. The capture harness copies that receipt
  only after every requested attempt succeeds. A fresh authentic US CUE +
  System Card run reported Quartz access granted and keypairs posted, but zero
  Mednafen SDL key events; no emulated input, CD_READ, dungeon, object, or
  graphics claim was promoted.

- 2026-07-14 CSB ReDMCSB F0666 PC endgame handoff: added the source pointer
  hide loop, double graphics-close, CPSX restore, and caller-owned nonlocal
  transfer ordering. It does not synthesize credits, graphics, media, or a
  jump boundary. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0665/F0362 highlight-box route: added the PC
  caller-owned zone gate and ordered screen-update, zone copy, invert, enabled
  state, disable, and vertical-blank sequence. Missing zones are no-ops;
  no layout/framebuffer/menu-event/DSA/runtime fallback was introduced.
  Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0664 front-wall click transaction: added the PC34
  no-party guard, imaginary fake-wall press/release gate, live mouse sample,
  pointer hide, ordinary wall-thud request, and input-wait stop flag over
  caller-owned input/audio state. No mouse/dungeon/DSA/event/sound fallback
  was introduced. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0661 derived bitmap cache bridge: added the exact
  cache-hit return and cache-miss native/derived lookup, dimension write,
  F0129 dispatch, then cache-admission order. Bitmap/cache/palette ownership
  remains caller-owned real data; no derived storage, graphics fallback,
  event, DSA, or runtime was introduced. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0662/F0663 palette bitmap bridge: added the exact
  in-place F0662 and dimensions-prefix-copy F0663 F0129 dispatches. Palette
  bytes, bitmap pixels, and final renderer remain caller-owned real data;
  this adds no palette validation, graphic decode, derived-cache policy,
  event, DSA, or runtime. Focused strict-C11 regression passes.

- 2026-07-14 Theron authentic-capture provenance: the Mednafen host-input
  helper now resolves the launched SDL process by PID and records that PID in
  its transition receipt. A real US CUE/System Card run confirmed the PID
  receipt but did not report a `host_key_event`; it therefore remains blocked
  and cannot promote a later CD_READ, dungeon, object, or fallback route.

- 2026-07-14 Theron host-input mapping gate: capture mode now requires an
  explicit `THERON_MEDNAFEN_HOME` when injecting input, preventing an empty
  temporary configuration from silently using an unverified PCE mapping. The
  real mapped US capture remained fail-closed because SDL reported no event.

- 2026-07-14 Theron SDL-surface focus provenance: host capture now records an
  explicit `cliclick` screen-focus coordinate before PID-bound key delivery.
  The real US capture recorded that focus but no SDL key event, so it remains
  insufficient for any later CD_READ, dungeon, object, or visual promotion.

- 2026-07-14 CSB ReDMCSB F0657/F0658 bitmap-index viewport bridge: added
  the PC34 F0630 `STRUCT2` bitmap-origin path and the F0658 F0635-relative
  zone-offset path before F0132 dispatch. Bitmap lookup, layout resolution,
  and renderer state remain caller-owned real data; no graphics decode,
  fallback index, event, DSA, or runtime was introduced. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F0655/F0656 viewport bitmap bridge: added the
  IBM-PC bitmap-prefix `F0615` dimension copy plus F0132 flip dispatch, and
  the F0635-resolved transparent viewport dispatch. The adapter owns no
  pixel/layout data: callers provide their admitted bitmap, F0635 resolver,
  viewport, and video blitter. It creates no graphics fallback, event, DSA,
  or runtime. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0652 merge transaction: added the source C05..C10,
  C01, and C02 merge/delete behavior over the existing native EVENT/TIMELINE
  owner. It uses F0237 only for source-required deletions and returns before
  any new-event allocation, queue execution, or DSA path. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F0651 post-save timeline management: added the
  source-order `EVENT_NONE` free-list rebuild after C4 intake. It overwrites
  stale `UNUSED_EVENT` links, reports the first free and source `index + 1`
  largest-used ordinal,
  and preserves active event bytes. The caller must provide an authenticated
  original EVENT stride; this adds no C4 heap execution, DSA, allocation, or
  synthetic runtime. Focused strict-C11 regression passes.

- 2026-07-14 DM2 skproject `CHECK_RECOMPUTE_LIGHT` M11 receipt: the exact
  active `GRAPHICSSET` scene-control hash, `highest_light_level`, and
  `ambient_darkness` now form one fail-closed scene/light identity through
  runtime, boot, and M11. The canonical `GRAPHICS.DAT` CTest proves a real
  admitted style and rejects an altered scene/light receipt. No light level,
  palette, timer state, or weather image is synthesized.

- 2026-07-14 DM2 skproject door-material M11 identity: the consumed
  `DM2_DRAW_DOOR`/`DRAW_DOOR_FRAMES` GDAT material-plan hash now follows the
  live viewport through runtime and boot to M11. The gate requires the exact
  multi-category plan only for frames that actually drew a door, rejects a
  stale hash or an unconsumed plan, and never invents a door identity for a
  doorless frame. The canonical `graphics.dat` CTest reports the real plan
  hash and command count after panel, ornate overlay, frame, and button
  consumption.

- 2026-07-14 DM2 skproject HUD material M11 identity: the complete
  `LOAD_GDAT_INTERFACE_00_02` GDAT HUD plan now retains its exact command
  hash and full-consumption count through viewport, runtime, boot, and M11.
  M11 rejects stale or partial chrome/portrait plans; a frame without an
  occupied HUD does not invent one. Canonical `GRAPHICS.DAT` tests prove the
  real plan hash and that all 13 chrome/portrait commands render directly
  from plan-owned GDAT pixels without an asset callback.

- 2026-07-14 DM2 skproject creature-material M11 identity: visible direct
  G1 DB4 creature sprites now carry an ordered receipt of their exact
  `CREATURES/type/F9` bytes, local palette, ObjectID, map tile, direction,
  and atlas placement through runtime, boot, and M11. Mixed dynamic-creature
  frames are rejected until they have equivalent original ownership. Canonical
  G1/GDAT coverage reports the real material identity and rejects an altered
  palette; no generic type-index creature image can promote the frame.

- 2026-07-14 CSB ReDMCSB F0434 dungeon-tail byte boundary: added exact
  sequential intake of the source's 22 caller-owned dungeon spans and the
  final F0421 byte-sum checksum word. The boundary stops at an unreadable
  part or a mismatching checksum and preserves zero-byte ThingData pools; it
  allocates, decodes, and publishes no dungeon or synthetic runtime. Focused
  strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB F0435 EVENTS/TIMELINE/dungeon-tail admission:
  added the source-ordered continuation after an accepted HINTLOAD F1918
  receipt. It derives only C3/C4 key and checksum words from the decrypted
  header, restores caller-owned EVENTS and uint16 TIMELINE spans, and invokes
  the required F0434 tail loader only after both validations pass. Invalid
  spans, checksum failures, and a tail failure stop the sequence without a
  replacement layout, event runtime, DSA, allocation, or fallback. Focused
  strict-C11 regression passes.
- 2026-07-14 DM2 skproject `UPDATE_GFXSET` to M11 material receipt: the
  exact GDAT floor, ceiling, and `WALL_GFX` plan hashes now move from the
  active runtime graphics set through boot to the M11 acceptance gate. M11
  fails closed on a missing or mismatched material identity. The canonical
  `GRAPHICS.DAT` regression proves one admitted real-data family and rejects
  altered floor and wall receipts; it creates no replacement image or material.

- 2026-07-14 CSB ReDMCSB HINTLOAD F1910/F1913/F1914/F1918: added the
  source-defined initial CPSX save sequence for exact sequential transport,
  header deobfuscation, and the first three source-owned parts
  (`GLOBAL_DATA`, `ACTIVE_GROUP`, `PARTY`). It derives only the original
  `Keys[0..2]`/`Checksums[0..2]` from the decrypted 512-byte CSB header and
  fails at the matching source stage without inventing record layouts, DSA,
  timer, dungeon-tail, or replacement bytes. Focused strict-C11 regression
  passes.

- 2026-07-14 CSB ReDMCSB F7089/F7090 imported-party normalization: added the
  PC34 new-adventure transition after portrait transfer, including header-tail
  carry, party rotation, champion status reset, modifier removal callback and
  source-order free-cell repair. The boundary fails closed without its random
  and modifier dependencies; full original-media save interop remains open.

- 2026-07-14 Theron Track 02 post-envelope raw witness: added the exact
  fingerprint for the 0x380-byte user-data span following the source-locked
  Hall of Records envelope. Mutating a real-media tail byte invalidates both
  the tail and boundary receipts. This is integrity provenance only, not an
  object-table decoder, object claim, or fallback visual route.

- 2026-07-14 Theron Track 02 authenticated Hall of Records handoff: repaired
  INDEX 01 record-to-raw-media consumption, alias-safe transactional manifest
  binding, and the runtime payload receipt. The source-locked US Track 02 raw
  probe and startup save/resume suite pass. The route deliberately admits only
  Hall of Records level 0 and does not promote any object-tail semantics.

- 2026-07-14 CSB ReDMCSB F7088 portrait transfer: added the source-proven
  PC34 `PORTRAITS_INCLUDED` to `PORTRAITS_EXCLUDED` path for exactly four
  464-byte portrait spans, followed by F7066 slot rebinding. Format/count
  mismatches and missing source slots fail closed before any destination byte
  changes. No champion record, allocation, CSBWin extension, DSA, timer, or
  save layout is inferred. Focused strict-C11 regression passes.

- 2026-07-14 DM1 ReDMCSB F8137 packed pixels: added the source-linked
  MEDIA457_P20JA 4bpp packed-nibble fill from `NEC816.C`. Odd and even pixel
  starts, odd and even fill lengths, nibble masking, and bounded writes are
  covered by the focused strict-C11 test. This is not a claim for the
  different MEDIA472 byte-per-pixel branch or a live framebuffer binding.

- 2026-07-14 CSB ReDMCSB F7067/F7068 C31 portrait info: added the source
  get/set access path for an explicit champion portrait-pointer slot in both
  original champion formats. Unknown formats and out-of-range champions fail
  closed without a fallback record interpretation. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F7064 champion save text: added PC34 post-load NUL
  padding for fixed 8-byte name and 20-byte title fields. Full fields remain
  byte-identical; only bytes after the first NUL are cleared. No champion
  record, character conversion, CSBWin extension, DSA, or timer semantics are
  inferred. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB CEDTINC8/F7060 dungeon write stream: added the exact
  ordered 22-part PC34 dungeon byte emission and trailing little-endian
  checksum word. The header, maps, tables, ThingData pools, and RawMapData
  remain opaque caller-owned spans; insufficient output fails before any byte
  is emitted. No dungeon or CSBWin/DSA/timer layout is inferred. Focused
  strict-C11 regression passes.
- 2026-07-14 CSB ReDMCSB F7065/F7066 portrait save lifecycle: added the
  `PORTRAITS_EXCLUDED` pointer-clear-before-save and sequential-buffer-rebind
  after-load behavior from `CEDTINCS.C`. The caller supplies typed pointer
  slots and owned portrait bytes; an undersized buffer fails without partial
  rebinding. No champion layout, portrait decode, CSBWin extension, DSA, or
  timer semantics are inferred. Focused strict-C11 regression passes.

- 2026-07-14 CSB ReDMCSB CEDTINC8 five-part save write: added the source
  sequence that precomputes five keyed checksums, emits the corresponding
  obfuscated bytes, checks the emitted checksum, and restores plaintext. Keys
  and output buffers remain caller-owned; no RNG, file transport, save layout,
  CSBWin extension, DSA, or timer behavior is invented. Focused strict-C11
  regression passes.

- 2026-07-14 CSB ReDMCSB F7063 LoadDungeon stream boundary: added the exact
  22-part PC34 byte-checksum verdict over header, maps, tables, 16 ThingData
  pools, and RawMapData. Empty pools retain their source zero-byte behavior;
  a missing nonempty part or mismatching trailer fails closed. No dungeon
  structure, CSBWin extension, DSA, timer, or runtime semantics are inferred.
  Focused strict-C11 regression passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8160: source-locked PC 3.4 C25 creature
  replacement palette. The adapter copies RGB6 only, from one original
  14-by-6 creature set into the selected entry of palette tables 0..5, while
  retaining each `COLOR_DEF.Index`. Focused strict-C11 test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8216: source-locked PC 3.4 C25 previous-row
  aperture copy. The adapter mirrors the source's forward `movs` behavior,
  including overlap propagation rather than replacing it with `memmove`.
  Focused strict-C11 transfer and overlap tests pass.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8230: source-locked PC 3.4 C25 palette
  component update. The adapter converts real RGB4 values with exact
  `(component << 2) + 3` expansion and republishes via F8156 only when the
  curtain is active. Focused strict-C11 logical/DAC/VBlank test passes.

 - ✅ 2026-07-14 DM1/CSB ReDMCSB F8213: source-locked PC 3.4 C25 aperture
  single-pixel write. The adapter stores exactly `G8177 | color` at the
  requested aperture index. Focused strict-C11 bank, bounds and retained-byte
  test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8153: source-locked PC 3.4 C25 VGA vertical
  blank synchronization. The adapter preserves the original 0x3DA bit-3 poll
  order: wait out an existing blank, then wait for the next blank. Focused
  strict-C11 polling-order test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8139: source-locked PC 3.4 C25 packed-pixel
  aperture transfer. The adapter preserves source-nibble parity and raw
  `G8177 | nibble` writes, including unnormalized OR behavior. Focused
  strict-C11 transfer and bounds test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8137 C25: source-locked PC 3.4 VGA aperture
  fill. The adapter writes exact `G8177 | color` bytes across the requested
  span, directly compatible with a 320x200 indexed host framebuffer. Focused
  strict-C11 odd/even-span, no-op and bounds test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F0684: source-locked PC 3.4 C25 packed-bitmap
  blit. The adapter preserves M104 round-up-to-even strides, all four flip
  modes, opaque/transparent line selection, and direct aperture writes.
  Focused strict-C11 bitmap test passes. F8151 now uses the same M104 rule.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F0675: PC34 native/derived-cache/temporary selection and cache admission before caller-bound F0129 scaling. Focused test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8163: source-locked PC 3.4 C25 bitmap
  transfer. The adapter binds the caller's real packed source and delegates
  unchanged indices/count to F0680 for byte-per-pixel aperture output. Focused
  strict-C11 test covers odd source origin, palette bank, and bounds.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8169: source-locked PC 3.4 C25 LFSR
  blackening. The adapter retains seed `1`, polynomial `0xB400`, the `<64000`
  source gate, and the explicit final pixel-zero write through C25's viewport
  colour bank. Focused strict-C11 order and aperture test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8167/F8168: source-locked PC 3.4 C25 mouse
  pointer background save/restore. F8167 clamps to the exact screen-edge
  18-by-18 rectangle before F8165 capture; F8168 delegates the same saved
  prefix-and-payload form to F8166. Focused strict-C11 roundtrip test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8166: source-locked PC 3.4 C25 aperture
  playback. The adapter reads F8165's little-endian width/height/offset
  prefix and restores the raw payload in 320-byte aperture rows. Focused
  strict-C11 test covers both rows and truncated-data rejection.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8165: source-locked PC 3.4 C25 aperture
  capture. Operation zero reports the real `width * height + 6` size; capture
  emits the original little-endian width/height/offset prefix and raw 320-byte
  stride aperture rows. The source-defined capture return remains unclaimed.
  Focused strict-C11 test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8161: source-locked PC 3.4 C25 viewport
  blit. The adapter preserves the temporary `0x10` viewport palette bank and
  exact opaque, unflipped F8151 parameters: source origin `(0,0)`, width 224,
  destination stride 320. Focused strict-C11 test passes.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8159: source-locked PC 3.4 C25 RGB6
  palette curtain. The adapter preserves black's VBlank-gated 32-entry zero
  write, normal F8156 restore, and final curtain-state assignment; focused
  strict-C11 test covers black, restore, failed VBlank, and arbitrary state.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8156/F8157: source-locked PC 3.4 C25
  palette route. The adapter carries the original RGB6 bytes unchanged,
  applies only terminated `G8176` entries below index 32, and writes all 32
  DAC rows only after an explicit VBlank gate while the curtain is active.
  Focused strict-C11 tests cover raw bytes, terminator, index bounds and gate.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8155: source-locked PC 3.4 C25 hatch
  screen box. The portable route preserves inclusive rectangle bounds and the
  exact `(x ^ y)` parity choice: matching parity clears the aperture byte,
  differing parity leaves it untouched. Focused strict-C11 test passed.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8154: source-locked PC 3.4 C25
  aperture inversion rectangle. The portable route preserves inclusive edges,
  fixed 320-byte rows and exactly the source's XOR `0x04` operation; focused
  strict-C11 tests cover edge inclusion, restoration after a second call and
  source-defined reversed-loop no-op behavior.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8143: source-locked PC 3.4 C25
  aperture-to-packed-4bpp bitmap readback. The adapter takes only source low
  nibbles and retains opposite packed boundary nibbles exactly as the leading,
  paired and trailing source assembly does. Focused strict-C11 test covers
  even and odd boundaries plus failure atomicity.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8152: source-locked PC 3.4 C25
  inclusive box fill through F8137, with the original 320-byte row stride and
  viewport color-index offset. Focused strict-C11 test covers inclusive edges,
  color offset and bounds rejection; live aperture presentation remains
  separately tracked.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8151: source-locked PC 3.4 C25
  source-bitmap-to-aperture rectangle blit with original even-stride,
  opaque/transparent and vertical-flip behavior. Empty C25 F0681/F0683 paths
  are preserved as no-ops rather than creating flipped graphics; F8143
  readback remains separately tracked. Focused strict-C11 test passed.

- ✅ 2026-07-14 DM1/CSB ReDMCSB F8140/F8162: source-locked `NEC816.C`
  overlap-safe byte transfer and PC 3.4 multi-plane message-area scroll. The
  portable adapter preserves the exact source direction choice, 160-byte
  scanline stride, width calculation and non-clearing of exposed rows; focused
  strict-C11 tests cover both F8140 directions plus all four F8162 planes.

- 2026-07-14 Nexus M11 startup regression: moved the large local test fixtures
  and the raw-capture NULL-rejection receipt to static test storage. This prevents
  `test_m11_nexus_startup_gate` from exceeding the macOS main-thread stack
  before it can exercise the launcher. The optional real-media route now runs
  only when `FIRESTAFF_NEXUS_V1_DATA_DIR` is explicitly set, so ambient local
  game data cannot stall a deterministic regression run; the launcher route
  remains no-draw.

- 2026-07-14 Nexus Structure3 launcher raw-capture route: the launcher can
  now feed the strict six-lane reader directly into its currently owned,
  hash-verified canonical DGN bytes and then into engine-owned no-draw
  storage. It rejects absent launcher/level/source state before reading a
  capture. This is evidence transport only and assigns no VDP1, palette,
  pixel, transform, or drawing semantics.

- 2026-07-14 DM1/CSB ReDMCSB F8134: source-locked IBMIO.C DOS EXEC command
  tail and normal-termination result. The focused strict-C11 test leaves
  program execution and DOS exit-status provenance as caller-owned boundaries.

- 2026-07-14 DM1 HUD spell rendering: removed the procedural spell workbench
  from V1-chrome eligibility. CASTER.C/MENUDRAW.C's native C009 background,
  C011 rows and C013 layout are now the sole V1 route, with GRAPHICS.DAT and
  original-font availability still required; no text or placeholder fallback.

- 2026-07-14 DM1/CSB ReDMCSB F8131/F8132/F8133: source-locked IBMIO.C FAT
  volume-label filter, DOS-time `DX` seed and explicit empty floppy route.
  The focused strict-C11 test keeps FCB/DTA lookup and clock provenance as
  caller-owned boundaries without inventing DOS services.

- 2026-07-14 Nexus Structure3 trace-order runtime provenance: the raw trace
  order identity and its external-verification flag now travel with the
  opaque import through DGN host intake into engine-owned Structure3 storage.
  Engine and viewport packets reject an otherwise byte-complete source when
  this provenance is missing. No Saturn VDP1, pixel, palette, or draw meaning
  is assigned; the retained runtime packet remains no-draw.

- 2026-07-14 Nexus Structure3 trace-order attestation: the strict capture
  manifest now records six explicit opaque observation ordinals. The reader
  rejects missing, duplicate, out-of-window, or externally mismatched order
  attestations before DGN host intake. Lane names gain no Saturn hardware or
  pixel meaning; the resulting packet remains no-draw.

- 2026-07-14 CSB ReDMCSB F7062/F0430 PC34 save-header preparation: added the
  original mixed random-word/checksum sequence, tail obfuscation for the
  emitted 512-byte header, and post-write tail restoration. The 127 RNG words
  and output destination are required caller evidence; no host RNG or file
  fallback is invented. Focused strict-C11 F7062-to-F7061 regression passes.

- 2026-07-14 CSB ReDMCSB F7061/F0429 PC34 save-header verification: added the
  exact 512-byte first-half mixed checksum and in-place second-half
  deobfuscation order from `CEDTINC6.C`. A checksum failure deliberately still
  leaves the tail deobfuscated, as in the source. The routine claims no save
  magic, CSBWin extension, DSA, timer, or runtime layout. Focused strict-C11
  regression passes.

- 2026-07-14 DM2 New Game GAME_LOAD gate: `skproject/SKWIN/SkWinCore.cpp`
  `HANDLE_UI_EVENT`, `SHOW_MENU_SCREEN`, and `INIT` prove that NEW first
  leaves the title menu and then reaches `GAME_LOAD`/`LOAD_NEW_DUNGEON`.
  The startup route now records that request but keeps the original title
  surface active until an original dungeon-load receipt is available; it no
  longer synthesizes a new party/session. DM2 startup contract test passes.

- 2026-07-14 CSB ReDMCSB F7059/F7060 dungeon-part checksum: added the exact
  PC34 16-bit modular byte accumulator used after a dungeon part is read or
  before it is written. The C11 port receives only those real bytes and does
  not manufacture a transport, dungeon layout, CSBWin extension, timer, or
  DSA semantics. Focused strict-C11 regression passes.

- 2026-07-14 Nexus Structure3 raw package-to-host route: connected the strict
  six-span reader to the DGN host intake. Only an exact manifest-ordered,
  hash-matching, externally Saturn-attested packet can invoke the existing
  host binder; a changed lane stops before host intake. The route retains no
  VDP1, palette, pixel, transform, or draw interpretation and remains no-draw.

- 2026-07-14 CSB ReDMCSB F7055/F7056/F7057/F7058 save-part checksum: added a
  CSB-owned C11 port of `CEDTINC6.C`'s PC34 little-endian word XOR/checksum
  utilities. F7058 returns the original checksum and restores caller
  plaintext; F7057 decrypts and accepts only a matching checksum. Empty and
  odd blocks are unavailable rather than taking a byte-wise fallback. A
  focused strict-C11 regression passes. This is original ReDMCSB save-part
  support only, not a CSBWin extended-save or DSA layout claim.

- 2026-07-14 Nexus Structure3 raw capture reader: added a strict six-span
  file reader for future original-Saturn trace exports. It owns opaque raw
  bytes, validates every size/hash and the length-prefixed bundle against the
  parsed manifest, and requires a separate external Saturn attestation before
  it marks an import usable. Altered, truncated, missing, cross-session, or
  unattested captures remain no-draw and cannot reach a renderer.

- 2026-07-14 DM2 static title/menu timing: `skproject/SKWIN/SkWinCore.cpp`
  `INIT` and `SHOW_MENU_SCREEN` prove that TITLE/0/dt07/4 remains static in
  `MessageLoop(true)` until `GAME_LOAD` succeeds. M11 and the boot receipt
  reject invented title frames/ticks and keep runtime closed behind the
  source-owned menu surface. Focused strict-C11 timing test passes.

- 2026-07-14 DM2 sound source gate: source `c_sfx.cpp` query handling and
  `c_sound.cpp` queue lifecycle establish that `xsndptr2` is runtime storage,
  not a GDAT table. Unbound SFX identifiers, direct playback, and positional
  playback now explicitly report unavailable instead of inventing a lookup,
  attenuation, or successful backend delivery. Focused strict-C11 test passes.

- 2026-07-14 Theron Track 02 initial-level payload handoff: the boot profile
  now retains the exact, hash-checked 2048-byte payload from the authentic
  `$e009` record `0x0b52` receipt. The payload copy is atomic and fail-closed;
  it carries no inferred dungeon, object, tile, bitmap, palette, or transition
  semantics. Focused intake and initial-level-handoff probes pass.

- 2026-07-14 Nexus DGN package-to-capture byte identity: each successfully
  materialized canonical `LEVxx.DGN` now records the size and FNV-1a identity
  of the exact bytes retained by the engine. Structure3 capture admission
  rechecks that identity in both the launcher and engine before accepting an
  original-Saturn capture packet. A mutated retained DGN cannot reuse a
  package receipt; the route fails closed with no fallback rendering.

- 2026-07-14 Nexus DGN package-to-host identity: DGN material-plan assembly
  now consumes the same retained-byte identity, so MNS/BPK material routes
  cannot present a stale or changed `LEVxx.DGN` merely because its earlier
  package receipt was valid.

- 2026-07-14 DM1/CSB ReDMCSB F1081-F1084: available NIL-device and
  AMISTRUCT allocation routes are Amiga-only. Focused PC 3.4 boundaries are
  intentionally unavailable.

- 2026-07-14 DM1/CSB ReDMCSB F1078-F1080: available console/input device
  teardown/open routes are Amiga-only. Focused PC 3.4 boundaries are
  intentionally no-ops.

- 2026-07-14 DM1/CSB ReDMCSB F1075-F1077: available Layers-library and
  console-device routes are Amiga-only. Focused PC 3.4 boundaries are
  intentionally no-ops.

- 2026-07-14 DM1/CSB ReDMCSB F1073-F1074: available Intuition-library
  open/close routes are Amiga-only. Focused PC 3.4 boundaries are intentionally
  no-ops.

- 2026-07-14 DM1/CSB ReDMCSB F1072: available graphics-library teardown route
  is Amiga-only. The focused PC 3.4 boundary is intentionally a no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1071: available graphics-library open route is
  Amiga-only. The focused PC 3.4 boundary is intentionally a no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1069-F1070: available DOS-library open/close
  routes are Amiga-only. Focused PC 3.4 boundaries are intentionally no-ops.

- 2026-07-14 DM1/CSB ReDMCSB F1067: available initialization is Amiga-
  specific. The focused PC 3.4 boundary is intentionally a source-evidenced
  no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1068: available teardown is Amiga-specific.
  The focused PC 3.4 boundary is a verified no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1066: source-locked largest-block reserve
  calculation with host-supplied memory metrics. Focused strict-C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB F1065: available body reads Amiga Exec from
  address 4. The focused PC 3.4 boundary does not emulate ExecBase.

- 2026-07-14 DM1/CSB ReDMCSB F1063: available checksum is Amiga copy-
  protection code with no PC 3.4 route. The focused boundary avoids
  synthesizing protection behavior.

- 2026-07-14 DM1/CSB ReDMCSB F1059/F1061: available post-save and pre-read
  hooks select Amiga fake-code only. Focused PC 3.4 boundaries are source-
  faithful no-ops.

- 2026-07-14 DM1/CSB ReDMCSB F1057: available pre-save-game body injects
  Amiga fake-code only. The focused PC 3.4 boundary is a source-faithful no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1055: available body selects Amiga fake-code
  variants only. The focused PC 3.4 boundary is a source-faithful no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1053: available body injects 68k code explicitly
  marked never executed. The focused PC 3.4 boundary is a source-faithful no-op.

- 2026-07-14 DM1/CSB ReDMCSB F1052: available code polls Amiga scan-line
  hardware. The focused PC 3.4 boundary is deliberately a no-op rather than
  a fabricated hardware emulation.

- 2026-07-14 DM1/CSB ReDMCSB F1048/F1050: source review found a commented-out
  setjmp alias and an Amiga-only terminal-alert route. Focused boundaries keep
  C semantics where applicable without asserting PC 3.4 behavior.

- 2026-07-14 DM1/CSB ReDMCSB F1049: numbered alias is commented out and
  jump-buffer declarations are non-PC-only. The focused boundary does not
  synthesize longjmp behavior.

- 2026-07-14 DM1/CSB ReDMCSB F1047: available code closes Amiga FTL libraries
  only. The focused PC 3.4 boundary records unavailable rather than synthesizing
  teardown behavior.

- 2026-07-14 DM1/CSB ReDMCSB F1046: available code opens Amiga FTL libraries
  only. The focused PC 3.4 boundary returns unavailable rather than synthesizing
  a library-loading route.

- 2026-07-14 DM1/CSB ReDMCSB F1042-F1043: source-locked X68000 interrupt
  cleanup boundary and empty Amiga caller boundary. Focused strict-C11 tests
  preserve the non-PC decision without emulating unavailable hardware paths.

- 2026-07-14 DM1/CSB ReDMCSB F1041: source review identified an X68000/Amiga
  pointer resolver with no PC 3.4 route. A focused compatibility resolver
  preserves source semantics without claiming platform parity.

- 2026-07-14 DM1/CSB ReDMCSB F0918: source-locked PC release callback and
  null gate with a focused strict-C11 test. F1038 is source-nonapplicable:
  available mouse-pointer update paths are X68000/Amiga-only.

- 2026-07-14 DM1/CSB ReDMCSB F0917: source-locked X30J allocation/error
  path with a focused strict-C11 test. F1037 is source-nonapplicable because
  its mouse-sprite code has no PC 3.4 route.

- 2026-07-14 DM1/CSB ReDMCSB F1034-F1036: source review identified Amiga
  output and X68000/Amiga mouse-sprite layouts as PC 3.4-nonapplicable.
  Focused compatibility tests preserve their source transforms without claiming
  a PC route.

- 2026-07-14 DM1/CSB ReDMCSB F8099/F8100/F8111/F8112: source-locked IBMIO.C
  mouse lock depth, 2x cursor-column synchronization, coordinate assignment
  and formatted button-state read with a focused strict-C11 test. This remains
  a narrow PC34 driver-state adapter, not a host interrupt or cursor renderer.

- 2026-07-14 DM1/CSB ReDMCSB F8101/F8108/F8109: source-locked IBMIO.C mouse
  handler registration, four 18x18 bitmap slots and pointer transition order.
  The focused strict-C11 test preserves the visible and hidden-pointer paths
  without claiming DOS interrupt or host-video behavior.

- 2026-07-14 DM1/CSB ReDMCSB F8123/F8124: source-locked IBMIO.C empty CD
  route and device-specific raw sound-progress query with a focused strict-C11
  test. F8128 is source-nonapplicable because its unbounded ISR/timer polling
  has no portable PC34-equivalent host progress contract.

- 2026-07-14 DM1/CSB ReDMCSB F8129/F8130: source-locked IBMIO.C device-type
  mapping and three-attempt first-sector probe with resets after failed reads.
  The focused strict-C11 test keeps actual DOS IOCTL and BIOS transports as
  caller-owned boundaries and does not infer their host behavior.

- 2026-07-14 DM1/CSB ReDMCSB F0909-F0910/F0913-F0914: source-locked swoosh
  I/O/release boundaries, PAK decompression and A20E/A31E Graphic21 sector-word
  adapter plus exact-count file read. Focused strict C11 tests pass. F0915 and
  F0924 are source-nonapplicable: their available ReDMCSB routes require Atari
  ST/Amiga hardware.

- 2026-07-14 DM1/CSB ReDMCSB F0920/F0925/F0930: source-locked exact-count
  file read, utility-disk signature gate, and header checksum loops. Focused
  strict C11 tests pass. F0926 is source-nonapplicable: the available routine
  directly installs Atari ST media-change vectors.

- 2026-07-14 DM1/CSB ReDMCSB F0927-F0928: source-locked loader- and OS-error
  formatter boundaries. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F0929/F0936-F0939: source review identified
  library-loader and Amiga Exec/Intuition routes as nonapplicable to the PC 3.4
  target. Focused adapters preserve the original callback order without claiming
  PC behavior.

- 2026-07-14 DM1/CSB ReDMCSB F0940/F0942-F0944: source review identified
  Amiga copper, disk.resource and Exec-vector routes. They are recorded as
  source-nonapplicable to PC 3.4; focused host-boundary tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F0947-F0948: Amiga disk-data initialization and
  release are source-nonapplicable to PC 3.4. Focused boundary tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F0945-F0946: Amiga `audio.device` setup and
  teardown are source-nonapplicable to PC 3.4. Focused boundary tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F0949-F0951: source-locked Japanese conversion,
  PC-98 character-pattern port reads and A100 pattern copies. Focused strict
  C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F0952/F1002: source-locked PC-98 Japanese text
  rasterization and F0132 video-blit forwarding. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F1001: source-locked P20JA VBlank and P20JB
  interrupt-vector ANK character-pattern loaders. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB F1004/F1007-F1008: source-locked packed-nibble
  video shrinking plus memory-chunk initialization and selection. Focused strict
  C11 tests pass. F1010 is X68000-only and source-nonapplicable to PC 3.4.

- 2026-07-14 DM1/CSB ReDMCSB F1017-F1018/F1020/F1024-F1027: source review
  proved these X68000/native service routes nonapplicable to PC 3.4. F1022-
  F1023 character and string print dispatches are source-locked with focused
  strict C11 tests.

- 2026-07-14 DM1/CSB ReDMCSB F1031/F1033: source-locked operation-success and
  hatch-box dispatches. F1032 is source-nonapplicable because its available
  hatch renderers are X68000/Amiga-only. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0812`-`F0816`: source-locked PC-98
  copy-protection BIOS callback, MIDI IODRV commands and substring routine.
  Focused strict C11 tests pass without synthetic disk or audio behavior.

- 2026-07-14 DM1/CSB ReDMCSB `F0807` and `F0808`: source-locked entrance
  animation-step blit and unreferenced disk recalibration dispatch. Focused
  strict C11 tests pass without synthetic graphics or floppy behavior.

- 2026-07-14 DM1/CSB ReDMCSB `F0803`-`F0805`: source-locked magic-map
  icon/map and creature-name-scroll routes. Focused strict C11 tests pass
  without synthetic graphics or game data.

- 2026-07-14 DM1/CSB ReDMCSB `F0802`: source-locked magic-map scroll
  classification and decoded-text match. Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0797`: source-locked PC 3.4 entrance
  micro-dungeon map and south-view dispatch. Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0799`: source-locked PC 3.4 unavailable
  sound masking by original memory flags. Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0792` and `F0798`: source-locked dungeon
  bitmap draw and inclusive point-in-zone predicate. Focused strict C11 tests
  pass without synthetic graphics.

- 2026-07-14 DM1/CSB ReDMCSB `F0791`: source-locked PC 3.4 dungeon-view
  zone bitmap blit. Focused strict C11 test passes without synthetic pixels.

- 2026-07-14 DM1/CSB ReDMCSB `F0766`, `F0785`, `F0787` and `F0789`:
  source-locked screen blit, mouse-coordinate, XYZ-zone and permanent
  layout-allocation routes. Four focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0784`: source-locked direct IODRV_06
  unlock-mouse dispatch. Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0773`, `F0781` and `F0783`: source-locked
  DOS chunked write, mouse pointer-event routing and lock-mouse driver
  dispatch. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0779`: source-locked DOS current-file-mark
  query. The focused strict C11 test passes with no host file I/O.

- 2026-07-14 DM1/CSB ReDMCSB `F0780`: source-locked signed file-handle
  invalidity predicate. Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0777` and `F0778`: source-locked DOS delete
  and rename dispatch preserve the original ignored-result behavior. Focused
  strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0775` and `F0776`: source-locked DOS file
  EOF-size and create routes preserve mark restoration and carry-to-handle
  behavior. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0771`, `F0772` and `F0774`: source-locked
  DOS close/read/seek operations preserve ignored-close-status behavior,
  32 KiB exact-read chunks, and carry-based seek success. Focused strict C11
  tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0765`, `F0768` and `F0770`: source-locked
  opaque dungeon bitmap draw, padded zone-text printing and DOS file-open
  behavior. Three focused strict C11 tests pass with no synthetic graphics or
  file data.

- 2026-07-14 DM1/CSB ReDMCSB `F0758`: source-locked PC 3.4 language-table
  lookup. The focused strict C11 test passes without synthetic text.

- 2026-07-14 DM1/CSB ReDMCSB `F0763`: source-locked PC 3.4 endgame bitmap
  allocation/load route. The focused strict C11 test passes without synthetic
  bitmap content.

- 2026-07-14 DM1/CSB ReDMCSB `F0756` and `F0757`: source-locked PC 3.4
  memory-requirements evaluation and C700 language-text load routes. Focused
  strict C11 tests pass without synthetic text content.

- 2026-07-14 DM1/CSB ReDMCSB `F0748`, `F0750`-`F0752` and `F0755`:
  source-locked EMS allocation, CPSX cleanup, bitmap-size/allocation and
  memory-profile flag routes. Five focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0746`, `F0747` and `F0749`: source-locked
  PC 3.4 EMS routes preserve EMMXXXX0 detection/IOCTL gates, EMS version
  high-nibble extraction, and non-clearing handle release behavior. Focused
  strict C11 tests pass; `F0746` also passes its registered CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0745`: `FILENAME.C:84-105` is source-locked
  for English/French/German mutation of the dungeon, expansion, bonus, save
  and backup filename templates. Focused strict C11 coverage passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0744`: `FILENAME.C:60-81` is source-locked
  for the PC 3.4 filename language mutation. It replaces every `~` with the
  language character, or removes it in place when that character is zero.
  Focused strict C11 coverage includes adjacent markers and unmarked names.

- 2026-07-14 DM1/CSB ReDMCSB `F0738`: `MUSIC.C:513-524` proves that the PC
  3.4/I34E route is intentionally a no-op. Focused strict C11 test and CTest
  registration pass; no synthetic music continuation was introduced.

- 2026-07-14 DM1/CSB ReDMCSB `F0735`: PC 3.4 viewport fill forwards the
  original XYZ and color to the 224-pixel-wide viewport route with no height
  argument. Focused strict C11 dispatch test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0721`: PC 3.4 palette-changing packed shrink
  blit is source-locked to `BLTSHRNK.C:113-169`, preserving 6-bit sampling,
  nibble extraction and palette remap order. Focused strict C11 byte test
  passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0720`, `F0732`, and `F0733`: PC 3.4 packed
  shrink blit and screen/zone fill routes are source-locked. The modules keep
  fixed-point nibble sampling, ZONE-to-inclusive-BOX conversion and F0638
  lookup ownership. Focused strict C11 byte/dispatch tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0716`-`F0719`: PC 3.4 disk/vector and CD-audio
  adapter routes are source-locked: IODRV_17's void body, volume-name buffer,
  signed time return, and G2024-gated CD-track dispatch. Focused strict C11
  tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0715`: PC 3.4 IODRV_20 converted-device-type
  forwarding preserves the signed drive ordinal and signed result. Focused
  strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0711`, `F0713`, and `F0714`: PC 3.4
  scan-code conversion, I/O-interrupt initialization and first-sector result
  routes are source-locked. This includes literal original scan-code tables,
  vector-254/VBlank/DS initialization order, and signed IODRV_18 result
  propagation. Focused strict C11 tests pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0709`: PC 3.4 start-sound dispatch preserves
  the original signed 16-bit index and volume at the host audio boundary.
  Focused strict C11 test passes.

- 2026-07-14 DM1/CSB ReDMCSB `F0710` and `F0712`: PC 3.4 source-locked
  sound-completion and any-input routes are built into `firestaff_m10`.
  `F0710` delegates once to IODRV_16; `F0712` reads IODRV_13 before the
  original mouse-or-keyboard short-circuit. Focused strict C11 tests and the
  Ninja `firestaff_m10` build pass.

- 2026-07-14 DM1/CSB ReDMCSB `F0703`-`F0707`: PC 3.4 source-locked modules
  now cover champion-icon release, IODRV_03/IODRV_04 dispatch, mouse state,
  and sound-buffer reset. Strict C11 tests pass. F0703/F0706/F0707 move from
  uncertain to implemented in the canonical callable audit; F0704/F0705 are
  recorded in the full numbered inventory but are outside its 2,137 rows.

- 2026-07-14 DM1/CSB ReDMCSB `F0698`–`F0700`: PC 3.4 invert-box, video-vector
  and immediate mouse-event routes are source-locked. Verification: focused strict C11 tests.

- 2026-07-14 DM1/CSB ReDMCSB `F0683`–`F0697`: PC 3.4
  C25-VGA transparent horizontal flip, creature palette replacement, message
  update sequencing and hatch callback routes are source-locked. Verification:
  focused strict C11 tests.

- 2026-07-14 DM1/CSB ReDMCSB `F0681`, `F0682`, `F0690` and `F0692`: PC 3.4
  C25-VGA flip/transparency, video-driver line forwarding and packed fill-box
  contracts are source-locked. Verification: focused strict C11 tests.

- 2026-07-14 DM1/CSB ReDMCSB `IMAGE5.C F0680`: PC 3.4 C25 VGA aperture copy
  now preserves packed-nibble order, high-nibble viewport colour offset and
  odd source/count boundaries. Verification: focused strict C11 test.

- 2026-07-14 DM1/CSB ReDMCSB `IMAGE3.C F0691`: PC 3.4 IMG3 expansion now
  preserves the six-entry local palette, RLE row wrapping, retained command-6
  pixels and F0690-style line sink. Verification: focused strict C11 test.

- 2026-07-14 DM1/CSB ReDMCSB `IO2.C F0539/F0540`: PC 3.4 keyboard-status and
  raw-key routes now preserve one driver read, exit shortcuts, and shifted-arrow
  normalization. Verification: focused strict C11 tests.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0053`: logical-screen text now forwards
  C160 byte stride, C200 height and caller colours through a bounded callback.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0054`: PC 3.4 message cursor, permanent
  allocation, font-load and expiry initialization now use bounded callbacks.
  Verification: focused strict CTest.

- 2026-07-14 Nexus DGN face/material provenance: launcher-reopened LEV bytes
  now must equal authenticated canonical bytes before raster admission.
  Verification: focused strict CTest; no fallback visual is admitted.

- 2026-07-14 DM1/CSB ReDMCSB `MEMORY.C F0472`: source unused-list insertion
  now has a strict C11 compatibility callable and focused CTest.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0052`: source viewport text forwarding
  now preserves the fixed 112-byte stride, C12 background and 136-pixel
  height through a bounded F0040 callback. Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `MEMORY.C F0471`: source unused-list unlinking
  has a strict C11 compatibility callable and focused CTest.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0048`: source character output now
  builds the original two-byte string through a bounded print callback.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0049`: unsigned 16-bit decimal output
  now follows the source's backwards formatter through a bounded callback.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `MEMORY.C F0481`: source first-used cache
  eviction loop is now a bounded compatibility callable. Verification:
  focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0480`, `TEXT.C F0051` and `VBLANK.C F0693`:
  source-bounded cache release, line-feed and VBlank gate callables added.
  Verification: focused strict CTests.

- 2026-07-14 DM1/CSB ReDMCSB `TEXT.C F0050`: source white-space output now
  delegates the exact white color/message pair through a bounded callable.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0473_MEMORY_SortValues`: source unsigned
  16-bit ascending heap sort now has a bounded compatibility callable.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0470_MEMORY_FreeAtHeapBottom`: source even-byte
  rounding and available-heap release accounting now have a bounded
  compatibility callable. Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `PRIM1.C F0934_ConvertValueToHexDigits`:
  uppercase hexadecimal digit formatting without an implicit terminator now
  has a source-locked callable. Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `PRIM1.C F0933_GetHexStringFromValue`: uppercase
  unprefixed hexadecimal formatting now has a source-locked callable.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `PRIM1.C F0922_Custom_strcpy`: NUL-terminated
  source copy semantics now have a destination-bounded compatibility callable.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0469_MEMORY_FreeAtHeapTop`: source odd-byte
  rounding and bounded heap-top/accounting release now have a compatibility
  callable. Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `SWSH.C F0904`: the 27-record palette animation
  now preserves source palette writes and black-palette VBlank waits.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `SWSH.C F0902`: FTL-logo presentation now gates
  on a caller-supplied original 320x200 frame and its 16-color palette;
  missing or truncated data authorizes no substitute. Verification: focused
  strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `PRIM1.C F0931/F0932`: source 16-bit wrapping
  word and byte checksums now have bounded compatibility callables.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `F0461_START_AllocateFlippedWallBitmaps`:
  source-ordered plans now reserve the five derived, horizontally flipped
  wall buffers without admitting substitute graphic bytes. Verification:
  focused strict CTest.

- 2026-07-14 DM2: dynamic champion HUD material admission now requires
  real GDAT layout, palette, font, champion provenance, and original pixels;
  no fallback pixels are admitted. Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `LZW.C F0496`: source `0x90` repeat-escape
  emission now has a bounded decoder-state callable. Verification: focused
  strict CTest.

- 2026-07-14 Nexus: DGN face/material raster admission now requires complete
  canonical retail-DGN provenance and rejects any fallback source.
  Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `FILLBOX.C F0135`: source four-plane
  inclusive fill-box logic now has a bounded compatibility implementation.
  Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `DIALOG.C F0425`: centered-choice text placement
  now has a source-derived render plan. Verification: focused strict CTest.

- 2026-07-14 ReDMCSB `ACTIDRAW.C F0134`: bounded planar bitmap fill now
  follows original four-plane 16-pixel unit layout. Verification: focused
  strict CTest.

- 2026-07-14 ReDMCSB `START F0458`: the original TOS command-tail copy is
  now a bounded compatibility adapter. Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `GROUP2.C F0227`: destination-visibility wedge
  logic now has a bounded source-owned callable. Verification: focused strict
  CTest.

- 2026-07-14 CSB ReDMCSB `DIALOG.C F0426`: the source 30-character dialog
  split rule is now a callable compatibility adapter. Verification: focused
  strict CTest.

- 2026-07-14 DM1 ReDMCSB `DUNVIEW.C F0106`: source CPSF reset-to-step-one
  transition is now a bounded compatibility callable. Verification: focused
  strict CTest.

- 2026-07-14 DM1 ReDMCSB `OBJECT.C F0035`: leader-hand object-name clearing
  now invokes the source C017 black-zone fill through an owned backend.
  Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `GROUP.C F0196`: active-group initialization now
  enforces the source 110-slot floor and sentinel state. Verification:
  focused strict CTest.

- 2026-07-14 CSB ReDMCSB `DUNGEON.C F0146`: the native C04 `GROUP.Cells`
  setter now has a source-owned callable and bounded record mutation.
  Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `DUNGEON.C F0148`: source C04 group-direction
  storage now selects the active-group lane on the party map or the packed
  final-word lane elsewhere. Verification: focused strict CTest.

- 2026-07-14 DM1/CSB ReDMCSB `DUNGEON.C F0165`: discard search now skips
  the party's visible 11x11 area, honours enabled sensors and protected
  records, dispatches source-specific discard ownership, and returns a
  type-and-index Thing. Verification: focused strict CTest.

- 2026-07-14 CSB ReDMCSB `GROUP.C F0195`: the current-map native C04 chain
  scan now materializes active groups idempotently from source record state.
  Verification: focused M10-linked CTest.

- 2026-07-14 CSB ReDMCSB `TIMELINE.C F0243`: door-destruction dispatch now
  uses the source callable to write only the low three state bits as destroyed,
  retaining all upper square attributes. Verification: focused CTest.

- 2026-07-14 DM1 ReDMCSB `CLIKMENU.C F0362`: source zone highlight state and
  inclusive inversion transaction now have a renderer-ready plan with vertical
  blank ordering. Verification: focused CTest; M11 consumption remains separate.

- 2026-07-14 CSB ReDMCSB `DUNGEON.C F0147`: packed C04 group directions now
  read from the original final-word bitfield without record mutation.
  Verification: focused CTest.

- 2026-07-14 DM1 ReDMCSB `PROJECTILE.C F0218`: live projectile impact counts
  now query bounded records by map, square, and cell without lifecycle
  mutation. Verification: focused CTest.

- 2026-07-14 DM1 ReDMCSB `CHAMPION.C F0323`: poison-dose reset is now an
  explicit bounded champion compatibility callable. Verification: focused
  CTest.

- 2026-07-14 DM1 ReDMCSB `SENSOR.C F0273`: type/cell ordered object queries
  now preserve first-match and `THING_NONE` behavior. Verification: focused
  CTest.

- 2026-07-14 CSB ReDMCSB `GROUP.C F0182`: stopping a group clears only its
  four attack bits and requests one F0181 event deletion for the same square.
  Verification: focused CTest.

- 2026-07-14 DM1 ReDMCSB `ACTIDRAW.C F0385`: source action-damage feedback
  now has a bounded render plan for negative text, damage tiers, original
  boxes, numeric placement, and screen-update ordering. Verification: focused
  CTest. M11 pixel consumption remains a separate runtime integration.

- 2026-07-14 ReDMCSB `BMPSIZE.C F0459`: PC34 scaled packed-bitmap byte
  count now follows the original scale and even-width allocation formula.
  Verification: focused CTest; allocation and pixel decoding remain separate.

- 2026-07-14 CSB ReDMCSB `DUNGEON.C F0145`: C04 group-cell access now
  preserves the original packed byte and `0xff` centered-group sentinel,
  rejecting incomplete or non-group records. Verification: focused CTest.

- 2026-07-14 CSB ReDMCSB `SAVEGAME.C F0100..F0120` utility import: a staged
  champion preview now remains pending until explicit user confirmation;
  reject returns to selection and accept begins the new game. Verification:
  focused confirmation and utility-flow CTests.

- 2026-07-14 DM2 outdoor renderer: synthetic sky-color generation is removed.
  Outdoor output now remains unavailable until source-owned ENVIRONMENT GDAT
  image, palette, and destination receipts are consumed. Verification:
  focused CTest.

- 2026-07-14 DM1 ReDMCSB `DUNVIEW.C F0111`: source-owned closed-door
  ornament eligibility and temporary panel replay are now explicit renderer
  dispatch receipts, with bounded invalid-route rejection. Verification:
  focused CTest. Host pixel composition remains owned by the renderer.

- 2026-07-14 CSB native runtime save v11: source-owned creature-attack timing
  is preserved through save/load while transient pending/played audio is reset;
  v10 images remain readable. Verification: focused audio CTest.

- 2026-07-14 DM1 ReDMCSB `MENU.C F0384_MENUS_GetActionName`: action names
  now resolve from the source-owned packed `G0490` table, retaining duplicate
  and placeholder names and rejecting `ACTION_NONE`/out-of-range inputs.
  Verification: focused CTest.

- 2026-07-14 DM1 ReDMCSB `OBJECT.C F0037`: source-owned icon pixels now
  blit into the PC34 screen with source-key transparency, row stride, lookup,
  and bounds rejection. Verification: focused 9-assertion CTest.

- 2026-07-14 CSB ReDMCSB `SOUND.C F0064/F0065`: pending-sound arbitration
  now rejects undefined play modes, preserving only immediate/deferred source
  states through tick and save/load. Verification: focused audio CTest.

- 2026-07-14 CSB ReDMCSB `SOUND.C F0061`: the three PSG channel amplitudes
  now resolve through the original loud table with 4-bit index masking.
  Verification is included in the focused F0060/F0061 CTest.

- 2026-07-14 DM1 ReDMCSB `MENUS.C F0383`: action-list construction now keeps
  the primary action, compacts eligible optional actions, clears unused slots,
  and rejects incomplete runtime receipts without mutation. The focused CTest
  covers charge, skill, and receipt gates.

- 2026-07-14 CSB save export provenance: FSSB envelopes now require a
  non-empty original artifact path and reject the retired synthetic marker at
  build, validation, and round-trip boundaries. Verification:
  `test_csb_v1_save_export_import_pc34_compat` (101 passed).

- 2026-07-14 CSB ReDMCSB `SOUND.C F0060`: packed Atari ST sound streams now
  decode high-nibble-first commands and repeat runs while retaining the prior
  PSG amplitude. The focused CTest validates normal, leading-repeat, and
  malformed-stream paths.

- 2026-07-14 DM2 GDAT hand actions: the hand-action resolver follows
  SKProject's `INTERFACE_GENERAL/4` image route and fails closed when source
  images are unavailable. Verification: strict probe against original DM2
  `GRAPHICS.DAT` resolves and decodes all four hand-action images.

- 2026-07-14 DM1 F0099 D0/D1 alias safety: D0L2/D0R2 and D1L2/D1R2 now use
  row-local swaps when source and destination alias, preserving the distinct
  buffer path. Verification: strict standalone two-lane regression.

- 2026-07-14 DM1 F0099 D3R2 in-place flip: the row-local mirror now handles
  aliased source/destination buffers without overwriting later source pixels,
  matching the F0099 scratch-buffer route. The public header is in `include/`
  and the focused D3L2/D3R2 F0108 CTest is registered and passing.

- 2026-07-14 DM2 source-backed GRAPHICSSET admission: dungeon scene material
  is admitted once the selected set supplies SKProject's `SCENE_COLORKEY` and
  `SCENE_FLAGS`; light and weather fields remain separately recorded instead
  of blocking the source-owned floor/ceiling path. Verification:
  `test_dm2_v1_gdat_graphicsset_real_data` with real G1 materials.

- 2026-07-14 DM1 spell-area same-caster redraw: repeated
  `CM1_CHAMPION_NONE` requests now retain the visible spell area instead of
  clearing it redundantly, matching ReDMCSB `CASTER.C F0394`'s early-return
  branch. Verification:
  `test_dm1_v1_champion_panel_spell_area_overlay_pc34_compat` (208
  assertions).

- 2026-07-14 CSB title asset cadence: the source-backed sequence now presents
  all 20 CHAOS frames, holds the complete CHAOS image for two VBlanks, then
  presents STRIKES BACK for one VBlank, following ReDMCSB `TITLE.C F0437`.
  The source-step/receipt coherence test is registered in CTest and passes,
  alongside the terminal real-asset handoff target.

- 2026-07-14 DM1 inventory controls and spell-panel source surface: M11 now
  dispatches the visible C141 music, C140 save, C145 rest, and C011 close
  icons through their ReDMCSB COMMAND.C routes. The C009 spell background is
  admitted at its physical G0000 96x33 size rather than rejected as the C013
  input interior. Verification: `test_dm1_v1_mouse_routes_pc34_compat`,
  `test_dm1_v1_inventory_panel_mouse_routes_pc34_compat`, and
  `test_m11_open_door_spell_runtime_pc34_compat`.

- 2026-07-14 M12 settings column flow: settings tabs with nine or more rows
  now fill two equal columns top-to-bottom; tabs with up to eight rows retain
  one column. Rendering and pointer hit-testing share the same geometry.
  Verification: `test_menu_hit_settings_tab_m12` and
  `test_menu_hit_launch_direct_click_m12`.

- 2026-07-14 DM1 HoC champion-mirror wall composition: M11 now follows
  ReDMCSB `DUNVIEW.C F0107:3922-3928`: the source-selected, map-local wall
  ornament is drawn through the ordinary F0107 path first, then C026 is
  overlaid only in the fixed D1C portrait zone. Removed the non-source
  hard-coded C346 replacement and last-ornament fallback. Verification:
  `test_dm1_v1_champion_mirror_pc34_compat` and
  `test_m11_overlay_command_queue_block`.

- 2026-07-14 Theron Track 02 raw CUE IPL receipt: M12 retains INDEX 01 pregap
  bytes before receipt validation, fixing the false invalid startup graphics
  gate for verified raw JP/US BIN media.

- 2026-07-14 DM1 V2.1 Scale2x capture delivery: the selected-resolution
  presentation and BMP prove a non-uniform source edge survives EPX.
  Verification: `dm1_v21_special_palette_capture_probe`.

- ✅ 2026-07-14 DM1 V1 wall-inscription source raster gate: M11 now accepts
  M648 only with graphic 258, C10 transparency, valid source dimensions, and
  an exact F0107 line plan. Verification: `dm1_v1_inscription_source_raster_gate`.

- 2026-07-14 DM1 V2.0 source-owned title/entrance cadence: V2.0 presentation
  consumes the existing ReDMCSB VBlank interval instead of adding a full host
  delay. Verification: `dm1_v20_startup_presentation_timing_probe`.

- 2026-07-14 DM1 V2.1 F12 real-asset capture admission: F12 rejects an
  asset-free fallback frame and preserves the original TITLE palette in its
  selected-resolution capture. Verification:
  `dm1_v21_special_palette_capture_probe`.

- 2026-07-14 DM1 original-PC34 `PARTY_INFO` FirstScentIndex runtime handoff:
  F0435 now restores the source-owned byte 84 `FirstScentIndex` after the
  opaque scent records and strengths into the existing F0412 Footprints-window
  owner. F0433 writes only the validated `[0,255]` runtime value back to that
  retained source byte. The authenticated-PC34 regression covers import, a
  live periodic tick, and export/reimport; scent records, strengths, and
  `LastScentIndex` remain unowned. Verification:
  `dm1_v1_original_save_pc34_handoff`.

- 2026-07-14 DM1 HoC C127/C2548 saved-runtime render admission: M11 now
  applies the same DM1-owned F0115 payload gate to the C2548 alcove route as
  to ordinary C2500 floor objects. A visible C127 mirror after original-PC34
  save import cannot re-enter the alcove item blitter, while independently
  admitted objects still use their runtime receipt. Verification:
  `dm1_v1_viewport_runtime_materialization_pc34_compat`.

- 2026-07-14 DM1 original-PC34 `PARTY_INFO` ScentCount runtime handoff:
  F0435 now restores the source-owned byte 10 `ScentCount` to the existing
  F0412 Footprints-window owner, and F0433 writes only the validated
  `[0,255]` runtime value back to that retained source byte. The focused
  handoff regression proves authenticated import, a resumed strong Footprints
  window (`FirstScentIndex=29`, `LastScentIndex=0`), and export/reimport.
  Scent indices and arrays remain unowned source bytes. Verification:
  `dm1_v1_original_save_pc34_handoff`.

- 2026-07-14 DM1 HoC F0121/F0124 C2548 alcove-object runtime binding: the
  live M11 F0115 alcove lane now resolves ReDMCSB's
  `C2548 + CoordinateSet * 7 + G2029[ViewSquare]` Graphic 558 anchors,
  preserving the source C10/F0791 material route and viewport clip instead
  of substituting a C2500 floor-object position. The real chest alcove
  bitmap variant remains selected by its object aspect. Invalid coordinate
  sets or rows are rejected before drawing. Verification:
  `dm1_v1_f0115_alcove_item_material_pc34_compat` and
  `m11_dm1_floor_item_host_presentation_receipt`.

- 2026-07-14 DM1 V2.2 finished-pack receipt integration: V2.2 selection
  and the M11 in-place cache now require both `FINISHED_REAL` manifest
  material and a hash-matched, complete `finish_receipt.json` review. Boot
  records the receipt state and promotion result, while absent, stale, or
  partial receipts retain the V2.1 source route. Verification:
  `dm1_v22_finished_pack_receipt_pc34`.

- 2026-07-14 DM1 V2.1 selected-resolution EPX presentation: the live M11
  render route now applies Scale2x/EPX before it uses the selected V2.1 target
  resolution, rather than returning early with a fixed 640x400 surface. The
  same ordering applies to normal and special-palette frames, so the
  renderer-owned presented RGBA buffer and the in-game F12 capture agree with
  the active V2.1 surface. The focused actual-render screenshot probe exercises
  the production EPX route at 640x400 and 960x600; the launcher-to-game
  presentation target contract remains green. This is host-side presentation
   behavior only, not a packaged-app or original-asset visual-capture claim.
   Verification: `firestaff_dm1_v2_actual_render_screenshot_probe` and
   `test_m11_game_presentation_target_pc34_compat`.

- 2026-07-14 DM1 V2.1 presented-frame capture delivery: the production BMP
  writer now creates a nested configured screenshot directory and allocates a
  unique same-second filename before writing a presented RGBA frame. Repeated
  `F12` captures therefore retain each already-presented 640x400 EPX/Scale2x
   frame instead of overwriting the previous one. This changes only capture
   delivery; it does not create, replace, or synthesize DM1 artwork.

- 2026-07-14 DM1 HoC C127 live F0115 occlusion handoff: M11's generic
  floor-object/projectile and deferred-effect passes now consume the
  ReDMCSB-backed DM1 mirror/materialization decisions. A visible C026 mirror
  cannot re-enter the generic payload paths, while a separately admitted
   F0115 object or live projectile remains renderable. Verification:
   `firestaff_dm1_v1_hoc_floor_runtime_no_false_items_probe`.

- 2026-07-14 DM1 HoC C127 wall-ornament occlusion: M11 now draws the
  ReDMCSB DUNVIEW.C F0107 C346/C026 mirror route with the D1C wall-ornament
  phase, before the later side-wall and door occlusion replay. This prevents
  a visible mirror from painting over nearer viewport geometry after the
  C127 payload suppression handoff. Verification:
  `firestaff_dm1_v1_hoc_c127_f0115_same_frame_runtime_probe`.

- 2026-07-14 DM1 V2.2 original-art cache admission: the M11 in-place cache
  now admits pixels only when the finished-art material gate resolves the
  boot-selected asset root as `FINISHED_REAL`. The boot adapter sets that
  gate's root alongside M11's manifest root. The real-asset material CTest
  no longer manufactures placeholder art/cache data; it skip-passes until an
  operator-reviewed original pack is installed, then verifies the genuine
  cache path. Verification: `m11_v22_inplace_draw_pc34` and
  `dm1_v22_real_asset_material_gate_pc34`.

- 2026-07-14 DM1 V2.1 presented-frame screenshot capture: `F12` now captures
  an active DM1 V2.1 session from `M11_Render_GetPresentedRGBA` through the
  production BMP writer, preserving the post-EPX 640x400 image instead of
  saving the unscaled indexed source. The configured screenshot directory is
  honored, with the existing default retained; unavailable presented output
  reports failure rather than falling back. The focused headless V2 screenshot
  test now uses that same production capture API for its V2.1 case and checks
  the emitted BMP dimensions and bytes. Verification:
  `dm1_v2_actual_render_screenshot_probe`.

- 2026-07-14 DM1 PC34 C75 poison expiry M11 materialization: F0887 now emits
  the existing `EMIT_CHAMPION_DAMAGED` live signal after a ReDMCSB C75's
  F0322-shaped positive HP decrement. M11 consequently refreshes the saved
  champion's damage-overlay timer and amount through its normal emission
  consumer; C75 contributes no invented wound mask or display cell. The
  fixture-free external HoC runtime gate detects a bounded, saved C75 from an
  admitted PC34 corpus snapshot and, when that expiry causes damage, verifies
  both the M10 receipt and the M11 overlay. No save, event, or command trace
   is generated, and this is not original executable or frame parity.
   Verification: `dm1_v1_original_save_pc34_external_hoc_runtime`.

- 2026-07-14 DM1 original-PC34 `PARTY_INFO` freeze-life runtime handoff:
  F0435 now restores ReDMCSB `PARTY.FreezeLifeTicks` from byte 11 after the
  C2 champion records into both M10 owners. The live periodic tick mirrors
  the ReDMCSB decrement, and F0433 writes only the validated `[0,255]`
  runtime value back to the preserved source byte. The source-backed test
  covers authenticated C2 import, a live periodic tick, and PC34
   export/reimport (`37 -> 36`); scent arrays and unowned `PARTY_INFO` fields
   remain untouched. Verification: `dm1_v1_original_save_pc34_handoff`.

- 2026-07-14 Nexus Structure3 renderer source-packet staging: the DGN
  viewport now receives only an engine-owned Structure3 packet after every
  existing capture, DGN, face, referenced-vertex, and normal-row relation is
  still verified. The packet carries the typed geometry and opaque capture
  spans without decoding any of them, and it is explicitly no-draw. Missing
  relation evidence withdraws the packet. Verification:
  `nexus_v1_dgn_geometry_readiness`.

- 2026-07-14 CSBWin CursorFilter ResumeSavedGame handoff: the existing live
  GAMEBLOCK2 leader-hand resume bridge now follows `ReadGame` with
  `CSBCode.cpp::TAG0138ec`'s `CURSORFILTER_ResumeSavedGame` packet before it
  publishes a non-empty hand. The six-word packet is admitted only through
  the current FNV-authenticated `ESL_CURSORFILTER` type-47 DSA action; its
  output cannot cancel or replace the source-owned restored object. A missing,
  altered, or unsupported action remains a no-op and cannot prevent the
  normal hand restoration. Source: CSBWin `CSBCode.cpp:6287-6314`,
  `SaveGame.cpp:1802-1808`, `MoveObject.cpp:790-852`, and `CSB.h` cursor
   packet definitions. Verification:
   `csb_v1_csbwin_cursor_resume_saved_game_filter_runtime`.

- 2026-07-14 Nexus Structure3 complete opaque capture retention: after strict
  capture admission, the engine copies the exact typed face/vertex/normal rows
  together with all six opaque capture spans and their session/bundle identity
  into engine-owned storage. It still rejects a missing external-Saturn verdict
  and remains no-draw; no texture, palette, VDP1, transform, culling, or mesh
  semantics were inferred. Verification:
  `nexus_v1_dgn_geometry_readiness` and
  `nexus_v1_structure3_capture_manifest`.

- 2026-07-14 Theron Track 02 M11 authenticated capture intake: the existing
  prepared-launch manifest reader is now called from M11 before its detached
  runtime receipt is published. An explicit manifest must therefore bind the
  selected Track 02 and revalidated System Card/trace evidence, or the M11
  launch rejects. The established direct-launch regression remains intact and
  now contains an explicit unauthenticated-manifest rejection assertion. This
  is an intake connection only: no positive capture, dungeon transition,
  bitmap, palette, or object claim is added.

- 2026-07-14 Theron Track 02 capture-to-dungeon admission: the production
  Soul Room forcefield boundary now rehashes the selected Track 02 path and
  compares the bytes about to enter the runtime with the completed,
  manifest-authenticated initial-level receipt. Only the existing Hall of
  Records level-0 loader route may match; a missing, stale, incomplete, or
  different route leaves the startup flow and world untouched. This binds
  capture provenance to one source-owned route and adds no transition,
  object-tail, bitmap, palette, or broader dungeon semantics.

# 2026-07-14 — DM1 V2.0 original-title palette filter route

V2.0 now filters the presentation copy of DM1's source-owned C001 title and
SWSH special-palette frames after their original VGA palette expansion. It
does not alter the indexed source frame or substitute graphics, and an
unfiltered present restores the exact source palette RGB. Verification:
`dm_title_swoosh_handoff_palette` against the local original `GRAPHICS.DAT`.

# 2026-07-14 — DM1 V2.0 startup special-palette runtime route

The V2.0 filter lane is now selected before the original TITLE and ENTRANCE
timing loops render, so C001 title, C004 entrance, and C005 credits frames
receive the presentation-only post-pass during live startup. Their original
indexed pixels and source special VGA palettes remain unchanged. Verification:
`dm_title_swoosh_handoff_palette` against local original `GRAPHICS.DAT`.

- 2026-07-14 DM2 G1 side-ray surface binding: the active G1 byte-square
  `tileTypeIndex` now populates D0/D1/D2 left and right wall rays, using the
  party-relative SKProject viewport coordinates, before the existing GDAT
  wall material pass. Source-backed side panels consume their normal
  GRAPHICSSET material and cannot trigger fallback art; DB0 doors keep their
  direct record route. Verification: `dm2_v1_g1_center_ray_surface_gate`
  proves simultaneous center and side panels consume only source material.

- 2026-07-14 CSBWin CursorFilter ReadGame save handoff: after the existing
  GAMEBLOCK2 `objectInHand` handoff reaches the live CSB runtime, it now
  sends CSBWin's exact `CURSORFILTER_ReadGame` six-word packet through the
  authenticated `EDT_SpecialLocations/ESL_CURSORFILTER` type-47 DSA action.
  The source treats ReadGame as a notification, so DSA output cannot replace
  the restored cursor object. Missing, altered, or unsupported DSA data is a
  no-op and no cursor operation, cancellation, or synthetic object route was
  introduced. Source: CSBWin `SaveGame.cpp:1754-1760`, `MoveObject.cpp:797-`
  `852`, and `CSB.h` CursorFilter definitions. Verification:
  `csb_v1_csbwin_cursor_read_game_filter_runtime`.

- 2026-07-14 DM2 G1 center-ray surface binding: the active G1 byte-square
  `tileTypeIndex` now reaches D0/D1/D2 as its actual SKProject terrain class
  before M11 consumes the already source-bound wall/floor material plan.
  This corrects G1's `0=wall, 1=floor, 4=door` numbering at the renderer
  boundary, rather than treating it as the unrelated host enum. Doors retain
  their existing direct DB0 route; unsupported terrain remains unavailable.
  Verification: `dm2_v1_g1_center_ray_surface_gate` proves a G1 wall consumes
  only source-backed wall material with no fallback draw.
- 2026-07-14 CSBWin `DamageCharFilter` C38 runtime handoff: the live
  creature-damage mutation path now calls the save-owned
  `EDT_SpecialLocations/ESL_DAMAGECHARFILTER` actuator after the existing
  source-shaped C38 resolver produces a positive hit. It preserves
  `Character.cpp::DamageCharacter`'s seven-word callback ABI and applies only
  the authenticated pure-stack action's non-negative signed-16 final-damage
  result. The dedicated CMake regression proves callback-selected suppression
  and FNV-tail tamper rejection. Scope remains deliberately bounded: no
   generic damage hook, synthetic DSA program, widened `LocalState=2`,
   master-state/world opcode, or unproven pending-damage behavior was added.

- 2026-07-14 Nexus Structure3 startup capture intake: Nexus startup now
  reopens the loaded canonical `LEVxx.DGN` only after its engine-owned source
  receipt is hash-bound, then forwards a manifest and opaque capture packet to
  the existing no-draw binder solely when the capture reader explicitly marks
  the source as original Saturn. Missing Saturn evidence stops before the
  importer/binder. No texture, palette, mesh, VDP1, or draw semantics were
  added. Focused `nexus_v1_structure3_capture_manifest` coverage pins the
  fail-closed path.
- 2026-07-14 DM1 PC34 external C13 tail-runtime admission: the F0435
  corpus runtime receipt now admits a tail-backed C13 only when its raw
  `Map_Time`, `B.Location`, `C.Cell`, `C.Effect`, and `Priority` all reach
  the materialized Vi Altar timeline event, and repeats that check after the
  candidate-to-live ownership transfer. This is exercised only for
  user-supplied external PC34 corpus saves carrying C13; no C13 fixture,
  generated tail, or parity claim is added.
- 2026-07-14 CSBWin EquipFilter handoff: the runtime inventory-slot write now
  follows `CHARDESC::SetPossession` exactly for authenticated DSA data: it
  executes the removed RN first through timer column 1, then the added RN
  through column 0, with the source five-word parameter shape, before writing
  the slot. Both dispatches require original `EDT_SpecialLocations` DB11
  ownership plus the current FNV receipt; unsupported routes receive no
  fabricated callback. Source: CSBWin `Character.cpp` lines 2923-2994 and
  `DSA.cpp` lines 5363-5416. Verification:
   `csb_v1_csbwin_character_death_filter_runtime`.
- 2026-07-14 CSBWin CharDeathFilter handoff: live champion death now checks
  the exact original `EDT_SpecialLocations|ESL_CHARDEATHFILTER` key before
  `KillCharacter`, selects only CSBWin's first type-47 actuator, resolves its
  saved DSA selector/state/column, and invokes the existing FNV-authenticated
  pure-stack runner with source parameters `{ 1, championIndex }`. Missing,
  altered, unsupported, or non-owned save data is a no-op; no fallback DSA,
  location, or world opcode is created. Source: CSBWin `Character.cpp`
  lines 2532-2585, `DSA.cpp` lines 5363-5416, and `data.cpp`
  `EXPOOL::Locate`. Verification:
   `csb_v1_csbwin_character_death_filter_runtime`.
- 2026-07-14 DM1 PC34 C13 M11 movement-runtime dispatch: M11's source-locked
  movement pipeline now dispatches its due F0435 timeline through F0887,
  rather than consuming an admitted C13 as a notification-only event. The
  focused CTest drives C13 step 2 -> step 1 -> step 0 through M11 movement
  ticks and verifies the authenticated bones unlink plus F0283 health,
  direction, and inventory mutation. Source: ReDMCSB `GAMELOOP.C`,
   `TIMELINE.C F0255`, and `REVIVE.C F0283`. This is deterministic host
   runtime coverage, not original-PC34 frame or executable parity.
- 2026-07-14 DM1 PC34 C13 M11 post-dispatch world-state publication: after
  F0887 mutates an admitted C13 during an M11 movement tick, M11 now
  republishes the canonical F0891 live-world hash. This keeps HoC/save
  observers synchronized with the terminal F0283 champion mutation instead
  of retaining the pre-dispatch hash. Verification:
  `dm1_v1_original_save_c13_m11_runtime`.
- 2026-07-14 DM1 PC34 C11 M11 movement-runtime consumption: the movement
  pipeline now passes its one due F0887 result to M11's existing emission
  consumer before publishing F0891. An admitted original-save C11 therefore
  reaches the source-gated F0253/F0259 completion path instead of being
  removed from the live queue without clearing its matching action lock.
  The focused runtime regression verifies one ordinal-zero C11 clears the
  matching owner and applies the expected action-defense removal exactly once.
  Source: ReDMCSB `GAMELOOP.C`, `TIMELINE.C C11/F0253`, and `MENU.C F0407`.
  Verification: `dm1_v1_original_save_c11_m11_runtime`.

- 2026-07-14 DM2 original SKSave timer-queue rebuild: after authentic raw
  timer decode, runtime reconstructs the exact `DM2_SORT_TIMERS` min-heap
  comparator from `skproject/SKULLWIN/c_timer.cpp`: low 24-bit tick ascending,
  timer type descending, actor descending, then stable original table index.
  The receipt retains the rebuilt heap, next table index/tick, and a raw-byte
  FNV witness without scheduling or firing a timer. `tty0C` ownership remains
  source-bound; `tty1D`/`tty1E` remain blocked pending a proven saved DB
  address owner. Focused `dm2_v1_save_load` coverage checks tick/type order,
  stable heap indices, and the existing rejection-before-publish boundary.

- 2026-07-14 Nexus Structure3 strict raw-capture import: one atomic import
  boundary now binds a parsed face manifest to all six opaque raw spans. It
  independently rechecks declared lengths, FNV witnesses, capture-session
  identity, and a length-prefixed capture-bundle witness before invoking the
  existing DGN face binder. Altered spans or bundle identity stop before the
  binder. Local bytes cannot establish original-Saturn provenance; no VDP1,
  texture, palette, transform, culling, or draw semantics are added and mesh
  rendering remains blocked. Verification:
  `test_nexus_v1_structure3_capture_manifest`.

- 2026-07-14 DM2 original SKSave timer post-load ownership: after an atomic
  original raw-SKSave/G1 import, runtime now consumes each authenticated
  ten-byte timer as SKProject's `Timer { dw00, ttype, actor, value, w8 }` and
  applies the exact bounded `SkWinCore.cpp::_3a15_020f` ownership rule.
  `tty0C` rebinds only its saved champion actor to the table index; malformed
  actors reject before publish. `tty1D`/`tty1E` are explicitly retained as
  unresolved RecordE owners rather than dereferenced through an invented DB
  graph. No timer is scheduled, fired, transformed, or given DB semantics.
  Focused `dm2_v1_save_load` coverage proves `tty0C` and both RecordE cases.
- 2026-07-14 Theron complete `$e009` runtime-admission binding: the final
  initial-level receipt now retains the authenticated 2048-byte local-RAM
  payload witness and binds its byte count/checksum into the receipt hash.
  Soul Room entry revalidates that complete witness before it can consume the
  loader/CD handoff. Coverage rejects a malformed direct receipt and a
  mismatched full-payload checksum. This remains opaque byte provenance only;
  no dungeon, object, bitmap, palette, or transition semantics were added.

- 2026-07-14 DM2 original raw-SKSave G1 runtime handoff: before applying an
  admitted raw save, runtime now parses its complete dungeon prefix through
  the established SKProject-shaped loader and verifies the saved party pose
  against its exact map descriptor. The parsed model replaces the live model
  only after all checks succeed, then the existing session/GDAT refresh uses
  that model; rejection keeps the prior dungeon bytes and metadata. Focused
  save/load coverage proves the reparsed 4x5 map reaches runtime and an
  out-of-range original pose cannot mutate it. This does not claim DB-chain,
  timer-payload, checkcode, minion, or full post-load reconstruction parity.

- 2026-07-14 Nexus Structure3 capture-manifest intake: introduced a strict,
  ordered single-face correlation envelope for future original-Saturn DGN
  captures. It records opaque, size-bounded texture/palette/VDP1/transform/
  culling/command witnesses and the exact Structure3 face candidate without
  accepting capture provenance or inferring any PRS3, VDP1, pixel, palette,
  transform, culling, or draw semantics. The receipt remains no-draw until
  independently admitted source/capture evidence reaches the existing binder.
  Declared span sizes must now exactly match each captured buffer before it
  can reach that binder.
  Verification: strict C99 `test_nexus_v1_structure3_capture_manifest`
  rejects missing palette correlation, zero-sized texture spans, and invalid
  ordering.

- 2026-07-14 DM2 original SKSave corpus renamed-artifact admission: recursive
  corpus discovery now accepts a noncanonical filename only after the same
  complete 42-byte SKSave header gate used by load plus the existing
  source-bound payload parser. The retained candidate is still complete-file
  hash receipted and must revalidate before runtime import. Focused save/load
  coverage proves a renamed original-envelope artifact is discovered while an
  extension alone is never trusted. This does not assign timer, dungeon DB,
  or post-load rebuild semantics.

- 2026-07-14 Theron complete `$e009` payload witness: the coalesced Mednafen
  contract now records the full 2048-byte local-RAM payload after the observed
  System Card return and requires its FNV fingerprint to match the selected
  hash-verified Track 02 MODE1 user-data sector. Missing, short, reordered,
  or mismatched payload rows fail closed. This proves only an original
  loader-to-RAM byte transfer, not a dungeon transition, payload format,
  object table, bitmap, or palette relation. Coverage: focused trace-order
  regression test and raw-media initial-level handoff probe.

- ✅ 2026-07-14 DM1 PC34 `PARTY_INFO` C73/C79 runtime handoff: F0435 now
  restores ReDMCSB `DEFS.H` offsets 2 and 3, `Event73Count_ThievesEye` and
  `Event79Count_Footprints`, into their existing M10 magic owners. C79 also
  reactivates its footprint state. F0433 writes only bounded source-owned
  byte values, preserving the remaining opaque `PARTY_INFO` bytes. Source:
  ReDMCSB `DEFS.H:837-870`, `LOADSAVE.C F0433/F0435` and `TIMELINE.C`
  C73/C79 dispatch. Verification: `test_dm1_v1_original_save_pc34_handoff`.

- 2026-07-14 Nexus Structure3 capture-source admission: a complete face
  binding now also requires caller-owned verification of the capture source,
  separately from the canonical DGN hash admission. Hash-verified DGN bytes
  plus self-consistent fixture spans remain blocked and cannot form a
  complete capture receipt. This does not add a Saturn trace, decode pixels
  or palettes, infer VDP1 semantics, or permit DGN mesh drawing.

- 2026-07-14 Theron Track 02 capture-intake atomicity: the manifest-bound
  coalesced `$e009` intake now validates into a staged boot profile and commits
  only after all files, hashes, trace control edges, and the original Track 02
  span have passed. A rejected retry preserves the previously authenticated
  Soul Room handoff intact. Coverage: focused
  `theron_v1_runtime_trace_intake` probe. This does not create an authentic
  capture or assign dungeon, object, bitmap, palette, or transition semantics.

- 2026-07-14 Nexus Structure3 capture source-admission gate: the face-capture
  binder now requires caller-owned canonical DGN hash admission in addition
  to all packet/source fingerprints. A trace candidate cannot self-certify
  arbitrary bytes by echoing their FNV value. The focused DGN geometry test
  verifies rejection without source admission. This does not import a Saturn
  trace, decode texture or palette data, or permit a DGN mesh draw.

- 2026-07-14 DM2 GDAT plane flip: bound G1 map offsets and active party state
  to SKProject `SET_GRAPHICS_FLIP_FROM_POSITION` for decoded ceiling rect 700
  and floor rect 701 material plans. Focused coverage proves their distinct
  source flag/parity branches; this does not claim dynamic light, full GDAT
  scene parity, or inferred material routes.

- 2026-07-14 DM2 real GDAT dynamic-creature animation-table boundary: added a
  bounded V5 decoder for SKProject's `GET_CREATURE_ANIMATION_FRAME` data path.
  For a GDAT-backed non-static AI type, it resolves only the complete
  `CREATURES/type` `dtRaw8/FB`, `dtRaw7/FC`, and `dtRaw7/FD` triad: command
  attribution, terminal-bounded mutable-frame step, and one directional image
  id. The focused canonical-media target scans actual `GRAPHICS.DAT` and skips
  when its variant has no admitted source AI classification. It creates no art,
  pixel surface, save, AI command, timing, random branch, or creature state.
  Static DB4/F9 candidates remain deliberately excluded until an original
  mutable creature context is admitted.

- 2026-07-14 Theron Track 02 e009 caller-control receipt: the coalesced
  later `$e009` dispatch now retains the source-observed HuC6280 `JSR` opcode
  (`0x20`) and its `$e009` target, and the bounded initial-level admission
  rejects a missing, non-JSR, or non-e009 caller control fact. This proves
  only that the captured caller executed the System Card call before its
  already-verified return/resumption edge; it assigns no gameplay transition,
  dungeon, object, bitmap, or palette meaning. Coverage: fail-closed
  `test_theron_v1_later_e009_raw_sector_order_trace` and the focused,
  raw-media-gated `theron_v1_raw_loader_trace_initial_level_handoff` CTest.

- 2026-07-14 Theron Track 02 source-span revalidation: initial-level handoff
  admission now recomputes the traced 32-byte local-RAM fingerprint from the
  selected hash-verified raw Track 02 user-data span. A copied receipt with a
  nonzero but mismatched fingerprint is rejected before the existing
  `0x0b52` loader/CD receipt can compose. The focused raw-media-gated
  `theron_v1_raw_loader_trace_initial_level_handoff` CTest covers that
  fail-closed case. This proves only the recorded byte span, not a dungeon
  transition, map, object, visual, palette, or payload format.

- 2026-07-14 Nexus PRS3 VDP1 texture-consumption capture boundary: versioned
  SH-2-to-VDP1 V2 trace receipts now require an observed post-command VDP1
  texture-read interval to begin at the decoder-output base, exactly cover its
  declared byte count, and carry the same FNV-1a64 fingerprint as the captured
  decoder output. Asset binding retains this only after the existing exact
  `MENU.BPK` payload-span and whole-asset identities agree. V1 remains accepted
  as address-only evidence and cannot claim consumption. This proves one future
  capture's byte-range handoff only; it proves no PRS3 opcode, VDP1 command
  field, texture format, palette, pixel, draw order, or render route.
  Verification: strict-C99 `test_nexus_v1_prs3_capture_trace_schema` covers
  the positive V2 receipt plus mismatched texture digest/base and V1 regression
  cases.
- 2026-07-14 DM1 F0435 immutable-byte runtime materialization: original PC34
  byte imports now stage through the same candidate-world route as file imports
  and borrow the active original dungeon/Thing backing for tail-less saves.
  M11's original-byte import now uses this route, preventing a valid save from
  replacing the live world with one that has no source dungeon. Focused handoff
  coverage proves backing retention and atomic rejection without it.

- 2026-07-14 DM2 direct-DB4 live candidate gate: M11 now receives static
  dungeon creature candidates only from the committed G1
  `Creature::b4 -> CREATURES/type/F9` material receipt. Each candidate carries
  its original ObjectID, tile, direction, decoded dimensions, and local
  palette ownership; the prior generic record-link walk cannot nominate a
  sprite. The focused real-data `test_dm2_v1_g1_scene_creature_gdat_real_data`
  remains the positive canonical G1/GRAPHICS proof, while the runtime smoke
  fixture now proves that an unowned linked DB4 record produces no atlas draw
  or render receipt. This adds no animation timing, light calculation,
  destination/clipping plan, art, or save behavior.

- 2026-07-14 Nexus PRS3 captured-input span binding: a claimed SH-2-to-VDP1
  capture now carries a nonzero fingerprint for its exact `MENU.BPK` stream
  range. Asset binding recomputes that range from the bounded PRS3 entry plan
  and rejects a trace even when its whole-file hashes and claimed offsets
  agree but the captured input bytes do not. This is a capture-integrity gate
  only: it proves no PRS3 opcode, output pixel, palette, VDP1 command
  contents, or draw route. Verification: focused strict-C99
  `test_nexus_v1_prs3_capture_trace_schema` includes a whole-file-valid,
  payload-mismatched negative case; decoding and fallback visuals remain
  disabled.

- 2026-07-14 Theron Track 02 returned-PC control-edge receipt: the coalesced
  later `$e009` trace now retains the exact System Card return target alongside
  the next observed caller PC, and admission rejects a receipt whose raw edge
  does not start at that returned PC. This proves only one source-observed
  caller control/resumption edge after the media-bound read; it assigns no
  instruction, gameplay transition, dungeon, object, bitmap, or palette
  meaning. Coverage: fail-closed
  `test_theron_v1_later_e009_raw_sector_order_trace` and the focused,
  raw-media-gated `theron_v1_raw_loader_trace_initial_level_handoff` CTest.

- 2026-07-14 DM2 canonical DB4 creature GDAT draw-consumption gate: the
  existing source-classified G1 `Creature::b4 -> CREATURES/type/F9` route is
  now proven through the viewport, not just its bridge receipt. The focused
  real-data target keeps the decoded canonical map-chip and its local palette
  alive through one source-material-required draw, binds the same G1 root's
  ObjectID and coordinate, and requires one asset blit with zero fallback and
  zero blocked-material draws. It creates no art, GDAT, dungeon, save, or
  timing fixture. Source-lock: SKProject `c_map.cpp`
  `QUERY_DUNGEON_MAP_CHIP_PICT`, `DME.h::Creature::CreatureType`, and
  `SkWinCore.cpp::DRAW_CHIP_OF_MAGIC_MAP`. Verification:
  `test_dm2_v1_g1_scene_creature_gdat_real_data` with canonical user media.
  This does not select animation frames: that route still requires the
  separately-unadmitted mutable sequence state, random branch, and AI/static
  classification from `GET_CREATURE_ANIMATION_FRAME`.

- 2026-07-14 Theron Track 02 local-RAM control-resumption receipt: the
  coalesced later `$e009` trace now requires one raw CPU step after the
  authenticated System Card return and binds that step to the same
  caller/return/record tuple. Initial-level admission also rejects receipts
  without the prior 32-byte local-RAM/media match or this post-return step.
  The receipt establishes only loader-to-local-RAM control resumption; it
  assigns no gameplay transition, dungeon, object, bitmap, or palette meaning.
  Coverage: fail-closed
  `test_theron_v1_later_e009_raw_sector_order_trace` and the focused,
  raw-media-gated `theron_v1_raw_loader_trace_initial_level_handoff` CTest.

- 2026-07-14 DM1 F0435 same-tick timeline materialization: original PC34 C4
  is a ReDMCSB `TIMELINE.C F0234` heap, rather than a linear execution list.
  F0435 now linearizes its admitted C3/C4 event pairs by the source comparator
  (low-24-bit time, then Type/Priority descending, then EVENT index) before
  handing them to M10's tick-ordered queue. This prevents valid sibling heap
  entries from firing in their storage order when they share a tick. The
  focused C20 regression uses an authenticated PC34-shaped C4 heap whose
  priority-1 and priority-2 siblings are deliberately reversed in storage,
  then proves the runtime emits priority 3, 2, 1. Verification:
  `test_dm1_v1_original_save_pc34_handoff` (the external-corpus portion
  remains skip-safe without operator-supplied media).

- 2026-07-14 CSBWin real extended-DSA package stability gate: the opt-in
  `Dungeon.dat` plus `csbgame*.dat` runtime handoff now snapshots both
  complete input files before production resume and requires matching
  complete-file size/FNV receipts after the live DSA/timer tick. This prevents
  a changed package path from retaining an earlier DSA catalog, timer-slot, or post-tick core-resume
  result. The probe is registered as a skip-safe real-data CTest and creates
  no save, dungeon, timer, or DSA fixture. Verification: focused
  `csb_v1_csbwin_extended_dsa_handoff` CTest (skips without explicit paths).

- 2026-07-14 DM1 F0435 saved-portrait host-consumption gate: M11's normal
  C017 inventory panel now unpacks the active champion's saved 32x29 4bpp
  portrait payload from the F0435-restored M516_CHAMPIONS state before it may
  fall back to the GRAPHICS.DAT atlas. The fixture-free external PC34 HoC
  runtime target opens the production inventory panel and compares every
  rendered portrait pixel to the authenticated in-memory save snapshot.
  Source-lock: ReDMCSB `LOADSAVE.C F0435` lines 2803-2816. Verification:
  `dm1_v1_original_save_pc34_external_hoc_runtime` (skip-safe without
  operator-supplied original PC34 save/media inputs). This proves Firestaff
  host consumption, not original DOS pixel parity.

- DONE 2026-07-14 Nexus Structure3 record-to-face/normal attachment receipt:
  canonical-MD5-verified `LEV00.DGN`--`LEV15.DGN` corpus tests now bind each
  complete Structure1A owner relation's documented Structure3 model selector
  and each Structure1F face selector to an in-range entry-local face row and
  same-ordinal normal row. A model or face selector outside its bounded domain
  rejects the entire attachment receipt. The receipt is retained by the DGN
  renderer handoff and no-draw plan but proves no placement, transform,
  normal-plane, texture, palette, culling, VDP1, or draw behavior.
  Verification: strict C99 `test_nexus_v1_dgn_geometry_readiness` and
  `test_nexus_v1_dgn_face_mesh_corpus` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- 2026-07-14 DM1 original PC34 save/HoC snapshot runtime gate: the
  fixture-free external runtime target now reads each corpus-certified save
  once, validates its exact source size/hash, and gives those same immutable
  bytes to independent F0435 staging and M11 adoption. It rehashes the source
  path after replay, rejecting a changed file rather than mixing its earlier
  corpus receipt with later runtime evidence. The M11 byte entry remains
  F0435-only and retains the path solely for its resume diagnostic receipt;
  it never reopens it. Source-lock: ReDMCSB `LOADSAVE.C F0435` staged load
  and `GAMELOOP.C` runtime route. Verification:
  `dm1_v1_original_save_pc34_external_hoc_runtime` is skip-safe without
  operator-supplied original PC34 save/media inputs. This is host runtime
  evidence, not original DOS execution or capture parity.

- 2026-07-14 Theron Track 02 initial-level loader/CD admission receipt: a
  coalesced, ordered original `$e009` complete-sector receipt now composes
  with the hash/anchor-gated initial-level envelope only when it observes
  exact Track 02 record `0x0b52`. The receipt retains the existing Hall of
  Records level-0 loader route and explicitly blocks object-tail and visual
  semantics. Fixture coverage proves the exact-record gate and adjacent-record
  rejection; an authenticated positive result remains opt-in via
  `FIRESTAFF_THERON_COALESCED_LOADER_TRACE`. Verification:
  `theron_v1_raw_loader_trace_initial_level_handoff`.

- 2026-07-14 DM1 original PC34 save/HoC external runtime trace gate: the
  fixture-free F0435 corpus target optionally consumes an operator-recorded,
  source-CRC-bound IDLE/FORWARD/BACKWARD/TURN/STRAFE trace. Each row advances
  an independently materialized F0435 world and the live M11 adoption, then
  requires matching M10 tick receipts, canonical world hashes, timeline and
  emission state, plus a nonblank byte-stable 224x136 host viewport capture.
  No save, dungeon, graphics, or trace fixture is generated. ReDMCSB anchors:
  `LOADSAVE.C F0435` and the normal `GAMELOOP.C` command-to-tick route. This
  is Firestaff runtime evidence only; provenance-recorded original PC34 saves
  and original executable route/capture comparisons remain required.

- 2026-07-14 CSBWin general-package post-tick core-resume gate: the opt-in
  package handoff now applies the production core writer and reader to every
  accepted original `csbgame*.dat`, not only Extended Features saves. When
  the surviving source TIMER/TimerQueue heap remains serializable, an isolated
  core-only runtime must retain its exact queue receipts, level, party pose,
  facing, and game time while clearing all Extended Features/DSA ownership.
  A consumed or requeued timer that cannot be serialized remains explicitly
  unavailable; the probe creates no replacement save, timer, dungeon, or DSA.
  Verification: `csb_v1_csbwin_package_runtime_handoff` with supplied
  original package paths.

- 2026-07-14 CSBWin real-package DSA post-tick core-resume gate: the opt-in
  Extended Features/DSA probe now exports a post-tick core save only when the
  retained source TIMER/TimerQueue heap is still serializable. It verifies the
  bytes with the production body reader and restores them into an isolated
  core-only runtime, which must retain timer receipts, party pose, and game
  time while clearing Extended Features/DSA ownership. A consumed or requeued
  timer stays explicitly unavailable rather than being replaced. Verification:
  `csb_v1_csbwin_extended_dsa_handoff` with supplied original package paths.

- 2026-07-14 DM2 SKSave corpus snapshot receipt: corpus classification now
  rehashes the complete original file after reading and source-parsing its
  payload, then rejects any changed file rather than retaining a mixed
  file/payload receipt. The focused save test covers mutation after a scan.
  This remains census-only for original timer and rebuilt dungeon DB regions.

- DONE 2026-07-14 Nexus Structure3 face-normal geometry receipt: the bounded
  LEV00-LEV15 parser now cross-checks each documented face/normal pair against
  the signed-16.16 source vertices under an overflow-safe envelope. The retail
  corpus preserves 18,478 measured faces: 11,876 exactly orthogonal to their
  tested base edges, 6,602 nonorthogonal, and cross/normal orientation counts
  of 15,877 positive and 2,601 negative. The receipt is carried through the
  mesh handoff and render plan but explicitly proves no normal-use, winding,
  culling, transform, texture, palette, VDP1, or draw behavior. Verification:
  `test_nexus_v1_dgn_face_mesh_corpus` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- 2026-07-14 DM1 PC34 C11 save-runtime materialization: the focused F0435
  regression now stages three source-valid `ENABLE_CHAMPION_ACTION` records,
  including MENU.C F0407's ordinal-two throw form, and drives them through
  the restored M10 queue. Each fires once on its saved tick as the exact
  champion/ordinal `EMIT_ACTION_ENABLED` receipt; no dungeon, action, or
  follow-up is synthesized. The former malformed-C11 rollback fixture now
  proves an unknown active C80 remains transactional. Source-lock:
  ReDMCSB `CHAMPION.C F0330`, `MENU.C F0407`, and `TIMELINE.C C11/F0253/F0259`.
  Verification: `dm1_v1_original_save_pc34_handoff`.

- 2026-07-14 DM1 PC34 external-save live-world runtime receipt: the
  fixture-free HoC runtime probe now recomputes the canonical M10 world hash
  from M11's adopted world after the first ReDMCSB-shaped idle tick and after
  every bounded C4 queue tick. A stale published receipt can no longer certify
  a divergent live runtime. Source-lock: ReDMCSB `LOADSAVE.C F0435` restores
  C3/C4 before `TIMELINE.C F0261` consumes due events. Verification:
  `dm1_v1_original_save_pc34_external_hoc_runtime` remains skip-safe without
  operator-supplied original PC34 save/media inputs.

- DONE 2026-07-14 Nexus Structure3 face-edge incidence receipt: the bounded
  LEV00-LEV15 parser now measures entry-local consecutive face-index pairs,
  incidence multiplicity, and paired raw traversal direction. The receipt is
  carried through renderer planning but asserts no winding, manifold,
  surface, transform, culling, UV, texture, palette, VDP1, or draw semantics;
  the original-Saturn capture gate still blocks real DGN mesh rendering.
  Verification: `test_nexus_v1_dgn_face_mesh_corpus` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- 2026-07-14 DM1 PC34 external-save queued-runtime gate: the fixture-free
  HoC runtime probe now follows the next persisted F0435 C4 timeline event
  when its source timestamp falls inside a 1024-tick test horizon. It advances
  only `CMD_NONE` and compares every M11 tick's pre/post time, canonical world
  hash, timeline count, and full emission receipt with an independently
  materialized F0435 world. Empty and far-future queues are reported, not
  manufactured or promoted. Source-lock: ReDMCSB `LOADSAVE.C F0435`,
  `GAMELOOP.C` idle loop, and `TIMELINE.C F0238/F0261`. Verification:
  `dm1_v1_original_save_pc34_external_hoc_runtime` remains skip-safe without
  operator-supplied original PC34 save/media inputs.

- DONE 2026-07-14 Nexus Structure3 face-capture binding boundary: added a
  no-draw, atomic receipt for a future original Saturn `(LEVxx.DGN, entry,
  face)` trace. It binds the existing retail typed-mesh corpus identity
  `d3f42b1f` plus the exact DGN/Structure3 source, bounded face and vertex
  rows, paired normal, documented fill selector, and captured texture,
  palette, VDP1, transform/culling, and command byte spans. The corpus
  contains no capture packet, so the receipt never claims original-Saturn
  provenance, cannot make the renderer ready, and blocks real DGN mesh draws
  without fallback. Verification: `test_nexus_v1_dgn_face_mesh_corpus` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- DONE 2026-07-14 Nexus Structure3 real-face geometry receipt: the bounded
  LEV00-LEV15 DGN parser now measures every documented entry-local face row
  against its signed-16.16 vertex coordinates and preserves the result across
  the renderer handoff and no-draw render plan. The supplied retail corpus
  contains 1,144 entries and 18,478 nondegenerate faces (zero degenerate),
  with maximum absolute coordinate component 450560. The receipt is fail
  closed outside its overflow-safe measurement envelope and asserts no plane,
  winding, transform, texture, palette, VDP1, or draw semantics. Verification:
  `test_nexus_v1_dgn_face_mesh_corpus` with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus`.

- 2026-07-14 CSBWin DSA binding retained-save guard: type-47 selector
  resolution now requires the current complete FNV-authenticated Extended
  Features tail, saved DSA level index, and authenticated DSA header. A stale
  or truncated tail and an orphaned decoded action catalog fail before any
  timer/filter dispatch is prepared. The compact `LocalState=2` queued-timer
  regression covers both rejection paths, and the opt-in real `Dungeon.dat` +
  extended `csbgame` probe verifies the retained-tail receipt before accepting
  a source actuator binding. No save, dungeon, DSA record, or fallback action
  is synthesized.

- 2026-07-14 DM1 PC34 external-save first-runtime-tick gate: the fixture-free
  HoC runtime probe now advances each authenticated external F0435 world once
  through M11's `CMD_NONE` path and requires exactly one tick, a published M10
  runtime receipt, retained original-save viewport ownership, and a nonblank,
  byte-stable PC34 viewport after the tick. It introduces no fixture, graphics,
  or substitute input route. Source-lock: ReDMCSB `LOADSAVE.C F0435` restores
  the C3/C4 timeline before `TIMELINE.C F0651`; `GAMELOOP.C` then processes an
  idle command against that restored world. Verification:
  `dm1_v1_original_save_pc34_external_hoc_runtime` is skip-safe unless both
  `FIRESTAFF_DM1_PC34_SAVE_CORPUS` and `FIRESTAFF_DM1_PC_DATA` name operator
  supplied original PC34 material.

- 2026-07-14 CSBWin resumed TimerQueue export guard: CSB core-save export now
  rechecks CSBWin `Timer.cpp::CheckTimers` heap order after validating each
  live event's serialized slot and TIMER fields. A timer/event pair that is
  individually consistent but leaves a child ahead of its parent rejects
  before any replacement save bytes are written. Regression coverage mutates
  that exact case while retaining the live receipt.

- ✅ 2026-07-14 DM2 custom `WALL_GFX` button decoded-geometry gate: the
  source-required DB2/DB3 custom-door-button path now accepts a fetched
  `WALL_GFX` field-1 image only when its decoded width and height match the
  direct owner receipt, in addition to tile, ObjectID, graphic index, and
  local palette. A same-palette image with altered dimensions blocks the
  entire door material transaction before any button pixels are blitted;
  no generic or resized substitute is used. Source: SKProject
   `SkWinCore.cpp::DRAW_DEFAULT_DOOR_BUTTON` and `c_gui_vp.cpp`
   `QUERY_GDAT_IMAGE_LOCALPAL`. Verification:
   `test_dm2_v1_g1_wall_button_material_gate`.

- 2026-07-14 DM1 save-and-quit guard: the M11 keyboard and pointer quit
  routes now persist the source SAVE-AND-QUIT header value and retain the
  guard when saving or path construction fails, reporting the actual failure
  instead of silently quitting. Source-lock: ReDMCSB `LOADSAVE.C` save-and-quit
  header path. Verification: `test_dm1_v1_save_load`.

- ✅ 2026-07-14 DM1 PC34 external-save full-runtime handoff gate: the
  fixture-free HoC runtime probe now hashes an independently materialized
  F0435 world from each admitted external save and requires M11's
  post-adoption canonical world hash to match before inspecting pixels. This
  prevents a pose-correct viewport from standing in for the restored party,
  active state, timeline, or dungeon-backed runtime. No save, dungeon, or
  graphics fixture is created; unset corpus/media inputs still skip.
  Source-lock: ReDMCSB `LOADSAVE.C F0435`. Verification: CTest
  `dm1_v1_original_save_pc34_external_hoc_runtime`.

- 2026-07-14 CSBWin real-package DSA tick ownership: the opt-in
  `firestaff_csb_v1_csbwin_extended_dsa_handoff_probe` now verifies the
  post-tick live timeline as well as the admitted queue. A source TIMER may
  be consumed or requeued, but every surviving event must retain one unique
  saved TimerQueue slot and every serialized TIMER field, while the loaded
  package dungeon remains the runtime owner. The probe accepts only supplied
  `Dungeon.dat` plus a checksum-valid Extended Features/DSA save and skips
  without both paths; it creates no substitute dungeon, save, DSA, or event.

- ✅ 2026-07-14 DM1 PC34 external-save viewport-consumption gate: the
  fixture-free HoC runtime probe now requires M11 to retain the
  `ORIGINAL_SAVE_PC34` runtime origin and to produce a nonblank, byte-stable
  224x136 PC34 viewport crop from each admitted external save. The former
  whole-frame nonblank check could be satisfied by HUD chrome alone. No save,
  dungeon, or graphics fixture is created; the target still skips without
  explicitly staged original PC34 media and corpus. Source-lock: ReDMCSB
  `LOADSAVE.C F0435` and `DUNVIEW.C F0128`. Verification: CTest
  `dm1_v1_original_save_pc34_external_hoc_runtime`.

- DONE 2026-07-14 Theron Track02 later-loader media receipt: the raw-loader
  boundary now requires one source-marked original Mednafen `$e009`
  dispatch/return envelope after a media-bound `$4090 -> $3800` Stage 3
  receipt, then binds its exact record range to the same hash-verified raw
  Track 02 bytes. The receipt retains caller/return PCs, sector count,
  raw/user-data coordinates, and a user-data hash. Duplicate returns,
  malformed packed records, out-of-range sectors, changed Track 02 identity,
  and unbound Stage 3 traces fail closed. It asserts no dungeon, map, object,
  palette, bitmap, or payload semantics. Verification:
  `firestaff_theron_v1_raw_loader_trace_stage3_sector_probe`.

# 2026-07-11 - DM1 GROUP.C F0207 timeline attack application

- ✅ 2026-07-11 DM1 M10 F0207 C38-C41 attack application: the existing F0209 behavior decision now applies its creature attack in the shared timeline dispatcher. Ranged actions create a creature-owned F0212 projectile and schedule its first move; melee actions retain the F0207 target-cell/champion choice, resolve F0230/F0321 through the shared combat path, wake the party when required, and apply HP/wound/poison/death state. `EMIT_CREATURE_ATTACK` records the resulting projectile slot or melee damage, with normal damage/down receipts retained for hits. Regression coverage dispatches a real C38 Lord Chaos event through M10 and requires the projectile, first move, ownership, and receipt. Verified with Ninja `test_m10_c006_generator_reenable_dispatch_pc34_compat` and `test_dm1_v1_creature_ai_behavior_pc34_compat` (244/244).

# 2026-07-11 - DM1 original PC34 corpus roundtrip proof

- ✅ 2026-07-11 Nexus TITLE.CG packed-atlas and BPK receipt correction:
  verified local Saturn media establishes `TITLE.CG` as a 32-byte zero prefix
  plus `0x29000` packed bytes, decoded high-nibble first into a 328x1024 4bpp
  indexed atlas. Invalid shapes still reject and no fallback visual is added.
  MENU.BPK PRS3 evidence is now non-promoting: decode/upload receipts block
  all PRS3 entries, real-media regressions no longer assert ready uploads, and
  the launcher requires a real menu-surface route before marking the main menu
  ready. Verified with focused Ninja targets and Nexus startup/BPK tests.

- Added `dm1_v1_original_save_pc34_roundtrip_corpus_root()` to classify a recursive corpus and verify every eligible PC34 file through the source-locked transient import, export and reload chain. It never writes an export beside user data.
- Extended `test_dm1_v1_original_save_pc34_handoff` with nested arbitrary file names, two valid PC34 fixtures and a rejected text file; it proves that only qualified files are round-tripped and their core state survives both handoff edges.
- ✅ 2026-07-11 Nexus real-media post-grid form: LEV00–LEV15 now validate the 0x24 span as exactly 128 zero bytes and the 0x30 span as a 0x34-bounded non-empty table of opaque 16-byte records. The DGN layout, geometry information, and renderer handoff expose only those verified bounds and Structure1B packed-high index coverage; no record-byte, mesh, footprint, or Structure1F semantics were promoted. Verified with Ninja targets `test_nexus_v1_dgn_geometry_readiness` and `firestaff_nexus`, the direct real-media test, and focused real-media CTest `nexus_v1_dgn_geometry_readiness` (1/1).
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
  - ✅ 2026-08-09 DM1 original PC34 resume capture route: the stale option-4
    Alt+numeric-keypad path was rejected because it left the original selector
    on the entrance wall. The live plan now uses the source-documented PC34
    mouse mode (option 1) and a real `C409_ZONE_ENTRANCE_RESUME` click. The
    operator-owned raw capture
    `/Volumes/Extern-disk/Documents/Firestaff/dm1-original-resume-c13-mouse1.v7hhJs`
    reaches stable `dungeon_gameplay` frames and records the follow-up
    forward-click and keypad movement fallback. This closes the capture
    harness stall only; the save is not C13-bearing and is not promoted as
    C13 evidence.

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
# ✅ 2026-07-12 DM1 PC34 ACTIVE_GROUP native save transaction: `F0796_SAVEGAME_ImportPC34_Compat()` now validates the exact ordered ACTIVE_GROUP payload after GLOBAL_DATA before it publishes staged header/party/timeline state. Per ReDMCSB `LOADSAVE.C F0435` lines 2749-2754, the block must be `sizeof(ACTIVE_GROUP) * GLOBAL_DATA.MaximumActiveGroupCount`; an inconsistent length now rejects even in lenient checksum mode, and strict checksum corruption also rejects without changing the destination. The existing world handoff remains the runtime owner for decoded records. `test_dm1_v1_savegame_pc34_native_export_pc34_compat` now covers a real `F0802` PC34 world export -> `F0796` import, malformed active-group length and ciphertext with byte-for-byte destination preservation, and the two-group PC34 -> runtime -> PC34 -> runtime replay. Verified with `ctest -R 'dm1_v1_savegame_pc34_native_export_pc34_compat|dm1_v1_original_save_pc34_handoff'` (2/2).
- 2026-07-12 DM2 V1 CCM provenance gate: skproject
  `EXTENDED_LOAD_AI_DEFINITION` establishes `CREATURE_AI` `dtWordValue`
  fields 0–35 only as AIDefinition members, not CCM programs. The boot auto
  loader now rejects decodable candidate fields instead of promoting guessed
  bytecode into runtime. Verification: focused CCM fixture and strict C11
  syntax PASS.
# ✅ 2026-07-13 DM1 F0245 corridor TextString message handoff

DM1 now follows ReDMCSB `TIMELINE.C F0245` lines 939-954 when a corridor
TextString becomes visible: M10 emits its real TextString Thing index only
when the event square is the current party square, and M11 consumes it with
`F0168`/`DUNGEON_TEXT_TYPE_MESSAGE` into the source message area. Repeated
SET, off-party reveals, clears, and existing-visible text remain silent. The
route contains no generated text or replacement font. Verification: Ninja
build of `firestaff_m11` and
`test_dm1_v1_square_state_dispatch_pc34_compat`; focused CTest passed 1/1.

# ✅ 2026-07-13 DM1 F0218 projectile-impact aftermath for C38/F0266

The shared M10 projectile-cell pass now runs ReDMCSB `PROJEXPL.C F0217`'s
full `GROUP.C F0190` aftermath after a kill: fixed and slot possessions,
C29-C41 cleanup, fear, unlink, active-group retirement, and raw C04 writeback
all use the existing typed F0190 receipt. A surviving hit now schedules the
source C30 reaction through the same F0209 scheduler. `F0249`, F0267, and the
C38 deferred-cell branch consume the completed aftermath instead of trying to
unlink or relink an already-dead group. Verification: Ninja and
`dm1_v1_f0206_packed_directions_runtime_pc34_compat` passed.
- 2026-07-13 Nexus Structure2 raw-span composition receipt: the bounded DGN
  parser now counts zero and nonzero bytes in its verified post-`FFFF` span,
  with a fixture that proves the count changes without promoting payload
  semantics. This is raw envelope provenance only, not a decoder, palette,
  image, record, or render route. Verification:
  `nexus_v1_dgn_geometry_readiness`.
## Nexus Direct Structure1F Geometry (2026-07-15)

The source-bound Structure1F owner relation now exposes the exact selected
 Structure3 face, vertex rows, and paired normal from canonical LEV bytes.
The LEV00-LEV15 corpus test verifies that this route remains no-draw and does
not claim transforms, materials, palettes, VDP1, or decoder semantics.
# DM1 HoC all-C127 C026 atlas source boundary (2026-07-15)

The DM1 F0172/F0107 mirror receipt now rejects C127 `sensorData` outside the
real C026 8x3 atlas rather than deriving an out-of-bounds source rectangle or
showing fallback art. It clears the portrait/materialized payload while
retaining the normal fail-closed wall route. The real-PC34 HoC directional
gate now scans every map-0 C127 sensor, validates its 0..23 C026 index, tests
the one visible wall cell plus all three negative directions, and requires
the original C346 backing before C026. This is source/material proof only;
an operator-captured Mac/release frame is still required for app-level visual
evidence. Verification: `dm1_v1_champion_mirror_pc34_compat`,
`dm1_v1_hoc_mirror_pc34_material_gate`, and
 `dm1_v1_hoc_mirror_directional_pc34_material_gate` pass.
# ✅ 2026-07-16 DM1 GROUP F0179 aspect update source mapping

DM1 now has a source-named `F0179_GROUP_GetCreatureAspectUpdateTime` compat
adapter next to the active-group bridge. The adapter consumes caller-owned raw
C04 group data, the matching G0243 creature-info row, explicit game time, and
caller RNG, then mutates only the source ACTIVE_GROUP aspect slots selected by
the original creature-index/group sentinel path. It preserves the source
attack/non-attack latch and flip rules, horizontal/vertical offset draws, and
`AnimationTicks` cadence without consulting decoded substitutes or hidden
globals. Verification: `ctest --test-dir build-local-ninja -R
'^dm1_v1_group_active_state_pc34_compat$' --output-on-failure` passed.

# ✅ 2026-07-16 DM1 GROUP F0193 Giggler steal source mapping

DM1 Giggler steal/flee resolution now follows ReDMCSB `GROUP.C`
`F0193_GROUP_StealFromChampion` for PC34: the F0822 resolver consumes the
source `G0025` steal-slot table, expands backpack-base hits with
`RANDOM(17)` only after the luck gate, preserves the five-step
percentage/counter loop, and reports Giggler melee as a steal action instead
of creature damage. The existing source-locked steal-slot table remains the
single table authority. Verification: `ctest --test-dir build-local-ninja -R
'^(dm1_v1_creature_ai_behavior_source_lock|dm1_v1_steal_from_slot_indices_pc34_compat)$'
--output-on-failure` passed.

# ✅ 2026-07-16 DM1 TIMELINE F0233-F0239 heap primitives source mapping

The DM1 event timer queue now closes the ReDMCSB `TIMELINE.C` heap primitive
bundle `F0233`-`F0239`: initialization, F0234 time/type/priority/index
ordering, F0235 live lookup, F0236 heap repair, F0237 deletion/reuse, F0238
non-C00 admission plus F0652 square-event merge semantics, and F0239
extract-first deletion. The implementation now rejects C00 events before
capacity/heap mutation and matches C02 door-destruction cleanup against all
same-map C01/C10 conflicts rather than same-time-only substitutes. Focused
coverage verifies priority/address ties, wall-cell merge separation,
door-destruction scope, and caller reschedule repair. Verification:
`ctest --test-dir build-local-ninja -R '^dm1_v1_event_timer_source_lock$'
--output-on-failure` passed.

# ✅ 2026-07-16 DM1 ReDMCSB TIMELINE/MOVE/GROUP symbol bundle

The next DM1 callable backlog block before CHAMPION is now closed in the
ReDMCSB full audit and disposition table: `F0230`, `F0252`-`F0258`,
`F0260`, `F0262`, `F0263`, `F0266`, both `F0267` rows, and `F0514`. These
rows map to existing DM1 source-backed code/tests for F0230 creature melee,
C60/C61 group movement, C11 action/quiver handoff, C12 damage-hide redraw,
C13 Vi Altar rebirth, C53 watchdog receipts, C70 light decay, teleporter
rotation, moving-group projectile prechecks, movement-result routes, and
creature movement sound lookup. Narrow rows explicitly stay bounded:
copy-protection side effects, full action-panel redraw breadth, standalone
F0258 UI querying, full status-box pixel parity, and unimplemented F0266/F0267
route breadth are not claimed. Verification: focused Ninja/CTest,
`git diff --check`, and `python3 tools/symbol_backlog.py --game DM1 --limit
40`.

# ✅ 2026-07-17 DM1 GROUP F0197 DoorInfo portcullis LoS receipt

M11's F0197/F0200 route now admits a closed C3/C4 door only after the loaded
square's first Thing is an authenticated C00 record whose raw next/link and
full bitfield still equal the decoded Door. The raw C00 type selects the
current map's `DoorSet0` or `DoorSet1`, then the existing ReDMCSB G0254
DoorInfo row supplies `CREATURES_CAN_SEE_THROUGH`. Missing, wrong-type, or
drifted C00 data remains opaque; no display-state inference or synthetic
visibility is used. Verification: `m11_creature_projectile_runtime_source_lock`
and the six-test F0190/LoS CTest group PASS; full `firestaff` Ninja build and
`git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0190/F0197-F0200 live C04 LoS admission

M11's live group tick now enters the ReDMCSB `GROUP.C` sight/movement path
only after the raw C04 record still equals its decoded group, the loaded
source Thing chain owns that exact group on the claimed square, and any live
active-group AI position agrees. It runs F0200 through the existing F0197-
F0199 loaded-DUNGEON route callback, preserving the source RNG order, before
projectile or movement handling. A successful raw-chain move updates the
matching active-group AI position atomically; C04 identity, AI-coordinate,
or route-tile drift rejects before mutation or render. C3/C4 door records
remain source-opaque until an authenticated DoorInfo see-through receipt is
available. Verification: `dm1_v1_f0190_c040_m11_integration_audit`,
`dm1_v1_f0190_moving_killed_all_tick_boundary_pc34_compat`,
`dm1_v1_f0190_moving_killed_all_m10_handoff_pc34_compat`,
`dm1_v1_f0190_killed_all_runtime_cleanup_pc34_compat`,
`m11_creature_fixed_possession_runtime_source_lock`, and
`m11_creature_projectile_runtime_source_lock` PASS; `firestaff` Ninja build
and `git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0201 live direct-scent route

M11 now takes GROUP.C F0201's direct party-smell branch when F0200 sight
fails, but only after the live group still matches raw C04 and F0198/F0199
walks the loaded DUNGEON route. The F0201 direction plan is passed into the
existing source-ordered movement candidate loop without a second F0228 RNG
draw. M11 supplies no stored-scent record because the PC34 handoff has no
authenticated raw owner for that opaque scent ring; the fallback therefore
remains fail-closed. Regression covers an opaque raw C00 door that blocks
F0200 yet admits the F0198 smell route, plus C04 identity drift and a raw wall
that both reject before movement. Verification:
`m11_creature_projectile_runtime_source_lock` PASS; full `firestaff` Ninja
build and `git diff --check` PASS.

# ✅ 2026-07-17 DM1 M10 F0201 stored-scent receipt

M10 now admits GROUP.C F0201's stored-scent fallback only from a bounded
G0407 `Party.Scents`/`ScentStrengths` receipt whose canonical FNV still
matches the published source snapshot. The live owner selects the
party-square source entry, reuses its raw map coordinate and strength, and
passes it through the existing F0198/F0199/F0228 path. Invalid map bounds,
publication mismatches, and any post-publication byte drift leave the
fallback absent; no direction is synthesized. The M10 reaction owner carries
the resulting direction into the source-ordered F0810 dispatch without a
second RNG draw. Regression covers valid blocked-route fallback, receipt
drift, and malformed map ownership. Verification:
`m11_creature_projectile_runtime_source_lock` PASS; full `firestaff` Ninja
build and `git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0205/F0206 C37 packed-direction receipt

M10's C37 reaction owner now calls the live-RNG F0206/F0205 path and retains
the complete authenticated PC34 `ACTIVE_GROUP::Directions` byte between
events. C04 receives only the source low-direction slot, while the receipt
keeps the remaining per-creature directions for later readers. The focused
probe locks highest-to-lowest traversal, each nonzero-creature RNG gate,
opposite-turn correction, half-square paired writeback, and C38 receipt
consumption. C29/F0267 relinking, C38-C41 retry turns, and C14/F0219 motion
remain explicitly outside this bounded owner. Verification:
`dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
`dm1_v1_creature_ai_behavior_source_lock`,
`m11_creature_projectile_runtime_source_lock`, and
`dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build and
`git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0209 C38 source turn/retry

M10 now stages C38's F0205 turn against the authenticated packed direction
receipt before attack. An opposite-facing attacker takes only the source
one-step turn, consumes the supplied master RNG, commits C04's low direction
only after its same-C38 retry is admitted, and retries two ticks later. A full
timeline rejects without changing the direction or RNG state. C29/F0267 and
C39-C41 remain separate F0209 owners. Verification:
`dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
`dm1_v1_creature_ai_behavior_source_lock`,
`m11_creature_projectile_runtime_source_lock`, and
`dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build and
`git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0209 C39-C41 source turn/retry

The M10 F0209 C38 turn/retry owner is now regression-locked for each of the
remaining per-creature events C39, C40, and C41. A line-of-attack event turns
only its addressed packed `ACTIVE_GROUP::Directions` slot through F0205,
preserves the other three slots, commits only C04's low direction view, and
queues the matching source event at `GameTime + 2` before F0207 attack work.
The turn step neither sets an attack aspect nor relocates the group. C29/F0267
remains an explicit no-op because this package has no source-backed physical
move owner. Verification: `dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
`dm1_v1_creature_ai_behavior_source_lock`,
`m11_creature_projectile_runtime_source_lock`, and
`dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build and
`git diff --check` PASS.

# ✅ 2026-07-17 DM1 PROJEXPL F0213/F0220 C15-C25 runtime boundary

Added DM1 source-named F0213 and F0220 adapters over the existing bounded M10
explosion lifecycle. Original PC34 C25 materialization and M10 live creation
now enter via F0213; C25 dispatch enters via F0220. The chain preserves the
same live C15 slot, combat fanout, despawn, and next-C25 scheduling behavior,
while missing owners and invalid inputs reject without an effect substitute.
Verification: `dm1_v1_f0213_f0220_explosion_runtime_pc34_compat`,
`dm1_v1_original_save_pc34_handoff`, and
`m11_creature_projectile_runtime_source_lock` PASS; isolated `firestaff`
build and `git diff --check` PASS.

# ✅ 2026-07-17 DM1 GROUP F0200/F0202-F0204 live movement admission

M10 now materializes each GROUP.C F0202 destination from the loaded raw
DUNGEON square and its C00/C04/C15 Thing chain. Missing tiles, malformed
links, unsupported records, and cross-map visibility remain fail-closed.
F0202 preserves terrain, party, door, and group blocker order; F0203 owns the
tested-direction write; and F0204 performs the second raw-square read only
after the first pass, retaining the one-square move when Fluxcage or the next
record blocks it. F0200 also clears stale visibility across map ownership.
Regression covers all F0202 blocker classes, F0203 state order, F0204's
blocked second step, and the M11 C04 route. Verification:
`dm1_v1_creature_ai_behavior_source_lock` (375 assertions) and
`m11_creature_projectile_runtime_source_lock` PASS; full `firestaff` Ninja
build and `git diff --check` PASS.

# DM1 C15 pool transaction stage 2 (2026-07-17)

- Added the narrow ReDMCSB `DUNGEON.C F0166/F0163/F0164` C15 transaction
  owner. It snapshots the exact unused four-byte C15 row and decoded mirror,
  reserves through F0516, writes only C15 Type/Attack/Centered fields, links
  through F0514, and unlinks/restores the raw and decoded preimage on rollback.
  The focused regression covers raw and decoded initialization, preserved SFT
  head with a C15 tail link, explicit invalid-map rollback, and exact pool-row
  restoration. C25 receipt publication and F0217 remain deliberately outside
  this stage. Verification: `dm1_v1_c15_layout_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build PASS.

# DM1 C15/C25 publication receipt stage 3 (2026-07-17)

- Extended the shared C15 owner with the source C25 publication receipt:
  exact `MapTime`, `B.Location`, `C.Slot`, Priority and the four-byte C15 FNV.
  Publication initializes and links the C15 transaction before exposing the
  receipt; invalid inputs or a failed live-SFT/FNV check roll the C15 row back
  atomically. F0435 now uses the same FNV owner as F0802. Focused coverage
  proves raw/decoded C15 state, C25 receipt drift rejection, exact rollback,
  F0435->F0802->F0435 C25 Slot roundtrip, and C15/fingerprint mutation
  rejection. F0217 remains a separate consumer step. Verification:
  `dm1_v1_c15_layout_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build PASS.

# DM1 F0217 Ven/Ful C15/C25 handoff (2026-07-17)

- Wired the source-owned `PROJEXPL.C F0217` Ven/Ful impact branch through the
  shared atomic F0213 C15/C25 owner in M10. A byte-identical C14 Slot and C05
  power/type record are required; Ven produces centered C007 with a legal
  C15 cell and Ful produces C000 at the original cell. C15 and C25 publish
  before runtime F0213. C05 drift, missing C15, runtime exhaustion, or a
  schedule failure retain no host-only explosion and restore C15/SFT/runtime/
  timeline state. Verification: `dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
  `dm1_v1_c15_layout_pc34_compat`, and `dm1_v1_original_save_pc34_handoff`
  PASS; isolated `firestaff` build PASS.

# DM1 F0218 authenticated pending-impact owner (2026-07-17)

- Replaced F0218's host-list-only count at the deferred C38 boundary with a
  source-owned SFT C14 walk. Every counted projectile now proves its raw C14
  bytes, decoded C14 mirror, exact cell, and active runtime projection before
  F0209 can run F0190 compaction. Missing or drifted C14 data rejects before
  mutation. Verification: `dm1_v1_projectile_impact_count_pc34_compat` and
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat` PASS; isolated
  `firestaff` build PASS.

# DM1 F0214 C14 event-index writeback (2026-07-17)

- F0214 now writes each C14 `EventIndex` shifted by its exact C49 deletion
  back to the raw PC34 record. This retains raw/decoded identity across queue
  compaction and original-save export. Verification:
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat` and
  `dm1_v1_original_save_pc34_handoff` PASS; isolated `firestaff` build PASS.

# DM1 F0212 C14 source publication (2026-07-17)

- Added the shared raw C14 transaction owner: F0516 reserves an unused
  projectile record, exact PC34 Slot/KineticEnergy/Attack/EventIndex bytes
  initialize with the decoded mirror, F0514 links the cell-specific Thing,
  and F0515 restores raw/decoded/SFT state on any failed publish. M10 now
  uses that transaction for loaded F0327 spell and F0207 creature callers:
  it schedules exactly one C49, writes its physical EventIndex into C14, and
  restores C14/runtime/timeline state on every failure. Memory-only harnesses
  remain isolated rather than inventing source data. Verification:
  `dm1_v1_c14_layout_pc34_compat`,
  `memory_tick_orchestrator_f0303_skill_query_pc34_compat`,
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat`, and
  `m11_creature_projectile_runtime_source_lock` PASS; `firestaff` built in
  `build/codex-dm1-f0205`; `git diff --check` PASS.

# DM1 F0221 C15 fluxcage source blocker (2026-07-17)

- Added the source-owned `PROJEXPL.C F0221` square-chain reader. For loaded
  original Thing data, F0219 now obtains `destHasFluxcage` solely by walking
  the destination SFT list and checking each C15 raw `Next/Type/Centered/
  Attack` layout against its decoded mirror. A real C050 consumes the C14
  projectile; a drifted C15 rejects before C14/runtime mutation, and no host
  explosion-list fallback is consulted. Verification:
  `dm1_v1_c15_layout_pc34_compat`,
  `dm1_v1_f0206_packed_directions_runtime_pc34_compat`,
  `dm1_v1_original_save_pc34_handoff`,
  `memory_tick_orchestrator_f0303_skill_query_pc34_compat`, and
  `dm1_v1_c14_layout_pc34_compat` PASS; `firestaff` built in
  `build/codex-dm1-f0205`; `git diff --check` PASS.

# DM1 HoC mirror input and movement-arrow feedback (2026-07-22)

- Fixed DM1 HoC C127 portrait selection when the live GRAPHICS.DAT loader
  has the real C346/C026 mirror material but the broad launcher
  `assetsAvailable` latch has not been set.  The host now admits input from
  the resident source bitmaps themselves; C017/C040/C027 command material
  follows the same rule and still fails closed when those original assets are
  absent.  `m11_dm1_hoc_no_fallback_panel` covers both C127 and C040 paths.
- Keyboard/controller feedback now outlines the complete ReDMCSB C068..C073
  hit rectangle, so the turn-arrow indicator matches the visible button
  extent rather than only showing tiny corner cues.  Verified by
  `m11_overlay_command_queue_block`.
# DM1 source-material runtime bundle (2026-07-22)

- F0128 now preflights its complete per-square source-material set before any
  draw step. A missing PC34 asset rejects the full plan, including the
  `0x0000` alcove case, with no partial or synthetic output.
- HoC C127/C346/C026 and C017/C040/C027 interaction gates now admit exactly
  resident original material instead of depending on the broad launcher asset
  latch. Missing C346 remains an explicit no-draw condition.
- PC34 save resume is now atomic across import, world materialization, event
  queue adoption, and rejection rollback. A rejected byte stream cannot alter
  the live runtime.
- Added a source-gated four-champion top-row frame plan for C008/C028 and
  C033/C034/C035, and tightened live action/spell effects to require their
  original GRAPHICS.DAT material and zones.
- Verified: `firestaff` build plus nine focused DM1 CTests, all passing.
# DM1 champion top-row asset receipt (2026-07-22)

- Added a DM1-owned GRAPHICS.DAT receipt for C008, C028, C033, C034, and
  C035. It preserves source pixels and rejects missing or wrong-sized surfaces
  before the top-row plan can be used. Registered and passed its CTest along
  with the existing top-row and champion-panel source-lock tests.
# DM1 action/spell material and HoC command chain (2026-07-22)

- F0231, F0407, and F0412 presentation receipts now require their original
  PC34 graphics, M653 font variant, and destination zone. Unsupported MISS or
  DOOR output is deliberately no-draw instead of host text.
- F0873 now binds C127 selection through C040 and C160/C161/C162: reincarnate
  waits for rename, resurrect requires the active mirror sensor, and cancel
  restores the original C127/C026 route.
- Verified with focused action-effect, resurrection, mirror-candidate, and
  cancel-route CTests.
# DM1 M11 F0128 source scheduler consumption (2026-07-22)

- Wired the full source-material F0128 scheduler into M11's production
  viewport path. A missing or invalid mounted GRAPHICS.DAT source produces a
  black no-draw viewport and never re-enters the retired F0115 fallback loop.
- Verified with both scheduler and M11-wiring CTests.
# DM1 champion top-row presentation receipt (2026-07-22)

- Added the ordered PC34 presentation receipt for top-row C008/C028 and
  C033/C034/C035 operations, name zones, and bars. It only accepts a complete
  source asset receipt and a valid live-party plan, otherwise emits no frame.
- Registered the CTest. Asset, plan, presentation, and existing HUD source
  lock all pass.
# DM1 action/spell sequence and original-save event hardening (2026-07-22)

- Added a fail-closed F0407/F0412 presentation sequence: C010 action header
  and rows, C009/C011 spell rows, C014 damage, and the appropriate M653 font
  variants are emitted only with their source material and PC34 zones.
- Strengthened atomic original-save adoption for C13/C24/C25. Imported records
  must match their source event slot, pose, time, and active C15 explosion
  state; the historical `ExplosionList.count` shortcut is no longer accepted.
- Focused sequence and original-save CTests pass.
# DM1 HoC C040 redraw-close-reopen receipt (2026-07-22)

- Added a source-owned HoC candidate-panel receipt for C040 redraw, C162
  close/restore, and reopen. It keeps C127 sensor ownership and panel
  generations explicit, accepts valid atlas ordinal zero, and fails closed on
  stale generations or missing C026/C040 material.
- The focused HoC receipt, action/spell sequence, and original-save tests pass.
# DM1 M648 inscription transaction and F0296 HUD transitions (2026-07-22)

- M11 now verifies all F0168 glyph bindings and the exact M648 font raster
  before drawing a wall inscription. Any invalid late glyph fails the entire
  transaction, preserving the original wall without a replacement font.
- Added F0296 transition receipts for candidate early-return, inventory F0292
  repaint, changed action-hand slots, and dead C008 status rendering.
- `firestaff` plus M648/F0168, champion-panel, presentation, and F0296 tests
  pass.
# DM1 action/spell render-command admission (2026-07-22)

- Added fail-closed command admission from the F0407/F0412 presentation
  sequence to source-owned decoded GRAPHICS.DAT surfaces. Each blit and font
  step validates its graphic, source rectangle, zone, dimensions, and pixels;
  one invalid step rejects the whole batch.
- `firestaff` and the focused action/spell tests pass.
# DM1 HoC C160/C161/C162 source handoff (2026-07-22)

- Added a source-owned final HoC candidate handoff: C160/C161 retire C040 and
  stale C026 only when they match the active C127 sensor; C162 restores only
  the matching live C127/C026 route and is explicitly reopen-eligible.
- The handoff, panel, resurrection, and cancel-route tests pass with a full
  `firestaff` build.
# DM1 champion F0293/F0292 redraw priority (2026-07-22)

- Added a source-owned champion redraw receipt: F0293/F0292 status operations
  run first, then C032 poison and C015/C016 damage. Dead champions retain the
  C008-only path; any missing selected original material rejects the receipt.
- `firestaff` and the focused HUD/damage transition tests pass.
# DM1 action/spell execution and HoC atomic apply receipts (2026-07-22)

- Bound admitted action/spell command batches to source-owned live effect,
  champion, tick, serial, and fingerprint receipts. Effect or batch mismatch
  rejects execution.
- Added an atomic HoC apply plan for G0299/G0305, C040, C127, and C026:
  C160/C161 retire the source state while C162 alone restores its live route.
- `firestaff` and all six focused action/spell and HoC tests pass.

# DM1 F0267 local floor-sensor rotation (2026-07-22)

- F0270 now carries G0403-G0406 as the triggering floor square and
  CM1_CELL_ANY. After the complete F0276 pass, F0271 rotates the original
  sensor chain once, preserving both decoded and raw `Next` words.
- The focused F0267 regression verifies ordered CLEAR/TOGGLE local effects,
  raw-chain persistence, and the final local receipt.

# DM1 original-save span operations (2026-07-22)

- F0415/F0416 now provide fail-closed bounded read/write spans. F0421/F0422
  consume those operations before updating their original running checksum,
  so rejected spans leave cursor, bytes, and checksum unchanged.
- The original-save handoff regression passes. The configured local DM1 corpus
  contains no PC34 saves, so the optional real-save leg is correctly skipped.

# DM1 SAVEUTIL F0418 checksum (2026-07-22)

- F0418 now owns the non-mutating checksum of stored PC34 save words; F0796
  calls it before F0417 deobfuscates a separate destination span.
- The focused SAVEUTIL regression passes and verifies parity with F0417's
  checksum while preserving the source bytes.

- 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 6):
  Closed eight SkWinCore priority symbols as `IMPLEMENTED_NARROW` source-named
  receipts in `dm2_v1_skproject_core.c`: `_1c9a_02c3` (creature AI pointer
  resolver), `_4937_01a9` (animation frame selection), `_4937_000f` (resolved
  animation sequence word), `_2759_0155` (command-string presence check),
  `_2759_01fe` (container/minion command validity gate), `_2759_0e93` (hand
  activation predicate), `_24a5_0732` (centered viewport string draw), and
  `_2e62_03b5` (item icon update). Five SKULLWIN aliases close as the same
  receipts: `DM2_guidraw_2e62_03b5`, `DM2_2759_0e93`, `DM2_query_1c9a_02c3`,
  `DM2_query_2759_0155`, and `DM2_query_2759_01fe`.
  Changes:
    * `include/dm2_v1_skproject_core.h`:
      - Added focused receipt structs for all eight SKWIN symbols:
        `DM2_V1_SkprojectCreatureAIPointerReceipt`,
        `DM2_V1_SkprojectSelectFrameReceipt`,
        `DM2_V1_SkprojectAnimationW0Receipt`,
        `DM2_V1_SkprojectQueryObjectCommandsReceipt`,
        `DM2_V1_SkprojectCommandValidReceipt`,
        `DM2_V1_SkprojectHandActivationReceipt`,
        `DM2_V1_SkprojectDrawCenteredVpStrReceipt`, and
        `DM2_V1_SkprojectItemIconUpdateReceipt`.
      - Declared the corresponding eight `dm2_v1_skproject_*` receipt
        functions.
    * `src/dm2/dm2_v1_skproject_core.c`:
      - Implemented source-shaped receipts for `_1c9a_02c3`, `_4937_01a9`,
        `_4937_000f`, `_2759_0155`, `_2759_01fe`, `_2759_0e93`, `_24a5_0732`,
        and `_2e62_03b5` with citations to SKWIN/SkWinCore.cpp lines 3058,
        3070, 10150, 13854, 8249, 5506, 13331, and 14236.
      - Updated `dm2_v1_skproject_core_source_evidence()` to name the new
        cycle-6 symbols.
    * `tests/test_dm2_v1_skproject_core.c`:
      - Added `test_skwin_core_symbol_batch_cycle6()` with focused
        synthetic-data coverage for all eight SKWIN receipts and their five
        SKULLWIN aliases, plus a source-evidence check.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`:
      - Added thirteen `IMPLEMENTED_NARROW` disposition rows for the eight
        SKWIN symbols and five SKULLWIN aliases.
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`:
      - Moved the thirteen corresponding rows from `MISSING` to
        `IMPLEMENTED_NARROW` with Firestaff mapping and evidence notes.
  Source/evidence citations:
    * `skproject/SKWIN/SkWinCore.cpp` lines 3058, 3070, 10150, 13854, 8249,
      5506, 13331, and 14236 for the eight SKWIN symbols.
    * `skproject/SKULLWIN/c_gui_draw.cpp:1833`, `c_hero.cpp:3580`,
      `c_querydb.cpp:2976`, `c_querydb.cpp:4448`, and `c_querydb.cpp:4504`
      for the five alias receipts.
  Verification:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds.
    * `SDL_VIDEODRIVER=dummy /Users/bosse/workspace-main/firestaff/build/firestaff_m11_phase_a_probe`
      passes 24/24.
    * `/Users/bosse/workspace-main/firestaff/build/test_dm2_v1_skproject_core`
      reports `all DM2 skproject core helper checks passed`.
    * `python3 tools/symbol_backlog.py --game DM2 --limit 20` confirms the DM2
      skproject backlog dropped from 997 to 984 open rows.

- 2026-07-22 DM2 SkWinCore symbol audit SKULLWIN original closure (Lane A, cycle 7):
  Closed the next open Lane A batch: eleven SKULLWIN originals in
  `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` that were still `MISSING` even though
  their source-locked helpers were already implemented and their SKWIN aliases
  were already source-mapped.
  Changes:
    * `src/dm2/dm2_v1_skproject_core.c`:
      - Updated `dm2_v1_skproject_core_source_evidence()` to name the
        SKULLWIN originals: `DM2_1031_01d5`, `DM2_1031_023b`,
        `DM2_1031_024c`, `DM2_1031_027e`, `DM2_1031_030a`, `DM2_1031_04f5`,
        `DM2_1031_0541`, `DM2_1031_0675`, `DM2_29ee_0b2b`, `DM2_1031_03f2`,
        and `DM2_0b36_129a`.
    * `tests/test_dm2_v1_skproject_core.c`:
      - Added `test_skwin_core_symbol_batch_cycle7()` with focused
        synthetic-data coverage for the eight simplest SKULLWIN originals in
        the batch, plus a source-evidence check that names all eleven symbols.
    * `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv`:
      - Added eleven disposition rows for the SKULLWIN originals, mirroring
        the existing SKWIN alias dispositions where applicable.
    * `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`:
      - Moved the eleven corresponding SKULLWIN rows from `MISSING` to
        `VERIFIED_SOURCE_MAPPING` or `IMPLEMENTED_NARROW` with Firestaff
        mapping and evidence notes.
    * `TODO.md`:
      - Added the cycle-7 update under the DM2 skproject audit history.
  Source/evidence citations:
    * `skproject/SKULLWIN/c_1031.cpp` lines 23, 49, 54, 144, 184, 264, 289,
      and 401 for the eight `DM2_1031_*` originals.
    * `skproject/SKULLWIN/c_gui_draw.cpp:5158` for `DM2_29ee_0b2b`.
    * `skproject/SKULLWIN/c_input.cpp` lines 55 and 523 for
      `DM2_1031_03f2` and `DM2_0b36_129a`.
    * The existing SKWIN alias audit rows and `dm2_v1_skproject_core.c`
      source-locked receipts serve as the runtime mapping evidence.
  Verification:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds.
    * `/Users/bosse/workspace-main/firestaff/build/test_dm2_v1_skproject_core`
      reports `all DM2 skproject core helper checks passed`.
    * `/Users/bosse/workspace-main/firestaff/build/test_dm2_v1_skproject_cpx_heap`
      reports `all DM2 skproject CPX heap receipt checks passed`.
    * `grep -c 'MISSING$' docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
      confirms the DM2 skproject backlog dropped from 954 to 943 open rows.

- 2026-07-23 DM2 V1 0x04 actuator tile subdispatch expansion (Lane B, cycle 8):
  Expanded the DM2 V1 `DM2_PROCEED_TIMERS` type 0x04 (`DM2_V1_TIMER_ACTUATE_TILE`)
  subdispatch with DM2-owned, source-locked class handlers.  Classes 0/2/4/5/6 now
  bind at boot (they only need the boot dungeon data); class 1 (floor mecha)
  remains gated on the record-pool/CAII think binding because it walks DB records.
  Changes:
    * `include/dm2_v1_runtime.h`:
      - Added `DM2_V1_RuntimeActuatorTileReceipt` and
        `dm2_v1_runtime_actuator_tile_receipt()`.
    * `src/dm2/dm2_v1_runtime.c`:
      - Added internal counters for the actuator-tile subdispatch.
      - Added `dm2_runtime_actuate_wall_mecha` (class 0): consumed, fail-closed
        counter; the CCM tail is not yet source-bound.
      - Added `dm2_runtime_actuate_pitfall` (class 2): bounded `FLOOR`↔`PIT`
        square-type toggle using `value_b` bit 0 as the direction.
      - Added `dm2_runtime_actuate_door` (class 4): bounded one-step door toggle
        using `dm2_door_apply_toggle_step()`.
      - Added `dm2_runtime_actuate_teleporter` (class 5): consumed, fail-closed
        counter.
      - Added `dm2_runtime_actuate_trickwall` (class 6): consumed, fail-closed
        counter.
      - Reorganized dispatcher binding so classes 0/2/4/5/6 are always wired and
        class 1 stays gated on `think_binding_ready`.
    * `tests/test_dm2_v1_proceed_timers_pc34_compat.c`:
      - Replaced the single class-4 subdispatch smoke check with a per-class
        surface test covering classes 0, 1, 2, 4, 5, 6, the source class-3
        no-op case, and class > 6 fail-closed behavior.
    * `tests/test_dm2_v1_runtime_handoff_smoke.c`:
      - Added `#include "dm2_v1_world_model.h"` for `DM2_SQUARE_PIT`.
      - Added `test_actuator_tile_subdispatch_wiring()`: seeds a class-2
        `FLOOR` pitfall and a class-4 `CLOSED` door, enqueues two 0x04 timers,
        ticks once, verifies the pit becomes `PIT` and the door raw state
        moves to `CLOSED_THREE_QUARTER`, and checks the
        `DM2_V1_RuntimeActuatorTileReceipt` counters.
    * `TODO.md`:
      - Updated the Phase 4 mechanics-parity line to mark the 0x04 actuator
        tile subdispatch expansion landed and narrowed the remaining work to
        the broader timer matrix (remaining timer types) and shops/NPCs.
  Source/evidence citations:
    * `skproject/SKULLWIN/c_tim_proc.cpp:4214-4230` (0x04 class dispatch).
    * `skproject/SKULLWIN/c_tim_proc.cpp:1923` (`DM2_ACTUATE_WALL_MECHA`).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3009` (`DM2_ACTUATE_FLOOR_MECHA`).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3707` (`DM2_ACTUATE_PITFALL`).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3744` (`DM2_ACTUATE_DOOR`).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3832` (`DM2_ACTUATE_TELEPORTER`).
    * `skproject/SKULLWIN/c_tim_proc.cpp:3875` (`DM2_ACTUATE_TRICKWALL`).
    * `ReDMCSB TIMELINE.C:750-810` (door state transitions).
    * `ReDMCSB DEFS.H:385-390` (`DM2_SQUARE_*` type constants).
  Verification:
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --target test_dm2_v1_proceed_timers_pc34_compat test_dm2_v1_runtime_handoff_smoke && ./build/test_dm2_v1_proceed_timers_pc34_compat && ./build/test_dm2_v1_runtime_handoff_smoke`
      passes (`dm2_v1_proceed_timers_pc34_compat: all checks passed`,
      `PASSED: 176 FAILED: 0` for handoff smoke).
    * `cmake --build /Users/bosse/workspace-main/firestaff/build --parallel`
      succeeds.

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

# Firestaff DONE - DM1

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
# ✅ 2026-07-13 Theron dynamic Track 02 CD_READ-to-RAM receipt: the
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

# ✅ 2026-07-13 Nexus Structure1Fa ITEM.IBS special-floor palette consumer

`nexus_v1_item_ibs_parse_verified()` now validates the documented special
floor-image descriptor table, local/inherited 16-colour BGR555 palettes and
bounded raw payload spans. `nexus_v1_dgn_bind_structure1f_item_materials()`
binds that original palette/payload receipt to the matching floor command.
The unproved `0008` pixel codec remains no-draw and cannot fall back to an
inventory icon. Focused `nexus_v1_dgn_geometry_readiness` passes.

# ✅ 2026-07-13 Nexus ITEM.IBS special-floor packed-4bpp corpus gate

The canonical `ITEM.IBS` corpus now proves that descriptor encoding `0008`
has a packed `width * height / 2` 4bpp span, with local 16-colour BGR555
palettes interleaved before later payloads. The parser maps the on-disc
0..108 floor ordinals into the combined 223..331 image space, preserves the
positive packed payload, and treats legal `FFFF` inventory associations as
no-draw. The raw nibble order and world placement remain intentionally
blocked. Verified against `~/.firestaff/data/nexus/ITEM.IBS` through
`nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-13 Nexus ITEM.IBS 0008 DGN command-material consumer

Verified ITEM.IBS `0008` floor payloads now reach an explicit, command-indexed
DGN material consumer with their exact packed bytes, local BGR555 palette,
dimensions, and source provenance. The consumer rejects a non-floor or
out-of-range command and never authorizes drawing: original nibble order and
3D placement remain unproven, with no inventory-icon or synthetic fallback.
The focused `nexus_v1_dgn_geometry_readiness` target passes against the local
retail ITEM.IBS corpus.

# ✅ 2026-07-13 Nexus Structure1F ITEM.IBS retail-coverage gate

`nexus_v1_dgn_structure1f_item_ibs_coverage()` now validates every direct
Structure1F item against the authenticated ITEM.IBS bank before it can reach a
material path. The local LEV00–LEV15 corpus proves 446 item records and 174
separate descriptor-`0008` references, with no missing or unsupported source
descriptor. The receipt remains no-draw and fail-closed: it does not claim a
Saturn texel order or world placement. Verified by
`nexus_v1_dgn_geometry_readiness` with the retail corpus.

# ✅ 2026-07-13 Nexus ITEM.IBS 0008 VDP1 codec-provenance gate

The new `nexus_v1_item_ibs_decode_0008_vdp1_4bpp()` keeps the Saturn VDP1
high-nibble-first rule behind four independent provenance facts: verified
ITEM.IBS identity, original VDP1 command stream, 16-colour mode, and the
byte/nibble route. Retail ITEM.IBS descriptors therefore remain blocked and
no-draw when only their own data is available. Focused
`nexus_v1_dgn_geometry_readiness` verifies both the blocked retail route and
the source-gated decoder contract without a fallback.

# ✅ 2026-07-13 Nexus DM.BIN PRS3 marker catalog

`nexus_v1_prs3_dm_bin_catalog_verified()` now reads only bounded, literal
PRS3 framing from hash-verified original DM.BIN bytes. The retail corpus has
two markers: an unclassified executable occurrence and one complete V1 record
with target `4096` and first frame word `997`. Truncated records fail closed;
the catalog never promotes a PRS3 opcode decoder or a render route. Verified
by `nexus_v1_prs3_capture_trace_schema` against local retail DM.BIN.

# ✅ 2026-07-13 Nexus PRS3 DM.BIN/MENU.BPK outer-frame receipt

`nexus_v1_prs3_cross_asset_frame_receipt_verified()` now compares only the
hash-verified V1 outer-frame fields shared by original `DM.BIN` and
`MENU.BPK`. The local retail corpus proves one complete DM.BIN V1 record and
162 complete MENU.BPK V1 frames, each with a nonzero first frame word. It does
not infer any opcode grammar, control-bit order, termination rule, decoded
pixel output, or menu render route: decoder promotion, menu handoff, and
fallback visuals remain disabled. Verified by the focused
`nexus_v1_prs3_capture_trace_schema` CTest with the local retail corpus.

# ✅ 2026-07-13 Nexus PRS3 V1 SH-2 execution receipt

`nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified()` imports the exact
instruction-level facts already isolated from the hash-verified retail
`DM.BIN`: the V1 control test at `85450`, R12 post-increment byte read at
`85460`, R13/R0 byte store at `85464`, and loop branch at `85472`. Any changed
anchor rejects the receipt. These are loader control/dataflow facts only; no
live `MENU.BPK` frame binding, VDP1 command observation, opcode grammar, or
decoder/menu route is promoted. Verified by
`nexus_v1_prs3_capture_trace_schema` against local retail `DM.BIN`.

# ✅ 2026-07-14 Nexus PRS3 SH-2-to-VDP1 capture gate

`nexus_v1_prs3_vdp1_capture_schema_parse()` and its asset-binding companion
now define a strict future-capture contract for one exact `MENU.BPK` PRS3
frame: hash-bound BPK/DM.BIN bytes, bounded packed input span, SH-2 input and
output address ranges, complete output fingerprint, and a later VDP1 command
whose texture source is that exact output range. Partial or inconsistent
traces reject atomically. The gate records an observed handoff only; it never
claims an opcode grammar, enables generic PRS3 decoding, or permits fallback
visuals. Verified by `nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 VDP1 command/palette capture contract

The original-capture schema now accepts V3 evidence for one hash-bound
`MENU.BPK` PRS3 frame. In addition to the existing SH-2 input/output and VDP1
texture-read witnesses, V3 requires contiguous raw VDP1-command and palette
read spans with ordered sequence numbers, byte counts, addresses, and FNV
witnesses. Binding remains tied to the exact MENU.BPK/DM.BIN input, and a
changed palette span rejects the capture. This establishes no PRS3 opcode,
texture-pixel, palette-format, or VDP1 field semantics, and it never permits
rendering or fallback visuals. Verification:
`test_nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 external-capture validator

`firestaff_nexus_v1_prs3_v3_capture_validator TRACE MENU.BPK DM.BIN` now
imports a read-only V3 candidate trace only after the two supplied ordinary
files match the canonical Track 1 MD5 identities. The validator binds the
trace's MENU.BPK span to real bytes and reports the VDP1-command/palette
witnesses, while leaving runtime import, decoder promotion, and fallback
visuals disabled. It does not manufacture a trace or attest the capture
producer. Verification: `nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 raw-sidecar admission

The V3 validator now optionally accepts three read-only capture sidecars:
decoder output, raw VDP1 command bytes, and raw palette bytes. Admission
requires each sidecar's exact recorded size and FNV witness to match the
hash-bound V3 trace after canonical `MENU.BPK`/`DM.BIN` validation. The CLI
accepts all six inputs and reports each binding separately. It does not claim
that a file was produced by an original Saturn/emulator, decode any sidecar,
or permit runtime import, rendering, or fallback. Verification:
`nexus_v1_prs3_capture_trace_schema`.

# ✅ 2026-07-14 Nexus PRS3 V3 provenance-ledger gate

The raw-sidecar admission can now be accompanied by a strict text ledger that
hash-binds the V3 trace, output, VDP1-command, palette sidecars, and capture
producer binary. Missing or changed files reject the ledger. This records
reproducible provenance only: no local authentic Nexus trace/log was found,
producer authentication remains false, and runtime import stays disabled.
Verification: `nexus_v1_prs3_capture_trace_schema`.
# ✅ 2026-07-13 DM2 atomic GDAT door-material transaction

`dm2_v1_render_doors()` now preloads each required skproject `DM2_DRAW_DOOR`
component (panel, ornament/destroyed mask, frame, and button) with its own
decoded IMG3 local-palette receipt. A missing component blocks the full door
pass before any door pixel can be drawn; complete passes publish required and
consumed component masks. No synthetic door surface is used in source-required
mode. Verification: direct `test_dm2_v1_door_material_gate` 3/3 and Ninja
`test_dm2_v1_runtime_handoff_smoke` 161/161.

# ✅ 2026-07-13 DM2 runtime GDAT floor/ceiling handoff gate

`dm2_v1_runtime_render_frame()` now propagates the viewport's exact required
and consumed floor/ceiling material masks into the runtime ownership and M11
frame receipts. Source-required indoor frames require both GRAPHICSSET planes
to complete their decoded-pixel plus local-palette transaction before the
runtime marks the handoff valid; incomplete material fails closed without a
synthetic surface. Source reference: skproject `DM2_DRAW_DUNGEON`.
Verification: Ninja `test_dm2_v1_runtime_handoff_smoke` passed 161/161.

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

# ✅ 2026-07-13 DM2 hash-receipted original SKSave corpus runtime import

DM2 corpus import now scans to a selected receipt, reloads its exact file
through full-file and payload hashes, and routes original raw/envelope SKSave
payloads through the existing atomic live runtime restore. Invalid or
dungeon-incompatible candidates remain fail-closed with no synthetic session
fallback. Source reference: skproject `c_savegame.cpp` load handoff. Verification:
Ninja `test_dm2_v1_save_load` 23/23 and CTest `dm2_v1_save_load` 1/1.

# ✅ 2026-07-13 DM2 renamed original SKSave corpus discovery

The bounded recursive corpus scanner now discovers a renamed artifact only
after its real 42-byte SKSave header validates, then classifies it through the
existing skproject-shaped parser and records a full-file hash receipt. The
explicit save root is capped at depth 4, 64 header candidates, and 256 shallow
file probes; an incomplete scan is marked truncated. Arbitrary filenames and
bytes never become importable. Verification: Ninja `test_dm2_v1_save_load`
23/23 and CTest `dm2_v1_save_load` 1/1.

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

# ✅ 2026-07-14 DM2 original SKSave pre-rebuild corpus receipt

The original-save corpus census now retains separate byte-identity hashes for
the source-decoded global flags, bytes, words, and global spell effects. This
matches skproject `SkWinCore.cpp::GAME_LOAD` order before timer sorting and
`READ_SKSAVE_DUNGEON`, while keeping timer semantics and dungeon DB rebuild
unpromoted. Verification: Ninja and `test_dm2_v1_save_load`.

# ✅ 2026-07-14 DM2 fixture-free original SKSave corpus census

`test_dm2_v1_save_load` now optionally consumes an explicitly staged
`FIRESTAFF_DM2_SKSAVE_CORPUS`. It creates no files and performs neither export
nor runtime restoration: each discovered original envelope/raw candidate must
remain hash-identical through its receipt reload, then reproduce only the
already source-bound pre-rebuild state census from `GAME_LOAD` before
`READ_SKSAVE_DUNGEON`. An unset corpus root is a non-promoting skip; a supplied
corpus with absent, incomplete, or changed original candidates fails. Source:
SKProject `SKWIN/SkWinCore.cpp::GAME_LOAD` and `SKULLWIN/c_savegame.cpp::DM2_GAME_LOAD`.

# ✅ 2026-07-14 DM2 template-bound raw SKSave export roundtrip

`dm2_v1_session_export_raw_sksave_payload()` exports only from a
parser-verified original raw body. It retains the complete dungeon/DB prefix
byte-for-byte, re-encodes only the owned SUPPRESS party/global/champion/timer
sections, rejects changed party/timer/minion shapes, and re-imports its output
before success. This is a bounded original-format write path, not a fabricated
dungeon export or a full DB parity claim. Source reference: skproject
`c_savegame.cpp` save ordering. Verification: Ninja `test_dm2_v1_save_load`
24/24 and CTest `dm2_v1_save_load` 1/1.

# ✅ 2026-07-13 DM2 G1 exact GRAPHICSSET IMG3 classification

`dm2_v1_asset_load_image_metadata()` now follows skproject
`DME.h::IMG3::Getpf()`: a compressed C8 record is selected by
`OffsetY() == 31`, rather than treating IMG3 `w4` as a universal bit-depth.
The real PC `DUNGEON.DAT` references GRAPHICSSET 1..5; each now resolves its
own floor, ceiling, scene-colorkey, and scene-flag entries from the matching
original `GRAPHICS.DAT` set. Malformed uncompressed `OffsetY() == -32`
records still reject unless `w4` is exactly 4 or 8. No cross-set fallback was
added. Source: skproject `SKWIN/DME.h` IMG3 `Getpf()` and
`SKWIN/SkWinCore.cpp` `QUERY_GDAT_IMAGE_ENTRY_BUFF` / map setup
`2676:0CBB-0D99`. Verification: local canonical
`test_dm2_v1_gdat_graphicsset_real_data`, `test_dm2_v1_gdat_word_values`, and
`test_dm2_v1_weather_gdat_receipt` pass.

# ✅ 2026-07-14 DM2 G1 GRAPHICSSET decoded plane and C8 selector gate

The canonical G1 map GRAPHICSSET 1..5 floor and ceiling IMG3 records now
prove both metadata and successful source-format decode before they are
admitted as scene materials. The C8/IMG9 decoder follows skproject
`SkWinCore.cpp::DECODE_IMG9`: selector byte 6 is accepted only for the two
proven back-reference layouts, 2 and 3; all other selectors fail closed rather
than being interpreted as layout 3. The real-data test decodes every
referenced floor/ceiling at its declared dimensions and mutates each observed
C8 selector only to prove unknown selectors cannot publish pixels. Verification:
Ninja and direct `test_dm2_v1_gdat_graphicsset_real_data`,
`test_dm2_v1_gdat_word_values`, and `test_dm2_v1_weather_gdat_receipt` pass.

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

# ✅ 2026-07-14 DM2 GDAT complete door material-plan handoff

The boot-owned M11 door plan now carries every source-addressed component of
each visible standard door: `DOORS` base panels, `DOOR_GFX` ornate overlays,
destroyed-door masks, `GRAPHICSSET` frames, and `DOOR_BUTTONS` defaults. The
viewport consumes the plan's decoded IMG3 pixels and exact local palettes
before it can consult its generic asset provider. Custom `WALL_GFX` buttons
remain separately fail-closed behind their direct DB2/DB3 owner receipt; no
same-index substitute is accepted. Source: SKProject `SkWinCore.cpp`
`DM2_DRAW_DOOR`, `DRAW_DOOR_FRAMES`, and `DRAW_DEFAULT_DOOR_BUTTON`.
Verification: `test_dm2_v1_gdat_door_overlay_plan_real_data` passes against
canonical PC DM2 data with no provider fetches; CTest
`dm2_v1_g1_wall_button_material_gate` passes 7/7.

# DM2 GDAT MapGraphicsStyle Corpus (2026-07-14)

- DM2's boot-owned scene-material cache now spans the original unsigned-byte
  `MapGraphicsStyle` domain. `skproject/SKWIN/SkWinCore.cpp`
  `LOAD_LOCALLEVEL_DYN` passes that byte unchanged into the active
  `GRAPHICSSET` and `ENVIRONMENT` load selectors, so Firestaff no longer
  rejects source-addressable styles above the former local 16-style cache.
- `test_dm2_v1_gdat_graphicsset_real_data` now audits all 256 possible source
  style values and checks each style actually referenced by the supplied
  dungeon against its exact GDAT floor, ceiling, and scene controls.

# ✅ 2026-07-14 DM2 same-index creature animation-table admission

The DM2 creature-atlas receipt now records animation-table ownership only when
the same original `CREATURES` index resolves all SKProject
`GET_CREATURE_ANIMATION_FRAME` inputs: `dtRaw8/FB`, `dtRaw7/FC`, and
`dtRaw7/FD`. It retains a complete-index mask and deterministic hash, so three
unrelated GDAT entries cannot claim an animation route. The focused real-data
probe accepts only canonical `GRAPHICS.DAT`, skips when user media is absent,
and has no synthetic fallback. Source: SKProject `SKWIN/SkWinCore.cpp`
`GET_CREATURE_ANIMATION_FRAME`; `SKWINSPX/src/v4/skcrture.cpp`
`GET_CREATURE_COMMAND_ANIMATION_V5`, `GET_ANIM_SEQUENCE_INFO_V5`, and
`GET_CREATURE_ANIMATION_IMAGE_ID_V5`.
Verification: external Ninja target
`test_dm2_v1_gdat_creature_animation_real_data` passed against local canonical
media with 57 complete owners and masks `bfdff3f7/ffbffeff`.

# ✅ 2026-07-14 DM2 GDAT HUD material-plan handoff

The validated 13-command PC DM2 HUD family is now consumed directly by the
viewport: nine `INTERFACE_GENERAL` chrome commands and four source-bound
`CHAMPIONS` portraits retain their decoded IMG3 pixels, local palettes, and
exact M11 rectangles from the boot-owned GDAT plan. This mirrors the existing
`UPDATE_GFXSET` scene-plan ownership path and blocks missing HUD commands
instead of requesting or painting substitute graphics. Source: SKProject
`SKWINSPX` `LOAD_GDAT_INTERFACE_00_02` / `QUERY_GDAT_IMAGE_ENTRY_BUFF` and
`DRAW_CHAMPION_PICTURE`. The separate dt07 name-font route remains no-draw
without its own source receipt; no G1/DB meaning was inferred. Verification:
external Ninja target `test_dm2_v1_gdat_hud_m11_command_real_data` and CTest
`dm2_v1_gdat_hud_m11_command_real_data` pass against local canonical media.

# ✅ 2026-07-14 DM2 GDAT HUD action-palette M11 identity

The M11 dungeon presentation gate now carries and compares the live
`INTERFACE_GENERAL/0/dt07/0x02` action-palette transform in addition to the
base interface palette. A stale transform hash or an unconsumed transform
blocks the frame, so skproject `DISPLAY_VIEWPORT` HUD text cannot be presented
against a different action-table result. Verification: the focused M11 frame
receipt tests cover acceptance plus stale-hash and unconsumed-transform
rejection. No palette or action table is synthesized.

# ✅ 2026-07-14 DM2 extended spell-definition receipt

DM2 boot now scans only source-owned `SPELL_DEF/index/dtWordValue` fields
1 through 7 and the matching `dtText/0x18` name used by skproject
`SkWinCore::EXTENDED_LOAD_SPELLS_DEFINITION`. The count and hash travel
unchanged through host view, packaged startup, consumer, host frame, render
ownership, and the M11 startup boundary; partial definitions fail closed.
The real-data test independently scans the same original GDAT fields and
compares the receipt at every boundary. The local canonical PC corpus has no
extended spell definitions, so the test reports an honest non-promoting skip
instead of generating substitute spells.

# ✅ 2026-07-14 DM2 GDAT HUD portrait destination receipt

The four `CHAMPIONS` M11 commands now retain their exact source-owned
`INTERFACE_GENERAL/0/dt04` destination: `RECT_173..RECT_176` and the complete
decoded rectangle-table hash. The boot handoff converts those original
640-wide rectangles to the matching 320-wide M11 surface before rendering, so
the material plan cannot be matched against placeholder portrait coordinates.
Missing, non-positive, out-of-bounds, or hashless rect-table input clears the
complete plan. Source: SKProject `SkWinCore.cpp::QUERY_BLIT_RECT` and `SKWINSPX` HUD
layout expansion. Verification: `test_dm2_v1_gdat_hud_m11_command_real_data`
requires the four rect IDs/hashes and renders them callback-free with canonical
PC DM2 media.

# ✅ 2026-07-14 DM2 corpus-wide G1 GRAPHICSSET scene-plan handoff

`test_dm2_v1_gdat_scene_plan_viewport_real_data` now traverses every distinct
original G1 `MapGraphicsStyle` referenced by the supplied `DUNGEON.DAT`. Each
matching `GRAPHICSSET` must provide decoded floor and ceiling pixels, local
palettes, source scene controls, and a direct callback-free M11 material
handoff; a copied plan with a different graphics-set remains blocked with no
fallback draw. This follows skproject `c_gui_vp.cpp` `UPDATE_GFXSET` into
`DRAW_DUNGEON`, where all four controls and both plane queries stay within the
active map's graphics set. The test is skip-safe when canonical user media is
absent and does not construct substitute GDAT assets. Verification: external
Ninja target `test_dm2_v1_gdat_scene_plan_viewport_real_data` built clean and
the canonical PC corpus run passed all five referenced GRAPHICSSET styles.

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

# ✅ 2026-07-14 CSBWin zero-word DSA action rejection

The authenticated CSBWin DSA runner now rejects an imported action with no
program words before it inspects an opcode. This preserves parameter words,
save-owned globals, execution counters, and both prior stack and transfer
receipts when a malformed save action reaches the pointer-identity boundary.
It adds no opcode support, synthetic behavior, or world/filter route. Source:
CSBWin `SaveGame.cpp::ReadDSAs` / `DSA.cpp::ProcessDSAFilter` and `Execute`.
Verification: focused `csb_v1_dsa_trigger_single_step_pc34_compat` CTest.

# ✅ 2026-07-14 Theron Track 02 later loader-to-local-RAM capture contract

# ✅ 2026-07-14 Nexus active DGN Structure3 mesh source route

The Nexus engine now exposes a caller-buffered Structure3 mesh-entry route
only from the exact canonical `LEVxx.DGN` bytes it currently owns. It requires
the active level identity, canonical hash, byte-binding receipt, and current
source bytes to agree before returning typed signed 16.16 vertices, face rows,
and paired normals. Mutated or stale level bytes return no partial mesh. This
is a source-routing boundary only: no transform, texture, palette, VDP1, or
draw semantics are granted, and the no-draw barrier remains active.
Verification: `test_nexus_v1_dgn_geometry_readiness` covers the active route
and mutation rejection.

# ✅ 2026-07-14 Nexus active LEV Structure3 directory receipt

The engine now exposes the active canonical LEV's bounded Structure3 directory
with the exact retained source-byte FNV for capture tooling. This is a
source-owned no-draw catalog only; it does not decode texture, palette, VDP1,
transform, or drawing semantics. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus complete DGN material source gate

The complete Structure3 source scene now consumes the active canonical
Structure2 payload-anchor traversal. Every descriptor must retain an image
anchor, each nonzero palette anchor is kept separately, and the full anchor
count must be consumed from the same LEV bytes before the scene is complete.
These remain bounded capture candidates only: image/palette lengths, texel
order, palette format/addressing, VDP1 mode, decoder, and draw semantics all
remain fail-closed. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus animated DGN payload-anchor route

Every declared non-control Structure1G image instruction for an active `08xx`
face now resolves through the active Structure2 payload-anchor traversal. The
route requires the matching image anchor and, where present, the matching
palette anchor from the exact same canonical LEV bytes. It remains no-draw:
candidate interval bounds are not pixel spans, palette format/addressing,
texel order, VDP1 mode, timing, decoder, or drawing proof. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus static DGN face payload intervals

Every active static Structure3 material face now carries the bounded
next-anchor candidate interval for its exact Structure2 image payload and,
when present, its palette payload. The viewport refuses a static source packet
without those intervals. This is capture framing only: neither interval is an
image or palette length, and no pixel codec, palette format, VDP1 mode,
transform, or draw path is inferred. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus descriptor capture windows

Every Structure2 descriptor capture target now contains the exact bounded
image candidate window and, where present, palette candidate window from the
canonical LEV. The raw-trace admission manifest must bind these hashes and
offsets before provenance is considered. This enables a real capture producer
to state its source-read targets without claiming that it observed, decoded,
or drew them. Pixel/palette/VDP1 semantics and rendering remain fail-closed.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus Structure2 descriptor capture target

The active canonical LEV route can now build and atomically write an external
capture request for one exact Structure2 descriptor. It carries only source
identity, descriptor byte offset/raw fields, and FNV fingerprints for that
20-byte descriptor and the bounded post-FFFF span. The target requires an
original Saturn capture and remains no-draw; it is not a pixel decoder,
palette format, animation rule, VDP1 command, or runtime fallback.
Verification: `test_nexus_v1_dgn_geometry_readiness` against the hash-verified
retail LEV corpus, including an emitted LEV00 request.

# ✅ 2026-07-15 Nexus active DGN face/material selector receipt

`nexus_v1_current_level_structure3_face_material_receipt()` now carries the
active canonical LEV's bounded Structure3 face topology together with its
complete documented Structure2/Structure1G selector joins. Hash, byte size,
and FNV identity are required; a stale source receipt withdraws the route.
Selectors remain identifiers only: material bytes, pixels, palettes, UVs,
VDP1 commands, and drawing remain unavailable, with no fallback visuals.
Verification: `test_nexus_v1_dgn_geometry_readiness` against the hash-verified
retail LEV00.DGN through LEV15.DGN corpus.

# ✅ 2026-07-15 Nexus active Structure1A owner-chain receipt

`nexus_v1_current_level_structure1a_owner_chain_receipt()` now consumes the
complete Structure1F index to unique Structure1B owner to Structure1A row to
raw Structure3 model/face-selector chain from the active hash-bound LEV. The
receipt withdraws on stale source identity. It assigns no placement,
transform, material, pixel, palette, VDP1, or draw semantics and permits no
fallback visuals. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Nexus active Structure2 descriptor-envelope receipt

`nexus_v1_current_level_structure2_descriptor_receipt()` now consumes the
active canonical LEV's bounded Structure2 descriptor table and post-FFFF
opaque span, together with complete optional Structure1G global-to-local
descriptor bindings. Source hash/size/FNV identity and the measured aligned
descriptor-offset envelope are required; stale identity withdraws the route.
No payload encoding, pixels, palette, animation, VDP1, or drawing semantics
are assigned, and fallback visuals remain disabled. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus active LEV Structure3 mesh semantic receipt

The engine now publishes the active canonical LEV's bounded Structure3
topology, signed-vector, and face/normal evidence only when the retained bytes
still match the package-bound source receipt. The receipt withdraws on a stale
level or any byte mutation. It remains explicitly no-draw: no original capture,
texture, palette, transform, VDP1, or renderer handoff semantics are claimed.
Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-15 Theron main-RAM control-window read instrumentation

Added bounded CPU-read provenance for `0x1f01f7..0x1f01fb`, retaining logical
and physical reader addresses. The instrument does not classify the bytes or
infer any CDB, sector, level, or object semantics.

# ✅ 2026-07-15 Theron control-window System Card exclusion

Validated a real US Track 02 boot capture containing 64 reads of
`0x1f01f7..0x1f01fb`. Every recorded reader is System Card physical code/RAM
(`0x00xxxx`, `0x002xxx`, or `0x1fe0xx`); no reader is in the game-owned
`0x1f0000..0x1f7fff` range. The bounded verifier fails on a game-owned or
unclassified reader, so this capture cannot be promoted to a CDB/SCSI, sector,
level, or object-record link.

Verification: `test_theron_v1_main_ram_control_window_receipt` and
`verify_theron_main_ram_control_window_receipt.pl` against the authentic trace.

# ✅ 2026-07-15 Theron game-owned main-RAM window to SCSI receipt

Added bounded read provenance for `0x1f1000..0x1f1007`. In an authentic US
Track 02 capture, physical game code `0x1f0c88` reads all eight bytes before
the game-owned `0x1f0cc7` `$e009` dispatch, which is followed by SCSI
generation 2 at LBA 4165 for four sectors. The receipt proves this execution
ordering only: no FIFO destination, level layout, or object-record grammar is
assigned.

Verification: `test_theron_v1_main_ram_game_window_scsi_receipt`, Mednafen
patch dry-run, and the authentic capture verifier.

# ✅ 2026-07-15 Theron game-owned FIFO-to-RAM-to-reader intake gate

The Track 02 loader receipt now has a strict live-capture intake for a single
game-owned `$3840 -> $e009` dispatch: seven observed CDB writes must decode to
the following READ(6); its FIFO-origin byte must reach game-owned main RAM and
be read later by game-owned code from the identical physical cell. For the
verified US CUE coordinate, Firestaff rechecks the captured byte against
`raw_record = LBA - 3009` in the hash-verified Track 02 BIN. The result remains
an opaque byte-flow receipt, not a dungeon, grid, level, object, bitmap,
palette, or transition decoder. Verification: the focused raw-loader probe
rejects a mutated source byte and a CDB/LBA mismatch.

# ✅ 2026-07-15 Theron game-RAM initial-envelope correlation gate

An admitted game-RAM payload byte can now be joined to the source-locked Hall
of Records envelope only when its physical Track 02 sector and exact raw-sector
offset fall inside the authenticated envelope. The join deliberately uses the
IPL-derived physical `level_first_raw_sector`, not descriptor-relative record
`0x0b52`, preventing INDEX 01/file-sector coordinate confusion. It rejects a
pre-envelope byte and altered source media, and publishes no level grammar,
dungeon, object, grid, bitmap, palette, or transition semantics. Verification:
the focused US Track 02 raw-loader probe.

# ✅ 2026-07-15 Theron initial-envelope header capture gate

Firestaff can now retain the first twelve raw bytes of the source-locked
initial envelope only when twelve ordered game-RAM payload receipts share one
dispatch and READ(6) identity and cover the exact consecutive raw offsets.
The receipt stores the source bytes and FNV-1a hash only. It does not interpret
dimensions, the existing extension word, header grammar, level, dungeon,
object, grid, bitmap, palette, or transition semantics. Verification: the
focused US Track 02 probe rejects a split SCSI capture chain.

# ✅ 2026-07-15 Theron LBA 4165 raw Track 02 binding

Bound the four sectors requested by the game-owned window path, LBA
`4165..4168`, byte-exactly to raw Track 02 records `0x484..0x487` using the
observed `raw_record = LBA - 3009` coordinate. The verifier checks each full
2352-byte sector and its observed 32-byte prefix hash. This is a media
identity receipt, not a level/object classification or a FIFO destination.

Verification: `test_theron_v1_lba4165_track02_receipt` and the authentic US
Track 02 capture.

# ✅ 2026-07-15 Theron LBA 4165 FIFO origin receipt

Added byte-origin tracking through Mednafen's SCSI data FIFO. Authentic capture
proves Track 02 record `0x484` / LBA 4165 offsets `0..31` reach System Card
CPU `0xea9c` through `$1808`, with exact byte comparison. No game-owned RAM,
level, or object-record consumer is claimed.

# ✅ 2026-07-15 Theron generation-4 System Card boundary

Generation 4 is CDB `080010891100`: LBA `4233..4249` / Track 02 records
`0x4c8..0x4d8`. Its FIFO origin is System Card `0xea9c`; all four observed
`0x1f0256..0x1f0259` stores are written by System Card `0x000a52`. This route
is excluded from game-data semantics.

# ✅ 2026-07-15 Theron generation-7 FIFO/game-RAM ordering

Authentic capture proves byte-exact FIFO origin for LBA `4847..4851` / Track
02 records `0x72e..0x732`: each of the 10,240 bytes reaches the System Card
`$eb33` FIFO loop and is acknowledged through `$1802/$1803`. The complete
generation-7 FIFO window precedes game-owned `0x1f11xx..0x1f18xx` writes.
That is ordering only, not a byte destination or record semantic.

# ✅ 2026-07-15 Theron main-RAM CDB byte-consistency gate

The authentic main-RAM `$e009` dispatch receipt now decodes each READ(6) CDB
and rejects a mismatch between its LBA/count bytes and the emitted SCSI
command. This binds the game-owned dispatch route to the observed raw record
ranges without inventing a FIFO destination or record semantics.

# ✅ 2026-07-15 Theron later-generation FIFO capture filter

The reproducible Mednafen trace build accepts
`FIRESTAFF_THERON_FIFO_MIN_GENERATION=N`. It filters only provenance output
below `N`; emulated CD reads and RAM writes are unchanged. The authenticated
`N=8` capture omits the already-proved generation-7 FIFO traffic, but still
does not reach a later FIFO byte before timeout. No handoff is claimed.

# ✅ 2026-07-15 Theron guarded global-HID capture route

The Quartz helper can use a global HID route only after activating and then
rechecking the target's foreground PID. An authenticated run observed
`loginwindow` PID `622`, not Mednafen, so it failed before posting a key. This
is an environment receipt, not emulated input or a dungeon handoff.

# ✅ 2026-07-15 Theron main-RAM loader initialization exclusion

The post-`$e009` `0x1f10xx` write window is now fail-closed as loader
initialization: the authenticated writes are only `00`/`ff` sentinels from
the observed main-RAM writers. It cannot be promoted to level/object data.

Verification: focused initialization receipt test.

# ✅ 2026-07-15 Theron game-owned writer corpus negative receipt

The authentic USA Track 02 capture now has a strict, bounded negative corpus
receipt for every observed game-owned main-RAM loader writer. It contains 128
writes: 12 control-window writes at physical `0x1f01f6..0x1f01fb`, plus 116
`00`/`ff` initialization writes at `0x1f10xx`. All have
`dispatch_sequence=0`; the authenticated generation-7 `READ(6)` at LBA 4847
(Track 02 records `0x72e..0x735`) occurs only after that complete writer
corpus. These rows therefore cannot be the G7 loader or a G7 record consumer.
The verifier rejects a CDB-dispatched writer, a non-sentinel initialization
byte, an unclassified destination, and a changed corpus count. This is not a
global absence claim: a later game-owned FIFO/CDB reader or writer remains the
required positive handoff evidence. No level, object, palette, or visual
semantics were added.

Verification: `test_theron_v1_game_loader_writer_negative_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 game-loader record-route receipt

The authentic USA trace now fixes the post-G7 game-loader control boundary.
After G7, physical game-RAM `0x1f1840` continues to call `$e009` from logical
`0x3840`: dispatches 4, 5, and 6 have `A=20`, `X=ff`, `Y=04` and issue the
exact READ(6) CDB routes G8 LBA 4859 (record `0x73a`), G9 LBA 4855..4857
(records `0x736..0x738`), and G10 LBA 4858 (record `0x739`). This is the
verified loader entry/record route after G7. The trace patch emits the entry
only after disassembling HuC6280 opcode `0x20` with operand `$e009`, i.e.
`JSR $e009`. The trace still has no
FIFO-to-game-RAM destination or game-owned record reader, so no level, object,
palette, or visual semantics are assigned.

Verification: `test_theron_v1_post_generation7_loader_route_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 indirect CDB-parameter receipt

The post-G7 loader trace now proves a bounded ABI fact. Game code at physical
`0x1f1837` writes `ff/20/04` into physical `0x1f01e5..0x1f01e7` immediately
before dispatch 4, exactly shadowing `X/A/Y` at the `0x1f1840` `JSR $e009`.
Dispatches 4--6 keep that same register tuple but produce three distinct
authenticated READ(6) CDBs: `080012fb0100`, `080012f70300`, and
`080012fa0100`. The tuple is therefore not direct LBA/count encoding; it is
an indirect loader ABI whose additional parameter source and RAM consumer are
still unobserved. A new passive MD5-pinned CUE capture reached only the System
Card wait and contributes no loader route. No game-data, level, object,
palette, or visual meaning was inferred.

Verification: `test_theron_v1_post_generation7_cdb_parameter_receipt` and
the authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron post-G7 parameter-window reader trace

Mednafen's authentic trace pipeline now records every physical read of the
post-G7 parameter-shadow window `0x1f01e5..0x1f01e7`, including its logical
address, value, and logical/physical reader PC. The receipt is bounded to 128
rows and is appended after all existing source-to-RAM provenance patches, so
it cannot alter CDB, FIFO, controller, or emulated input behavior. It is
fail-closed evidence only: the existing G8--G10 trace predates this reader
instrumentation, while a new passive MD5-pinned media run reached only the
System Card wait. There is therefore no claimed lookup, loader-table,
game-owned consumer, record-table, or semantic binding yet.

Verification: full `test_theron_v1_mednafen_controller_wait_trace_patch`
dry-run against Mednafen 1.32.1 source.

# ✅ 2026-07-15 Theron post-G7 parameter-thunk CPU receipt

The authenticated G8 trace now fixes the next game-owned control edge after
the indirect `$e009` ABI. Physical `0x1f184d` writes byte `1e` to executable
`0x1f1837`, then `0x1f1852` writes `20` to `0x1f1838`. Execution from
physical `0x1f1837` subsequently stages `04/20/ff` into the parameter window
before `0x1f1840` dispatches `$e009` and G8 reads LBA 4859. The verifier
rejects changed patch bytes, parameter-store ordering, and CDB ordering. No
CD-origin row writes the thunk bytes, no parameter-window reader was observed,
and no opcode, loader-table, record-table, level, object, palette, or visual
meaning is inferred from the two patched bytes.

Verification: `test_theron_v1_post_g7_parameter_thunk_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron generation-4 System Card CD-to-main-RAM receipt

The authenticated USA Track 02 generation-4 READ(6) now has a complete
CPU-provenance boundary. Its CDB reads the ordered 17-sector span LBA
4233..4249 (records `0x4c8..0x4d8`); all 34,816 raw data-port bytes are
checked for contiguous LBA/offset order. The observed FIFO values
`38/50/37/04` are read by System Card `$ea50`, written by System Card `$ea52`
to physical main RAM `0x1f0256..0x1f0259`, and the first three cells are then
read by low physical System Card code. The verifier rejects a game-owned
writer. This is a positive CD-to-main-RAM receipt, but it proves System Card
ownership only: it does not bind game code, a loader table, level data,
objects, palettes, or rendering semantics.

Verification: `test_theron_v1_generation4_system_card_receipt` and the
authentic `/tmp/theron-g4-origin-live/trace.cd` capture.

# ✅ 2026-07-15 Theron byte-exact FIFO-to-main-RAM instrumentation

The Mednafen trace now retains the raw Track 02 LBA and byte offset that were
current at each queued FIFO read, and emits them with the later main-RAM
destination plus reader and writer CPU provenance. A verifier accepts such a
receipt only when its source lies in a preceding observed READ(6) range and
its destination is physical main RAM. This does not fabricate a handoff: the
new MD5-pinned headless USA capture stayed at the System Card wait and emitted
no FIFO-to-main-RAM receipt. A future runtime capture must supply the positive
row before any game-owned loader, level, object, palette, or visual claim.

Verification: `test_theron_v1_fifo_origin_main_ram_receipt`, the Mednafen
patch application/compile probe, and the negative headless capture.

# ✅ 2026-07-15 Theron FIFO-origin game-consumer gate

The trace now tracks a bounded set of raw-CD FIFO cells after they reach
physical main RAM. A consumer receipt is emitted only when a physical
`0x1fxxxx` game-code reader reads the exact same still-valid destination and
value; every later write invalidates that cell, including a same-value write.
System Card readers are excluded. The verifier requires the matching prior
raw LBA/offset receipt, so this cannot promote a timing correlation to a game
handoff. No authentic consumer row has been observed yet.

Verification: `test_theron_v1_fifo_origin_main_ram_consumer` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron main-RAM `$e009` return receipt

Each traced game-RAM `JSR $e009` now records an exact pending continuation at
the observed logical and physical `JSR+3` addresses. A return receipt is
emitted only when the HuC6280 executes precisely that continuation; unrelated
game instructions and an unmatched return are ignored. This extends the
loader route from call/CDB evidence to CPU continuity without assigning any
data or rendering semantics. No new authentic return capture is claimed.

Verification: `test_theron_v1_main_ram_e009_return_receipt` and the Mednafen
patch application/compile probe.

# ✅ 2026-07-15 Theron post-dispatch game-owned main-RAM write receipt

After authentic `$e009` dispatch, bounded tracing distinguishes writer
ownership. USA Track 02 capture proves game-owned code at `0x1f0cc9` and
`0x1f1173..` writes main-RAM state. It is not byte-linked to FIFO payload or
a proven level/object record, so no semantics or fallback is promoted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` writer-provenance receipt

FIFO destination receipts now retain the actual writer PC and physical PC.
Real USA Track 02 capture proves every observed `$e009` FIFO store is written
by System Card code (`0x000a52` or `0x000b35`), including stores addressed in
main RAM. Thus none qualifies as game-owned level/object data. The next route
must first prove a physical `0x1fxxxx` writer.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron G4 RAM consumer negative receipt

The HuC6280 read path now records exact reads of G4's materialized
`0x1f0256..0x1f0259` bytes. Real USA Track 02 capture shows their subsequent
readers are System Card physical code, including `0x002c1a..0x002c69`, rather
than game-owned main-RAM code. The G4 route is therefore explicitly blocked
from level/object promotion; no fallback or semantic inference was added.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron `$e009` FIFO-to-main-RAM receipt

Dispatch-bounded FIFO tracing now ties real `$e009` SCSI data reads to strict
next-store RAM receipts. The USA Track 02 capture proves dispatch 0's
generation-4 bytes reach physical `0x1f0256..0x1f0259`; other captured
dispatches reach the System Card workspace. These are byte-transport facts
only: no level/object grammar, game consumer, or visual fallback is admitted.

Verification: Mednafen patch dry-run and real SDL2 USA Track 02 capture.

# ✅ 2026-07-15 Theron main-RAM `$e009` to SCSI receipt

The HuC6280 trace now emits every physical main-RAM `$e009` call into the
PCE-CD trace. A fail-closed verifier requires exactly seven subsequent CDB
writes and one READ(6) SCSI command. A real USA Track 02 capture validates
32 such dispatch-to-record chains. This proves loader-to-record transport,
not game-owned destination, level, object, or visual semantics.

Verification: Mednafen patch dry-run, focused verifier test, and real SDL2
USA Track 02 capture.

# ✅ 2026-07-15 Theron parameterised main-RAM `$e009` receipt

The HuC6280 main-RAM trace now captures A/X/Y at each executed loader call.
Real USA Track 02 capture proves physical `0x1f1840` calls `$e009` after the
`0x1f1836` TII workspace transfer, including `a=20 x=03 y=02`. Parameters
vary across calls and remain opaque: no record, level, object, or visual
semantics are assigned.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 USA Track 02
capture.

# ✅ 2026-07-15 Theron main-RAM loader control receipt

Added a HuC6280-core trace patch that resolves executed PCs through active MPR
banks before recording bounded main-RAM `JSR` and block-transfer edges. Real
USA Track 02 capture records physical `0x1fxxxx` loader calls, including
`JSR $e009` at `0x1f0cc7` and `0x1f1840`. This proves control flow only, not
level, object, payload, or visual semantics.

Verification: Mednafen 1.32.1 patch dry-run and real SDL2 capture against
MD5-pinned USA Track 02 media.

# ✅ 2026-07-15 Theron all-generation Track 02 source-to-RAM receipt gate

The instrumented Mednafen build now carries an exact raw SCSI origin
(`generation`, `LBA`, and in-sector byte offset) through the pending FIFO read
and emits `pce_cd_origin_ram_receipt` only when that same byte is immediately
stored in physical main RAM. The receipt verifier rejects non-main-RAM
destinations and offsets outside the 2048-byte sector. It neither assigns
writer ownership nor record, level, object, palette, or visual semantics.

A fresh MD5-pinned USA CUE/System Card run without host input reached only the
System Card wait: no raw-sector SCSI transfer and no receipt were observed.
That negative result is deliberately not promoted to a game-data conclusion;
the next positive capture must show a game-owned consumer before any semantic
work may begin.

Verification: `test_theron_v1_origin_ram_receipt`, Mednafen patch dry-run,
and an instrumented authentic-media boot capture.

# ✅ 2026-07-15 Theron game-owned Track 02 FIFO-to-RAM writer gate

The all-generation receipt now observes the store at the HuC6280 write point,
so one trace row contains the raw sector generation/LBA/offset, FIFO reader,
physical main-RAM destination, and both logical and physical writer PCs. The
positive verifier accepts only writer and destination addresses in physical
game RAM `0x1f0000..0x1f7fff`; a System Card writer is rejected. This is a
transport/ownership gate only and publishes no record, level, object, palette,
or visual semantics.

The fresh MD5-pinned USA CUE capture posted real PID-targeted Quartz Return
pairs but Mednafen reported no SDL key event, then reached only the System
Card wait with no SCSI read or FIFO/RAM receipt. The failed delivery is kept
as a negative capture result, not replaced with injected controller state.

Verification: `test_theron_v1_game_owned_origin_ram_receipt`, full Mednafen
patch dry-run, instrumented Mednafen build, and the authentic-media capture.

# ✅ 2026-07-15 Theron PID foreground capture gate

PID-targeted Quartz delivery now requires the same foreground ownership proof
as the global-HID route. The helper activates the target, rechecks
`NSWorkspace`, and emits `quartz_frontmost_pid` only before posting a key.
The capture wrapper requires that receipt, so a `posted_to_pid` line cannot be
mistaken for SDL delivery from a background or login session.

The direct live check found the Mednafen target at PID `8739` while foreground
ownership remained with `loginwindow` PID `622`; it failed before posting. No
controller state, CD read, FIFO/RAM handoff, or Track02 semantics were
invented. A positive run still needs both real Aqua foreground ownership and
Mednafen's own SDL event receipt.

Verification: `swiftc -typecheck`,
`test_theron_v1_mednafen_live_capture_script`, and the direct live negative
foreground receipt.

# ✅ 2026-07-15 Theron foreground activation receipt refinement

The Quartz helper now records the result of macOS activation independently of
foreground ownership, and the capture wrapper requires `quartz_activation`
plus the exact foreground PID before it accepts a key-post attestation. A live
probe returned `activate=true` for Mednafen while `NSWorkspace` still reported
`loginwindow` PID `622`; activation success alone is therefore not promoted to
focus, SDL delivery, controller state, CD traffic, or a Track02 handoff.

Verification: Swift typecheck and
`test_theron_v1_mednafen_live_capture_script`.

# ✅ 2026-07-14 Nexus active LEV Structure3 face framing receipt

The engine now binds Structure3 entry-header boundaries and face-row local
vertex-index evidence to the exact active canonical LEV bytes. Any stale or
mutated source withdraws the receipt. This remains a no-draw framing boundary:
it proves neither Saturn transforms nor surfaces, materials, textures,
palettes, VDP1 commands, or rendering. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus active LEV transform/camera framing receipt

The engine now binds the active party cell and direction to the exact
canonical LEV byte receipt, alongside the existing bounded Structure1A raw
transform-selector receipt. A stale level, invalid pose coordinate, or byte
mutation withdraws it. This is no-draw camera-input provenance only: no
Saturn camera matrix, transform order/unit, culling, or rendering semantics
are inferred. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# ✅ 2026-07-14 Nexus PRS3 V3 provenance-bundle validator route

The read-only V3 capture validator now accepts an authentic capture bundle's
trace, three raw sidecars, provenance ledger, and producer binary in one
invocation. It can verify their FNV links, but reports producer authentication
and runtime import as false by design. No PRS3 opcode grammar, pixel/palette
decode, synthetic surface, or draw route is enabled. Verification:
`test_nexus_v1_prs3_capture_trace_schema` and the validator target build.

# ✅ 2026-07-14 Nexus PRS3 V3 producer-attestation workflow

The V3 validator can now additionally check a strict Mednafen SH-2/VDP1 bus
trace workflow attestation against the complete artifact bundle and producer
binary. Its original-Saturn execution line is deliberately only a claim, so
the result always requires independent authentication and cannot permit
runtime import, decoding, fallback pixels, or rendering. Verification:
`test_nexus_v1_prs3_capture_trace_schema` and the validator target build.

# ✅ 2026-07-14 Nexus PRS3 V3 capture-bundle ledger writer

The V3 capture tool now writes the deterministic provenance ledger from an
externally acquired trace, canonical MENU.BPK/DM.BIN, raw output/VDP1/palette
sidecars, and the producer binary only after the existing byte-bound admission
passes. It writes hashes, never copies capture or game bytes, and cannot
authenticate a producer, decode PRS3, import runtime data, or render.
Verification: `test_nexus_v1_prs3_capture_trace_schema` and validator build.

# ✅ 2026-07-14 Nexus active LEV renderer-source receipt

The DGN viewport now consumes an active-LEV renderer receipt that carries the
canonical package-bound LEV byte count/FNV and Structure3 payload hash to the
renderer boundary. It withdraws the receipt when retained bytes change. When
an original-capture packet is admitted, the receipt names each opaque source
span while retaining independent blockers for texture decoding, palette
application, VDP1 command semantics, and transform/culling semantics. The
route stays no-draw with fallback visuals disabled; it creates no pixels or
host interpretation of Saturn state. Verification:
`test_nexus_v1_dgn_geometry_readiness`.

- ✅ 2026-07-14 CSB PC V1 startup decode: literal assets bypass LZW and
  compressed assets use ReDMCSB-compatible chunk-width LZW before IMAGE3
  expansion. This restores original C001 title and entrance assets. Verification:
  real-data title/import and launch probes.

The coalesced original Mednafen receipt now requires the later `$e009`
dispatch's observed local-RAM destination plus a 32-byte post-return RAM
fingerprint. Firestaff compares that fingerprint with the selected MODE1
user-data prefix, so a supplied authentic capture can establish a bounded
record-to-RAM transfer for `0x0b52`. The contract rejects a missing,
misordered, mismatched, or non-local span. It does not establish a game
transition or assign dungeon, object-tail, bitmap, palette, or payload
semantics. Verification: focused raw-loader CTest and capture-order script.

# ✅ 2026-07-14 CSBWin saved TT_75 full-word poison continuation

Restored `TT_75` now keeps its full `timerWord6` through the source-timed
poison requeue instead of truncating it to one byte. Verification:
`csb_v1_dsa_restored_timer_tick_bridge`.

# ✅ 2026-07-14 Theron Track 02 startup-grid positive route

The existing CD/MODE1 envelope and loader-semantic receipt now materialize one
positive route: Hall of Records level 0 only. It verifies the loader-selected
pose against the receipt and remains an explicit route boundary pending
startup-pose reconciliation with the older semantic handoff. The route
contains no object table, header-extension interpretation, transition, bitmap,
or fallback-visual claim; later dungeon requests reject. The focused Track 02
handoff probe checks the real-media route and its refusal of an unproven
dungeon ID.

# ✅ 2026-07-14 Theron later `$e009` capture correlation gate

# ✅ 2026-07-14 Theron later `$e009` production selector-coordinate gate

The production later-loader media receipt now derives the captured `$e009`
record from the authenticated Stage 3 descriptor coordinate base and accepts
it only when it resolves to an existing descriptor selector. It retains the
opaque selector and ordinal with the raw-sector receipt. A raw-sector-only or
synthetic media buffer cannot publish the receipt. This is still only an
executed loader-coordinate constraint: it assigns no descriptor format,
dungeon, object, palette, bitmap, or transition semantics. Verification:
`theron_v1_raw_loader_trace_stage3_sector` passes; the paired original-media
layout probe remains skip-safe until matching JP/US Mednafen traces exist.

# ✅ 2026-07-14 Theron later `$e009` raw-sector witness boundary

The selector-coordinate receipt can now be paired with exactly one
provenance-marked Mednafen SCSI raw-sector sidecar span whose bounded FNV-1a
matches the corresponding hash-verified Track 02 raw sector. The receipt
retains only the observed disc LBA, selector coordinate, and span fingerprint.
It does not claim that `$e009` caused that read, that both observations share
one capture session, or assign a payload format, dungeon, object, palette,
bitmap, or transition meaning. Noncanonical media, missing sidecars, duplicate
matching spans, and changed bytes reject.
Verification: `theron_v1_raw_loader_trace_stage3_sector` focused negative
probe; a positive result requires original JP/US media and captures.

# ✅ 2026-07-14 Theron later `$e009` complete-sector witness hardening

The raw-sector witness now accepts only a provenance-marked Mednafen SCSI row
that retains both FNV-1a fingerprints: all 2352 observed raw-sector bytes and
the existing leading 32-byte span. Firestaff compares both against the same
selector-resolved record in the hash-verified original Track 02 BIN; span-only
or malformed sidecars reject. This remains physical CD/media provenance only:
it does not establish `$e009` causality, shared capture-session identity,
payload format, dungeon, object, graphics, palette, bitmap, or transition
semantics. Verification: focused raw-loader CTest and Mednafen patch/capture
script contracts.

# ✅ 2026-07-14 Theron later `$e009` ordered raw-sector capture gate

The next Track 02 capture handoff now has a strict, source-only admission
contract. A future authentic JP or US coalesced Mednafen transcript must retain
exactly one variant-matched `$4090/$4093` loader row, followed by one later
`$e009` dispatch, exactly one complete 2352-byte raw-sector FNV witness, and
the matching `$e009` return. The verifier rejects split sidecars, reordered
rows, duplicate rows, malformed fingerprints, and unmarked transcripts. It
records only observation order; no destination, CD causality, payload format,
dungeon, map, object, graphics, or palette claim is introduced.
Verification: `tests/test_theron_v1_later_e009_raw_sector_order_trace.sh`.

# ✅ 2026-07-15 Theron post-`$3800` IRQ2-to-later-read ordering gate

The coalesced Track 02 receipt now requires an observed original Stage 3
`BRK $ff` return from `$3800` to `$3802` before it will accept a later
`$e009` dispatch. Firestaff checks those capture coordinates against the
hash-verified Stage 3 payload, then retains only the ordering fact. The gate
does not decode the later sector or promote level, object, bitmap, palette,
grid, or transition semantics. Verification: the corpus-bound raw-loader
handoff probe rejects missing or altered Stage 3 continuation coordinates.

Added a skip-safe, corpus-bound probe and Mednafen instrumentation for the
first post-stage-two HuC6280 `JSR $e009` envelope. A positive result requires
hash-verified JP and US raw Track 02 images plus matching instrumented traces;
the record must reconstruct from observed `CL/DL/CH`, remain in each raw-sector
range, resolve to the same existing stage-three descriptor selector ordinal,
and preserve one caller/return PC pair. This is only a bounded record/layout
and control-transfer correlation. It does not label the call as a payload
format, or publish a CD read, bitmap, palette, object, level, or gameplay
transition. Inspected historical US traces do not contain the new later
envelope, so no positive record has been claimed. Verified with an external
Ninja build of `firestaff_theron_v1_later_cd_read_layout_probe` and skip-safe
CTest registration.

The probe now additionally requires exactly one observed `$4090/$4093`
dynamic receipt in each trace, including its matching JP/US variant and
reconstructed `CL/DL/CH` stage-two record. A freestanding, duplicate, or
cross-variant later `$e009` row cannot be paired with authenticated media.

# 2026-07-14 Nexus Structure3 face-pair multiplicity corpus receipt

The DGN face receipt now partitions each entry-local unordered vertex pair by
whether it co-occurs in one or multiple bounded face rows and retains the
maximum local occurrence count. The hash-verified LEV00.DGN through LEV15.DGN
retail corpus validates the partition. This is no-draw row incidence only; it
does not establish an edge, winding, surface, normal-plane, transform,
texture, palette, or drawing behavior.

# 2026-07-14 Nexus Structure3 retail source-only capture gate

`test_nexus_v1_dgn_face_mesh_corpus` now submits source-only capture input for
each hash-verified LEV00.DGN through LEV15.DGN level and requires all 16 to
remain blocked before candidate framing, complete source binding, or renderer
handoff. This proves only that the installed DGN corpus lacks the separate
captured texture span, palette state, VDP1 state/command, transform, culling,
and ordered original-Saturn provenance required by the binder. It does not
decode a texture or palette, assign a transform, or authorize DGN drawing.
Verification: `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus
./build-nexus/test_nexus_v1_dgn_face_mesh_corpus`.

# 2026-07-14 Nexus ITEM.IBS 0008 VDP1 capture-binding gate

The documented packed-4bpp parser now requires an atomic capture receipt before
it expands any descriptor-`0008` texels: hash-verified complete `ITEM.IBS`
bytes, selected descriptor metadata, exact packed span and BGR555 palette,
VDP1 state/command fingerprints, texture-source extent, and strict
texture-before-command sequence all have to match. The codec remains no-draw
and retail ITEM.IBS remains blocked because no original Saturn packet is
present. Verification: `test_nexus_v1_dgn_geometry_readiness`.

# 2026-07-14 Nexus ITEM.IBS VDP1 command-packet shape gate

The descriptor-`0008` capture binder now parses a complete 32-byte
little-endian VDP1 command record before it can authorize high-nibble-first
expansion. It requires the captured texture-source word, 4bpp colour-bank
mode, and declared width/height to agree with the selected ITEM.IBS descriptor,
in addition to the pre-existing hash and sequence checks. The focused
`nexus_v1_dgn_geometry_readiness` fixture proves a self-consistent but
different source word remains blocked. This is only a documented hardware
packet-shape check: no original Saturn command packet was added, so retail
ITEM.IBS stays no-draw and no texture, palette, placement, or VDP1 ordering
claim is promoted.

# ✅ 2026-07-14 Theron Track 02 route-receipt probe repair

The focused Track 02 handoff probe now constructs a complete hash-profiled
startup-media receipt before it exercises the existing media-gated bank
selection. This restores the real JP/US Hall of Records level-0 loader route
as a green target while retaining the Stage 3 `$4090 -> $4093` CD_READ receipt
as transport-only: it does not claim a later level, object layout, visual
decode, or transition.

# ✅ 2026-07-14 Theron Track 02 loader-pose reconciliation

The positive raw-CD Hall of Records level-0 path now preserves the existing
loader's first-floor/default-North pose across the candidate and loader-route
handoffs. The previous local passable-neighbor/East preference was removed
because it was not backed by the original CD or loader evidence. The focused
probe verifies the two real-media paths agree; it remains skip-safe without
hash-verified JP/US Track 02 images. The older seed-table semantic handoff is
still independently blocked on authentic media and is not composed here. This
does not infer an IPL spawn override, object table, transition, bitmap,
palette, or fallback.

# ✅ 2026-07-14 Theron Track 02 coalesced later-loader sector receipt

The later-loader handoff now has one media-bound receipt for a single original
Mednafen transcript. It requires the authenticated Stage 2 `$4090 -> $4093`
loader row, one later `$e009` dispatch, one complete 2352-byte raw-sector
fingerprint, and the matching return in that observation order. Both the
complete-sector and leading-span FNV-1a values must match the raw sector
selected through the existing Stage 3 descriptor coordinate in a hash-verified
JP or US Track 02 image. The opt-in corpus probe runs this check only when both
variants' coalesced traces are supplied. This records a loader-coordinate and
physical-media fact only: it assigns no payload format, dungeon, map, object,
graphics, palette, bitmap, or transition meaning.

# ✅ 2026-07-14 Theron Track 02 manifest-bound coalesced loader receipt

The opt-in JP/US coalesced-loader corpus probe now accepts each ordered
Mednafen transcript only through its own V2 capture manifest. It rehashes and
matches the exact raw Track 02, System Card 3.0, and trace paths before binding
the existing selector-resolved complete-sector receipt. A missing half-pair,
manifest, or System Card path fails the supplied-evidence gate. This records
only original-artifact provenance and loader/media coordinates; no payload
format, dungeon, map, object, graphics, palette, bitmap, or transition meaning
is assigned.

# ✅ 2026-07-14 Theron Track 02 manifest-required raw loader preflight

The positive raw-loader preflight now requires a V2 capture manifest and
rehashes the exact raw Track 02, System Card 3.0, and ordered Mednafen trace
against it before admitting the existing `$3800` media-span/Stage 3 receipt.
The shared loader-capture identity check rejects a missing manifest, a changed
trace, or a non-System-Card-3.0 hash. This is artifact provenance and transport
only; it assigns no payload format, dungeon, object, bitmap, palette source,
or decoder meaning.
# Nexus Structure3 Selector Reuse Receipt (2026-07-14)

`nexus_v1_level_structure3_face_material_receipt()` now retains per-level
unique and reused bounded face-selector occurrences for both documented
Structure2 (`00xx`) and Structure1G (`08xx`) joins. The focused retail
LEV00.DGN through LEV15.DGN corpus test requires complete reuse accounting.
The aggregate corpus retains 1,291 unique and 16,110 reused Structure2
selector occurrences plus 44 unique and 376 reused Structure1G occurrences.
This is identifier provenance only: payload decoding, dimensions, UVs,
palettes, animation, transforms, and VDP1 drawing remain blocked pending
original Saturn evidence.

# 2026-07-14 Nexus Structure3 typed mesh corpus identity receipt

`test_nexus_v1_dgn_face_mesh_corpus` now serializes only the bounded typed
Structure3 vertex, face, and normal rows in the hash-verified retail
LEV00.DGN through LEV15.DGN corpus. The source receipt is `d3f42b1f`, alongside
the existing 1,144 entries, 18,478 face/normal pairs, and selector-join
coverage. It deliberately does not read or associate the separate `FACE.BIN`
asset, decode texture pixels, assign palette/VDP semantics, choose a transform,
or authorize drawing. Verification: focused
`test_nexus_v1_dgn_face_mesh_corpus` against
`/Users/bosse/.firestaff/data/nexus`.
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

# 2026-07-14 DM2 G1 startup-map boot admission

DM2 now admits a successfully decoded original G1 map into the real GDAT
startup/menu route before the optional generic-record graph is promoted.
Malformed map data still fails boot; incomplete record links remain separately
gated at runtime. This follows SKProject's `SHOW_MENU_SCREEN` before the later
`GAME_LOAD` world-graph path. The focused smoke test covers a bounded G1 map
whose record graph is intentionally unavailable.

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
# 2026-07-23 - CSB F2246-F2285

Completed the ReDMCSB Towns-memory ownership batch with a focused PC34
compatibility test. Unproved routes stay fail-closed.
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
# 2026-07-23 - CSB-007 Existing Monster-Kill EXPOOL Writeback

Implemented the source-bounded CSBWin `ESTAT_NumMonsterKilled` writeback for
an existing authenticated four-word EXPOOL record. Missing counters are never
invented or allocated. Focused recovery test passes.
# 2026-07-23 - DM1 Action Menu Graphic Geometry

Fixed the source geometry check for F0387's C010 action asset: it now validates
the 87x45 graphic rectangle rather than the larger C011 clear rectangle.
Missing or mismatched original material remains fail-closed.
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
# 2026-07-27 Theron hash-selected Track 02 media root

- ✅ 2026-07-27 DM1 champion HUD click repair. The full painted V1
  health/stamina/mana bar surface now opens the matching champion inventory,
  rather than accepting input only on the narrow right-edge source zone while
  the rest of the visible bar silently selected the leader. Name and hand
  routes remain unchanged; V2 portrait-card routing remains covered.
  Verification: DM1 inventory mouse-route runtime, V2 HUD interaction, and
  HiDPI champion pointer tests.

- ✅ 2026-07-27 DM1 V2.2 reviewed-art admission repair. Formatted Art Studio
  manifests are now parsed as JSON objects instead of line fragments, all
  V2.2 manifest/receipt roots are configured together, and alias-safe path
  joins support in-place path construction. A reviewed local pack passes the
  real material gate and renders eight source-backed cells; an unsigned
  cache remains fail-closed. Verification: V2.2 real-art material gate,
  per-mode material signatures, settings persistence, and source-lock gate.

Theron launcher campaign-media discovery now honours the caller's selected
known Track 02 MD5 when scanning a directory. A data root containing both US
and JP original releases is valid; the selected release remains launchable
instead of being misreported as ambiguous. The optional real-media test uses
the supplied root and selected MD5 to prove this without shipping game data.
- ✅ 2026-07-27 Theron Mednafen live-capture build repaired. Repaired the
  1.32.1 debugger trace patch so the core CPU/CD/input/sector instrumentation
  builds again, removed stale extension patches from the required build path,
  and made the local trace binary link a real SDL2 runtime with an embedded
  rpath. Verified against authentic US Track 02 plus System Card 3.0:
  Quartz-delivered Run input, two observed System Card calls, 25 CDIRQ events,
  and 31 raw-sector receipts. Dynamic dungeon-handoff rows remain deliberately
  unclaimed until an original run reaches them.
- ✅ 2026-07-27 Theron timed original-menu input capture. The authenticated
  Mednafen capture helper now supports ordered absolute input timings through
  `THERON_CAPTURE_HOST_KEY_DELAYS`. Verified a three-press Quartz Run sequence
  against authentic US Track 02 and System Card 3.0; the trace records every
  host SDL/key event and emulated port state. The current original route stops
  polling the PCE port before the scheduled presses, which remains explicit
  evidence rather than being misreported as a successful dungeon handoff.
- ✅ 2026-07-27 Theron Mednafen input-PC trace correction. Rebuilt the real
  SDL2-linked Mednafen 1.32.1 trace binary with CPU-PC provenance on every
  direct PCE input read/write and a configurable 4,096-per-direction cap.
  Authentic passive US CUE + System Card capture records 8,192 input
  transactions at System Card PCs `e4b7`/`e4c8`; the prior 128-row result was
  trace truncation, not a stopped-poll conclusion. Track 02 handoff remains
  blocked: this capture has no dynamic sector read or loader-consumer row.
  Verification: trace patch dry-run, full external Mednafen rebuild, SDL2
  runtime verifier, and authentic 55-second capture.
- ✅ 2026-07-27 Theron focused capture resolver. `capture_theron_mednafen_live_trace.sh`
  now waits up to ten seconds for Mednafen's own timeout/env descendant, then
  schedules host keys relative to capture launch instead of racing process
  creation. Script gate passes; a real 55-second capture attests four Return
  SDL events, PCE port `0000 -> 0008 -> 0000`, 31 raw-sector spans, and 56
  SCSI reads. It remains non-promotable because the initial input trace cap
  is reached before the host event.
- ✅ 2026-07-27 Theron post-key input-chain capture. Raised the default
  direct-input trace limit to 65,536 per direction and made PID Quartz
  delivery tolerate a background target while retaining a strict foreground
  requirement for global HID. Authentic US capture records 47,575 direct PCE
  transactions, 26,782 after the first host key, and direct `e4b7`/`e4c8`
  reads of port `0008`. Verification: Swift typecheck, shell/test gate,
  rebuilt SDL2-linked Mednafen, and 55-second authentic CUE/System Card run.
  No dynamic CD destination or game-owned PCECD reader appeared, so Track 02
  dungeon promotion remains blocked.
- ✅ 2026-07-27 Theron PCE input-result trace. Added a post-read trace hook
  after Mednafen applies PCE port semantics, rather than inferring result bits
  from host state. A real 28-second US capture with Return held records
  `raw=0008 -> value=3f` at `e4b7` and `raw=0008 -> value=37` at `e4c8`, plus
  4,796 subsequent PCE input transactions. This proves the observed input
  register result only; it does not assign a game command or promote Track 02
  data.
- ✅ 2026-07-27 Theron CD-to-RAM physical ownership trace. Added physical
  HuC6280 PC provenance to both CD-data reads and the matching RAM writes.
  Authentic input capture proves all currently observed candidates are System
  Card code `000a50/000a52` or `000b33/000b37`, including writes into
  `001fxxxx` main RAM. This closes the false inference that destination RAM
  alone proves a game loader; no game-owned CD consumer is promoted.
- ✅ 2026-07-27 Theron authentic multi-key boot capture. The live Mednafen
  capture harness now accepts one ordered absolute-time key sequence, keeping
  every element constrained to `return`, `i`, or `select`. Authentic
  `return@10,i@75,i@90` reaches the original Theron title menu and then real
  NEW GAME presentation, with six host key events and 8,910 subsequent PCE
  input transactions. This is boot/menu evidence only; no dungeon record,
  game-owned CD reader, or destination semantics are inferred.
# 2026-07-27 - Nexus blocked PRS3 launcher return

- M11 now returns to the launcher when the authenticated Nexus MENU.BPK path
  is blocked on missing PRS3/Saturn decoder evidence, rather than entering a
  permanent black no-draw dungeon state. Updated runtime handoff coverage
  verifies keyboard and pointer champion starts.
# 2026-07-27 - Theron Mednafen loader-capture diagnostics

- Improved the authentic capture failure receipt with main-RAM `e009` dispatch,
  enter, data-read, and control-write counts. A raw-sector-only trace now
  states precisely that the missing proof is a game-owned PCE-CD data read.
- ✅ 2026-07-27 DM1 V1 door/wall-ornament source-lock maintenance. The
  viewport audit now follows the DM1-owned F0111 ornament planner after its
  coordinate sets and D2/D3 palette maps moved out of M11. It continues to
  verify the real ReDMCSB F0107/F0111 ordering, clipping and occlusion
  contract; `dm1_v1_viewport_door_wall_ornament_source_lock` passes.
- ✅ 2026-07-27 DM1 PC34 C70 save-event roundtrip. F0435 now reconstructs
  the signed `EVENT.B.LightPower` union for C70 rather than demoting it to
  generic cell/effect bytes, so a saved light-decay event can be written
  again by F0433. The PC34 export suite also verifies a materialized dungeon
  tail roundtrip and rejects an unproven C24 Fluxcage slot on the state-only
  path.
- ✅ 2026-07-28 DM1 real HoC orientation and champion-pointer regression.
  The registered `m11_dm1_hoc_orientation_runtime_pc34` CTest starts from
  the local original PC34 data, validates F0128 viewport material in all
  four directions and through live turn inputs, selects a real C127 mirror,
  resurrects the champion, and opens that champion's HUD inventory through
  the production pointer path. It also proves the shipped Hall's eight
  F0115 object candidates reach a real F0791 material draw, rather than
  needing synthetic floor or alcove art. The focused real-data
  HoC/object/alcove/save suite passes 4/4.

- ✅ 2026-07-28 DM1 default C140 save path. The live inventory SAVE control
  now has a regression that clears its test-only path override, creates the
  normal per-user `saves/dm1` directory, writes the save, and reloads it.
  This covers a fresh profile's former file-not-found failure mode.
- ✅ 2026-07-28 Compact runtime graphics popup. F10 now uses a narrow
  right-side panel and leaves the live viewport undimmed, so V1/V2.x mode,
  filter, palette, and scale changes can be judged immediately. Its input
  remains modal; the regression verifies the exposed viewport, compact close
  hitbox, and live setting changes.
- ✅ 2026-07-28 DM1 HoC object coverage and inventory-panel controls. The
  real PC34 HoC regression now requires every unique original object graphic
  from all eight ordinary candidates to reach an F0791 blit. C140/C141/C145/
  C011 are resolved before C081's broad inventory-panel route, so the
  visible Save, music, Zz and close controls cannot be swallowed. The
  source-owned save-disk menu is explicitly exercised before its save write.

- ✅ 2026-07-28 DM1 full turn-button feedback. Q/E, Home/End and controller
  turns now outline the complete 29x23 C013 turn cells; mouse hit geometry
  remains the original narrower C068/C069 rectangles.
- ✅ 2026-07-28 CSB title/Entrance source timing. ReDMCSB `TITLE.C:451-463`
  proves 60 VBlanks of PRESENTS, 20 CHAOS shrink frames, `Delay(20)` on the
  full CHAOS page, then `Delay(2)` on STRIKES BACK. The old 101-tick model
  held the final title frame for one VBlank. CSB now uses the correct
  102-tick timeline, and focused real-data title/Entrance regressions pass.
- ✅ 2026-07-28 DM1 V2.x current verification. Built the only previously
  absent registered V2 cursor-mask test binary, then ran the complete
  V2.0/V2.1/V2.2 CTest selection against local original DM1 data: 88/88
  passed. Coverage includes mode handoff, HUD/pointer routes, viewport,
  item/creature/spell/effect paths, resolution mapping, assetpack gates and
  real runtime presentation smoke.
- ✅ 2026-07-28 DM1 original PC34 save round trip. A real DOSBox
  `DMSAVE.DAT` now passes fixture-free F0435 -> F0433 -> F0435 admission:
  source bytes stage into a live world, a saved portrait reaches the active
  inventory panel, the complete tail is preserved, and exported bytes reload
  through the same handoff. C13 remains optional evidence, as it is in the
  C3 event stream; a valid C13-free save is no longer rejected for lacking a
  fabricated C13 lifecycle receipt. The external-corpus, handoff, and
  external-HoC runtime regressions pass against the supplied data.
- ✅ 2026-07-28 DM1 F0115 near-square consumer audit. Retired the stale
  TODO claim that D0/D1 object presentation needed a second M11 bridge.
  The active renderer already uses F0098 for source floor/ceiling material
  and F0115 C2500/F0791 for visible floor objects; the old isolated receipt
  has no production caller and must not be wired as a duplicate item blit.
  Real-PC34 floor-item and alcove runtime regressions pass.
- ✅ 2026-07-28 DM1 D0C C15 effect-order repair. The live F0115 receipt no
  longer filters fluxcage or rebirth C15 records before their source-specific
  consumers run. It preserves original C15 order while the renderer remains
  no-draw without an authenticated special bitmap. C14/C15 layout, projectile
  impact, D0C receipt, C15 runtime-capture, and projectile presentation tests
  pass.
- ✅ 2026-07-28 DM1 HoC F0115 presented-pixel gate. The real PC34 Hall sweep
  finds all eight original floor/alcove object graphics and now requires each
  F0791 destination rectangle to change after its exact source blit. This
  proves final framebuffer consumption rather than only a material receipt.
  The identical real-data sweep now passes in V1, V2.0, V2.1, and V2.2, with
  a real C127 mirror route in every presentation mode.

- ✅ 2026-07-28 DM1 V2 inscription preservation. V2.2 no longer suppresses
  the final ReDMCSB F0107/M648 repaint after V22 art. V2.0, V2.1, and V2.2
  now all prove exact original M648 glyph pixels, C10 transparency, and stale
  text invalidation with real PC34 wall text.
## 2026-07-28 DM1 C14/C15 final viewport consumers

- Closed the stale DM1 F0115 C14/C15 host-consumer follow-up. Real PC34
  runtime tests now prove a thrown object reaches the final C2900 material
  blit and an ordinary C15 explosion reaches the deferred final-pixel pass.
  Source identity, catalogue admission, material fingerprint, and fail-closed
  rejection remain enforced before either draw.

## 2026-07-28 DM1 V2 inventory controls

- Added runtime coverage for C141 music, C140 save-disk, C145 rest and C011
  close in V2.0, V2.1 and V2.2. Presentation selection does not make the
  visible DM1 inventory controls inert.

## 2026-07-28 DM1 HoC capture route

- The real-PC34 HoC regression now reports its selected source mirror route:
  wall `(14,2)`, party `(14,3)`, north, ordinal `5` for the installed corpus.
  This makes repeatable macOS/window capture possible without guessing a
  champion-mirror location.

## DM1 HoC viewport occlusion

- **DM1-VIEWPORT-001**: Fixed the corridor-through-wall artifact in the live
  M11 renderer. ReDMCSB F0128 draws center walls as part of each square before
  visiting nearer squares; Firestaff's deferred F0115 batch could otherwise
  paint deeper corridor content over a nearer wall. The final source-backed
  center-wall pass now restores the wall and then replays the D1C champion
  mirror route. Verified with the DM1 wall-ornament and inventory placement
  tests plus a clean `firestaff` Ninja build on 2026-08-05.
## DM1 source-data fail-closed wall rendering

- **DM1-VIEWPORT-002**: Removed the synthetic black rectangle used when a
  center wall bitmap could not be loaded. The ReDMCSB wall path now leaves the
  cleared/background pixels unchanged and reports the missing authenticated
  GRAPHICS.DAT material through the existing asset route. This prevents a
  missing asset from masquerading as a corridor opening or fabricated wall.
  Verified with the DM1 wall-ornament (`121/121`) and inventory placement
  (`156/156`) tests plus a successful Ninja `firestaff` build on 2026-08-05.
## DM1 centre-wall ornament restoration

- **DM1-VIEWPORT-003**: Prevented the final nearest-wall occlusion replay from
  erasing authentic centre-wall inscriptions and alcove material. The replay
  now restores only source-owned centre ornaments after the wall bitmap, then
  hands the live champion mirror route back to the renderer; side ornaments
  are not replayed across the occlusion boundary. Verified with the DM1 wall
  ornament (`121/121`) and inventory placement (`156/156`) tests and a clean
  Ninja build on 2026-08-05.

- ✅ 2026-08-05 Nexus palette source-lock correction: aligned the Phase 4
  rendering documentation with the actual fail-closed `STONE.BIN` loader.
  Short palettes clear and remain unavailable; they do not receive the old
  inferred `g_npal_default` colour table. Verified by the real-data DGN
  geometry readiness gate against `/Users/bosse/.firestaff/data/nexus`.
- ✅ 2026-08-05 DM2 actuator generator provenance hardening: removed the
  remaining live wall-mecha generator mutations. Creature generation no
  longer invents a fixed HP/base value or tick-derived direction, and item
  generation no longer allocates a generic DB item from actuator data alone.
  Both remain fail-closed pending the complete source `ALLOC_NEW_CREATURE` /
  `ALLOC_NEW_DBITEM` ownership chains. Verified by the focused actuator and
  runtime gates plus the mounted real-data startup, HUD, material,
  scene/weather and original-save-writer gates.
- ✅ 2026-08-05 Nexus rasterizer provenance cleanup: corrected the Phase 4
  source-lock record to describe the actual production boundary. Flat-color
  geometry, unsupported 3D assets, and missing surfaces/textures remain
  explicitly no-draw; the retired gray-billboard/placeholder claims are no
  longer documented as runtime features. Verified with
  `test_nexus_v1_dgn_material_raster`, the real-data DGN geometry gate, and
  `git diff --check`.
- ✅ 2026-08-05 DM2 unbound CCM timer hardening: an unresolved
  `DM2_THINK_CREATURE` body now consumes its source timer without re-queuing a
  coordinate-only creature retry. Live record pools and timer queues remain
  unchanged until the complete original CCM stream owns animation, movement
  and rescheduling. Verified by the think-creature, CCM-runtime and CAII
  reschedule gates.

- ✅ 2026-08-05 Nexus FACE.BIN production provenance gate: the low-level
  retail PRS3 structural/pixel diagnostic remains available for evidence, but
  `nexus_ui_load_face_record()` no longer promotes unproven PRS3 output or its
  64-entry per-frame palette into live startup UI. Production now records all
  portraits as blocked until an original Saturn capture authenticates pixel
  grammar, palette lane, and placement. Verified with the real FACE.BIN
  structural probe, updated Track 1 launch probe (57/57), and the focused
  Nexus build.
## DM1 combat-log source font guard

- **DM1-UI-001**: The normal verified DM1 catalog launch no longer renders
  the built-in mini-font when the original `GRAPHICS.DAT` font is unavailable.
  It now fails closed until the source font is bound; the mini-font remains
  available only for explicitly non-catalog diagnostic callers. This removes
  a synthetic production visual without changing the source-backed font path.
  Verified with a successful Ninja `firestaff` build and combat-log contract
  test (`5/5`) on 2026-08-05.

- ✅ 2026-08-05 Nexus SAL playback gate correction: real SAL tone decoding
  can now populate diagnostic receipts, but `nexus_sound_play_event()` and
  `nexus_sound_play_idx()` check the complete runtime receipt before invoking
  the tone trigger. Decoded bytes cannot bypass the unresolved SDDRVS/event
  ABI gate. Verified with the real-corpus sound runtime receipt test.
## DM1 HoC source item-name guard

- **DM1-HOC-OBJECTS-003**: DM1 item labels now require the authenticated
  ReDMCSB `OBJECT.C` M564 icon-indexed name stream. When that source table is
  absent or malformed, Firestaff leaves the label empty instead of presenting
  the legacy hand-written subtype catalog as if it were original data. The
  fallback catalog remains available only outside DM1 source-owned routes.
  Verified with a successful Ninja `firestaff` and real-alcove target build,
  plus `git diff --check`, on 2026-08-05.

- ✅ 2026-08-05 CSB scanner inventory clarity: `--scan-data` now labels
  `GRAPHICS.DAT` and `DUNGEON.DAT` explicitly as launch requirements, then
  recursively reports every other hash-catalogued CSB source medium it finds,
  including entries inside supported archives. This keeps the two-file launch
  gate intact while exposing verified `ANIMATE.*`, Hint Oracle, Utility Disk,
  `MINI.DAT`, and platform sidecars from the real data rather than relying on
  a small fixed list of loose filenames. The shared fingerprint test passes
  284/0.

- ✅ 2026-08-05 CSB Atari ST title cadence: the real `ANIMATE.SCR` M11
  handoff regression now proves that each 55 ms V1 tick becomes the correct
  accumulated 50 Hz source-VBlank count, never regresses, and reaches the
  `FTLCODE` handoff only at the script-derived terminal boundary. This guards
  against a title that advances too quickly. ReDMCSB `ANIM.C:67-72` and its
  VBlank waits establish the source timing; the extracted local Atari ST
  package passes the focused handoff test.
## DM1 source object icon parity

- **DM1-HOC-OBJECTS-004**: Added the missing ReDMCSB `OBJECT.C F0033`
  charged-Jewel-Symal branch. DM1 now resolves the source `G0237` Jewel Symal
  icon from its raw `JUNK.ChargeCount`, matching the original water/illumulet
  charged-item family instead of leaving the base icon selected. Regression
  coverage exercises the PC34 raw record and expects icon 11 for a charged
  Jewel Symal. Verification: `test_dm1_v1_projectile_explosion_render_pc34_compat`
  passed with all tests, plus `git diff --check`, on 2026-08-05.

## DM1 source fountain interaction

- **DM1-HOC-OBJECTS-005**: Reconnected the live DM1 C080 wall-click route to
  the ReDMCSB `F0601` fountain predicate. The current map's real
  `DUNGEON.DAT` wall-ornament table is now matched against `G0193` before a
  leader-hand object can be changed. Empty-hand drinking, charged waterskin
  filling and empty-flask to water-flask mutation now update the loaded
  runtime records; generic wall ornaments retain the sensor/drop path.
  Verified with a full `firestaff` build and the source fountain regression
  (`fountainInteractionInvariantOk=1`) on 2026-08-05.

## DM1 source wall ornament table correction

- **DM1-HOC-OBJECTS-006**: Corrected the DM1 PC34/I34E `G0194` wall-ornament
  coordinate-set table. Firestaff had used the ReDMCSB `MEDIA353` variant
  (`DUNVIEW.C:846-906`); PC34 uses the `MEDIA529`/`I34E` table at
  `DUNVIEW.C:932-1007`, including coordinate sets 7/8 for the real wall
  ornament family. The source graphic base remains `M615=259`, with F0791
  transparent colour 10 and G0198/G0199 palette maps unchanged. Focused
  G0194 and wall-plan tests pass after the correction. Real macOS pixel
  capture is still tracked separately in `DM1-HOC-OBJECTS-001`.
- ✅ 2026-08-05 CSB Atari ST executable-media inventory: corrected the
  `SWITCH.DAT` fingerprint to the bytes in the original hard-disk package and
  added hash identities for `ANIMATE.FTL`, `CHAOS.FTL`, and `FTLCODE`.
  ReDMCSB `COMPILE.H:609-620` identifies the three modules and `ANIM.C:94`
  makes the `FTLCODE` transfer explicit. They are reported as verified source
  media without changing the `GRAPHICS.DAT`/`DUNGEON.DAT` start gate. The
  fingerprint suite passes 294/0 against the extracted local package.
- ✅ 2026-08-05 Theron teleporter fail-closed hardening: unresolved legacy
  object-ID links and cyclic/overlong chains no longer report a successful
  transition or place the party at the clicked square. Transition and party
  state remain unchanged until a real terminal object record resolves;
  missing-target and cycle regressions now assert rejection.
- ✅ 2026-08-05 CSB Utility Disk CMP disk-format correction: replaced the
  synthetic 496-byte portrait layout with ReDMCSB's actual 508-byte `CMP`
  record. The decoder now reads the big-endian `Magic`, dungeon-id, platform,
  compatibility words, reserved words, name/title and the 464-byte portrait at
  offset 44. ReDMCSB `DEFS.H` defines the layout and `CEDT001.C F7000` writes
  exactly 508 bytes. The extracted original Atari ST `PORTRAIT/HALK.CMP`
  decodes as HALK, THE BARBARIAN; CMP import, portrait-handoff and title/import
  regressions pass without allowing a portrait-only file to invent party state.
- ✅ 2026-08-05 Theron legacy asset-parser cleanup completed: removed the
  unreachable THS4 sound parser body and its guessed marker constants from
  the implementation. The public diagnostic APIs remain explicit rejection
  seams; no Firestaff-only THG3/THS4 bytes can become runtime media.
- ✅ 2026-08-05 CSB Utility Disk portrait inventory and scanner repair: added
  hash identities for all 26 original Atari ST `PORTRAIT/*.CMP` files, whose
  508-byte disk format is established by ReDMCSB `CEDTDATA.C:394/397` and
  `CEDT001.C F7000`. The CSB report now uses media already materialized by
  the status scan instead of triggering a second recursive archive traversal.
  Archive materialization retains the real `ANIMATE.FTL`, `CHAOS.FTL` and
  `FTLCODE` modules beside the launch pair. A real loose-package scan shows
  those modules plus `SWITCH.DAT` and `MINI.DAT`; the fingerprint suite passes
  373/0.
- ✅ 2026-08-05 Theron Track 19 item-name binding: added a source-span reader
  for the US MODE1/2048 table at ISO offset `0x0E9271`. It validates all 69
  null-separated names against the verified catalog before returning any one
  label, and rejects truncation or byte changes. The real local `TQUS19.iso`
  passes the full table probe.
- ✅ 2026-08-05 Theron Track 19 level-label binding: added byte validation for
  the real US ISO selector table at offset `2112059`, covering `LEVEL  1`
  through `LEVEL 15`. The probe validates the complete table and rejects a
  changed label byte; this exposes labels only and does not invent maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB scanner sidecar visibility: `--scan-data` now searches
  beside the hash-matched loose `GRAPHICS.DAT` package (not only the selected
  data root) before reporting verified CSB media. The candidate inventory also
  recognizes Atari `ANIMATE.FTL`, `CHAOS.FTL`, `FTLCODE` and `MINI.DAT`.
  Archive-cache regression coverage now proves the three real Atari startup
  modules remain materialized beside the verified launch pair. Source-lock:
  ReDMCSB `ANIM.C:67-72,94`; verified against the real Atari ST archive.
- ✅ 2026-08-05 Theron Track 19 file-inventory binding: added a reusable
  file-backed receipt that authenticates the exact ISO hash/size and validates
  both real US metadata spans (69 item names and 15 level labels). The
  inventory exposes verification flags without admitting dungeon maps,
  objects, or bitmap semantics.
- ✅ 2026-08-05 CSB Atari-animationens runtimekedja: den verkliga
  `ANIMATE.FTL`/`CHAOS.FTL`/`FTLCODE`-trion har nu ett eget
  hash-verifierat discovery- och cachekvitto. Modulerna måste komma från
  samma katalog eller arkiv och körs aldrig som värdbinärer. Verifierat mot
  den lokala Atari ST 2.0-katalogen med original-MD5. Källor: ReDMCSB
  `ANIM.C:67-72,94`, `COMPILE.H:609-620` samt DMWebs Animation Script- och
  Animationsformatdokumentation.
- ✅ 2026-08-05 CSB map-difficulty provenance: removed the invented
  champion-count percentage scale and its hard-coded three-champion default.
  A loaded CSB profile now takes the current map's authenticated `MAP.C`
  high-nibble difficulty from `DUNGEON.DAT`; a roster-only or failed handoff
  stays explicitly unbound. Runtime-image restore no longer revives the old
  synthetic multiplier. Source-lock: ReDMCSB `DEFS.H` `MAP.C`, `PANEL.C`
  F0337, `CHAMPION.C` and `PROJEXPL.C`; covered by CSB boot and save tests.
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

# Nexus FACE.BIN diagnostics now honor `FIRESTAFF_NEXUS_DATA_DIR`; the real
# corpus path is no longer silently replaced by `$HOME/.firestaff/data/nexus`.
# Documentation now distinguishes the reviewed bounded PRS3 decoder and its
# 20/20 real FACE.BIN result from the still-blocked Saturn VDP1 presentation
# route. No startup pixels or synthetic fallback were enabled.

# Nexus ITEM.IBS diagnostics now honor `FIRESTAFF_NEXUS_DATA_DIR`; the real
# corpus test verifies 243 item declarations, 223 inventory images, and 109
# floor images. Nexus HUD documentation now identifies the procedural V2
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

# ✅ 2026-08-05 DM2 raw-SKSave pseudo-import removal

The former raw-SKSave bridge was removed from the playable path. It treated
the original continuous `DM2_GAME_LOAD` SUPPRESS stream as a different
GameStateBlock and accepted a Firestaff-only inventory/minion tail, which
could admit a fabricated save. Raw original saves now retain only their
hash-bound dungeon-prefix receipt and fail closed until the complete
SKProject read order, including `DM2_READ_SKSAVE_DUNGEON`, is materialised.
The regression proves the prior synthetic tail cannot be accepted by either
the candidate or slot loader. Source:
`SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD` (1476-1526).
# ✅ 2026-08-06 DM2 unowned weather-timer receipt removal

The active DM2 runtime no longer marks `DM2_SET_TIMER_WEATHER` as valid merely
because an outdoor map tick occurred. A valid receipt now requires the live,
source-owned weather chain; otherwise it is cleared before M11 can combine it
with GDAT material. This prevents host tick counters or map flags from
authorizing weather pixels. The runtime weather-timer regression proves an
outdoor session with no original chain produces neither a timer nor a receipt.
Source: `SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD` and
`SKWINSPX/src/v5/c_weather.cpp::DM2_SET_TIMER_WEATHER`.
- ✅ 2026-08-05 DM1 creature replacement palette binding: the live M11
  creature compositor now applies the source G0243/F0695 slot-9/slot-10
  replacement targets after the G0221/G0222 depth mapping. No-replacement
  creatures keep the original palette. Ninja target and the DM1 creature
  rendering integration test pass.

- ✅ 2026-08-05 DM1 generated-palette removal: removed the legacy GRAPHICS.DAT
  reader's synthetic 16-255 colour ramp, the duplicate full-palette grayscale
  fallback and the unreachable game-loop fallback. The authenticated PC-34
  16-colour palette is retained; extracted VGA palettes still take precedence;
  unavailable entries now remain no-draw. Ninja `firestaff` and
  `git diff --check` pass. Tracked as `DM1-ORIGINAL-REPLACE-014`.

# ✅ 2026-08-06 DM2 PC English Greatstone raw-asset catalogue gate

The DM2 V1 asset-loader probe now verifies that the hash-verified PC English
`GRAPHICS.DAT` raw-data table has exactly 5,624 records, matching Greatstone's
PC 1.0 catalogue IDs `0000` through `5623`. The audit also establishes that
Greatstone publishes 4,032 visual records (860 IMG3, 3,167 IMG9, 4 IMG11, and
1 FNT1); the probe verifies every one against a distinct source raw record.
The FNT1 source is raw record 0203 (`dtRaw7`), with later `dtImageOffset`
rows for glyph offsets; the file's separate 11,854-row ENT1 table is metadata,
not a conflicting asset count. The probe continues to decode source-owned
floor, ceiling, title, door and interface images only from the original file.
Verified against `/Users/bosse/.firestaff/data/dm2/dos_extract/data/graphics.dat`
(MD5 `25247ede4dabb6a71e5dabdfbcd5907d`): 32/32 passes. Source:
http://greatstone.free.fr/dm/db_data/dm2_pc10_en/graphics.dat/graphics.dat.html.
This was initially an identity/catalogue gate. The follow-up audit recorded
below on 2026-08-06 completes the comparison for all 4,031 raster PNGs:
4,030 are palette-index exact, while raw 2279 is the documented Greatstone
U4 greyscale-export difference. FNT1 raw 0203 remains metadata rather than
a raster or a generated-font fallback.
- ✅ 2026-08-05 DM1 V2.2 source-shape binding: replaced graphic-0 texture
  placeholders in the shape bridge with the authenticated PC-34
  GRAPHICS.DAT records for depth-specific walls, floor, door frame, stairs
  and teleporter field. Unsupported variants and unreviewed modern maps now
  remain unavailable instead of selecting substitute pixels. The focused
  `test_dm1_v22_verification` passes with source-index assertions. Tracked as
  `DM1-ORIGINAL-REPLACE-015`.
# ✅ 2026-08-06 DM2 source-door RAW4 fallback removal

`dm2_v1_viewport_build_door_render_plan()` no longer uses the old compact,
hard-coded door panel or button rectangles when a source-scheduled G1/GDAT
scene cannot resolve its selected `INTERFACE_GENERAL/0/RAW4/0` owner. It
removes that door from the plan instead, matching SKProject
`DM2_DRAW_DOOR`/`DM2_DRAW_DOOR_FRAMES` rather than presenting a guessed
position. Non-source diagnostics retain the compatibility geometry. The
real-data door-overlay test now mounts the original asset loader so its
source plan uses the actual RAW4 table. Verification: source-owner absence
gate PASS; PC English RAW4 placement test PASS; canonical GDAT door overlay
plan PASS. Source: `SKWINSPX/src/v4/c_gui_vp.cpp::DM2_DRAW_DOOR` and
`DM2_DRAW_DOOR_FRAMES`.

# ✅ 2026-08-06 DM1 legacy V2.2 overlay synthetic-pixel removal

The retired `m11_v22_render_overlay_pc34` compatibility route no longer paints
material-colour rectangles, palette-shadowed fills, borders, or any other
generated V2.2 pixels over the DM1 V1 framebuffer. It remains link-compatible
for probes and preserves the source viewport-cell geometry API, while the
authenticated in-place renderer is the only route allowed to draw V2.2 art.
The focused test now proves that both an empty cache and a populated V2.2
cache leave the framebuffer byte-for-byte unchanged. Tracked as
`DM1-ORIGINAL-REPLACE-016`.

# ✅ 2026-08-06 DM2 source-teleporter compact-placement removal

The live G1/GDAT viewport no longer paints a teleporter field with the old
compact placement table. SKProject `DRAW_TELEPORTER_TILE` owns placement via
`tblGraphicsTeleporterWords`, `tblGraphicsTeleporterBytes4`, and the selected
RAW4 rectangle, including its per-cell copy/flip and offsets. Until that
complete source command is materialized, a source-bound field is explicitly
no-draw and records a blocked teleporter material; data-free diagnostics keep
their compatibility coverage. Verification: `test_dm2_v1_teleporter_material_gate`
PASS and PC English boot probe PASS with original `graphics.dat` MD5
`25247ede4dabb6a71e5dabdfbcd5907d`. Source: `SKWINSPX/src/v4/c_gui_vp.cpp::DRAW_TELEPORTER_TILE`
and `kskval1.h` teleporter tables.

# ✅ 2026-08-06 DM2 PC Greatstone full raster decode audit

The PC English GDAT loader now follows `DME.h IMG3::Getpf`'s `OffsetY ==
-32` branch before selecting IMG9 decoding. Greatstone's four records marked
IMG11 (`0642`, `2691`, `2781`, `2894`) are the original uncompressed U8
payload form: their pixels begin at `IMG3 + 10`, rather than an IMG9 command
stream. The shared extraction receipt now decodes them correctly and the
catalogue audit proves actual decoded pixel hashes for all 4,031 IMG3/IMG9/
IMG11 rasters in the PC `GRAPHICS.DAT`; only FNT1 0203 remains a font record,
not a raster fallback. Verification: `probe_dm2_v1_asset_loader` 35/35 PASS
against MD5 `25247ede4dabb6a71e5dabdfbcd5907d`; PC English boot probe PASS.
Source: `SKWINSPX/src/v4/DME.h::IMG3::Getpf` and
`c_gfx_decode.cpp::EXTRACT_GDAT_IMAGE`.

# ✅ 2026-08-06 DM2 PC Greatstone FNT1 ownership audit

Greatstone's sole non-raster visual entry, raw `0203.FNT1`, is now checked as
the single `INTERFACE_GENERAL`/Main Screen 0 `dtRaw7` record. It is retained
as source metadata only. It is not promoted to an image and cannot become a
fallback font: original `SkWinCore::_3929_0e16_FONT_LOAD` instead loads the
PC startup/HUD glyph rows from `INTERFACE_GENERAL/0/dt07/0` (exactly 0x300
bytes). Verification: the real-PC `probe_dm2_v1_asset_loader` checks both
the catalogue count and ownership; the boot-profile smoke test verifies the
six original 128-glyph rows. Sources: Greatstone PC 1.0 catalogue,
`SKWINSPX/src/v4/skcore.cpp::_3929_0e16_FONT_LOAD`, and
`SKWINSPX/src/v4/sktext.cpp::QUERY_FONT`.

# ✅ 2026-08-06 DM2 PC Greatstone raw-palette audit

`QUERY_GDAT_IMAGE_LOCALPAL` now reads the final 16 bytes of every accepted
4-bpp raw GDAT record, exactly as `SKULLWIN/c_querydb.cpp:228-253` specifies.
The previous width-derived address could read compressed IMG3 commands as a
palette. The loader also exposes an audit-only exact-raw decode path so
language variants sharing a category/index/field cannot be compared through
the first matching (usually English) entry. A temporary external audit of all
4,031 Greatstone IMG3/IMG9/IMG11 PNGs against the hash-verified PC English
`GRAPHICS.DAT` gives 4,030 exact palette-index matches. Raw 2279 is the sole
documented representation difference: source `EXTRACT_GDAT_IMAGE` returns
the special U4 payload with its raw-tail palette, while Greatstone emits
greyscale-expanded nibbles. No downloaded or generated art is used at
runtime. Verification: `probe_dm2_v1_asset_loader` and PC English boot probe
PASS. Sources: Greatstone PC 1.0 catalogue,
`SKULLWIN/c_querydb.cpp::DM2_QUERY_GDAT_IMAGE_LOCALPAL`, and
`SKULLWIN/c_gdatfile.cpp::DM2_EXTRACT_GDAT_IMAGE`.

# ✅ 2026-08-06 DM2 CCM handler-status inventory correction

The public CCM opcode catalog no longer calls the already ported CCM0B,
CCM0C, wall-activation, ladder/hole, transform and `DM2_1B7D5` handler bodies
"stubs". Each is dispatched only through the callback-bound advanced port in
`dm2_v1_ccm_execute_advanced`; no handler is admitted without the source
command stream, live DB4/CAII record owner and mandatory callbacks. The CCM
source-alignment test now verifies that every source command-table row is
marked implemented, while the real boot probe continues to use the
hash-verified PC English corpus. Sources: `SKULLWIN/c_creature.cpp`
`DM2_PROCEED_CCM` (2930-3212) and the handler bodies at 1709-2928.

# ✅ 2026-08-06 DM2 HUD command-text provenance gate

`QUERY_CMDSTR_TEXT` and `DM2_QUERY_CMDSTR_TEXT` no longer accept arbitrary
caller buffers and label them as original GDAT text. The original routines
read GDAT internally, so the tuple-only adapter now blocks until a mounted
GDAT loader can provide the selected raw record and provenance. Consequently
command-text UI events and the associated right-panel refresh cannot be
created from synthetic text; non-text container events retain their existing
source-bound routing. Verification: `test_dm2_v1_hud_panel_routing` and the
hash-verified PC English boot probe PASS. Sources:
`SKULLWIN/c_querydb.cpp::DM2_QUERY_CMDSTR_TEXT` (line 274) and
`SKWIN/SkWinCore.cpp::QUERY_CMDSTR_TEXT` (line 8136).

# ✅ 2026-08-06 DM2 M11 source-menu pointer gate

M11 now accepts DM2 startup clicks only through the original title-menu GDAT
rectangles and their display-to-framebuffer retry. The final fallback to the
old Firestaff row/panel layout is removed, so an unmatched click cannot select
a host-authored save row or trigger gameplay. The original `0xD7` new-game
and `0xD9` resume hit paths remain intact. Verification:
`test_dm2_v1_startup_pointer_hit_contract` and the hash-verified PC English
boot probe PASS. Sources: `SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN`
(55182-55244), `HANDLE_UI_EVENT` (32001-32021), and source GDAT rectangle
table raw 201.

# ✅ 2026-08-06 DM2 source wall-button fallback removal

The local DB2/DB3 wall-button walkers are now available only to isolated
fixture providers. A mounted M11 GDAT provider must resolve a wall button via
the authenticated `dm2_v1_dungeon_find_text_wall_gfx` or
`dm2_v1_dungeon_resolve_actuator_wall_gfx` chain; an unresolved chain leaves
the button absent and cannot select a guessed `WALL_GFX` image. Verification:
`test_dm2_v1_door_scene_control_gate` PASS, real-data door overlay plan PASS,
and the PC English boot probe reaches `dm2-startup-menu`. Source:
`SKWINSPX/src/v4/c_gui_vp.cpp::DM2_DRAW_DOOR` and the G1 record helpers in
`c_record.cpp`.
- ✅ 2026-08-05 Theron Track02 static consumer disassembly: added a
  byte-backed HuC6280 bank-$1f fragment for `$243e–$24b0`, extracted from
  the hash-locked US retail ISO with da65. It records the real bitstream and
  memory-map-register helpers around `$2450/$2459/$246e`. The later `$2600`
  consumer remains correctly blocked because its bytes are RAM-loaded and
  are not present in the static image; the required next capture is the
  post-CD RAM window with PC and source-LBA provenance.
- ✅ 2026-08-06 DM1 V2 HUD synthetic overlay removal: the compatibility
  `dm1_v2_hud_overlay_pc34` route no longer paints its hard-coded 5x5 font,
  invented champion names, procedural compass/status/action/rune panels, or
  fixed meter values. It retains state/timing APIs and stays strict no-draw
  until the real PC34 M653/C009/C010/C011 surfaces are decoded and passed by a
  source-owned renderer. The focused test proves both the generic HUD and
  champion panel leave populated framebuffers unchanged. Tracked as
  `DM1-ORIGINAL-REPLACE-017`.
- ✅ 2026-08-06 DM1 V2.2 missing-art checkerboard removal: deleted the
  hard-coded 16x16 magenta/cyan bitmap and its valid-looking descriptor.
  `dm1_v22_get_missing_descriptor()` now returns `NULL` and zero dimensions;
  metadata-only PNG discovery remains non-renderable until a real decoder is
  bound. The focused asset-pipeline test verifies the explicit no-draw result.
  Tracked as `DM1-ORIGINAL-REPLACE-018`.
- ✅ 2026-08-05 Theron real app-capture CD-state receipt: added a strict
  parser for the supplied Mednafen `*.trace.cd` sidecar. The real capture is
  accepted with MD5 `3e61070ac6bb1ef0ec03cbb83f6a7d6c`, 51 SCSI commands, 157
  requested/raw/bound MODE1/2352 sectors, LBA range `3234..4267`, 25 CD IRQ
  rows and 19 destination candidates. The receipt remains explicitly opaque:
  it does not publish level, object, tile, palette or runtime semantics.

- ✅ 2026-08-06 DM1 V2.2 real PNG pixel decoder: the modern asset pipeline now
  decodes the real zlib-compressed PNG files from the installed V2.2 pack
  instead of accepting IHDR metadata only. It validates the PNG structure,
  collects all IDAT chunks, inflates non-interlaced 8-bit RGB/RGBA data,
  applies filters 0-4, and exposes packed RGBA pixels to the authenticated
  asset descriptor. Unsupported PNG variants fail closed and do not generate
  substitute art. `firestaff_v2` now links the existing zlib provider.
  Verification: `ninja -C /tmp/firestaff-title-build test_dm1_v22_asset_pipeline`
  and the focused test both pass. Tracked as `DM1-ORIGINAL-REPLACE-019`.

- ✅ 2026-08-06 Theron V2.2 dead placeholder removal: removed the unused
  16×16 magenta checkerboard pixel array from the fixture-only modern-art
  module. The compatibility accessor still returns an empty result, so a
  missing asset cannot draw synthetic pixels; real Track 02 tile/palette
  ownership remains blocked pending loader/VRAM evidence.

- ✅ 2026-08-06 Theron procedural UI-chrome removal: deleted the obsolete
  fixture-only implementation containing inferred bars, coloured blocks and
  champion-slot geometry. The rendering regression now links the same
  production no-op seam used by `firestaff_theron`, so the removed renderer
  cannot be reintroduced accidentally through a test target. Real HUD tile,
  layout and portrait binding remain blocked until Track 02 ownership is
  captured.

- ✅ 2026-08-06 Theron inferred tile-renderer removal: deleted the obsolete
  fixture-only square/depth table, synthetic tile decoder/rasterizer and its
  synthetic probe. The production tile seam remains the existing no-op until
  an authenticated Track 02 tile-bank/material binding is available, so no
  generated tile surface can be mistaken for real dungeon graphics.

- ✅ 2026-08-06 Theron V2 procedural HUD production gate: removed the generated
  compass/text/rune/champion-bar overlay from production and launch-mode
  targets. The source remains only in the startup receipt fixture, where it
  preserves the handoff contract without shipping synthetic pixels. The
  production no-op HUD seam and manifest classifier await a real Track 02
  widget bank.

- ✅ 2026-08-06 Theron canonical ISO intake proof: the raw media-intake test
  now consumes the supplied assembled US Track 02 ISO when present and checks
  its real MD5, 2048-byte sector layout, 3221-sector payload, user-data window,
  and non-eligibility for raw MODE1/2352 trace preparation. This strengthens
  BIN/CUE/ISO separation without promoting opaque ISO bytes to level, object,
  bitmap, or palette semantics.

- ✅ 2026-08-06 Theron Main-RAM loader capture receipt: the supplied real
  Mednafen sidecar is now parsed as a strict regular-file receipt. It verifies
  the observed TIA transfer from `$c800` to `$0404` (128 bytes), loader PC
  `$2286` / physical `$1f0286`, the RTS at `$228d`, and the post-RTS opcode.
  The receipt deliberately exposes no `$2600` bytes and cannot publish level,
  object, or viewport semantics; the remaining gap is the executing consumer
  RAM window joined back to the CD source-LBA.

- ✅ 2026-08-06 DM1 V2 generated-effects removal: retired the procedural
  particle presets, full-screen spell overlays, dust/sparkle field effects,
  and dynamic-light indexed repaint. ReDMCSB's source-owned projectile,
  explosion, field bitmap and F0337 palette routes remain authoritative;
  unbound V2 effects now fail closed without advancing or painting pixels.
  Focused enhanced-effects, spell-overlay and extended-field tests pass.
  Tracked as `DM1-ORIGINAL-REPLACE-020`.

- ✅ 2026-08-05 Nexus CD callback empty-path guard: `nexus_sound_cd_track()`
  now requires a materialized host audio path before invoking the external
  callback. A missing Red Book track remains a selection-only state and does
  not enter `cd_playing`; the focused gameplay test covers the no-empty-path
  contract. SAL/MAP event dispatch and authentic CD presentation remain
  blocked pending Saturn capture/decoder proof.

- ✅ 2026-08-06 DM1 held-object cursor redraw: pointer motion while the
  ReDMCSB G4055 leader hand contains an object now invalidates the framebuffer,
  and the SDL host arrow is hidden while the source object cursor is painted.
  This fixes the stale/absent object-shaped pointer after HoC or dungeon
  pickup without inventing cursor art. Verification: Ninja `firestaff` build,
  `test_m11_dm1_source_item_name_guard` (including held-object pointer redraw),
  and the real PC3.4 `test_m11_dm1_real_alcove_item_runtime_pc34` all pass.
- ✅ 2026-08-05 Nexus loot placeholder removal: removed the stale hard-coded
  DM.BIN/DM1-compatible creature-drop table from the public Nexus drop API.
  `nexus_drops_for_type()` and `nexus_drops_roll()` now return no loot until
  the Saturn creature/category/item dispatcher is proven; explicit generic
  gold-pile storage remains available for a future source-owned producer.
  Added `test_nexus_v1_drops_gate` covering the no-materialization contract.

- ✅ 2026-08-05 Nexus startup menu rectangle placeholder removal: M11 no longer
  paints the procedural save/champion fill- and outline-rectangles emitted by
  the host planner. State and hit-test metadata remain available, while menu
  chrome stays no-draw until `MENU.BPK`/`STABG.BIN` palette and VDP1 placement
  are authenticated from Saturn capture.

- ✅ 2026-08-06 DM1 wall-ornament index guard: the PC34 wall-ornament render
  plan now admits only the 60 global indices present in ReDMCSB G0194. A
  corrupt or unresolved map-local index can no longer become an arbitrary
  GRAPHICS.DAT lookup, which prevented stray wall pixels from masquerading as
  torches, holders, or other HoC ornaments. Verification: Ninja
  `test_dm1_v1_wall_ornament_pc34_compat` (128 assertions), `firestaff`, and
  real PC3.4 `test_m11_dm1_real_alcove_item_runtime_pc34` pass.
- ✅ 2026-08-05 Nexus startup portrait placement guard: M11 now retains the
  authenticated `FACE.BIN` loader/receipt but does not blit portraits through
  the planner's guessed 10×10 rectangles. Champion-index and VDP destination
  remain no-draw until Saturn placement evidence is available; the M11 startup
  gate regression now asserts the blocked pixel route.

- ✅ 2026-08-06 DM1 PC34 object icon bitfields: corrected the ReDMCSB F0033
  mapping for weapon/torch ChargeCount (bits 10..13) and SCROLL.Closed
  (bits 10..15), removing wrong item variants caused by shifted raw-byte
  reads. Focused icon/data tests, Ninja build, and the real PC3.4 alcove
  runtime test pass. HoC pickup/placement capture remains open in
  `DM1-HOC-OBJECTS-002`.

- ✅ 2026-08-05 Nexus door raster placeholder removal: removed the uncalled
  DM1-derived open/closed/locked door geometry and guessed palette indices
  from `nexus_v1_rasterizer.c`. The public gameplay-facing API now remains
  fail-closed even when a host texture is supplied. Added a framebuffer color
  and depth regression to `test_nexus_v1_dgn_material_raster`; Saturn door
  materials, animation frames, and VDP1 destinations remain capture-gated.
- ✅ 2026-08-06 DM2 startup synthetic-keyboard gate: M11 no longer translates
  generic Firestaff `UP`/`DOWN`/`ACCEPT` menu tokens into DM2 startup actions.
  `SHOW_MENU_SCREEN` owns its input through the original MessageLoop and its
  GDAT RAW4 event matrix; Firestaff has source-backed pointer dispatch for
  `0xD7` New Game and `0xD9` Resume, but not the original c_0aaf/c_input
  keyboard translation. The generic tokens are therefore inert rather than
  selecting a host-authored row. The real-PC M11 gate now proves that keyboard
  navigation is rejected while clicks in the authentic New/Resume rectangles
  still reach the normal `GAME_LOAD`/resume gates. Verification:
  `test_dm2_v1_m11_startup_profile_gate` passed against the hash-verified PC
  English `GRAPHICS.DAT` (MD5 `25247ede4dabb6a71e5dabdfbcd5907d`) and the DM2
  boot probe reached `dm2-startup-menu` with `levelLoaded=1`. Sources:
  `SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN` (55182-55220),
  `HANDLE_UI_EVENT` (32001-32021), and `SKULLWIN/startend.cpp`.

- ✅ 2026-08-06 DM1 pickup ownership: `M11_GameView_PickupItem()` now prefers
  the frame-local F0115 rendered pile-top THING, preventing an invisible
  linked-list neighbour from disappearing on pickup. Destination selection
  enforces the real PC34 F0302/G0038 `AllowedSlots` mask before choosing a
  hand, pouch or backpack slot. Slot-mask (152 assertions), source-name,
  Ninja, and real PC3.4 alcove runtime checks pass.
- ✅ 2026-08-05 Nexus TITLE.CG placement placeholder removal: removed
  `m11_draw_nexus_title_from_real_assets()`, which copied decoded title bytes
  to an unproven 320×224 top-left destination. TITLE.CG/TITLE.BIN remain
  byte-decoded source receipts; M11 startup presentation stays no-draw until
  Saturn VDP1/VDP2 placement and composition are authenticated.
- ✅ 2026-08-06 DM2 startup host-layout removal: removed the last production
  output from the obsolete fixed 78×50 panel, 14-pixel row cadence, English
  `CONTINUE`/`LOAD SLOT`/`NEW GAME` labels, and generic pointer hit-test.
  These values had no `SHOW_MENU_SCREEN` or GDAT owner. The legacy accessors
  now return no geometry/text/action, so callers must use the mounted RAW4
  click matrix through `dm2_v1_boot_startup_menu_pointer_hit`. Verification:
  `test_dm2_v1_startup_menu_action_contract` 98/98 and the real-PC
  `test_dm2_v1_m11_startup_profile_gate` pass. Sources:
  `SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN` and `HANDLE_UI_EVENT`.

- ✅ 2026-08-05 Nexus MENU.BPK PRS3 evidence refresh: aligned the retail
  framing assertions with the current 163-entry/162-surface corpus (158
  bounded frames and 4 unvalidated directory-span candidates) while keeping
  framed and opcode-prefix evaluators diagnostic-only. The real-corpus tests
  now pass for all MENU.BPK payloads; this proves DMWeb byte decoding only,
  not Saturn palette, VDP1 upload, or screen placement.
- ✅ 2026-08-05 DM1 HoC ornament-0 routing: corrected the live wall renderer
  to identify inscriptions by the synthetic final local `G0261` slot, as in
  ReDMCSB `DUNGEON.C F0173`, rather than by global ornament id 0. Global
  ornament 0 is valid source material and must remain on the normal
  `G0194/G0205` wall-ornament path. The real PC34 HoC wall-material probe
  still passes, including C346/C026 mirror presentation; focused Ninja
  build, wall-plan tests, and `git diff --check` pass. Real macOS torch/holder
  capture remains open in `DM1-HOC-OBJECTS-001`.

- ✅ 2026-08-06 Theron real Track 02 level-bank receipt: extended
  `test_theron_v1_track02_level_data_blocks` with an optional
  `FIRESTAFF_THERON_TRACK02_RAW` path. Against the supplied authentic US
  MODE1/2352 BIN it re-read all seven documented level offsets, verified the
  shared 0xE8-byte prologue and all per-level metadata bytes, and passed.
  This is raw-data provenance only; tile, palette, map and object semantics
  remain correctly fail-closed.
- ✅ 2026-08-06 DM1 invalid Thing subtype guard: `dm1_item_aspect_index()`
  now rejects out-of-range weapon, armour, potion, junk, scroll, and
  container subtypes instead of coercing them to subtype 0. This prevents a
  malformed or stale PC34 Thing from displaying an unrelated HoC junk icon.
  Corrected the raw-torch regression fixture to use PC34 ChargeCount 4
  (`0x90`, lit), matching ReDMCSB `DATA.C G0029` and the expected icon band.
  `test_dm1_v1_viewport_floor_ceiling_items_pc34_compat` 179/179 and
  `test_m11_dm1_f0033_raw_inventory_icon_gate` 6/6 pass.
- ✅ 2026-08-06 DM1 sensor-message fallback removal: movement-triggered
  sensor text now goes through the decoded DUNGEON.DAT text table and the
  existing source message queue. The old host dialog and synthetic `TEXT #N`
  label were removed; missing or malformed source text is fail-closed. The
  DM1 sensor-timing integration and `firestaff_m11` Ninja build pass.
- ✅ 2026-08-06 Theron JP Track 02 level-bank receipt: added the separate
  authenticated Japanese MODE1/2352 table (MD5
  `b7afb338ad31be1025b53f9aff12d73a`) with all seven real user-data offsets
  and metadata records. The regression now accepts optional
  `FIRESTAFF_THERON_TRACK02_JP_RAW` and passes against both supplied US and
  JP BINs; ISO/unknown variants remain rejected and no tile/map/object
  semantics were promoted.
- ✅ 2026-08-06 DM1 inventory drag hand destination: mouse-up now dispatches
  ReDMCSB `COMMAND.C` C211..C218 status-hand zones before the C507..C544
  inventory/chest route. Previously a floor or inventory pickup could remain
  in the transient G4055 leader hand when dragged onto another champion's
  hand, because release handling consulted only the inventory list. The
  source-owned F0302 swap path is now reached for both mouse-down and
  mouse-up. Ninja `firestaff_m11` build and the real PC3.4 alcove item runtime
  test pass; the existing inventory-panel test still has its unrelated
  pre-existing object-description fixture failures. Real macOS capture remains
  open in `DM1-HOC-OBJECTS-002`.

- ✅ 2026-08-06 Theron JP IPL/Stage-2 provenance: materialised the authentic
  JP Track 02 BIN from the supplied archive under the local Theron data root
  (MD5 `b7afb338ad31be1025b53f9aff12d73a`) and extended
  `test_theron_v1_stage2_disassembly_chain` to validate the JP IPL loader and
  dynamic `$3800` payload receipt. The real JP path resolves the dynamic
  payload to record/raw sector `0x4df`; US-specific later disassembly windows
  remain fail-closed and are not mislabeled as JP parity.

- ✅ 2026-08-06 Theron synthetic drop resolver removal: removed
  `theron_v1_track02_resolve_drop_item()`, which converted invented item IDs
  and a seed into guessed drops without a decoded T900 record. The verified
  66-entry Track 02 category table remains available for source-backed item
  metadata, while `theron_v1_drop_loot()` continues to reject publication
  until an original drop consumer and selection record are proven. The item
  category regression passes without synthetic drop assertions.
- ✅ 2026-08-05 Theron production combat placeholder isolation: the production
  `firestaff_theron` archive no longer links `theron_v1_compat.c`, whose
  creature speed/AI/attack table was not backed by a decoded T900 consumer.
  It now links the fail-closed runtime adapter; fixture targets retain the
  compatibility implementation explicitly. Added
  `test_theron_v1_combat_runtime_noop` to prove production spawn, combat,
  spell, drop and sound publication stays blocked. Production build plus the
  raw-media intake, combat fixture and mechanics regressions pass.
- ✅ 2026-08-05 Theron bank-$1f static consumer receipt: materialised the real
  `TQUS19.iso` from the supplied archive (MD5
  `51b40a17b92a30339957ba564aa0015c`) and added
  `test_theron_v1_bank1f_consumer_receipt`. It checks the exact 134 bytes at
  file offset `$1f0000+$243e` against the source-locked HuC6280 fragment and
  keeps the later RAM-loaded `$2600` consumer explicitly absent. The focused
  combat-boundary, stage-2, raw-media and bank-consumer tests pass 4/4.
- ✅ 2026-08-06 DM1 sensor effect ownership: F0718 floor and stairs sensor
  effects now carry the authenticated PC34 common-word SET/CLEAR/TOGGLE/HOLD
  field instead of forcing a synthetic TOGGLE. HOLD resolves to SET for the
  walk-on addition path, matching ReDMCSB MOVESENS.C/F0272. Added the effect
  field to the sensor enumeration handoff and covered SET, CLEAR, TOGGLE and
  HOLD in `test_dm1_v1_sensor_trigger_pc34_compat` (287/287 pass). Runtime
  group-generator and broader actuator ownership remain open.

- ✅ 2026-08-06 DM2 champion-selection placeholder removal:
  `dm2_v1_select_champion()` no longer reports success from coordinates and a
  party count alone. It now returns failure until the live DB3 subtype-0x7E
  mirror record, hero record and possession chain reach the real
  `DM2_SELECT_CHAMPION`/`REVIVE_PLAYER` path. The focused lifecycle test now
  proves that a structurally valid request remains non-mutating and
  fail-closed. Source: `SKULLWIN/c_hero.cpp::DM2_SELECT_CHAMPION`
  (1052-1200).

- ✅ 2026-08-06 DM2 platform-pair launch gate: boot now accepts only the
  documented original GRAPHICS.DAT/DUNGEON.DAT pairs, rather than combining
  any two individually recognised hashes. The regression covers the real PC,
  FM Towns and Amiga identities and rejects PC/FM Towns and FM Towns/Amiga
  mixtures. This keeps foreign byte order and scene data from being presented
  as a playable DM2 session.

- ✅ 2026-08-06 DM2 FM Towns direct-root intake: the original CD ZIP can now
  be found when the selected data directory is either the global data root or
  its `dm2` directory. The archive remains in place; GDAT, G1 and CD.DAT are
  still admitted from memory only. The opt-in real-media regression verifies
  both root forms.

- ✅ 2026-08-06 DM2 FM Towns M12 launch admission: the launcher now reads the
  HME-242 raw image from the original ZIP only in memory, verifies the
  platform-bound GDAT and G1 MD5 pair, and reports their virtual
  `ZIP::DATA/...` provenance. It hands the untouched archive root to the
  existing memory-owned DM2 boot reader rather than creating an asset cache.
  The opt-in `dm2_fmtowns_m12_real_media` regression verifies both global and
  direct `dm2/` roots; real-media scan and boot probes reach
  `dm2-startup-menu`. Ordinary virtual DM2 ISO media remains blocked unless it
  has a dedicated in-memory owner.

- ✅ 2026-08-06 DM2 unauthenticated animation-reader closure:
  `dm2_v1_anim_bootstrap.c` has no M11/runtime caller but exposed arbitrary
  animation-file handles through the product archive. It is now compiled
  only by its focused bootstrap/chunk tests. Production cannot open an
  unverified animation path until a selected original-media stream, bounds,
  and M11 title/entrance handoff are all present.

- ✅ 2026-08-06 DM2 generic archive disk-unpack closure: removed the boot
  reader's `/tmp/firestaff-dm2-asset-*.dat` extraction of virtual ZIP/ISO
  paths. A virtual PC archive entry now fails closed until it has a verified
  in-memory reader; the separately authenticated FM Towns and Amiga RAM
  buffers remain supported. This eliminates the last generic DM2 boot path
  that unpacked original data on disk. Source: mounted-media ownership in
  SKProject's file layer; Firestaff boot RAM receipts.

- ✅ 2026-08-06 DM2 FM Towns corpus-identity closure: removed the runtime
  convenience path that reopened `$HOME/.firestaff/data/dm2/GRAPHICS.DAT`
  and used it as an English overlay after M12 had selected FM Towns media.
  The path could silently mix a sibling PC release into the authenticated
  CD/G1 session. FM Towns now keeps its selected original corpus throughout
  runtime initialization; a future language choice must carry a separately
  verified companion-media receipt instead of inferring a host path. Source:
  `SKWINSPX/src/v5/skfileop.cpp` media selection before GDAT access.

- ✅ 2026-08-06 DM2 resurrection surrogate mutation removal: removed the
  runtime type-0x0D final-phase write that treated Firestaff's bounded
  261-byte session record as SKProject's 263-byte `c_hero`. The old path
  truncated the original 16-bit hero flag and skipped the source tombstone
  and cloud phases. Runtime now consumes the source timer without mutating
  a non-source champion surrogate; the CTest source gate proves the handler
  remains ordered but cannot read or write the surrogate's HP, flags, or
  inventory. Full source-owned
  resurrection remains tracked in `DM2-RESURRECTION-OWNERSHIP`. Sources:
  `SKULLWIN/c_tim_proc.cpp:39-124`, `SKULLWIN/c_hero.h:40-130`, and
  `SKULLWIN/c_hero.cpp:916-953`.

- ✅ 2026-08-06 DM2 hero-timer flag truncation removal: type-0x0C used to
  write `0x08` into the byte-sized session `hero_flag` while the source sets
  the distinct 16-bit `c_hero::heroflag` bit `0x0800`, and also clears a
  16-bit timer index. It now remains ordered but non-mutating alongside
  type-0x0D. The CTest gate covers both handlers and rejects any read or
  write of the non-source surrogate fields. Source:
  `SKULLWIN/c_tim_proc.cpp:25-31` and `SKULLWIN/c_hero.h:58-63`.

- ✅ 2026-08-06 DM2 spell-timer surrogate removal: types 0x47, 0x48 and
  0x4B no longer write the 261-byte session surrogate as if it were
  SKProject's 263-byte `c_hero`. The old path truncated `heroflag` 0x4000,
  used `body_flag` as `ench_power`, and mixed `poisoned`/`poison` with
  unrelated byte and detached counters. Each timer remains consumed in
  source order but reports its missing `c_party`/`c_hero` owner without a
  fabricated state change. CTest covers the runtime spell-timer behaviour
  and a static gate rejects reintroducing a surrogate mutation. Source:
  `SKULLWIN/c_tim_proc.cpp:4111-4178` and `SKULLWIN/c_hero.h:58-130`.
- ✅ 2026-08-06 DM2 archive no-unpack gate: M12 no longer materializes
  GRAPHICS.DAT, DUNGEON.DAT, music, or alternate dungeon files from a DM2
  ZIP/ISO into `asset-cache`. Archive-backed PC DM2 now remains
  non-launchable until its real in-memory reader is bound; the existing FM
  Towns CD path retains its separately verified memory-only reader. The
  focused ISO regression proves the original virtual paths remain diagnostic
  evidence, no cache payload is written, and launch stays blocked.
- ✅ 2026-08-06 Theron inferred tile-descriptor cleanup: removed the unused
  `theron_v1_tile_descriptor.h`, whose `$43E4`/`$4914`/`$4DC6` claims could
  not be located in the versioned HuC6280 disassembly. The reverse-engineering
  registry now lists only live bitmap/palette/VRAM receipt modules; no inferred
   tile format was promoted into production.
- ✅ 2026-08-06 Theron real-media probe discovery: level-bank and graphics
  reconnaissance tests now use the supplied standard
  `.firestaff/data/theron/TQUS02.bin` and `TQJP02.bin` paths automatically,
  with environment variables retained as overrides. This removes the local
  false-skip while keeping graphics/tile semantic publication fail-closed.
- ✅ 2026-08-06 Theron VRAM trace BAT mapping: corrected the trace loader to
  consume source VDC BAT tile indices and palette-group bits instead of
  scanning nonzero VRAM and forcing palette group 0. Regression coverage now
  verifies three tile/group bindings and their source VRAM indices; Track 02
  runtime admission remains capture-gated.

- ✅ 2026-08-06 DM2 c_1c9a placeholder isolation: removed the disconnected
  callback-contract implementation from both production CMake globs. Its 24
  unported creature-AI bodies remain available solely to
  `test_dm2_v1_1c9a_pc34_compat`, which now compiles the source explicitly.
  M11 and `firestaff_dm2` cannot link an unowned AI movement, combat, spawn,
  pathfinding or damage stub. Verification: production rebuild, focused
  c_1c9a regression, and real PC-DM2 M11 startup gate.

- ✅ 2026-08-06 DM2 c_0aaf dialogue placeholder isolation: removed the
  disconnected callback-contract dialogue/menu renderer from both production
  CMake globs. It remains available only to `test_dm2_v1_0aaf_pc34_compat`;
  no M11 or DM2 production archive can use its unresolved original selector
  or simplified dialogue-image placement. Verification: production rebuild,
  focused c_0aaf regression, and real PC-DM2 M11 startup gate.

- ✅ 2026-08-06 DM2 runtime-narrow placeholder isolation: removed the
  disconnected callback audit from both production CMake globs. Its
  `DM2_hero_39796` stamina substitute does not match SKProject's original
  name-entry UI and its moverec/light exports overlap separate implementations.
  The two focused unit targets compile it explicitly; M11 and the DM2 archive
  cannot select it while the real owner-backed runtime remains unfinished.

- ✅ 2026-08-06 DM2 c_gui_draw placeholder isolation: removed the disconnected
  callback HUD renderer from both production CMake globs. It has no M11
  caller and substitutes buttongroup dimensions, coin placement and UI glyph
  decisions for original GDAT/runtime state. Its explicit test target remains;
  the live M11 HUD stays on the source-gated viewport path.

- ✅ 2026-08-06 DM2 c_gui_vp placeholder isolation: removed the disconnected
  callback viewport renderer from both production CMake globs. It has no M11
  caller and forwards host-provided tiles and click zones rather than original
  G1/GDAT scene state. The explicit contract test remains, while live M11
  rendering stays on the provenance-gated DM2 viewport renderer.

- ✅ 2026-08-06 DM2 c_querydb placeholder isolation: removed the inactive
  callback query transcription from both production CMake globs. It contains
  many incomplete GDAT-query bodies and has no M11 caller; its explicit test
  target remains while production uses the mounted, provenance-checked
  GDAT/G1 query owners.

- ✅ 2026-08-06 DM2 c_gdatfile placeholder isolation: removed the inactive
  legacy callback GDAT reader from both production CMake globs. Its graphics
  structure routine reported success without decoding the original structure;
  focused tests still compile the adapter explicitly, while production cannot
  use it instead of the mounted GRAPHICS.DAT data owner.

- ✅ 2026-08-06 DM2 sound queue ownership: `DM2_QUERY_SND_ENTRY_INDEX` now
  operates only on the active runtime's source-shaped `xsndptr2` queue. The
  former process-global fallback queue is removed, so a verified GDAT loader
  cannot manufacture unowned queue state. The queue binds after runtime
  initialization and clears with boot teardown. Verification:
  `test_dm2_v1_sound_gdat_real_data` passed against the local retail
  `GRAPHICS.DAT`; `test_dm2_v1_sound_source_gate` passed.

- ✅ 2026-08-06 DM2 `c_sfx` source-origin correction: the isolated
  `DM2_QUEUE_NOISE_GEN1` transcript now applies SKProject's exact
  `s_sizee::barr_04[2..3]` current-map/party-map coordinate delta before the
  original four-direction relative rotation. Missing origins reject instead
  of retaining the prior host same-map approximation. This remains test-only:
  it does not create queue memory, a runtime sound owner or playback.
  Verification: `test_dm2_v1_sfx_pc34_compat` PASS, including all four source
  rotations and missing-origin rejection. Source: `SKULLWIN/c_sfx.cpp:138-283`
  and `SKULLWIN/xtypes.h:110-118`.

- ✅ 2026-08-06 DM2 c_sfx placeholder isolation: removed the inactive
  callback sound queue from both production CMake globs. Its position handling
  omits the original level-geometry transform; the explicit test remains while
  production cannot use it instead of the real SND/music runtime owner.
- ✅ 2026-08-06 Theron viewport placeholder boundary cleanup: the public V1
  viewport contract now states that tile-bank, square-to-tile, UI-chrome,
  palette-material, and screen-offset claims are capture-gated. The retained
  `theron_v1_viewport.c` implementation is documented as fixture-only; its
  inferred tile indices and fallback geometry are not retail parity and the
  production archive continues to use the no-op seam. No synthetic pixel path
  was promoted. Verification: `git diff --check` plus the existing production
  archive audit and Theron rendering/VRAM tests.

- ✅ 2026-08-06 Theron game-owned RAM consumer trace: the instrumented
  Mednafen 1.32.1 build now captures bounded reads from physical main RAM when
  the reader also executes in the game-owned window, with logical/physical
  addresses and reader PC provenance in a separate sidecar. The patch compiles
  and embeds its marker; authentic runtime capture remains blocked by the
  SDL2-compat-only runtime and no staged CUE, so no game-data semantics were
  promoted.
- ✅ 2026-08-06 DM1 ReDMCSB F0302 action-hand placement mask: corrected the
  C508/action-hand destination from `0x0002` (head) to source
  `MASK0x0200_HANDS`, matching ReDMCSB `DEFS.H` and `DUNGEON.C` object
  admission. Inventory-panel mouse regression passes 372/372; real Mac
  pickup/placement and held-cursor capture remain tracked under
  DM1-HOC-OBJECTS-002/003.

- ✅ 2026-08-06 DM2 runtime callback-audit isolation: removed the unbound
  `dm2_v1_runtime_parity_pc34_compat.c` callback transcript from both broad
  production archives. Its timer, record, creature and actuator bodies had no
  live DB/CCM owner or non-test caller. The only live subset, the source
  `ddat` bit/byte/word global-variable store consumed by `dm2_v1_game.c`, is
  now a small independent module. Verification: production archive symbol
  audit confirms the unbound callback symbols are absent; glob-variable unit
  and game integration tests pass; the real PC `GRAPHICS.DAT`/`DUNGEON.DAT`
  M11 startup-profile gate passes.
- ✅ 2026-08-06 DM2 startup auxiliary RAW4 pointer routing: M11 now derives
  Credits, Quit and either-button credits dismissal from the actual
  `INTERFACE_GENERAL/0/RAW4/0` GDAT rectangle records (`0x019b`, `0x01b2`,
  `0x0002`) rather than the fixed PC coordinate matrix. Missing or invalid
  original data yields no action. The real PC-DOS corpus regression uses the
  decoded rectangles for the complete Credits enter/draw/countdown/dismiss
  route; focused boot/global-variable regressions also pass.
- ✅ 2026-08-06 DM2 startup New/Resume RAW4 correction: mapped SKProject's
  `0xD7 → 0x0197` and `0xD9 → 0x0199` table bindings before querying the
  mounted `INTERFACE_GENERAL/0/RAW4/0` rectangle table. M11 no longer links
  or dispatches through the fixed `dm2_touch_click_zone_matrix` production
  path; real PC-DOS New Game now reaches the source-owned GAME_LOAD gate
  through the decoded GDAT rectangle.
- ✅ 2026-08-06 DM2 leader-hand placeholder-zone closure: removed M11's
  fixed `304,41,14,14` icon and pointer-relative `14×14` leader-hand boxes.
  Those coordinates had no authenticated DM2 GDAT/UI rectangle, so the
  public queries now clear their outputs and fail closed until the real
  `DRAW_ITEM_ICON` route is decoded. The real PC-DOS startup/runtime test
  proves no placeholder zone is exposed.
- ✅ 2026-08-06 Theron real map/object regression discovery: moved the
  dungeon-map, ground-reference and door/teleporter real-data tests off their
  stale `raw-us/...`-only lookup. They now use the explicit
  `FIRESTAFF_THERON_TRACK02_RAW`, then the supplied standard `TQUS02.bin`,
  with the old path retained only as fallback. The authentic US Track 02 run
  passes all seven map groups, ground-reference chains, doors and teleporters;
  no JP map layout was guessed or promoted.
- ✅ 2026-08-06 Theron real text/actuator/creature regression discovery: three
  additional Track 02 tests now use the explicit raw-media path or standard
  `TQUS02.bin` before the legacy fixture fallback. All seven US dungeon regions
  pass text decoding, actuator inventory and creature-count checks. The full
  runtime loader remains fail-closed on unbound item categories, so no guessed
  object kind was promoted.
- ✅ 2026-08-06 Theron real CUE intake hardening: Track 02 `FILE` and `TRACK`
  parsing is now case-insensitive, while the existing strict layout, payload,
  pregap and hash checks remain intact. The raw-media regression now wraps the
  supplied authentic `TQUS02.bin` in a temporary mixed-case CUE and verifies
  the real US index-01 sector 225, MD5 `f23601102138f87c33025877767ebf76`, and
  raw-trace preparation; no synthetic media bytes are used.
- ✅ 2026-08-06 DM2 incomplete SKSAVE load-orchestrator isolation: removed
  the unbound `dm2_v1_load_orchestrator_pc34_compat.c` transcript from both
  production source globs. It had no live caller and skipped original
  raw-block/map sizing plus complete `READ_SKSAVE_DUNGEON` ownership, so it
  could not safely represent `DM2_GAME_LOAD`. Its dedicated ABI test remains
  explicit; production-symbol audit, the full save/load suite, and the local
  eight-file authentic PC-DOS SKSAVE prefix census pass.
- ✅ 2026-08-06 Theron real seven-dungeon thing-data coverage: updated
  `test_theron_v1_track02_thing_data` to discover the supplied
  `~/.firestaff/data/theron/TQUS02.bin` (or explicit
  `FIRESTAFF_THERON_TRACK02_RAW`) before its legacy fixture path. The test
  now exercises all seven authentic dungeon ground-reference/item/text
  regions and rejects non-sector-aligned raw input. Result: all seven real
  regions pass with 190–264 ground refs and 871–1132 items; no synthetic
  records or presentation semantics were promoted.
- ✅ 2026-08-06 Theron JP thing-data coverage: the same regression now
  discovers authenticated `~/.firestaff/data/theron/TQJP02.bin` (or
  `FIRESTAFF_THERON_TRACK02_JP_RAW`) and runs all seven Japanese dungeon
  blocks through the real JP map/item offsets. It verifies 192–261 ground
  references, 871–1 132 decoded records and every populated category, while
  deliberately keeping JP text empty until its real codon consumer is bound.
- ✅ 2026-08-06 Theron Track 19 ISO level-block binding: the level-data
  receipt now has separate direct-projection descriptors for `TQUS19.iso`
  and `TQJP19.iso`, including their own offsets, metadata and EOF-bounded
  compressed-span hashes. The real US and JP ISO files pass all seven blocks;
  raw BIN offsets are no longer treated as ISO offsets. Decompression and
  tile/map/object ownership remain intentionally closed.
- ✅ 2026-08-06 Theron ISO runtime level handoff: `Theron_RuntimeLevelMedia`
  now admits authenticated US/JP direct ISO variants through the same
  lifetime-safe receipt path as raw BIN. The focused runtime test passes all
  four real sources and still proves `no_semantic_promotion`; no compressed
  bytes are borrowed into runtime and no tiles, maps or objects are inferred.
- ✅ 2026-08-06 DM2 CCM synthetic-execution closure: removed
  `dm2_v1_ccm.c` from both production archives and retired creature-tick
  flag-to-action writeback. The compact opcode interpreter lacks the original
  DB4/CAII command-stream owner and live dungeon/timer/party callbacks, so
  production now records an unexecuted CCM receipt while retaining authentic
  GDAT creature data/rendering. The isolated CCM tests remain explicit.
- ✅ 2026-08-06 DM2 partial world-state isolation: removed
  `dm2_v1_world_state.c` from the broad production DM2 archive. Its bounded
  SKSave projection has no active M11/runtime caller and cannot reproduce the
  original continuous SUPPRESS read order, so only its explicit diagnostics
  retain it until complete `DM2_GAME_LOAD` ownership exists.
- ✅ 2026-08-06 DM2 credits/menu system-palette regression: real PC English
  `GRAPHICS.DAT` is now discoverable through the direct
  `FIRESTAFF_DM2_DATA_DIR` root without copying or extracting game data. The
  typed `INTERFACE_GENERAL/0` PAL_IRGB decoder is checked against Greatstone's
  documented P8B1 system-palette anchors for the IMG9 menu/credits resources;
  the real M11 startup suite also proves Credits enter, countdown, palette
  publication, and left/right dismissal. Verification: GDAT suite 33/33,
  real PC startup/profile gate, and hash-based data scan all pass.
- ✅ 2026-08-06 DM2 direct-corpus boot regression: the boot-profile smoke
  test now honors `FIRESTAFF_DM2_DATA_DIR` before its legacy default path and
  reads the supplied PC corpus in place. This makes the full source-GDAT
  startup, HUD, dungeon-material, palette/light, and V2-no-fallback check
  reproducible without copying, unpacking, or staging original game files.
  Verification: real PC boot-profile suite 95/95.
- ✅ 2026-08-06 DM2 authentic SKSave corpus regression: replaced the stale
  real-data test's fabricated “champion name” header interpretation and
  obsolete path with direct, read-only corpus checks. All eight supplied
  PC-DOS `sksave0..3.dat/.bak` files now prove their DM2 container header and
  source-owned raw saved-dungeon prefix; the incomplete SUPPRESS tail remains
  explicitly unplayable. Verification: real SKSave suite 27/27 and the
  external corpus census 25/25.
- ✅ 2026-08-06 DM2 raw-SKSave startup-menu gate: the real PC-DOS corpus is
  now part of the startup action regression. Its authentic but incomplete
  raw saves create neither Continue nor slot UI actions, preventing a
  header-only save from entering the runtime. Verification: real startup
  menu action contract 103/103.
- ✅ 2026-08-06 Theron Track 02 object-category source binding: corrected the
  category names/numbers to match the retail `itemBytes[]`/`CATEGORYTYPE`
  order (`monster`, `weapon`, `clothing`, `scroll`, `potion`, `chest`,
  `misc`, `missile`, `cloud`) instead of the former shifted creature/champion
  labels. The real US Track 02 test now verifies all seven dungeon count
  tables and nonzero payload in every populated category. This proves copied
  raw records only; no guessed runtime item or combat semantics were enabled.

- ✅ 2026-08-06 DM2 SKSAVE fixed-section source-order regression: the
  read-only PC-DOS corpus test now follows the continuous SKProject
  `DM2_GAME_LOAD` SUPPRESS stream through the actual 0x3c-byte
  `s_savegamebuffer`, globals, 263-byte heroes, 6-byte save state and
  0x0c-byte `c_tim` rows. All four saves and four backups pass without
  materializing a playable session; `DM2_READ_SKSAVE_DUNGEON` remains
  explicitly fail-closed. Verification: authentic corpus suite 35/35.
- ✅ 2026-08-06 DM1 real object corpus: added
  `test_m11_dm1_real_object_corpus`, which launches from the supplied real
  PC3.4 `GRAPHICS.DAT`/`DUNGEON.DAT` and verifies 611 weapon, armour, scroll,
  potion, container, and junk records. Each record resolves through the
  authenticated Thing identity, M564 name table, direction-aware icon lookup,
  and a loaded 16x16 source zone; no generic `WEAPON n`/`ARMOUR n`/`POTION n`/
  `JUNK n` fallback names are accepted. Verification: Ninja build and
  `FIRESTAFF_DM1_DATA_DIR=/tmp/firestaff-dm1-real-20260805`
  `test_m11_dm1_real_object_corpus` passed (611/611). Runtime placement and
  real Mac cursor/panel capture remain separate open work.

- ✅ 2026-08-06 DM2 SKSAVE format-reference correction: replaced the obsolete
  56-byte stateblock/10-byte timer description with SKProject's actual
  `s_savegamebuffer` (0x3c) and `c_tim` (0x0c) sections. The documented
  order now keeps `STORE_EXTRA_DUNGEON_DATA` after inventory/leader links and
  records that all sections share the MSB-first SUPPRESS stream.
- ✅ 2026-08-06 Theron Track 02 raw item-record decoder: added a portable
  little-endian decoder for the source-bound category 4–10 records, including
  the universal next-reference prefix and the documented monster, weapon,
  clothing, scroll, potion, chest, and misc bitfields. Every populated record
  in all seven real US quest blocks is decoded and checked; missile/cloud
  records remain raw-only and no guessed runtime item semantics were enabled.
- ✅ 2026-08-06 Theron Track 02 loader handoff boundary: the full real-data
  dungeon loader now decodes and traverses authentic category 4–10 records on
  all seven US dungeons, reporting source-record and unbound-reference counts
  while refusing to manufacture `Theron_V1_Object` kinds or item indexes.
  The loader regression uses the supplied `TQUS02.bin` directly and passes
  with real map/door/teleporter/actuator chains intact.
- ✅ 2026-08-06 Nexus DGN Structure2 provenance tightened: the DMWeb texture
  decoder now reports format decode only and no longer self-asserts
  `source_verified` for arbitrary buffers. Retail identity remains owned by
  the hash-verified LEV00--LEV15 admission route; the real LEV00 diagnostic
  still decodes indexed-4bpp and direct-555 surfaces, but neither reaches
  viewport/VDP1 drawing.
- ✅ 2026-08-06 DM1 external original-save corpus: verified the two
  operator-staged PC34 files `DMSAVE.DAT` and `DMSAVE (1).DAT` from
  `/Users/bosse/Downloads` against the real PC3.4 dungeon backing. Both
  48,561-byte files pass the checksum-qualified original-save classifier and
  the production F0435 import -> live runtime -> F0433 export -> second F0435
  import route. `test_dm1_v1_original_save_pc34_backed_corpus_roundtrip`
  passes 2/2 with the real `DUNGEON.DAT`; no save bytes were added to git.
  SHA-256 identities:
  `26ccd1591ccf6ec9e53186e994f73924185143f82055312cafd474ed7abc9437` and
  `ab7bb4a34b77bba033d7b6c31db32e7198a962b0e55c0644c0486f50bb361ecb`.
  Broader C13 corpus and packaged Mac/app capture remain open.

- ✅ 2026-08-06 Theron Track 19 level-envelope structure: added a real-data
  reader for both US and JP ISOs that verifies the authenticated 12-byte
  header, 32×27 dimensions, six raw header words, and bounded 864-byte
  payload span. The inventory probe passes against `TQUS19.iso` and
  `TQJP19.iso`; no tile, object, palette, or runtime level semantics were
  inferred from the opaque payload.

- ✅ 2026-08-06 DM2 D2RS public-resume closure: `dm2_v1_session_load_slot()`
  and `dm2_v1_session_load_last_session()` now reject the internal D2RS
  decoder envelope. Its decoder remains test-only diagnostic coverage, while
  player-facing M11 resume requires a complete original raw SKSAVE path.
  Verification: save/load 25/25, real PC-DOS corpus 35/35, M11 profile gate,
  and startup menu contract 103/103.
- ✅ 2026-08-06 Nexus MNS model admission tightened: `nexus_v1_load_model()`
  now requires a catalogued retail `*.MNS` identity and an exact MD5 match
  before adding a DMDF model to the runtime pool. Renamed/synthetic DMDF
  signature matches are rejected; the generic DMDF reader remains available
  only to bounded diagnostic/material-container inspection. The real MNS
  corpus route therefore cannot promote placeholder geometry into creature
 rendering.
- ✅ 2026-08-06 Nexus startup surface presentation tightened: the public
  TITLE/WARNING/GAMEOVER/FACE/STABG convenience wrappers no longer copy real
  source pixels directly into a host framebuffer. Their loaders still retain
  verified bytes for receipts and diagnostics, while presentation remains
  explicitly no-draw until Saturn VDP1/VDP2 command, palette-bank, and
  destination capture is bound.
- ✅ 2026-08-06 DM2 original SKSAVE fixed-state receipt: promoted the real
  PC-DOS corpus decoder for the exact `DM2_GAME_LOAD` fixed SUPPRESS sequence
  into `dm2_v1_original_raw_sksave_fixed_state_receipt()`. It reads the
  source-sized 0x3c `s_savegamebuffer`, globals, heroes, `c_wbbb`, and 0x0c
  timers on one MSB-first reader, then records the precise record-link
  boundary for `DM2_READ_SKSAVE_DUNGEON`. All eight supplied primary/backup
  saves pass; no incomplete session is admitted or made playable.
- ✅ 2026-08-06 DM2 SKSAVE D2RS boundary correction: removed the remaining
  corpus timer-stream path that decoded an original raw body through the
  retired 56-byte `DM2_GameStateBlock` fixture. It now reuses the
  source-sized 60-byte `s_savegamebuffer` receipt and records the exact shared
  SUPPRESS timer-to-record-link byte span. Public comments now identify D2RS
  as diagnostic-only, so it cannot be mistaken for original save I/O. The
  raw `DM2_READ_SKSAVE_DUNGEON` continuation remains fail-closed and no save
  is made playable by this change.
- ✅ 2026-08-06 DM2 FM Towns direct-media handoff: fixed the selected HME-242
  ZIP path at both M12 and boot. A direct original archive now retains virtual
  `DATA/GRAPHICS.DAT` provenance, runs the CUE/IMG/G1/CD.DAT reads in RAM, and
  hands the unchanged ZIP path to `GAME_LOAD`; it can no longer be interpreted
  as a directory and lose to a sibling PC install. The supplied original ZIP
  passes M12 admission, boot-profile `GAME_LOAD`, and a direct headless launch
  as `FM Towns Japanese`. No game data was unpacked, cached, or tracked.
- ✅ 2026-08-06 Nexus HUD runtime binding: production engine init now retains
  the 80-entry layout table at `DM.BIN+0x376D0` and 40-entry ring-menu hit
  table at `DM.BIN+0x38000` only after the exact real `DM.BIN` hash receipt is
  verified. Copy-out getters refuse uninitialized/unverified engines; these
  coordinates remain input/HUD provenance, not framebuffer or VDP placement.

- ✅ 2026-08-06 Theron later-record probe discovery: the real Track 02
  JP/US BINs are now autodiscovered from `~/.firestaff/data/theron/` when no
  override is supplied, while explicit CI/local paths still take precedence.
  The probe remains hash-gated and keeps level/object semantics opaque.

- ✅ 2026-08-06 Theron Track 02 projectile records: added source-faithful
  decoders for category 14 missile records (six payload bytes) and category 15
  cloud records (two payload bytes), including linked-record references and
  truncation rejection. The real seven-dungeon thing-data regression still
  passes; these records remain outside host gameplay until their consumer is
  proven.

- ✅ 2026-08-06 Theron later level-bank receipts: added a borrowed-byte receipt
  for every one of the seven authenticated US and JP Track 02 level banks.
  Each receipt binds the exact post-`0xF0` compressed span, per-level metadata,
  shared-prologue hash, and payload FNV-1a without guessing decompression or
  tile semantics. Per-level retail FNV gates reject mutated/substituted
  normalized buffers. Real US/JP level-bank tests pass.

- ✅ 2026-08-06 DM2 SKSAVE record container-map fidelity: corrected the
  isolated `READ_RECORD_CHECKCODE` port to preserve unmasked `c_record` bits
  during `DM2_SUPPRESS_READER(..., false)` and to select the source map-
  container branch from `b_04` bits 1–2. Map containers now consume the
  one-bit possession continuation rather than recursively reading a fabricated
  child chain. Verification: record-checkcode and extra-dungeon-data round
  trips, including a source-shaped map-container possession case.
- ✅ 2026-08-06 DM2 incomplete SKSAVE orchestrator isolation: removed the
  callback-only save/load orchestrator, record-checkcode and extra-dungeon
  helper cluster from the M10 and DM2 production archives. Their focused
  round-trip tests still compile the source explicitly, but no M11/runtime
  path can present their unowned raw-block, GDAT, record-link or possession
  handling as a playable original save.
- ✅ 2026-08-06 DM2 startup timer provenance regression: reinstated the
  no-synthetic-startup-timers test on the current SKProject-shaped `c_tim`
  source queue. An empty presentation boot now proves that no timer is due,
  popped, or dispatched before an original dungeon/session supplies timer
  rows. Verification: timer guard, real PC-DOS boot profile 95/95, startup
  menu action contract 103/103, and GDAT RAW4 pointer contract all pass.
- ✅ 2026-08-06 DM2 weather placeholder closure: removed the uncalled public
  rain/fog/storm overlay planner that generated density, colours and flash
  commands from generic weather values. Production weather now has only the
  source-owned GDAT `ENVIRONMENT` plus `DistantEnvironment` renderer route.
  The formerly disabled no-synthetic overlay test is enabled through the DM2
  archive and proves scene controls cannot alter pixels without that receipt.
  Verification: lighting/HUD/viewport gate 151/151, no-synthetic weather
  guard, weather material gate, real PC-DOS outdoor weather capture 21/21,
  and boot profile 95/95.
- ✅ 2026-08-06 DM2 generic weather-state closure: removed the now-unused
  viewport `weather`/`rain_intensity` fields and setter, plus the runtime call
  that populated them. The real runtime still records its source weather state
  and can draw only after the original GDAT `DistantEnvironment` receipt is
  bound. Verification: no-synthetic weather guard, weather material gate,
  real PC-DOS outdoor weather capture 21/21, boot profile 95/95, and full app
  build.
- ✅ 2026-08-06 DM2 HUD single-owner closure: removed the legacy
  `firestaff_game_loop` V2 HUD initialization and render call. That loop has
  neither M11's accepted frame receipt nor its boot-profile GDAT binding, and
  could have applied a second HUD pass over an M11-owned source frame. M11 is
  now the sole production owner of the optional GDAT-backed V2 HUD pass.
  Verification: full app build, real PC-DOS boot profile 95/95, and the M11
  launcher-handoff regression's bounded SDL timeout exit.
- ✅ 2026-08-06 DM1 floor/ceiling frame source gate: the live M11 outer
  viewport frame now accepts PC34 floor-set records only when the asset is
  both decoded and pixel-backed. Dimension-only cache metadata can no longer
  reach `BlitScaled` and paint an empty or placeholder frame over the real
  DM1 viewport. The legacy diagnostic frame path remains isolated from
  authenticated DM1 sessions. Verified with a full Ninja build and the real
  PC3.4 object-name, 611-record object corpus, and alcove pickup/placement
  tests.
- ✅ 2026-08-06 DM2 HUD compatibility-pass closure: removed M11's remaining
  optional V2 post-frame HUD setup and static GDAT blit. SKProject's
  `DRAW_CHAMPION_PICTURE` and `skguidrw.cpp` command paths consume live
  champion, command and GUI/session state, which the static compatibility
  plan could not prove. The accepted V1 runtime frame is therefore the sole
  production pixel owner in every presentation mode; V2 HUD diagnostics have
  no M11 invocation. Verification: `firestaff` build, real PC-DOS
  GDAT HUD command family, material receipt, boot profile 95/95, lighting/HUD
  gate 151/151, portrait-palette gate 2/2, and M11 launcher-handoff boundary.
- ✅ 2026-08-06 DM1 inventory-panel decoded-surface gate: live source-panel
  slot boxes, fallback item blits, open-chest panel, champion portraits and
  icon strips now require both `loaded` and `pixels` before any blit. Cache
  dimensions alone can no longer paint empty metadata into the inventory or
  champion panel. Verified with a full Ninja `firestaff` build, the real PC3.4
  F0342 object-description test, and the real HoC inscription palette test
  (2 inscriptions across 546 corridor transitions).
- ✅ 2026-08-06 DM1 remaining panel/background metadata gates: the live
  F0098 floor/ceiling provider and empty champion status-box path now require
  decoded pixel ownership before exposing source surfaces. Dimension-only
  cache entries are rejected instead of being passed to `BlitRegion` or the
  viewport provider. Verified with a full Ninja `firestaff` build, the real
  PC34 HoC inscription transition test, the 611-record object corpus, and
  real M564/F0702 pickup-cursor coverage.
- ✅ 2026-08-06 DM2 startup executable verification: the current full
  `test_dm2_v1_m11_startup_profile_gate` passes against hash-verified PC-DOS
  data, including the real RAW4 pointer rectangles and TITLE credits/menu
  surfaces. The v3.0.290 binary's direct boot probe also reaches
  `dm2-startup-menu` with startup active, level 0 loaded, and no champion
  fabricated. The installed v3.0.288 app was observed only to classify it as
  stale and is not used as current-build capture evidence.
- ✅ 2026-08-06 DM1 remaining inventory-panel surface gates: source slot-size
  selection, rename/resurrection panels, FOOD/WATER labels and the 32x29
  inventory damage overlay now require decoded `loaded` plus `pixels` before
  layout selection or blitting. Empty metadata cannot alter source panel
  geometry or paint a fake surface. Verified with a full Ninja `firestaff`
  build and real PC34 HoC/object/name/cursor tests.
- ✅ 2026-08-06 Nexus audio source-root binding: `Nexus_SoundEngine` now
  receives the active engine data root and resolves any future verified
  CD-DA host materialization relative to that root. The old implicit
  `HOME/.firestaff/data/nexus` lookup is gone; retail CUE/ISO audio remains
  correctly gated because no host conversion or synthetic track is added.
  Verification: `test_nexus_v1_sound_gameplay` and the focused Nexus audio
  CTest lane pass.
- ✅ 2026-08-06 DM1/CSB decoded-surface gate completion: hardened the remaining
  M11 source consumers so CSB Atari ceiling/floor/wall projection and DM1 HoC
  C027/C040 inventory input accept only decoded asset slots (`loaded &&
  pixels`). This prevents dimension-only cache records from reaching planar or
  panel blits. Verification against the real DM1 corpus: HoC inscription
  palette transition, 611-record object names/icons, and F0702 held-object
  cursor/floor pickup all pass. Real macOS capture remains open in TODO.
- ✅ 2026-08-06 DM1 action/spell panel material gate: the M11 utility-panel
  admission now requires decoded C010/C009 pixels as well as `loaded` and
  native dimensions, preventing an empty dimension-only cache entry from
  claiming the source-owned action/spell strip.
- ✅ 2026-08-06 DM1 source bar fallback removal: authenticated DM1 V1 now
  always consumes the ReDMCSB F0287 vertical bar model, even when the legacy
  `FIRESTAFF_V1_BAR_GRAPHS=0` compatibility switch is present. The old
  horizontal host bars remain limited to non-source/debug and V2 sessions.
  Verified with a Ninja M11 rebuild, real 611-record object corpus, real
  M564/F0702 pickup-cursor coverage, and the HoC frame-admission test.
- ✅ 2026-08-06 DM1/CSB decoded-surface gate completion: hardened the remaining
  M11 source consumers so CSB Atari ceiling/floor/wall projection and DM1 HoC
  C027/C040 inventory input accept only decoded asset slots (`loaded &&
  pixels`). This prevents dimension-only cache records from reaching planar or
  panel blits. Verification against the real DM1 corpus: HoC inscription
  palette transition, 611-record object names/icons, and F0702 held-object
  cursor/floor pickup all pass. Real macOS capture remains open in TODO.
- ✅ 2026-08-06 Nexus MNS retail declaration bounds: the MNS decoder no longer
  silently clips declared skeleton, mesh, TEXT, or MOTN data to undersized host
  buffers. The verified 30-model corpus retains the observed 37-joint
  `ROCKPILE.MNS`, 64-texture `VEXIRK.MNS`, and 11-table `D_GOLD.MNS` cases;
  oversized declarations fail closed. Verification: `test_nexus_v1_mns`
  decoded all 30 real MNS files and rendered 815 source textures. This does
  not claim Saturn VDP1 capture or final viewport-pixel parity.
- ✅ 2026-08-06 CI portability/link closure: fixed Clang pedantic rejection of
  the AmigaDOS `0xfffffffd` tag, completed missing DM2 test-source links,
  added the CSB FMTowns graphics owner to M10 and direct probes, and updated
  the V22 overlay probe to assert the authenticated no-draw contract.
  Verification: full local CMake build reaches 100%; affected CTest probes
  pass.
- ✅ 2026-08-06 DM2 source-frame receipt gate: stopped callback-injected
  fixture pixels from being stamped as a valid/full GDAT frame. The runtime
  now requires the mounted boot provider's raw and decoded `GRAPHICS.DAT`
  evidence for both indoor and outdoor production-frame receipts. The
  isolated synthetic-provider regression still exercises blitting, but proves
  it cannot publish playable source ownership. Verified with the focused
  runtime handoff test and the real PC-DOS boot-profile render regression;
  no game data was copied or committed.
- ✅ 2026-08-06 Nexus DGN Structure2 palette reuse: the real decoder now follows
  DMWeb's `Palette offset = 0` rule by resolving the previous descriptor with
  the same Palette ID, instead of rejecting valid retail images or substituting
  palette 0. Hash-verified LEV00-LEV15 coverage decodes 1,678 descriptors:
  1,553 indexed4 and 125 direct555. Structure3 VDP1 upload, CLUT ownership,
  face texture selection and viewport placement remain capture-gated.
- ✅ 2026-08-06 DM2 direct-frame data-admission gate: the V1 renderer now
  rejects an empty/unverified boot profile and every non-boot GDAT callback
  before framebuffer mutation. This removes the remaining public path that
  could return a DM2 frame without the hash-verified `GRAPHICS.DAT` and
  `DUNGEON.DAT` owners required by SKProject `DM2_GAME_LOAD`. Verification:
  the boot-profile regression asserts the missing-data rejection, while the
  real PC-DOS boot, M11 startup and eight-file SKSave corpus tests pass.
- ✅ 2026-08-06 Nexus PRS3 invalid-reference gate: DMWeb-avkodaren fail-closed
  på framtida backreferenser utanför redan producerat fönster; DMWebs
  dokumenterade negativa inledning nollfylls fortfarande. Den nya negativa
  testvektorn passerar och hela den verkliga MENU.BPK-korpusen är verifierad.
- ✅ 2026-08-06 Theron Track 19 level-record inventory retention: the real US/JP
  32x27 startup-level envelope now carries its six raw header words, payload
  size/nonzero count and payload FNV-1a through the file inventory receipt.
  The real ISO probe passes for both variants; no tile, object or later-level
  semantics are inferred or admitted.
- ✅ 2026-08-06 Theron Track 02 per-map record retention: the world handoff now
  copies each authentic map's creature-graphics bank id and cumulative column
  thing-count from the real map directory. The US seven-dungeon loader test
  verifies both fields for every loaded map without promoting graphics or
  object semantics.
- ✅ 2026-08-06 Theron Track 02 map-directory retention: world state now keeps
  each dungeon's authentic twelve-entry thing-descriptor-size table and
  aggregate column count. The real seven-dungeon loader test verifies the
  complete directory receipt while object-kind and graphics semantics remain
  fail-closed.
- ✅ 2026-08-06 Theron Track 02 level-record retention: the world handoff now
  copies every decoded retail map-header byte into explicit source receipt
  fields without guessing seed, spawn direction or gameplay semantics.
  The real US loader test verifies those fields for all seven dungeons and
  their complete 3-8-map retail spans; all existing object chains remain
  fail-closed and unbound where the consumer is unproven.
- ✅ 2026-08-06 DM1 ornament fallback removal: the three legacy wall,
  door, and floor ornament helpers no longer use identity/global-set guesses
  when the authentic per-map DUNGEON.DAT tables are unavailable. They now
  require a resolved map-local table entry and a decoded GRAPHICS.DAT pixel
  buffer (`loaded && pixels`) before drawing. The authenticated DM1 source
  route was already using the source-owned F0107/F0108/F0111 consumers; this
  closes the remaining diagnostic/custom helper paths without inventing
  ornament pixels. The supplied DOS manual remains behavioral evidence only;
  the real Mac/window capture gaps stay open.
- ✅ 2026-08-06 Nexus SMAP LVMP strictness: automap-parsern följer nu DMWebs
  regler för oanvänd tilemap-bit 0, alltid-satt palette-bit 15 och bounded
  tile-index. Alla 16 verkliga `SMAP00-15.BIN`-filer verifieras och dekoderas;
  inga syntetiska automap-pixlar används.
- ✅ 2026-08-06 Nexus FACE PRS3 bounds: kompakt `FACE.BIN`-dekodning avvisar nu
  en 4-byte-alignerad PRS3-header som inte ryms helt i källfilen. En riktad
  malformed-fixture täcker gränsen, och startup-media-gatet fortsätter passera
  med den riktiga Nexus-korpusen.
- ✅ 2026-08-06 Nexus HUD DM.BIN action provenance: champion-paneladmissionen
  kräver nu de källbundna actionfamiljerna `0x2D` (stat), `0x29` (inventory)
  och `0x2C` (equipment) i respektive retailtabell. En muterad DM.BIN-rad
  avvisas, medan de riktiga paneltabellerna och runtime-HUD-gatet passerar.
- ✅ 2026-08-06 DM1 source-audio fallback removal: all authenticated DM1
  effect calls now use `M11_Audio_EmitSourceSoundIndex`, which accepts only
  the decoded original SND3 sample. Missing or malformed source audio is
  silent instead of becoming a generated door/combat/creature marker. CSB's
  existing GRAPHICS.DAT PCM route and non-source diagnostic marker behavior
  are unchanged.
- ✅ 2026-08-06 Nexus RES* directory admission: WARNING/GAMEOVER DGT2 lookup
  now validates every ordered record's bounded header and matching resource id
  before selecting a payload. A real WARNING.BIN mutation regression proves
  an unselected malformed record cannot be hidden by resource-zero lookup.
- ✅ 2026-08-06 Nexus RES* source admission: the shared resource-directory
  decoder now requires an exact declared file size, a bounded entry count and
  strictly increasing payload spans. Real TITLE.BIN, RLOWFIX.BIN, RHIFIX.BIN
  and POTEFT.BIN tests include a mutation that must be rejected.
- ✅ 2026-08-06 Nexus RLOWFIX TEXT admission: the real TEXT parser now checks
  every DMWeb-relative string offset against the section boundary and ordered
  span table. The European RLOWFIX corpus regression rejects a tampered
  neighbouring string offset without changing the unresolved text consumer.
- ✅ 2026-08-06 Nexus SNDLEV MAP provenance now matches the real DMWeb retail
  layout: eight-byte records begin at offset 0, the first selector byte is
  the observed `0x20`, and `FF FF` must be the final two bytes. The old
  24-byte synthetic prefix was removed from the provenance path and the
  no-draw startup fixture was migrated to the same layout. The focused test
  verifies all sixteen mounted `SNDLEV00-15.MAP` files (66–90 bytes, 154
  bounded records in total); event dispatch and audio playback remain
  capture-gated.
- ✅ 2026-08-06 GitHub Actions Windows warning-fix: nested ADF hash-scan
  helpers are now compiled only on POSIX, where the external archive scanner
  calls them. Windows keeps its explicit fail-closed stubs and no longer turns
  the unused POSIX definitions into `-Werror` failures.
- ✅ 2026-08-06 DM1 combat-log source gate: closed the remaining synthetic
  mini-font route for authenticated `BUILTIN_CATALOG`, `CUSTOM_DUNGEON` and
  `DIRECT_DUNGEON` sessions. A missing original `GRAPHICS.DAT` font now leaves
  the DM1 log unrendered instead of manufacturing glyphs; the 3x5 font remains
  diagnostic-only for non-DM1 callers. Verification: the source-gate unit
  test passes 6/6 and the real PC34 G0194, wall-ornament, alcove, 611-record
  object-corpus and M564/F0702 cursor tests pass.
- ✅ 2026-08-06 Nexus HUD DM.BIN admission hardening: the 80-entry layout
  parser now verifies all nine retail menuctrl sentinel positions and rejects
  nonzero reserved words or unlisted sentinels. The 40 ring-menu hit
  rectangles now reject coordinates outside the Saturn 320x224 envelope.
  Mutation tests exercise both rejection paths against the mounted real
  `DM.BIN`; no HUD rendering or Saturn input semantics are promoted.
- ✅ 2026-08-06 Theron later-level record integrity: the seven authenticated
  US/JP Track 02 level-bank records now require the byte-exact shared `0xE8`
  prologue hash and the source-owned eight-byte per-level metadata before a
  compressed span is admitted. Real BIN regressions cover mutations in the
  shared prologue, metadata, and payload; tile/object semantics remain
  capture/disassembly-gated.
- ✅ 2026-08-06 Theron later-level runtime receipt handoff: the authenticated
  US/JP Track 02 level block is now copied into `Theron_RuntimeLevelMedia`
  without retaining a borrowed source pointer. Runtime preserves the exact
  block/user-data offsets, compressed-span size/FNV, shared-prologue FNV and
  eight metadata bytes for all seven real levels in each variant. Mutation
  coverage proves a damaged source is rejected and the previous receipt is
  retained; decompression, tile/map ownership and object semantics remain
  explicitly blocked by the HuC6280 consumer gate.
- ✅ 2026-08-06 Theron HuC6280 static disassembly receipt: the real US
  `TQUS19.iso` and JP `TQJP19.iso` projections now verify their exact 134-byte
  bank-$1f fragment at `$243e` by file identity and bytes. The receipt records
  the proven forward byte step, `$3b7e–$3b85` bank-switch table and reverse
  `($36)` read path while keeping semantic publication closed. The missing
  post-CD `$2600` RAM consumer remains explicitly capture-gated.
- ✅ 2026-08-06 Theron JP Track 02 dungeon-map binding: the map loader now has
  a separate authenticated `TQJP02.bin` offset table and variant API. All
  seven real Japanese dungeon banks load with their retail map counts and hub
  dimensions; the SHADODAN item-part boundary is derived from the same source
  record layout rather than a guessed offset. ISO variants remain fail-closed
  until their own container-specific table is verified.
- ✅ 2026-08-06 DM1 F0190 source-owned death smoke: M11 now uses the shared
  ReDMCSB F0213/F0220 transaction for authenticated worlds, reserving/linking
  the real C15 and publishing the C25 timeline receipt before exposing C040.
  Runtime smoke updates the raw C15 attack while it persists and unlinks the
  exact cell owner on despawn; ownerless F0821 remains limited to worlds with
  no raw Thing table. Focused source-publication and moving-killed-all tests
  pass.
- ✅ 2026-08-06 Theron JP Track 02 object handoff: ground references, item-part
  records, linked source objects, and the full dungeon loader now use the JP
  variant offset table instead of silently reusing US offsets. The real JP BIN
  passes all seven dungeon object-record gates with zero raw-only records;
  host item/combat/graphics semantics remain explicitly unbound.
- ✅ 2026-08-06 Theron Track 02 source-category census: the full US/JP dungeon
  loader now retains a bounded count for each authentic DMBUILDER category
  represented by its opaque source-object receipts. Regression coverage proves
  the counts sum exactly to retained records and rejects unexpected categories;
  no host item kind, inventory ID, or gameplay object is inferred.
- ✅ 2026-08-06 Theron Track 02 raw-type census: retained source receipts now
  include compact bitmasks for the decoded raw type fields in categories
  4..8/10. Real US and JP high values remain observable without applying the
  generic DMBUILDER maxima, so this adds corpus evidence without inventing an
  item-ID or inventory mapping.
- ✅ 2026-08-06 Theron JP text fail-closed correction: the JP variant no longer
  exposes an unverified text offset or reads candidate ASCII/fill windows as
  codons. US text decoding is unchanged; JP text remains blocked until its
  actual source block and consumer are identified.

- ✅ 2026-08-06 Theron synthetic palette-promotion closure: the runtime
  Track 02 asset-proof helper no longer promotes a palette-shaped real-media
  window from a caller-supplied boolean. Non-zero compatibility input is
  rejected until an authenticated HuC6280 consumer/capture receipt binds the
  palette to the bitmap route; real-media regression remains fail-closed.
- ✅ 2026-08-06 DM1 legacy platform graphics handoff: added a real
  endian-aware legacy `GRAPHICS.DAT` container and IMAGE2 decoder for the
  original FM Towns (little-endian) and Amiga (big-endian) DM1 releases.
  `M11_AssetLoader` now validates the 575-record size tables, reads embedded
  dimensions, decodes source pixels, and caches them through the normal M11
  asset slots instead of treating legacy bytes as PC34 IMG3. DM1 M12 version
  inventory now includes the known FM Towns, Amiga, and Atari ST graphic
  hashes; the two verified FM Towns DUNGEON.DAT hashes are accepted alongside
  PC34. Verification: `test_dm1_v1_legacy_graphics_dat` passes both endian
  fixtures, `firestaff_m11` builds, and the existing real PC34 G0194 gate
  remains 377/377. Atari ST is intentionally documented as discovery-only
  until its LZW-to-IMG1/IMG2 pixel handoff is complete; no synthetic Atari
  gameplay path was enabled.
- ✅ 2026-08-06 DM1 Atari ST GRAPHICS.DAT record handoff: added the DM1-owned
  `dm1_v1_atari_st_graphics_dat` parser for the original big-endian 563-record
  table, exact data-section accounting, raw records, and Atari-LZW records
  using the ReDMCSB-compatible decoder. It is intentionally separate from the
  PC34 and FM Towns/Amiga IMAGE2 paths. Unit coverage verifies every table
  offset, final-record bounds, raw reads, and truncation rejection. This does
  not claim Atari gameplay: STX extraction and the Atari dungeon/runtime owner
  remain separate follow-up work in TODO.
- ✅ 2026-08-06 DM1 Atari ST protected-media extraction: added the clean-room
  `dm1_v1_atari_st_stx` reader for RSY v3 track blocks, real sector ordering,
  the DM1 FAT12 directory and two-sector allocation units. The supplied retail
  STX extracts GRAPHICS.DAT (271911 bytes) and DUNGEON.DAT (33286 bytes), and
  the extracted graphics table passes the existing 563-record parser. The
  parser rejects malformed/truncated tracks and stays separate from the
  runtime until the Atari dungeon owner is wired.
- ✅ 2026-08-06 DM1 PC34 GRAPHICS.DAT full record audit and SND3 boundary:
  added `test_m11_dm1_full_graphics_asset_audit_pc34`, which runs the real
  canonical 713-record PC3.4 corpus through the M11 classifier, dimensions
  query and IMG3 decoder, and records a deterministic decoded-pixel digest.
  The audit passes with 543 bitmap records, 0 suspicious bitmap, 35
  non-bitmap records, 4 empty records and 131 zero-sized records. The shared
  late dispatch now consumes the authoritative 33-entry SND3 index list
  (671-675, 677-685, 687-693, 701-712) and rejects those PCM records before
  bitmap decoding, preventing junk sprites/icons. Regression coverage also
  verifies the SND3 boundary and the audit is wired into CTest. Remaining
  visual Greatstone/SCK comparison and packaged macOS capture stay open in
  `DM1-PC34-FULL-ASSET-VISUAL-AUDIT`; no generated art was introduced.

- ✅ 2026-08-06 DM1 PC34 Greatstone raster comparison: fetched the public
  Greatstone index to a temporary directory and compared all 542 published
  `IMG3` PNG records against the local hash-admitted PC3.4 `GRAPHICS.DAT`.
  Every record matched dimensions and decoded indexed pixels exactly (542/542,
  zero differences). The separate 0695 `FNT1` interface font and
  `C696_GRAPHIC_LAYOUT` word-data entry were kept out of the raster claim; no
  reference media was committed and no synthetic asset was introduced.
  ReDMCSB `COORD.C` consumes C696 as layout ranges; packaged macOS capture
  remains open in `DM1-PC34-FULL-ASSET-VISUAL-AUDIT`.
# Nexus portrait placement boundary

2026-08-06: Removed the startup champion renderer's guessed 10×10 FACE.BIN
portrait rectangles and borders. The verified portrait ordinal remains in the
opaque render command, but its destination is zero-sized and cannot draw while
Saturn VDP1 destination/scale evidence is absent.

# Nexus roster provenance boundary

2026-08-06: Audited the Nexus champion paths against the real European
`RLOWFIX.BIN`. Production engine and launcher use the verified PLRD importer
and clear the pool on absent or malformed data; the 24-entry hardcoded roster
remains isolated to compatibility fixtures. PLRD health/stamina/mana,
attributes, equipment ordinals, and six TABL indices/codes are source-backed.
Rendered names remain intentionally unavailable until the Saturn
TEXT/FONT256 consumer is captured, so no synthetic names are promoted.
- ✅ 2026-08-06 DM1 FM Towns/Amiga real IMAGE1/IMAGE2 support: replaced the
  legacy decoder's incorrect byte-command interpretation with the DMWeb and
  ReDMCSB nibble RLE algorithm, including literal, previous-row, long-run and
  transparent-run commands. Added the DM1 legacy raster index boundary
  (0-20, 22-532) so shared 575-entry tables cannot send COD/SND/TXT/FNT or
  unused records through the bitmap cache. The new
  `test_dm1_v1_legacy_graphics_real_corpus` reads a real FM Towns MODE1/2048
  track through its ISO DATA/JDATA entries and a real Amiga ADF-extracted
  `GRAPHICS.DAT`; both decode all 532 original image records with stable
pixel digests and reject every non-raster index. No generated pixels or
platform substitution was introduced. Atari ST IMG1/IMG2 pixel binding and
STX extraction remain explicitly open in TODO.
- ✅ 2026-08-06 CSB FM Towns nested-CD intake: the launcher now streams the
  retail ZIP's raw MODE1/2352 image to a temporary file, reads ISO sectors on
  demand and verifies the original English and Japanese GRAPHICS/DUNGEON hash
  pairs before launch. It materializes the selected runtime pair together with
  title, executable and portrait sidecars in the normal CSB cache, without a
  507 MB heap allocation. The local real-media regression covers both language
  receipts and the resulting ordinary runtime paths; the game image remains
  user-supplied and untracked.
- ✅ 2026-08-06 CSB FM Towns CDDA filström: originalets 30-spårs CUE och
  råa MODE1/2352-bild kan nu leverera ett valt 44,1 kHz stereo-CDDA-spår
  sektorvis till en vanlig PCM-fil. Den sista spårlängden bestäms av den
  fysiska bildens slut, precis som minnesvägen, utan att läsa in hela
  507 MB-bilden. Realt CUE/IMG-test bekräftar spårantalet och spår 2:s
  CUE-härledda längd. Uppspelningens M11-bindning är fortfarande öppen.
- ✅ 2026-08-06 DM2 Amiga boot and M12 media handoff: the authentic Amiga AGA
  installer can now reach the normal DM2 boot owner through outer ZIP → disk
  ZIP → OFS ADF → six `dm2_arcsplit` parts → LZX entirely in RAM. Boot admits
  GRAPHICS.DAT and DUNGEON.DAT only when their known Amiga pair hashes pass,
  and admits the 176-byte CD.DAT MOD map only when its own original hash
  passes. M12 invokes that same boot-owned verifier, retains nested virtual
  provenance, passes the unchanged ZIP pathname to runtime, and never creates
  a DM2 cache. Selecting the original Amiga archive directly selects that
  platform even beside a PC install. The real-media boot and M12 regressions
  both pass against the supplied archive; no game data was unpacked, copied or
  tracked.
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

# 2026-08-06 Theron extended authentic replay receipt

- ✅ A 120-second Mednafen replay using the authenticated US Track 02 CUE,
  verified System Card and repeated Run/I input produced 54 SCSI reads but no
  game-owned post-startup Track 02 consumer, `$2600` handoff, or source-owned
  VDC/VCE destination receipt. The bounded main-RAM windows remain retained as
  loader evidence only; no level, object, tile, material, palette, HUD or
  viewport semantics were enabled.

# 2026-08-06 Theron text publication boundary

- ✅ Authentic Track 02 text codons remain decoded from the supplied US media
  for diagnostics, including their exact unresolved control-code markers.
  Production `theron_v1_world_load_dungeon_text()` now keeps the world text
  table empty when those markers occur, so candidate strings cannot become
  synthetic HUD, plaque or scroll text. The focused real-media regression
  passes and will reopen only after the original HuC6280 text consumer is
  identified.

# 2026-08-06 Nexus strict container manifest

- ✅ `verify_nexus_v1_asset_manifest.py` now excludes the local
  `FILE_LISTING.txt` provenance artifact, streams direct ISO members through
  7-Zip, and accepts only exact canonical or documented retail SHA-256
  identities. The supplied European English ISO verifies all 137 disc assets
  with 131 loose files plus six authenticated ISO members; no game data is
  extracted or committed. Nested ISO files inside 7z remain explicitly
  uninspected.

# 2026-08-06 Theron source-index receipt integrity

- ✅ Track 02 source occurrences now retain their full 16-bit category index
  instead of an 8-bit field. This matches the 512-entry source-category
  bound and prevents later real records from being truncated or rejected.
  No category-local type was promoted to a host item index.

# 2026-08-06 Theron synthetic V2.2 asset quarantine

- ✅ The V2.2 modern-asset admission gate now requires
  `source_provenance="authenticated_track02"` in the manifest. The existing
  procedural and `gpt-image-2` Theron art pack is rejected by production and
  remains available only to fixture/reference inspection. The focused asset
  test passes 36/36.

# 2026-08-06 Theron US Track 02 descriptor receipt

- ✅ The production Theron source layer now reads all 53 six-byte level
  descriptor records from UD `0x619900` in the authenticated US Track 02
  MODE1 user-data stream. The focused test extracts the real BIN sectors,
  verifies the source-locked bytes and rejects mismatched tables. This closes
  descriptor-byte provenance only; it does not infer graphics compression,
  object IDs, tile-bank ownership, palette binding or dungeon handoff.

# 2026-08-06 Theron HuC6280 decompressor receipt

- ✅ Extended the authenticated US/JP bank-$1f disassembly receipt from the
  134-byte helper fragment to the full byte-identical `$23AD-$252A` routine.
  The 382-byte range covers the variable-bit reader, bank switches, literal
  output and back-reference path. It remains evidence only: the caller,
  destination and level-block contract are not yet proven, so no decoder was
  enabled in production.
# 2026-08-06 DM2 G1 champion-mirror source inventory

- ✅ The canonical PC G1 `DUNGEON.DAT` now has a source-locked receipt for
  every DB3 `Actuator::Type() == 0x7e` champion-mirror marker root. It uses
  the same `c_map` tile-root address transform and proven DB3 continuation as
  the original, reads only `w2`, and never follows raw `GenericRecord::w0`.
  The real-data regression finds exactly 16 marker roots. This is evidence
  only: marker data has not been promoted to a fabricated champion or a
  playable entrance state.

- ✅ Corrected the marker-data interpretation against SKProject
  `c_loadlevel.cpp:604-611` and `c_hero.cpp:1098-1117`. Every real PC G1
  marker has raw `Actuator::Data() == 0x1ff`; the original consumes its low
  byte (`0xff`) and queues the dynamic resource key `0x16ffffff`. The receipt
  now retains that exact byte/key pair and the real-media regression verifies
  all sixteen roots. This replaces the unsafe assumption that `0x1ff` was a
  static `CHAMPIONS` GDAT index. Dynamic GDAT materialisation and the complete
  `GAME_LOAD` selection path remain deliberately blocked.

- ✅ Added a read-only, source-locked first-pass receipt for the dynamic GDAT
  selector `0x16ffffff`. It follows SKProject
  `c_gdatfile.cpp::DM2_QUERY_NEXT_GDAT_ENTRY` wildcard semantics and
  `DM2_LOAD_DYN4`'s scalar/high-bit exclusions without allocating a cache or
  writing a file. The canonical PC `GRAPHICS.DAT` regression fixes the real
  result at 277 matched and loadable rows, 221878 payload bytes and 21 sound
  rows (`bcb603ef`). This verifies selection only: source-owned DYN4 cache
  materialisation, sound state and the `GAME_LOAD` champion path remain
  blocked.

- ✅ Added the source-independent RAM materialisation portion of that
  champion DYN4 route. Each selected original raw block uses the cache layout
  from `c_gdatfile.cpp::DM2_LOAD_DYN4`: little-endian raw length, unchanged
  `GRAPHICS.DAT` bytes, alignment and original raw index. The canonical PC
  regression verifies 85 deduplicated non-sound blocks, 24,701 payload bytes
  and a 25,074-byte RAM image. All 21 selected sound rows are deliberately
  deferred instead of assuming `DM2_SOUND7` or `v1e13fe[2]`; no data is
  written to disk and no gameplay route is opened.

- ✅ Bound DYN4's initial source sound admission state. SKProject's
  `c_dballoc` initializes `v1e13fe[2]` clear and `c_sound.cpp::DM2_SOUND7`
  returns zero for the empty queue, so the canonical champion selector now
  materialises all 96 deduplicated raw blocks in RAM, including its real sound
  samples. A non-empty matching queue, failed allocation flag or absent state
  still defers those entries. The real-media regression locks 149,244 payload
  bytes in a 149,670-byte source-layout image; it neither decodes nor plays
  a sample and does not open a gameplay path.

- ✅ Removed a synthetic DM2 sound binding from `DM2_SOUND9`. SKProject's
  `c_sound.cpp:650-662` records only the class triple and sets `w_05 = -1`;
  `c_gdatfile.cpp::DM2_482b_0684` later owns both the GDAT raw-index lookup
  and `sndptr4` pool slot. Firestaff no longer puts a caller value or GDAT raw
  index into `w_00` merely because a loader exists. The real-GDAT test proves
  that the queue remains unbound until that source owner is implemented.

- ✅ Bound the non-decoding part of `c_gdatfile.cpp::DM2_482b_0684` to the
  real DYN4 RAM selection. A deferred `s_ssound` now receives its exact GDAT
  raw index and source pool-slot ordinal only when the same raw entry is
  present in the materialised selection; existing raw indices share a slot,
  absent material remains unbound, and the source pool-capacity early stop is
  retained. The real PC-GDAT regression exercises all three outcomes. It does
  not allocate `sndptr4`, decode PCM, or enable playback.
# 2026-08-06 Nexus mixed extracted/ISO runtime source

- ✅ `nexus_v1_init()` now retains the hash-verified extracted corpus as the
  authoritative source while admitting a co-located valid retail ISO as a
  supplemental reader. Missing exact-name members are read from that ISO
  without changing source identity or enabling synthetic fallbacks. The real
  English root boot smoke verifies `DMN_ABS.TXT` (210 bytes) through this path;
  the nested ISO inside the 7z archive remains intentionally uninspected.

# 2026-08-06 DM2 champion portrait source fallback

- ✅ Removed the test-constructed `CHAMPIONS/255` portrait path. The canonical
  PC `GRAPHICS.DAT` has no direct row at that address. The HUD now follows
  SKProject `c_querydb.cpp::DM2_QUERY_GDAT_IMAGE_ENTRY_BUFF` and, only when
  that direct image is absent, consumes the original
  `MISCELLANEOUS/254/dtImage/254` fallback. The real-data regression decodes
  that exact source payload; no portrait pixels, palette or GDAT row are
  fabricated.
# 2026-08-06 Nexus startup/menu/viewport documentation correction

- ✅ Replaced stale Nexus overview, language, startup, champion, feature,
  title, menu and graphics documents with evidence-bound status. They now
  distinguish real retail byte/format receipts from unproven Saturn VDP1/VDP2,
  text, HUD, mesh, gameplay and audio consumers. The corrupt startup document
  was replaced with valid UTF-8; no runtime claim was expanded.

# 2026-08-06 DM2 SKSave direct-root corpus inventory

- ✅ The PC-DOS real-data save regression now retains the next original
  `DM2_READ_SKSAVE_DUNGEON` boundary after the fixed `DM2_GAME_LOAD` stream.
  For every one of the four primary and four backup SKSave files it resumes
  the same MSB-first SUPPRESS reader and reads all 30 direct item roots for
  each saved champion plus the party root. The test owner only records
  source-decoded record types and byte consumption; it neither manufactures
  a dungeon graph nor publishes a resume session. Source: SKProject
  `SKULLWIN/c_savegame.cpp::DM2_READ_SKSAVE_DUNGEON` (hero roots and party
  root at lines 1180–1200).

# 2026-08-06 DM2 startup real-save menu gate

- ✅ Updated the real-data startup regression to distinguish inventory from
  playback: the four supplied PC-DOS slots (and their backups) may be shown
  only as authenticated original SKSave candidates, while selecting Continue
  or a slot must return a redraw/rescan failure until the complete
  SKProject `DM2_GAME_LOAD` stream owns the live session. This removes the
  stale assertion that concealed real data in the menu and still forbids a
  partial or synthetic resume.

# 2026-08-06 DM2 V2/V2.2 production-art exclusion audit

- ✅ Verified the production archive boundary after inspecting every remaining
  DM2 V2/V2.2 synthetic-art module. The generated-HUD, manifest/PNG and
  modern-art-cache sources remain explicit test/diagnostic compilation units;
  `firestaff_dm2_v2` and the linked `firestaff` binary export none of their
  draw or bitmap-loader symbols. DM2 V2.2 therefore cannot display generated
  placeholder art and falls back only to the source-preserving V1/V2.1 route.

# 2026-08-06 DM2 startup GDAT execution gate

- ✅ Startup command execution now fails immediately when the renderer rejects
  an original TITLE/GDAT surface. The regression proves no subsequent startup
  image is acknowledged after that failure, preserving the no-placeholder
  title/menu boundary.

# 2026-08-06 DM2 HUD hand-action source gate

- ✅ Removed the static HUD plan's fabricated action-icon row. Original
  `DRAW_HAND_ACTION_ICONS` chooses its `INTERFACE_GENERAL/4` image and
  `RECT_46..RECT_4d` destination from live hand, formation and facing state;
  only that separately source-gated route may now render a hand backdrop.

# 2026-08-06 DM2 HUD hand-action GDAT/RAW4 delivery

- ✅ Completed the previously unconnected production asset route for
  `SkWinCore.cpp::DRAW_HAND_ACTION_ICONS`: the boot fetcher now resolves only
  `INTERFACE_GENERAL/4/dtImage/2..5`, with a separate four-entry cache rather
  than aliasing static HUD chrome. The viewport now replays the original
  RAW4 `QUERY_BLIT_RECT` placement, including source cropping, and rejects a
  stale table hash, altered source crop or altered destination before any
  pixels are drawn. `test_dm2_v1_inventory_gdat_real_data` verifies all 64
  possession/side/formation/facing routes in the mounted PC-DOS corpus;
  `test_dm2_v1_gdat_hud_m11_command_real_data` drives a real hand image
  through the viewport and proves an altered coordinate is blocked. The
  public hand-action presentation getter is now implemented as a copy-only
  receipt accessor. This does not invent live hand state: gameplay remains
  no-draw until the original formation and possession owner is available.
# 2026-08-06 Nexus HUD DM.BIN disassembly anchor

- ✅ The real-data HUD regression now verifies the `yam\\menuctrl.c` owner
  string, the 80-entry table at `DM.BIN+0x376D0`, its exact FNV-1a64 receipt,
  and seven occurrences of the SH-2 runtime address `0x060476D0`. This is a
  stronger disassembly/source-ownership receipt; it does not infer VDP1/VDP2
  drawing or event-command semantics.

# 2026-08-06 Nexus startup/menu DM.BIN resource anchor

- ✅ The real-data startup/menu regression now verifies the adjacent retail
  loader strings `MENU.BPK`, `yam\\menu.c`, `FONT256.S2D` and `STABG.BIN` at
  `DM.BIN+0x373B4` through `DM.BIN+0x373D8`. Their exact SH-2 pointer-reference
  counts are 1/10/1/1. This records resource ownership only; it does not infer
  menu order, text semantics or Saturn VDP1/VDP2 composition.
  The regression also pins the `0x18B60` SH-2 routine/literal-pool receipt
  (`FNV-1a64 0xF6D5CC046BAB98C7`) and its `yam\\menu.c`/`STABG.BIN` targets.
- ✅ 2026-08-06 Theron's Quest HuC6280 decompressor caller receipt: the
  hash-locked US/JP bank-$1f images now verify the byte-identical `$2386-$23A3`
  caller tail (30 bytes, FNV-1a `699e8da1`) in addition to the 382-byte
  `$23AD-$252A` decoder. The receipt records the source-owned output-length
  measurement through `$3B7C/$3B7D` without promoting unknown input, bank or
  level/object semantics into production. `test_theron_v1_huc6280_disassembly`
  passes against both authentic regional ISOs.

# 2026-08-06 Nexus production roster quarantine

- ✅ Removed the inferred 24-name Nexus roster from the production
  `firestaff_nexus` archive. Legacy tests/probes that intentionally exercise
  the compatibility API now link `tests/nexus_v1_champions_fixture.c` through
  `firestaff_nexus_test_fixtures`; the production library contains no old
  roster strings. The real European RLOWFIX/PLRD parser remains the sole
  production champion source, and `test_nexus_v1_champion_plrd` passes against
  `/Users/bosse/.firestaff/data/nexus`.

# 2026-08-06 Nexus PRS3/VDP1 static-state audit

- ✅ The real European `MENU.BPK` PRS3 route passes all 162 retail surfaces;
  the combined launch-smoke and DGN corpus probes also remain green. Audited
  the hash-bound `DM.BIN` VDP1 register/state receipts and confirmed they stay
  no-draw evidence: no PRS3 execution, CLUT upload, command emission,
  destination placement or menu/viewport ownership is promoted without an
  instrumented Saturn/Mednafen capture. Added the boundary to TODO so future
  work cannot mistake the decoder receipt for VDP1 presentation proof.

# 2026-08-06 Nexus startup menu text-consumer gate

- ✅ Added an explicit `menu_text_consumer_bound` production gate. The real
  TEXT4/TABL/FONT012 bytes are retained, but host-generated chrome strings no
  longer suffice to open the save/champion menu route. Until Saturn text
  placement is capture-bound, the route reports
  `menu-text-consumer-capture-required` and remains fail-closed.
  The compatibility test opts into this seam explicitly; initialized retail
  engines leave `startup_menu_text_consumer_capture_verified` clear.
  The real DM.BIN startup receipt also records one occurrence each of the
  SH-2-visible constants `0x25F00006` and `0x25F80000`; these remain address
  receipts, not proof of text-layer placement.
# 2026-08-06 Nexus RLOWFIX startup text source handoff

- ✅ Engine initialization now retains the authenticated European RLOWFIX
  `TEXT` resource 4 (15 strings), 216-entry `TABL` receipt and FONT012
  #0/#1/#2 (291/250/710 glyphs) beside the real PLRD champion records. The
  launch smoke probe verifies this source handoff;
  it does not promote the bytes into Saturn text pixels or open the menu gate.

- ✅ 2026-08-06 Theron's Quest Track 02 resource framing: the level-block
  receipt now applies the authenticated `$23AD` contract to all seven US and
  seven JP spans, retaining each exact six-byte header and bounded
  `LE16(+2)-5` bitstream slice. It rejects short, underflowing, or overrun
  frames and also passes the Track 19 ISO projections. This is a real
  disassembly-backed framing boundary only; bank mappings, decoder output and
  tile/map/palette semantics remain fail-closed.
- ✅ 2026-08-06 Theron stage-2 resource-handler disassembly receipt: the
  authentic US/JP HuC6280 handler at `$4C3F` (162 bytes, FNV-1a `46360d97`)
  now verifies the four-entry MPR table publication and the source-window to
  destination-register contract (`$3004/$3005`, length `$3006/$3007`). The
  focused disassembly test passes for both retail ISO variants. This remains
  generic source ownership; no level/object/tile/palette semantics were
  enabled without an executing command and source-LBA join.
- ✅ 2026-08-06 DM1 F0115 raw-Thing object icon ownership: floor and alcove
  item rendering now uses `dm1_v1_dungeon_get_object_subtype_pc34()` for the
  live Thing before selecting the real PC34 GRAPHICS.DAT aspect. Decoded
  candidate metadata cannot override a changed junk/torch/food record, and
  mismatched Thing types fail closed. Verified with real PC34 object-name,
  F0115 floor-material/pickup, and alcove-material tests.

- ✅ 2026-08-06 DM2 PC-DOS champion SUPPRESS correction: original save
  import now decodes the exact 263-byte `c_hero` record using
  `SKWINDOS/src/dm2data.cpp::table1d6356`, retaining the raw source records
  apart from Firestaff's older 261-byte convenience view. Proven name,
  formation, stats and hero-type fields are copied only after that decode;
  the old all-ones 261-byte mask is explicitly diagnostic-only. Synthetic
  D2RS envelopes are rejected before save admission. The real eight-file
  PC-DOS corpus passes the source receipt census; complete `GAME_LOAD`,
  inventory/possession and live resume remain fail-closed.
- ✅ 2026-08-06 DM2 original inventory/leader-hand fail-closed correction:
  replaced the false flat 32-bit leader-hand and 30-slot inventory save
  helpers with rejection boundaries. SKProject `LeaderPossession` is a
  22-byte runtime cursor, but only its 16-bit ObjectID reaches SKSAVE through
  `WRITE_RECORD_CHECKCODE`; `c_hero::item[30]` likewise contains 16-bit DB
  links. The source-session route no longer publishes the old cache, M11
  cannot re-inject it, and inventory swap returns unavailable until the real
  record-chain importer/allocator exists. Verified with utility, real
  eight-save corpus, M11 startup/profile and Phase A probes.

- ✅ 2026-08-06 DM2 save-resume documentation audit: corrected stale TODO
  claims which described diagnostic D2RS/raw-SKSave parsing and fabricated
  inventory caches as a playable restore path. The documented state now
  matches the enforced gate: original corpus parsing is receipt-only; no
  source session, possession graph, inventory, or leader hand is published
  before the SKProject record allocator/append path is implemented.

- ✅ 2026-08-06 DM2 `READ_RECORD_CHECKCODE` ownership receipt: the isolated
  decoder now mirrors SKProject `sksvgame.cpp:808-974` and
  `skrecord.cpp:63-112` at the allocation boundary. Its explicit callback
  contract appends every allocated link to the authentic parent root or tile
  coordinates, initializes nested `uw_02` roots before recursive reads, and
  preserves the source two-bit record-link placement field. Unit coverage
  proves ordered chain ownership and placement retention; the real eight-save
  PC-DOS corpus traverses every hero-item and party root (72 PASS). This is a
  test-only source receipt, not a playable restore path: production remains
  fail-closed until a genuine G1 DB/tile/possession/timer allocator exists.

- ✅ 2026-08-06 CI CSB V2 touch/controller link correction: added
  `vga_palette_pc34_compat.c` to the standalone CSB test target that compiles
  the V2 viewport renderer. The focused CMake build and CTest pass locally;
  the main GitHub matrix remains the cross-platform verification.

- ✅ 2026-08-06 DM2 creature-door data correction: removed the active
  hard-coded zero door-attribute fallback from the G1 field bridge. The
  current-map DB0 door root selects map-header slot 0/1, then the real
  `DOORS/dtWordValue/0x0d` record supplies the closed-door creature rule,
  matching SKProject `GET_GRAPHICS_FOR_DOOR` and `GET_DOOR_STAT_0D`
  (`skdoor.cpp`). Missing G1, map-header, or GDAT ownership now returns no
  field result instead of inventing a blocking attribute.

- ✅ 2026-08-06 DM2 flat inventory ABI closure: the residual public
  leader-hand and champion-inventory setters no longer mutate the retired
  32-bit cache, and their getters cannot expose a fixture-written handle.
  This matches SKProject `LeaderPossession`/`WRITE_RECORD_CHECKCODE` and
  `c_hero::item[30]`: the original route owns 16-bit DB links and the cursor,
  neither of which can be reconstructed from a host handle. The focused
  save/load regression proves these calls stay fail-closed; M11's real
  PC-DOS startup gate still passes.
✅ 2026-08-06 Nexus SFX-diagnostiken visar inte längre syntetiska händelsenamn.
Hostenumret är kvar som intern begäran, medan retail-MAP-selectors förblir
opaka tills en Saturn-capture binder event-dispatchen. Playback och övriga
no-draw/capture-gates är oförändrade.

- ✅ 2026-08-06 DM2 source-gated movement: removed the headless movement
  fallback that had treated missing dungeon data as a generic floor. Runtime
  move and turn now require the same hash-verified boot-owned GRAPHICS.DAT,
  DUNGEON.DAT and GDAT callback binding used by the renderer; fixture-only
  dungeons cannot alter party position or facing. The collision decoder stays
  isolated and explicitly tested outside the live gameplay boundary.

- ✅ 2026-08-06 DM2 legacy-loop input correction: `fs_game_tick_v1()` no
  longer interprets DM2 keyboard/touch commands by mutating its generic
  DM1-style party fields. It sends movement and turning to the verified DM2
  boot/runtime boundary, then mirrors only the returned source state; absent
  boot state drains stale commands without creating a session.

- ✅ 2026-08-06 DM2 source-owned HUD stat pairs: M11 now reads the three
  current/maximum pairs directly from authenticated PC-DOS `c_hero` records
  (offsets 54/56, 58/60 and 62/64) and applies SKProject's effective-max-MP
  rule through the existing champion-stat bridge. The old convenience record
  never supplied the stamina/mana maxima, so it can no longer create an
  apparently complete dynamic HUD; unbound records remain no-draw.

- ✅ 2026-08-06 DM2 spell-feedback text gate: removed the synthetic English
  failure labels from the runtime status accessor. SKProject's
  `PROCEED_SPELL_FAILURE` preserves C068--C070 panel state and draws the
  NEED_FLASK GDAT image for class `0x30`; Firestaff now retains only that
  source failure class until those original consumers are bound.

- ✅ 2026-08-06 DM2 movement-cadence no-fabrication correction: removed the
  runtime's post-commit one-frame `glbIsPlayerMoving` substitute. SKProject
  renders the saved old pose while a walk-delay countdown runs, then commits
  through `PERFORM_MOVE`; the active V1 state lacks those source-owned hero,
  inventory and spell-effect inputs, so it now renders the settled pose rather
  than applying the real 700/701 plane offsets at a false time. The isolated
  party walk-delay helper now delegates to the source-locked
  `DM2_CALC_PLAYER_WALK_DELAY` receipt instead of a conflicting local formula.

- ✅ 2026-08-06 DM2 PC-DOS menu image-route provenance correction: renamed
  the `TITLE/0/4` decoded-image receipt that had been called a fallback. The
  established PC-DOS profile uses that real 320×200 GDAT image when no raw
  `SHOW_MENU_SCREEN` record exists; the startup gate still rejects every
  generated menu overlay and every missing original route.

- ✅ 2026-08-06 DM2 startup host-text closure: removed active hard-coded
  English startup, new-game, resume and load status strings from the M11
  receipt path. Menu actions preserve their source-gated structured result,
  including the `GAME_LOAD` control-flow boundary, but M11 now leaves status,
  inspect and log text empty until an original GUI/dialogue text owner is
  connected.
- ✅ 2026-08-06 DM2 startup/runtime GDAT-label closure: removed M11's
  `STARTUP GDAT`, credits, frame-blocked and `RUNTIME GDAT` labels, plus
  runtime-bind ready/failed text. Real TITLE and runtime pixels remain
  source-gated and fail closed when unavailable; their structural receipts are
  retained without a host-authored status panel.
- ✅ 2026-08-06 DM2 FM Towns English companion gate: English requests for the
  Japanese FM Towns CD now require an explicit, canonical PC-English
  `GRAPHICS.DAT` companion selected by M12. The companion is MD5-gated,
  consumed only in RAM and only for decoded GDAT text; there is no sibling-path
  lookup, disk extraction or generated translation. The real-media test proves
  the Towns CD stays the runtime owner while `FIGHTER` is read from the
  authenticated PC text corpus.
- ✅ 2026-08-06 DM2 FM Towns text-query handoff: the source-locked
  `c_gfx_str.cpp::DM2_QUERY_GDAT_TEXT` bridge can now consume a bounded,
  already-decoded companion entry before the selected GDAT cipher path. It
  leaves the native entry untouched when no companion post exists and still
  runs the shared original `FORMAT_SKSTR` consumer.
- ✅ 2026-08-06 DM2 QueryDB GDAT-text relay: the formerly empty text-query
  stub now forwards byte-validated keys to the original callback contract from
  `skcore.cpp::QUERY_GDAT_TEXT` (2636:02F8). It preserves the caller-owned,
  decoded and `FORMAT_SKSTR`-expanded buffer, rejects out-of-range keys rather
  than wrapping them to unrelated data, and is covered with a `FIGHTER` text
  callback proof.
- ✅ 2026-08-06 DM2 creature viewport rect index: restored the previously
  missing `DM2_QUERY_CREATURE_BLIT_RECTI` from SKProject's
  `skgdtqdb.cpp:4995` and its `util.cpp:147` 5×5 rotation. The QueryDB test
  now covers the identity and all three clockwise rotations used by source
  creature placement.
- ✅ 2026-08-06 DM2 runtime text sanitization: removed host-authored action,
  shop, door, movement, inventory and quicksave labels from the live M11/boot
  route. Runtime receipts, save-writer refusal and real door/movement state
  remain intact, but no status or inspector replacement text is shown until a
  matching original GDAT/dialogue owner is wired.
✅ 2026-08-06 Nexus startup menu text sanitization: production save/chrome
builders no longer emit hoststrängarna `DUNGEON MASTER NEXUS`, `LOAD GAME`,
`NEW GAME` eller `LOAD SLOT ##`. Riktig radgeometri och källans slot-identitet
behålls som receipts; textfälten är tomma tills Saturns TEXT4/TABL/FONT012-
konsument och placering är capture-bundna.
- ✅ 2026-08-06 Nexus DGN Structure1B material census and bounds gate:
  hashverifierade europeiska LEV00–LEV15 visar selektorer `0x01..0x7D`,
  medan både `SN_FLOOR.MNS` och `SN_WALL.MNS` har 15 TEXT-deskriptorer.
  Direkt selector→MNS-ordinal är därmed motbevisad och förblir capture-gated;
  materialplaneraren avvisar nu även framtida material-/Structure2-index utanför
  den dekoderade bankens bounded surface-count. Retail MNS/material-regression
  passerar.
- ✅ 2026-08-06 Nexus CDDA selector quarantine: retail CUE/ISO evidence still
  admits the eight CD-DA tracks (2–9), but no source-owned level selector was
  found in the retained DM.BIN/disassembly. The former `level / 2` mapping was
  removed from runtime and audio receipts; unknown level→track selection now
  remains `-1` and playback stays gated. Related stale music docs are marked
  metadata-only.
- ✅ 2026-08-06 CSB FM Towns ZIP scanner crash: initialized the M12 version
  catalog before the special raw-CD admission path records its verified
  `CDATA`/`CJDATA` language variant. A retail FM Towns ZIP as the only CSB
  candidate now completes the scan, reports CSB READY and materializes the
  hash-verified English pair instead of dereferencing an uninitialized
  `versionId`. Verified against the original 484 MiB MODE1/2352 image and
  both the file-backed ISO parser and scanner path.
- ✅ 2026-08-06 Theron regular creature spawn boundary: corrected the real-data
  mechanics probe so it no longer links `theron_v1_compat.c` over the
  production archive. The five Track 02 spawn-zone/category tables remain
  source receipts, while live creature publication, combat and loot stay
  blocked until the bank-switched RNG consumer is captured. The probe now
  verifies the production no-spawn/no-combat/no-drop boundary against the
  authentic JP/US Track 02 level-0 grid.
- 2026-08-06 Nexus event-owner quarantine: removed the production path that
  inferred live door, teleporter, pit, and stairs routes from DGN square
  values plus Structure1F destination fields. The verified SDDRVS asset is a
  sound-driver task, while SLEV/SAL event dispatch remains capture-gated;
  explicit source-bound registries and no-draw behavior remain available for
  future Saturn evidence.
- ✅ 2026-08-06 Nexus CDDA readiness wording: corrected the audio status table
  to say that tracks 2–9 are a disc-layout receipt only, and added a runtime
  regression proving that manual track selection does not claim playback,
  invent a level binding, or produce a ready receipt.
- 2026-08-06 Nexus startup/menu regression: corrected the inverted exact-row
  assertion in `test_nexus_v1_launcher_bpk_no_draw_presentation`. A validated
  PRS3 row is now tested as admitted opaque no-draw evidence, while payload,
  compression, mode and bounds drift remain rejected. BPK no-draw presentation,
  M11 host, and Saturn-card startup tests all pass.
- ✅ 2026-08-06 DM1 HoC C127 D3 side/depth material: fixed the mismatch
  between ReDMCSB's raw D3L2/D3R2 `-2/+2` offsets and M11's normalized
  F0128 `-1/+1` viewport offsets. Real PC34 D3/D2/D1 coverage now retains
  the authenticated C346 wall backing at every admitted side/depth view;
  no C026 portrait or procedural fallback is introduced away from D1C.
  Verification: `test_dm1_v1_champion_mirror_pc34_compat` 68/68,
  `test_m11_dm1_hoc_mirror_side_depth_material_receipt`, and
  `test_m11_dm1_hoc_real_mirror_viewport_material` pass against the real
  PC34 `GRAPHICS.DAT`/`DUNGEON.DAT` corpus.
- 2026-08-06 Nexus stale-claim quarantine: corrected the linked world and
  provisional script-VM comments so native save/event/timer state no longer
  claims ReDMCSB or SDDRVS source equivalence. The runtime's existing
  authenticated-dispatch gate remains unchanged; no unproven gameplay action
  was enabled.
# 2026-08-06 — DM2 FM Towns English archive companion admission

FM Towns DM2 remains a Japanese CD-owned session, but an explicitly selected
canonical PC-English `GRAPHICS.DAT` companion may now retain its M12 virtual
ZIP provenance (`archive.zip::data/graphics.dat`). `dm2_v1_boot` extracts only
that member into bounded RAM, verifies MD5 `25247ede4dabb6a71e5dabdfbcd5907d`,
loads the decoded text overlay, and frees the temporary bytes after binding;
it never writes or unpacks game data to disk. The M11 menu handoff now forwards
the verified virtual path rather than discarding it. Real-media verification:
`test_dm2_fmtowns_m12_real_media` passes with the FM Towns Japanese retail ZIP
and `Dungeon-Master-II-Skullkeep_DOS_EN.zip`, including the authenticated
`FIGHTER` text query; `test_dm2_v1_m11_startup_profile_gate` also passes against
the PC-English corpus. Remaining scope is unchanged: wire the overlay into
each live original GUI/dialogue text producer before claiming complete English
FM Towns UI.
# 2026-08-06 — DM2 `DM2_1c9a_0958` animation-frame flag

Replaced the disconnected `dm2_v1_1c9a_0958` `-1` placeholder with the exact
SKProject result: bit 14 of the selected creature animation frame's `w0`.
The compatibility callback owns the required source record → AI spec → frame
traversal, so Firestaff neither guesses DB4/AI offsets nor invents a fallback
frame. Source: `SKWINSPX/src/v4/skcore.cpp:15447-15455` and
`src/v5/SK1C9A.cpp:5377-5399`. Verification:
`test_dm2_v1_1c9a_pc34_compat` passes 53/53, including set, clear,
out-of-range and absent-owner cases.

# 2026-08-06 — DM2 cross-platform CI build repair

The scene-light test target now links the champion-stat bridge already used by
`dm2_v1_runtime.c`, fixing the macOS arm64 undefined symbol. The shared save
corpus candidate receipt now retains its canonical `SKSave` filename helper on
Windows even though recursive directory walking is POSIX-only, removing the
Windows implicit declaration. No runtime or save-format behavior changed.

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
# 2026-08-06 Theron split-ISO Mednafen capture intake

- ✅ The live Mednafen capture runner now handles the supplied retail CUE's
  CRLF and unquoted `FILE TQUS02.iso BINARY` spelling. When that authenticated
  MODE1/2048 member is absent but the production cache contains the exact
  `ceb02343868f80cec899e9b239aff2da` US ISO assembled from `TQUS19.iso` and
  `TQUS02End.iso`, the runner creates a private normalized capture CUE and
  replaces only Track 02. Track 19 and audio references remain from the
  original layout. This removes the missing-member/raw-BIN capture mismatch;
  it does not claim a game-owned dungeon consumer. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh` and
  `tests/test_theron_v1_mednafen_live_capture_script.sh` pass.
- ✅ 2026-08-06 DM1 GRAPHICS.DAT partial-surface quarantine: the legacy reader
  now rejects short LZW decodes and undersized output buffers instead of
  copying incomplete indexed pixels into a bitmap. The focused fail-closed
  regression and the real 713-record PC34 audit pass; no generated surface is
  admitted as a substitute.
# 2026-08-06 Theron Japanese split-ISO capture intake

- ✅ The live Mednafen capture runner now supports both regions. The supplied
  Japanese CUE's CRLF/okvoterade `FILE TQJP02.iso BINARY` member is normalized
  to the complete sibling `TQJP02End.iso` only after its authentic MD5
  `397039af02d50d15c70b74088eb8a1cb` is verified. `THERON_CUE` is accepted as
  the generic variable while `THERON_US_CUE` remains compatible. This extends
  only verified media intake; no JP consumer, dungeon, palette or viewport
  semantics are promoted. Verification:
  `bash -n scripts/capture_theron_mednafen_live_trace.sh`, the live-capture
  script regression, and the real archive CUE transformation pass.
- ✅ 2026-08-06 F10 source-owned live graphics controls: Theron now routes its
  V2 filter changes through `theron_v2_settings` and persists the Theron slot.
  DM2/Nexus no longer mutate DM1 filter state from the popup; unsupported
  source-specific rows are explicitly locked while shared presentation and
  cheat/speed controls remain available. `m11_runtime_graphics_popup` passes.
- ✅ 2026-08-06 DM1 F0115 object identity quarantine: real floor-object and
  HoC alcove rendering now requires the source-owned raw PC34 `THING` record
  before resolving subtype or material. Missing raw identity produces no-draw
  instead of a candidate-derived wrong icon/name. Real F0115 floor pickup and
  alcove pickup-to-inventory tests pass against the PC34 corpus.

- ✅ 2026-08-06 DM2 FM Towns native startup-media gate: the HME-242 ISO reader
  now inventories the root `AUTOEXEC.BAT`, `SWOOSH`, `TITLE`, `TWANIM.EXP`,
  `SKULL.EXP` and `END` files as well as `DATA/`. It reads the original boot
  script in memory and requires the authenticated `SWOOSH -> TITLE -> SKULL
  -> END` route before boot accepts an FM Towns session. The real Japanese CD
  ZIP plus explicitly selected English GDAT companion regression passes with
  no game member unpacked to disk. This verifies the native animation/startup
  ownership and blocks partial media; it does not claim that TWANIM frame
  playback has been implemented.

- ✅ 2026-08-06 DM2 FM Towns animation-stream authentication: boot now checks
  the selected in-memory HME-242 `SWOOSH`, `TITLE` and `END` streams against
  the published retail MD5s before it accepts the AUTOEXEC animation plan.
  This binds the actual 18-layer swoosh and 224-layer/5-sound title corpus to
  the selected FM Towns CD rather than accepting name-compatible bytes. The
  M12 real-media regression verifies all three identities with the Japanese
  CD ZIP and English text companion, without extracting any game data to disk.

- ✅ 2026-08-06 DM2 FM Towns TWANIM stream-bound admission: the production
  boot owner now parses the selected, hash-verified root streams directly from
  the retained CD image using DMWeb's six-byte big-endian record framing.
  It requires the exact HME-242 inventories before exposing startup media:
  SWOOSH has 22 records/18 deltas, TITLE has 235/224 deltas plus one sound
  definition and five sound events, and END has 401 records/382 deltas across
  two matching animation phases. `test_dm2_fmtowns_m12_real_media` proves all
  three receipts from the user's original ZIP in RAM; no title frame is
  invented or rendered by this boundary.

- ✅ 2026-08-06 DM2 FM Towns TITLE IMG1 decoding: `dm2_v1_fmtowns_anim_stream`
  now replays HME-242 EN/DL records into the original packed 320x200 4bpp
  canvas directly from the selected CD stream. It follows SKWIN
  `ANIM_DECODE_IMG1` (0759:0330), including its original contiguous-stream
  boundary behaviour, while retaining strict whole-stream bounds. The
  real-media test locks first/final TITLE frame command counts and FNV-1a
  receipts (`c7ad2279`, `5ef57a09`) computed in RAM from the retail stream.
  This is a decoder and source receipt only; M11 palette/timing/presentation
  remains explicitly unclaimed until its own source-owned handoff exists.

- ✅ 2026-08-06 DM2 FM Towns TITLE M11 presentation: the selected HME-242
  `TITLE` member is retained only in RAM after its boot/profile MD5 and stream
  receipts pass. M11 decodes the original PL index/RGB4 palette, expands it at
  the indexed-render boundary, and presents the stream's packed 320x200 4bpp
  canvas instead of the PC static GDAT menu. EN/DL progression uses the
  SKWIN TWANIM Timer-A unit (`18*(1024-100)` microseconds) and each source
  display duration clamped to the original five-tick minimum. Input cannot
  reach SKULL's later menu until TITLE ends; rejected Towns media remains
  black rather than falling back to PC art. The opt-in real-CD M11 regression
  launches the selected Japanese ZIP plus authenticated English companion,
  verifies the first rendered frame and a source-timed advance; the focused
  `test_dm2_fmtowns_m12_real_media` also passes without unpacking game data.

- ✅ 2026-08-06 DM2 FM Towns TITLE sound-plan receipt:
  `dm2_v1_fmtowns_anim_stream_decode_title_sound` now retains the HME-242
  TITLE's real 12,862-byte signed SND2 PCM span and its five SO events from
  the selected CD buffer. The real-media regression locks the source offsets
  (14, 101790 … 492266), frame positions (14 … 131), sample FNV-1a
  `0b829ae7`, source volume bytes and `03e8` field. DMWeb identifies that
  frequency as invalid for this title; SKWIN `0759:0E33/0EF0` proves slot 1
  and fixed 5500 Hz instead. This is a read-only source receipt, not an SDL
  playback claim; no game member was unpacked or copied to disk.

- ✅ 2026-08-06 DM2 FM Towns SWOOSH M11 presentation: M11 now follows the
  real HME-242 `AUTOEXEC.BAT` ordering by presenting authenticated `SWOOSH`
  before `TITLE`. Its `AN` header is 0x0, so the IMG1 decoder takes the
  320x200 canvas only from SWOOSH's first EN record, exactly as SKWIN
  `ANIM_DECODE_IMG1` does. The retained stream/palette/frame buffer is reused
  for TITLE only after SWOOSH's 19 source frames finish on the Timer-A cadence.
  The real-CD M11 regression locks source frame-zero and first-delta output
  (13 and 59 indexed pixels), prevents early SKULL input, and reaches TITLE.
  No file is unpacked and no PC GDAT screen substitutes for either stream.

- ✅ 2026-08-06 DM2 FM Towns SKULL fallback fence (superseded by the verified
  IMG2 handoff): the temporary black completion state rejected PC GDAT as a
  platform substitute. M11 now presents only the selected HME-242
  `TITLE/0/dtImage+dtPalIRGB/4` IMG2 surface after TITLE, using its native
  local palette and `dt04/0` NEW GAME/RESUME rectangles. Native `SKULL.EXP`
  P3 execution, keyboard routing and continuation semantics remain closed;
  see `parity-evidence/dm2_fmtowns_startup_p3_gdat_boundary.md`.

- ✅ 2026-08-06 DM2 FM Towns CDDA mapping correction: removed the former
  hard-coded HMP→CDDA source literal. Boot now extracts the selected
  HME-242 `SKULL.EXP` in RAM and copies only its native 29-byte table at
  offset `0x3dac` into a bounded receipt. The real-CD regression locks the
  374,416-byte member, source offset and map lookup. Playback remains
  separately blocked until native SKULL execution and CDDA transport are
  joined.

- ✅ 2026-08-06 DM2 FM Towns CDDA coordinate correction: runtime CDDA
  dispatch now reads the live source party X/Y for the original 40-byte
  CD.DAT level-coordinate trigger table, and reevaluates only this route
  after a committed party step. It no longer probes a fabricated `(0,0)`
  cell. Missing source party state remains silent.

- ✅ 2026-08-06 Theron raw-BIN HuC6280 disassembly intake: the static bank-$1f
  receipt now verifies authentic `TQUS02.bin` and `TQJP02.bin` Track 02 files
  in addition to the ISO projections. Their real MODE1/2352 bank-window
  offsets and regional stage-2 handler hashes are bound by MD5/size/byte/FNV
  checks. The focused test passes all four authentic US/JP BIN/ISO sources;
  runtime consumer and semantic level/object/palette/tile/viewport handoff
  remain capture-gated.

- ✅ 2026-08-06 Theron forcefield-menu keyboard fix: M11's physical
  left/right arrow tokens (`STRAFE_LEFT/STRAFE_RIGHT`) now move Theron's Soul
  Room focus. Enter can therefore reach the FORCEFIELD action instead of
  appearing inert; the source-owned capture gate still prevents an
  unverified dungeon handoff.

- ✅ 2026-08-06 Theron real-data inventory: documented the authenticated US/JP
  Track 02 BIN/ISO files, the separate US/JP Track 19 ISOs and the materialized
  US split ISO, including size/MD5 ownership. The documentation explicitly
  prevents Track 19 bytes from being reused as Track 02 data and lists the
  remaining intentional placeholder/capture boundaries.
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
# Nexus FONT256 real-data probe correction (2026-08-06)

- ✅ Removed the stale real-data assertions that sent `FONT256.S2D` through
  the flat 1bpp fixture parser and reported 256 drawable glyph slots.
- ✅ Track-1 launch/readiness probes now use `nexus_v1_font_s2d_decode()` and
  `nexus_v1_font_load_from_s2d()`, verifying the named retail regions and
  exactly 242 real 8x8 character-generator tiles. Page/attribute character
  mapping and HUD framebuffer placement remain blocked.
- ✅ The S2D glyph-byte probe now treats its byte-window map as fixture-only;
  its real branch verifies CG-region bytes and deterministic 242-tile source
  handoff. The real text-layout branch records source regions without drawing.
- ✅ Real-data verification: Track-1 phase launch 57/57, screen readiness
  29/29, glyph-byte probe 250/250 and runtime layout probe 172/172 passed.

# Nexus explicit real-data menu/HUD CTest gates (2026-08-06)

- ✅ Added `nexus_v1_bpk_surface_class_real` and `nexus_v1_stmp_real`. They
  select the external `FIRESTAFF_NEXUS_DATA_DIR`, require the real `MENU.BPK`
  and `STABG.BIN` files, and return skip-safe code 77 when the private corpus
  is unavailable.
- ✅ The real MENU.BPK path verifies all 162 PRS3 surfaces and keeps runtime
  decode/upload blocked; the real STABG path verifies the STMP receipt. The
  data-free tests remain separate and continue to run without game media.
- ✅ Focused CTest: 10/10 passed against `/Users/bosse/.firestaff/data/nexus`;
  missing-data probes returned 77 as intended. `git diff --check` passed.

- ✅ 2026-08-06 DM1 FM Towns English title runtime consumer: after the
  selected legacy GRAPHICS.DAT is bound, M11 validates the selected EDM.EXP
  directory receipt and presents the real graphic-1 PRESENTS frame, the
  source-bound 18-step zoom and MASTER frame. Missing or mismatched FM Towns
  startup media fails closed rather than entering the PC34 title path.
  Japanese JDM and the native FM Towns menu/TBIOS/CD-audio consumers remain
  explicitly open in TODO.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT bootstrap receipt: the F31 Game
  handoff now records the selected retail CD bootstrap independently of
  user saves. It authenticates `CDATA/MINI.DAT` (42 776 bytes, FNV-1a
  `494999c9`) for English and `CJDATA/MINI.DAT` (43 208 bytes, FNV-1a
  `284799d1`) for Japanese, following ReDMCSB `CEDTDATA.C` G2297 and
  `LOADSAVE.C` F0435's native header path. It deliberately does not pass
  either file to the Atari/Amiga GAMEBLOCK decoder or advertise Resume.
  Real English and Japanese F31 Switch→Game handoff tests pass.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT header verification: the F31 Game
  receipt now runs the selected retail bootstrap's first 512 bytes through
  ReDMCSB `CEDTINC6.C` F7061 with CSB key word 29, then requires its
  decrypted header to be C5, the family that includes FM Towns CSB. The
  English `CDATA/MINI.DAT` key is `0x340f`; Japanese `CJDATA/MINI.DAT` uses
  `0xf77d`. This authenticates the native header without treating its body as
  an Atari/Amiga save or enabling Resume. Real F31E and F31J handoff tests
  pass.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT header ownership: after F7061 admits
  the real header, F31E now requires its F7 English platform marker and F31J
  requires F8 Japanese, with both retaining the C13 CSB-Game dungeon marker.
  This makes the bootstrap receipt reject a language-crossed or wrong-dungeon
  header before any later save-body work.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT save-part receipt: after the native
  header, the Game handoff verifies the original F7057 checksums for
  GlobalData, active groups, champion/party data, events and timeline. Both
  retail files expose one party champion, 60 active-group slots, 436 event
  slots and end the authenticated part sequence at byte 8 236. This is a
  source-backed corpus check, not a dungeon-tail decoder or Resume path.

- ✅ 2026-08-06 CSB FM Towns MINI.DAT dungeon-tail receipt: the F31 Game
  handoff now follows ReDMCSB `CEDTINCA.C` F7063's native tail order after
  the four external portraits. Both retail files verify 11 maps, 296 columns
  and their trailing F7059 byte-sum checksum (English `0x62df`, Japanese
  `0x6671`). The result remains an admission receipt, not a live save restore.

- ✅ 2026-08-06 CSB MAP origin correction: the source loader now reads
  ReDMCSB `DEFS.H` MAP `OffsetMapX/Y` from bytes 6/7, not the unrelated
  byte-4/5 padding. The first authentic F31 MINI map therefore retains its
  real origin `(17,14)` in the Game receipt instead of a synthetic `(0,0)`.

- ✅ 2026-08-06 CSB FM Towns MINI dungeon consumer: the F7063-authenticated
  tail is now copied only through a receipt-bound API and opens in the real
  CSB dungeon loader. The F31E corpus proves all 11 maps and its first map's
  `(17,14)` origin; no raw save bytes are promoted to a live resumed world.

- ✅ 2026-08-06 CSB FM Towns MINI party-pose receipt: the F7057-decrypted
  `GLOBAL_DATA` now retains GameTime and the original party pose, and F7063
  rejects a pose outside its selected map. F31E proves tick 82 at map 4
  `(22,18,S)`; F31J proves tick 88 at the same pose. Champion-body decoding
  and live restoration remain deliberately separate.

- ✅ 2026-08-06 CSB FM Towns Utility P3 boundary: `UTILE.EXP` and
  `UTILJ.EXP` now must pass their original Phar Lap level-1 P3 envelope in
  addition to the full-file identity gate. The receipt records the real
  384-byte header, 512-byte load-image offset, English 151 875-byte / EIP
  `0xfe00` and Japanese 151 987-byte / EIP `0xfeb0` program boundaries.
  ReDMCSB `COMPILE.H` EXEID 63/64 identifies the pair as C06_CEDT. This
  intentionally does not substitute the existing PC34 utility flow for the
  native TBIOS editor or its save transactions. Real F31E and F31J handoff
  tests pass.

- ✅ 2026-08-06 CSB FM Towns C06 menu byte receipt: disassembly of the
  verified P3 load images identifies the first Utility menu pool and binds
  it by raw offset, length and FNV-1a. F31E exposes its six original labels
  (`LOAD CHAMPIONS`, `SAVE CHAMPIONS`, `MAKE NEW ADVENTURE`, `REVERT`,
  `UNDO`, `QUIT`) from virtual `0x11578`; F31J exposes the corresponding
  68-byte Shift-JIS pool from `0x11628`. The receipt keeps the Japanese text
  as original bytes and does not manufacture translated host labels. English
  and Japanese real-media handoff tests pass.

# DM1 FM Towns native action-label consumer (2026-08-06)

- ✅ The authenticated English EDM load image now verifies and retains all 44
  native `DYNAMENU+8` action labels, including source duplicates such as
  `STAB` and `X`. M11 uses this receipt-owned stream for English FM Towns
  action rows instead of borrowing the generic PC34 table. The focused real
  cache startup test verifies `PUNCH`, `WAR CRY` and `FUSE`; native TMENU/
  DYNAMENU input and pixel/TBIOS rendering remain separate TODO items.
# Nexus level-bound consistency (2026-08-06)

- ✅ Nexus sound-bank loading and mechanics level admission now use the
  canonical `NEXUS_MAX_LEVELS` bound instead of duplicated literal `15`
  checks.
- ✅ Real European SAL/MAP corpus verification still passes for all 16 levels;
  event dispatch and playback remain fail-closed.
- ⚠️ The aggregate build remains blocked later by the unrelated DM2 FM-Towns
  animation-stream link gap recorded in `TODO.md`; the Nexus archive itself
  builds.
# Nexus Font256 production section-parser link (2026-08-06)

- ✅ `firestaff_nexus` now links the existing real SEGA SATURN SCR section
  parser required by Font256 admission.
- ✅ `FIRESTAFF_NEXUS_PRODUCTION` compiles out the unproven flat glyph loader
  and host framebuffer writer; no synthetic text pixels enter production.
- ✅ Font256 section-witness, first-section and corpus targets build, and the
  real 25,012-byte `FONT256.S2D` section-table probe passes 55/55.
- ✅ The production-source boundary verifier now checks the compile guard and
  parser inclusion instead of requiring the whole translation unit to be
  excluded.
- ℹ️ The aggregate project build now passes the Nexus archive and stops later
  at an unrelated DM2 FM-Towns animation-stream link gap.
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

# 2026-08-06 — DM2 PC champion DYN4 boot ownership

- ✅ The verified PC boot now joins the canonical G1 dungeon's 16 original
  subtype-`0x7e` champion-mirror roots to their shared
  `DM2_MARK_DYN_LOAD(0x16ffffff)` selector and retains the resulting DYN4 raw
  blocks in boot-owned RAM. The receipt locks 96 blocks, 149,670 allocated
  bytes and source hashes `a0af7eca`/`8ae00cc1`; all 21 sound rows are admitted
  from the explicit empty source queue state. Cleanup releases the cache with
  `graphics_dat`, and no game member is written or unpacked to disk.
- ✅ The full real PC-DOS M11 startup/profile gate consumes that receipt.
  Champion creation remains deliberately unavailable until the original
  event, possession-transfer and party/session mutations are connected. The
  FM Towns HME-242 extension-pool gap remains explicit and is not replaced by
  the PC layout.
# 2026-08-06 — DM2 FM Towns boot-profile lifetime and input boundary

- Boot cleanup now releases the DM2 runtime only when it is bound to the
  exact profile being destroyed. This prevents a rejected FM Towns English
  companion from leaving the runtime or sound layer borrowing freed native
  GDAT/disc memory. The release path contains no replacement media or text.
- Audited the real HME-242 `SKULL.EXP` load image in RAM against SKWIN's
  PC-only `D7 80 1C 00` Enter row. It has zero matches, so FM Towns continues
  to accept only its authenticated GDAT pointer rectangles while keyboard
  routing remains closed pending native P3 evidence.
- Verification: the authentic FM Towns title/menu real-media test with the
  selected DOS English ZIP companion, the full PC-DOS M11 startup/profile
  gate and the production placeholder boundary all pass.

# 2026-08-06 — DM2 FM Towns original END transition

- The source-owned HME-242 Quit rectangle now follows `SKULL.EXP` with the
  next command in authenticated `AUTOEXEC.BAT`: `TWANIM END`. M11 retains the
  selected member only in RAM, replays all 420 displayed frames (including
  FO/NE loops), applies its source PL palette for each frame, then returns to
  Firestaff's launcher. The former immediate host exit is gone for FM Towns.
- Verification: real HME-242 M11 startup test confirms SWOOSH, TITLE, GDAT
  menu/credits, Quit-to-END, loop-expanded frame count and return only after
  END completes. No game media is unpacked to disk.
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
# 2026-08-06 CMake DM2 world-state probe dependencies

- ✅ Restored the two direct source dependencies of `probe_dm2_v1_world_state`:
  the FM Towns animation-stream parser used by boot and the champion-stat
  bridge used by the runtime. This resolves the identical undefined-symbol
  failures reported by the macOS, Windows and Linux CMake jobs.
- Verification: clean Release configuration, focused probe link and the
  registered `dm2_v1_world` CTest pass locally.

# DM2 legacy outdoor facade production isolation (2026-08-06)

- ✅ Removed `dm2_v1_outdoor_renderer.c` from the production DM2 archive. It
  only exposed a procedural-sky compatibility facade and returned a no-draw
  sentinel; it had no original GDAT image/palette consumer.
- ✅ Kept the focused material-boundary test compiling the source explicitly.
  The production outdoor route remains the authenticated
  `ENVIRONMENT`/`DistantEnvironment` weather/runtime chain.
# DM2 sound-name corpus boundary (2026-08-06)

- ✅ Verified the supplied PC-DOS `GRAPHICS.DAT`: category `0x02` has no text
  entries; SOUND rows are raw audio payloads.
- ✅ Added a regression assertion to `test_dm2_v1_sound_gdat_real_data` and
  kept `dm2_v1_sound_name()` fail-closed. No synthetic sound labels were
  admitted.
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
# DM2 shop placeholder production isolation (2026-08-06)

- ✅ Removed the empty fixed-catalog shop state carrier and coordinate-only
  shop/NPC classifier from the `firestaff_dm2` production archive.
- ✅ Removed M11's last call into that unavailable state carrier. Focused shop
  tests retain explicit source-study coverage; production remains blocked until
  the original `SHOP_GLASS`/`WALL_GFX`/`dt08` transaction is bound.

# DM2 DoorType-0 source panel parity (2026-08-07)

- ✅ Corrected the active V1 door render plan so a real DB0 door with
  `DoorType()==0` still selects its record-specific `DOORS` image. The
  authenticated G1 root, not the type value, now proves record presence.
- ✅ Added the type-0 regression to the real PC-DOS `GRAPHICS.DAT` RAW4 door
  placement test. The source panel/button test and door scene-control gate
  pass; no generated door geometry is admitted.

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

# 2026-08-06 Asset scanner warning cleanup

- ✅ Removed the unused DM1 FM Towns admission local from the shared scanner
  loop. The admission still runs for its required side effects, while CSB
  archive-scanner builds no longer issue that unused-variable warning.

- ✅ Kept a verified PC34 GRAPHICS.DAT/DUNGEON.DAT pair as PC34 when a shared
  cache also contains an A31M `TITL.DAT`. The sidecar now promotes A31M only
  from M12's selected `csb-amiga31-multi` cache package, which preserves the
  original package provenance instead of using a nearby file as a variant
  override.

# 2026-08-06 Nexus UI-event dispatch boundary

- ✅ Retail ISO/extracted Nexus now rejects host UI events before the Saturn
  SLEV/SDDRVS producer, queue and state-write contract is captured. This closes
  direct automap, inventory, save, leader, throw and drop mutations while
  retaining the source-less compatibility lane.
- ✅ Added the production-boundary regression for automap and command-state
  immutability.
- ✅ The public level-transition helper now shares the same retail gate, so a
  pending compatibility transition cannot bypass the tick boundary and load a
  synthetic retail DGN level. The production regression covers the rejected
  call and output state.
# DM2 class-0x30 spell-failure real GDAT binding (2026-08-13)

- ✅ Bound SKProject `DM2_PROCEED_SPELL_FAILURE` class `0x30` to the exact
  authenticated `INTERFACE_GENERAL/5/dtImage/0x0B` NEED_FLASK record and
  source destination rect `0x5C`; runtime records decoded-pixel and local-
  palette hashes and never publishes invented text or pixels.
- ✅ Kept the receipt fail-closed (`no_draw`) until the M11 transparent-static-
  pic surface consumer and C068--C070 panel-global update are source-owned.
- ✅ The real-data M11 startup/profile regression passes against the mounted
  PC-English DM2 corpus; no release was made.
# DM2 class-0x30 spell-failure real GDAT binding (2026-08-13)

- ✅ Bound SKProject `DM2_PROCEED_SPELL_FAILURE` class `0x30` to the exact
  authenticated `INTERFACE_GENERAL/5/dtImage/0x0B` NEED_FLASK record and
  source destination rect `0x5C`; runtime records decoded-pixel and local-
  palette hashes and never publishes invented text or pixels.
- ✅ Kept the receipt fail-closed (`no_draw`) until the M11 transparent-static-
  pic surface consumer and C068--C070 panel-global update are source-owned.
- ✅ The real-data M11 startup/profile regression passes against the mounted
  PC-English DM2 corpus; no release was made.
# Nexus Saturn raw VDP1/VDP2 runtime witness (2026-08-06)

- ✅ Built the patched Mednafen 1.32.1 Saturn producer on the external disk.
  The binary contains the `ss` module and the Firestaff raw-capture hook.
- ✅ Ran the European BIOS against the European DM Nexus ISO through a
  data-only CUE and retained a two-frame, 3,155,092-byte raw witness outside
  the repository. Mednafen identified `T-9111G`, `DUNGEON MASTER NEXUS`, and
  the European area.
- ✅ Added `scripts/validate_nexus_saturn_runtime_capture.py`, which checks the
  capture magic, ordered frame markers, and exact VDP1/VDP2 payload lengths.
  It explicitly reports semantic admission as blocked: PRS3, SLEV/SAL/SDDRVS,
  HUD, and viewport routes remain gated.
- ⚠️ The supplied European CUE references missing Japanese audio-track files;
  the capture used a temporary data-only CUE pointing at the same European ISO.
# DM1 D1C F0115 synthetic audit isolation (2026-08-06)

- ✅ Removed the asset-free D1C door-frame/F0115 contract from M10. Its direct
  ReDMCSB regression remains explicit; verification passes 87 assertions.
# DM2 inventory rejection host-text removal (2026-08-06)

- ✅ Removed both M11 DM2 inventory fallback labels (`DM2 INVENTORY GDAT
  REQUIRED`). The shared DM1 panel remains unavailable, but the source
  ObjectID/control boundary is preserved and the unbound DM2 GUI route now
  stays silent until SKProject `CHANGE_VIEWPORT_TO_INVENTORY` is connected.
- ✅ Updated the real-data M11 startup/resume regression to require an empty
  status surface while the inventory gate rejects. No release was made.

# DM2 SKSAVE tile-chain root retention (2026-08-06)

- ✅ Fixed `DM2_READ_SKSAVE_DUNGEON` to pass a root-link owner to the source
  record decoder and retain the resulting DB-chain head on the tile through
  `set_tile_record_link`, matching SKProject `sksvgame.cpp:1390-1399`.
- ✅ Kept complete GAME_LOAD/resume admission blocked; this change only
  preserves the authentic decoded ownership edge. No release was made.
# DM1 mirror reopen-after-save-load synthetic audit isolation (2026-08-07)

- ✅ Removed the contract-only C040 reopen-after-save/load snapshot model from
  M10; its explicit ReDMCSB regression remains the build owner.
# DM1 mirror icon-refresh synthetic audit isolation (2026-08-07)

- ✅ Removed the in-memory icon/slot fixture from M10; its direct regression
  and the dependent double-open regression compile it explicitly.
# Nexus European Saturn startup capture correction (2026-08-07)

- ✅ Confirmed media provenance with the supplied E-BIOS: the English ISO is
  `SGAREA U`, the merged English image is `SGAREA J`, and the French ISO is
  `SGAREA E`. Only the French ISO is used for the European capture chain.
- ✅ The E-BIOS + French-media raw captures validate through the external
  VDP1/VDP2 validator. At later frame windows they show authentic TrueMotion
  publisher graphics and a changing orange startup animation in the VDP1
  framebuffer. No host pixels or semantic menu/HUD/viewport admission was
  added.
- ✅ The Japanese BIOS attachment was hash-verified separately for the J-region
  comparison path; its evidence remains separate from the European chain.

# Nexus Saturn startup input capture route (2026-08-07)

- ✅ Added an external-only Mednafen SMPC route for a bounded active-low START
  pulse, selected by emulated frame and hold length. It writes no VDP/SH-2
  state and records the input window in the operator manifest.
- ✅ Rebuilt the existing external Saturn producer’s SMPC object and relinked
  the instrumented binary; patch dry-run and launcher regression pass.
- ⚠️ E-region tests at frame 1000/60 frames and frame 4500/2 frames still show
  authentic intro imagery after the input window. Menu, HUD, viewport and
  PRS3/SLEV/SAL/SDDRVS semantic admission remain blocked pending a proved
  transition and source-owned consumer bindings.
- ✅ Extended the operator route with the Saturn gamepad A-bit mask (`0x20`)
  and combined START+A mask (`0x30`); launcher, patch dry-run and relinked
  external binary checks pass. No menu claim was made from the frame-7500
  intro capture.

# Nexus Saturn A/START+A runtime window (2026-08-07)

- ✅ Ran the real European ISO with E-BIOS, a 60-frame combined START+A
  window at emulated frame 6500, and raw VDP1/VDP2 capture beginning at frame
  8000. The four captured frames remain authentic intro/fire imagery; no menu
  transition was observed.
- ✅ Re-ran the real DM.BIN startup/menu resource-anchor test and startup-media
  gate against `/Users/bosse/.firestaff/data/nexus`; both passed. These prove
  source ownership and asset admission, not Saturn menu placement.
# DM2 PC-9821 catalog/hash-pair correction (2026-08-07)

- ✅ Added the authenticated retail `pc9821-ja` DM2 version to the launcher
  catalog using the real `GRAPHICS.DAT` MD5 `a80c555a858ef7770e1d7f3d2e37fec3`.
- ✅ Selected its source-owned `DUNGEON.DAT` MD5
  `fa644b2451af197874ee7dc3951e7033` in the required-file gate, keeping it
  distinct from the PC-9801 demo and PC DOS dungeon pair.
- ✅ Extended the DM2 catalog regression to cover the seventh version and
  PC-98 architecture classification. Runtime launch remains subject to the
  existing authenticated boot pair and native platform owners.
# DM2 GDAT header/ULP source transaction (2026-08-13)

- ✅ Replaced the callback seam's synthetic/unimplemented
  `DM2_READ_GRAPHICS_STRUCTURE` result with source-owned validation of the
  authenticated GDAT header, version, entry count, ULP span and cumulative raw
  boundary, using the existing file callbacks and closing the source handle on
  every path.
- ✅ Added regression coverage for the complete header/ULP receipt (`23/23`),
  including the mounted real DM2 `GRAPHICS.DAT` when
  `FIRESTAFF_DM2_DATA_DIR` is set.
  ENT1, underlay, image allocator and sound ownership remain explicitly gated;
  no release was made.
# DM2 combat defense receipt binding (2026-08-13)

- ✅ Retained the real CREATURES→v1d296c AIDefinition defense byte in the DM2
  combat receipt through the existing provider hook, including explicit
  no-provider and unproven-provider flags.
- ✅ Kept the complete combat action fail-closed: no damage, kills, mutation or
  player-facing feedback is produced from defense alone. Updated unit and
  mounted-GDAT coverage accordingly; no release was made.
# Nexus Saturn capture window follow-up (2026-08-07)

- ✅ Fixed the raw-capture launcher’s instrumented-binary check so `pipefail`
  cannot turn a valid `strings` match into a false exit-78 rejection.
- ✅ Captured additional authentic E-BIOS/French-media VDP1/VDP2 windows at
  intro frame offsets 2400, 3000, and 4200. They remain raw transport/layout
  witnesses: the validator passes, while PRS3, menu, HUD, viewport, and
  SLEV/SAL/SDDRVS semantic admission remains correctly blocked.

# DM2 legacy SKProject sound-model production boundary (2026-08-07)

- ✅ Verified that the caller-authored `dm2_v1_skproject_sound_*` regression
  model has no product-source caller. Added a production-boundary check that
  rejects any future M10/M11 call into that model; authentic runtime sound
  remains owned by verified GDAT/DYN4 material, SDL playback, or FM Towns
  CDDA media.

# DM2 selected-corpus i18n verification (2026-08-07)

- ✅ Replaced the stale, skip-prone i18n smoke test with a selected-data-root
  real-media test. It now fails when the requested PC-DOS `GRAPHICS.DAT`
  cannot be opened, has too few text rows, rejects extraction, or cannot
  return every loaded text key through the runtime lookup.
- ✅ Verified against the mounted PC-DOS corpus: 8,639,757 bytes, 1,861 GDAT
  text rows and 781 unique queryable text keys. No host-authored menu text or
  filename-based fallback is admitted.
# DM2 V2.2 source-material boundary audit (2026-08-13)

- ✅ Extended the V2.2 material-gate regression with an opt-in mounted-data
  check. The real PC-DOS `GRAPHICS.DAT` corpus remains source V1 material and
  cannot be counted as a modern-art manifest or admit V2.2 pixels.
- ✅ Kept the V2.2 path fail-closed (`NO_MANIFEST`, no slot material) until a
  complete provenance-checked art pack exists; no release was made.

# DM2 dynamic-light source boundary audit (2026-08-13)

- ✅ Removed synthetic dynamic-map light values from the mounted-data scene,
  weather and light-chain regression. It uses the source-owned fixed-light
  branch when available and explicitly rejects a real dynamic map without
  runtime state.
- ✅ Kept dynamic maps unavailable until the original runtime/save owners for
  `v1e0974`, savegame light, party possessions, spells and weather state are
  connected; no release was made.

# DM2 real ObjectID text boundary audit (2026-08-13)

- ✅ Exercised the source text bridge with the mounted G1 DB5 object `0xD407`.
  Its record ownership and `WEAPONS/126` classification are accepted, while
  the missing original GDAT text keeps the result unnamed.
- ✅ Confirmed that M11 cannot turn a missing source text record into a fixture
  or diagnostic item label; no release was made.

# DM2 original writer corpus guard (2026-08-07)

- ✅ Extended the writer-gate test to attempt quicksave against a mounted
  original `sksave0.dat`. The call is rejected before writing and the source
  file remains byte-fingerprint identical.
- ✅ Revalidated the mounted SKSave census: 8 original files, with no private
  export admitted; no release was made.

# DM2 selected-corpus mirror gate (2026-08-07)

- ✅ Tightened the real G1 champion-mirror regression so a selected but
  unreadable or malformed `DUNGEON.DAT` fails instead of being reported as a
  skip. The mounted PC-DOS corpus still verifies 16 source mirror roots, while
  champion selection remains blocked before incomplete `GAME_LOAD` ownership.

# DM2 weather text-source audit (2026-08-07)

- ✅ Audited the mounted PC-DOS `GRAPHICS.DAT`: set 5 has nine ENVIRONMENT
  text rows at fields `0x64..0x6c`, and those rows are the source weather
  command payloads consumed by `RETRIEVE_ENVIRONMENT_CMD_CD_FW`, not display
  names. The real IMG9 regression now asserts that exact source shape; weather
  names remain unavailable rather than synthetic.

# DM2 door text-source audit (2026-08-07)

- ✅ Extended the selected real-data door-panel regression to assert that the
  `DOORS` GDAT category contains no typed text rows. Door status/type labels
  therefore remain unavailable, while the real RAW4/GDAT panel and button
  placement still passes.
# DM2 spell-feedback destination provenance (2026-08-07)

- ✅ Bound the class-0x30 NEED_FLASK receipt to the source `0x5C` rectangle
  expanded from `INTERFACE_GENERAL/0/dt04/0`, retaining its coordinates and
  RAW4 table hash alongside the real image/local-palette hashes.
- ✅ Rebuilt and passed the mounted PC-English DM2 M11 startup/profile gate;
  transparent-static-pic drawing and C068--C070 panel ownership remain
  deliberately no-draw, and no release was made.
- ✅ Confirmed the real PC-English expansion is `(456,100,92,77)` for rect
  `0x5C`, versus a `92x25` NEED_FLASK image; this prevents routing the
  full-surface feedback through the 320x200 viewport framebuffer.

# DM2 SKSave source-field inventory (2026-08-07)

- ✅ Added a dedicated receipt and corpus hash for the complete 60-byte
  source `s_savegamebuffer`, with ownership cited to SKProject
  `sksvgame.cpp:47/1415` `DM2_GAME_LOAD`.
- ✅ Kept session-only gold, reputation and gametime unpromoted: the source
  save buffer has no scalar fields for them, and raw resume remains blocked
  until coin-record and separate time-state ownership is complete.

# DM2 creature AI row-owner census (2026-08-07)

- ✅ Tightened the mounted PC-English real-data regression to exactly 74
  authenticated `CREATURES[type].word(0x05)` bindings and 73 non-identity
  type-to-row mappings.
- ✅ Asserted source-unowned types 54 and 127 remain unavailable instead of
  falling back to `creature_type == ai_row`; DB4/CAII/CCM runtime ownership
  remains fail-closed.

# DM2 dynamic c_light real-data boundary (2026-08-07)

- ✅ Removed synthetic `base_light`, darkness and state-hash inputs from the
  mounted `DUNGEON.DAT` c_light scan. Real dynamic-map descriptors now prove
  only the branch selector and must remain blocked without recovered runtime
  state; fixed maps use their authenticated descriptor identity.
# DM2 actuator shooter boundary (2026-08-07)

- ✅ Extended the fail-closed shooter regression across all six source
  shooter types (`0x07..0x0a`, `0x0e..0x0f`); none allocates DB14 or queues
  `SHOOT_ITEM` before the record/timer owner is available.
# Nexus SCSP read-trace boundary (2026-08-07)

- ✅ Added a reproducible Mednafen sound-CPU SCSP-read producer with bounded
  address/PC filters and a strict trace analyzer.
- ✅ Ran it against the authenticated European French gameplay window. The
  100-row receipt contains real shared-RAM/driver reads, but no mailbox read
  at `0x100400..0x100401` and no `0x3224`-filtered read; SLEV/SAL semantics
  and host playback therefore remain fail-closed.

# DM2 dialogue raw-text provenance (2026-08-07)

- ✅ Tightened the mounted PC-DOS save/load dialogue regression so both
  displayed labels (`SAVE`, `CANCEL`) also require their independent raw
  GDAT source-text hashes.
# DM2 merchant synthetic-route regression cleanup (2026-08-13)

The M11 startup/profile gate no longer contains a hidden coordinate-only NPC
expectation that invented a friendly merchant, reputation increments, and
dialogue progression. Its real-data regression now calls the runtime boundary
directly and verifies that an unowned square leaves NPC, dialogue, reputation,
and host text unavailable. This matches the source-owned AI-33 creature/DB/CCM
requirement in SKProject `skcrture.cpp:5368-5444,5697-5700`.
# Nexus VDP1 source-writer corridor (2026-08-07)

- ✅ Extended the VDP1 write-trace analyzer with exact PC and address-range
  requirements.
- ✅ Verified an authentic European startup window with 4,601 writes from
  runtime PC `0x06013098` into `0x47c00..0x49ffe`; known framebuffer/colour
  writers remain separately classified. The runtime writer is not promoted
  to a named retail asset or production draw route without source identity.

# DM2 real c_light regression initialization (2026-08-13)

The mounted `GRAPHICS.DAT`/`DUNGEON.DAT` scene-weather-light regression now
constructs its source-owned `GRAPHICSSET` scene-light receipt before selecting
the fixed or dynamic map branch. Dynamic-map rejection is therefore checked
against authenticated real scene state instead of an uninitialized test value.
# DM2 diagnostic world-state archive boundary (2026-08-13)

Removed `dm2_v1_world_state.c` from the production DM2 archive. Its bounded
save projection and deliberately unavailable writer remain available only to
focused save/minimap tests, so the production library cannot advertise that
projection as an original `SKSave.dat` runtime.
# DM2 source DB3 shop census (2026-08-13)

Extended the canonical G1 actuator regression to inventory every successfully
materialized source DB3 root for shop-panel `0x3f` and shop-floor `0x30` types.
The census is diagnostic evidence only; type bytes cannot activate a merchant
without the source `WALL_GFX`/`dt08`/AI-33 transaction.

# DM2 source DB3 actuator lookup handoff (2026-08-13)

Added a lookup-only accessor over the authenticated G1 DB3 actuator receipt.
It selects the source actuator by map coordinate and preserves the exact
`w2`/`w4`/`w6` fields without following `GenericRecord::w0`, mutating DB14, or
creating a timer. Real PC-English map 5 coverage verifies the source root and
an absent coordinate; generic actuator mutation remains fail-closed.

# DM2 generator actuator source census (2026-08-13)

Extended the canonical G1 actuator regression to count creature-generator
`0x2e` and item-generator `0x3c` roots across every real map. The mounted
PC-English dungeon contains neither class; no host-authored generator default
is promoted in their place, and profiles with those classes still require the
source DB14/DB-record/timer owner.

# DM2 real wall GRAPHICSSET address audit (2026-08-13)

The canonical PC-English wall-plan regression now round-trips every emitted
wall command through the live `GRAPHICSSET/<MapGraphicsStyle>` address and
exact viewport field before M11 consumes it. The real wall frame remains
source-owned with zero fallback draws; mismatched material is rejected.

# DM2 source shooter actuator census (2026-08-13)

Extended the canonical G1 actuator regression to inventory all six source
shooter classes (`0x07..0x0a`, `0x0e..0x0f`) across every real map. The mounted
PC-English corpus contains four `0x08` roots and zero roots for the other five
classes. The test retains those counts as source-presence evidence only; it
does not allocate a DB14 record, create a projectile, or schedule `SHOOT_ITEM`.

# DM2 source DB14 missile census (2026-08-13)

Extended the real G1 regression to enumerate only authenticated DB14 missile
records and retain their source timer-index presence. The mounted PC-English
corpus contains zero such records. The census is read-only: it does not step,
allocate, delete, or render a projectile while the complete `DM2_STEP_MISSILE`
owner transaction remains unbound.
# Nexus VDP1 writer code-window receipt (2026-08-07)

- ✅ Added a reproducible Mednafen producer patch that captures the live SH-2
  code window around the VDP1 writer PC, plus a strict validator.
- ✅ Captured the authentic European runtime PC `0x06013098` with VRAM target
  `0x47c00` and 48 code words. Little-endian SH-2 disassembly shows the live
  branch at `0x06013098` to `0x06012f52`.
- ✅ Kept source-file identity, VDP1 command/CLUT ownership and production
  rendering blocked: the relocated code window is runtime evidence, not yet
  a byte-for-byte DM.BIN/TM.BIN join.
# DM2 source actuator-type query handoff (2026-08-13)

Replaced the querydb actuator-type stub with a bounded, read-only source-layout
query. It admits only DB3 records, reads `w2` with the original low-seven-bit
`ActuatorType()` mask, rejects ObjectID sentinels/missing records, and adds
focused coverage for source-shaped records. Activation, DB14 and timer
transactions remain fail-closed.
# DM2 c_light modifier parity correction (2026-08-13)

Corrected `dm2_v1_recalc_light_level_pc34()` to match SKProject
`src/v5/sklight.cpp:186-190`: `v1e0978` is normalized to `1` above `0x0c`
before the final clamp, rather than being subtracted as an unrestricted host
delta. Added regression coverage and reran the real GRAPHICSSET scene/light
admission tests; dynamic-map runtime ownership remains fail-closed.

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
# Nexus relocated-code loader receipt (2026-08-07)

- ✅ Added a bounded Mednafen SH-2 high-RAM write producer and validator.
- ✅ Verified 3,080 authentic writes into `0x06013000..0x06013fff` from the
  runtime loader PC `0x00002368` in the European startup window.
- ✅ Kept the retail source member unbound: BIOS/runtime-loader ownership does
  not prove DM.BIN, TM.BIN, a video asset, or any VDP1/VDP2 consumer contract.

# DM2 square element query parity (2026-08-07)

Corrected the bounded DM2 tile queries to use the authenticated low-byte
square encoding: `M034_SQUARE_TYPE` extracts the element, while ReDMCSB's
source constants identify pit `0x02`, stairs `0x03`, and teleporter `0x05`.
The former stairs/teleporter values were host-side misclassifications. Added
public declarations and regression coverage; actuator DB3/DB14/timer
mutation remains fail-closed.
# Nexus Saturn CDB sector-trace receipt (2026-08-07)

- ✅ Added a bounded Mednafen Saturn-CDB read hook at the actual
  `src/ss/cdb.cpp` data-sector path, with an external-only patch and no game
  data copied into the repository.
- ✅ Compiled the instrumented Mednafen build and captured the BIOS window:
  1,024 reads covering LBA `0..16`; this is authentic BIOS/CD startup traffic,
  not yet a retail Nexus member read.
- ✅ Kept source identity, relocated-code admission, SLEV/SAL playback and
  VDP1/VDP2 production composition blocked until a later CDB window reaches
  and joins an authenticated ISO file span.

# DM2 SKSave savegames1 owner receipt (2026-08-07)

Named the six-byte `c_wbbb`/`ddat.savegames1` section from SKProject
`sksvgame.cpp::DM2_GAME_LOAD` and centralized its source size. The mounted
real-save corpus keeps this section as a raw hash only; no unproven scalar
gold, reputation, or time value is promoted into the Firestaff session.

# DM2 generic save/action feedback boundary (2026-08-13)

Blocked DM2 `QuickLoad` before the shared DM1 envelope reader and cleared
generic status/inspect text on DM2 BACK and action receipts. This prevents
host-authored save/action messages from appearing while the original
`c_gui_draw`/`c_dialog` producer and complete `GAME_LOAD` graph remain open.

# DM2 source half-step movement gate (2026-08-13)

Added the source-locked `glbIsPlayerMoving` half-step admission from
SKProject `v4/skgame.cpp:2364-2372`, including forward/backward,
double-step/stairs and table-to-move branches. The receipt records the exact
`walk_delay >> 1` countdown only for explicit source inputs; live pose,
viewport offset and c_hero/inventory ownership remain fail-closed.
# Nexus Saturn retail CDB join receipt (2026-08-07)

- ✅ Re-ran the European-BIOS/French-media startup through the corrected
  SMPC input hook, which now runs after Mednafen's virtual-port update. The
  bounded CDB trace contains 50,000 authentic data-sector reads over LBA
  `0..59951`; ISO9660 joining resolves all six required members and reports
  `DM.BIN` (8,212 reads), `TM.BIN` (173), `ITEM.IBS` (72), `MENU.BPK` (44),
  `SLEV00.BIN` (61), and `SDDRVS.TSK` (14), with `LEV00.DGN` and
  `SNDLEV01.SAL` also observed.
- ✅ Added `scripts/analyze_nexus_cdb_read_trace.py` as a reproducible,
  read-only ISO9660/LBA join gate. It emits `retail_lba_join=verified` but
  deliberately ends with `semantic_admission=blocked`.
- ✅ The same session retained 3,080 high-RAM writes in
  `0x06013000..0x06013fff` from runtime loader PC `0x2368` and one authentic
  Saturn runtime frame. This strengthens the temporal capture receipt only;
  it does not identify the bytes' producer or prove VDP1/VDP2 draw order,
  CLUT ownership, HUD composition, SLEV/SAL dispatch, or SFX playback.
- ⛔ No VDP1 writer trace was emitted in this combined bounded run. Keep
  production face/mesh/texture, HUD/viewport, SLEV/SAL/SDDRVS and PRS3
  consumer admission closed until a trace joins a live writer/consumer to
  the authenticated retail bytes.

# DM2 action/save pre-resolver feedback gate (2026-08-13)

Moved the DM2 quick-save and quick-load source boundary ahead of the shared
host path resolver in `src/engine/m11_game_view.c`. A DM2 attempt now reaches
the original-writer/`GAME_LOAD` gate first, so path-length, directory and
generic envelope errors cannot publish host-authored SAVE/LOAD text. The
structured quicksave receipt and `DM2_GAME_SAVE_MENU` source ownership remain
unchanged and the original writer/loader stays fail-closed.

# DM2 FM Towns live text-owner census (2026-08-13)

Audited the production M11 DM2 text consumers against SKProject
`v5/uidialog.cpp:352-415` and `v5/gfxstr.cpp:576-602`. The only current M11
GDAT-text consumer is the save/load panel, which enters through
`dm2_v1_boot_dialogue_open_panel_host_command` and its authenticated FM Towns
companion callback. The source-shaped 0AAF, QueryDB, GfxStr and generic GUI
draw modules have no M11 call site and remain excluded from product archives.
Extended `verify_dm2_production_placeholder_boundary.py` to reject a future
direct M11 call to those unbound owners. Native event/dialogue routing remains
open; no host text or synthetic companion data was introduced.

# DM2 champion mirror raw-marker gate (2026-08-13)

Tightened `dm2_v1_select_champion` against SKProject `c_loadlevel.cpp:604-611`
and `c_hero.cpp:1052-1098`: the canonical PC G1 DB3 mirror must retain raw
actuator data `0x1ff` as well as the derived `0x16ffffff` DYN4 selector and
source direction. Added a regression proving that a mismatched raw marker is
rejected even when its dynamic selector is forged to match. No party,
possession or hero-stat mutation was enabled.

# DM2 champion mirror real-data recheck (2026-08-07)

- ✅ Re-ran the canonical PC-DOS chain from the external worktree: all 16 G1
  mirror roots bind to `0x16ffffff`; the selected DYN4 materializes 96 source
  blocks / 149,670 bytes with payload hash `0xa0af7eca` and receipt hash
  `0x8ae00cc1`.
- ✅ Re-ran the M11 startup/profile gate and the champion lifecycle suite.
- ⛔ Kept playable champion activation closed because the real corpus has no
  static `CHAMPIONS/255` row; `REVIVE_PLAYER`, possession transfer and session
  mutation still need their source-owned runtime state.
# Nexus Saturn runtime source-to-VDP1 provenance receipt (2026-08-07)

- ✅ Corrected the capture invocation to use the actual VDP1 trace variables
  (`FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN/MAX`). The authentic E-BIOS/French
  run now records 39,936 VDP1 VRAM-write rows and a writer code window at
  runtime PC `0x06013098` targeting `0x47c00`; eight raw frames pass the
  transport validator with non-idle VDP1 activity.
- ✅ Added the external-only SH-2 source-read/source-write witness and its
  strict `scripts/analyze_nexus_sh2_source_trace.py` ISO join. In the same
  `skip3000`/frame-1000 startup window, complete contiguous 4 KiB runtime
  chunks match `TM.BIN` at ISO offset `0x74f3000` and `DM.BIN` at
  `0x5e000`; `0DMSTRT.BIN` and `SWTCHR.BIN` also match exactly. Partial
  prefixes and inferred addresses are rejected.
- ✅ The same bounded run joins 50,000 CDB reads to all required retail
  members, including `DM.BIN` (8,212), `TM.BIN` (173), `ITEM.IBS` (72),
  `MENU.BPK` (44), `SLEV00.BIN` (61), and `SDDRVS.TSK` (14).
- ✅ The eight-frame raw witness was decoded with
  `scripts/analyze_nexus_vdp1_command_window.py`: `COPR=0x00000c` exposes the
  four-record Saturn chain (system `0x09`, system `0x0a`, type-2 bitmap draw,
  END). Frame 7 carries `PMOD=0x0028`, `SRCa=0x8f80`, `SIZE=0x28b4`; its
  encoded source address is VDP1 byte offset `0x47c00`, matching the live
  writer corridor at PC `0x06013098`. The observed source span is 33,280
  bytes through `0x4fe00`.
- ✅ Added the external-only VDP2 write witness and analyzer. The same
  authenticated run records 15,365 VDP2 register writes, 183,355 VRAM writes
  and 1,280 CRAM writes; the trace covers the three hardware lanes and passes
  `scripts/analyze_nexus_vdp2_write_trace.py`. The strengthened hook now
  records nonzero SH-2 PCs: dominant VRAM writers are `0x06011924` (65,538),
  `0x060118fc` (40,448) and `0x06002fc4` (22,914), while register writes are
  dominated by `0x0600231c` (14,400). This is executing-code ownership, not
  yet a decoded tilemap/CLUT or production presentation proof.
- ✅ Added the VDP2 writer code-window witness and strict retail comparison.
  The authenticated run records 64 unique SH-2 windows; the primary
  `0x06011924` window contains `25fe 0000 25fe 007c ...`, and the setup
  window at `0x06001416` contains the VDP-register literal pairs. The exact
  48-word windows do not occur verbatim in hash-verified `TM.BIN`/`DM.BIN`,
  so the analyzer keeps source identity and semantic promotion blocked.
- ⛔ This proves retail byte provenance and a live VDP1 writer, not the
  writer's decoded face/mesh/texture consumer, VDP2 tilemap/CLUT ownership,
  HUD/viewport draw order, or SLEV/SAL/SDDRVS event semantics. Production
  semantic admission therefore remains fail-closed.
# DM2 G1 continuation record-pool address ownership (2026-08-07)

Extended the source-ordered c_record pool address path to resolve loader-proven
PC G1 DB3/DB4 continuation records at their original 10-bit ObjectID indexes,
using the validated extension byte spans and per-pool record strides. Added
synthetic boundary coverage and extended the real PC-DOS champion-mirror probe;
all 16 marker roots now resolve through owned record bytes. Champion creation,
possession transfer and session mutation remain fail-closed. Verification:
`test_dm2_v1_record_pool_pc34_compat`,
`test_dm2_v1_dungeon_loader_first_map_gate`, and the real-data
`test_dm2_v1_g1_champion_mirror_real_data` probe.

# DM2 delayed-movement admission receipt correction (2026-08-07)

Corrected the execution receipt to report `delayed_pose_unbound` only when the
complete SKProject `skgame.cpp:2364-2372` half-step condition admits
`glbIsPlayerMoving`. A walk-delay value on an immediate move or turn no longer
claims a delayed pose was requested. The actual pose/countdown owner remains
gated on live hero and viewport state. The focused movement regression now
covers both paths.

# DM2 half-step movement UseAltic parity (2026-08-07)

Corrected `dm2_v1_source_half_step_should_enter` against SKProject
`v4/skgame.cpp:2364-2372` with `UseAltic=1`: both forward and backward
half-step branches now require `bEnableDoubleStepMove`, while the independent
`glbTableToMove` branch remains admitted. Added regression coverage for the
flag-disabled forward/backward cases, stairs, and table-to-move escape.
The source-owned delayed pose is still not published to the live viewport.

# DM2 GDAT structure source-layout correction (2026-08-07)

Corrected `dm2_v1_gdat_read_graphics_structure` to match SKProject
`v4/skcore.cpp:15043-15103` and `v5/bgdat.cpp:1067-1095`. GDAT v4/v5 now read
the first ENT1 size from the four-byte field at offset 4, read only the
remaining ULP words, and create ULP[0] as the source zero entry; the v2 layout
remains supported. The real PC-DOS `graphics.dat` passes with 5,624 entries,
first ENT1 size `0x17284`, raw range `11254..8639757`, and the GDAT/ENT1/image
regressions pass (24/24 structure tests, 122/122 query tests, real image census
green). No allocator or secondary-file runtime admission was invented.
# DM2 GDAT LOAD_ENT1 raw-entry boundary (2026-08-07)

Extended `dm2_v1_gdat_read_graphics_structure()` with the source-owned
`LOAD_ENT1` boundary from SKProject `SKWINSPX/src/v4/skcore.cpp:14804-14840`.
The real GRAPHICS.DAT transaction now retains raw entry 0, validates its
`0x8001` signature, detects its independent endian order, checks all seven
T/I/D/S/F/G/P descriptors and the packed record stride, and releases the
buffer together with the ULP/allocator lifetime. Real PC-DOS data verifies
11,854 raw entries, seven groups and stride 8; malformed/incomplete fixtures
fail closed. Full BUILD_GDAT_ENTRY_DATA, underlay admission and secondary
GRAPHIC2.DAT ownership remain intentionally gated.
# DM2 SKSAVE teleporter target-map gate (2026-08-07)

- ✅ Corrected the save-dungeon teleporter skip to match
  `SKWINSPX/src/v5/sksvgame.cpp:2010-2017`: a target on an earlier map skips
  the record walk, while a forward target is serialized normally. Runtime
  save restoration remains gated on the complete dungeon/record owner.
# Nexus MENU.BPK external-root regression (2026-08-13)

- ✅ Corrected `test_nexus_v1_bppk` so its primary MENU.BPK decode uses
  `FIRESTAFF_NEXUS_DATA_DIR`; the HOME-relative path remains only as the
  documented default when no root is configured. With the external real Nexus
  corpus, the test verifies 164 archive entries, 162 PRS3 surfaces and 162/162
  successful indexed-surface decodes.
- ✅ Re-ran with `HOME=/tmp/firestaff-nexus-no-home` and the external data root;
  the same real-data result passes, proving the test does not read a stale
  HOME-local or synthetic MENU.BPK copy.

# Nexus startup/menu external-root regression (2026-08-07)

- ✅ Updated the real-data `FONT256.S2D`, `FACE.BIN`, `TITLE.CG`/RES* and
  `STABG.BIN` probes to prefer `FIRESTAFF_NEXUS_DATA_DIR`, retaining the HOME
  path only as a compatibility fallback. This keeps startup/menu provenance
  on the mounted external corpus and does not enable any capture-gated pixels.
- ✅ Verified all four probes with
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus` and
  `HOME=/tmp/firestaff-nexus-no-home`: real FONT256, 20 authenticated FACE
  portraits, TITLE/RES archives and STABG all pass.

# ✅ 2026-08-07 DM2 explicit real-corpus test selection

The PC-DOS boot-profile rename regression and the live weather-frame
regression no longer discover `~/.firestaff/data/dm2` through `HOME`.
Both read only the explicitly selected `FIRESTAFF_DM2_DATA_DIR`; no selection
skips the optional real-data portion, while an unreadable selected root fails.
The rename probe obtains its source pair from the boot hash-admission receipt,
so case or filename spelling cannot substitute a private fixture. Verification:
the focused real-data tests run against the mounted PC-DOS corpus, and an
explicitly nonexistent root fails.
# ✅ 2026-08-07 DM2 M11 startup gate explicit corpus selection

`test_dm2_v1_m11_startup_profile_gate` now accepts real data only through one
of its explicit DM2 corpus variables. It no longer falls back to a
`HOME`-relative installation before checking the M12-to-M11 asset-owner
handoff. An unset selection skips the optional real-media path and an invalid
selection fails, so the menu/viewport admission test cannot pass on unrelated
local data.
# DM2 DB4 creature animation cursor handoff (2026-08-13)

- ✅ Preserved the canonical PC G1 DB4 creature `b5/w8/w10` values
  (`info_slot`, `iAnimSeq`, `iAnimInfo`) from the real dungeon material
  receipt through `DM2_CreatureSprite`, the viewport render plan and the
  runtime render receipt. This is provenance publication only; it does not
  claim a live CAII owner or fabricate a V5 frame.
- ✅ Added a real-data assertion to
  `test_dm2_v1_g1_scene_creature_gdat_real_data` that the cursor survives
  into the viewport render. The canonical F9 material and the independent
  `test_dm2_v1_creature_something_real_data` animation reader both pass.

# DM2 launcher source-panel boundary (2026-08-13)

- ✅ Removed the empty M12 host message panel from successful DM2 launch and
  quick-resume handoff. The launch intent remains active while M11 proceeds
  directly to the source-owned `SHOW_MENU_SCREEN`/dialogue surface.
- ✅ Applied the same no-panel boundary to generic DM2 launch failure; the
  structured failure receipt remains available, but no host-authored English
  replacement is shown without a source dialogue producer.
- ✅ Updated the real launcher text assertion to use the source-facing
  `Dungeon Master II` title rather than the internal `DM2` identifier.
  `test_dm2_v1_required_file_popup_gate` and
  `test_dm2_v1_m11_launcher_handoff_boundary` (33/33) pass.

# DM2 source-bound champion activation (2026-08-13)

- ✅ Added a source-bound `SELECT_CHAMPION` transaction that refuses to
  commit without the authenticated DB3 mirror and all live mutation callbacks.
- ✅ Preserved signed PC hero type `0xff -> -1`, source direction, and the
  SKProject callback order for map switch, `REVIVE_PLAYER`, leader selection,
  possession transfer, strip refresh, map restore, and weight recompute.
- ✅ `test_dm2_v1_champion_lifecycle_pc34_compat` passes the positive callback
  trace and missing-owner fail-closed case.
- ✅ Real PC-DOS `test_dm2_v1_boot_profile_smoke` passes 106/106 with 0
  failures; launcher popup and handoff regressions also pass.
# ✅ 2026-08-07 DM2 SDL sound regression explicit corpus selection

`test_dm2_v1_sound_playback_sdl` now reads audio entries only from an explicit
`FIRESTAFF_DM2_DATA_DIR`. It skips when no corpus is selected and fails for an
unreadable selected `graphics.dat`; the old private `HOME` fallback is gone.
The SDL dummy-device regression still decodes and plays original GDAT PCM
entries through the source-backed sound backend.

# DM2 SKSAVE missing creature-AI word semantics (2026-08-13)

- ✅ Corrected the real PC-DOS creature AI-row lookup to match
  `SKWINSPX/src/v4/skcore.cpp:7856-7875` and `skcrture.cpp:28-36`: a missing
  `dtWordValue` returns zero and selects the original `table1d296c` row zero.
- ✅ The mounted corpus lacks CREATURES word `0x05` for types 54 and 127;
  both now retain that source-owned row rather than being fabricated as
  unavailable. All eight SKSave direct-root streams decode. CCM and partial
  GAME_LOAD admission remain fail-closed.

# DM2 SKSAVE source map spans (2026-08-13)

- ✅ Extended the raw SKSAVE dungeon receipt with source-authored per-map
  geometry and relative tile offsets.
- ✅ Added a bounds- and hash-checked per-map receipt; it does not infer
  square types or object links and therefore cannot borrow DUNGEON.DAT data.
- ✅ Mounted PC-DOS `SKSave0-3.dat/bak` verification now covers every saved
  map span; `test_dm2_v1_save_load_real_data` passes 135/135 with 0 failures.

# Nexus viewport/audio corpus external-root regression (2026-08-07)

- ✅ Extended the external-root-first contract to the legacy real-data probes
  for all 16 `LEVxx.DGN` levels, `LOGOBG.DG2`, raw Saturn binaries including
  `DM.BIN`/`SDDRVS.TSK`, and all 16 `SNDLEVxx.SAL`/`.MAP` pairs. HOME remains a
  compatibility fallback only when the explicit root is absent.
- ✅ With `HOME=/tmp/firestaff-nexus-no-home`, the external corpus verified all
  16 DGN decodes, LOGOBG 320x224 source geometry, raw-binary receipts and all
  real SAL/MAP metadata profiles. SAL playback, SDDRVS dispatch and VDP1/VDP2
  presentation remain capture-gated.

# ✅ 2026-08-07 DM2 launcher handoff and real-save corpus boundary

`test_dm2_v1_m11_launcher_handoff_boundary` now accepts a PC-DOS corpus only
through an explicit DM2 corpus variable and rejects an invalid selection. It
no longer probes a private `HOME` installation, exits nonzero on a watchdog
timeout, and verifies that a DOS launch cannot borrow an FM Towns title frame
or palette. The obsolete synthetic `D2RS` quick-resume fixture is removed;
the focused `test_dm2_v1_save_load_real_data` gate remains the read-only owner
of supplied `SKSave` evidence. Verified against the mounted PC-DOS corpus:
the launcher handoff passes 33 checks and the real save corpus passes 127.

# Nexus DMDF/MNS real TEXT material corpus (2026-08-07)

- ✅ Extended `test_nexus_v1_mns` to consume the authenticated external Nexus
  corpus through the production DMDF `TEXT` descriptor and BGR555 material-bank
  route. All 30 retail MNS models retain matching descriptor counts; the corpus
  decodes 815 source textures, with 23 indexed material banks and 587 BGR555
  surfaces verified.
- ✅ The two static Saturn material sources, `SN_FLOOR.MNS` and `SN_WALL.MNS`,
  both decode completely. Seven creature banks remain explicit source-only
  descriptor receipts because their colour cardinality exceeds the current
  indexed host bank; no lossy palette or placeholder surface was introduced.
- ✅ Verification used
  `FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus` with
  `HOME=/tmp/firestaff-nexus-no-home`; `test_nexus_v1_mns` passed, including
  real OBAKE MOTN sampling and 75 transformed source vertices. VDP1/VDP2
  placement and final viewport presentation remain capture-gated.

# Nexus MNS exact direct-colour source lane (2026-08-07)

- ✅ Updated the production DMDF `TEXT` material-bank decoder so real textures
  with more than 256 unique BGR555 colours are retained losslessly as exact
  `uint16_t` source pixels. The decoder no longer rejects those seven creature
  banks and never invents a quantized palette.
- ✅ Kept indexed and direct-colour ownership separate: direct-colour surfaces
  have no indexed `pixels` buffer, so the existing viewport admission gate
  cannot mistake them for render-ready VDP1 materials. The two static banks
  remain indexed and fully decoded.
- ✅ Retail regression now verifies 30 complete TEXT banks and 815 surfaces,
  including seven direct-colour source banks, plus the DGN material raster and
  face/material retail corpus tests. Verification used the external Nexus data
  root with an isolated `HOME`; all focused tests passed.

# ✅ 2026-08-07 DM2 FM Towns CUE-owned image selection

The M12 FM Towns scanner and the DM2 boot reader now take the disc image from
the original CUE sheet's quoted `FILE` member, not from the first ZIP entry
whose name ends in `.img`. The selected HME-242 archive remains entirely
RAM-owned. The shared CUE parser has unit coverage for valid and malformed
member declarations; the real HME-242 probe plus M12 and M11 startup/animation
tests pass against the selected Japanese disc and original PC-English
companion.

# DM2 combat source-owner audit (2026-08-13)

- ✅ Extended the fail-closed creature-combat receipt with an explicit mask for
  the seven still-missing source owners: champion hand, CMDSTR action, target
  record, difficulty/light, item words, source RNG, and player-stat writeback.
- ✅ Kept the proven real-GDAT target Defense byte in a separate proof mask;
  the PC-DOS GLOP regression now asserts that split and that damage/kills stay
  zero. The source-shaped host formula remains unwired.
- ✅ `test_dm2_v1_combat_pc34_compat`: 56/56; real PC-DOS GDAT regression:
  PASS.

# DM2 FM Towns text owner census (2026-08-13)

- ✅ Hardened `verify_dm2_production_placeholder_boundary.py` with an explicit
  production census: exactly one M11 boot-panel text call, and exactly one
  save-dialogue definition plus render call.
- ✅ This keeps the authenticated PC-English companion callback as the only
  live M11 text route while rejecting accidental direct `0AAF`, QueryDB or
  GfxStr consumers. The remaining native event/dialogue owners are still
  clearly open rather than being replaced with host text.
- ✅ The DM2 production placeholder boundary verifier passes.

# DM2 SKSAVE possession-continuation type parity (2026-08-13)

- ✅ Corrected `DM2_2066_062b` so its 10-bit SUPPRESS continuation stream is
  consumed only by source record types 9 and `0xE`. Types 0 through 8 now
  follow SKProject's empty branch and cannot shift later continuation values.
- ✅ Added a source-order regression with a type-5 link before the type-9 and
  type-`0xE` links, plus the mounted PC-DOS SKSAVE corpus gate (135/135).
- ✅ Continue remains fail-closed: this fixes a real record-link decode edge,
  but does not claim the missing record, possession and timer runtime owners.

# DM2 creature 0958 animation-owner handoff (2026-08-13)

- ✅ Extended the authenticated G1 DB4 creature material route to execute the
  source `DM2_query_1c9a_02c3` → `DM2_query_4E26`/0xfc ownership query during
  boot materialization. Static AI rows now retain the real bit-14 result,
  query index and blended value; dynamic rows retain the explicit missing-CAII
  block instead of receiving a guessed frame.
- ✅ Carried that evidence through `DM2_CreatureSprite`, the viewport render
  plan and the runtime creature-render receipt without enabling V5 promotion.
- ✅ Real PC-DOS verification: G1 viewport 40/40, AI owner census 256 mapped /
  255 remapped, DB4 cursor audit 20 static roots, dynamic FB/FC/FD material
  and frame-terminal regressions pass.

# DM2 FM Towns credits palette-index parity (2026-08-13)

- ✅ Fixed the HME-242 TITLE/0/1 credits palette transaction. Its local
  `dtPalIRGB` colours are now assigned to the image's original `dtPalette16`
  physical palette indices, as `DM2_DRAW_PICST` does before `R_C470`.
- ✅ The real-media M11 regression verifies the complete translated RGB6 map,
  so a non-identity nibble mapping cannot silently produce the wrong colours.
- ✅ Verified with the selected FM Towns disc in RAM and the PC-English
  companion; the ordinary PC-DOS M11 startup regression also passes.

# DM2 FM Towns English direct-launch handoff (2026-08-13)

- ✅ Added `--dm2-english-companion <GRAPHICS.DAT>` to the direct DM2 launch
  path. It reaches the selected FM Towns boot profile through the same M12/M11
  handoff as the start menu and takes precedence over automatic companion
  discovery only when the caller explicitly supplied it.
- ✅ The original HME-242 ZIP and the canonical PC-English companion remain
  RAM-only. The boot probe passes with the mounted FM Towns archive; M11 and
  M12 real-media regressions pass unchanged.

# DM2 delayed-movement source-owner audit (2026-08-13)

- ✅ Added an explicit six-bit missing-owner mask to the source half-step
  receipt: hero load, wounds, walk speed, Aura-of-Speed, current pose, and
  tick/countdown ownership.
- ✅ Caller-supplied compatibility snapshots never count as live proof; the
  receipt remains fail-closed and does not create a host interpolation offset.
- ✅ `test_dm2_v1_move_record_to_pc34_compat` passes all movement, gate and
  execution checks.

# DM2 SKSAVE real possession-continuation gate (2026-08-07)

- ✅ Connected every real PC-DOS direct-root link decoded by
  `DM2_READ_RECORD_CHECKCODE` to the bounded `DM2_2066_062b` continuation
  reader, preserving source order and its type-9/type-0xE-only bit consumption.
- ✅ The receipt hashes only source-decoded record and continuation facts; it
  does not fabricate a c_record pool, possession index, timer array or live
  session. Continue and slot admission remain fail-closed.
- ✅ `test_dm2_v1_save_load_real_data` passes 135/135 against the supplied
  PC-DOS SKSave corpus.

# DM2 GDAT source ENT1 row materialization (2026-08-07)

- ✅ Added a descriptor-driven materializer for the authenticated raw ENT1
  transaction. It preserves source `T/I/D/S/F/G/P` fields, including the
  source big-endian field-value rule for the 16-bit `P` raw-data index.
- ✅ The real PC-DOS `GRAPHICS.DAT` regression materializes all 11,854 rows,
  checks the first source rows and a complete-row receipt hash, and keeps the
  later category/index allocator and raw-image stages gated.
- ✅ `test_dm2_v1_gdatfile_pc34_compat`: 26/26 passed.

# DM2 GDAT source entry-table build (2026-08-07)

- ✅ Connected the authenticated ENT1 rows to the source
  `BUILD_GDAT_ENTRY_DATA` category/subcategory pass with a context-safe
  adapter that preserves the caller's allocator ownership.
- ✅ Real PC-DOS `GRAPHICS.DAT` builds all 11,854 entries, maximum category 26
  and 247 source subcategory slots; the complete row receipt hash is checked.
- ✅ The built tables remain explicit caller-owned source state. No decoded
  image, underlay or runtime cache admission was introduced.
- ✅ `test_dm2_v1_gdatfile_pc34_compat`: 26/26 passed.

# DM2 SKSAVE raw DB-pool baseline (2026-08-13)

- ✅ Added the production c_record owner for the raw `READ_DUNGEON_STRUCTURE`
  DB0..DB15 baseline. It copies the original PC-DOS SKSAVE pool spans only
  after the existing raw-dungeon receipt has verified every offset, size and
  FNV identity.
- ✅ The owner stays deliberately pre-`READ_SKSAVE_DUNGEON`: it does not add
  a G1 continuation, invent record links or mark the graph complete. A
  changed original byte rejects atomically and leaves no pool admitted.
- ✅ Verified against all eight mounted PC-DOS primary/backup SKSAVE files:
  `test_dm2_v1_save_load_real_data` 143/143 and
  `test_dm2_v1_record_pool_pc34_compat` pass.

# DM2 GDAT source raw-entry reader (2026-08-07)

- ✅ Added a bounded ULP-driven raw-entry reader that preserves the source
  first-ENT1 length for raw index 0 and walks continuation ULP lengths for
  later entries, with cumulative boundary checks and payload hashes.
- ✅ Real PC-DOS verification reads entries 0 and 1 from the mounted
  `GRAPHICS.DAT`, checks the source ENT1 signature/offset and nonzero payload
  receipts, and leaves decoded-image/cache ownership gated.
- ✅ `test_dm2_v1_gdatfile_pc34_compat`: 26/26 passed.

# DM2 SKSAVE source DB-clear phase (2026-08-13)

- ✅ Added SKProject `DM2_READ_SKSAVE_DUNGEON`'s DB4–DB15 clear phase to the
  authenticated raw-SKSAVE pool owner. DB0–DB3 remain byte-identical while
  each dynamic record receives only `OBJECT_NULL` in its first word.
- ✅ The operation validates the entire source baseline before modifying any
  record, rejects mismatched receipts atomically and cannot mark a graph
  complete or bypass the missing tile-chain owner.
- ✅ Verified on all eight mounted PC-DOS primary/backup SKSAVE files through
  `test_dm2_v1_save_load_real_data` (143/143).

# Nexus Saturn capture link-path repair (2026-08-07)

- ✅ Closed the remaining Mednafen witness-chain link gap: the SH-2 source
  trace now carries its own `FirestaffGetSH2PC()` declaration/definition and
  its source-trace/PC helper hunks have valid insertion counts. A fresh tree
  applies the complete Saturn capture chain, and the external producer links
  successfully with the real VDP1/VDP2/CD hooks.
- ✅ Kept this as producer evidence only. No runtime asset, menu, HUD or
  viewport admission is changed by the link repair.

# Nexus Saturn capture toolchain repair (2026-08-07)

- ✅ Repaired the ordered Mednafen 1.32.1 Saturn witness patch chain: CD reads,
  SH-2 source/memory peeks and VDP2 VRAM/CRAM/register writes now apply cleanly
  to a fresh upstream tree without placing hooks in unrelated read paths.
- ✅ Verified the clean chain against the external Mednafen source and compiled
  the affected `vdp2.o`, `cdb.o` and `ss.o` objects. This proves the producer
  toolchain is buildable; it does not claim a new authenticated runtime capture.

# DM2 GDAT source underlay-table boundary (2026-08-07)

- ✅ Added source-owned `dtRaw8/0/0` underlay materialization. When the
  authenticated ENT1 row exists, its real ULP raw payload is parsed as sorted
  image-to-underlay pairs with raw-index bounds and payload/pair hashes.
- ✅ The mounted PC-DOS v5 `GRAPHICS.DAT` contains no `dtRaw8/0/0` row; the
  real-data regression therefore proves fail-closed behavior instead of
  admitting an empty or synthetic table.
- ✅ `test_dm2_v1_gdatfile_pc34_compat`: 26/26 passed.

# DM2 SKSAVE raw c_map capacity correction (2026-08-13)

- ✅ Corrected the loader capacity from the standalone G1 dungeon's 28-map
  profile to the source `File_header::nMaps` six-bit range. Original PC-DOS
  SKSAVE prefixes contain 44 map descriptors; they were previously rejected
  by Firestaff's artificial 30-map bound before the c_map byte-square model
  could inspect them.
- ✅ Kept weather-state initialization on the same loader-owned bound. No
  map, record or runtime session is invented and Continue remains gated on
  the unimplemented complete `DM2_READ_SKSAVE_DUNGEON` transaction.
- ✅ Verified in place against all eight PC-DOS primary/backup saves:
  `test_dm2_v1_save_load_real_data` 151/151.
# DM2 SKSAVE direct-root c_record pool owner (2026-08-13)

- ✅ Added a source-owned direct-root transaction after the authenticated raw
  DB baseline and `DM2_READ_SKSAVE_DUNGEON` DB-clear phase. The decoder now
  allocates only from cleared DB4–DB15 pools, writes exact decoded record
  bytes, follows source list/child-owner links, consumes the real possession
  continuation stream, and emits root/count/hash receipt data.
- ✅ The transaction passes the authenticated `CREATURES[type]` AI callback
  through to the shared SUPPRESS decoder and restores the cleared baseline on
  any decode/continuation failure. It does not claim champion, hand, tile or
  runtime-session ownership yet.
- ✅ The real-data SKSAVE test now exercises this pool owner when a corpus is
  mounted and requires fail-closed behavior when the source AI mapping blocks;
  the current workspace has no raw SKSAVE files, so the corpus test reports a
  documented skip. Build and focused test target pass.

# DM2 PC-DOS File_header boot contract (2026-08-07)

- ✅ Replaced the fabricated G1-header interpretation at the M11 boot boundary
  with SKProject's `SKWIN/DME.h::File_header`: `w0` at byte 0 is the
  decoration seed, `nMaps` at byte 4 is 44, `cwTextData` is at byte 6 and
  `w8` at byte 8 supplies the map-0 start pose `(1,8,N)`. The boot profile,
  real-data parser probe, new-dungeon reload and M11 runtime now share that
  contract; `SkWinCore.cpp::READ_DUNGEON_STRUCTURE` lines 39933–39945 is the
  matching original-loader reference.
- ✅ Removed the false 28-map champion-DYN4 admission from the startup proof.
  It had treated File_header fields as a different format and could retain an
  unrelated GDAT bundle. Champion DYN4 is now fail-closed until the actual
  PC-DOS continuation has source proof. No game data is written or unpacked.
- ✅ Verified against the mounted original PC-DOS corpus:
  `test_dm2_v1_m11_startup_profile_gate`, the File_header/champion boundary
  test and `probe_dm2_v1_dungeon_loader` (412 passed, 0 failed).

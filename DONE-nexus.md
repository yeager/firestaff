# Firestaff DONE - NEXUS

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-07-11 - Nexus FACE.BIN readiness gate

- `FACE.BIN` now has a byte-evidenced descriptor: a 56-byte `FACE` header,
  20 variable PRS3 frames, 128..131-byte opaque prefixes, 3,136-byte declared
  output per frame, and a two-byte container tail. PRS3 opcode and prefix
  palette semantics remain unproven. `nexus_ui_expand_face_record_48x48()`
  therefore leaves every canonical PRS3 frame blocked and
  `nexus_ui_load_face_record()` leaves no surface allocated on this path.
- M11 now honors the Nexus launcher asset receipt for title, save, and champion
  startup input instead of replaying blocked actions through ungated host facts.
  Canonical compact FACE media remains at title with `blocked-faces` and cannot
  reach champion rendering.
- `test_m11_nexus_startup_gate` and the new skip-safe
  `firestaff_nexus_v1_face_media_probe` read staged canonical Saturn bytes,
  validate exact first/final PRS3 frame bounds, and prove the non-crashing,
  no-synthesis gate. Verification: focused Ninja build; both direct probes
  pass with local Saturn media; strict C11 `-Wall -Wextra -Werror` syntax
  checks pass for the FACE descriptor and startup loader.
- ✅ 2026-07-11 CSB DSA source `LOAD` opcode-family handoff: added a bounded
  CSBWin DSA-word decoder/executor for one complete `DSACMD_LOAD` family,
  operating only on checksum-authenticated, runtime-owned `DSAAction` words
  after resume. It implements `A..Z`, `INTEGER`, `ABS`, `DOLLAR`, and
  `INTEGER32`, including CSBWin's signed five-bit next-state and `-16`
  extension word. `LOAD_ABS32` is retained as source-illegal because
  `DSA.cpp` has no execution case for its declared selector. Unsupported
  opcodes do not run or receive substitute behavior. Focused DSA unit and
  authenticated resume-handoff checks pass. Source: CSBWin `Data.h:1686-1708,
  1947-1984`; `DSA.cpp:1074-1189`; `SaveGame.cpp` DSA read boundary.
- ✅ 2026-07-11 DM2 GDAT fallback removal (`DM2-GDAT-FB-01`, `DM2-GDAT-FB-02`): real boot-profile frames now mark `GRAPHICSSET/<MapGraphicsStyle>` floor/ceiling and selected `WALL_GFX` failures as explicit blocked no-draw material receipts instead of painting the old gray/brown planes or aggregate/per-panel wall rectangles. Synthetic and no-data renderer paths keep their deterministic fallback behavior. Runtime frame ownership carries the blocked count/mask and refuses a full GDAT frame while any required base material is blocked. Source lock: skproject `SKWIN/SkWinCore.cpp` `MapGraphicsStyle()`/`GDAT_CATEGORY_GRAPHICSSET` queries and `DRAW_MAP_CHIP` `GDAT_CATEGORY_WALL_GFX` `QUERY_DUNGEON_MAP_CHIP_PICT` route. Coverage: the runtime handoff smoke test forces missing floor, ceiling, and wall GDAT assets and verifies receipts plus untouched framebuffer pixels; the hash-verified boot-profile gate requires zero blocked material receipts on the real frame. Verification: `cmake --build build --target test_dm2_v1_runtime_handoff_smoke test_dm2_v1_boot_profile_smoke --parallel 4` completed clean; `ctest --test-dir build -R '^dm2_v1_runtime_handoff_smoke$' --output-on-failure` passed.

- ✅ 2026-07-11 Theron Track02 repeatable nonstartup-region catalog: scanned
  both hash-verified raw JP/US Track 02 BIN sector streams after excluding the
  six indexed all-zero logical containers. The new strict locator retains 11
  repeatable nonzero MODE1 user-data runs only when their sector spans differ
  by the observed one-sector physical offset and their full user-data byte
  counts, nonzero counts, and hashes agree. It records both variants' raw and
  sector bounds plus rejected nonrepeatable runs as negative evidence. The
  catalog accepts no same/unknown variant, exports no bytes, assigns no
  semantics, and remains opaque/promotion-blocked. Verification: strict
  `cc -std=c11 -Wall -Wextra -Werror` compile of `theron_v1_track02.c`; direct
  focused `firestaff_theron_v1_track02_nonstartup_sector_receipt_probe` passed
  against the staged raw JP/US media (0 failures, 0 skips). The normal CMake
  target remains blocked by unrelated CSB undeclared DSA opcode symbols.
- ✅ 2026-07-11 DM2 G1 map-owned `GenericRecord::w0` traversal gate: replaced
  the over-broad whole-pool direct-link census with the source-shaped walk
  rooted only in `c_map` tile links. The new validator resolves each
  map-reachable ObjectID through the proven `READ_DUNGEON_STRUCTURE`
  text-adjacent 16-pool bases, rejects invalid DB type/index and record bounds,
  and rejects repeated record addresses per chain. This follows skproject
  `SKWINSPX/src/v4/skcore.cpp` `GET_ADDRESS_OF_RECORD` / `GET_NEXT_RECORD_LINK`
  lines 1184-1224 and `SKWIN/DME.h` `GenericRecord::w0` lines 831-847; it does
  not turn unused pool words into inferred links. Verification: focused target
  build passed; `test_dm2_v1_dungeon_loader_first_map_gate` passed 70/70,
  including unreachable-invalid, reachable-out-of-range, and reachable-cycle
  gates; the hash-verified PC G1 real-data probe passed 44/44 against
  `~/.firestaff/data/dm2/DUNGEON.DAT`. Honest scope: real G1 remains
  non-traversable because a map-reachable `w0` still fails strict validation.
# 2026-07-11 - Nexus Structure1G animation declaration handoff

- ✅ 2026-07-11 CSB F0115 native weapon/scroll/food composition: extended the CSB-only native resolver from chest/potion rows to all 46 PC34 weapon subtypes, the scroll, and junk food types 29-36. Mappings are source-locked to ReDMCSB `DUNGEON.C F0141:1147-1154`, `G0237` object-info rows 23-68 and 156-163, and `DUNVIEW.C G0209:1216-1307 / F0115:4923-5073`; `GRAPHICS.DAT` indices are generated only from those G0209 native-relative rows. The native compositor now applies horizontal mirroring only for source-marked `GraphicInfo & 0x0001` art, retains chest alcove suppression, and rejects unsupported indices. Focused CSB test covers resolver boundaries, right-lane no-flip behavior, and every admitted real-asset graphic when `FIRESTAFF_CSB_GRAPHICS_DAT` is staged.

- ✅ Nexus now validates the documented Structure1G declaration grammar using
  the canonical LEV00-LEV15 corpus: counted 8-byte descriptors, a terminal
  `FF` descriptor whose trailing bytes remain unconstrained, Structure2 image
  instructions, backward `FF FE` gotos, and `FF FF` sequence ends. The parser
  no longer assumes named Structure1 header pointers are address ordered.
  Real-media evidence is 14 present/valid tables, 51 declarations/sequences,
  154 image instructions, and 51 backward gotos.
- ✅ The source-evidenced Structure1B low-nibble animated-floor form is now
  carried into the DGN render plan with its animation ID and first Structure2
  image index. Only LEV08 uses it: 41 cells, all bound to declared animation
  ID 0. There is no inferred timing, flag execution, model-face animation, or
  static-material substitution. Since no verified Structure2-to-DMDF/BPK
  material bridge exists, any visible animated floor blocks the real DGN plan
  with fallback disabled.
- Verification: strict C11 `-Wall -Wextra -Werror` syntax checks;
  `test_nexus_v1_dgn_geometry_readiness` with local canonical LEV00-LEV15;
  focused CTest `nexus_v1_dgn_geometry_readiness` (1/1); and
  `firestaff_nexus_v1_dgn_material_corpus_probe` against local Track 1 media.
- ✅ 2026-07-11 DM1 V1 PC34 champion-panel runtime pixels: reordered M11's
  normal V1 panel lane to match ReDMCSB `STARTUP2.C` (`CASTER.C F0394`,
  `ACTIDRAW.C F0387`, then `MENUDRAW.C F0395`), corrected the real
  `G0498[12] -> C04` empty-hand palette remap, and kept C011's two
  12-scanline source rows non-overlapping at y=50..61 and y=62..73. The
  headless probe now skips unrelated launcher screenshot-gallery I/O before
  opening its selected real-data entry. Verification:
  `firestaff_dm1_v1_champion_panel_pixels_runtime_probe` passed against real
  DM1 PC 3.4 `GRAPHICS.DAT`/`DUNGEON.DAT`; all four C089..C092 empty-hand
  icons matched all 256 source pixels, and C011 lines 2 and 3 retained all
  168 checked source pixels.
# ✅ 2026-07-12 CSB F0267 object sensor-to-event bridge: loaded ordinary-object movement now carries each F0276 remote floor-sensor result through F0272 target-square resolution and schedules its F0268 `TIMELINE_EVENT_SQUARE_STATE` in source order. The route supports source event types fakewall, teleporter, pit, and door; it preserves same-map target context, target cell, SET/CLEAR/TOGGLE effect, and the one-tick minimum for zero delay. The existing square-state dispatcher owns the final mutation. Source: ReDMCSB `MOVESENS.C F0267/F0268/F0272/F0276` and `TIMELINE.C F0242/F0244/F0250/F0251`. Verified by `test_csb_v1_f0267_loaded_chain_pc34_compat`, including an object C004 sensor routed to a delayed door event.

# ✅ 2026-07-12 CSB F0276 object source-unlink ordering: the live C49 associated-object teleporter route now calls the real-format C004 sensor pass both after source materialization/link and immediately after its source unlink, before target relink. The shared C004 path models ReDMCSB `MOVESENS.C F0276` `AddThing ^ RevertEffect` behavior, including HOLD resolving to SET for addition and CLEAR for removal. Object pit and stairs hops use the same source-removal pass. Regression: `test_csb_v1_runtime_tick_accumulator` locks the C49 C004/C05 source chain and its coalesced pending CLEAR result; `test_csb_v1_teleporter_rotation_runtime_pc34_compat` passes 114/114. The broad runtime accumulator has one pre-existing unrelated `MOVE_FORWARD boundary reaches the bounded open-step runtime movement` failure.

# ✅ 2026-07-12 CSB F0276 C001 object pressure-plate chain: extended the live real-format ordinary-object sensor pass from C004 to source C001 floor plates. The C001 path follows ReDMCSB `MOVESENS.C F0276` lines 1608-1655 and 1664-1667: it evaluates the pre-link-equivalent object/group/party occupancy state, then resolves `AddThing ^ RevertEffect` and HOLD. The dedicated C49 associated-object → object-scope C05 regression proves source C001/C05 preservation, target relink, and the ordered add SET/source-unlink CLEAR result. Verification: `test_csb_v1_f0276_object_chain_pc34_compat` passed 8/8; `test_csb_v1_f0267_loaded_chain_pc34_compat` passed.

# ✅ 2026-07-12 CSB F0276 audible C004 object route: triggered Audible C004 floor-object sensors now request ReDMCSB's prioritized switch sound before publishing their ordinary F0272/F0268 square event. Source: `MOVESENS.C F0276` lines 1770-1772 and `SOUND.C F0064`. Verification: the dedicated real-format C49 materialization regression passed 7/7 and locks `SOUND_SWITCH`, volume 64, priority 4, one audio request, and the target fakewall SET event; the focused F0267/F0276 CTest group passed 3/3.

# ✅ 2026-07-12 CSB F0276/F0272 C004 OnceOnly writeback: a triggered real-format C004 floor-object sensor now disables itself before publishing its first effect, preserving its data bits. Source: ReDMCSB `MOVESENS.C F0272` lines 1191-1193 and F0276 trigger path. Verification: the dedicated C49 materialization regression passed 7/7 and proves sensor-type zeroing plus the initial fakewall SET event; the focused F0267/F0276 CTest group passed 4/4.

# ✅ 2026-07-12 CSB F0276/F0272 C004 Value timing: C004 object sensor remote effects now carry the original four-bit `Remote.Value` delay into F0268 timeline scheduling instead of always using the current tick. Source: ReDMCSB `MOVESENS.C F0272` lines 1194-1203. Verification: the dedicated C49 materialization regression passed 6/6 and proves a source `Value=3` trigger at game time 1 queues its fakewall SET for time 4; the focused F0267/F0276 CTest group passed 5/5.

# ✅ 2026-07-12 CSB F0276/F0272 C004 target-cell semantics: remote C004 effects now preserve `Remote.TargetCell` only for wall targets; fakewalls, doors, pits, teleporters, and corridors queue `CELL_NORTHWEST` as in ReDMCSB F0272. Source: `MOVESENS.C F0272` lines 1201-1207. Verification: the dedicated C49 C004 regression passed 5/5 and proves an encoded cell 3 fakewall target queues cell 0; the focused F0267/F0276 CTest group passed 6/6.

# ✅ 2026-07-12 CSB F0276 C004 Revert ordering: locked the existing source `AddThing ^ RevertEffect` behavior with a real-format C49 associated-object → object-scope C05 chain. A non-HOLD Revert C004 suppresses the source materialization/addition and publishes its SET only when F0267 unlinks the object before teleporter relink. Source: ReDMCSB `MOVESENS.C F0276` lines 1663-1694 and 1760-1778. Verification: the dedicated regression passed 7/7; the focused F0267/F0276 CTest group passed 7/7.

# ✅ 2026-07-12 CSB F0276/F0270/F0271 C004 LocalEffect: object-triggered local C004 effects now retain the final local `CLEAR`/`TOGGLE` while scanning the square and rotate the complete source sensor run only after the pass, with no F0268 remote event. Source: ReDMCSB `MOVESENS.C F0270` lines 1080-1098, F0271 lines 1100-1158, and F0272/F0276 local-effect path. Verification: the dedicated C49 two-sensor regression passed 6/6; the focused F0267/F0276 CTest group passed 8/8. The separate C10 steal-skill local effect remains outside this bounded rotation route.

# ✅ 2026-07-12 CSB F0276/F0270 C10 local skill XP: a real-format C004 LocalEffect value 10 now follows ReDMCSB's immediate F0269 path instead of being treated as a deferred sensor rotation. Object-triggered C49 materialization divides the original 300 Steal XP by party count, skips dead champions after division, and credits both hidden Steal (8) and base Ninja skill XP. Source: ReDMCSB `MOVESENS.C F0269` lines 1038-1078, `F0270` lines 1088-1094, and `CHAMPION.C F0304` lines 879-906. Verification: dedicated live C49 regression passed 8/8; the manually rebuilt focused nine-binary F0267/F0276 group passed while shared CMake regeneration was blocked by two unrelated missing Theron probe sources.

# ✅ 2026-07-12 CSB F0276 C007 floor-creature group route: C04 group relocations now run source/destination F0276 passes and use the real-format sensor scan for C002/C007 group eligibility. A C007 sensor on the destination publishes its normal F0272/F0268 fakewall SET event after the group is relinked. Source: ReDMCSB `MOVESENS.C F0267` lines 800-867 and `F0276` lines 1658-1778, especially C007 lines 1712-1715. Verification: `test_csb_v1_f0276_group_creature_sensor_pc34_compat` passed 4/4; the manually rebuilt focused nine-binary F0276 group also passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C002 party route: party movement now evaluates C002 floor Theron/party/creature sensors at the live F0267 destination. It follows ReDMCSB's party/no-group eligibility, then uses the existing F0272/F0268 remote effect route. Source: ReDMCSB `MOVESENS.C F0267` lines 792-857 and `F0276` lines 1686-1689. Verification: `test_csb_v1_f0276_party_c002_sensor_pc34_compat` passed 5/5 through `MOVE_FORWARD`; the manually rebuilt focused ten-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C002 group route: the shared C002/C007 group eligibility route now has a dedicated live C04 move regression. A C04 group landing on a C002 floor Theron/party/creature sensor publishes the normal F0272/F0268 fakewall SET event. Source: ReDMCSB `MOVESENS.C F0267` lines 800-867 and `F0276` lines 1686-1689. Verification: `test_csb_v1_f0276_group_c002_sensor_pc34_compat` passed 4/4; the manually rebuilt focused eleven-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C001 group route: live C04 group movement now admits the ReDMCSB C001 floor Theron/party/creature/object branch only when party, ordinary objects, and another group are absent. A C04 group landing on C001 publishes the normal F0272/F0268 fakewall SET event. Source: ReDMCSB `MOVESENS.C F0276` lines 1678-1685. Verification: `test_csb_v1_f0276_group_c001_sensor_pc34_compat` passed 4/4; the manually rebuilt focused twelve-binary F0276 group passed with strict runtime compilation.

# ✅ 2026-07-12 CSB F0276 C001 party route: live party movement now evaluates C001 floor Theron/party/creature/object sensors only after a true F0267 move into an empty, group-free destination. Same-square F0284 turns pass `PartySquare` and correctly suppress C001. Source: ReDMCSB `MOVESENS.C F0267` lines 792-857 and `F0276` lines 1678-1685. Verification: `test_csb_v1_f0276_party_c001_sensor_pc34_compat` passed 6/6 through `MOVE_FORWARD` and `TURN_RIGHT`; the manually rebuilt focused thirteen-binary F0276 group passed with strict runtime compilation.

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

# 2026-07-13 Nexus DGN static-material selector guard

The DGN runtime no longer treats Structure1B bytes 3/4 as direct
`SN_WALL.MNS` material IDs. The real LEV00-LEV15 corpus contains values beyond
the bank's 0..14 descriptor range, so a hash-bound MNS pair now requires an
explicit selector-binding proof before it can promote a DGN render plan. The
existing material-raster test supplies that proof only as a controlled host
fixture; production remains blocked without a Saturn executable/capture route.
Verification: Ninja build plus `test_nexus_v1_dgn_material_raster` and
`test_nexus_v1_dmdf_embedded_blocks` against the real local MNS asset.

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
# Nexus MNS TEXT Atomic Material Route (2026-07-13)

- `nexus_v1_dmdf_decode_text_material_bank()` now fails closed for the whole
  authenticated MNS TEXT bank when any descriptor cannot occupy a unique
  256-entry host slot, allocation fails, or a surface exceeds the indexed
  palette capacity. The DGN route cannot use a partial original material bank.
- `test_nexus_v1_dmdf_embedded_blocks` covers the structurally valid but
  out-of-bank source-ID rejection and passes against both local canonical
  `SN_FLOOR.MNS` and `SN_WALL.MNS` assets.

# ✅ 2026-07-13 DM2 G1 DB2 dungeon-text decoding

DM2 V1 now consumes the original PC G1 direct DB2 `TextMode()==0` text-table
route from skproject `SkWinCore.cpp QUERY_MESSAGE_TEXT` (0CEE:159B): visible
map-5 `Text::TextIndex()` values decode three 5-bit glyphs per little-endian
`dunTextData` word until the source terminator. The runtime receipt retains
the source map position and ObjectID for later interaction presentation.
Mode-one GDAT message rows and private phrase-bank escapes 29/30 explicitly
remain unavailable, with no generated replacement text. Verification:
`test_dm2_v1_g1_text_message_runtime` checks literal decoding, source
placement, phrase-bank rejection, GDAT-only skip, and untrusted-receipt
rejection.

# ✅ 2026-07-13 DM2 G1 world-model c_record/map provenance handoff

`dm2_world_from_mem()` now transfers the validated loader-owned PC G1 source
record into the world model instead of discarding it after tile materialization.
`dm2_world_get_verified_g1_map_source()` exposes it only after the exact
skproject `READ_DUNGEON_STRUCTURE` text-adjacent pool transform and the
transactional incomplete map-boot receipt both validate. The retained source
pins the original byte-square map base, c_record pool bases, extension boundary,
and 878 materialized roots, while `GenericRecord::w0` graph traversal remains
disabled. Verification: the real-data handoff test passed against the
hash-verified 39,437-byte PC G1 `DUNGEON.DAT`; the focused loader gate passed
87/87.

# ✅ 2026-07-13 DM2 G1 per-map runtime admission receipt

The loader now validates a selected original PC G1 map at the exact skproject
`c_map.cpp` boundary before runtime consumption: its descriptor-bounded raw
span, source-order c_record pool gate, and every map-owned ObjectID root are
reclassified as direct, DB3, DB4, or explicitly blocked. The receipt reads no
record payload and never reads `GenericRecord::w0`. Real map 0 admits 22 direct
roots; map 16 admits its raw span while retaining 11 DB3, 2 DB4, and 3 blocked
DB8/DB10 roots. Verification: new real-data test against hash-verified PC G1
`DUNGEON.DAT` passed; focused loader gate passed 87/87.

# ✅ 2026-07-13 DM2 G1 direct DB0 Door runtime receipt

Runtime-admitted G1 maps now consume direct DB0 `Door::w2` records through
the source-defined `c_map.cpp` root route. The read-only receipt retains only
the `DME.h` button, door-type, button-state, opening-direction, ornament,
fireball, and chopping bits; `GenericRecord::w0` is neither read nor followed.
The real PC G1 map 9 receipt locks three direct doors, including the active
button at `(7,3)` with ObjectID `0x0006`. Verification: real-data DB0 test,
runtime-map admission test, and focused loader gate all passed.

# ✅ 2026-07-13 DM2 G1 direct DB3 Actuator runtime receipt

Runtime-admitted G1 map 5 now consumes only direct DB3 `Actuator` records.
The receipt binds the source-defined `DME.h` `w2`, `w4`, and `w6` fields:
type/data, graphic number, flags, delay/action, and target pose. It never
reads `GenericRecord::w0`, skips the separately bounded DB3 extension range,
and cannot publish an unadmitted map. The canonical 39,437-byte PC G1
`DUNGEON.DAT` proves 16 direct roots, beginning with ObjectID `0x4c04` at
`(6,14)`. Verification: real-data Actuator test and the focused 87-check
dungeon-loader gate passed.

# ✅ 2026-07-13 DM2 G1 direct DB4 Creature runtime receipt

Runtime-admitted G1 map 17 now consumes only direct DB4 `Creature` records.
The read-only receipt binds `DME.h` `b4` `CreatureType` and `w6` `HP1`; it
does not read `GenericRecord::w0`, the `w2` possession ObjectID, or any DB4
extension record. The canonical 39,437-byte PC G1 `DUNGEON.DAT` proves four
direct roots, beginning with ObjectID `0x1098` at `(4,4)`. Verification:
real-data Creature test and focused 87-check dungeon-loader gate passed.

# ✅ 2026-07-13 DM2 G1 direct DB5 Weapon runtime receipt

Runtime-admitted G1 map 17 now consumes only direct DB5 `Weapon` records.
The receipt binds `DME.h` `w2` `ItemType`, `Important`, and `Charges` fields
without reading `GenericRecord::w0` or attempting a record chain. The
canonical 39,437-byte PC G1 `DUNGEON.DAT` proves two placements of ObjectID
`0xd407`, at `(5,8)` and `(6,1)`. Verification: real-data Weapon test and
focused 87-check dungeon-loader gate passed.

# ✅ 2026-07-15 DM2 direct DB5 Weapon GDAT no-draw gate

`Weapon::ItemType()` now has a source-bound runtime material route to
`WEAPONS/item/F9`, retaining the direct G1 owner tile, ObjectID, direction,
raw hash, image metadata, and local-palette hash only when every exact GDAT
read succeeds. Boot and runtime refresh the receipt with the active map.
The local canonical corpus proves the safety boundary: its two direct map-17
DB5 roots use item 126, but `GRAPHICS.DAT` contains no exact
`WEAPONS/126/dtImage/F9`; raw lookup is attempted once and the route rejects
before metadata or palette access. This is deliberately no-draw, not a
substitute item surface. Verification: local canonical real-data gate.

# ✅ 2026-07-15 DM2 direct G1 item decoded-pixel binding

The direct DB5 and DB9 `DRAW_MAP_CHIP` routes now retain a row-wise hash of
the exact decoded `WEAPONS/itemType/F9` or `CONTAINERS/containerType/F9`
bitmap only after boot proves the matching virtual GDAT address, raw-byte
receipt, dimensions, and decoded pixels. M10 recomputes that hash immediately
before the item blit, so a stale or replaced decoded provider buffer is
blocked even when its ObjectID, tile, dimensions, and local IMG3 palette
match. Missing canonical DB5/DB9 map-chip media remains no-draw. Verification:
`test_dm2_v1_g1_weapon_viewport_material_gate` and
`test_dm2_v1_g1_container_viewport_material_gate` exercise both a valid draw
and a changed-pixel rejection.

# ✅ 2026-07-15 DM2 raw SKSave DB-pool offset receipt

Raw SKSave receipts now retain exact source-order offsets for each DB pool and
the `warr_00[1]` map-data span beside the existing count/hash evidence. These
offsets do not authorize unknown record or link decoding. The focused save/load
fixture proves the empty-pool/map-data boundary at byte 68 and SUPPRESS starts
at byte 88.

# ✅ 2026-07-15 DM2 raw SKSave DB-record address receipt

An admitted raw SKSave can now yield a hash-bound `(DB pool, record index)`
receipt with the exact skproject record size and source offset. The boundary
revalidates the complete prefix and rejects absent pools, invalid indices,
zero-sized source pools, and out-of-prefix addresses. It exposes no decoded
record fields and follows no links. Verification: focused save/load 26/26,
including a source-sized DB0 record at byte 68.

# ✅ 2026-07-15 DM2 raw SKSave runtime layout handoff

`dm2_v1_runtime_restore_save_candidate()` now retains the authenticated raw
dungeon receipt on its save candidate and rejects before its atomic dungeon
swap unless the reparsed layout exactly reproduces SKProject
`DM2_READ_DUNGEON_STRUCTURE`'s map, column, ground-stack, text, DB-pool, and
map-data boundaries. Every nonempty DB pool also proves its first and final
source record address against the retained prefix; empty pools must have no
span. This consumes no record links or object fields. Verification: focused
raw-SKSave candidate test plus syntax checks for the DM2 parser, runtime, and
save/load test translation units.

# ✅ 2026-07-15 DM2 raw SKSave reachable-record gate

Original raw-SKSave runtime restoration now gives `GenericRecord::w0` meaning
only after `c_map.cpp` marks a square as thing-bearing. Those roots must pass
the bounded `c_record.cpp` ObjectID/pool/terminating-chain gate before the
candidate can replace the live dungeon. A malformed marked square with no
ground-stack root is rejected atomically; unused pool slots remain opaque and
cannot cause rejection. Verification: focused raw-SKSave runtime regression
plus syntax checks for DM2 runtime and save/load test translation units.

# ✅ 2026-07-15 DM2 raw SKSave load-to-first-frame handoff

The DM2 runtime now retains an exact raw-SKSave map/pool receipt only after
the source-layout dungeon and session commit together. The V1/M10 render path
marks it consumed only when the live dungeon prefix hash, byte count, and
party pose still match the accepted `GAME_LOAD` candidate; boot's render
receipt carries the resulting identity for its M11 consumer. A rejected raw
candidate leaves both the prior dungeon and prior handoff receipt intact.
Verification: focused raw-SKSave restore test drives the accepted candidate
through a first V1 frame, then proves malformed record-chain rejection remains
atomic; syntax checks cover runtime, boot, and save/load test units.

# ✅ 2026-07-15 DM2 raw SKSave encoded timer byte-span receipt

The original-save corpus state receipt now preserves the source byte window
for raw-SKSave SUPPRESS timer entries: exact payload offset, byte count, and
hash are derived only after the file-hash gate, raw dungeon-prefix parser, and
source importer agree on the same original candidate. The receipt does not
dispatch timers or claim rebuilt DB graph ownership. Verification:
`test_dm2_v1_save_load` covers a renamed raw corpus artifact with a real
encoded timer span and keeps envelope candidates at zero raw-timer span.

# ✅ 2026-07-15 DM2 raw SKSave DB0 Door receipt

The corpus path can now decode `SKWIN/DME.h::Door` fields from raw DB0 `w2`
after a hash-bound DB-record receipt validates its address. It returns only
Button, DoorType, ButtonState, OpeningDir, ornate index, and fireball/chopping
flags. `w0`, map attachment, and every record link remain opaque. Verification:
focused save/load 26/26 with a source-sized DB0 record and invalid-index gate.

# ✅ 2026-07-15 DM2 raw SKSave DB3 Actuator receipt

The original-save corpus path now decodes only `SKWIN/DME.h::Actuator`
`w2/w4/w6` from a hash-bound DB3 record: type/data, graphics and action flags,
and target direction/cell. It does not execute the actuator, attach it to a
tile, or inspect `w0`. Verification: focused save/load 26/26 with a
source-sized DB3 record plus invalid-index rejection.

# ✅ 2026-07-15 DM2 raw SKSave DB4 Creature receipt

The original-save path now decodes only `SKWIN/DME.h::Creature` `b4`
CreatureType and `w6` HP1 from a hash-bound DB4 record. Possession, remaining
HP words, animation state, AI, and links stay opaque. Verification: focused
save/load 26/26 with source-sized DB4 data and invalid-index rejection.

# ✅ 2026-07-15 DM2 raw SKSave DB5 Weapon receipt

The original-save path now decodes only `SKWIN/DME.h::Weapon` `w2` ItemType,
Important, and Charges from a hash-bound DB5 record. `w0`, ownership/location,
and all links remain opaque. Verification: focused save/load 26/26 with a
source-sized DB5 record and invalid-index rejection.

# ✅ 2026-07-15 DM2 live DistantEnvironment weather materialization

Outdoor presentation now consumes the exact ten-byte `DME.h::DistantEnvironment`
slot at the final GDAT weather draw. The skproject `cmFW`, `cmCD`, `w4/w6`, and
`b8/b9` registers select mirror mode, rectangle, offsets, and scale; the slot
hash and GDAT command must still match or the layer is not drawn. Verification:
focused weather receipt gate and renderer material gate passed.

# ✅ 2026-07-13 DM2 G1 direct DB9 Container runtime receipt

Runtime-admitted G1 map 9 now consumes the direct DB9 `Container` record at
`(11,19)`. The receipt binds only `DME.h` `b4` `IsOpened` and `ContainerType`
bits. It never reads `GenericRecord::w0`, the `w2` contained-object ID, or a
container chain. The canonical 39,437-byte PC G1 `DUNGEON.DAT` proves ObjectID
`0xe408` with closed/type-zero state. Verification: real-data Container test
and focused 87-check dungeon-loader gate passed.

# ✅ 2026-07-13 DM2 G1 direct-root family census

The partial G1 boot receipt now retains the direct-root count by record type
while reading only `c_map.cpp` ground-stack ObjectIDs. The canonical
39,437-byte PC G1 `DUNGEON.DAT` has direct roots only for DB0, DB1, DB2, DB3,
DB4, DB5, and DB9. DB6, DB7, DB8, and DB10 through DB15 have no direct root,
so no later family can be materialized without a separately proven route.
This reads no record payload or `GenericRecord::w0`. Verification: explicit
build-only real-data census target and focused 87-check dungeon-loader gate
passed.

# ✅ 2026-07-13 DM2 G1 direct tile-to-c_record address receipt

The loader now resolves a runtime-admitted G1 `(level,x,y)` tile through the
skproject `c_map.cpp` ground-stack lookup into the exact direct
`c_record.cpp` `base + record_size * index` address. It accepts only already
proven DB0 through DB5 and DB9 records, validates every resulting offset and
size before the G1 extension boundary, and returns no payload pointer. The
canonical real-data test locks direct DB0, DB1, DB2, DB3, DB4, DB5, and DB9
addresses; all other types, extensions, `w0`, and possession routes reject.
Verification: build-only real-data address target and focused 87-check
dungeon-loader gate passed.

# ✅ 2026-07-13 DM2 weather `dtText` provenance boundary

`dm2_v1_weather_gdat` now follows skproject `c_weather.cpp` rather than
treating environment fields as drawable images. It resolves the six exact
`QUERY_GDAT_TEXT(0x17, MapGraphicsStyle, 0x67..0x6c)` command payloads,
retains byte identity in a receipt, and derives the original cloud
`0x10/0x40/0x80` and rain `0x40/0x80/0xc0` selectors. Missing, wrong-type,
or partial commands reject; raw `dtText` is not decoded or rendered until
`QUERY_CMDSTR_TEXT` and its encoding state are source-proven. Verification:
Ninja and `dm2_v1_weather_gdat_receipt` passed.

# ✅ 2026-07-13 DM2 dialogue GDAT material receipt

`dm2_v1_dialogue_gdat_receipt()` binds the exact skproject `c_gui_vp.cpp`
`GRAPHICSSET` dialogue shell fields `-4..-2` and glyph field `3` to source
IMG3 metadata and their `QUERY_GDAT_IMAGE_LOCALPAL` tails. A missing image,
wrong graphics set, non-4bpp record, or absent palette rejects before any
dialogue draw. This is material provenance only: it does not synthesize text,
layout, or pixels. Verification: Ninja and `dm2_v1_dialogue_gdat_receipt`.

# ✅ 2026-07-13 DM2 source save/load-panel orchestration

The runtime now retains the complete `SKULLWIN/c_dialog.cpp`
`DM2_dialog_OPEN_DIALOG_PANEL` command instead of only its follow-up
`RECT_453` save-name redraw. It requires the original
`DIALOG_BOXES/0x81/dtImage/0` material and local palette, the GDAT field-0
and field-1 labels, source palette slots 12/11, and raw4 rectangle IDs
`4`, `450`, `466`, `467`, and `451`. Missing, wrong-type, or empty source
label data fails closed. M11 receives only this source-owned command and
remains responsible for drawing it when a real save/load session is active;
no synthetic panel, text, colour, or coordinates were added. Verification:
Ninja and `dm2_v1_dialogue_gdat_receipt`.

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
# ✅ 2026-07-15 DM2 source-owned map-transition context

Trigger and direct G1 teleporter level handoffs now refresh their complete
map-owned runtime context: door/ornament lists, bounded G1 c_record material
receipts, `UPDATE_GFXSET` scene planes, `CHECK_RECOMPUTE_LIGHT` receipt, and
weather material/destination receipts. The target level's original map type
also selects outdoor mode. All previous receipts are cleared before this
transaction, so a missing destination route fails closed rather than drawing
the prior map's GDAT pixels. Verification: boot smoke 88/88 and save/load
26/26.
# ✅ 2026-07-15 DM2 M11 map-transition source receipt gate

The public M11 transition gate now applies the same `UPDATE_GFXSET` WALL_GFX
semantics to its runtime receipt as to its boot receipt. Indoor frames require
one exact non-empty source wall plan. Outdoor maps may omit an indoor plan
only when both receipts explicitly carry zero wall-plan identity and command
count; a stale or invented plan is rejected. The map-transition and watermark
fixtures now include the required source command-count evidence instead of
implicitly accepting an incomplete indoor receipt.

The same owner geometry can now be emitted with the bounded Structure1A
rotation-selector corpus as a source-only transform capture target. It stays
 no-draw and requires original Saturn execution evidence before any transform
 or camera semantics can be promoted.
# ✅ 2026-07-15 DM2 live weather source-context binding

`DistantEnvironment` slots now enter the runtime only when their ten original
bytes revalidate through the current `MapGraphicsStyle` weather GDAT receipt.
The runtime retains that map token, graphics set, and source receipt identity
with the admitted slots and clears all slots whenever scene control refreshes.
This blocks a same-command weather record from a previous map from drawing on
the next map; unavailable timer output remains no-draw.

That capture target now also carries the exact full 24-byte Structure1A table
and a separate selector-column fingerprint from the active canonical LEV.
This locks a future trace to the raw table without claiming a rotation unit,
matrix, camera convention, culling rule, or draw behavior.

The direct-owner transform-trace admission now rechecks that table identity,
the owner selector, model/face, raw execution lane, and transform-state
snapshot before storing an opaque trace receipt. Missing Saturn provenance
leaves the route blocked; admission never interprets transform bytes.

An external capture can now enter through separate manifest, raw-execution,
and transform-state sidecars. The file reader requires distinct paths and
passes all bytes through the same source gate, but cannot manufacture Saturn
attestation or make the retained state drawable.

The sidecar path now also consumes a separate independent-review attestation.
It must match the direct owner, raw Structure1A table and selector column,
plus both sidecar hashes before it can assert original-Saturn provenance. The
result remains opaque and no-draw; Firestaff does not assign transform or
graphics meaning to the captured bytes.

# Nexus MENU.BPK PALT / WARNING.BIN correlation (2026-07-15)

The canonical MENU.BPK PALT trailer and WARNING.BIN resource 0's documented
DGT2 CLUT share 224 identical indexed big-endian 16-bit words. This establishes
only a source-owned BGR555 word-encoding correlation. The other 32 PALT words
remain raw source values; no PRS3 entry association, palette application, pixel
decode, or rendering route is enabled.

The documented DGT2 BGR555 reader's low-15-bit colour mask also matches 255 of
256 indexed PALT words: 31 raw mismatches are bit-15-only, while index 130 is
one actual colour-word difference. This strengthens the byte correlation but
does not declare PALT to be DGT2 or promote any decoder/palette route.

# Nexus PRS3 V1 nonzero control-byte path (2026-07-15)

The canonical DM.BIN loader now has a source-locked SH-2 receipt for its
nonzero low-bit fallthrough: shifted R11 is tested against mask 1, `BT` selects
the separate zero-side corridor, and the nonzero path guards input, reads one
byte through `@R12+`, then reaches the existing R2 byte store. This proves only
a bounded direct-byte path. It does not establish the zero-side grammar, token
names, buffer ownership, termination, palette use, PRS3 pixels, or rendering.

# Nexus PRS3 V1 zero-side termination condition (2026-07-15)

The hash-bound zero-side corridor now retains the exact SH-2
`CMP/EQ R1,R10; ADD #1,R10; BF/S` control sequence and branch target. It
proves that the repeat loop returns only while the compared registers differ.
That is a static termination condition, not a claim that either register is a
run length or that the zero side is a backreference. No PRS3 output, palette,
or renderer route is enabled.

# Nexus PRS3 V1 zero-side two-byte source span (2026-07-15)

The verified zero-side corridor now records its adjacent `MOV.B @R12+` reads:
one source byte, its sign extension, then the immediately following source
byte. This proves an exact two-byte sequential input span before the raw merge.
It does not assign fields, history access, output, token, palette, or pixel
semantics, and cannot enable decoding or rendering.

# Nexus PRS3 V1 zero-side merge order (2026-07-15)

The source-bound DM.BIN receipt now proves the full static merge sequence:
both sequential inputs are zero-extended, the second byte is copied and shifted
four bits, masked with the PC-relative `0x0f00`, and ORed into the first byte;
the source then masks the second register with immediate `15`. This fixes the
raw merge order as `byte0 | ((byte1 << 4) & 0x0f00)`. It does not assign the
word a PRS3 semantic or permit decoding, palette application, or rendering.

# Nexus PRS3 V1 post-merge control edge (2026-07-15)

The zero-side receipt now binds the actual post-merge sequence: mask R7 with
15, add 2, add merged R4, `CMP/GT R7,R4`, then `BT` back to the PRS3 control
re-entry. It proves that exact static branch condition and target only. No
register role, token rejection, termination, output, palette, decoder, or
renderer meaning is inferred.

# Nexus PRS3 V1 source-counter terminal path (2026-07-15)

The hash-bound DM.BIN receipt now proves that the refill, nonzero, and zero-side
source-counter guards all branch to one shared epilogue. That path writes zero
to R0 before the routine's RTS. It is intentionally recorded only as a static
failure-shaped terminal path: it does not prove a PRS3 end marker, externally
observed status, successful termination, decoder output, palette, or rendering.

# Nexus PRS3 V1 branch-local source consumption (2026-07-15)

The source-locked DM.BIN receipt now records the two exact SH-2 R14 counter
debits that precede direct stream reads: the nonzero low-bit corridor executes
`ADD #-1,R14` before its one-byte `@R12+` read, and the zero-side corridor
executes `ADD #-2,R14` before its two adjacent `@R12+` reads. The receipt
rejects either changed instruction. This establishes only static input-byte
consumption by branch; it does not name PRS3 tokens, fields, output/history
behavior, palette use, decoded pixels, or a renderable surface.

# Nexus PRS3 V1 indexed-byte control operands (2026-07-15)

The source-locked zero-side receipt now records the exact static operand flow
from `MOV.B @(R0,R13),R1` through `CMP/EQ R1,R3` and then `CMP/EQ R1,R10`,
which is immediately followed by the existing `BF/S` repeat decision. The
receipt rejects a changed indexed load or compare corridor. This identifies no
PRS3 token, history buffer, copied/output byte, length, terminator, palette,
pixel, or drawing behavior; those meanings still need an authenticated Saturn
execution trace.

# Nexus PRS3 V1 nonzero post-store control route (2026-07-15)

The retail DM.BIN receipt now locks the complete static nonzero post-store
corridor. After the guarded `R2 -> @(R13,R0)` byte store it executes
`ADD #1,R6`, `AND R5,R6`, and branches to the shared PRS3 control re-entry.
The receipt rejects a changed update, mask, or re-entry branch. This is a
source-control fact only: it does not establish a literal token, R6/R13
allocation, output byte order, decoded data, palette, pixels, or rendering.

# Nexus PRS3 V6 dynamic control-operand capture (2026-07-15)

The VDP1 trace schema now admits V6 only when it includes one ordered dynamic
operand witness for each low-bit branch: the real nonzero and zero R14 debit
PCs, before/after R14 values, before/after R12 cursor values, and decrement to
input-read sequence order. Source binding rechecks both PCs against the locked
DM.BIN receipt. The witness remains unauthenticated capture evidence and does
not prove PRS3 token grammar, output semantics, palette, pixels, or drawing.
# ✅ 2026-07-15 Theron Track 02 transfer-destination call-entry receipt

The original Mednafen trace now admits the Track 02-derived TII destination
only when its bound JSR reaches an exact main-RAM entry row. The nested receipt
retains original byte-range and call provenance without classifying code or
data. Verification: genuine Mednafen 1.32.1 patch dry-run, Ninja focused
targets, `test_theron_rendering` 18/18,
`test_theron_v1_startup_save_resume_pc34` 258/258, raw-loader probe skip-safe,
and the capture contract pass.

# Nexus PRS3 V7 nonzero source/output witness (2026-07-15)

V7 now requires one bounded MENU.BPK byte witness for the nonzero control
corridor: payload offset and byte, the source-locked nonzero output-store PC,
observed output byte/address, and ordered write sequence. Asset binding checks
the input byte against the selected real stream and rejects an output byte that
does not match the source-owned direct-byte route. This is one capture-bound
transfer candidate, not a complete PRS3 literal/token grammar, decoder,
palette/pixel interpretation, or render permission.

# Nexus PRS3 V8 zero-side source/merge witness (2026-07-15)

V8 now admits a bounded zero-side witness only when two adjacent MENU.BPK
source bytes match the selected real stream, their read PCs match the retail
DM.BIN corridor, and their observed merge equals `byte0 | ((byte1 << 4) &
0x0f00)`. The zero-side corridor has no proven direct output store, so V8
deliberately records no fabricated output pair. This does not identify a
token, offset, history copy, length, output, palette, decoded pixel, or draw.
# ✅ 2026-07-15 Theron Track 02 destination copied-byte receipt

The entered routine at the Track 02-derived TII destination now requires its
observed opcode to equal the exact first source byte copied from `$3c88`.
The receipt retains copied and original source addresses while leaving routine,
level, object, palette, bitmap, and rendering semantics unclassified.

# ✅ 2026-07-15 Theron Track 02 copied-entry successor receipt

The first observed successor after the copied destination entry now has to
remain inside the same TII destination span and match its corresponding
original byte (`$3c89`). This extends the byte-to-execution chain without
assigning instruction, record, dungeon, object, palette, bitmap, or rendering
meaning.

# Nexus Structure1F direct face capture bridge (2026-07-15)

One active canonical Structure1F owner can now write an atomic capture-producer
manifest containing its exact LEV identity, Structure1A owner/model/rotation,
selected Structure3 face ordinal, face/vertex/normal row fingerprints, parsed
vertex indexes, and raw Structure1A transform-table fingerprints. The writer
requires the existing source-bound geometry and transform receipts and emits
only `original_saturn_capture_required=1` and `no_draw_only=1`. It does not
decode materials or establish transforms, culling, VDP1, palette, pixels, or
rendering.
# ✅ 2026-07-15 Theron Track 02 copied-entry second-successor receipt

Mednafen now emits a second source-owned successor row after a main-RAM call
entry's first successor. Firestaff admits it only when it remains inside the
same copied TII span and matches original Track 02 byte `$3c8a`. This proves a
third bounded byte-to-execution observation, not instruction role, control
semantics, CD-record selection, dungeon data, or visual meaning.

# Nexus Structure1F direct face host manifest gate (2026-07-15)

The package boundary now consumes a direct face capture manifest only after
rebuilding the active canonical Structure1F target and matching its LEV hash,
owner location, Structure1A/Structure3 references, face/vertex/normal row
fingerprints, vertex indexes, and transform-table fingerprints. Any malformed
or changed field is fail-closed. Acceptance preserves `no_draw_only`; it is
not evidence of a Saturn transform, culling result, material, VDP1 command,
palette, pixels, or rendering.

# Nexus Structure1F capture target trace-route binding (2026-07-15)

Direct-owner transform trace intake now has five distinct files: the canonical
direct-face target, debugger manifest, raw trace, transform snapshot, and
independent attestation. It consumes and validates the target first, so a
changed or malformed package target blocks trace admission even when the other
sidecars remain valid. An accepted trace remains opaque and no-draw; it does
not establish transform, material, palette, pixels, culling, VDP1, or rendering.
# ✅ 2026-07-15 Theron Track 02 copied-entry BRA receipt

The Mednafen main-RAM loader trace now emits HuC6280 `BRA` control rows. The
Track 02-derived entry admission requires opcode `0x80`, its exact copied
displacement byte, and the emulator-computed target to agree. The receipt
records only this bounded control transfer; it does not classify the target as
loader code, a record selector, dungeon data, object data, palette, bitmap, or
rendering behavior.

# Nexus Structure1F launcher host capture gate (2026-07-15)

The launcher now exposes a direct-face capture intake that independently
rechecks its initialized, active, canonical LEV source before delegating to
the engine manifest gate. This carries the exact accepted request to the host
boundary without allowing a caller to substitute another level's data. The
receipt remains capture-only and no-draw: no Saturn transform, face material,
palette, VDP1 command, pixel, or rendering claim is added.

# Nexus Structure1F VDP1 capture prerequisite (2026-07-15)

The direct-face launcher gate now also retains the canonical `DM.BIN` static
SH-2 VDP1 state-write receipt. Its proven raw fields include the `0x04 = 2`
control write, the VDP1-VRAM base handoff, and `0x06 = 0x8000`, `0x08 = 0`,
and `0x0a = 0xffff`; a changed or
unverified executable blocks capture intake. This is a capture prerequisite,
not a claim that those registers select a Structure1F face or prove a command,
transform, material, palette, pixel, or rendering result.

# Nexus Structure1F direct material corpus gate (2026-07-15)

All canonical LEV00--15 files now pass a direct-owner material audit. It finds
zero source-proven Structure1F-to-Structure2 static material links through the
current owner/model/face relation, so the engine cannot turn a shared selector
into a texture or VDP1 claim. The missing link remains an original-Saturn
capture requirement; no material or renderer path was enabled.

# Nexus Structure1F raw VDP1 capture host route (2026-07-15)

`nexus_v1_launcher_dgn_direct_face_raw_capture_intake()` now routes an
authenticated six-lane Structure3 capture through the initialized canonical
LEV host only after the direct Structure1F manifest and the static DM.BIN VDP1
capture prerequisite pass.
`nexus_v1_engine_bind_structure1f_direct_face_raw_capture()` compares the full
DGN/Structure3 face candidate (source, mesh corpus, entry, ordinal,
face/vertex/normal rows, and selector) before retaining opaque capture bytes.
Missing or altered lanes, attestations, source identity, binder receipts, or
face fields fail closed. The route is explicitly no-draw and proves no VDP1
command, texture, palette, transform, or raster semantics.
Verification: `test_nexus_v1_direct_static_material_capture` against local
canonical LEV00--15 and `test_nexus_v1_structure3_capture_manifest` pass.

# Nexus Structure1F VDP1 texture/palette material link (2026-07-15)

`nexus_v1_engine_bind_structure1f_vdp1_material_capture()` now requires the
runtime-owned direct-face capture lanes to be byte-identical to their
authenticated six-lane source. It additionally requires a documented texture
command, a complete VDP1-VRAM snapshot with the exact CMDSRCA texture window,
and a unique copy of that 32-byte command in VDP1 VRAM. The receipt retains
the command's raw `CMDCOLR` word and the exact palette-lane hash together, but
does not interpret either as CRAM/CLUT addressing, colour format, texel order,
or pixels. The launcher consumes this receipt after the opaque capture handoff
and remains fail-closed/no-draw. Verification:
`test_nexus_v1_direct_static_material_capture` and
`test_nexus_v1_dgn_geometry_readiness` pass.

# Nexus Structure1F VDP1 mode-1 lookup decoder (2026-07-15)

The new `nexus_v1_vdp1_decode_mode1_lookup_texture()` implements only the
documented VDP1 mode-1 lookup route: four-bit texture samples are consumed
high nibble then low nibble, and `CMDCOLR * 8` addresses the 32-byte,
16-entry lookup table in the same authenticated VDP1 VRAM snapshot. It emits
raw 16-bit VDP1 colour codes, never RGBA pixels or a draw command. The engine
enters this path only after the direct Structure1F capture, byte-identical
runtime lanes, command, and CMDSRCA window gates pass. An optional witness
comparison is byte-for-byte but cannot authenticate a caller-supplied witness.
This follows Sega's VDP1 User's Manual, sections 6.3--6.5; VDP2/CRAM and
game-specific palette semantics remain blocked. Verification:
`nexus_v1_vdp1_lookup_decode` and
`nexus_v1_direct_static_material_capture` pass.

# Nexus Structure1F VDP1 mode-1 VDP2 palette-chain verifier (2026-07-15)

`nexus_v1_vdp1_resolve_mode1_palette_capture()` now consumes the documented
mode-1 lookup result only alongside an explicitly attested full VDP2 CRAM
capture (4 KiB) and its raw register image. It preserves the VDP1 source-index
rules from `CMDPMOD`: index 0 is transparent when SPD is clear, and index F
ends a source scanline when ECD is clear. A lookup value with bit 15 set is
decoded as RGB555; otherwise the verifier adds captured `SPCAOS` to the
11-bit sprite dot-colour value, applies captured `RAMCTL/CRMD` address rules,
and reads the selected RGB555 or RGB888 CRAM entry. Prohibited CRMD=3,
partial state, and unattested captures fail closed. The output is a
source-independent inspection receipt, never a host draw or a claim of final
Saturn composition. The focused test verifies transparent, direct-RGB,
CRAM-addressed, end-code, post-end suppression, and capture-attestation
rejection paths. It does not claim an actual Nexus output witness: a real
capture's final VDP1/VDP2 pixel stream must still byte-match before rendering
can be enabled. Verification: `nexus_v1_vdp1_lookup_decode`,
`nexus_v1_direct_static_material_capture`, and
`nexus_v1_dgn_geometry_readiness` pass.
# ✅ 2026-07-15 Nexus PRS3 nonzero transfer debit gate

The external SH-2 PRS3 nonzero transfer binder now requires the real
DM.BIN-proven nonzero R14 source-counter debit PC (`0x14dd2`) before it accepts
an opaque MENU.BPK byte-transfer candidate. The previous `zero_branch + 2`
delay-slot shortcut (`0x14dce`) is rejected by regression coverage, so a stale
capture cannot satisfy the nonzero path. This remains evidence-only: no PRS3
opcode grammar, decoder, output ownership, palette, pixel, VDP1, or menu draw
route is promoted. Verification: `nexus_v1_prs3_capture_trace_schema`,
`nexus_v1_bpk_archive`, `nexus_v1_bpk_surface_class`, and
`nexus_v1_bpk_prs3_payload_evidence` pass.

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
# ✅ 2026-07-16 DM2 runtime smoke provider wall-count fixture

The DM2 runtime smoke fixture's injected viewport provider now exposes a
`320x200` wall atlas, large enough for the skproject wall-frame source windows
used by the renderer's rectangle validation. This restores the provider-backed
ten-wall draw count while keeping `gdat_wall_material_plan_consumed == 0`, so
synthetic provider fetches remain distinct from a boot-owned GDAT wall-plan
receipt. Verification: `build-local-ninja/test_dm2_v1_gdat_wall_plan_viewport_real_data`
passes, and `dm2_v1_runtime_handoff_smoke` now passes its wall-count and
wall-plan split assertions. Remaining smoke blockers are the independent
indoor floor/ceiling and outdoor sky/ground local-palette gates.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target execution receipt

The raw loader trace now records a target row only when Mednafen actually
fetches the exact target computed by the source-bound copied-entry `BRA`.
Firestaff retains the target opcode solely as opaque control-flow evidence and
requires the source PC, source physical PC, target and executed main-RAM PC to
agree. This does not assert loader, CD-record, dungeon, object, palette,
bitmap, or rendering semantics.

# ✅ 2026-07-15 Theron Track 02 copied-entry BRA target JSR receipt

The Mednafen trace now binds the first observed `JSR` after an executed
copied-entry BRA target to that exact target's main-RAM control path. Admission
requires the preceding target receipt and ordered trace rows. The JSR target
is retained as opaque control evidence only, without any assertion about a CD
record, loader routine, dungeon data, objects, palette, bitmap, or rendering.

# ✅ 2026-07-15 Theron Track 02 post-BRA JSR CD-record receipt

Firestaff can now admit a strict control-to-media join: an executed post-BRA
JSR must write the CD data register, then a canonical READ(6) and FIFO-origin
row must select a byte matching the hash-verified Track 02 sector at the
observed LBA. The resulting record coordinate remains opaque provenance, not
a loader name, level, object table, palette, bitmap, or rendering claim.

# ✅ 2026-07-15 DM2 source floor/ceiling c_light palette receipt

M11 now consumes SKProject `DM2_DISPLAY_VIEWPORT`'s authenticated
`glbLightLevel * 10` parameter through the stationary
`QUERY_TEMP_PICST/_32cb_0804` floor and ceiling palette path. The source
`_4976_4226[0/1]` controls are both zero, so the original fixed-point formula
retains that parameter unchanged. Each real IMG3 local palette is remapped by
the original interface action table, hashed, and bound to the exact c_light
receipt before the viewport accepts either plane. A present `GRAPHICSSET`
`dt07/0` or `dt07/1` originally remained blocked pending exact
`TRANSLATE_PALETTE` decoding; that narrow limitation is superseded by the
following source-lookup implementation. No substitute table, brightness, or
pixels are admitted. Verification: `dm2_v1_c_light_receipt`,
`dm2_v1_boot_profile_smoke`, and `dm2_v1_save_load` pass.

# ✅ 2026-07-15 DM2 floor/ceiling dt07 TRANSLATE_PALETTE

The M11 floor/ceiling c_light path now implements Skproject
`TRANSLATE_PALETTE` exactly: a present `GRAPHICSSET/dt07/0` or `/1` is a
direct 256-entry byte lookup applied to the local IMG3 palette before the
normal `_0b36_037e` light remap. The consumed lookup window, transformed
palette and c_light receipt are independently hash-bound. Missing dt07 retains
the source's non-fog branch; a partial lookup blocks the material route rather
than borrowing colour data. Verification: `dm2_v1_c_light_receipt`,
`dm2_v1_boot_profile_smoke`, `test_dm2_v1_gdat_graphicsset_real_data`, and
`dm2_v1_save_load` pass.

# ✅ 2026-07-15 DM2 moving floor/ceiling palette lookup selection

M11 now follows SKProject `_32cb_0804`'s moving branch for the real
floor/ceiling output route. While stationary it selects `GRAPHICSSET/dt07/0`
and `/1`; while moving, it selects `/9` and `/10` after the source's
`cls4 += 9` transition. The selected source field joins the palette lookup,
c_light and draw-order hashes before the viewport blits the decoded IMG3
planes. Verification: `dm2_v1_c_light_receipt`,
`dm2_v1_boot_profile_smoke`, `test_dm2_v1_gdat_graphicsset_real_data`, and
`dm2_v1_save_load` pass.

# ✅ 2026-07-15 Theron Track 02 CUE startup contract

The Track 02 launch resolver now follows the same CUE shape that the Theron
media classifier exposes to startup/menu code: `FILE`, `TRACK`, `MODE1`, and
`INDEX` keywords are accepted case-insensitively, and a CUE must contain
exactly one Track 02 `INDEX 01` before its BIN/ISO payload can be mounted.
This keeps real `MODE1/2048` ISO CUE media launchable while rejecting partial
or ambiguous CUE metadata. No dungeon, object, bitmap, palette, or fallback
semantics are inferred. Verification: `test_theron_v1_track02_cue_layout`,
`test_firestaff_theron_media_classify`, and
`test_m12_theron_missing_track02_popup_gate` pass.

# ✅ 2026-07-16 Theron Track 02 raw-only initial-envelope intake

The `$0b52` initial-envelope loader intake now carries the authenticated
Track 02 media variant and admits the complete-payload handoff only for the
JP/US raw BIN variants. ISO byte lookup remains an inspection boundary, but a
`MODE1/2048` ISO cannot reuse a raw-BIN loader/object-table route or become a
synthetic dungeon substitute. Verification: `theron_v1_track02_loader_intake`
and `theron_v1_raw_loader_trace_initial_level_handoff` pass.

# ✅ 2026-07-16 Theron Track 02 loader semantic gate

The real `$0b52` loader handoff now carries a hash-covered semantic-gate
receipt beside the full payload, initial envelope, and post-envelope bytes.
It exposes real byte availability while keeping dungeon-record,
object-table, bitmap, palette/RGBA, and fallback-visual promotion explicitly
blocked until an original consumer proves them. Verification:
`ctest --test-dir build-local-ninja -R
'theron_v1_track02_loader_intake|theron_v1_raw_loader_trace_initial_level_handoff'
--output-on-failure` passes.

# ✅ 2026-07-16 Theron post-$3800 consumer semantic gate

The Track 02 loader intake now exposes a separate post-`$3800`
consumer-trace gate. It promotes dungeon-record, object-table, bitmap,
palette, and source RGBA availability only when the original same-capture
consumer trace matches the already rehashed loader payload, level-envelope,
and post-envelope checksums. Synthetic dungeon/object/bitmap/palette
promotion and fallback visuals remain hard blockers. Verification: strict
compile of `theron_v1_track02_loader_intake.c` and focused
`theron_v1_track02_loader_intake` coverage for positive source admission,
stale checksum, missing consumer, synthetic, fallback, and pre-promoted-gate
rejections.

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

# ✅ 2026-07-16 Theron bounded Track 02 route after session handoff

The Theron runtime-admission surface now has a post-session-handoff bounded
Track 02 route receipt. It consumes the admitted US raw Track 02 FIFO
session handoff plus a route receipt carrying corpus evidence, then preserves
the capture mask, no-fallback semantic role mask, startup-level anchor,
blocked object-table anchors, blocked non-startup-level anchors, and route
hashes. It remains runtime-capture-required and refuses exact object/level
semantic promotion, object-table admission, level admission, payload
semantics, visual semantics, and fallback visuals. Verification:
`cmake --build build-local-ninja --target
firestaff_theron_v1_runtime_admission_probe`, `ctest --test-dir
build-local-ninja -R '^theron_v1_runtime_admission$' --output-on-failure`,
and focused `git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject picture descriptor receipts

DM2 now maps `DM2_QUERY_PICST_IMAGE` and `DM2_QUERY_GDAT_SUMMARY_IMAGE`
outside the shared skproject core files. The receipts bind picture descriptors
to real GDAT image-entry metadata, source dimensions, offsets, and local
palettes where present. The source `cls1 == 0xff` summary-image bypass is
preserved as no-GDAT/no-draw state. No synthetic HUD/dungeon visual is created.
Verification: `cmake --build build-local-ninja --target
test_dm2_v1_gdat_querydb_receipts --parallel 4`, `ctest --test-dir
build-local-ninja --output-on-failure -R '^dm2_v1_gdat_querydb_receipts$'`,
and focused `git diff --check` passed.

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

# ✅ 2026-07-16 Theron Track 02 decoded-route render proof producer

Theron runtime admission now constructs `Theron_V1RuntimeTrack02RenderAssetProof`
from decoded Track 02 route receipts instead of probe-filled proof fields. The
producer accepts only the same admitted US Track 02 consumer session with
matching level/object/all-dungeon route hashes, decode-ready non-startup level
and object-table receipts, a complete startup bitmap atlas, promotable palette
window evidence, nonzero decoded hashes, and no synthetic/fallback visual
flags. This is a fail-closed producer contract; real ISO/BIN/CUE capture still
has to provide the decoded receipts for broader non-startup dungeons.
Verification: `firestaff_theron_v1_runtime_admission_probe`,
`ctest -R '^theron_v1_runtime_admission$'`, and focused `git diff --check`
passed.

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

# ✅ 2026-07-16 DM2 skproject palette core symbol bundle

DM2 now maps the `SKULLWIN/c_gfx_pal.cpp` palette-core block:
`color_to_palettecolor`, `ui8_to_palettecolor`, `palettecolor_to_ui8`,
`palettecolor_to_pixel`, `DM2_CONVERT_DRIVERPALETTE`,
`DM2_SELECT_PALETTE_SET`, `DM2_UPDATE_BLIT_PALETTE`, and `DM2_xlat_palette`.
The implementation follows skproject byte-wrapper semantics, ARGB-to-DMPAL
RGB6 conversion, palette-set mode receipts, active blit palette pointer
assignment, and conversion-table palette translation. Live host palette upload
and fade blitting remain renderer work, so the driver/fade rows are deliberately
marked narrow. Verification: `cmake --build build-local-ninja --target
test_dm2_v1_skproject_core -j2` and `ctest --test-dir build-local-ninja
--output-on-failure -R '^dm2_v1_skproject_core$'` passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon-only consumer grammar gate

Added a narrow post-$3800 object/dungeon consumer grammar gate to
`theron_v1_track02_loader_intake`. It consumes the same real loader payload
boundary as the existing semantic gate, but admits only object-table and
dungeon-record grammar provenance when the same-capture original trace proves
both consumers and the payload/envelope/post-envelope checksums match. Bitmap,
palette, RGBA, runtime handoff, fallback visuals, and synthetic promotions are
explicitly rejected on this route. Also repaired the Theron raw-loader final
bind against the current startup-media receipt by reading the Soul Room raw
route spans directly from the receipt fields instead of the removed helper
type. Verification: direct focused C11 build/run of
`test_theron_v1_track02_loader_intake` passed, strict syntax-only checks for
the touched header/source/test and raw-loader source passed, and targeted
`git diff --check` passed.

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

# ✅ 2026-07-16 DM2 skproject querydb image-entry receipts

DM2 now maps `DM2_QUERY_GDAT_IMAGE_ENTRY_BUFF` and
`DM2_QUERY_GDAT_IMAGE_METRICS` outside the shared skproject core files. The new
asset-loader receipts select only real parsed GDAT `dtImage` rows, use
skproject's real `MISCELLANEOUS/FE/FE` default-image route when the requested
image is absent, and fail closed if that source data is missing or malformed.
No synthetic HUD/dungeon visual is generated or admitted. Verification:
`cmake --build build-local-ninja --target test_dm2_v1_gdat_querydb_receipts
--parallel 4`, `ctest --test-dir build-local-ninja --output-on-failure -R
'^dm2_v1_gdat_querydb_receipts$'`, and focused `git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject querydb pict-bits/map-chip receipts

DM2 now maps `DM2_QUERY_PICT_BITS` and
`DM2_QUERY_4BPP_PICT_BUFF_AND_PAL` outside the shared skproject core files.
The new receipts keep skproject's route split: image-descriptor mode bit 2
requires real GDAT image-entry data, mode bit 3 requires caller-owned cached
bitmap evidence, current-bitmap mode requires an existing bitmap, and the
4bpp map-chip path requires a real loadable `dtImage` field `0xF9` plus its
local 16-colour palette. Missing/non-4bpp/default-image routes fail closed.
Verification: `cmake --build build-local-ninja --target
test_dm2_v1_gdat_querydb_receipts --parallel 4`, `ctest --test-dir
build-local-ninja --output-on-failure -R '^dm2_v1_gdat_querydb_receipts$'`,
and focused `git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject xrect codec

DM2 now maps `DM2_QUERY_RECT`, `DM2_COMPRESS_RECTS`, and `READ_WORD` in the
shared skproject core layer. The new bounded table codec follows
`SKULLWIN/c_xrect.cpp` and `SKWIN/SkWinCore.cpp`: raw4 groups become
source-shaped rnodes, common X/Y and byte/word rectangle fields are preserved,
zero/missing/truncated rectangles fail closed, and no synthetic rectangle data
is generated. Verification: `cmake --build build-local-ninja --target
test_dm2_v1_skproject_core -j4` and `ctest --test-dir build-local-ninja
--output-on-failure -R '^dm2_v1_skproject_core$'` passed.

# ✅ 2026-07-16 Theron Track02 object/dungeon consumer byte-window binding

The Track 02 post-`$3800` consumer gates now require concrete same-capture
object/dungeon evidence before accepting the existing consumer markers. The
trace facts must carry nonzero dungeon/object consumer PCs plus payload-window
offsets, byte counts, and checksums that match the already verified initial
level envelope and post-envelope object-candidate slice from the real `$0b52`
loader read. The narrow object/dungeon grammar receipt retains those PCs and
windows while keeping field decode, bitmap, palette, RGBA, runtime handoff,
synthetic promotion, and fallback visuals closed. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched Theron header/source/test passed, and targeted
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track02 consumer-to-CD-read coordinate binding

The post-`$3800` Track 02 consumer facts now bind object/dungeon evidence back
to the exact raw loader/CD-read handoff before either the narrow grammar gate
or the broader consumer semantic gate can open. The facts and receipts retain
the `$0b52` record's user-data offset `$114`, destination `$3800`, and 2048-byte
payload size alongside the existing payload, level-envelope, post-envelope,
consumer-PC, and byte-window checksums. Mutated loader destination, payload
size, record-local offset, object window, or dungeon window evidence all fail
closed, with bitmap/palette/RGBA/runtime/fallback visuals still blocked on the
object/dungeon-only route. Verification: focused C11
`test_theron_v1_track02_loader_intake` build/run passed, strict syntax-only
checks for the touched intake header/source/test passed, and targeted
`git diff --check` passed. At that point the wider
`theron_v1_runtime_admission.c` syntax check still remained blocked by the
missing `Theron_Track02NonstartupContainerIndex` API closed below.

# ✅ 2026-07-16 Theron Track02 nonstartup container-index blocker closure

The missing `Theron_Track02NonstartupContainerIndex` API is now defined and
implemented as an opaque, fail-closed real-data bridge. It is built from the
existing hash-gated nonstartup sector receipt and indexes only verified,
contiguous user-data windows from real raw Track 02 data whose receipt already
marks them opaque and promotion-blocked. The index records descriptor entry,
raw offset, user-data offset, byte count, and hash evidence for later
object/dungeon consumer binding, but it does not decode object tables, levels,
bitmaps, palettes, text, runtime state, or visuals. Runtime-admission syntax
and object compilation now pass again without admitting fallback visuals.
Verification: strict syntax-only checks for `theron_v1_track02.h`,
`theron_v1_runtime_admission.h`, and `theron_v1_runtime_admission.c` passed;
`src/theron/theron_v1_runtime_admission.c` object build passed; focused C11
`test_theron_v1_track02_loader_intake` build/run passed; and targeted
`git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject record-name helper

DM2 now maps skproject `getRecordNameOf` as a bounded ObjectID record-family
receipt. The helper extracts only the source record-type nibble, returns the
corresponding DM2 record-family label, and records `SKWIN/SkWinCore.cpp:824`
provenance. It does not traverse records, query GDAT, draw anything, or
provide fallback content. Verification: focused C11 build/run passed with
`cc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_record_name_helper.c src/dm2/dm2_v1_record_name_helper.c
-o /tmp/test_dm2_v1_record_name_helper &&
/tmp/test_dm2_v1_record_name_helper`.

# ✅ 2026-07-16 DM2 skproject UI-event name helper

DM2 now maps skproject `getUIEventName` as a bounded HUD/UI event-name receipt.
The helper admits only event codes already source-locked by `ADJUST_UI_EVENT`
and `HANDLE_UI_EVENT`, including spell/leader/hand adjustment events and the
title-menu new/resume codes. Unknown codes fail closed without fallback labels.
Verification: focused C11 build/run passed with `cc -std=c99 -Wall -Wextra
-Werror -O2 -Iinclude tests/test_dm2_v1_ui_event_name_helper.c
src/dm2/dm2_v1_ui_event_name_helper.c -o
/tmp/test_dm2_v1_ui_event_name_helper &&
/tmp/test_dm2_v1_ui_event_name_helper`.

# ✅ 2026-07-16 DM2 skproject source-name helper batch

DM2 now maps skproject `getSpellTypeName`, `getSkillName`, and
`getStatBonusName` as bounded source-name receipts. The helper admits only
existing DM2 constants for spell type, base skill/class, and champion stat
bonus names, and blocks unknown values without fallback labels. `getXActrName`
remains open until an exact actuator-name table is available. Verification:
focused C11 build/run passed with `cc -std=c99 -Wall -Wextra -Werror -O2
-Iinclude tests/test_dm2_v1_source_name_helpers.c
src/dm2/dm2_v1_source_name_helpers.c -o
/tmp/test_dm2_v1_source_name_helpers &&
/tmp/test_dm2_v1_source_name_helpers`.

# ✅ 2026-07-16 DM2 skproject extended spell-definition loader

DM2 now maps skproject `EXTENDED_LOAD_SPELLS_DEFINITION` as a bounded
GDAT `SPELL_DEF` load receipt. It consumes exact dtWordValue fields 1-7 and
optional dtText field `0x18`, preserves sparse custom spell indexes, adapts
only the original spell-value rows covered by the source count, and fails
closed for missing required fields or U8-wide source values. No synthetic
spell records, runtime effects, projectiles, timers, or UI spell actions are
introduced. Verification: focused C99 build/run passed with `cc -std=c99
-Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_extended_spells_definition.c
tests/test_dm2_v1_extended_spells_definition_asset_stub.c
src/dm2/dm2_v1_extended_spells_definition.c -o
/tmp/test_dm2_v1_extended_spells_definition &&
/tmp/test_dm2_v1_extended_spells_definition`.

# ✅ 2026-07-16 DM2 skproject DBSPEC word-value alias

DM2 now closes skproject `DM2_QUERY_GDAT_DBSPEC_WORD_VALUE` from
`SKULLWIN/c_record.cpp:352` through the existing bounded
`QUERY_GDAT_DBSPEC_WORD_VALUE` helper. The helper keeps the source
`OBJECT_NULL` zero path and rejects absent caller-provided GDAT word rows
without fabricating scalar data; the existing SKWIN symbol disposition covers
the same-name SKWIN rows. Verification: focused C99 build/run passed with
`cc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_predicate_helpers.c src/dm2/dm2_v1_predicate_helpers.c
-o /tmp/test_dm2_v1_predicate_helpers &&
/tmp/test_dm2_v1_predicate_helpers`.

# ✅ 2026-07-16 DM2 skproject original graphics-data open alias

DM2 now closes skproject `ORIGINAL__GRAPHICS_DATA_OPEN` from
`SKWIN/SkWinCore.cpp:3179` through the existing bounded GRAPHICS.DAT file-open
receipt. The focused receipt locks first-open primary/secondary handle
admission, nested open-counter increments, and source sys-error codes
`0x29`/`0x1f` without opening host files or fabricating graphics data.
Verification: focused C99 build/run passed with `cc -std=c99 -Wall -Wextra
-Werror -O2 -Iinclude tests/test_dm2_v1_graphics_data_file_open.c
src/dm2/dm2_v1_asset_loader.c -o /tmp/test_dm2_v1_graphics_data_file_open &&
/tmp/test_dm2_v1_graphics_data_file_open`.

# ✅ 2026-07-16 DM2 skproject GDAT IMG3/U4 querydb repair

DM2 now accepts the real IMG3/U4 header variant where signed Y offset `-32`
stores the 4bpp marker in the next header word. `QUERY_GDAT_IMAGE_ENTRY_BUFF`,
`QUERY_GDAT_IMAGE_METRICS`, `QUERY_PICT_BITS`, `QUERY_PICST_IMAGE`, and
`DM2_EXTRACT_GDAT_IMAGE` now share that source-shaped bpp decoding path, so the
querydb receipt test no longer rejects real U4 image payloads or falls back to
synthetic pixels. Verification: focused C99 build/run passed for
`tests/test_dm2_v1_img3_u4_header.c`, and the broader
`tests/test_dm2_v1_gdat_querydb_receipts.c` build/run now reports
`105 passed, 0 failed`.

# ✅ 2026-07-16 DM2 skproject direct GDAT query wrappers

DM2 now closes skproject `DIRECT_QUERY_GDAT_ENTRY_DATA_BUFF` and
`DIRECT_QUERY_GDAT_TEXT` from `SKWIN/SkWinCore2.cpp`. The wrappers expose only
exact parsed GDAT raw payload bytes: the generic data-buffer route accepts
typed loadable entries and rejects scalar rows, while the text route is locked
to `dtText` and does not format or fabricate text. Verification: focused C99
build/run passed with `cc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_direct_gdat_query.c src/dm2/dm2_v1_asset_loader.c -o
/tmp/test_dm2_v1_direct_gdat_query && /tmp/test_dm2_v1_direct_gdat_query`;
the broader GDAT querydb receipt test still reports `105 passed, 0 failed`.

# ✅ 2026-07-16 DM2 skproject SkWinCore2 GDAT word helpers

DM2 now closes `QUERY_GDAT_POTION_SPELL_TYPE_FROM_RECORD`,
`QUERY_GDAT_POTION_BEHAVIOUR_FROM_RECORD`,
`QUERY_GDAT_WATER_VALUE_FROM_RECORD`, and `QUERY_GDAT_DOOR_IS_MIRRORED`.
The first three follow the source `QUERY_GDAT_DBSPEC_WORD_VALUE` aliases for
fields `0x4d`, `0x05`, and `0x43`; the door helper reads DOORS
`dtWordValue` field `0x20`, preserving explicit zero mirror flags. Missing
rows fail closed and no potion, water, door, text, or graphic data is
fabricated. Verification: focused C99 build/run passed with `cc -std=c99
-Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_skcore2_gdat_word_helpers.c
src/dm2/dm2_v1_asset_loader.c -o
/tmp/test_dm2_v1_skcore2_gdat_word_helpers &&
/tmp/test_dm2_v1_skcore2_gdat_word_helpers`; the broader GDAT querydb receipt
test still reports `105 passed, 0 failed`.

# ✅ 2026-07-16 Theron Track 02 multi-level runtime handoff gate

Theron Track 02 now has a level-transition/runtime-handoff gate above the
object gameplay state. The new handoff requires same-capture trace proof for
source and target level selectors, target level byte count/hash, target object
runtime-state hash, party-placement binding, and object-pool state binding.
`theron_v1_runtime_publish_track02_level_transition()` then installs the target
level, publishes that level's verified object pool, places the party at the
target level start pose, clears the pending stairs transition, and invalidates
runtime media. This path deliberately stays separate from the older
bitmap-complete dungeon route so real level/object state can advance without
promoting unproven palette/pixels. Dungeon runtime admission, dungeon draw,
synthetic dungeon/object data, and fallback visuals remain denied. Verification:
Ninja built `firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object gameplay-state handoff gate

Theron Track 02 now has a second gate after object placement: object gameplay
semantics. It accepts compact object-table rows only when the same-capture trace
proves the supported runtime kind set, flags low bits as object state, argument
as quantity, preserved flags, and a runtime-state hash. A separate world handoff
then mutates only the selected loaded level's object pool, removes stale objects
for that level, preserves objects from other levels, updates thing count/current
level, and invalidates runtime media. It still denies dungeon runtime admission,
dungeon draw, bitmap/palette/RGBA promotion, synthetic objects, and fallback
visuals. The runtime-admission probe wires this into the optional real
object/dungeon HuC6280 trace path; plain real CUE/BIN remains fail-closed source
proof without such a trace. Verification: Ninja built
`firestaff_theron_v1_runtime_admission_probe` and
`test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
direct default and local US-CUE runtime-admission probes passed; syntax checks
and `git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 object placement-state gate

Theron Track 02 now has a fail-closed object-placement state receipt after the
level/object loader-route proof. It consumes the verified compact object table
and same-capture route trace, binds selected dungeon/level rows, table checksum,
level mask, row hashes, first-row x/y/level/flags/argument bytes, and a placement
state hash. It deliberately keeps object-kind gameplay semantics under review and
does not allow world object publish, runtime admission, dungeon draw, bitmap/
palette/RGBA promotion, synthetic decode, or fallback visuals. The runtime
admission probe's optional object/dungeon HuC6280 trace branch now carries the
full chain to placement state and parses the object table from the real Track 02
container window. Verification: Ninja built `firestaff_theron_v1_runtime_admission_probe`
and `test_theron_v1_track02_loader_intake`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
the direct runtime-admission probe passed both default and local US-CUE real-media
runs; syntax checks and `git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject FIND_LADDER_AROUND dungeon receipt

DM2 now closes `FIND_LADDER_AROUND` from `SKWIN/SkWinCore.cpp:9060`.
The existing dungeon receipt builds again after a narrow G1/dungeon-loader
header repair: standalone consumers now see the G1 receipt types, PC G1
extension record fields, raw map-corpus receipts, and the conservative
record-list traversal gate. The helper scans only loaded `DUNGEON.DAT`
square facts for source-ordered stairs-up/stairs-down candidates and keeps
missing candidates as explicit not-found receipts. Verification: focused
C99 build/run passed with `cc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
tests/test_dm2_v1_find_ladder_around.c src/dm2/dm2_v1_find_ladder_around.c
src/dm2/dm2_v1_dungeon_loader.c -o /tmp/test_dm2_v1_find_ladder_around &&
/tmp/test_dm2_v1_find_ladder_around`; the G1 text runtime header smoke also
passed with `tests/test_dm2_v1_g1_text_message_runtime.c`.

# ✅ 2026-07-16 DM2 startup menu/HUD real GDAT receipt

DM2 boot now exposes a source-owned startup menu/HUD GDAT receipt that joins
the decoded `TITLE` title/menu surfaces, the original `0xD7`/`0xD9`
startup click rectangles, the interface palette, and the 9-command static
M11 HUD chrome plan from the same verified `GRAPHICS.DAT`. The real-data HUD
test now verifies that receipt and renders the static HUD plan directly from
canonical GDAT material with provider callbacks disabled; altered HUD palette
material remains fail-closed without fallback drawing. The stale synthetic
champion-name/HeroType expectation was removed from this HUD test so champion
portrait admission stays on its separate save-bound source gate. Verification:
`test_dm2_v1_gdat_hud_m11_command_real_data`,
`test_dm2_v1_startup_menu_action_contract` 100/100,
`test_dm2_v1_save_load` 25/25, `cc -std=c11 -Wall -Wextra -Werror -Iinclude
-fsyntax-only src/dm2/dm2_v1_boot.c`, and `git diff --check`.

# ✅ 2026-07-16 DM2 skproject SUPPRESS save-symbol receipt

DM2 now closes skproject `DM2_SUPPRESS_INIT`, `DM2_SUPPRESS_WRITER`,
`DM2_SUPPRESS_FLUSH`, `DM2_SUPPRESS_READER`, `DM2_WRITE_1BIT`,
`DM2_READ_1BIT`, and the matching SKWIN aliases through a source-named
SUPPRESS receipt in `dm2_v1_save_load`. The receipt proves the actual
stateful bitstream contract used by real SKSave sections: zeroed init state,
MSB-first source-bit writes, pending-bit carry across adjacent sections,
flush padding/clear, reader carry, fill=0/fill=1 behavior, and fail-closed
underflow rejection. No synthetic save payload, menu, HUD, or viewport data is
introduced. Verification: Ninja build/run of `test_dm2_v1_save_load` passed
25/25, CTest `dm2_v1_save_load` passed 1/1, and
`python3 tools/symbol_backlog.py --game DM2 --limit 20` now reports DM2 1179.

# ✅ 2026-07-16 DM2 skproject `_1031` UI predicate dispatch batch

DM2 now closes the contiguous SKWIN `SkWinCore.cpp` UI predicate slots
`RETURN_1`, `IS_GAME_ENDED`, `_1031_0023`, `_1031_003e`, `_1031_007b`,
`_1031_009e`, `_1031_00c5`, `_1031_00f3`, and `_1031_012d`. The new
skproject-core state keeps the original `sk1891` node shape and maps
`_4976_0cba[0..8]` to caller-owned runtime facts: game-ended flag,
selected panel token, champion inventory, champion HP, rotated
`GET_PLAYER_AT_POSITION`, `_4976_5dbc`, active champion index, and
selected spell panel. No menu art, viewport rendering, or ANIM fallback was
introduced; `_0759_0855/_0869/_08e7` remain open because they are not direct
prerequisites of this UI predicate flow. Verification: syntax check passed
with `cc -std=c11 -Wall -Wextra -Iinclude -fsyntax-only
src/dm2/dm2_v1_skproject_core.c tests/test_dm2_v1_skproject_core.c`;
Ninja built `test_dm2_v1_skproject_core`; the direct test binary and CTest
`^dm2_v1_skproject_core$` passed; `python3 tools/symbol_backlog.py --game DM2
--limit 20` now reports DM2 1137.

# ✅ 2026-07-16 DM2 skproject `_1031` UI node traversal batch

DM2 now extends the same SKWIN `SkWinCore.cpp` UI runtime family through
`_1031_014f`, `_1031_0184`, `_1031_01ba`, `_1031_023b`, `_1031_027e`, and
`_1031_01d5`. The `_4976_0cba` dispatcher now covers slots 0..11, with the
new predicates bound to active champion rune count, magical-map state,
selected spell panel, and right-panel type. The node-flow helpers consume
source-shaped `sk1891` child cursors, `_4976_169c`-style child bytes,
`sk16ed`-style leaf metadata, and `QUERY_EXPANDED_RECT`/
`QUERY_TOPLEFT_OF_RECT` rectangle state. No menu graphics, viewport renderer,
or fallback UI data was introduced. Verification: syntax check passed with
`cc -std=c11 -Wall -Wextra -Iinclude -fsyntax-only
src/dm2/dm2_v1_skproject_core.c tests/test_dm2_v1_skproject_core.c`; Ninja
built `test_dm2_v1_skproject_core`; direct test binary and CTest
`^dm2_v1_skproject_core$` passed; `python3 tools/symbol_backlog.py --game DM2
--limit 20` now reports DM2 1131.

# ✅ 2026-07-16 DM2 skproject `_1031` UI action/tree runtime batch

DM2 now extends the same source-backed UI-node runtime through `_1031_024c`,
`_1031_030a`, `_1031_03f2`, `_1031_04f5`, `_1031_050c`, `_1031_0541`,
`_1031_0667`, `_1031_0675`, and `_1031_098e`. The production core now carries
source-shaped action lists, leaf metadata, clickrect refresh state, mouse queue
filtering, capture-release/reset receipts, recursive hit-testing, recursive
action-code search, and tree selection via the existing `_1031` dispatcher.
No menu graphics, viewport renderer, synthetic UI data, or host mouse fallback
was introduced. Verification: syntax check passed with `cc -std=c11 -Wall
-Wextra -Iinclude -fsyntax-only src/dm2/dm2_v1_skproject_core.c
tests/test_dm2_v1_skproject_core.c`; Ninja built `test_dm2_v1_skproject_core`;
direct test binary and CTest `^dm2_v1_skproject_core$` passed;
`python3 tools/symbol_backlog.py --game DM2 --limit 20` now reports DM2 1122.

# ✅ 2026-07-16 DM2 top movement/map receipt batch

DM2 now closes the current top open skproject movement/map rows:
`DM2_ARRANGE_DUNGEON`, `DM2_PERFORM_MOVE`, and `DM2_move_075f_1bc2`.
`ARRANGE_DUNGEON` is mapped to the real `dm2_v1_dungeon_load` map layout
receipt, preserving PC G1 partial record-graph state instead of fabricating
c_record semantics. `PERFORM_MOVE` now uses source square-type constants for
wall/secret-door/fake-wall, pit, lava, inaccessible, door, cooldown, and
outdoor-bypass admission. `move_075f_1bc2` real DUNGEON.DAT target receipts
now feed PERFORM_MOVE accepted and indoor-blocked cases without fallback or
viewport rendering. Verification: Ninja build/run of
`test_dm2_v1_arrange_dungeon_receipt`, `test_dm2_v1_perform_move_receipt`,
and `test_dm2_v1_move_075f_1bc2` passed; CTest regex
`^dm2_v1_(arrange_dungeon_receipt|perform_move_receipt|move_075f_1bc2)$`
passed 3/3; syntax checks for `dm2_v1_perform_move.c` and
`dm2_v1_move_075f_1bc2.c` passed; backlog now reports DM2 1176.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette source-window gate

Theron Track 02 now has a fail-closed bitmap/palette source receipt above the
proved multilevel runtime route. The receipt consumes only a verified
level-transition runtime result, binds the same Track 02 record and
source/target levels to palette raw/user-data offsets, palette checksums,
bitmap atlas route facts, and a combined source hash, and rejects hash drift,
pixel-output claims, M11 render admission, dungeon draw, and fallback visuals.
No bitmap decoder, palette decoder, pixel output, synthetic visual, or M11
render promotion was added. The acute integration break from the new helper
name was fixed by using the existing `theron_v1_runtime_mix_hash` helper, and
`ninja -C build/ninja-dm2 firestaff` now completes. Verification:
`ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs.

# ✅ 2026-07-16 Theron Track 02 bitmap/palette decode-vector gate

Theron Track 02 now has a positive decode-vector receipt after the
bitmap/palette source-window gate. The receipt consumes the source-bound
record/level route plus the real US Track 02 bytes, re-decodes the HuC6260
4bpp palette window, builds the indexed startup bitmap atlas from the same
media, and admits only exact checksum/route/tile/nonzero-pixel agreement. It
retains the first palette word/RGB triplet, atlas route geometry, first source
bitmap offsets, and first decoded pixel-row hash as proof vectors. The result
sets palette decode, bitmap decode, and pixel output verified, but keeps M11
runtime consumption, M11 rendering, dungeon draw, and fallback visuals closed.
No guessed decoder, fallback image, host upload, or dungeon render promotion
was added. Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# ✅ 2026-07-16 Theron Track 02 M11 Soul Room runtime consumption

Theron now binds the positive Track 02 bitmap/palette decode vector to a
production M11 runtime-consumption receipt for the verified Soul Room level-0
surface. `theron_v1_world_runtime_media_for_level()` now returns the retained
Soul Room surface for level 0, so the existing live `Theron_RuntimeLevelMedia`
path can select it through `THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL`. The new
M11 consumption receipt requires the real world runtime-media surface to match
the decode vector's Soul Room route bit, offsets, geometry, route checksum,
tile count, and nonzero-pixel count, then verifies exact 1:1 placement and
clip bounds before allowing host presentation. Checksum drift, bad host bounds,
scale changes, missing world media, non-Soul Room routes, dungeon draw, and
fallback visuals all remain fail-closed. The real US-CUE probe now builds the
production startup media receipt from the real Track 02 bytes, binds it into a
live world, and proves the M11 Soul Room consumption receipt from that world.
Verification: `ninja -C build/ninja-dm2 firestaff`;
`ninja -C build/ninja-dm2 test_theron_v1_track02_loader_intake
firestaff_theron_v1_runtime_admission_probe`; CTest
`^(theron_v1_runtime_admission|theron_v1_track02_loader_intake)$` passed 2/2;
syntax checks for the touched Theron source/test/probe passed; the direct
runtime-admission probe passed both default and local US-CUE real-media runs;
`git diff --check` passed.

# ✅ 2026-07-16 DM2 skproject `_1031` UI action latch/button-centering batch

DM2 now closes the remaining source-backed `_1031` action-latch helpers:
`_1031_0a88`, `_1031_0b7e`, `_1031_0c58`, and `_1031_10c8`, plus the direct
SKULLWIN equivalents for `_0a88`, `_0c58`, and `_10c8`. The production core
now exposes source-shaped receipts for action-list hit selection, event-code
selection, pending mouse-event queue flush, and NODATA button-group
mouse-rect copy plus centered child-rect calculation. The flow uses existing
`_1031_01d5` real rect resolution and caller-owned UI queue/state; it does not
introduce menu art, viewport rendering, synthetic UI events, or host fallback.
Verification: `cc -std=c11 -Wall -Wextra -Iinclude -fsyntax-only
src/dm2/dm2_v1_skproject_core.c tests/test_dm2_v1_skproject_core.c`;
`cmake --build /tmp/firestaff-dm2-startup-build --target
test_dm2_v1_skproject_core --parallel 4`;
`/tmp/firestaff-dm2-startup-build/test_dm2_v1_skproject_core`;
`ctest --test-dir /tmp/firestaff-dm2-startup-build -R
'^dm2_v1_skproject_core$' --output-on-failure`;
`ninja -C build/ninja-dm2 firestaff`; `git diff --check`.
`python3 tools/symbol_backlog.py --game DM2 --limit 20` now reports DM2 1115.

# ✅ 2026-07-16 DM2 skproject `_2405` item/rect runtime batch

DM2 now closes the SkWinCore `_2405_00ec`, `_2405_011f`, and `_2405_014a`
family plus the direct SKULLWIN `_2405_011f` wrapper. The production core now
has caller-owned `_2405` runtime receipts for blit-rect query offsets,
source-margin inflation, and DBSPEC word-6 item icon entry selection through
equip/selected-hand gates, game tick plus DBIndex, player direction, item
charge buckets, and tick-modulo variants. The path consumes real item state,
GDAT DBSPEC facts supplied by the caller, and existing ADD_ITEM_CHARGE/
GET_MAX_CHARGE helpers; it does not draw synthetic art, touch viewport renderer
files, or fabricate item/asset state. Verification: strict C11 syntax for
`src/dm2/dm2_v1_skproject_core.c` and `tests/test_dm2_v1_skproject_core.c`;
Ninja build/direct run of `test_dm2_v1_skproject_core`; focused CTest
`^dm2_v1_skproject_core$`; full `ninja -C build/ninja-dm2 firestaff`; and
`git diff --check` passed. `python3 tools/symbol_backlog.py --game DM2
--limit 20` now reports DM2 1111.

# ✅ 2026-07-16 DM2 skproject `_0b36` cached-picture/button-group dirty-rect batch

DM2 now closes the contiguous SkWinCore `_0b36_00c3`, `_0b36_0c52`,
`_0b36_0d67`, and `_0b36_11c0` family plus the direct SKULLWIN
`DM2_guidraw_0b36_0c52` and `DM2_image_0b36_11c0` wrappers. The production
core now has source-shaped receipts for binding cache-index mement payload
headers into picture metadata, allocating an 8bpp button-group backing pict
from caller-owned expanded rects and cache indexes, tracking dirty-rect reuse,
replacement, compaction and clipping, and drawing cached picture bits through
blit rect, offset rect, DRAW_DEF_PICT, and dirty-rect handoff. Missing payloads
or missing caller-owned state fail closed; no synthetic picture/menu/viewport
data was introduced. `_0b36_0cbe` and `_0b36_129a` remain open because they
are separate source routes.

Verification: strict C11 syntax for `src/dm2/dm2_v1_skproject_core.c` and
`tests/test_dm2_v1_skproject_core.c`; direct focused run
`cc -std=c11 -Wall -Wextra -Iinclude tests/test_dm2_v1_skproject_core.c
src/dm2/dm2_v1_skproject_core.c src/dm2/dm2_v1_asset_loader.c -o
/tmp/test_dm2_v1_skproject_core && /tmp/test_dm2_v1_skproject_core`; and
`python3 tools/symbol_backlog.py --game DM2 --limit 20` now reports DM2 1105.
The CMake test target currently stops before DM2 on an unrelated shared
worktree CSB compile error in `src/csb/csb_v1_runtime_pc34_compat.c` around
line 18289, so that file was left untouched.

# ✅ 2026-07-16 DM2 skproject `_0cee` wall-decoration runtime batch

DM2 now closes the SkWinCore `_0cee_17e7`, `_0cee_1815`, and `_0cee_185a`
family and upgrades the direct SKULLWIN `DM2_map_0cee_1815` /
`DM2_map_0cee_185a` receipts to verified source mappings. The production core
now exposes a single wall-decoration chain receipt that binds the source random
hash formula to the real dungeon seed, current map index, map width/height,
WallGraphicsRandomDecorations candidate limit, gate words, rotation/row
selector sequence, v1e02cc-style candidate table, and out-of-map ornate-alcove
sanitize step. It uses caller-owned map/GDAT state and does not touch viewport
renderer files or fabricate wall art.

Verification: strict C11 syntax for `src/dm2/dm2_v1_skproject_core.c` and
`tests/test_dm2_v1_skproject_core.c`; direct focused run
`cc -std=c11 -Wall -Wextra -Iinclude tests/test_dm2_v1_skproject_core.c
src/dm2/dm2_v1_skproject_core.c src/dm2/dm2_v1_asset_loader.c -o
/tmp/test_dm2_v1_skproject_core && /tmp/test_dm2_v1_skproject_core`; and
`python3 tools/symbol_backlog.py --game DM2 --limit 20` now reports DM2 1102.

# ✅ 2026-07-16 DM2 skproject `_0759` ANIM stream/main runtime batch

DM2 now closes SkWinCore `_0759_0855`, `_0759_0869`, and `_0759_08e7` as a
bounded ANIM runtime plan in `dm2_v1_anim_bootstrap`. `_0759_0855` resets the
caller-owned streamed-animation arena, `_0759_0869` resolves source offsets
inside that arena with inactive/out-of-bounds fail-closed receipts, and
`_0759_08e7` now has a source-shaped main setup plan for parsed `+A` flags,
IBMIO event drain, screen/sound allocation, heap-vs-stream selection from
farcoreleft/capacity, stream offset resolving, stream reset, palette-zero and
clear/release side effects. `_0cee_2df4` remains open because the source route
is creature AI-spec `w30`, not part of this ANIM stream path.

Verification: strict C11 syntax for `src/dm2/dm2_v1_anim_bootstrap.c` and
`tests/test_dm2_v1_anim_bootstrap.c`; direct focused run
`cc -std=c11 -Wall -Wextra -Iinclude tests/test_dm2_v1_anim_bootstrap.c
src/dm2/dm2_v1_anim_bootstrap.c -o /tmp/test_dm2_v1_anim_bootstrap &&
/tmp/test_dm2_v1_anim_bootstrap`; and `python3 tools/symbol_backlog.py --game
DM2 --limit 20` now reports DM2 1099.
- 2026-07-16 CSB startup runtime-consumption adapter: the source-named
  ReDMCSB `F0437_STARTEND_DrawTitle`, `F0438_STARTEND_OpenEntranceDoors`,
  `F0580_ENTRANCE_DrawDoorAnimationStep`, and `F0581_ENTRANCE_BlitDoors`
  now have a CSB-owned consumption adapter that derives facts exclusively from
  one generation-matched real PC3.4 startup session and complete-support
  receipt. The adapter verifies the complete title route and four distinct
  phase hashes, terminal C017/C040 HUD, the final 31-step entrance route,
  receipt-only draw/input, and the absence of fallback callbacks or wrapper
  routes before invoking the source gates.
  Duplicate phase hashes and generation mismatches fail closed. The production
  complete-support receipt also rejects duplicate or zero phase hashes.
  Verification: `firestaff_m10` build and CTest
  `csb_v1_startup_runtime_coupling_adapter_pc34_compat` PASS.

- 2026-07-16 CSBgraphics HUD session binding: C017 and C040 selected by
  CSBWin's `Graphics.cpp::LocateNthGraphic` plan now load from the
  hash-admitted `CSBgraphics.dat` cache instead of being silently rejected by
  the PC34 session loader. The binding requires the cache path and 32-character MD5
  receipt, an existing HUD redraw plan entry, source C017/C040 geometry and a
  complete LZW decode before either surface can enter the live session. The
  session accepts an override HUD only when both panels have that ownership;
  a partial override never falls back to a mixed source. Verification:
  `firestaff_m10` build PASS.

- 2026-07-16 CSB F0437/F0807 startup regression repair: `TITLE.C F0437` now
  treats the first C426 STRIKES BACK presentation as the completion boundary
  for its two-VBlank full-size CHAOS hold, so direct live-frame sampling keeps
  the complete PRESENTS/CHAOS zoom/CHAOS hold/STRIKES mask. The F0807
  entrance-to-HUD transition now consumes C004, C002/C003 and C017/C040 from
  the same verified startup session through its asset-owned readiness facts,
  rather than requiring an unrelated host presentation flag. No wrapper or
  fallback surface is admitted. Verification: CTest
  `csb_v1_boot_title_import_ui_gate_pc34_compat` PASS.

- 2026-07-16 Theron Track 02 host dungeon-consumer binding: host admission is
  closed to `theron-v1-original-host-route-track02-dungeon` and preserves an
  FNV identity checksum alongside the authenticated level, object, bitmap, and
  palette proof hashes. Alternate routes and checksum drift reject before a
  host surface can be admitted; no unknown dungeon/object format was decoded
  and fallback visuals remain prohibited. Verification: CTest
  `theron_v1_runtime_admission` and `theron_v1_track02_loader_intake` PASS.

- 2026-07-16 Theron Track 02 source-owned runtime media: level-bank selection
  now requires title, stage, Soul Room, and forcefield indexed surfaces to be
  raw-source-verified and to share one Track 02 MD5. Cross-disc replacement at
  surface admission and MD5 drift before selection fail closed. This carries
  real media provenance into runtime selection only; palette binding, level
  geometry, object layout, and dungeon drawing remain unpromoted. Verification:
  CTest `theron_v1_track02_loader_intake` and `theron_v1_runtime_admission`
  PASS.

- 2026-07-16 Theron Track 02 runtime-media variant binding: source-owned
  level-bank selection now accepts only four raw-source surfaces sharing a
  known Track 02 MD5 whose JP/US variant equals the live runtime-media identity.
  Unknown MD5 labels and variant drift reject before selection. No palette
  route, level geometry, object-table layout, or synthetic visual was promoted.
  Verification: CTest `theron_v1_track02_loader_intake` and
  `theron_v1_runtime_admission` PASS.

- 2026-07-16 Theron Track 02 palette runtime admission: an authenticated
  direct VCE index/low/high store receipt now carries its verified Track 02
  MD5 and JP/US variant into a matching raw-source runtime bitmap surface.
  Identity drift and render promotion reject. The admission records source
  provenance only: palette-window-to-bitmap semantics, level geometry, object
  layout, dungeon drawing, fallback visuals, and synthetic visuals remain
  blocked. Verification: CTest `theron_v1_palette_runtime_admission` and
  `theron_v1_track02_loader_intake` PASS.

- ✅ 2026-07-17 Nexus Saturn 8 KiB memory-card opaque intake: added a strict
  16 x 512-byte, FNV-provenance and title/champion-route admission boundary.
  Mutated, short, unproven, fallback-enabled, or route-inactive images reject;
  no proprietary save fields are decoded or promoted. Verification:
  `nexus_v1_saturn_save_capture` PASS; the explicit real-file probe is
  skip-safe without a local image.

- ✅ 2026-07-17 Nexus operator-only Mednafen PRS3 replay manifest: the
  external launcher now refuses identity drift across BIOS, disc, MENU.BPK,
  DM.BIN, LEV00.DGN and replay FNV/epoch fields before launch, and writes one
  immutable dry-run manifest without copying assets or generating a trace.
  Verification: `nexus_v1_mednafen_capture_launcher` PASS.

- ✅ 2026-07-17 Nexus Structure1F LEV00--15 capture-target corpus planner:
  a strict fixture-matrix verifier now binds each level's package, DGN,
  descriptor, mesh, and face candidate identities before emitting only
  original-Saturn, no-draw trace targets. Cross-level, duplicate, missing, and
  mutated identities reject. The external SHA-256 corpus probe is skip-safe
  when local retail LEV files are absent. Verification:
  `nexus_v1_structure1f_corpus_capture_plan` PASS.

- ✅ 2026-07-17 Nexus SLEV00--15 task-body capture-target planner: every
  row now cross-binds the existing source/header/literal receipt, raw-trace
  identity, and ordered entry/task/callback observation to an opaque target
  location. Unknown opaque opcode labels, unknown callback ownership, order
  drift, and source or trace drift reject. No task dispatch or fallback script
  behavior is enabled. Verification: `nexus_v1_slev_task_body_capture_plan`
  PASS; the local SHA-256 corpus/trace probe is skip-safe.

- ✅ 2026-07-17 Nexus SNDLEV/SAL capture-target planner: unique bounded MAP
  selector routes now bind their SAL window, SDDRVS MD5, and ordered
  selector-dispatch/SAL-read/driver-output raw-trace observations. Duplicate
  or ambiguous routes, source/trace drift, and any decode or playback claim
  reject. Verification: `nexus_v1_sal_capture_plan` PASS; the external
  SHA-256 corpus probe is skip-safe.

- ✅ 2026-07-17 Nexus Mednafen Saturn campaign import: a canonical, complete
  external trace export now binds PRS3, Structure1F, SLEV, and SAL capture
  evidence to one raw-trace FNV/length and SHA-256-labelled debugger export.
  Partial, mixed, reordered, or semantically promoted evidence rejects.
  Verification: `nexus_v1_saturn_capture_campaign_import` PASS; the external
  probe is skip-safe without local retail traces.

- ✅ 2026-07-17 Nexus PRS3 original-execution evidence import: a strict V10
  receipt now requires explicit independent Saturn authentication, exact
  MENU.BPK/DM.BIN stream binding, complete SH-2 input/output witnesses, and a
  later VDP1 source command. Any partial range, fingerprint drift, ordering
  drift, or unauthenticated export rejects. Verification:
  `nexus_v1_prs3_original_execution_import` PASS; external input probe is
  skip-safe.

- ✅ 2026-07-17 Nexus PRS3 multi-capture adjudicator: authenticated complete
  execution receipts from at least two distinct MENU.BPK modes now require
  consistent opaque bit-order and termination observations, unique streams,
  and matching complete I/O-to-VDP1 contracts before decoder review can be
  considered. Contradiction rejects and decoder/render promotion remains zero.
  Verification: `nexus_v1_prs3_multi_capture_adjudicator` PASS; external
  multi-mode probe is skip-safe.

- ✅ 2026-07-17 Nexus Structure3 face/texturing capture planner: exact DGN
  face/mesh/descriptor provenance now joins Structure1F static material and
  PRS3/VDP1 candidate evidence into a capture-only request. Descriptor, trace,
  candidate, or face drift rejects; pixel/geometry inference and drawing stay
  disabled. Verification: `nexus_v1_structure3_face_texturing_capture_plan`
  PASS; external probe is skip-safe.

- ✅ 2026-07-17 Nexus multi-level DGN capture adjudicator: all LEV00--15 now
  require exact Structure1F/2/3 identity joins and unique opaque frame/command
  coverage. Cross-level, descriptor, DGN, and ordering drift reject while
  decoder, mesh semantics, and rendering remain disabled. Verification:
  `nexus_v1_dgn_multi_level_capture_adjudicator` PASS; external probe is
  skip-safe.

- ✅ 2026-07-17 Nexus active multi-level DGN capture ingress: engine-owned
  dungeon routing now retains only active-level opaque capture coverage after
  a full adjudication receipt matches the loaded DGN FNV. Level or source drift
  and promoted render claims clear it. Verification:
  `nexus_v1_dgn_campaign_engine_ingress` PASS.

- 2026-07-16 Theron Track 02 Soul Room capture-to-runtime admission: the
  source-owned Soul Room runtime surface now accepts only a complete startup
  media capture and authenticated CD-read trace that agree on MD5/variant,
  offsets, checksum, route mask, atlas checksum, and runtime identity. The
  admission proves byte provenance only; VCE relation, pixel decoding, level
  geometry, object layout, dungeon drawing, fallback visuals, and synthetic
  visuals remain blocked. Verification: CTest
  `theron_v1_bitmap_capture_runtime_admission`,
  `theron_v1_palette_runtime_admission`, and
  `theron_v1_track02_loader_intake` PASS.

- 2026-07-16 Theron Track 02 provenance runtime consumer: the capture-bound
  Soul Room receipt now joins the runtime-owned opaque initial loader record
  only when both retain one verified Track 02 identity. It carries the exact
  payload, level-envelope, and post-envelope receipts forward while marking an
  original level/object consumer trace as required. No level/object fields,
  pixel or palette semantics, drawing, fallback visual, or synthetic visual is
  admitted. Verification: CTest
  `theron_v1_track02_provenance_runtime_consumer`,
  `theron_v1_bitmap_capture_runtime_admission`,
  `theron_v1_palette_runtime_admission`, and
  `theron_v1_track02_loader_intake` PASS.

- 2026-07-16 Theron Track 02 level/object trace preparation: a future
  authenticated post-$3800 grammar receipt now has to match runtime's exact
  loader record, checksums, consumer PCs, and bounded payload windows before
  its windows can be retained. Both field decoders remain explicitly required;
  no level/object fields, bitmap/palette semantics, runtime handoff, drawing,
  fallback visual, or synthetic visual is admitted. Verification: CTest
  `theron_v1_track02_level_object_trace_preparation`,
  `theron_v1_track02_provenance_runtime_consumer`,
  `theron_v1_bitmap_capture_runtime_admission`, and
  `theron_v1_track02_loader_intake` PASS.

- 2026-07-16 Theron Track 02 loader-record container gate: runtime now
  accepts the opaque initial loader record only with a known Track 02 MD5 and
  a MODE1/2352 user-data-aligned raw offset. Unknown identities and malformed
  sector locations reject before provenance or trace preparation. This is
  container ownership only; record grammar, level/object fields, pixels,
  palettes, drawing, fallback visuals, and synthetic visuals remain blocked.
  Verification: CTest `theron_v1_track02_provenance_runtime_consumer`,
  `theron_v1_track02_level_object_trace_preparation`,
  `theron_v1_bitmap_capture_runtime_admission`, and
  `theron_v1_track02_loader_intake` PASS.

- 2026-07-17 Nexus DGN campaign engine ingress: the active-level opaque
  campaign receipt now binds exact PRS3 trace FNV and byte count to the active
  placement ingress, alongside DGN and frame/command identity. Any level,
  package, trace FNV, or trace-size drift makes it not capture-ready. This is
  evidence-only; decoder, mesh, texturing, and rendering remain disabled.
  Verification: CTest `nexus_v1_dgn_campaign_engine_ingress` and
  `nexus_v1_dgn_multi_level_capture_adjudicator` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus operator-only multi-level capture campaign launcher:
  deterministic LEV00--15 Mednafen job plans now require explicit operator
  opt-in, staged retail paths, SHA-256-shaped identities, complete DGN/PRS3,
  SLEV, and SAL plans. Missing retail assets skip safely; identity or target
  drift rejects. The plan launches nothing and cannot generate evidence or
  graphics. Verification: CTest
  `nexus_v1_multi_level_capture_campaign_launcher` PASS; full `firestaff`
  build PASS.

- 2026-07-17 Nexus campaign retail asset hash gate: the operator-only planner
  now reads each staged BIOS, disc/ISO/BIN/CUE member, MENU.BPK, and DM.BIN
  directly and compares its actual SHA-256 before admitting jobs. Missing,
  non-ordinary, malformed, or mismatched paths reject without extraction or
  repository writes; absent retail assets remain skip-safe. Verification:
  CTest `nexus_v1_multi_level_capture_campaign_launcher` PASS; full
  `firestaff` build PASS.

- 2026-07-17 Nexus ISO/BIN/CUE capture-member hash gate: ISO members are now
  read through the existing Nexus ISO reader and SHA-256 checked in process.
  Malformed CUE sheets, missing members, and multiple Nexus-valid data tracks
  reject; the temporary fixture proves a valid `DM.BIN` member without any
  extraction or copy. Verification: CTest
  `nexus_v1_multi_level_capture_campaign_launcher` PASS; full `firestaff`
  build PASS.

- 2026-07-17 Nexus Saturn-card engine lifecycle: the engine now retains only
  an admitted opaque 8 KiB card FNV and monotonically increasing startup route
  epoch. Rejected or replaced receipts clear readiness; card or epoch drift
  rejects consumption. No FNXS fallback or save semantics are admitted.
  Verification: CTest `nexus_v1_saturn_save_capture_engine_lifecycle` PASS;
  full `firestaff` build PASS.

- 2026-07-17 Nexus M12/launcher Saturn-card startup gate: launch selection now
  requires the engine's exact opaque card FNV and route epoch. Card/epoch
  drift rejects, and the receipt explicitly keeps native FNXS usage off.
  Verification: CTest `nexus_v1_launcher_saturn_card_startup` and
  `nexus_v1_saturn_save_capture_engine_lifecycle` PASS; full `firestaff`
  build PASS.

- 2026-07-17 Nexus direct Saturn-card discovery: startup now scans direct
  files for exactly one opaque 8 KiB card, binds its FNV identity through the
  engine route epoch to the M12 launcher gate, and rejects none, multiple, or
  epoch-drift candidates. No FNXS or save semantics are consumed.
  Verification: CTest `nexus_v1_saturn_card_discovery_launch` PASS; full
  `firestaff` build PASS.

- 2026-07-17 Nexus virtual Saturn-card diagnostics: ZIP/ISO/BIN/CUE-style
  virtual identities are retained only as payload-free diagnostics. Virtual
  only candidates cannot launch; mixed direct/virtual candidates reject.
  Verification: CTest `nexus_v1_saturn_card_discovery_launch` PASS; full
  `firestaff` build PASS.

- 2026-07-17 Nexus atomic Saturn-card boot binding: champion startup now
  requires direct-card provenance, exact card FNV, nonzero package identity,
  and current route epoch in one opaque receipt. Stale epoch, card/package
  drift and virtual routing reject before startup. Verification: CTest
  `nexus_v1_saturn_card_boot_binding` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus Structure1F raw provenance: parser-observed record offset,
  length and FNV are retained with package identity and threaded into the
  existing no-draw source packet. Package drift and out-of-bounds spans reject.
  Verification: CTest `nexus_v1_structure1f_provenance` PASS; `firestaff_nexus`
  build PASS.

- 2026-07-17 Nexus Structure2 raw provenance: existing payload-anchor packets
  now carry bounded descriptor offset/length/FNV and package identity; consumer
  checks reject package drift and out-of-bounds spans. No codec, palette or
  pixel decoding is admitted. Verification: CTest
  `nexus_v1_structure2_provenance` PASS; `firestaff_nexus` and full
  `firestaff` builds PASS.

- 2026-07-17 Nexus Structure3 raw face provenance: package geometry packets
  now retain bounded face offset/length/FNV and package identity; consumer
  checks reject package drift and out-of-bounds spans. No PRS3, palette or
  pixel decoder is admitted. Verification: CTest
  `nexus_v1_structure3_provenance` PASS; `firestaff_nexus` and full
  `firestaff` builds PASS.

- 2026-07-17 Nexus Structure3 material-reference provenance: package geometry
  packets now normalize descriptor FNV plus bounded image/palette offsets and
  lengths from existing source targets. Consumer checks reject drift and
  unbounded intervals; no PRS3/palette/pixel decoder is admitted. Verification:
  CTest `nexus_v1_structure3_material_provenance` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus MENU.BPK launcher provenance gate: startup assets now retain
  bounded archive/table metadata from the existing validated upload plan and
  block the real menu route when it is absent or malformed. PRS3 decode remains
  closed. Verification: CTest `nexus_v1_launcher_bpk_provenance_gate` PASS;
  full `firestaff` build PASS.

- 2026-07-17 Nexus MENU.BPK PRS3 no-draw presentation receipt: upload rows
  now retain an FNV-1a value over their already bounded source payload, and the
  validated receipt binds the selected entry index, offset, length, FNV, PRS3
  version/pixel-count, and stream-header facts into the launcher/M11
  presentation path. Hash drift, span overflow, header drift, non-PRS3 and any
  draw-capable row reject; accepted evidence remains explicitly no-draw with no
  decoder or emitted pixels. Verification: CTest
  `nexus_v1_launcher_bpk_no_draw_presentation` and
  `nexus_v1_launcher_bpk_provenance_gate` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus M11 MENU.BPK no-draw host lifecycle: the launcher now
  admits the validated PRS3 presentation receipt only against the active
  whole-package FNV and a monotonic route epoch, then carries it into the
  actual full-start M11 host receipt as explicitly draw-disabled evidence.
  Stale epochs, package drift and cross-entry presentation state reject; every
  rejection clears prior host admission, while a later exact epoch may be
  admitted again. PRS3 decode and rendering remain disabled. Verification:
  CTest `nexus_v1_m11_bpk_no_draw_host` and
  `nexus_v1_launcher_bpk_no_draw_presentation` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus PRS3 compression-descriptor provenance: each M11-eligible
  MENU.BPK PRS3 row now carries the recognized raw mode byte, bounded opaque
  post-header body offset/length/FNV, and header-declared pixel/output sizes.
  The archive, launcher, and engine-owned host receipt compare the descriptor
  exactly; unknown modes, malformed declarations, span overflow, and FNV
  drift fail closed before M11. The receipt remains no-draw and explicitly
  exposes neither a decoder nor pixels. Verification: CTest
  `nexus_v1_launcher_bpk_no_draw_presentation` and
  `nexus_v1_m11_bpk_no_draw_host` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus PRS3 per-entry M11 no-draw selection: the title/menu host
  may now select any engine-owned verified PRS3 upload row rather than only
  the archive's first anchor row. The archive anchor remains checked, while
  the selected row's entry, payload, declared output, and opaque compressed
  body descriptor must exactly match the engine cache. Body-FNV or declared
  output drift clears the host route. No decoder, pixels, or draw promotion is
  exposed. Verification: CTest `nexus_v1_launcher_bpk_no_draw_presentation`
  and `nexus_v1_m11_bpk_no_draw_host` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus PRS3 transition-stable no-draw receipt: M11 host readiness
  now re-finds its selected entry in the current engine upload cache and
  rebuilds the complete presentation receipt before launcher or card-bound
  startup consumption. A body-FNV, output declaration, entry, or payload
  mismatch rejects stale row reuse even with the same package/card/epoch.
  Focused host and card lifecycle tests retain draw-disabled, no-decoder, and
  no-pixel invariants. Verification: CTest
  `nexus_v1_launcher_bpk_no_draw_presentation`,
  `nexus_v1_m11_bpk_no_draw_host`, and
  `nexus_v1_saturn_card_m11_no_draw_startup` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus direct LEV source rehash admission: active DGN binding now
  rechecks the selected ordinary `LEVxx.DGN` file's MD5, byte count, and
  FNV-1a identity before it can feed the existing Structure1F/2/3 no-draw
  chain or M11 admission. A post-discovery source mutation fails closed even
  when the already loaded DGN buffer still matches the old receipt. No source
  payload is retained, and no geometry, texture, decoder, or rendering claim
  is added. Verification: CTest `nexus_v1_lev_corpus_discovery` and
  `nexus_v1_saturn_card_m11_no_draw_startup` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus direct LEV header/Structure1F descriptor provenance: the
  selected direct DGN route now derives its 20-byte container-header and
  complete parser-counted Structure1F span directly from the bounded DGN
  layout parser. M11 retains exact offsets, lengths, entry count, package FNV,
  and raw span FNVs, tied to the existing direct-file rehash, card/package,
  and route-epoch gates. Malformed pointers, source-size/package drift, and
  stale saved descriptor FNVs reject before no-draw readiness. The spans stay
  opaque: no mesh, texture, palette, pixel, decoder, or draw claim is added.
  Verification: CTest `nexus_v1_lev_corpus_discovery` and
  `nexus_v1_direct_static_material_capture` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus selected Structure1F M11 descriptor intake: direct-LEV
  M11 admission now rebuilds the chosen parser-observed Structure1F record
  and binds its entry index, bounded offset/length/FNV, route epoch, package
  FNV, and raw Structure3 face/Structure2-envelope reference to the saved
  no-draw receipt. Readiness rebuilds and compares that intake, so stale row
  FNVs or offsets outside the parser-counted Structure1F table reject even if
  card/package/epoch state otherwise matches. The face/mesh/material join is
  opaque provenance only; it grants no mesh, texture, palette, pixel, decoder,
  or rendering semantics. Verification: CTest
  `nexus_v1_direct_static_material_capture` and
  `nexus_v1_lev_corpus_discovery` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus Structure2/face descriptor M11 intake: a selected,
  parser-bound static Structure1F row now also requires its exact Structure3
  face and 20-byte Structure2 descriptor receipt at direct-LEV M11 admission.
  The receipt binds face/descriptor offsets, lengths and FNVs, the bounded
  image/palette candidate identities, DGN rehash/package identity, route
  epoch, and the existing card/package launcher route. Readiness rebuilds and
  compares the complete chain, rejecting stale Structure2 FNVs and face bounds
  drift. Candidate intervals remain opaque capture windows: no texture, pixel,
  palette, decoder, or draw promotion is granted. Verification: CTest
  `nexus_v1_direct_static_material_capture` and
  `nexus_v1_lev_corpus_discovery` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus M11 Structure2 face capture-replay intake: the admitted
  direct-LEV Structure3 face/Structure2 descriptor chain can now emit a
  payload-free, operator-only capture target and later consume an
  original-Saturn replay observation without changing its source contract.
  Target construction rehashes the ordinary LEV file and binds card, package,
  epoch, DGN size/FNV, selected face/descriptor, and opaque candidate spans.
  Replay intake rebuilds that target and accepts only matching trace, VDP1
  command, texture-candidate and palette-candidate identities; candidate or
  card drift rejects. The result remains no-draw with decoder, texture, pixel,
  palette, and format semantics disabled. Verification: CTest
  `nexus_v1_direct_static_material_capture` and
  `nexus_v1_lev_corpus_discovery` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus external VDP1 capture-envelope admission: M11 can now
  import a read-only original-Saturn VDP1 capture only through a fixed V1
  envelope. The importer checks magic, version, exact header size, an
  end-bounded opaque payload and its FNV, then rebuilds and matches the active
  direct-LEV replay target: route epoch, package/card/DGN identities and size,
  Structure3 face, Structure2 descriptor, and image/palette candidate FNVs.
  Header, descriptor, payload-hash, or route drift rejects and produces only
  the default no-draw receipt. The imported bytes are neither retained nor
  decoded; VDP1 command, pixel, palette, texture, mesh, and rendering claims
  remain disabled. Verification: CTest
  `nexus_v1_direct_static_material_capture` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus local Mednafen VDP1 capture plan: added an operator-only
  external launcher for the `NXSVDP1C` V1 capture envelope. It SHA-256
  rechecks the selected local BIOS and original disc, selects the Mednafen
  BIOS option only from an explicit `us`, `jp`, or `eu` region, and writes an
  owner-only plan binding route epoch, package/card/DGN identities and size,
  face/descriptor, and image/palette candidate FNVs. Dry-run never creates a
  capture; `--launch` passes the same immutable route to a capture-capable
  Mednafen through environment variables and fails unless that external
  producer writes the V1 magic. BIOS, disc, and capture payload bytes are
  never copied or committed, and no decoder/render permission is granted.
  Verification: `nexus_v1_mednafen_vdp1_capture_launcher` PASS; full
  `firestaff` build PASS.

- 2026-07-17 Nexus M12/M11 VDP1 capture-required lifecycle: the local
  BIOS/disc-bound plan is now consumed as an explicit launcher route rather
  than an implicit missing asset. M12/M11 publishes `capture-required` with
  no draw while the V1 envelope is absent, retaining the BIOS region and
  SHA-256 identities plus the complete rehashed direct-LEV card/package/epoch,
  DGN, face, descriptor and candidate target. Resumption rebuilds that target
  and delegates to the strict V1 importer; a valid envelope yields only an
  evidence-ready no-draw receipt, while card or route drift returns to the
  capture-required state. No BIOS/disc/payload is retained, and decoder,
  texture, palette, mesh, render, and fallback permissions remain false.
  Verification: CTest `nexus_v1_direct_static_material_capture` and
  `nexus_v1_mednafen_vdp1_capture_launcher` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus M11 SLEV task-body startup admission: a selected target
  from the existing all-level SLEV task-body capture plan can now enter M11
  only after its source-order original-trace fields match the active direct
  SLEV identity and the pre-existing direct SAL/MAP/SDDRVS, card, package, and
  epoch admission. Missing trace identity or a changed SLEV FNV rejects. The
  receipt retains opaque entry/body/callback locations only and keeps script
  dispatch, SFX playback, codec use, and fallback scripts disabled.
  Verification: CTest `nexus_v1_saturn_card_m11_no_draw_startup` and
  `nexus_v1_slev_task_body_capture_plan` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus M11 SLEV-to-SAL/SDDRVS no-op startup binding: M11 now
  joins an admitted opaque SLEV task-body target to an existing bounded SAL
  container receipt, SNDLEV MAP table receipt, and direct SDDRVS identity.
  An explicit original-trace binding must repeat the task-trace, SAL
  descriptor, MAP table, and driver FNVs alongside the active direct
  SLEV/SAL/card/package/epoch route. Table or descriptor FNV drift rejects.
  The result is deliberately no-op and no-draw: SLEV commands, MAP selectors,
  SAL payloads, SDDRVS ABI, codec use, playback, script dispatch, and fallback
  behavior remain disabled. Verification: CTest
  `nexus_v1_saturn_card_m11_no_draw_startup`,
  `nexus_v1_sal_container_provenance`, and
  `nexus_v1_sndlev_map_provenance` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus external SLEV/SAL command-capture intake: M11 now accepts
  an external `NXSLSC01` V1 envelope only after fixed magic/version/header,
  exact end-bounded opaque payload and payload FNV, and a live match to the
  admitted task-to-SAL no-op route. The header repeats route epoch,
  package/card, SLEV task-trace and source FNV, SAL descriptor, MAP table, and
  SDDRVS FNVs; a changed MAP identity or payload hash rejects. Imported bytes
  are not retained or parsed, and dispatch, playback, codec use, selector/event
  meaning, fallback scripts, and draw remain disabled. Verification: CTest
  `nexus_v1_saturn_card_m11_no_draw_startup`,
  `nexus_v1_slev_task_body_capture_plan`,
  `nexus_v1_sal_container_provenance`, and
  `nexus_v1_sndlev_map_provenance` PASS; full `firestaff` build PASS.

- 2026-07-17 Nexus M12/M11 `NXSLSC01` capture-required/resume route: the
  existing no-op SLEV-to-SAL startup receipt can now enter an operator-only
  capture-required state with explicit BIOS region/SHA-256 and disc SHA-256.
  Resume re-admits the live direct SLEV/SAL/MAP/SDDRVS, card, package, and
  epoch route before importing the fixed V1 capture envelope; task/MAP drift
  invalidates the route. The local Mednafen plan emits only a hash-bound
  manifest and launch environment for a capture-capable external producer. It
  neither copies BIOS/disc bytes nor generates/retains payload data. Commands,
  audio, decoder, playback, draw, and fallback remain disabled. Verification:
  CTest `nexus_v1_saturn_card_m11_no_draw_startup`,
  `nexus_v1_slev_task_body_capture_plan`, `nexus_v1_sal_container_provenance`,
  `nexus_v1_sndlev_map_provenance`, and
  `nexus_v1_mednafen_slev_sal_capture_launcher` PASS; full `firestaff` build
  PASS.

- 2026-07-17 Nexus local `NXSLSC01` preflight closure: the operator artifact
  verifier now independently binds the fixed V1 header, exact task/SAL/MAP/
  SDDRVS/card/package/epoch route, source-task FNV, and exact end-bounded
  opaque payload FNV before the M12/M11 resume path reaches the existing
  importer. MAP or payload drift fails closed and leaves the route no-draw and
  no-op. The paired Mednafen plan remains external-only and hash-bound to the
  operator's BIOS and disc; no BIOS, disc, or capture payload is copied into
  the repository. Verification: CTest
  `nexus_v1_saturn_card_m11_no_draw_startup`,
  `nexus_v1_verify_slev_sal_capture_artifact`, and
  `nexus_v1_mednafen_slev_sal_capture_launcher` PASS (3/3); full `firestaff`
  build PASS; `git diff --check` PASS. No retail capture, script dispatch,
  audio codec/playback, decoder, or rendering is claimed.

- 2026-07-17 Nexus `NXS1OMC1` atomic owner/material capture admission: a
  fixed external envelope now fail-closes unless it exactly repeats the
  existing source-bound Structure1F/1A owner, Structure3 face row, and
  Structure2 descriptor/image/palette candidate identities. It also requires
  an exact end-bounded opaque payload FNV and nonzero raw-trace witness. The
  receipt deliberately leaves owner-to-entry mapping, face/mesh topology,
  texture/palette/VDP1 meaning, decoder, fallback visuals, and draw disabled.
  Verification: CTest `nexus_v1_owner_material_capture_admission` PASS; full
  `firestaff` build PASS; `git diff --check` PASS. No retail capture is
  claimed.

- 2026-07-17 Nexus M12/M11 `NXS1OMC1` capture-required/resume lifecycle: the
  operator-only route now freezes one selected Structure1F/1A owner,
  Structure3 face, and Structure2 material target with BIOS/disc identities.
  Resume reconstructs that target from the live engine and requires every
  source, owner, face, descriptor, and candidate fingerprint to agree before
  it accepts the opaque artifact receipt. Missing corpus/engine state, target
  drift, or a route claiming owner mapping fails closed into capture-required,
  no-draw state. Verification: CTest
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_direct_static_material_capture` PASS (2/2); full `firestaff` build
  PASS; `git diff --check` PASS. No mesh, texture, palette, VDP1, decoder, or
  rendering claim is made.

- 2026-07-17 Nexus `NXS1OMC1` strict import bridge: M12/M11 now hands an
  accepted owner/material artifact to the existing atomic Structure1F/1A,
  Structure3, and Structure2 trace admission only after reconstructing the
  live target, matching the artifact raw-trace witness by exact FNV/size, and
  requiring an external original-Saturn attestation plus the existing manifest
  checks. Any missing engine/corpus, altered trace, or malformed route returns
  capture-required/no-draw. Verification: CTest
  `nexus_v1_owner_material_capture_admission` PASS; full `firestaff` build
  PASS; `git diff --check` PASS. No retail artifact, mesh/pixel semantics,
  decoder, or rendering is claimed.

- 2026-07-17 Nexus `NXS1OMC1` independent-witness adjudication: two opaque
  admitted artifacts now establish capture coverage only when their exact
  source, descriptor, and face identities agree while both artifact and
  raw-trace FNVs are distinct. Duplicate witnesses and target drift fail
  closed. The receipt remains no-draw and makes no owner mapping, topology,
  VDP1, texture, palette, decoder, or rendering claim. Verification: CTest
  `nexus_v1_owner_material_capture_admission` PASS; full `firestaff` build
  PASS; `git diff --check` PASS. No retail capture is claimed.

- 2026-07-17 Nexus `NXS1OMC1` multi-route opaque capture campaign intake:
  two to sixteen already imported owner/face/descriptor witnesses can now be
  retained only when each repeats its exact source, Structure1F/1A owner,
  Structure3 face, Structure2 descriptor, capture, raw-trace, and admitted
  engine-trace identity. Duplicate dungeon level, source, capture, or trace
  identity and every target drift reject. The campaign stores coverage facts
  only and remains no-draw: no payload retention, owner mapping, topology,
  VDP1, texture, palette, decoder, or rendering is claimed. Verification:
  CTests `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); full `firestaff`
  build PASS; `git diff --check` PASS. No retail capture is claimed.

- 2026-07-17 Nexus M12 `NXS1OMC1` campaign capture-required routing: the
  launcher can now export an operator-only, BIOS-region and BIOS/disc-hash
  bound collection route for two to sixteen witnesses, then resume only after
  the strict campaign importer accepts that exact count of already imported
  original-Saturn opaque witnesses. Hash or count drift, partial inputs, and
  invalid route metadata fail closed to capture-required/no-draw. The route
  retains campaign identity coverage only, never trace/payload bytes, and does
  not permit owner mapping, topology, VDP1, texture, palette, decoding, or
  rendering. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); full `firestaff`
  build PASS; `git diff --check` PASS. No retail capture is claimed.

- 2026-07-17 Nexus `NXS1OMC2` multi-route original-capture index admission:
  M12 campaign resume now additionally requires a fixed versioned artifact
  with an exact bounded row table for every admitted dungeon witness. Each row
  repeats level, source, Structure2 descriptor, Structure3 face, opaque
  capture, raw-trace FNV, and raw-trace length; the header and row-table FNV
  must match before the launcher resumes. Corrupt, truncated, hash-drifted, or
  cross-route artifacts return to a valid capture-required/no-draw route. The
  index retains no payload or trace bytes and makes no topology, VDP1, texture,
  palette, decoder, or rendering claim. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; shared Ninja integration
  build PASS; `git diff --check` PASS. No retail capture is claimed.

- 2026-07-17 Nexus `NXS1OMC1` selected-witness artifact preflight: a campaign
  row can now be consumed only after the external fixed capture envelope is
  reparsed and its opaque payload hash, raw-trace size/FNV, DGN level,
  Structure3 face, Structure2 descriptor, and existing opaque engine-trace
  receipt all exactly repeat that row. Altered trace bytes or any target drift
  fail closed to no-draw; capture and trace bytes are never retained. This is
  evidence provenance only and makes no payload-format, mesh, VDP1, texture,
  palette, decoder, or rendering claim. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus M12 route-bound `NXS1OMC1/2` selected-witness admission:
  one explicit Structure1F/Structure3/Structure2 campaign row can now leave
  capture-required only after the M12 route reconstitutes the bounded campaign
  artifact and runs the selected external-capture preflight for that exact row.
  An out-of-range selection, campaign drift, artifact drift, or trace drift
  remains capture-required/no-draw. The resulting receipt is opaque and grants
  no payload interpretation, mesh, VDP1, texture, palette, decoder, or render
  permission. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus M12 multi-route `NXS1OMC1/2` witness selection: a bounded
  selection of two to the campaign's exported number of unique dungeon-witness
  indices is now capture-required/no-draw. Each index is checked against the
  same hash-bound campaign route before import; duplicate and out-of-range
  selections fail closed. The receipt retains selected row identities only and
  grants no cross-level geometry, VDP1, texture, palette, decoder, or render
  permission. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus M11 multi-route dungeon capture start: the live launcher
  now turns a selected NXS1OMC1/2 campaign row into capture-required only when
  the current dungeon level exactly matches one selected witness. The start
  receipt repeats only level/source/Structure2-descriptor/Structure3-face
  identity; unselected levels and campaign drift receive no route and remain
  no-draw. It makes no geometry, texture, palette, VDP1, decoder, or rendering
  claim. Verification: CTests `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus M11 source-bound multi-route target plan: a selected live
  dungeon start now carries the exact existing Structure1F/Structure3/
  Structure2 capture target only after it rechecks level, source FNV,
  face-row FNV, descriptor FNV, capture-required flags, and no-draw policy
  against the selected campaign row. Descriptor or target drift fails closed.
  The plan is opaque capture provenance and grants no geometry, texture,
  palette, VDP1, decoder, or rendering permission. Verification: CTests
  `nexus_v1_owner_material_capture_admission` and
  `nexus_v1_owner_material_capture_campaign` PASS (2/2); isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus PRS3 original-execution export-byte SHA-256 admission: the
  V10 Mednafen evidence importer now computes SHA-256 over the supplied export
  bytes using the existing Firestaff receipt primitive and requires exact
  equality with the external attestation, alongside its prior FNV and complete
  SH-2 input/output/VDP1 checks. A correctly shaped but altered hash fails
  closed. This is evidence-only and grants no PRS3 grammar, decoder, pixel,
  palette, or render permission. Verification: CTest
  `nexus_v1_prs3_original_execution_import` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail trace is claimed.

- 2026-07-17 Nexus PRS3 full output-range/VDP1 capture admission: opaque
  output-range and VDP1 capture bytes now require exact FNV and SHA-256
  attestation against an already authenticated complete execution receipt.
  Byte drift fails closed; no PRS3 decoder, pixel/palette meaning, or rendering
  is permitted. Verification: CTest `nexus_v1_prs3_execution_capture_admission`
  PASS; isolated `build-nexus-codex` full `firestaff` build PASS; `git diff --check`
  PASS. No retail capture is claimed.

- 2026-07-17 Nexus `FONT256.S2D` strict SCR admission: a read-only receipt
  now accepts only the canonical 25,012-byte, SHA-256-attested Saturn font and
  rechecks its `SEGA SATURN SCR` header plus the four bounded section-table
  rows at indices 0/2/4/6. It retains source/table/section FNV witnesses and
  explicitly keeps glyph layout, character encoding, pixel decode, and draw
  permission disabled. The local original file is never copied into the
  repository. Verification: real-data CTest `nexus_v1_font256_s2d_admission`
  PASS; isolated `build-nexus-codex` full `firestaff` build PASS; `git diff
  --check` PASS.

- 2026-07-17 Nexus `FONT256.S2D` first-section capture witness: the next
  source-bound receipt rechecks the SHA-attested SCR admission, live source
  FNV, table FNV, and section-zero FNV before retaining only the first 16 raw
  bytes at `0x0120` as a capture-required target. It makes no claim about
  text fields, glyph layout, palette, character encoding, pixels, or drawing.
  Verification: real-data CTests `nexus_v1_font256_s2d_admission` and
  `nexus_v1_font256_s2d_section_witness` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus `FONT256.S2D` whole-first-section capture receipt: the
  source-bound witness now produces a one-item iterator for exactly
  `[0x0120,0x2130)` with the whole-section FNV, after rechecking the live
  source, SCR table, section, and preamble witnesses. It is explicitly
  capture-required and emits no invented subrecords, glyph layout, palette,
  pixel, or draw semantics. Verification: real-data CTests
  `nexus_v1_font256_s2d_admission`, `nexus_v1_font256_s2d_section_witness`,
  and `nexus_v1_font256_s2d_first_section_capture` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus WARNING.BIN raw DGT2 source admission: reusing the existing
  bounded `RES*`/DGT2 resource-zero lookup, the new receipt requires the
  canonical 101,256-byte SHA-256-attested source and records only raw DGT2,
  CLUT, and pixel-span offsets, lengths, and FNV witnesses. Source or identity
  drift rejects. No CLUT conversion, palette semantics, pixel decode, or draw
  permission is granted. Verification: real-data CTest
  `nexus_v1_warning_dgt2_source_admission` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus WARNING.BIN RES-star/DGT2 descriptor admission: the existing
  bounded lookup now has a source-only receipt that rechecks the canonical raw
  admission and attests the RES declared size, complete 12-byte descriptor
  table, ordered DGT2 descriptor offsets, resource-0 descriptor, DGT2 header,
  and PP header by exact FNV. Descriptor, source, or route drift rejects. No
  CLUT conversion, palette/image meaning, pixel decode, or draw permission is
  added. Verification: real-data CTests
  `nexus_v1_warning_dgt2_source_admission` and
  `nexus_v1_warning_dgt2_descriptor_admission` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus WARNING.BIN DGT2/PP raw payload admission: resource 0 now
  binds its full descriptor-to-next-descriptor window, raw PP header and its
  width/height fields, 512-byte post-header prefix, declared body span, and
  two trailing source bytes by exact offsets, lengths, and FNV witnesses.
  Width/height are retained as un-interpreted header values; no CLUT, pixel
  format, decode, palette, or drawing claim is made. Verification: real-data
  CTests `nexus_v1_warning_dgt2_source_admission`,
  `nexus_v1_warning_dgt2_descriptor_admission`, and
  `nexus_v1_warning_dgt2_pp_payload_admission` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus PRS3 output-to-VDP1 command-order receipt: authenticated
  capture admission now carries the trace-observed final output-write and
  subsequent VDP1-command sequences, and requires the external command-state
  attestation to repeat that strictly increasing pair. Missing, equal,
  reversed, or drifted sequences fail closed. This proves no command grammar,
  decoder, pixel, palette, or rendering behavior. Verification: CTest
  `nexus_v1_prs3_execution_capture_admission` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail capture is claimed.

- 2026-07-17 Nexus PRS3 stream-identity capture admission: opaque output/VDP1
  capture attestation now must repeat the authenticated execution receipt's
  MENU.BPK FNV, DM.BIN FNV, and entry index as well as byte hashes and command
  ordering. Cross-stream or entry drift fails closed. This grants no decoder,
  pixel, palette, or rendering behavior. Verification: CTest
  `nexus_v1_prs3_execution_capture_admission` PASS; isolated
  `build-nexus-codex` full `firestaff` build PASS; `git diff --check` PASS.
  No retail trace is claimed.

- 2026-07-22 Nexus PRS3 source-byte capture admission: the opaque final
  admission now rehashes the actual MENU.BPK and DM.BIN source files and
  extracts exactly the V10 witness stream at its recorded offset/length before
  retaining an output-range and VDP1-command capture. Full assets, bounded
  stream, output bytes, and VDP1 bytes each require both FNV-1a and SHA-256
  agreement; stream identity and the output-write-to-command sequence must
  match the independently authenticated execution receipt. Asset or byte drift
  fails closed. The receipt is evidence-only and explicitly grants no PRS3
  grammar, decode, pixel/palette, VDP1-layout, or rendering permission.
  Verification: CTests `nexus_v1_prs3_execution_capture_admission` and
  `nexus_v1_prs3_original_execution_import` PASS; real-artifact probe remains
  skip-safe without a local authenticated Saturn export.

- 2026-07-17 Nexus M11 Structure3 topology-descriptor intake: the selected
  direct-LEV Structure1F route now rederives its documented Structure3b face
  framing, bounded entry-local vertex table, ordered referenced-vertex-row
  fingerprint, and paired normal row before no-draw admission. The receipt is
  bound to level, route epoch, package FNV, selected Structure1F row, exact
  face span, and all derived offsets/lengths/FNVs; vertex-row drift and an
  out-of-bounds vertex table reject. It is capture-required only and does not
  promote mesh topology to a surface, winding, transform, material, texture,
  palette, VDP1, decoder, or draw claim. Verification: focused
  `nexus_v1_direct_static_material_capture` CTest is skip-safe without local
  retail corpus; full `firestaff` build PASS.

- 2026-07-17 Nexus `NXS3TOP1` external Structure3 topology-capture import:
  the launcher now emits an immutable replay target only after it rehashes
  the active direct LEV and rederives the admitted Structure1F/Structure3
  face, vertex-table, referenced-vertex, and paired-normal provenance. The
  importer accepts only the fixed V1 header, exact level/card/package/epoch
  and span/FNV bindings, plus a bounded opaque payload with a matching FNV.
  It retains no payload and remains capture-required/no-draw: it grants no
  topology semantics, mesh, transform, texture, palette, VDP1, decoder, or
  rendering claim. Verification: focused CTest
  `nexus_v1_direct_static_material_capture` PASS (1/1); full `firestaff`
  build PASS; `git diff --check` PASS. No retail topology capture is claimed.

- 2026-07-17 Nexus operator-only `NXS3TOP1` Mednafen launch plan and
  M12/M11 lifecycle: the local plan accepts only an executable plus ordinary
  SHA-256-verified BIOS and original-disc files, a selected BIOS region, and
  the exact rehashed direct-LEV/Structure1F/Structure3/card/package/epoch
  topology target. It writes a deterministic external launch manifest, does
  not copy or retain media or payload bytes, and requires explicit
  `--operator-only --launch` before invoking Mednafen. The matching M12/M11
  receipt remains capture-required/no-draw until `NXS3TOP1` is re-imported
  through the existing bounded opaque importer; card-route drift rejects the
  resume. No topology, mesh, pixel, palette, VDP1, decoder, or renderer claim
  is promoted. Verification: CTest
  `nexus_v1_direct_static_material_capture` and
  `nexus_v1_mednafen_structure3_topology_capture_launcher` PASS (2/2); full
  `firestaff` build PASS; `git diff --check` PASS.

- 2026-07-17 Nexus `NXS3TOP1` local artifact verifier/import bridge: a
  read-only local verifier now compares a plan's SHA-256 BIOS/disc identities
  and all fixed V1 LEV/Structure1F/face/vertex/normal header metadata before
  accepting an artifact for the live route. The engine-side preflight repeats
  those BIOS, disc, LEV, face, vertex, normal, payload-bound, and payload-FNV
  checks before delegating to the existing opaque importer; any drift retains
  the capture-required/no-draw route. It retains no payload and promotes no
  topology, mesh, pixel, palette, VDP1, decoder, or renderer semantics.
  Verification: CTest `nexus_v1_direct_static_material_capture`,
  `nexus_v1_mednafen_structure3_topology_capture_launcher`, and
  `nexus_v1_verify_structure3_topology_capture_artifact` PASS (3/3); full
  `firestaff` build PASS; `git diff --check` PASS. No retail artifact is
  claimed.

- 2026-07-17 Nexus `NXSVDP1C` Structure2/Structure3 material local artifact
  verifier/import bridge: local verification now binds the existing Mednafen
  plan's SHA-256 BIOS/disc identity to fixed V1 DGN/route, face, descriptor,
  image/palette candidate, opaque payload, and nonempty trace/VDP1-command
  witnesses. The M12/M11 bridge repeats those checks before it invokes the
  existing opaque VDP1 importer; BIOS, card-route, or candidate drift retains
  the capture-required/no-draw route. No payload is retained and no PRS3,
  pixel, palette, VDP1 command, decoder, or rendering semantics are claimed.
  Verification: CTest `nexus_v1_direct_static_material_capture`,
  `nexus_v1_mednafen_vdp1_capture_launcher`,
  `nexus_v1_verify_structure3_topology_capture_artifact`, and
  `nexus_v1_verify_vdp1_capture_artifact` PASS (4/4); full `firestaff` build
  PASS; `git diff --check` PASS. No retail material artifact is claimed.

- 2026-07-17 Nexus `NXSPRS3M` MENU.BPK PRS3 material-capture preflight: the
  new local verifier accepts only a fixed V1 envelope whose card, package,
  epoch, selected entry, bounded compressed-body offset/length/FNV, and
  declared-output byte count exactly match the existing M11 no-draw MENU.BPK
  receipt. Payload bounds/FNV plus nonempty opaque trace witnesses are
  required, but no payload is retained or decoded. Entry, body, output, and
  card-route drift reject without promoting PRS3, Structure2 material, pixel,
  palette, VDP1, decoder, or rendering behavior. Verification: CTest
  `nexus_v1_prs3_material_local_artifact` and
  `nexus_v1_verify_prs3_material_capture_artifact` PASS (2/2); full
  `firestaff` build PASS; `git diff --check` PASS. No retail PRS3 artifact is
  claimed.

- 2026-07-17 Nexus operator-only `NXSPRS3M` Mednafen capture plan and
  M12/M11 lifecycle: the local plan requires SHA-256-verified BIOS and disc,
  then binds MENU.BPK package, card/epoch, entry, compressed body, and
  declared-output metadata into a deterministic external manifest. M12/M11
  publishes only a capture-required/no-draw receipt and resumes solely after
  the existing local opaque-artifact preflight revalidates the same route.
  No media or payload is retained, and no PRS3 decoder, pixels, palette,
  Structure2 material semantics, or rendering is promoted. Verification:
  CTest `nexus_v1_prs3_material_local_artifact`,
  `nexus_v1_verify_prs3_material_capture_artifact`, and
  `nexus_v1_mednafen_prs3_material_capture_launcher` PASS (3/3); full
  `firestaff` build PASS; `git diff --check` PASS. No retail artifact claimed.

- 2026-07-17 Nexus Saturn-card/M11 atomic champion-start binding: the direct
  opaque 8 KiB card receipt and active M11 MENU.BPK PRS3 no-draw host receipt
  now form one launcher-owned startup receipt. Both require the same package
  FNV and route epoch; cross-package title/card state, stale host/card epochs
  and card drift reject before the direct-card champion route is marked ready.
  The card remains opaque and the M11 side remains draw-disabled with no PRS3
  decoder. Verification: CTest `nexus_v1_saturn_card_m11_no_draw_startup`,
  `nexus_v1_m11_bpk_no_draw_host`, and
  `nexus_v1_launcher_saturn_card_startup` PASS; full `firestaff` build PASS.

- 2026-07-17 DM1 original-save C-event handoff: C2 champion `ActionIndex`
  and `PoisonEventCount` now round-trip through F0435/F0802/F0796 with PC34
  byte-width validation. C25 explosion and C29 group-reaction exports are
  source-receipt-only; malformed, synthetic, source-byte, and runtime-drift
  states reject. The C05/C06/C07/C08/C09/C10 and C02 source-square fixtures
  now isolate their target event with authenticated C70 queue peers.
  Verification: `test_dm1_v1_original_save_pc34_handoff` PASS.

- 2026-07-17 DM1 C2 PARTY_INFO Event71 ownership: F0435 imports byte 86
  `Event71Count_Invisibility` into both runtime mirrors, and F0802 preserves
  it as a range-checked source byte. The C71 regression proves authenticated
  source materialization; `test_dm1_v1_original_save_pc34_handoff` PASS.

- 2026-07-17 CSB F0435 native-save provenance: the original-save runtime
  receipt retains the validated raw-header FNV-1a, active verified Dungeon.dat
  MD5, and their composed source identity. A read-only verifier rereads the
  native header and rejects FNV, Dungeon, or composed-identity drift without
  loading or mutating runtime state. The door-to-HUD handoff fixture now also
  declares the complete C001/C002/C003/C004/C017/C040 surface contract, so
  the terminal pose reaches HUD only through the same full-session gate.
  Verification: CTest `csb_v1_startup_package_identity_pc34` PASS (with
  explicit absent-corpus SKIP); isolated `firestaff` build PASS; `git diff
  --check` PASS.

- 2026-07-17 CSB TITLE.C source-step capture: visual startup capture now uses
  coherent title frame/source-step pairs: PRESENTS 59/1, CHAOS 60/2, full
  CHAOS 77/19, held CHAOS 79/21, and STRIKES 100/22. The focused package
  identity test locks those boundaries and verifies title capture through the
  existing real-package receipt path. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; `test_csb_v1_boot_runtime_handoff`
  now passes the visual-sequence assertion (458 pass, 10 unrelated remaining);
  full `firestaff` build and `git diff --check` PASS.

- 2026-07-17 CSB TITLE.C presentation receipt: the boot route receipt and
  focused package-capture regression now agree on the source-owned 20-tick
  CHAOS zoom span. This removes the stale 18-tick expectation without opening
  an inferred title, entrance, HUD, or door route. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; handoff title-source-lock
  assertion PASS (459 pass, 9 unrelated remaining); full `firestaff` build
  and `git diff --check` PASS.

- 2026-07-17 CSB TITLE.C CHAOS-hold receipt: the full CHAOS capture is bound
  to frame 77/source step 19, and the hold tick is derived from that source
  step rather than a stale frame offset. The focused package test verifies the
  render-view hold classification and two-tick receipt span. Verification:
  CTest `csb_v1_startup_package_identity_pc34` PASS; handoff CHAOS-hold
  assertion PASS (460 pass, 8 unrelated remaining); full `firestaff` build
  and `git diff --check` PASS.

- 2026-07-17 CSB TITLE.C STRIKES receipt: the render-view receipt now binds
  STRIKES to the source-owned frame 80/step 21 transition, with phase tick
  derived from that source-step boundary. The focused package test covers the
  resulting one-tick STRIKES receipt. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; handoff STRIKES assertion PASS
  (461 pass, 7 unrelated remaining); full `firestaff` build and `git diff
  --check` PASS.

- 2026-07-17 CSB ENTRANCE.C pointer handoff: `begin_door_opening` now
  publishes source-owned pre-open delay as opening step 0, leaving C002/C003
  frame ownership for the later animation state. The focused regression locks
  that boundary; pointer action, host-input dispatch, post-input door render,
  and redraw receipts all pass through the same state. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; handoff pointer assertion PASS
  (465 pass, 3 MD5-scan failures remaining); full `firestaff` build and `git
  diff --check` PASS.

- 2026-07-17 CSB startup graphics-MD5 receipt invalidation: a changed graphics
  proof now atomically clears the retained startup receipt hash and hex form,
  preventing stale package identity from reaching session admission. The
  focused receipt regression covers this exact drift. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; handoff graphics-MD5 assertion
  PASS (three later Dungeon/re-admission assertions remain); full `firestaff`
  build and `git diff --check` PASS.

- 2026-07-17 CSB startup Dungeon/re-admission boundary: metadata drift now
  marks a startup receipt emission invalidated, so restoring MD5 fields in
  place cannot republish its identity. A separate scanner/profile snapshot is
  required to issue the original verified pair anew. Focused coverage proves
  Dungeon drift, rejected in-place repair, and fresh reissue; the full CSB
  handoff fixture is green. Verification: CTest
  `csb_v1_startup_package_identity_pc34` PASS; `test_csb_v1_boot_runtime_handoff`
  PASS (469 assertions); full `firestaff` build and `git diff --check` PASS.
# Nexus multi-level direct-LEV capture-plan gate (2026-07-17)

- The operator-only multi-level capture launcher now requires a valid,
  direct-file-only LEV00--LEV15 discovery receipt and exact per-level DGN FNV
  agreement before emitting a capture job. The receipt retains identities
  only; it does not materialize LEV payloads, decode graphics, or enable draw.
  Focused launcher coverage and the full `firestaff` build are green.

# Nexus direct SLEV/SAL no-runtime launch provenance (2026-07-17)

- Nexus now discovers only direct, hash-verified `SLEV00`--`SLEV15`,
  `SNDLEV` SAL/MAP, and `SDDRVS.TSK` files and retains path, size, MD5, and
  FNV identity only. M11 accepts an active-level receipt only when those
  identities match its current auxiliary sources and the active card/MENU.BPK
  route epoch. Missing, mixed, stale, and cross-level inputs fail closed;
  script dispatch and SFX playback remain blocked. Focused discovery and M11
  lifecycle CTests plus the full `firestaff` build are green.

# Nexus direct SDDRVS dungeon admission (2026-07-17)

- M11 dungeon admission can now carry a direct, hash-verified `SDDRVS.TSK`
  identity only when the existing direct-LEV, card, MENU.BPK package, route
  epoch, and active level-aux receipts still agree. The driver file is
  rehashed and FNV/size-checked at consumption, so missing or mutated bytes
  invalidate the prior discovery receipt. This remains no-draw and
  no-dispatch: it does not parse SDDRVS, execute scripts, decode audio, or
  play sound. Focused stale/mutated/missing coverage and the full `firestaff`
  build are green.

# Nexus direct SAL/SLEV dungeon admission (2026-07-17)

- M11 can now retain one no-draw dungeon receipt for a level-local direct
  SLEV/SAL/MAP identity only when its direct LEV, card, MENU.BPK package,
  epoch, and active auxiliary receipt agree. Every direct asset is rehashed
  and checked against its stored size/FNV at consumption; missing, mutated,
  and stale identities fail closed. The route still blocks script dispatch,
  codec promotion, and SFX playback. Focused fixture coverage and the full
  `firestaff` build are green.

# Nexus direct SAL container provenance (2026-07-17)

- Direct SAL consumption now accepts only the source-backed `dsp01.EX`
  preamble, exact full-file FNV/size, and one bounded opaque payload interval
  with its own FNV. M11 SAL/SLEV dungeon admission requires that receipt and
  rejects malformed headers, source drift, and empty/out-of-bounds intervals.
  No codec, sample format, decode, or playback is implied. Focused container
  CTests, the full `firestaff` build, and `git diff --check` are green.

# Nexus direct SNDLEV MAP table provenance (2026-07-17)

- Direct MAP consumption now requires exact source FNV/size, its documented
  24-byte opaque header, bounded 8-byte row table, and `ff ff` terminator.
  M11's SAL/SLEV dungeon receipt requires this no-playback provenance beside
  the direct SAL container receipt; malformed, unterminated, or drifted maps
  fail closed. No selector/event meaning, codec, or playback was added.
  Focused parser coverage, full `firestaff`, and `git diff --check` are green.

# Nexus M11 SNDLEV MAP row no-playback route (2026-07-17)

- M11 now admits a selected opaque MAP row only after it rehashes the direct
  MAP source and exactly binds the row index, offset, fixed length, row FNV,
  table FNV, level, package, card, and route epoch to the existing no-draw
  dungeon receipt. Mutated/stale sources and rows outside the bounded table
  reject. This does not interpret MAP selector/event bytes or permit codec
  promotion, script dispatch, or audio playback. Focused MAP provenance
  CTests and the full `firestaff` build are green.

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

# CSBWin active timer-queue M11 handoff (2026-07-17)

- M11 resume coverage now follows CSBWin `GAMEBLOCK2.NumTimer`, not the
  serialized TimerQueue storage capacity. The verified fixture retains only
  its two active queue slots in source order, with `timeline_queue.gameTick`
  equal to the restored save tick; the third stored slot remains inactive.
  `csb_v1_m11_csbwin_timer_queue_resume` verifies that atomic valid handoff
  and rejects a checksum-corrupt queue before M11 publishes a session. The
  full `csb_v1_m11_startup_resume_gate` is now green. Verification: both
  focused CTests PASS; isolated `firestaff` build and `git diff --check`
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

# CSBWin DSA restored-TIMER owner receipt (2026-07-17)

- Tightened the existing checksum-bound DSA/save runtime handoff with an
  immutable per-execution TIMER owner receipt. The receipt now binds the
  original TimerQueue slot and TIMER index to the exact CSBWin Timer.cpp
  function, saved action/position bytes, target location, concrete type-47
  actuator location, and already authenticated DSA action. Currentness
  rebuilds that owner selection under the existing save, Dungeon.dat, startup
  session and runtime-tick handoff before M11 may commit the outcome. Any
  function/action/slot/location/action-selection drift rejects without rerun
  or mutation. Unknown opcode bodies remain blocked by the existing source
  verifier; no wrapper, timer synthesis, or new DSA semantics were added.
  Verification: `csb_v1_csbwin_dsa_runtime_admission_pc34_compat` and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MESSAGE queue transaction (2026-07-17)

- Tightened the source-proven `DSACMD_MESSAGE`, `MESSAGE32`, and
  `DESSAGE32` execution family around CSBWin DSA.cpp's
  `QueueDSASwitchAction` boundary. Decoded message operands now remain a
  bounded pending request until the entire checksum-authenticated DSA action
  accepts; only then can the existing runtime queue owner publish its source
  timer event. A following unowned opcode leaves the queue unchanged. This
  retains the original M/D route, delay, action and target operands without
  inventing an event, target-room mapping, wrapper, or general interpreter.
  Verification: `csb_v1_dsa_pure_control_pc34_compat` and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA COPYTELEPORTER transaction (2026-07-17)

- Made source-proven `DSACMD_COPYTELEPORTER` and `COPYTELEPORTER32` atomic
  with the rest of an authenticated DSA action. The exact decoded source and
  destination location operands now remain pending until all later action
  words accept, then invoke only the existing runtime DB1/CELLFLAG copy
  owner. A later rejected or unowned opcode cannot leave a partial dungeon or
  save-runtime mutation. Missing owner data, unknown actions and unsupported
  opcode bodies remain closed; no teleporter, cell, link or event is created.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DB3 COPY runtime transaction (2026-07-17)

- Tightened source-proven `STKOP_Copy` (DSA.cpp:4696-4721) at the live
  runtime boundary. A new optional owner callback receives the staged source
  and destination Things together, re-resolves both as complete DB3 records,
  and requires the current source bytes to equal the staged six-byte image
  before writing the destination. The regular runtime path uses that callback
  inside its existing candidate Dungeon/save transaction, so source drift,
  wrong type, unavailable owner, or a later unknown opcode cannot publish a
  partial mutation. Pure stack consumers retain the existing staged fallback;
  no generic copy API, timer event, wrapper, or synthetic action was added.
  Verification: `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA GeneratorDelayStore owner transaction (2026-07-17)

- Tightened `STKOP_GeneratorDelayStore` (DSA.cpp:2876-2915) at its DB3
  runtime boundary. Its pending request now retains the source
  GeneratorDelay@ value, and the candidate runtime re-reads the same loaded
  type-6 generator or source-defined type-0 fallback before committing the
  byte-eight delay. A changed/missing owner or a following unknown opcode
  leaves the candidate and live Dungeon unchanged. This reuses the existing
  save/runtime candidate handoff only; it creates no generator, timer, queue
  event, wrapper or inferred opcode behavior. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA GeneratorDelayStore DB3 runtime receipt (2026-07-17)

- Extended the existing source-owned `STKOP_GeneratorDelayStore` transaction
  with an atomic DB3 receipt: selected location, original delay, committed
  delay, and whether CSBWin selected the type-6 generator instead of its
  defined type-0 fallback. This data publishes only after the existing
  re-read/compare commit succeeds. A saved dispatch retains its exact
  TimerQueue/TIMER scope through the pre-existing restored-timer receipt; this
  package adds no queue route, timer body, generator allocation, wrapper, or
  synthetic save behavior. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MissileInfoStore DB14/TIMER transaction (2026-07-17)

- Tightened `STKOP_MissileInfoStore` (DSA.cpp:2824-2846) around its paired
  DB14 and TIMER ownership. The action now retains the original four-value
  DB14/TIMER image before applying stack operands. Candidate commit re-reads
  that exact image, then uses the existing strict runtime path that confirms
  the serialized TIMER is valid and represented by exactly one active queue
  event before updating DB14 and TIMER direction together. Stale/mixed DB14,
  TIMER or queue evidence and later unknown actions remain non-mutating. No
  missile, timer, event, wrapper or inferred opcode behavior was added.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MissileInfoStore runtime receipt (2026-07-17)

- Extended the source-owned `STKOP_MissileInfoStore` receipt after its
  existing atomic DB14/TIMER commit. It now records the concrete DB14 Thing
  and all four source words before and after mutation; a saved dispatch adds
  the already strict unique TimerQueue/TIMER scope only after that receipt is
  complete. The regression locks the original DB14 range/damage/direction
  image, the committed image, and the fail-closed stale-owner behavior. No
  generic missile, queue, timer, or opcode interpreter route was introduced.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DB14/TIMER owner receipt (2026-07-17)

- Hardened `STKOP_MissileInfoStore` publication with its separate DB14-linked
  TIMER owner. The candidate must preserve the DB14 timer index and one
  identical serialized TimerQueue slot while revalidating TIMER function,
  time, and the raw position byte before/after direction mutation. The
  execution receipt now carries those facts alongside the existing DB14
  Thing and four-word images. Missing, duplicate, reordered, stale, or mixed
  owner data rejects before live Dungeon or receipt publication. This adds no
  timer body, missile route, queue synthesis, or fallback opcode behavior.
  Verification: `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA StoreExCellFlg EXPOOL receipt (2026-07-17)

- Extended the existing source-owned `STKOP_StoreExCellFlg` transaction with
  a full EXPOOL before/after receipt. The opcode now requires the current
  eight-word cell-flags owner before staging its source flag byte, and a
  committed action records the packed location and all eight words before and
  after replacement. Missing or stale EXPOOL data and later invalid opcode
  bodies reject before any save-tail publication. Saved TIMER/queue scope
  remains exclusively the existing restored-dispatch receipt; no allocator,
  timer, queue, or generic interpreter was added. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA StoreExCellFlg save-tail identity receipt (2026-07-17)

- Completed the runtime side of `STKOP_StoreExCellFlg` (CSBWin
  `DSA.cpp:3298-3328`). Its existing DB11/EXPOOL candidate receipt now
  includes the admitted tail FNV before and after the source eight-word
  replacement. The runtime adapter accepts only `set_extended_cell_flags ==
  1`, so the source API's negative failure result cannot be treated as a C
  truthy commit. A tail whose FNV has drifted rejects before action execution
  or runtime receipt publication. The common restored-dispatch path remains
  the only owner of saved TimerQueue/TIMER scope; no queue, timer, allocator,
  wrapper, or synthetic cell state was introduced. Verification:
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_runtime_skin_expool_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

- The common restored-dispatch handoff now also revalidates an EXPOOL-mutating
  execution receipt before it records or exposes saved TimerQueue/TIMER scope.
  Its post-write tail must still be valid, complete, and equal to both the
  declared and receipt FNV; drift clears the saved handoff and makes the
  execution and runtime-chain readers reject. This remains an identity gate,
  not a DB11 allocator, TIMER constructor, or opcode interpreter. Verification:
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, `csb_v1_dsa_localstate1_save_handoff`,
  and `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA TalentsStore wing save receipt (2026-07-17)

- Bound high-bit `STKOP_TalentsStore` (CSBWin `DSA.cpp:4291-4338`) to the
  existing `CHARDESC::SaveToWings` DB11/EXPOOL owner. A completed action now
  publishes its actual wing fingerprint, talents word before/after, and the
  complete tail FNV before/after only after the candidate re-reads the same
  eight `EDT_Character` records on both sides. A missing wing remains the
  source no-op; a later unsupported word leaves the candidate tail unpublished.
  The common receipt-currentness gate rejects tail drift before the execution
  receipt or restored TimerQueue/TIMER handoff can be consumed. No character
  creation, swap, DB11 allocation, generic save write, queue, or interpreter
  route was added. Verification: `csb_v1_wing_expool_runtime`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA TalentsStore party receipt (2026-07-17)

- Completed low-bit `STKOP_TalentsStore` publication from the actual imported
  CSBWin owner. The focused regression serializes the `DSA::Read` action
  table, validates its RCS, imports and selects the saved `(18, 1, 0, 0)`
  action in file order, then confirms that the source `AMPERSAND2` action
  updates the live party CHARDESC talents word. The execution receipt binds
  champion count, fingerprints, and talents before/after; any later talents
  drift makes it unavailable. This fixes only the lost context-to-runner
  party-talents return path. No synthetic DSA action, character/wing mutation,
  timer, queue, save allocation, or general interpreter route was added.
  Verification: `csb_v1_dsa_queued_localstate2_timer` PASS; isolated
  `firestaff` build and `git diff --check` PASS.

# CSBWin DSA FalsePit DB1 receipt (2026-07-17)

- Added a distinct runtime receipt for source `STKOP_FalsePit` (CSBWin
  `DSA.cpp:2859-2876`). It is admitted only after an RCS-authenticated imported
  DSA action selects one existing `roomPIT`, changes only CELLFLAG bit zero,
  and preserves the complete five-word DB1/CELLFLAG image before and after.
  Mixed cell writes, a non-pit state, room-type drift, and later CELLFLAG
  drift fail closed before a save/runtime handoff can consume the receipt. The
  implementation adds no pit traversal, timer, queue, renderer, or inferred
  cell semantics. Verification: `csb_v1_dsa_queued_localstate2_timer` PASS;
  isolated `firestaff` build and `git diff --check` PASS.

# CSBWin DSA ExperiencePlus CHARDESC receipt (2026-07-17)

- Extended the admitted `STKOP_ExperiencePlus`/`Magic.cpp::AddToSkill`
  candidate with a source-owned save receipt. It records the selected
  CHARDESC skill and its source basic-skill pair, increment, and both values
  before/after. Before publication the runtime rederives the exact UI16
  increment, cap, and no-LevelUp result from the unmodified party image and
  compares it to the candidate. Later selected/basic-skill drift makes the
  execution receipt and restored TimerQueue/TIMER handoff unavailable. The
  unimplemented LevelUp transaction, random/stat/UI work, generic XP writes,
  and inferred timer behavior remain closed. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA LevelUp prerequisite receipt (2026-07-17)

- Added a read-only, source-bound preflight for the existing
  `Magic.cpp::AddToSkill` LevelUp boundary. It publishes only when the exact
  proposed selected/basic CHARDESC skill pair crosses the unadjusted-mastery
  threshold, retaining UI16 increment, both values before/after, and mastery
  before/after. The receipt is current only while the source skill pair still
  equals its precondition. It deliberately executes no LevelUp code and
  commits no XP, random, stat, UI, save, or timer result. A future complete
  original LevelUp owner must consume this prerequisite atomically. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA MonsterStore DB4 runtime receipt (2026-07-17)

- Bound the already source-owned `STKOP_MonsterStore` (`DSA.cpp:4075-4125`)
  commit to an atomic DB4 runtime/save receipt. Staging now preserves the
  complete eight-word group image before the first write. Before the cloned
  Dungeon is published, runtime re-reads that exact DB4 Thing on both sides
  and requires complete before/after equality, along with the original
  selected-field write mask. The published receipt expires if the live DB4
  image later drifts, which also closes the existing restored timer handoff.
  A later unsupported source word leaves DB4 and the receipt untouched. No
  group allocation, link traversal, timer or queue creation, generic opcode
  handling, or LevelUp RNG/stat/UI route was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA CellStore CELLFLAG/DB0/DB1 runtime receipt (2026-07-17)

- Bound source-owned `STKOP_CellStore` (`DSA.cpp:3837-3956`) to the existing
  byte-map and first matching DB0/DB1 record owner. The pending write keeps
  the complete five-word `CellFetch` image before its first mutation. Before
  cloned Dungeon bytes publish, runtime re-reads the same location from both
  Dungeon images and requires complete before/after equality plus the source
  write mask. The receipt becomes unavailable after drift in any field that
  the CellFetch owner exposes. A later unsupported opcode leaves CELLFLAG,
  DB0/DB1, and receipt untouched. No cell, record, Thing link, timer, queue,
  inferred room rule, or generic interpreter path was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_wing_expool_runtime`, `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_localstate1_save_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA CopyTeleporter DB1/CELLFLAG runtime receipt (2026-07-17)

- Extended the existing `DSACMD_COPYTELEPORTER` source-owner transaction with
  complete five-word CellFetch images for source, destination-before and
  destination-after. Candidate publication re-reads all three images across
  the cloned Dungeon boundary; receipt currentness rejects later source or
  destination DB1/CELLFLAG drift. The focused fixture uses two actual
  byte-map teleporter squares, their source-first-Thing entries, and two DB1
  records. It proves atomic payload/CELLFLAG copy and destination drift
  rejection. No teleporter, record link, timer, queue, or generic cell route
  was added. Verification: `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA object-property runtime receipt (2026-07-17)

- Bound the existing CSBWin DSA object-property family (`SetCurse`,
  `SetBroken`, `SetCharges`, `SetPoisoned`, and `SetSubType`) to its raw
  DB5/DB6/DB8/DB10 owner at the staged commit boundary. The receipt retains
  Thing, source property kind, and normalized value before/after; it is
  current only while the same raw property still equals the recorded result.
  The DB5 fixture proves a real SetCharges commit and charge-field drift
  rejection. Unsupported Thing/property pairs and unknown opcode paths remain
  closed; no generic object mutation, item allocation, or timer route was
  added. Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA Random GAMEBLOCK2 runtime receipt (2026-07-17)

- Bound `STKOP_Random` to the existing authenticated CSBWin
  `GAMEBLOCK2.RandomNumber` owner with exact seed before/after in the runtime
  receipt. Receipt currentness requires the verified GAMEBLOCK2 summary and
  exact post-action seed, so later seed drift closes runtime/save consumption.
  The focused fixture verifies the source seed transition and drift rejection.
  No host RNG, timer, queue, or random-dependent opcode route was added.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DisableSaves GAMEBLOCK2 runtime receipt (2026-07-17)

- Bound `STKOP_DisableSaves` to the existing staged CSBWin save-policy owner
  with exact before/after state in the runtime receipt. Receipt currentness
  rejects later policy drift. The focused fixture proves the zero-to-one
  transition and drift rejection. No save operation, timer, queue, fallback
  policy, or broader GAMEBLOCK2 mutation was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA DiscardText DB2/F0168 runtime receipt (2026-07-17)

- Bound `STKOP_DiscardText` to the existing TT_OPENROOM DB2/F0168 one-message
  receipt. The runtime receipt retains the admitted message identity before
  the authenticated clear and the exact cleared after-image; a replacement
  text receipt invalidates later runtime/save consumption. The focused fixture
  proves the clear and replacement-drift rejection. No text queue, log,
  renderer fallback, or generated text was added. Verification:
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA SETSKIN EXPOOL receipt and cache admission (2026-07-17)

- Tightened `AMPERSAND2/SETSKIN` (CSBWin `DSA.cpp:3122-3135`) at the existing
  EXPOOL/DB11 candidate boundary. The opcode now requires a readable current
  `EDT_Skins` owner before staging and, after the complete authenticated
  action commits, records its packed location and exact skin byte before and
  after the write. Missing ownership and a later unsupported action leave the
  candidate, live save state, and receipt untouched. The existing restored
  dispatch remains solely responsible for appending saved TimerQueue/TIMER
  scope; no timer, queue, allocator, wrapper, or synthetic skin is added.
- The live HUD skin consumer now compares its saved-tail FNV/size receipt on
  every grid admission and clears cached columns on tail drift, truncation, or
  absence. It cannot present a prior save's EXPOOL bytes as a fallback.
  Verification: `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_runtime_skin_expool_handoff`, `csb_v1_phase7_verification`,
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA ModifyMessage timer-scope receipt (2026-07-17)

- Bound `STKOP_ModifyMessage` (DSA.cpp:4931-4947) to the authenticated
  runtime execution receipt. Its source-clamped SET/CLEAR/TOGGLE triplet is
  now exposed only when the existing timer-scope runner has completed a
  verified action; callers cannot fabricate the values through a generic
  timer, queue, or save field. The triplet remains transient and does not
  mutate a TIMER or serialize a replacement save. Verification:
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`,
  `csb_v1_csbwin_dsa_runtime_admission_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin DSA ModifyMessage saved-timer receipt (2026-07-17)

- Connected the existing `STKOP_ModifyMessage` effect to the real restored
  `ProcessTimers` handoff. Each admitted saved-timer dispatch starts with
  CSBWin's source 0/1/2 SET/CLEAR/TOGGLE map; a completed DSA action may
  publish its clamped map only alongside the revalidated queue slot, TIMER
  index, function, action, position, time, and authenticated DSA identity.
  Failed or stale queue input clears the transient execution receipt. This
  remains an observational runtime receipt: it creates neither a queue nor a
  serialized timer and does not infer any unknown opcode body. Verification:
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

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

# CSBWin queued LocalState=1 save-transition receipt (2026-07-17)

- Aligned the LocalState=1 save handoff with the imported CSBWin DSA record:
  `m_state`, `m_localState`, `m_groupID`, and `m_stateCount` are four
  source words, not the obsolete two-byte state plus byte flags layout. The
  RCS-validated candidate now writes only the source `m_state` word and the
  execution receipt records its before/after state and post-write tail FNV.
  A queued restored timer adds its exact queue slot, TIMER index, time and
  authenticated DSA action identity to that receipt. Corrupt RCS or stale
  queue evidence publishes neither a state transition nor a receipt. No
  generic state machine, queue, or synthesized save was added. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin queued LocalState=0 DB3 state receipt (2026-07-17)

- Added the source-owned `DSA.cpp::PutState` LocalState=0 path. It writes
  only the selected type-47 DB3 actuator's `DSAstate` nibble in word2 after
  revalidating the admitted Extended Features RCS/FNV tail, selected action,
  and source actuator identity. The execution receipt identifies DB3 storage,
  state before/after, tail FNV, and the existing exact saved queue/TIMER
  scope. A corrupt tail rejects before either DB3 or receipt publication; no
  generic state, queue, or fallback action was introduced. Verification:
  `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# CSBWin queued LocalState=2 ParameterB receipt (2026-07-17)

- Completed the existing compact `DSA.cpp::PutState` LocalState=2 path with
  a source-bound runtime receipt. A committed type-47 DB3 `ParameterB` state
  transition now records its storage kind, state before/after, authenticated
  Extended Features tail FNV, and the exact restored TimerQueue/TIMER scope.
  The regression proves compact state one to two and proves a corrupted DSA
  RCS rejects before either DB3 or receipt publication. No widened ParameterB
  record, generic state machine, queue, or fallback opcode was added.
  Verification: `csb_v1_dsa_localstate1_save_handoff`,
  `csb_v1_dsa_queued_localstate2_timer`,
  `csb_v1_dsa_copy_runtime_handoff`,
  `csb_v1_dsa_pure_control_pc34_compat`, and
  `csb_v1_dsa_admitted_restored_timer_bridge` PASS; isolated `firestaff`
  build and `git diff --check` PASS.

# Nexus WARNING.BIN PP source-faithful execution (2026-07-17)

- Audited Sega Saturn/32X Graphic References ST-124-R1 section 6 and bound
  its PP contract to the canonical SHA-256-attested `WARNING.BIN` resource 0:
  a six-byte PP header, 256 big-endian BGR555 CLUT words, and one indexed byte
  per 240x96 image pixel. `nexus_v1_warning_dgt2_pp_execute()` rechecks every
  admitted source/prefix/body/trailing FNV and exact boundary, copies only the
  23,040 source index bytes and 256 unconverted BGR555 words to exact-size
  caller buffers, then requires an explicit presentation callback. Missing,
  mutated, undersized, or callback-rejected input publishes no execution
  receipt. The executor contains no default image, RGBA conversion, CLUT
  replacement, trailing-byte interpretation, or visual fallback. Verification:
  `nexus_v1_warning_dgt2_source_admission`,
  `nexus_v1_warning_dgt2_descriptor_admission`,
  `nexus_v1_warning_dgt2_pp_payload_admission`, and
  `nexus_v1_warning_dgt2_pp_execution` PASS against the local canonical file;
  isolated `firestaff` build PASS.

# Nexus WARNING.BIN PP M11 presentation (2026-07-17)

- Connected only canonical `WARNING.BIN` resource 0 to M11's real 320x200
  indexed presentation surface. Every warning-frame route rechecks the
  engine's exact source identity, reopens the original bytes, and rebuilds the
  source/descriptor/payload/execution receipt before writing the 240x96 index
  plane. The host palette is built only from the source's 256 BGR555 words in
  ST-124 section-6 order, `B4..B0/G4..G0/R4..R0`, using exact 5-to-6 bit
  replication for M11's RGB6 palette API. Body drift, a noncanonical source,
  an incorrect host surface size, or any receipt failure leaves the cleared
  frame untouched; the former title/UI-surface warning alternatives are not
  used. No synthetic texture, title/menu image, colour substitution, decoder,
  or visual fallback was added. Verification:
  `nexus_v1_warning_dgt2_source_admission`,
  `nexus_v1_warning_dgt2_descriptor_admission`,
  `nexus_v1_warning_dgt2_pp_payload_admission`,
  `nexus_v1_warning_dgt2_pp_execution`, and
  `nexus_v1_warning_dgt2_m11_presentation` PASS against the local canonical
  file; isolated `firestaff` build PASS.

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

# Nexus Structure1F direct directory admission (2026-07-17)

- Added `nexus_v1_structure1f_directory_admit()` on top of the existing DGN
  parser and direct LEV corpus identity route. It rehashes each canonical
  direct LEV00--LEV15 file, rejects source/package drift, and retains only the
  final Structure1F directory span plus its six parser-observed count-derived
  record spans, fixed record sizes, source tags and raw FNV witnesses. A
  mutated directory byte, changed size, out-of-range span or cross-level
  identity produces no receipt. The result is explicitly no-draw and grants no
  face, mesh, transform, material, texture, palette or host-placement meaning.
  Verification: `nexus_v1_structure1f_provenance` and
  `nexus_v1_structure1f_directory_admission` PASS against the local 16-level
  direct corpus; isolated `firestaff` build PASS.

# Nexus Structure1F first-family typed record admission (2026-07-17)

- Added a strict admission for the first parser-observed Structure1F family
  only: tag `0x10`, eight-byte rows, and the documented byte-ordered `x` and
  `y` fields at offsets 1 and 2. It retains the arithmetic `y * 64 + x` cell
  ordinal and copies bytes 3--7 as opaque data. The admission rehashes the
  source package, requires the existing direct-corpus directory identity and
  exact family span/FNV, and rejects cross-level identity reuse, altered row
  tags, and out-of-range row indices. It grants no item behavior, face, mesh,
  transform, material, texture, palette, decoder, or draw meaning.
  Verification: `nexus_v1_structure1f_directory_admission` and
  `nexus_v1_structure1f_item_admission` PASS against the local LEV00--LEV15
  corpus; isolated `firestaff` build PASS.
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

# Nexus Structure3 entry framing admission (2026-07-17)

- Added `nexus_v1_structure3_entry_admit()` above the existing direct-source
  Structure3 target receipt. It requires the rehashed ordinary file, level,
  package FNV, target FNV and 40-byte header to agree before retaining the raw
  tag, two counts, three contiguous count-bounded 12-byte spans and their FNV
  witnesses. Header-boundary, target, package or source drift rejects with no
  receipt. The admission remains explicitly no-draw and grants no geometry,
  normal, material, texture, transform, palette, pixel or rendering meaning.
  The fixture covers positive framing, target drift and a malformed boundary
  after the file identity is refreshed. Verification:
  `nexus_v1_structure3_target_admission_fixture` and
  `nexus_v1_structure3_entry_admission_fixture` PASS; full
  `build-nexus-codex` build PASS.

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

# DM2 CCM stream owner/grammar — the message loop (2026-07-20)

- Bound DM2_13e4_0982 (skproject/SKULLWIN/c_ai.cpp:5341-5647), the CCM
  message-loop body DM2_THINK_CREATURE runs for a living creature, as the
  bounded slice `dm2_v1_ccm_loop_pc34_compat`. The slice owns the stream
  grammar end to end: the savegame/aidef/0x13 pre-check with its
  adddata(4) payload skip, the !flag standalone DM2_4FCC (newly exported
  from the round-14 module via the shared dm2_v1_anim_4fcc_walk helper),
  the flag branch's b_1a = b_17 dance with the bound DM2_14cd_062e
  byte@0x12 head and the mode 6/7 facing write, the dying branch's
  data-backed table1d607e cloud gate (CREATE_CLOUD receipted, never
  simulated), the 0x32..0x34 setmticks special, the bound round-14 GAF,
  and the loop itself — the row byte@2 & 0x40/0x80 gates, the bound
  DM2_13e4_01a3 per-loop init (lazy v1e0584 + the v1e058d RAND16 draw on
  the session LCG), the DM2_PROCEED_CCM dispatch receipted through the
  proven DM2-005 matrix (handler bodies stay host-owned, fail-closed),
  and the bound DM2_50CB deterministic stream step whose result 2 breaks
  the loop while 0/1 continue it. The m_15785 end honors the v1e0570
  suppression before the delta, calls the bound round-14 1c9a_0a48, and
  delegates the re-queue to the round-13 end_requeue; the non-loop exits
  reach the m_15843 tail composed from the bound delete-timer +
  ticketed-enqueue primitives. New CTest `dm2_v1_ccm_loop_pc34_compat`
  (nine scenarios) PASS; new canonical companion test
  `dm2_v1_creature_something_real_data` admits the real GRAPHICS.DAT
  animation tables (dtRaw8/0xfb + dtRaw7/0xfc) for 57 creature types
  through the actual loader. dm2_v1 lane 216 tests, same 27 known
  baseline failures, zero new failures.

# DM2-002 tile record-link walk + XACT/timer-proc AI-stop callers (2026-07-20)

- Bound the DM2-002 tile record-link walk as a first-class bounded
  primitive in new module `dm2_v1_tile_record_walk_pc34_compat`:
  DM2_GET_TILE_RECORD_LINK (skproject/SKULLWIN/c_map.cpp:61-69, the
  bit-0x10 object flag + column-index ground-stack head over the proven
  loader binding) and the bounded next-link walk
  (c_record.cpp:54-57 — OBJECT_END_MARKER terminates, corrupt chains
  bounded by the declared record count, fail-closed). On top of it the
  two blocked AI-stop callers are now bound: DM2_PROCEED_XACT_85
  (c_ai.cpp:2078-2117) — the cell chain walk with the DB-index > 3
  break, the DB2 word@2 probe ending with slot byte@0x1e = 1,
  byte@0x1a = 59 and the source return -2, and the walk-end tail
  running the bound DM2_ai_13e4_0360 AI-stop (dir 0x13, argl0 1) before
  the unconditional byte@0x1a = 51 write and return -3; and
  DM2_ACTIVATE_CREATURE_KILLER (c_tim_proc.cpp:2907-2988) — the
  rectangular sweep with map-bounds skip, per-cell DM2_GET_CREATURE_AT,
  the DM2_1c9a_09b9 record word@8 filter, action 0xb mode-word
  semantics (0/1 skip, 2 bound AI-stop, above 2 aborts the sweep) and
  action 0x28 running the bound DM2_ATTACK_CREATURE with the 0x8000
  attack-word flag. New CTest `dm2_v1_tile_record_walk_pc34_compat`
  PASS. dm2_v1 lane 219 tests, same 27 known baseline failures, zero
  new failures.

# DM2 DELETE_CREATURE_RECORD mutating tail — tile-rooted cut + dealloc (2026-07-20)

- Bound the DM2_DELETE_CREATURE_RECORD mutating tail
  (skproject/SKULLWIN/c_record.cpp:1416-1424) as
  `dm2_v1_caii_delete_creature_record_tail` in the CAII module,
  composing with the round-12 decision head. The tile-rooted
  ground-stack cut (c_record.cpp:1419 — the DM2_MOVE_RECORD_TO x == -4
  skip00823/3CE7D path's observable end state, c_moverec.cpp:630-683)
  is bound through the proven record-pool list-cut semantics with a
  bounded membership pre-walk (corrupt chains cannot spin the splice),
  and the chain head is rewritten in the dungeon ground-stack table
  through the new loader setter `dm2_v1_dungeon_set_first_thing`
  (byte-square 0x10-flag cells only; additive, loader tests unchanged).
  DM2_DEALLOC_RECORD (c_record.cpp:1205-1208) is bound: record word@0
  becomes the 0xffff free marker. The 3CE7D timer/text side effects and
  recursive DM2_1c9a_0fcb inside the cut, DM2_DROP_CREATURE_POSSESSION
  and the DM2_1c9a_0247 tagged-dballoc cleanup stay unbound behind
  named receipts, never simulated. New CTest
  `dm2_v1_delete_creature_tail_pc34_compat` PASS. dm2_v1 lane 220
  tests, same 27 known baseline failures, zero new failures.

# DM2 DROP_CREATURE_POSSESSION — generated drops + possession walk (2026-07-21)

- Bound DM2_DROP_CREATURE_POSSESSION
  (skproject/SKULLWIN/c_record.cpp:1537-1752) as a bounded slice in new
  module `dm2_v1_drop_possession_pc34_compat`, retiring the delete
  tail's possession-drop receipt. The mode == 0 generated-drops loop
  (GDAT CREATURES drop fields 0x0A..0x14) delegates to the proven
  `dm2_v1_drops_place_source_slots` binding with the destination head
  now tile-rooted in the dungeon ground-stack table; the possession
  chain walk prefetches each next link before the move, randomizes item
  direction bits when the creature's AI flags bit0 is clear
  ((party_dir + RANDBIT) & 3 on the party cell, RANDDIR elsewhere,
  folded into the handle), appends DB != 0x0e items to the drop cell,
  and deallocates DB 0x0e records (word@0 = 0xffff). Fail-closed:
  flag-less drop cells, corrupt destination chains (bounded end
  pre-walk), unwired AI flags (stops before the first RNG draw),
  missing LCG. DM2_QUEUE_NOISE_GEN2 stays host-owned, receipted. New
  CTest `dm2_v1_drop_possession_pc34_compat` PASS. dm2_v1 lane 221
  tests, same 27 known baseline failures, zero new failures.

# DM2 INVOKE_MESSAGE + complete DELETE_CREATURE_RECORD composition (2026-07-21)

- Bound DM2_INVOKE_MESSAGE (skproject/SKULLWIN/c_tim_proc.cpp:4332-4367)
  as a bounded slice in new module
  `dm2_v1_invoke_message_pc34_compat`: setmticks (c_timer.h:66),
  settype 0x4, the RG3UW actor mapping (0->1, 1->3, 2->2, else the
  c_tim init default 0), setxyA/setxyB (c_timer.h:82,90), and the
  ticketed DM2_QUEUE_TIMER enqueue. The message dispatch at processing
  stays host-owned.
- Bound the COMPLETE DM2_DELETE_CREATURE_RECORD
  (skproject/SKULLWIN/c_record.cpp:1357-1425) as a source-ordered
  composition in new module `dm2_v1_delete_creature_full_pc34_compat`,
  wiring the bound drop into the delete tail: GET_CREATURE_AT early
  return; jz_test8 AI gate + table1d607e[GDAT word@1] &4 probe
  data-backed through the caii module's new read-only provider/table
  accessors; the word@0xc decode + receipted map swap + bound
  DM2_INVOKE_MESSAGE(x, y, 0, 0, gametick + 1); the CAII slot byte@1a
  clear; the tile-rooted cut (membership pre-walk before any
  mutation); the bound DM2_DROP_CREATURE_POSSESSION (a fail-closed
  drop skips the dealloc); DM2_1c9a_0247 receipted host-owned;
  DM2_DEALLOC_RECORD bound. Gate-open with unknown/out-of-span GDAT
  word@1 fails closed before any mutation. New CTest
  `dm2_v1_delete_creature_full_pc34_compat` PASS. dm2_v1 lane 222
  tests, same 27 known baseline failures, zero new failures.

# DM2 0fcb branch wired to the complete DELETE_CREATURE_RECORD composition (2026-07-21)

- Wired the DM2_1c9a_0fcb record-delete branch
  (skproject/SKULLWIN/c_1c9a.cpp:5956-5957) to the COMPLETE
  DM2_DELETE_CREATURE_RECORD composition through a session-owned hook
  (`dm2_v1_caii_set_delete_creature_full_fn`), keeping the caii
  module's link boundary; when wired, `dm2_v1_caii_free_slot` runs the
  composition (source call DM2_DELETE_CREATURE_RECORD(x, y, 0, 1))
  instead of the standalone decision head, which remains as the
  unwired fallback. New runtime session/test-support accessor
  `dm2_v1_runtime_caii_set_slot_mode_byte` mirrors the source's
  slot-mode writers. CTest `dm2_v1_caii_free_runtime_pc34_compat`
  extended: full lifecycle through the runtime boundary — activation,
  dying-mode slot, branch taken data-backed, composition end-to-end
  (invoke timer queued, cut, drop, dealloc), no think timer afterwards.
  18/18 PASS. dm2_v1 lane 222 tests, same 27 known baseline failures,
  zero new failures.

# DM2 0fcb delete composition production-wired in the runtime (2026-07-21)

- Wired the COMPLETE DM2_DELETE_CREATURE_RECORD composition into
  dm2_v1_runtime.c itself: dm2_runtime_ensure_think_binding now wires
  the session-owned hook (dm2_runtime_delete_creature_full), so the
  runtime boundary runs the composition (source call
  DM2_DELETE_CREATURE_RECORD(x, y, 0, 1), c_1c9a.cpp:5956-5957)
  without any test-side wiring.  The hook uses the runtime session's
  tick counter, party accessors, a session-owned DropRng, and the
  creature module's GDAT drop-word accessors.  New read-only accessor
  dm2_v1_runtime_last_delete_full_receipt.  The composition sources
  were added to all 22 remaining build targets compiling
  dm2_v1_runtime.c.  CTest dm2_v1_caii_free_runtime_pc34_compat
  verifies the production wiring (18/18 PASS).  Full project rebuild
  clean; dm2_v1 lane 222 tests, 19 environment baseline failures
  (missing game assets), zero failures in any CAII/delete/drop test.

# DM2 CAII activation call sites bound + runtime floor-mecha wiring (2026-07-21)

- Bound the two direct DM2_ALLOC_CAII_TO_CREATURE activation call sites
  as CAII-module slices: dm2_v1_caii_animate_activation
  (DM2_ANIMATE_CREATURE, c_tim_proc.cpp:2859-2900 — flags bit0 SET AND
  record byte@5 == 0xff allocates; GET_CREATURE_AT early return;
  AI-spec flags data-backed through the wired provider; the CCM tail
  PREPARE/UNPREPARE_LOCAL_CREATURE_VAR + DM2_ai_13e4_0806/071b stays
  host-owned, receipted) and dm2_v1_caii_moverec_activation
  (DM2_moverec_3CE7D, c_moverec.cpp:960-985 — byte@5 != 0xff updates
  the pending think timer IN PLACE; byte@5 == 0xff with flags bit0
  CLEAR allocates — the OPPOSITE gate of the animate site;
  SET_MINION_RECENT_OPEN_DOOR_LOCATION stays host-owned).  New timeline
  primitive dm2_v1_source_timer_update_payload binds the source's
  setxyA(x, y) + setmticks(map, getticks()) in-place payload update
  (c_moverec.cpp:977-978, c_timer.h:66/82) over the session ticket,
  preserving queue order.  Runtime wiring: the 0x04 actuator dispatch
  reads the square class through a bound tile_class_at provider
  (dm2_v1_dungeon_get_square_type, c_tim_proc.cpp:4283-4287) and square
  class 1 runs a bounded DM2_ACTUATE_FLOOR_MECHA chain walk
  (c_tim_proc.cpp:3009-3532 + 4297-4299) whose DB3 type-0x3a records
  fire the animate activation; a DB > 3 chain link takes the source's
  whole-function return, corrupt chains fail closed bounded, all other
  record types stay host-owned.  Session receipt published through
  dm2_v1_runtime_floor_mecha_receipt.  New test
  dm2_v1_caii_activation_sites_pc34_compat (all checks passed: both
  gates, the in-place payload update, no-op/stale-ticket fail-closed
  paths, unknown-provenance fail-closed).  Full project rebuild clean;
  dm2_v1 lane 223 tests, 19 environment baseline failures (missing
  game assets), zero new failures.
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

# DM1 F0267 local floor-sensor rotation (2026-07-22)

- F0270 now carries G0403-G0406 as the triggering floor square and
  CM1_CELL_ANY. After the complete F0276 pass, F0271 rotates the original
  sensor chain once, preserving both decoded and raw `Next` words.
- The focused F0267 regression verifies ordered CLEAR/TOGGLE local effects,
  raw-chain persistence, and the final local receipt.

# CSB F0275 C004 typed hand removal (2026-07-22)

- C004 now has its own matching-hand path: it consumes the original hand
  object and queues the normal F0272/F0261 target effect without borrowing
  C011/C017's final-same-cell condition.
- The focused runtime regression covers a later C03 in the same cell and
  verifies that neither sensor is rotated or removed.

# DM1 original-save span operations (2026-07-22)

- F0415/F0416 now provide fail-closed bounded read/write spans. F0421/F0422
  consume those operations before updating their original running checksum,
  so rejected spans leave cursor, bytes, and checksum unchanged.
- The original-save handoff regression passes. The configured local DM1 corpus
  contains no PC34 saves, so the optional real-save leg is correctly skipped.

# CSB title-to-Entrance runtime handoff (2026-07-22)

- M11 now captures the C001 STRIKES BACK sample at source frame 100, after
  the complete CHAOS hold, rather than the obsolete frame-80 boundary. The
  release-app receipt consequently remains valid for the real title session.
- When the terminal title tick hands control to ENTRANCE.C, M11 consumes that
  tick before accepting the first Entrance plan. This prevents the former
  black-screen rejection at the title/Entrance boundary.
- The real local CSB M12-to-M11 handoff regression passes 500/500.

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

# Theron V1 source-locked CD-DA track routing receipt (Lane E, cycle 10)

Closed TODO.md item (5) under the 2026-07-11 Theron original-media
synthetic-path audit: implemented a source-locked CD audio track routing
receipt that gates any future Theron V1 audio output on original CUE
metadata and locally staged CD-DA tracks.


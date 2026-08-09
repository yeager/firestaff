# Firestaff TODO - DM1

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Current DM1 Follow-up

- 2026-08-09 real nested-data resolution is complete: the runtime and launcher
  asset-status fast path recognize the authenticated DOS PC 3.4 `DATA` tree
  below both `/Users/bosse/.firestaff/data` and a per-game DM1 root. The C015
  bottom-row probe now narrows to that real data path, feeds source-owned DM1
  text state, and verifies that host message telemetry is not rendered. This
  closes a data-discovery defect; it does not close the authentic C13-save or
  original-pixel capture gates.

- 2026-08-09 Downloads audit found two distinct authentic PC 3.4 `DMSAVE.DAT`
  files (both checksum-valid and F7057-envelope-valid). The second completes
  a 15-event/15-active-group in-memory compatibility roundtrip. Neither file
  has an adjacent operator provenance sidecar, and no C13 event count is
  promoted from this discovery; the authenticated C13-save gate remains open.

- 2026-08-09 release-capture validation now rejects negative, zero, and
  overflowing framebuffer dimensions with a 64-bit size check. This closes a
  metadata-safety defect only; it does not create or admit missing original
  pixels.

- 2026-08-09 HoC runtime-probe path resolution is complete: the no-creature-AI
  guard now selects the authenticated DOS PC 3.4 `DATA` tree from a broad
  `/Users/bosse/.firestaff/data` root and from a per-game root. The real-data
  probe passes 4/4; this removes a false discovery/scan failure and does not
  claim C13 or original-pixel parity.

- 2026-08-09 creature-AI ownership wording is corrected: live DM1 behavior is
  owned by the M10 source-ordered F0209 timeline; the M11 map-scan routine is
  retained only for isolated diagnostic worlds without an admitted source
  timeline. This removes a misleading "M10 no-op" description and does not
  claim broader creature-combat parity.

- 2026-08-09 verification-path repair is complete: pass76 route-state JSON is
  stored under `parity-evidence/verification`, and pass608/live-row checks no
  longer depend on the removed historical `verification-m11` path. This fixes
  the gate wiring only; it does not promote original-vs-Firestaff parity.

- 2026-08-09 original-frame classifier correction is complete: authentic
  gameplay with the source cyan movement strip is no longer misclassified as
  the entrance menu, while green ENTER/RESUME/QUIT controls over the same
  corridor remain blocked. This improves capture diagnosis only; it does not
  promote duplicate or semantically incomplete original frames. Authentic
  C13-save and full original pixel-capture gates remain open.

- 2026-08-09 F0143 source-ownership correction is complete: the live M11
  armour-defense path now resolves G0239 values through the authenticated raw
  ARMOUR Thing record from DUNGEON.DAT. A missing or malformed record no
  longer falls through to a parallel M11 table. Original C13-save and
  original launcher/panel pixel-capture gates remain open.

- 2026-08-09 tick-orchestrator F0143 source-ownership correction is complete:
  the live combat tick now uses the same raw ARMOUR Thing record and no longer
  carries a second subtype table. Original C13-save and original
  launcher/panel pixel-capture gates remain open.

- 2026-08-09 live C13 producer correction is complete: Vi Altar drops now
  derive the rebirth owner from the authenticated bones JUNK record, preserve
  its Thing cell, and publish the F0255 step-2 fields (`aux0`, `aux1`, and
  `aux4`). Ownerless/stale icon matches stay on the normal drop path, and a
  full or rejecting timeline restores the hand item. This fixes the runtime
  producer; the authentic C13-bearing save and original launcher/panel capture
  gates remain open.

- 2026-08-09 C13 trigger classification correction is complete: the live
  click route now resolves the current map's wall ornament through the
  authenticated DUNGEON.DAT table and G0192 ordering before allowing a Vi
  Altar rebirth. Square and arched alcoves remain ordinary storage targets.
  The authentic C13-bearing save and original launcher/panel capture gates
  remain open.

- 2026-08-09 C13 save-identity correction is complete: the world hash is now
  republished after the source-owned rebirth event is appended, closing the
  immediate-save window between the bones drop and the next tick. Authentic
  C13-save and original pixel-capture gates remain open.

- 2026-08-09 C012 receipt correction is complete: the generator's source icon
  index now survives F0275 result initialization and reaches the authentic
  F0167 allocator. A focused regression covers both the positive empty-hand
  route and the occupied-hand rejection.

- 2026-08-09 F0275 wall-object mutation is complete: C012 generators, C013
  storage take/store, and C016 exchangers now update the real Thing chain and
  leader hand transactionally. Failed links or allocations roll back and do
  not publish a remote effect. The authentic C13-bearing save and original
  launcher/panel pixel-capture gates remain open.

- 2026-08-09 real-data root audit: the complete `/Users/bosse/.firestaff/data`
  scan reports all five games `READY`, and DM1 auto-launch reaches runtime
  from authenticated FM Towns English data. The archive-backed scan can take
  tens of seconds; an intermediate `SCANNING GAME DATA` screen is not a
  missing-data result. Remaining DM1 completion gates are source/capture
  requirements, notably original HoC/Mac pixel capture and an authenticated
  C13-bearing PC34 save.

- 2026-08-09 startup save-census correction is complete: classifier discovery
  no longer emits false roundtrip failures without an original `DUNGEON.DAT`
  owner. Keep the backed PC34 roundtrip and remaining C13/original-capture
  gates open until their real evidence is present.

- 2026-08-09 resume atomicity correction is complete: file and byte imports
  validate the DM1 resume receipt while the candidate world is still isolated.
  A rejected receipt therefore cannot replace the active world or leave a
  half-applied resume state. The remaining resume work is authentic C13 and
  original-pixel evidence, not another save-state fallback.

- 2026-08-09 mixed-root resolver correction is complete: when the real DM1
  root contains both extracted DOS PC 3.4 files and archive members for other
  editions, the runtime now selects the hash-verified loose DOS `DATA` pair
  before recursive archive lookup. This prevents a virtual sibling edition
  from owning a PC34 launch; no data is copied or synthesized by this route.

- 2026-08-09 C15/C25 ownership cleanup is complete: if an authenticated
  persistent C25 update loses its source C15 record, the stale C15 is removed
  before C25 despawn. This prevents a later F0115 scan from exposing an
  orphaned source explosion. The focused F0213/F0220, F0248, and F0190
  runtime checks pass.

- 2026-08-09 conditional top-row material correction is complete: active
  F0659 shield borders and F0662 invisibility remaps now fail closed when
  their real GRAPHICS.DAT material is absent; ordinary frames do not require
  either conditional surface. This prevents an incomplete HUD frame from
  being admitted as parity-complete.

- 2026-08-09 top-row atomicity correction is complete: a rejected source
  receipt now clears the DM1 HUD zones and stops; it cannot fall through to
  the legacy partial painter. Status-bar receipt failure is treated the same
  way, so an incomplete source frame is never published as a valid HUD.

- 2026-08-09 movement-panel material correction is complete: the host input
  feedback cue is now emitted only after the original C013 movement-arrow
  surface has been loaded and blitted. Missing source material remains black.

- 2026-08-09 action/spell paint correction is complete: the active F0394
  spell route now reports C009/C011 blit failure to the final-paint gate;
  runtime capture cannot be admitted for a frame whose original panel was
  not actually painted. The 33 focused action/spell lifecycle and material
  tests pass.

- DM1 F0115/F0128 now has an M10 source-material scheduler for normal
  objects, projectiles, and explosions. It requires admitted PC34 surfaces
  and preserves the source object -> projectile -> explosion restart order.
  Real multi-effect corpus capture remains open; no missing surface may be
  replaced by a marker or generated sprite.

- F0135/F0732/F0733/F0735 now have bounded planar dispatch coverage. The DM1
  viewport has a fail-closed F0134/F0135 material consumer that accepts only a
  verified original planar surface; keep title, HUD, and final viewport
  composition open until their real-data render consumers are independently
  verified. PANEL.C F0344 food/water bars now consume that admitted material
  with the source two-pixel F0135 shadow order and original warning colors;
  remaining callers still need their own source-material bindings.

- DM1 PANEL.C F0344/F0351 champion health/stamina rows are now fail-closed
  against the session-bound M653 font and admitted C020 panel. Keep original
  pixel capture of the complete inventory-stat panel open; missing or host
  font material must leave the source panel without generated text.

  - 2026-07-31 F0351 layout repair: empty-hand eye inspection now remains in
    C101 rather than opening the generic M11 dialog. Skill and statistic rows
    use M653's visible six-pixel cell advance, so their source C557/C559
    coordinates fit the 144-pixel panel. The remaining work is an original
    pixel capture comparison, not host-font layout recovery.

  - 2026-07-31 C101 containment regression: the real-PC34 inventory runtime
    now compares inactive and F0351-active framebuffers and proves that even
    the longest `ANTI-MAGIC`/`ANTI-FIRE` rows do not alter pixels outside the
    original 144-pixel C101 panel. This closes the reported oversized-text
    regression boundary; original DOS pixel comparison remains open.

- F0134 now has a bounded planar bitmap-fill implementation and a source-owned
  viewport material consumer. Keep wider production caller mapping open; no
  renderer may substitute a generated surface when material admission fails.

- DM1 F0134/F0135 champion food/water/status-box material admission is closed:
  the live F0345 C020/C030/C031 panel transaction now requires exact decoded
  original surfaces and records their pixel fingerprints. Remaining F0134/F0135
  work is wider viewport/title production-caller mapping, not champion-panel
  fallback rendering.

- CSB C009 now scans only the hash-verified original PC34 `DUNGEON.DAT` for an
  unambiguous fakewall SET route and rejects arbitrary parser-valid input. No
  local original corpus has produced a positive run. Keep remaining sensor
  types, removal paths, and DSA interactions open.

- CSB C012 now admits only hash-verified original PC34 `DUNGEON.DAT`; F0167
  allocation failure is fail-closed and cannot rotate the source cell or queue
  F0272/F0268. No local original corpus has produced a positive run. Keep the
  broader F0167/F0275 semantics open.

- G0378/G0379 have event-context evidence only; retain original global ABI
  layout and wider group routing as open work.

- G0381 has event-local behavior evidence only; retain original global ABI
  layout and wider creature-combat routing as open work.

- G0382 has event-local behavior evidence only; retain original global ABI
  layout and wider creature-combat routing as open work.

- F0007/F0008 have C11 primitive coverage, but their source callsites still
  need ownership mapping before they are treated as complete runtime routes.

- F0009/F0010 have C11 primitive coverage, but their source callsites still
  need ownership mapping before they are treated as complete runtime routes.

- `G0374_l_WatchdogTime_CPSE` is an explicit NOCOPYPROTECTION boundary, not a
  gameplay ownership gap; `G0369`, `G0372`, and `G0373` are source-audited
  storage contracts.

- Broaden live original-save corpus coverage and consume the validated
  SquareFirstThings/timeline receipt in the complete runtime transition.

- Obtain an operator-staged real PC34 corpus for a positive run of the
  An operator-supplied
  external corpus is still required for positive
  evidence. `FIRESTAFF_DM1_PC34_SAVE_CORPUS` is currently unavailable.
  payload identity through F0435 staging and adoption; C050 fluxcage remains

- Consume the action/spell feedback-frame receipt in M11 before final HUD
  paint commands are accepted.

- Admit HoC runtime clear/portrait commands only from matching apply
  presentation completion receipts, then consume them in M11.

- Consume active top-row/action host lifecycle receipts in M11 with original
  material proof and without fallback drawing.

- Perform real-data runtime capture of the now source-gated C13/HoC/top-row/
  action paths; no substitute material may be introduced during capture.

- Exercise the final M11 source-capture receipt with a configured real DM1
  corpus; missing/stale routes must remain capture-ineligible.

- Configure a real PC34 save corpus to exercise byte-identity capture; local
  DM1 data currently contains no original PC34 save.

- Capture configured original C13, HoC, champion, and action routes from a
  real runtime session; the combined gate is implemented and tested.

  - v1_viewport_wall_blit_transparency_gate (C10 owner =
    dm1_v1_viewport_3d_pc34_compat.c handoff.transparent_color = 10)

  - v1_viewport_field_zone_aspect_clip_gate (s_fieldRenderPlans in
    dm1_v1_field_teleporter_effect_pc34_compat.c)

  - v1_viewport_side_wall_ornament_source_gate (F0107 flip/native-offset
    in dm1_v1_wall_ornament_pc34_compat.c)

  - v1_door_button_ornament_coordinates_gate (G0208 frames in
    dm1_v1_viewport_3d_pc34_compat.c; G0207/G0200/G0201 in
    dm1_v1_door_ornament_render_pc34_compat.c)

  - v1_wall_ornament_coordinates_gate (G0194/G0205 DM1 table modules)

  - firestaff_dm1_v1_viewport_d0c_door_edge_ornament_gate_probe (stale
    probe expectation; DUNVIEW.C:8189-8214 gates the C09 gold hole blit
    on Event73Count_ThievesEye)
  The round-22 STILL-OPEN list was fully triaged in round 23 (see the
  round-23 entry above; three fixed, the rest classified/diagnosed).

  - `nexus_v1_startup_menu_pc34_compat` (#1741) — FIXED in round 21
    (see DONE.md same-date entry): three class-(a) fixture gaps
    (BPK trailer provenance, clobbered SFX seed, missing engine in the
    title-route block).

  - `nexus_v1_m11_launcher_handoff_boundary` (#674) — FIXED in round 22
    (see DONE.md 2026-07-22 entry). The real `TITLE.CG` reveal now
    remains drawable while `MENU.BPK` stays fail-closed awaiting PRS3
    capture, and ACCEPT exits a completed title instead of trapping on
    a blocked menu route. Verified passing against the local retail ISO
    (151 passed, 0 failed, 0 skipped). Remaining capture/data-bound
    gates are still intentionally blocked: (1) STABG.BIN cell decode
    semantics (`nexus_ui_load_stabg` stays inert until original Saturn
    evidence proves pixel order), (2) FACE.BIN PRS3 portrait decode
    (acc5abbc6 / 11c856653 / bc102aa4b ledger), and (3) title
    capture-surface + Saturn timing/frame capture evidence.

  - `nexus_v1_track1_phase_launch_extracted_root` (#1919, 57 PASS / 3
    FAIL) — class (c): the three FAILs ("startup FACE.BIN loaded all
    roster portraits", "... without portrait fallbacks", "... receipt
    is ready") require decoding real PRS3 portrait records, but
    `nexus_ui_expand_face_record_48x48` deliberately blocks PRS3 until
    its opcode grammar is proven ("Keeping this blocked protects the
    real portrait route"). Blocked on the ledgered FACE PRS3 capture
    campaign (acc5abbc6 corpus, 11c856653 targets, bc102aa4b ledger).
    NOT a fixture issue — do not relax the probe assertions.

  - `nexus_v1_track1_phase_launch_saturn_ja_iso` (#1920) — real Japan
    ISO data is now staged under `~/.firestaff/data/nexus-extras/saturn-ja`
    and the test runs against it; the three FAILs are the same FACE.BIN
    PRS3 portrait block as #1919, not missing ISO media. Do not relax
    the probe assertions.

  - Stale PRS3 placement/VDP1 test fixtures fixed (this pass):
    `nexus_v1_prs3_dgn_placement_adapter` (#1959),
    `nexus_v1_prs3_vdp1_capture_replay` (#1962), and
    `nexus_v1_prs3_placement_engine_ingress` (#1964) all failed because
    the engine's external PRS3 placement receipt now requires a nonzero
    `trace_size`; the tests only set `trace_fnv1a64`. Added `trace_size`
    to each fixture. These are now green.

  - needs per-probe side-band/frame recalibration (pose-specific
    thresholds calibrated to the old (1,2) corridor view):
    portrait_06/17_inventory_exit_restore,
    portrait_19/22_wall_ornament_no_float,
    portrait_00_d1r_no_portrait_192_gate,
    portrait_00_d2r_negative_072_gate,
    ordinal_2/6_d2l_negative, portrait_09_d1l_no_portrait_273_gate.

  - multi-pose walkpath/entrance remapping: portrait_12/22/08
    walkpath_from_entrance, portrait_07_walkpath_from_stairs,
    east_walkpath family (01/02/03/06/07/12, ordinal21),
    portrait_12/22 screenshot_receipt, portrait_12/22
    front_south_entry, portrait_01/21 front_east_entry.

  - interaction families needing remapped anchors:
    after_party_shuffle 02/07/09/11/14, approach_from_right
    01/04/05/18/22, approach_from_left 0/17, resurrects/reselect
    00/11/22, reincarnate_reselect 18, d2c_far_positive 01/11/22,
    portrait_22_input_focus_restore_022_gate,
    portrait_20_turn_away_return, portrait_05_south_return,
    champion_mirror_portrait_rect_south_return,
    portrait14 south_return, hoc_all_portraits_wall_coordinate_gate.

  - passing-vacuously probes worth tightening later (not urgent):
  different edge profile in the C346 bitmap; needs ReDMCSB/bitmap
  evidence, not a pose swap; both reverted to HEAD).
  Remaining 42, grouped for round 4:

  - approach family scan-geometry remapping (multi-pose loops, not
    single anchors): approach_from_right 04/05/18/22,
    approach_from_left 0/17.

  - resurrect/reselect 00/11/22, reincarnate_reselect 18.

  - d2c_far_positive 01/11/22 — far-view sensitive; lock only after
    the open (17,9)W far-view D1C question is answered.

  - portrait_22_input_focus_restore_022_gate,
    portrait_20_turn_away_return, portrait_05_south_return,
    champion_mirror_portrait_rect_south_return,
    portrait14 south_return, hoc_all_portraits_wall_coordinate_gate.

  - multi-pose walkpath/entrance remapping: portrait_12/22/08
    walkpath_from_entrance, portrait_07_walkpath_from_stairs,
    east_walkpath family (01/02/03/06/07/12, ordinal21),
    portrait_12/22 screenshot_receipt, portrait_12/22
    front_south_entry, portrait_01/21 front_east_entry.

  - door_nearby_no_float 02/06 (same C346 frame-edge class as the
    parked wall_ornament probes — verify before assuming pose-fix).

  - still passing-vacuously (tighten later):
  sensor cell at (7,8) south wall; 11 seeds the shipped
  sensorData=1 sensor to 11 as before (coordinate-agnostic helper).
  Remaining 13, grouped for round 6:

  - reincarnate_reselect 18 (PARKED — needs runtime triage of the
    F0282 C165 champion-slot flow, NOT pose-fixable).

  - portrait_06/17_inventory_exit_restore (parked: C040
    panel-survival behaviour contract, BUG-120/121 guards).

  - portrait_19/22_wall_ornament_no_float + door_nearby_no_float
    02/06 (C346 frame-edge signature class — needs ReDMCSB/bitmap
    evidence, not a pose swap).

  - portrait_12/22 front_south_entry, portrait_01/21
    front_east_entry, portrait_12/22 screenshot_receipt.

  - still passing-vacuously (tighten later):
  m11_game_view.c).  Both need engine/contract implementation, not
  probe re-basing.
  Remaining for round 24 (round-23 update, see below):
  renders at 100% match, but all five remain red on the previously

  - dm1_v1_movement_source_lock: different failure family —
    memory_tick_orchestrator_pc34_compat.c:F0888 disabled-movement
    gate probe expects the text 'movement_command_disabled_redmcsb_compat';
    triage whether the orchestrator symbol was renamed
    (stale re-anchor) or the seam was restructured.

  - the remaining sweep failures outside the movement family
  side-contents structure and the pre-c8ab48a2a kSideBlits swap —
  restoring them means reconciling with the F0115 square order and
  the receipt-based side-wall architecture, not a token tweak.
  pass505_dm1_v1_blocked_movement_collision_timing_gap re-anchored

  - 2026-07-17 update: the local G8 FIFO-output capture schema now requires
    the exact sequence-4 callsite, G8 READ(6), and one ordered FIFO-origin
    game-RAM output row while rejecting a consumer claim. The remaining
    instrumentation gap is an original Mednafen run that emits this row for
    G8; the schema fixture is metadata-only and is not corpus evidence.

  - 2026-07-17 update: the local Mednafen patch now emits the opt-in G8 marker
    only when `FIRESTAFF_THERON_G8_FIFO_OUTPUT_TRACE=1` and the existing
    FIFO-origin hook observes generation 8/LBA 4859/dispatch 4. The operator
    planner refuses missing binaries, CUE files, or an existing output path.
    A real direct CUE/BIN run remains required; no trace row was generated.

  - 2026-07-17 update: an immutable parsed G8 sidecar can now join only an
    already-ready opaque artifact corpus and the current M11 source-trace MD5
    under one media scan epoch. The receipt remains capture-required/no-draw,
    retains no payload bytes, and clears on trace or epoch drift. This is not
    an artifact import, loader-output consumer, route, or render promotion;
    a real direct CUE/BIN G8 output/consumer capture remains required.

  - 2026-07-17 update: the G8 sidecar/binding now retains only the observed
    FIFO row's offset, reader-PC, logical and physical destinations, writer
    PCs, and byte value under a rechecked capture fingerprint. Address-window,
    source-trace, lifecycle, corpus, plan, or retained-row drift rejects. This
    remains opaque capture-required/no-draw data, not a consumer claim or a
    basis for route, bitmap, palette, decoder, or render promotion.

  - 2026-07-17 update: the verifier's exact one-row G8 capture now carries a
    complete sequence/length/window identity: the observed FIFO sequence is
    both bounds, capture length is one byte, and the source window is
    `[source_offset, source_offset + 1)`. Its identity is rechecked through
    the artifact/corpus/capture-plan and M11 capture-required binding. Any
    sequence, length, window, source trace, lifecycle, corpus, or plan drift
    rejects; the missing original output/consumer chain remains the next
    admissible evidence.

  - 2026-07-17 update: the exact capture file now has an explicit canonical
    MD5, full-file FNV-1a, and one-row count bound into one file identity.
    M11 rechecks that identity as capture-required lifecycle metadata beside
    the current source-trace MD5, artifact corpus, and capture-plan identity.
    File hash, FNV, row-count, source-trace, corpus, plan, or lifecycle drift
    rejects; it does not make the row a consumer, route, or bitmap source.

  - 2026-07-17 update: that canonical capture-file identity is now also bound
    to the disassembled G8 sequence-4 dispatch (`3840`/`1f1840`, `20/ff/04`)
    and exact READ(6) CDB `08 00 12 fb 01 00` for LBA `4859`, one sector.
    M11 rechecks this capture-CDB identity with the current source trace,
    artifact corpus, and capture plan, clearing capture-required evidence on
    any callsite, CDB, source-trace, corpus, plan, or lifecycle drift. This
    remains metadata-only and cannot claim a consumer, route, or bitmap.

  - 2026-07-17 update: the verified `0x0b52` loader-output admission and the
    G8 FIFO witness can now join one current live-handoff receipt only when
    artifact-corpus plan identity, Track02 MD5, source-trace MD5, dynamic
    CD_READ ownership, and scan epoch all agree. The two records remain
    distinct opaque observations; bitmap/object, decode, draw, and fallback
    permissions must all remain false. This is capture-required/no-draw
    lifecycle metadata, not a G8 consumer, payload, route, or bitmap claim.

  - 2026-07-27 update: capture replay can now supply an explicitly recorded,
    frame-indexed PCE input plan when macOS cannot deliver physical keyboard
    events to the unbundled SDL process. The active Mednafen producer retains
    every replay pulse separately from host input and restores the physical
    input buffer after each emulated frame. The generic CD-state trace remains
    capped, but `$e009` enter/data-read/return rows now have their own bounded
    critical lane. An authentic US-CUE run with `run@480,run@720,i@1200`
    observed 37 main-RAM `$e009` windows and no active-window CD data reads.
    Replay is input automation only; it does not make the input, the media,
    or any level/object/bitmap/palette semantics synthetic, and it cannot
    promote a decoder, route, or drawing path. The next required evidence is
    still an original data-read/consumer chain from a relevant window.

  - 2026-07-27 update: the critical lane now also records PCE-CD control-port
    writes only while a main-RAM `$e009` call is active. An authentic US-CUE
    replay with the same three input pulses observed six windows, 117 bounded
    writes, and zero active-window FIFO/data-register reads. The writers are
    System Card control PCs (`$e90d`, `$e947`, `$e981`, `$ea3a`, `$ea34`),
    which establishes the real call-bounded CD-control path but no game-owned
    destination or record consumer. The `$3840` calls still resume nonlocally
    at `$3b36`; no level, object, bitmap, palette, or route semantics are
    assigned. Required next evidence remains an original FIFO-to-game-RAM
    transfer joined to a game-owned consumer.

  - 2026-07-17 update: M11 now rechecks the admitted later record against the
    current direct layout, capture plan, media scan epoch, and active replay
    tail. The resolved record must still be the replay's final record/raw
    sector and retain the same opaque user-data hash and observed sector

  - 2026-07-17 update: the admitted later-record candidate can now join the
    current dungeon-handoff plan target and opaque artifact corpus only when
    its record/raw sector, destination identity, replay tail, Track02 MD5,
    source-trace MD5, layout epoch, and plan identity agree. The retained
    result is one capture-required/no-draw coordinate receipt; it assigns no
    record family grammar, level/object semantics, bitmap/palette data, route,
    pixel, draw, or fallback behavior.
    checksum, otherwise no-draw readiness clears. The outstanding blocker is
    unchanged: an original positive CUE/BIN plus observed loader trace, not
    guessed record or level semantics.

  - 2026-07-17 update: a hash-first direct corpus discoverer now accepts only
    one explicit pair of regular CUE and coalesced loader-trace files. It
    reuses raw-media MD5 intake and the existing sector-record admission,
    retains only the trace MD5 plus opaque receipt, and passes READY through
    the M11 no-draw bridge. Absent files SKIP safely; virtual, malformed,
    ambiguous, or non-admitted candidates cannot produce readiness.

  - 2026-07-17 update: the normalized loader receipt now joins the closed
    HuC6280 loader/consumer event log before the existing opaque runtime route
    is marked ready. The next positive step remains an original observation
    chain; no fixture, inferred record format, bitmap or object data can
    substitute for it.

  - 2026-07-17 update: M11 now admits the opaque observed route only as the
    exact Soul Room epoch 1 then dungeon-handoff epoch 2 sequence under one
    stable converted source-trace MD5. Bad ordering, epoch, media identity or
    trace identity clears both route flags. Remaining work is still original
    graphics/object evidence; this lifecycle never enables a draw path.

  - 2026-07-27 update: the actual Launch action now selects a compatible
    hash-matched Track 02 version before this direct-media admission and then
    rebuilds the raw CUE IPL receipt from that selected payload. A real US
    19-track CUE passes admission, preserves its INDEX 01 pregap, and reaches
    the existing M11 handoff boundary. Remaining work is positive live
    Track-02 dungeon semantics/capture, not launcher admission.
  Remaining work is a user-supplied original direct Track 02 plus the three
  independently attested capture bundles. Do not promote virtual archive
  evidence, infer media layouts, decode level/object data, or create media
  copies while that source evidence is absent.

  - 2026-07-17 update: the actual M12 launch intent now retains a value copy
    of that direct receipt and M11 rechecks it against current M12 status
    immediately before startup. Path-only availability, payload/MD5 drift,
    virtual evidence, and an unbound intent stay blocked. Remaining work is
    still an operator-performed direct scan with the authenticated capture
    plan; generic asset discovery must not manufacture that admission.

  - 2026-07-17 update: the launch transaction now retains the same opaque
    three-route plan through M12, intent, boot bind and runtime detach.
    Boot rechecks the direct payload path/MD5 and plan before publishing a
    runtime receipt, so a changed CUE/Track02 identity stops before Soul Room
    or dungeon capture routes can be admitted. Remaining work is real
    campaign-capture evidence, not a decoder or a substitute route.

  - 2026-07-17 update: campaign-media reuse now requires a fresh raw-media
    receipt to match the retained direct receipt's canonical CUE and payload
    paths, variant/MD5, mode, INDEX 01, byte/sector counts, and exact
    user-data layout before its capture plan can remain current. Virtual or
    container evidence is still diagnostic only. Remaining work is an
    operator-supplied original direct Track 02 and independently attested
    capture bundles; no route, bitmap, palette, object, or level semantics
    have been inferred.

  - 2026-07-17 update: direct ISO, BIN, and CUE intake now accepts each known
    Track 02 MD5 only with its matching verified sector layout. Startup scan
    receipts retain a stable exact failure identifier for unavailable paths,
    malformed CUE, missing payload, sector alignment, unknown hash,
    hash/layout conflict, CUE INDEX 01 drift, and invalid user-data windows.
    A CUE may also resolve the known `TQJP02.iso`/`TQUS02.iso` materialized
    `End.iso` member name. These checks remain provenance-only: no media is
    copied, no graphics are synthesized, and no consumer or render route is
    promoted.

  - 2026-07-17 update: a validated direct ISO/BIN/CUE receipt with its
    existing identity-bound three-target plan may now enter M12 and M11 as
    `TRACK02 CAPTURE REQUIRED` when no later trace/artifact receipt is bound.
    M11 uses the established no-draw capture-required state and returns to
    the launcher on input instead of attempting startup-atlas admission.
    Plan/epoch/layout drift clears that state. This is not a game start,
    decoder, bitmap, route, or render admission; original capture evidence
    remains required before any later handoff can become ready.

  - 2026-07-17 update: a capture-required rescan now advances M11's media
    scan epoch only after the retained plan and canonical payload layout are
    revalidated. The declared ISO alias path may change while its resolved
    payload path, MD5, mode, CUE index, sector count, and user-data window
    remain exact; payload or plan drift clears capture-required state. M11
    presents the explicit `TRACK02 CAPTURE REQUIRED` no-draw status rather
    than attempting a graphics startup route. No decoder, bitmap, route, or
    synthetic media is introduced.

  - 2026-07-17 update: M11's observed Soul Room and dungeon-handoff loader
    sequence now rechecks that same fresh direct-only layout and its capture
    plan on every route bind. A stale CUE/index/sector/user-data receipt
    clears both route flags before handoff. Remaining work is still original
    trace and capture evidence; this lifecycle grants no decoder or draw path.

  - 2026-07-17 update: the sequence is now also bound to the exact MD5 of the
    converted HuC6280 event log, in addition to the Mednafen source trace.
    A log change between Soul Room and dungeon handoff clears the route pair.
    Remaining work is an original complete trace chain; no captured payload
    has been interpreted as level, object, bitmap, palette, or pixels.

  - 2026-07-17 update: a direct-layout-epoch replay receipt now accepts only
    strictly increasing observed dynamic CD_READ record/sector pairs. Duplicate,
    reordered, mixed-epoch, or layout-drifted records clear the opaque replay
    state. Remaining work is an original multi-record corpus; the receipt does
    not infer record payload or destination semantics.

  - 2026-07-17 update: M11 now consumes the active replay receipt for both
    live routes and retains its campaign-layout epoch plus final record/sector
    identity across the Soul Room to dungeon sequence. Stale epoch, replay
    invalidation, or final-identity drift clears both readiness flags. The
    remaining need is still an original multi-record trace corpus, not a
    level/object or graphics decoder.

  - 2026-07-17 update: an opaque launch trace identity now persists source
    trace MD5, HuC6280 event-log MD5, direct media identity, layout epoch, and
    final record/sector through M12 to M11. Any drift blocks campaign routes;
    no capture payload is decoded or drawn.

  - 2026-07-17 update: external Mednafen/HuC6280 trace bundles now require
    exactly one hash-verified direct candidate matching that launch identity.
    Virtual or ambiguous bundles reject; trace rows remain opaque.

  - 2026-07-17 update: the selected trace bundle now persists through the M12
    launch intent into M11 and is rechecked against the live source/event-log
    identity on each campaign route. Bundle drift clears readiness; no payload
    semantics or draw path opens.

  - 2026-07-17 update: trace-bundle route admission now retains and compares
    the campaign layout epoch, rejecting stale bundle reuse.

  - 2026-07-17 update: rejected M12 trace-bundle rebinds clear the prior
    bundle state, so virtual or mixed replacement evidence cannot leave stale
    readiness behind.

  - 2026-07-17 update: M11 now fingerprints the opaque three-route
    capture-target plan at Soul Room and requires the same identity at dungeon
    handoff. Any source-coordinate or output-identity drift clears both live
    route flags. The plan still grants no format semantics, decode, drawing,
    or fallback visuals.

  - 2026-07-17 update: direct external trace-bundle selection now retains the
    same opaque capture-target-plan FNV identity. M12 rejects stale intent or
    bundle reuse after plan drift, and M11 rechecks it before live route
    readiness. Trace payloads, bitmap/palette data, levels, and objects remain
    opaque and no-draw.

  - 2026-07-17 update: the M12 direct Track 02 re-scan boundary now clears
    previously bound launch-trace and external trace-bundle state unless the
    refreshed direct layout and capture-plan FNV remain exact. Missing,
    rejected, virtual, or changed media cannot leave stale campaign routes
    launchable; all trace payloads remain opaque and no-draw.

  - 2026-07-17 update: externally attested three-route capture readiness now
    retains its Mednafen trace MD5, distinct start/Soul Room/dungeon bundle
    MD5s, and opaque dungeon-window checksum in M11. Rechecking any changed
    receipt clears all capture route flags. This is identity-only lifecycle
    handling; it neither reads payload bytes nor enables a decoder or draw.

  - 2026-07-22 update: raw MODE1/2352 Track 02 media now bypasses the
    capture-required gate and auto-loads the Hall of Records initial level in
    M11 via `theron_v1_startup_runtime_load_initial_level`, producing the same
    `TQR level load` marker as the interactive startup path. MODE1/2048 ISO
    media and any launch with actual capture artifacts still follows the
    capture-required path. `theron_v1_runtime_screenshot_readiness` passes
    (cases=3) and `theron_v1_m11_direct_launch` remains green.

  - 2026-07-27 update: the authentic-capture launcher now rejects a stock
    Mednafen binary before it starts an original-media timeout. It requires
    the compiled `FIRESTAFF_THERON_IRQ2_TRACE` marker, so a binary that would
    silently ignore trace variables cannot yield a misleading empty capture.
    This is capture hygiene only; the outstanding positive Track 02 consumer
    trace and its semantics remain unproved.

  - 2026-07-27 verification: the local instrumented Mednafen build produced
    a provenance-marked trace against the authenticated US CUE. Its six-second
    no-input run observed only System Card boot (`transition=missing`, zero
    game-main-RAM E009 rows), so it contributes no Track 02 layout or consumer
    semantics. The next capture must use the configured real input route.

  - 2026-07-27 update: every authentic transition receipt now carries the MD5
    of the exact instrumented Mednafen executable. This captures producer
    identity beside the existing media hashes; it neither changes trace
    acceptance nor promotes any Track 02 semantics.

  - 2026-07-27 update: the reproducible Mednafen 1.32.1 build now includes
    the bounded main-RAM loader control-flow producer. Capture preflight
    requires both CPU/CD and main-RAM producer markers, preventing the older
    partial trace build from supplying weak evidence. The next input-driven
    run must still produce a positive game-owned consumer observation.

  - 2026-07-27 verification: an authentic US-CUE smoke now emits a
    line-delimited main-RAM control trace from the rebuilt producer. The
    observed `1f0286` TIA/RTS rows are System Card control flow only; they
    establish neither a game-owned consumer nor any Track 02 semantics.

  - 2026-07-27 update: the active main-RAM producer now separately reports
    exact `JSR $e009` control edges and the transition receipt counts them.
    A rebuilt authentic US-CUE smoke reports zero such edges, which is an
    observed absence for that boot window only. It does not claim FIFO,
    destination, record, or loader semantics.

  - 2026-07-17 update: a source-locked live-frame receipt now carries bounded
    decoded wall, floor and door spans plus the shared palette. It starts only
    from the accepted first-frame M11 receipt and permits subsequent frames
    only with consecutive frame numbers, one-step C0..C5 door transitions,
    unchanged path/MD5/palette identity, and fresh span FNV values. The raster
    consumer stays no-draw on skipped, mixed, stale, or incomplete frames.
    A cache adapter now admits those spans only from explicit caller-declared
    entry index, geometry, FNV, path/MD5 and palette receipt facts; it never
    maps a viewport surface to a default entry. Remaining work is operator
    supplied real declarations and live dungeon-state selection, not deriving
    wall/floor pixels or projection geometry.

  - 2026-07-17 update: the optional far D3L2/D3R2 door pair now joins the
    live M11 material draw plan only as one complete ReDMCSB
    `F0676/F0677 -> F0111 -> G0693` receipt. Both sides retain their distinct
    C3700/C3710 route, clip and C10 facts while sharing the checked
    `GRAPHICS.DAT` payload hash. An unpaired route or a missing/mixed side
    hash rejects the pair without changing the established D0/D1/D2 plan.
    Remaining viewport work is still a real decoded G0693 span plus matching
    palette/capture identity; do not derive pixels or add a fallback raster.

  - 2026-07-17 update: the mandatory D2C F0111 route now retains the checked
    ReDMCSB `F0121 -> F0111 -> G0694` capture facts inside its first-frame
    plan proof. The G0694 item, payload FNV and nonzero byte span must agree
    with C3760, 64x61 and C10 before M11 can consume the D2 command. A
    missing, stale, mixed or wrapper-like capture clears the whole plan; it
    does not fall back to an unscoped material hash. Remaining work is still
    a source-decoded G0693/G0694 span plus matching palette/capture identity,
    not inferred pixels or a substitute raster.

  - 2026-07-17 update: the first-frame byte handoff now consumes a dedicated
    D2C/D3 capture receipt before rasterization. It requires one original
    GRAPHICS.DAT path/MD5 identity, an exact palette capture FNV, a nonzero
    capture identity, and source-locked G0694 (64x61) plus, when enabled,
    paired G0693 (48x41) decoded spans. The spans must be the exact pointers,
    sizes and FNVs named by the receipt and by the D2/D3 commands; absent or
    mutated source, palette or span evidence is no-draw. Remaining work is an
    externally attested original capture that supplies these decoded spans and
    palette identity for a real package. Do not decode by guessed entry
    mapping, synthesize pixels or substitute palette data.

  - 2026-07-17 update: the next proven native-material step is now present.
    A source-selected F0489 G0694/G0693 packed 4bpp span is accepted only
    with its original capture path/MD5, raw payload FNV, palette FNV and
    capture identity, then expanded by the F0488 high/low-nibble rule into
    64x61/48x41 indexed bytes. The raw and decoded FNVs remain distinct and
    the latter is what the admissible D2/D3 commands consume. Truncation,
    palette drift, source drift or an omitted far span rejects before raster.
    Raster rechecks the retained capture identity and decoded D2/D3 FNVs, so
    a span mutated after bind also remains no-draw.
    Remaining work is source evidence mapping each F0489 native bitmap index
    to its exact original GRAPHICS.DAT load/decompression selection; do not
    infer that entry mapping from the G0693/G0694 numeric values.

  - 2026-07-17 update: the bounded big-endian `0x8001` GRAPHICS.DAT entry
    table can now be admitted with its count, exact compressed/decompressed
    table span and FNV, path/MD5 identity, and requested native bitmap index.
    ReDMCSB/CSBWin evidence currently does not establish a table row for
    F0489's G0693 or G0694 native bitmap cache index. The provenance receipt
    therefore records that mapping as unproven and blocks F0488 expansion and
    rasterization. A later package must supply a source-owned mapping receipt;
    numeric coincidence, guessed rows, synthetic pixels and fallback palettes
    remain forbidden.

  - 2026-07-17 update: live dungeon-state selection now consumes only an
    exact operator declaration for the current frame number, C0..C5 door
    state, wall/floor/door entry triple and source path/MD5. A state identity
    change invalidates the preceding selection before a new full declaration
    can be returned. Remaining work is feeding source-proven live dungeon
    state into this selector; there is no default material mapping.

  - 2026-07-17 update: the ingress now accepts only a verified-session/tick/
    dungeon-ownership projection and reads the selected real dungeon cell's
    low three bits as C0..C5 door state. The caller's entry triple is copied
    unchanged and never inferred from the grid. Remaining work is binding the
    boot receipt to this projection at the owner boundary without an include
    cycle or host substitute.

  - 2026-07-17 update: the boot viewport route no longer applies the generic
    `VIEWPORT_DERIVED` runtime-plan loop. Live material pixels have one
    explicit owner route requiring verified ingress, grid-derived door state,
    exact declaration selection, cache materialization and frame receipt;
    absent declarations remain no-draw. Remaining work is a positive
    operator-supplied real declaration corpus for this owner route.

  - 2026-07-17 update: a strict operator manifest now owns the same
    path/MD5-bound palette receipt as its parsed frame declarations. Duplicate
    frame identities, malformed source rows, and stale manifests reject before
    corpus admission; the boot manifest route clears any prior selection on
    rejection. A skip-safe local probe now reads only an operator-named
    manifest after matching hash-admitted graphics and exact palette evidence;
    it reports admission without drawing. Boot exposes only that manifest
    route, keeping raw declaration/corpus helpers private. Remaining work is
    a genuinely supplied real corpus and capture parity, not inferred entry
    mapping or a substitute draw route.

  - 2026-07-17 update: M11 now has a single explicit ingress for a restored
    CSBWin timer: caller-named location plus TimerQueue slot go to the
    existing authenticated runner, then M11 accepts only that runner's
    current queue/timer/action receipt. The ingress also requires the
    launch-owned ordinary package identity, save/Dungeon/session/tick
    identity, and wrapper-free session. Missing or stale identity, a wrong
    queue slot, or an unsupported action remains non-mutating and no-draw.
    The remaining gap is still a positive checksum-valid CSBWin save/Dungeon
    corpus, not an M11-side timer selector or inferred opcode behavior.

  - 2026-07-17 update: CSBWin DSA.cpp `STKOP_Copy` now enters the runtime as
    one source/destination DB3 transaction. The candidate runtime re-resolves
    both type-47 records and requires its staged six-byte source payload to
    still equal the loaded source before it writes the destination. A later
    unknown opcode, a stale source image, a wrong DB type, or missing owner
    rejects before live Dungeon bytes publish. This uses the existing
    candidate save/runtime handoff only; no generic copy primitive or timer
    behavior is inferred. Remaining mutation work requires a separately
    proven CSBWin owner family and positive original save/Dungeon corpus.

  - 2026-07-17 update: `STKOP_GeneratorDelayStore` now retains the original
    GeneratorDelay@ result with its pending DB3 delay byte. At commit, the
    runtime must re-read the same source-selected generator/fallback chain and
    match that delay before candidate Dungeon bytes may change. Source drift,
    missing ownership and a later unknown action reject without publication.
    This is strictly the existing type-6/type-0 CSBWin chain rule; it does not
    infer a generator, event, queue entry or timer behavior.

  - 2026-07-17 update: a committed `STKOP_GeneratorDelayStore` receipt now
    records its DB3 location, exact delay byte before/after, and whether the
    source-selected owner was the type-6 generator rather than the defined
    type-0 fallback. The existing restored-dispatch path may attach its saved
    TimerQueue/TIMER scope only after this receipt completes. Missing/stale
    DB3 ownership, queue scope, or an unsupported later action clears the
    receipt; no generator, timer, queue entry, or replacement save is made.

  - 2026-07-17 update: `STKOP_MissileInfoStore` now carries the original
    DB14 range/damage and owning TIMER-direction snapshot to its commit
    callback. Candidate publication re-reads the same DB14/TIMER pair and
    validates its active saved queue ownership before applying the staged
    range/damage/direction update. Drift, a missing queue owner and unknown
    actions reject atomically; no missile, TIMER or event is synthesized.

  - 2026-07-17 update: a committed `STKOP_MissileInfoStore` receipt now also
    retains the exact DB14 Thing and complete four-word source image before
    and after the paired DB14/TIMER direction mutation. The existing restored
    timer path attaches its unique queue slot/TIMER scope only after this
    receipt is committed; stale owner evidence clears the whole execution
    receipt. This adds provenance only, not a missile route, queue entry,
    timer body, or fallback opcode.

  - 2026-07-17 update: the DB14 receipt now additionally proves the linked
    missile TIMER owner across the candidate transaction: stable DB14 timer
    index, one unchanged TimerQueue slot, TIMER function/time, and raw
    position byte before/after the direction write. Any missing, duplicate,
    reordered, stale, or cross-profile TIMER owner rejects the whole DB14
    commit before Dungeon bytes or a runtime receipt publish.

  - 2026-07-17 update: runtime `STKOP_StoreExCellFlg` receipts now retain
    the authenticated EXPOOL-tail FNV before and after the eight-word DB11
    replacement. The callback accepts only its source success code, so a
    failed write cannot be mistaken for a committed DSA mutation. A mutated
    tail rejects before action/receipt publication; exact TimerQueue/TIMER
    scope remains exclusively the existing restored-dispatch handoff.

  - 2026-07-17 update: high-bit `STKOP_TalentsStore` now carries the existing
    `CHARDESC::SaveToWings` eight-record `EDT_Character` write into the
    runtime receipt. The receipt binds the admitted fingerprint, talents word
    before/after, and EXPOOL-tail FNV before/after; candidate publication
    re-reads the full wing bundle on both sides. A missing wing remains the
    source no-op, while a malformed/later unknown action or tail drift
    publishes no wing receipt and cannot reach the restored timer handoff.
    No character allocation, swap, generic save write, or queue behavior is
    inferred.

  - 2026-07-17 update: `STKOP_ExperiencePlus` now retains the source
    `Magic.cpp::AddToSkill` selected-skill/basic-skill pair in the runtime
    receipt. Candidate publication rederives the UI16 increment, cap and
    no-LevelUp result from the live CHARDESC before committing both rows;
    later selected/basic-skill drift makes the receipt and any restored
    TimerQueue/TIMER consumption fail closed. LevelUp, random/stat/UI work,
    generic XP writes and inferred timer behavior remain blocked.

  - 2026-07-17 update: the same `AddToSkill` owner now exposes a read-only
    LevelUp prerequisite receipt only when its proposed selected/basic-skill
    pair crosses the source unadjusted-mastery boundary. It binds the UI16
    increment, both CHARDESC rows before/after, and mastery before/after, and
    expires on source-row drift. It executes no LevelUp code and publishes no
    XP, stat, RNG, UI, save, or timer mutation; the complete original LevelUp
    transaction remains the required next positive owner.

  - 2026-07-17 update: `STKOP_MonsterStore` now binds CSBWin `DSA.cpp`'s
    existing DB4 group-record owner to the runtime receipt. The action keeps
    the complete eight-word group image before its first staged write and
    re-reads the exact same Thing on both the original and candidate Dungeon
    before publication. The receipt retains Thing, source write-mask, and
    complete before/after images; later DB4 drift makes it unavailable to the
    restored save/timer handoff. A later unknown word remains atomic and
    publishes neither DB4 bytes nor a receipt. No group allocation, relink,
    timer construction, queue behavior, generic interpreter, or LevelUp
    behavior is inferred.

  - 2026-07-17 update: `STKOP_CellStore` now binds CSBWin `DSA.cpp`'s
    existing CELLFLAG and first DB0/DB1 owner to the runtime receipt. The
    staged write retains the complete five-word `CellFetch` image before its
    first write; candidate publication re-reads the same location on both
    Dungeon images before exposing location, write-mask and full before/after
    values. Later drift in an exposed CELLFLAG/DB0/DB1 field makes the receipt
    unavailable to the restored save/timer handoff. A later unknown word
    remains atomic. No cell, DB0/DB1 record, Thing link, timer, queue, or
    generic cell semantics are constructed.

  - 2026-07-17 update: `DSACMD_COPYTELEPORTER` now retains the full
    source-cell, destination-cell-before and destination-cell-after `CellFetch`
    images around the existing DB1/CELLFLAG copy owner. Runtime re-reads all
    three images before candidate publication and rejects a later source or
    destination drift. The copy remains bounded to existing source/destination
    DB1 records and their CELLFLAG bytes; no teleporter, Thing link, timer or
    queue is allocated or inferred.

  - 2026-07-17 update: `STKOP_DisableSaves` now records the exact existing
    CSBWin save-policy value before/after its staged GAMEBLOCK2 handoff.
    Receipt currentness rejects policy drift; no save operation, timer, queue,
    fallback policy, or unrelated GAMEBLOCK2 state is inferred.

  - 2026-07-17 update: `STKOP_DiscardText` now retains the source-owned
    TT_OPENROOM DB2/F0168 text receipt before its authenticated clear and the
    exact cleared receipt after it. A replacement or altered text receipt
    makes runtime/save consumption fail closed. This remains one existing
    message receipt only; no text queue, log, renderer fallback, or generated
    text is opened.

  - 2026-07-17 update: `AMPERSAND2/SETSKIN` now requires the current
    source-owned `EDT_Skins` byte before staging a write. A completed action
    publishes packed location and skin byte before/after only after the
    existing EXPOOL/DB11 candidate accepts; a missing owner or later unknown
    opcode publishes neither tail change nor receipt. HUD skin-cache bytes
    now also revalidate the admitted tail FNV/size before consumption, so a
    changed, truncated, or absent save tail clears stale skin data rather
    than supplying a cache fallback. Exact saved TimerQueue/TIMER scope
    remains the existing restored-dispatch attachment, not a new queue path.

  - 2026-07-17 update: queued `LocalState=1` now retains the source
    `DSA::m_state` transition only after the imported four-word DSA header
    and its RCS-protected Extended Features stream agree. Its runtime receipt
    carries the exact saved queue/TIMER identity plus state before/after and
    the post-write tail FNV. A corrupt RCS, stale queue, missing body receipt,
    widened LocalState=2 state, slave route, and unsupported timer family
    clear/reject rather than publishing an inferred state or replacement
    save.

  - 2026-07-17 update: direct M11 F9 reload now retains the native ReDMCSB
    synthetic queue, opcode, or graphics route.
    The source-owned utility/HUD-menu raster admission is covered separately;
    no text/panel fallback is open.
    declaration manifest and matching capture identity. The remaining CSB

  - 2026-07-17 update: the local discovery adapter now accepts only bounded
    direct-file or virtual-member byte views, feeds each through the same
    candidate receipt, skips safely when none is exposed, and rejects an
    ambiguous/mixed set instead of choosing a save. It retains provenance and
    never materializes container data; M12 launch selection still needs to
    consume this CSB-only identity.

  - 2026-07-17 update: M12 CSB quick-resume now carries an explicit selected
    candidate identity. Its launch intent refuses a CSB save path when only a
    game id and path are present, so another game's scan cannot satisfy the
    CSB DSA/save route. Other games retain their existing quick-resume rules.

  - 2026-07-17 update: M12 now consumes the local CSB discovery receipt
    directly. It binds only one valid candidate whose source path equals the
    active CSB resume path; no candidate, a mixed receipt, or path drift
    clears the launch identity. Remaining work is session-binding that
    identity to CSBgraphics title/HUD/door material receipts.

  - 2026-07-17 update: the selected candidate identity now round-trips from
    the hash-verified M12 CSB launch into the opened M11 startup session
    without changing its tick or generation. M12 rejects missing, stale, or
    non-CSB discovery routes before that session is opened.

  - 2026-07-22 update: M11 and the boot-profile viewport adapter no longer
    synthesize a corridor when `DUNGEON.DAT` is absent. Both consume only the
    handoff-owned live grid and fail closed after clearing stale transient
    state. Remaining viewport work is real decoded material and visual
    capture, not a substitute map.

  - 2026-07-22 update: C001 and C004 now retain exact PC34 stream-boundary,
    indexed-raster and implicit-blank-tail facts through session admission.
    This is diagnostic evidence for the remaining visual title/Entrance bug;
    it does not claim that the observed palette or pixel presentation is
    correct yet.

  - 2026-07-23 C001 palette update: PRESENTS, CHAOS and STRIKES BACK now use
    their distinct source-backed F0437/CSBWin palette transactions instead of
    DM PC/F20 aliases. The remaining C001 visual work is real app-window
    capture and comparison, not a synthetic palette fallback.

  - 2026-07-23 C001 M11 plan-admission update: M11 now requires the active
    C001 plan to pass `csb_v1_boot_startup_title_capture_plan_admit_pc34`
    before selecting its special palette or presenting its indexed raster.
    This rejects stale source-step/rectangle metadata rather than collapsing
    it under a valid palette ID. Remaining work is external app capture and
    source-duration comparison.

  - 2026-07-23 C005 update: C202/F0442 now requires its real GRAPHICS.DAT
    credits surface and no longer exposes generated credits text. The
    C005 decoder/loader admission is source-verified; remaining work is app
    visual capture, not fallback UI.

  - 2026-07-23 C005 M11 host update: when the source clock reaches the
    terminal C001 tick before M11 has consumed the title phases, M11 now
    replays the verified C001 phase plans into that same session before
    F0442 presents C005. This prevents the source phase-mask gate from
    dropping the valid real credits raster to black. The real GRAPHICS.DAT
    regression verifies both the C005 host receipt and visible M11 frame;
    remaining work is external app capture.

  - 2026-07-23 C005 terminal-handoff update: the decoded credits surface now
    remains insufficient on its own. F0442 must publish a real host frame
    receipt, retaining its source tick and frame/raster hashes in the same
    C001-C005 session. If the optional F0442 credits route was used, F0807
    refuses the HUD handoff until a real C004/C002/C003 return frame is also
    consumed. The real GRAPHICS.DAT sequence regression proves the rejection
    before that return and the source-locked handoff after it. Remaining work
    is external app capture and source-duration comparison.

  - 2026-07-23 C017/C040 HUD package-plan update: the terminal neutral
    palette route now requires the exact decoded GRAPHICS.DAT record-boundary
    receipt for C017 and C040 as well as their source IDs, dimensions, and
    C040 transparency. Remaining HUD work is external app capture and
    original-presentation comparison, not a generated panel fallback.

  - 2026-07-23 F0134/F0135 source-bound fill update: HUD and door fill targets
    now originate exclusively from the admitted C017/C040 or C002/C003 host
    raster. The consumer rejects synthetic, stale, clipped, or non-GRAPHICS.DAT
    receipts; remaining work is live app capture rather than a fallback fill
    path.

  - 2026-07-23 C017/C040 M11 frame admission update: M11 requires the
    existing complete F0807 terminal HUD host frame to retain its source
    neutral palette before it may consume real `GRAPHICS.DAT` C017/C040
    surfaces. The check deliberately remains in the single readiness query:
    replaying the startup plan during panel blit breaks C002/C003 ordering.

  - 2026-07-28 C001 timing audit closed: `TITLE.C:451-463` is now represented
    as 60 + 20 + 20 + 2 VBlanks in the sequence, playback mask, session
    contract, and real-data regressions. Remaining CSB startup work is
    visual Mac/app capture, not title cadence.

  - 2026-07-23 C001 timing audit: ReDMCSB `STARTND2.C F0437` and CSBWin
    `_DisplayChaosStrikesBack` show 18 CHAOS zoom blits, two VBlanks of the
    full-size CHAOS page, then `VBLDelay(20)` after C426 STRIKES BACK. The
    integrated boot capture still owns the older 101-tick decomposition and
    hard-coded capture samples. Correcting those source timings requires a
    dedicated `boot.c` batch; it was deliberately excluded from the current
    M11 host-presentation work. The current real-asset regression now pins
    each admitted host phase, palette, and raster route so it cannot regress
    while that source-timing correction is pending.

  - 2026-07-17 update: title runtime lifecycle admission now records only the
    exact PRESENTS, CHAOS zoom, CHAOS hold, and STRIKES capture identities.
    Each transition checks the source palette, session generation, and
    monotonically increasing tick; a stale or skipped phase remains no-draw.

  - 2026-07-16 update: the CSB-owned F0437/F0438/F0580/F0581 consumption
    adapter accepts only a generation-matched verified session plus
    complete-support receipt. It rejects duplicate title phase hashes,
    incomplete title/door/HUD facts, fallback callbacks and wrapper routes
    before source-named title or entrance consumption. Remaining work is
    external Mac/app capture and original-presentation comparison, not
    readiness flags or fallback wrappers.

  - 2026-07-16 update: when both C017 and C040 are selected from a
    hash-admitted `CSBgraphics.dat`, the startup session now decodes them from
    the cached indexed entries only after path, MD5 receipt, runtime-plan
    route, entry geometry and decoded-byte count agree. A partial or malformed
    override cannot create a mixed fallback HUD. Remaining work is external
    Mac/app capture and the wider live HUD-state route, not substituting either
    panel from a wrapper or generated bitmap.

  - 2026-07-16 update: title playback now records the two-VBlank full-size
    CHAOS hold together with the first C426 STRIKES BACK frame, preserving the
    F0437 phase mask when a live consumer samples the transition boundary.
    The entrance-to-HUD edge now completes F0807 from the same verified
    session-owned C004/C002/C003/C017/C040 readiness facts, rather than a
    separate host presentation flag. Remaining work is external Mac/app
    capture and broader runtime HUD state, not a title or HUD wrapper.

  - 2026-07-16 update: a HUD page decoded from hash-admitted
    `CSBgraphics.dat` now folds its admitted MD5 receipt, resolved path, and
    C017/C040 entry pair into the session-to-live-frame binding hash. That
    value reaches the existing route, host-raster, and presentation/capture
    hashes, so identical pixels from another cache cannot be presented as the
    same HUD page. Original `GRAPHICS.DAT` HUD frames retain a zero source
    receipt. Remaining work is external Mac/app original-capture comparison
    and the wider live HUD-state route, not a wrapper or fallback panel.

  - 2026-07-16 update: the direct runtime capture consumer now walks all 31
    ReDMCSB F0438/F0807 C004/C002/C003 door-opening pages from one verified
    session. Each source step must produce its own route-bound raster capture;
    replayed, missing, synthetic, or wrapper-owned pages fail closed. Remaining
    startup work is external Mac/app comparison and broader live HUD state.
    The positive CSBWin DSA/save-runtime path remains separately blocked on an
    operator-supplied checksum-valid DSA-bearing save/dungeon corpus.

  - 2026-07-16 update: each declared title, entrance-door and HUD entry now
    retains its own cache-derived image receipt (source kind/path/MD5 and
    bounded archive span). The startup package grants draw eligibility only
    when that image receipt and the admitted palette receipt name the same
    source. Title/HUD/door identity mismatches remain no-draw; this is still
    provenance infrastructure, not a new startup renderer.

  - 2026-07-16 update: the hash-admitted local corpus scanner now walks only
    entries declared as exactly 768 bytes, bounded-decodes each candidate and
    reports its entry span, source path/MD5 and FNV identity. A second
    caller-declared path/MD5/entry/FNV admission is required before it emits
    source bytes as a receipt. The scan remains diagnostic and the runtime
    remains no-draw until a future source-owned presentation package consumes
    a real captured declaration.

  - 2026-07-16 update: boot now owns the candidate admission and retains the
    receipt only after its caller-declared path/MD5/entry/FNV matches a fresh
    candidate scan. CSBgraphics startup provenance exposes title/door/HUD
    palette readiness separately; absent or mismatched receipts make selected
    CSBgraphics HUD bindings unverified and keep M11 no-draw. Original
    `GRAPHICS.DAT` routes are not recolored or substituted by this gate.

  - 2026-07-16 update: host dungeon-consumer admission now accepts only the
    named original Track 02 host route and carries its FNV identity checksum
    beside the admitted level/object/bitmap/palette proof hashes. A changed
    route label or checksum fails closed. This hardens the real-data handoff
    only; unknown dungeon and object fields remain opaque until an
    authenticated original consumer trace binds their format.

  - 2026-07-16 update: level-bank selection also requires the runtime media
    identity's Track 02 variant to match that shared known MD5. Unknown
    32-character labels and JP/US identity drift reject before a bank enters
    runtime. This is source provenance only; it does not bind a palette to a
    route, decode level geometry, or promote object fields.

  - 2026-07-16 update: an authenticated direct VCE index/low/high store route
    now carries its verified Track 02 MD5 and variant into a runtime admission
    for one matching raw-source bitmap surface. Variant or MD5 drift rejects,
    and render, dungeon draw, and fallback remain closed. The VCE stores are
    not yet bound to a palette window or to any bitmap pixels; level geometry
    and object fields remain opaque.

  - 2026-07-16 update: the captured Soul Room raw span now admits its matching
    runtime surface only when the complete startup-media capture, authenticated
    CD-read trace, MD5/variant, raw offsets, checksum, route mask, atlas
    checksum, and runtime identity agree. This proves capture-to-runtime byte
    provenance, not a VCE relation, pixel decoding, level geometry, object
    layout, or dungeon drawing; all of those paths remain blocked.

  - 2026-07-16 update: runtime now consumes the capture-bound Soul Room
    receipt beside the opaque initial loader record only when both retain the
    same Track 02 identity. The resulting consumer preserves exact payload,
    envelope, and post-envelope provenance while explicitly requiring an
    original level/object consumer trace. It does not admit level fields,
    object fields, pixels, palette semantics, or drawing.

  - 2026-07-16 update: when an original post-$3800 grammar receipt arrives,
    runtime now verifies both record windows against its own loader payload
    before retaining them as a decoder-required preparation. Consumer PCs,
    checksums, and bounded payload windows must agree; level/object fields,
    bitmap/palette semantics, runtime handoff, and drawing remain blocked.

  - 2026-07-16 update: the runtime-owned initial loader record now accepts
    only a known Track 02 MD5 and a MODE1/2352 user-data-aligned raw offset.
    This closes malformed container provenance before trace preparation; it
    does not identify record grammar or authorize level/object fields.
    object fields, pixels, palette semantics, or drawing.

  - 2026-07-16 update: an explicit user-supplied CUE, BIN, or ISO can now be
    classified only after its payload MD5 matches a known Track 02 variant.
    CUE intake accepts exactly one Track 02 `MODE1/2352` or `MODE1/2048`
    binary member with exactly one `INDEX 01`, then retains only sector and
    logical user-data-window coordinates. The existing raw loader-trace route
    receives coordinates only from a known raw BIN CUE at its source-locked
    index sector; bare BINs and ISOs remain ineligible. No CUE/BIN/ISO path
    decodes level/object records, VCE data, bitmap pixels, or creates visuals.

  - 2026-07-16 update: a bounded external capture-trace manifest can now join
    only the authenticated raw CUE intake, raw trace coordinates, runtime
    provenance, and existing loader-window preparation. Its closed key set
    must exactly repeat the already accepted MD5, variant, loader receipt,
    consumer PCs, and both record windows; unknown, duplicate, or mismatched
    rows reject. The result is opaque level/object evidence only: field,
    bitmap/palette, pixel, draw, and fallback routes remain blocked.

  - 2026-07-16 update: runtime now admits an opaque Track 02 trace route only
    when that explicit manifest-bound evidence exactly matches the retained
    provenance and level/object preparation receipts. It rejects absent
    manifest evidence, unknown consumer PCs, changed windows, and attempts to
    pre-open draw. `opaque_route_ready` is preparation for a later reviewed
    decoder only; level/object decoding, bitmap/palette, pixels, draw, and
    fallback remain false.

  - 2026-07-16 update: an external-only local capture launcher now verifies
    an operator-supplied emulator path, CUE/BIN, and expected Track 02 MD5
    before writing a comment-only strict-manifest skeleton. It never launches
    the emulator, copies media, emits trace rows, or decodes source bytes.
    A later operator-filled manifest is rechecked through intake, binding, and
    runtime admission. The remaining positive step is an original emulator
    capture that supplies the observed loader/consumer fields without guesses.

  - 2026-07-16 update: the external-only capture request is now exposed as a
    dedicated local probe with explicit `--prepare` arguments. Missing inputs
    skip safely; malformed requests reject. The probe delegates to the strict
    intake/skeleton route and cannot launch an emulator, copy media, emit a
    trace, or decode Track 02 bytes.

  - 2026-07-17 update: the external runtime-validation route can now build
    its manifest in memory only from that HuC6280 log before entering the
    existing raw-media, provenance, and runtime-admission chain. It does not
    read or write the hand-edited manifest path on this route, and preserves
    the same opaque no-decoder/no-draw boundary.

  - 2026-07-17 update: an external-only Mednafen trace converter now accepts
    only the closed `mednafen-pce-instrumented` export grammar after the
    caller supplies both its source path and matching MD5. It copies the three
    observed HuC6280 rows verbatim in meaning into the strict event-log
    grammar, rejecting unsupported debugger lines, extra rows, and MD5 drift.
    It launches no emulator, copies no media, and creates no trace event; the
    remaining blocker is still an original capture containing the required
    loader and two consumer observations.

  - 2026-07-17 update: the external capture handoff can now consume only that
    MD5-attested converted log after its normal explicit Track 02 media
    preflight, then passes it through the existing opaque manifest/runtime
    admission. The media preflight occurs before any event-log output is
    written; unavailable or changed inputs reject and no decoder, pixels,
    palette semantics, or drawing is enabled.

  - 2026-07-17 update: the Mednafen converter now rejects output paths that
    are the source trace itself or resolve to the same file identity. This
    preserves the MD5-attested external observation against overwrite through
    textual path aliases; it still does not synthesize or interpret events.

  - 2026-07-17 update: conversion also refuses to replace an existing
    event-log file. A write or close failure rejects and removes only the
    partial file it just created; existing operator evidence remains intact.
    New output is created exclusively after preflight, so a concurrent existing
    log cannot be replaced between the check and write steps.

  - 2026-07-17 update: the derived strict event log is hashed after successful
    close and before conversion succeeds. That MD5 is retained only by a
    successful converted-trace runtime handoff; hash failure removes the newly
    created output and rejects.

  - 2026-07-17 update: external Mednafen trace sources must now be regular
    files. Devices, FIFOs, and directories reject before hash or parse work,
    keeping no-launch intake bounded and file-provenance-only.

  - 2026-07-17 update: converted event-log MD5 is now checked immediately
    before and after HuC6280 manifest/runtime consumption. A substituted log
    rejects, and runtime remains opaque with every decoder/draw flag false.

  - 2026-07-17 update: the same local probe now has a write-free `--inspect`
    mode for one explicit existing Mednafen export. It accepts only the closed
    grammar, regular-file boundary, and a stable before/after MD5, then reports
    that MD5 for the later explicit conversion; it creates no event log and
    opens no runtime, decoder, or draw route.
    The configured Track 02 CTest also exercises an explicit missing export as
    a safe skip, preventing absent user evidence from becoming a positive path.

  - 2026-07-17 update: `--discover <capture-root>` now scans at most 128
    direct entries of one explicit, non-symlink directory. It reports only
    regular files that pass the same closed inspection and stable-MD5 gate;
    unknown files and directories remain non-evidence. The verified local
    Theron roots currently contain no such export, so discovery skips without
    a log, media copy, runtime admission, or decoder action.

  - 2026-07-17 update: the verified local US raw CUE's explicit
    `000a50/000a52` or `000b33/000b37`, with zero non-System-Card PCECD reads
    and zero main-RAM loader TII transfers. This is stronger original-menu
    coverage only; it must not be interpreted as a Track 02 dungeon handoff.
    `PREGAP 00:03:00` is now accepted as the source-locked Track 02 LBA 225,

  - 2026-07-16 Theron draw-route gate update: M11 now has a guarded
    dungeon draw-route receipt that can consume the proven level-1 media,
    multilevel handoff, live level geometry, party pose, object placement,
    and bounded viewport-cell sample while keeping pixel blit and fallback
    visuals closed. The real US-CUE probe still cannot open that route because
    the current verified media path does not yet provide a real loaded
    nonstartup level/object placement pair for level 1. Remaining work is the
    actual real Track 02 level/object decode that feeds this receipt from
    CUE/BIN/loadertrace rather than a runtime fixture.

  - 2026-07-17 update: the live dungeon-handoff replay validator now admits
    exactly six ordered, existing receipts: authenticated CUE/Track 02,
    dynamic CD_READ, loader chain, direct-VCE output identity, source-bound
    bitmap identity, and the manifest-bound destination record window. Missing,
    reordered, or mutated rows reject. The destination remains an opaque
    offset/length/checksum boundary; level/object semantics, pixel decode,
    rendering, and fallback all remain false. Remaining blocker: one original
    Mednafen run must provide the ordered palette-output, bitmap-output, and
    destination-window observations alongside the already authenticated
    dynamic CD_READ trace. The explicit local probe safely skips when that
    external capture is absent.

  - 2026-07-17 update: the capture-artifact importer now accepts only one
    direct, stable-MD5 Mednafen trace and one direct, stable-MD5 bundle with
    the exact three ordered start/Soul Room/dungeon plan rows. The bundle must
    repeat the authenticated Track 02 MD5 and every CD-read, loader, palette,
    bitmap, and destination identity exactly; mixed, partial, reordered, or
    mutated artifacts reject. The resulting runtime receipt is opaque only.
    Remaining blocker: an operator-supplied complete original evidence bundle
    matching a live three-route plan. The local corpus has no `.bundle` file,
    so the discovery probe safely skips.

  - 2026-07-17 update: the multi-bundle campaign verifier now requires three
    distinct ready bundle MD5s in start, Soul Room, dungeon order. It compares
    their Track 02 and Mednafen-trace provenance plus every opaque CD-read,
    loader-output, palette-output, bitmap-transfer, and destination identity.
    Any reuse, route reorder, or contradictory identity rejects before the
    campaign receipt is retained. Remaining blocker: three independently
    captured original bundles from one matching provenance chain. The local
    corpus has fewer than three candidates and safely skips.

  - 2026-07-17 update: operator-only campaign-bundle emission now requires an
    already authenticated three-route plan, explicit raw CUE/Track 02 MD5,
    and a stable original Mednafen trace MD5 before it can exclusively create
    three route-selected manifests. Dry-run writes nothing; the external probe
    cannot construct a plan or capture row. Remaining blocker: a real
    authenticated plan plus complete Mednafen trace from which to attest the
    three manifests.

  - 2026-07-17 update: runtime now has a capture-campaign admission boundary
    that joins campaign provenance to active startup media and the opaque
    dungeon record window. Only a complete matching campaign marks start,
    Soul Room, and dungeon capture-ready; missing campaign, Track 02 drift,
    or window drift clears the receipt. This changes no startup selection and
    leaves decode, render, and fallback false. Remaining blocker: a real
    campaign receipt from independently captured bundles.

  - 2026-07-17 update: Hall-of-Records startup/save-resume receipt coverage
    now treats the post-descriptor entry-6 bytes as an opaque candidate only.
    It preserves source offsets, MODE1 user-data provenance, bounded hash, and
    fail-closed blocker state while rejecting semantic object-table mapping,
    field decoding, and cross-anchor row consensus. Remaining blocker: a
    complete original loader/consumer trace that binds this candidate.

  - 2026-07-17 update: the M11 Theron host route now retains capture-ready
    state only through the verified campaign admission and its active startup
    media receipt. Media MD5 drift, campaign destination drift, absent evidence,
    and non-Theron host state clear all start/Soul Room/dungeon readiness;
    no rendering or decoder gate changed. Remaining blocker: independently
    captured original campaign bundles that can reach this host boundary.

  - 2026-07-17 update: campaign media discovery now hashes explicit loose
    CUE/BIN/ISO media through the strict raw intake and searches configured
    roots through Firestaff's existing hash/container scanner. Virtual matches
    remain non-materialized, non-launchable identity evidence; multiple known
    Track 02 variants reject as ambiguous. Remaining blocker: a matching
    original CUE/BIN or ISO plus independently captured campaign bundles.

  - 2026-07-17 update: the Theron startup launch/profile path now consumes a
    campaign-media receipt only for one direct exact-layout candidate matching
    all three capture-plan identities. Profile MD5 drift, mixed variants, and
    virtual/archive evidence clear launchability while retaining diagnostics.
    Remaining blocker: operator-supplied original direct media and complete
    independent capture bundles for the selected variant.

  - 2026-07-17 update: runtime now consumes an accepted SRM receipt only as
    MD5/size/version/Track 02 identity. It rejects any missing admission fact
    and explicitly keeps restore, body semantics, and fallback disabled.

  - 2026-07-17 update: a local `--admit` SRM probe now exposes this exact
    external-only chain. It requires path, SRM MD5, exact size, admission
    version, and Track 02 MD5; no-input and missing corpus safely skip, while
    it never writes a save, restores state, or decodes bytes.

  - 2026-07-16 Theron level-1 draw blocker update: the real US-CUE path now
    emits an explicit level-1 M11 draw blocker receipt after the proven stage
    atlas/palette media consumption. The receipt records the real nonstartup
    Track 02 windows and object-table window from the binding gap, inspects
    runtime world geometry/object state, and fails closed when level-1
    geometry or object placement is not sourced from verified bytes. Remaining
    work is still the positive decoder: bind HuC6280 loadertrace/CD-read
    records to exact nonstartup level-1 geometry and object placement windows,
    then feed those real records into the existing M11 draw gate. Do not use
    synthetic geometry, synthetic objects, fallback visuals, or guessed level
    decoders to close this.

  - 2026-07-16 DM2 skproject alias audit update:
    14 already implemented HUD/item/object/query aliases were moved out of
    `MISSING` in `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`. The closed rows are
    `DM2_MONEY_BOX_SURVEY`, `DM2_SHOW_ATTACK_RESULT`,
    `DM2_REMOVE_POSSESSION`, `DM2_LOAD_PROJECTILE_TO_HAND`,
    `DM2_RETRIEVE_ITEM_BONUS`, `DM2_PROCESS_ITEM_BONUS`,
    `DM2_PUT_OBJECT_INTO_CONTAINER`, `DM2_QUERY_PLAYER_SKILL_LV`,
    `DM2_IS_MISSILE_VALID_TO_LAUNCHER`, `DM2_GET_MISSILE_REF_OF_MINION`,
    `DM2_IS_ITEM_HAND_ACTIVABLE`, and matching SKWIN aliases where present.
    Remaining rows still require source-backed implementation or explicit
    non-applicability.

  - 2026-07-22 DM2 c_gfx_blit.cpp symbol audit batch:
    renderer (`dm2_v1_fill_rect`, `dm2_v1_blit_scaled_material_bitmap_region_ex`,
    `dm2_v1_viewport_calc_stretched_size`, `dm2_v1_weather.c`). The DM2 skproject
    backlog dropped from 1074 to 1037 open rows.

  - 2026-07-22 DM2 SkWinCore alias follow-up:
    Closed the remaining SkWinCore aliases `_2066_1f37`, `_2066_1ec9`, and
    `_0cee_1a46` as `IMPLEMENTED_NARROW` aliases of the already-implemented
    SKULLWIN/c_map.cpp and dungeon-loader receipts. DM2 skproject backlog
    dropped from 1037 to 1034 open rows.

  - 2026-07-22 DM2 c_gfx_decode.cpp symbol audit batch:
    Closed the next SKULLWIN family after c_gfx_blit.cpp: `func_44c8_1202`,
    `spill_img3_pixels`, `read_img3_nibble`, `read_img3_duration`,
    `transparent_img3_pixels`, `decode_img3_overlay`, `dec9_1sub`, `dec9_1`,
    `dec9_2`, `dec9_3`, and `decode_img9` are now `IMPLEMENTED_NARROW`
    source-named decode receipts in `dm2_v1_gfx_decode_receipt.c`.  The
    `init`/`alloc` lifecycle entries are `VERIFIED_SOURCE_MAPPING` no-op
    boundary receipts, and `decode_img3_underlay` is mapped to the existing
    typed GDAT image loader.  A focused synthetic-data test
    (`test_dm2_v1_gfx_decode_receipt`) covers all 14 rows.  DM2 skproject
    backlog dropped from 1034 to 1021 open rows.

  - 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 3):
    synthetic-data coverage was added to `test_dm2_v1_skproject_core.c`.
    `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` and `SYMBOL_DISPOSITIONS.tsv`
    were updated. DM2 skproject backlog drops from 1021 to 1012 open rows.

  - 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 4):
    receipts was added to `test_dm2_v1_skproject_core.c` and passes.
    `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` and `SYMBOL_DISPOSITIONS.tsv`
    were updated. DM2 skproject backlog drops from 1012 to 1006 open rows.

  - 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 5):
    `test_dm2_v1_skproject_core.c` and passes.
    `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` and `SYMBOL_DISPOSITIONS.tsv`
    were updated. DM2 skproject backlog drops from 1006 to 997 open rows.

  - 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 6):
    receipts was added to `test_dm2_v1_skproject_core.c` and passes.
    `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` and `SYMBOL_DISPOSITIONS.tsv` were
    updated. DM2 skproject backlog drops from 997 to 984 open rows.

  - 2026-07-22 DM2 SkWinCore symbol audit batch (Lane A, cycle 7):
    now names the SKULLWIN originals. `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
    and `SYMBOL_DISPOSITIONS.tsv` were updated. DM2 skproject backlog drops
    from 954 to 943 open rows.

  - 2026-07-23 DM2 SkWinCore symbol audit batch (Lane A, cycle 8):
    now names all eight SKULLWIN originals. `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
    and `SYMBOL_DISPOSITIONS.tsv` were updated. DM2 skproject backlog drops
    from 943 to 935 open rows.

  - 2026-07-23 DM2 SkWinCore symbol audit batch (Lane A, cycle 9):
    `tests/test_dm2_v1_skproject_core.c` and passes. The source evidence string
    now names all seven SKULLWIN originals. `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
    was updated. DM2 skproject backlog drops from 935 to 928 open rows.

  - 2026-07-23 DM2 SkWinCore symbol audit batch (Lane A, cycle 10):
    now names all seven SKULLWIN originals. `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
    and `SYMBOL_DISPOSITIONS.tsv` were updated. DM2 skproject backlog drops from
    928 to 921 open rows.

  - 2026-07-23 DM2 SkWinCore symbol audit batch (Lane A, cycle 11):
    now names all six SKULLWIN originals. `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
    and `SYMBOL_DISPOSITIONS.tsv` were updated. DM2 skproject backlog drops from
    921 to 915 open rows.

  - 2026-07-16 DM2 sound helper update:
    `R_5044A`, `R_51AF6`, `R_4FF39`, `R_B65`, `R_928`, `R_8FE`, `R_5096A`,
    `R_51083`, `R_51B56`, `R_8E6`, and `R_8AF` now have source-backed
    bounded receipts in the DM2 sound adapter. `R_5F7`, `stop_all_sound`,
    `dtor`, and `sndptr6` now also have adjacent c_sound receipts. Remaining
    adjacent work is the larger live `DM2_PLAY_SOUND`/sample-buffer route once
    real runtime sample allocation state is available.

  - 2026-07-16 DM2 SUPPRESS save-symbol update:
    `DM2_SUPPRESS_INIT`, `DM2_SUPPRESS_WRITER`, `DM2_SUPPRESS_FLUSH`,
    `DM2_SUPPRESS_READER`, `DM2_WRITE_1BIT`, `DM2_READ_1BIT`, and matching
    SKWIN aliases now have a source-named receipt over the real SUPPRESS
    bitstream state. It proves MSB-first source-bit selection, cross-section
    pending-bit carry, fill policy, and underflow rejection without fabricating
    a save payload. Remaining adjacent save work is larger `SKLOAD_READ`,
    `SKSAVE_WRITE`, record-checkcode, possession-index, timer compaction, and
    GAME_SAVE_MENU/SUBSAVE/FSUBSAVE coverage against real SKSave data.

  - 2026-07-16 DM2 top movement/map update:
    `DM2_ARRANGE_DUNGEON`, `DM2_PERFORM_MOVE`, and
    `DM2_move_075f_1bc2` now have source-backed receipts over real arranged
    `DUNGEON.DAT` map layout, bounded movement target facts, and the
    PERFORM_MOVE admission plan. Real DUNGEON.DAT accepted and blocked target
    receipts are joined into PERFORM_MOVE without fallback or viewport
    rendering. Remaining adjacent top-open movement work starts at the next
    disjoint SkWinCore runtime helper or c_move/c_moverec runtime family, plus
    broader live mutation in `DM2_PERFORM_MOVE` beyond this non-mutating
    receipt family.

  - 2026-07-16 DM2 RECALC_LIGHT_LEVEL update:
    `DM2_RECALC_LIGHT_LEVEL` and the SKWIN `RECALC_LIGHT_LEVEL` alias now have
    verified source mappings through the DM2 c_light runtime receipt. The
    map-bound builder consumes real `DUNGEON.DAT`
    `Map_definitions::Difficulty()` descriptor receipts, keeps fixed and
    dynamic light branches separate, hashes the descriptor identity into the
    result, and still refuses to invent live base-light/darkness state from
    GRAPHICSSET controls. Remaining work is binding the original inventory,
    record, and darkness accumulators from real runtime/save state before live
    palette remaps can be promoted more broadly.

  - 2026-07-16 DM2 SkWinCore IBMIO/palette/anim/mouse update:
    `_0759_06c2`, `_0759_06db`, `_0759_071b`, `_0759_072c`,
    `_0759_065f`, and the full cursor bitmap pattern path once its real
    surface ownership is connected.

  - 2026-07-16 DM2 p130 ANIM/IBMIO runtime-vector update:
    `_0759_0126`, `_0759_06c2`, `_0759_06db`, `_0759_072c`,
    `_0759_071b`, `_01b0_1ed2`, `_0759_06b5`, and `_0759_065f`
    now have source-backed DM2 skproject-core receipts over caller-owned
    ANIM/IBMIO runtime state: interrupt-vector capture, timer callback
    countdown, event poll/consume, source 320x200 zero-fill, LFSR screen-line
    clear order, and sound-card availability. `_069a_03fc`, `_0759_07f2`,
    `_0759_0792`, and `_0759_0739` were moved to explicit
    `NONAPPLICABLE` because skproject itself leaves them as `Unr()` ANIM
    stubs. Remaining adjacent rows start at `_0759_0855`/`_0759_0869` and
    require real source evidence before promotion.

  - 2026-07-16 DM2 outdoor weather M11 material update:
    `DM2_DRAW_TEMP_PICST` now has a final receipt that requires the exact
    ENVIRONMENT `dtText`, GFX256 raw `dtImage`, decoded-pixel/local-palette
    identities, renderer transaction, and live `DM2_SET_TIMER_WEATHER`
    owner before the runtime binds weather to M11. The canonical corpus has
    real command text but lacks this complete image chain, so the added
    real-data test proves no-draw rather than fabricating a cloud or rain
    frame. Remaining work is a corpus with complete original weather image
    evidence, not a generic DM1 projection or procedural fallback.

  - 2026-07-16 DM2 startup menu host-input gate update:
    host-facts keyboard/menu input and pointer routes now reject inactive
    `startup_menu_active` state before constructing a startup snapshot. The
    title/menu GDAT surfaces, pointer click rectangles, interface palette, and
    static HUD chrome now share a boot-level real-GRAPHICS.DAT receipt. The
    remaining adjacent DM2 work is deeper live menu/HUD runtime capture and
    viewport material admission beyond this receipt, not synthetic menu input
    or fallback artwork.

  - 2026-07-16 DM1/CSB F1690/F1691/F1692 update:
    `F1690_GetASCIICode`, `F1691_Cconis`, and `F1692_Crawcin` now
    have DM1-owned narrow USIO keyboard input callables. They consume only
    explicit caller-owned raw-key/key-buffer state, preserve low-byte
    ASCII/control values, and do not synthesize scan-code translation,
    blocking host waits, fallback keyboard state, or input events.

  - 2026-07-16 DM1/CSB F7039/F7041 update:
    `F7039_DrawHealthOrStaminaOrMana` and
    `F7041_ProcessKeyboardInput` now have DM1-owned narrow CEDT006
    champion-editor/Hall-of-Champions receipts. `F7039` records only
    caller-owned name/value/screenY draw facts, and `F7041` edits only a
    caller-owned text buffer from an explicit key sequence; neither path
    synthesizes screen pixels, keyboard events, champion data, graphics
    resources, or fallback editor state.

  - 2026-07-16 DM1/CSB F1684/F1694 update:
    `F1684_GetMouseStatus` and `F1694_AddMouseInputToQueue` now have
    DM1-owned narrow input-runtime callables. `F1684` copies only explicit
    caller-owned mouse status, and `F1694` routes explicit mouse X/Y/button
    values through the existing COMMAND.C mouse command tables and queue
    limits; no host cursor polling, coordinate sampling, button synthesis, or
    fallback mouse events are introduced.

  - 2026-07-16 DM1/CSB F1172/F1173/F1174 update:
    `F1172_QueueMouseAndKeyboardInput`,
    `F1173_AddUsioDataToInputQueue`, and
    `F1174_AddPendingUsioDataToInputQueue` now have DM1-owned narrow
    input-runtime callables. They bridge explicit caller-owned USIO
    keyboard/mouse samples into the existing PC34 command queue and preserve
    pending input until an enqueue is accepted; no host polling, raw keyboard
    data, mouse coordinates, or fallback input events are synthesized.

  - 2026-07-16 DM1/CSB F1909/F1984/F2014 update:
    `F1909_CopyStringUntilCharacter`,
    `F1984_ConvertCharacterToLowerCase`, and
    `F2014_ConvertStringToLowerCase` now have DM1-owned source-named
    PC34 callables for bounded hint text copying and ASCII lowercasing.
    The implementation stays in caller-owned string buffers and does not
    synthesize hint-oracle, HTC/file, screen, palette, or input state.

  - 2026-07-16 CSB/ReDMCSB STARTEND F0439/F0441/F0442 update:
    `F0439_STARTEND_DrawEntrance`,
    `F0441_STARTEND_ProcessEntrance`, and
    `F0442_STARTEND_ProcessCommand202_EntranceDrawCredits` now have
    CSB-owned source-named entrance-boundary receipts. They accept only
    caller-provided host-view/ownership facts that prove real startup assets,
    package draw ownership, host input ownership, and no legacy render wrapper
    or fallback graphics. Remaining adjacent work is live wiring to the
    existing full startup host-view receipts and manual Mac/app capture
    evidence; do not add synthetic entrance, credits, title, or HUD payloads.

  - 2026-07-16 CSB/ReDMCSB F0440 update:
    `F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount` now has a CSB-owned
    source-named receipt for temporary entrance/credits GRAPHICS.DAT members.
    It accepts only caller-proven real decompressed byte counts for C004, C005,
    C534, or C535 through the reviewed F0490 load/decompress route, and rejects
    synthetic graphic bytes, synthetic file handles, and legacy graphics
    wrappers.

  - 2026-07-16 CSB/ReDMCSB F0806 update:
    `F0806_F0806_ENTRANCE_int` now has a CSB-owned entrance-loop handoff
    receipt. It accepts only source-locked entrance assets, input table setup,
    F0439 redraws, C099 wait-loop ownership, optional F0442 credits loops, the
    F0797 micro-dungeon receipt, and the F0807 door-animation receipt when the
    final decision loads a dungeon. Synthetic input, synthetic graphics bytes,
    fallback visuals, legacy entrance wrappers, and terminal credits/wait states
    are rejected.

  - 2026-07-16 CSB/ReDMCSB F0908/F0909/F0910 update:
    `F0908_InitSound`, `F0909_PlaySwooshSound`, and
    `F0910_ReleaseSwooshSound` now have CSB-owned pre-title swoosh sound
    receipts. They accept only the real source-bound 9078-byte sample with
    period 334, stereo channel binding, start-before-title ordering, finish/wait
    before stop, and release before TITLE.C F0437 consumes C001; synthetic
    sound data, host audio-device emulation, and legacy swoosh wrappers are
    rejected.

  - 2026-07-16 CSB/ReDMCSB F0902 update:
    `F0902_DrawFTLLogo` now has a CSB-owned pre-title FTL-logo receipt before
    the swoosh path. It accepts only caller-bound original `Graphic_FTLLogo`
    data with the source 320x200 packed frame, 160-byte row stride, and
    16-color palette shape before `F0908_InitSound`; synthetic graphic bytes,
    synthetic palette data, and legacy logo wrappers are rejected.

  - 2026-07-16 CSB/ReDMCSB runtime coupling F0437/F0438/F0580/F0581 update:
    `F0437_STARTEND_DrawTitle`, `F0438_STARTEND_OpenEntranceDoors`,
    `F0580_ENTRANCE_DrawDoorAnimationStep`, and
    `F0581_ENTRANCE_BlitDoors` now have CSB-owned source-named runtime
    coupling receipts. They require all C001 title phases
    (PRESENTS/CHAOS zoom/CHAOS hold/STRIKES BACK), C017/C040 HUD capture,
    the 31-step C002/C003 door-opening route, receipt-only draw/input, real
    startup asset binding, and no legacy wrapper/fallback visual route before
    the source symbols are accepted. Remaining adjacent work is live adapter
    plumbing from the full CSB host-capture gate and manual Mac/app evidence,
    not synthetic title, HUD, or door pixels.

  - 2026-07-16 CSB/ReDMCSB F0579 update:
    `F0579_ENTRANCE_InitializeBitPlanes` now has a CSB-owned source-named
    startup bitplane-init receipt. It requires the real 256x161 composite-door
    bitmap and 320x200 screen bitplane geometry from `ENTRANCE.C`, consumes the
    verified title/HUD/door runtime coupling receipt, and rejects legacy
    bitplane wrappers or synthetic visual surfaces. Remaining adjacent work is
    live adapter plumbing from the full CSB host-capture gate and manual
    Mac/app evidence, not placeholder bitmap planes or generated door pixels.

  - 2026-07-16 DM1/CSB F1111/F1133-F1157 update:
    `F1111_CPSX`, `F1133_AddCopperInterrupt`,
    `F1134_RemoveCopperInterrupt`, `F1135_CopperInterrupt_CPSX`,
    `F1140_InitializeColorPaletteFullBlack`,
    `F1148_CustomExceptCode_CPSX`, `F1149_Init_CPSX`,
    `F1150_Free_CPSX`, and `F1157_BackupA5` are now explicitly
    dispositioned as Amiga-host Copper/CPSX/register platform boundaries.
    The DM1 descriptor/test records AMIGINIT.C, COPERINT.C, AMIGAVID.C,
    and COPYPROE.C evidence and rejects synthetic palette writes,
    interrupt handlers, copy-protection/disk state, or A5 register backup
    behavior.

  - 2026-07-16 CSB/ReDMCSB startup graphics F0474/F0477/F0478/F0479/F0488/F0490 update:
    the source-named startup graphics wrappers now have CSB-owned bounded
    no-data receipts for the GRAPHICS.DAT open/header/load/decompress/expand/
    close chain. The existing positive CSB startup runtime route remains the
    owner for real verified `GRAPHICS.DAT` data; these wrappers fail closed and
    record no-synthetic-fallback status instead of inventing archive bytes,
    decompressed payloads, IMAGE2/IMG3 pixels, or file handles. Remaining
    adjacent work is wiring any additional live title/HUD callsites through
    the real startup asset session, not adding placeholder graphics data.

  - 2026-07-16 DM1/CSB F0535/F0557-F0563 update:
    `F0535_MEMORY_GetGraphicsDatFileSize`,
    `F0557_SCROLLER_Initialize`, `F0558_SCROLLER_CancelInitialize`,
    `F0559_SCROLLER_Deinitialize`, `F0562_SCROLLER_Task`, and
    `F0563_SCROLLER_UpdateMessageArea` are now explicitly dispositioned as
    Amiga-host PC34 platform boundaries. The DM1 descriptor/test keeps asset
    loading and TEXT.C message/scroll routes separate and rejects synthetic
    GRAPHICS.DAT sizing, host scroller tasks, or message-area callbacks.

  - 2026-07-16 DM1/CSB F0513 update:
    `F0513_DIALOG_DrawGameReadyToPlay_Unreferenced` is now explicitly
    dispositioned as an unreferenced Amiga-host dialog platform boundary for
    PC34. The DM1 descriptor/test locks AMIGA.H:304 / DIALOG.C evidence and
    rejects any synthetic ready-to-play host dialog substitute until an exact
    source-backed PC3.4 route exists.

  - 2026-07-16 DM1/CSB F0537/F0544 update:
    `F0537_INPUT_ReleaseResources` and
    `F0544_INPUT_ResetPressingEyeOrMouth` are now explicitly dispositioned as
    Amiga-host input platform boundaries for PC34. The DM1 descriptor/test
    locks the source anchors and the absence of an evidenced I34E/I34M PC3.4
    route, so remaining adjacent work must use a real source-backed input
    route rather than a generic host-resource or mouse-release substitute.

  - 2026-07-16 DM1 F0323 update: `F0323_CHAMPION_Unpoison` is now exposed as
    the source-named callable and the existing compat wrapper delegates to it.
    Remaining adjacent work is regenerating/updating the ReDMCSB audit row and
    routing live kill/death call sites through the named boundary where useful.

  - 2026-07-16 DM1 F0216 update: `F0216_PROJECTILE_GetImpactAttack` is now a
    DM1-owned source-named callable and the projectile champion/precheck routes
    consume it instead of reassembling the `Attack`/`KineticEnergy` fallback
    locally. The audit disposition is closed through focused source/test
    evidence. Remaining adjacent work is the broader thrown-projectile
    render/materialization parity already tracked outside this scalar
    impact-attack boundary.

  - 2026-07-16 DM1 F0039 update: `F0039_OBJECT_GetIconIndexInSlotBox` now has
    a DM1-owned source-named callable over the existing PC34 `G0030` slot-box
    table. The Atari/PRIM `F039_aaaL_` alias is dispositioned to that mapping.
    Remaining adjacent work is live HUD/panel callers consuming the named
    boundary where useful; no new synthetic slot-box table was introduced.

  - 2026-07-16 DM1/CSB F0357 update:
    `F0357_COMMAND_DiscardAllInput` now has a DM1-owned source-named callable
    over the existing PC34 input-command queue discard route. Focused C11
    coverage proves regular queued commands are flushed while the
    release/stop reserved commands are compacted and preserved. Remaining
    adjacent work is only opportunistic live callsite migration to the
    source-named boundary; no new input queue or synthetic command behavior was
    introduced.

  - 2026-07-16 DM1 F0024/F0026/F0030 update:
    `F0024_MAIN_GetMinimumValue`, `F0026_MAIN_GetBoundedValue`, and
    `F0030_MAIN_GetScaledProduct` now have DM1-owned source-named callables.
    DM1 combat scaled-product math consumes the named F0030 boundary; remaining
    adjacent work is opportunistic migration of other local min/clamp comments
    where it reduces ambiguity without broad churn.

  - 2026-07-16 CSB/ReDMCSB F1075-F1078 update:
    `F1075_OpenLayersLibrary`, `F1076_CloseLayersLibrary`,
    `F1077_OpenConsoleDevice`, and `F1078_CloseConsoleDevice` now have
    CSB-owned source-named no-op PC34 host boundaries under `include/csb*` and
    `src/csb`. The focused direct C11 test locks wrapper/compat no-op
    stability plus AMIGINIT.C line-range evidence for the Amiga layers.library
    and console.device routes. Audit dispositions and focused CMake targets are
    added as platform boundaries; no portable host behavior or fallback device
    route is claimed.

  - 2026-07-16 CSB/ReDMCSB F1085-F1087 update:
    `F1085_IntuitionVectorReplacement`, `F1086_ReplaceIntuitionVectors`, and
    `F1087_RestoreIntuitionVectors` now have CSB-owned source-named PC34
    boundaries. F1085 returns the source-defined zero callback; F1086/F1087
    remain no-op host boundaries for Amiga Intuition vector replacement and
    restore. The focused direct C11 test locks the named/compat callables and
    AMIGINIT.C source evidence. No portable Intuition vector route is claimed.

  - 2026-07-16 CSB/ReDMCSB F1090-F1094 update:
    `F1090_GetCSBInternalErrorMessage` and
    `F1091_GetCSBSystemErrorMessage` now expose the exact mutable
    AMIGINIT.C DisplayAlert byte templates, including their coordinate bytes
    and continuation markers. `F1092_GetHexadecimalDigits` now uses the
    source lowercase hexadecimal table. `F1093_DisplayAlertCSBInternalError`
    and `F1094_DisplayAlertCSBSystemError` now mutate the real templates like
    ReDMCSB before stopping at the PC34 host-alert boundary. Remaining adjacent
    work is the broader AMIGINIT startup/library surface, not these templates.

  - 2026-07-16 CSB/ReDMCSB F1159-F1163 update:
    `F1159_Empty`, `F1160_USIO_04_Empty`, `F1161_USIO_05_Empty`,
    `F1162_USIO_06_HidePointer`, and `F1163_USIO_07_ShowPointer` now have
    CSB-owned source-named PC34 boundaries. The focused direct C11 test locks
    the no-op stability and USIOMAIN.C/USIO1.C source evidence for the empty
    USIO vectors and pointer visibility routes. No host cursor route or input
    queue data is synthesized.

  - 2026-07-16 CSB/ReDMCSB F1168/F1170/F1171/F1305/F1307 update:
    `F1168_USIO_18_Empty`, `F1170_USIO_03_Expunge`,
    `F1171_USIO_19_LockDF0`, `F1305_OpenFTLLibrary`, and
    `F1307_FIO1_03_Expunge` now have CSB-owned source-named PC34 platform
    boundaries. The focused direct C11 test locks no-op stability plus
    USIO2.C/FIO1MAIN.C evidence for empty, expunge, DF0 lock, and FIO1 library
    routes. No queue or cursor data is synthesized here.

  - 2026-07-16 CSB/ReDMCSB F1164-F1167 update:
    `F1164_USIO_15_GetFirstQueuedUsioDataType`,
    `F1165_USIO_17_WaitUntilKeyboardOrMouseInput`,
    `F1166_USIO_16_ExtractFirstUsioDataFromQueue`, and
    `F1167_USIO_14_GetMouseStatus` now have CSB-owned bounded PC34
    boundaries over the ReDMCSB USIO2.C queue and mouse-status contracts.
    The focused direct C11 test covers empty queue, invalid index,
    source-named fail-closed extraction/wait behavior, F1167 no-host
    preservation, and source evidence without creating synthetic disk,
    keyboard, mouse, or queue input data. Remaining adjacent work is live
    host-input pumping and positive real input capture before any queue
    population can be claimed.

  - 2026-07-16 CSB/ReDMCSB FIO1 trackdisk boundary update:
    `F1106_IsTrackdiskDeviceOpened`, `F1107_GetDiskChangeCounter`,
    `F1109_GetDiskState`, and `F1114_CloseTrackdiskDevice` now have
    CSB-owned source-named PC34 boundaries over the ReDMCSB FLOPPYAM.C/FIO1.C
    trackdisk contract. The focused direct C11 test locks no-host behavior:
    no opened trackdisk device, disk-change counter `-1`, disk state `0`
    (source default: no disk), and no-op close. Remaining adjacent work is
    real host/media evidence before any positive disk, write-protection, or
    change-counter behavior can be claimed.

  - 2026-07-16 DM2 cache/mement free update: `FREE_CACHE_INDEX`,
    `FREE_INDEXED_MEMENT`, `FREE_TEMP_CACHE_INDEX`, and `FREE_PICT6` are now
    closed through skproject-backed bounded receipts. Remaining adjacent work
    is the broader CPX heap/mement chain owner, full `QUERY_RECT` codec, and
    real GDAT image-buffer ownership.

  - 2026-07-16 DM2 GDAT scalar/raw update: `dtWordValue` and `dtImageOffset`
    entries are now blocked from generic raw-payload lookup and remain
    available only through typed scalar APIs. Remaining adjacent work is
    applying the same source-owned separation to every live GDAT HUD/dungeon
    consumer.

  - 2026-07-16 DM2 xrect codec update: `READ_WORD`, `DM2_COMPRESS_RECTS`,
    and `DM2_QUERY_RECT` are now closed through skproject-backed bounded
    receipts. Firestaff now compresses raw4 rectangle groups into source-shaped
    rnodes, expands common/byte/word rect fields, rejects malformed tables, and
    uses no synthetic rect data. Remaining adjacent work is live UI/button-group
    consumers and broader GDAT HUD/dungeon wiring.

  - 2026-07-16 DM2 rect/cursor update: `DM2_OFFSET_RECT`,
    `OFFSET_RECT`, `PT_IN_RECT`, and `PTR_ADVANCE` are now closed through
    skproject-backed bounded receipts. Remaining adjacent work is UI/button-
    group rect adjustment and live consumers, not synthetic geometry.

  - 2026-07-16 DM2 update: `DM2_dballoc_3e74_24b8`,
    `DM2_dballoc_3e74_2162`, and `DM2_LOAD_DYN4` are now closed through
    skproject-backed allocation/filter receipts. Remaining adjacent DM2 work
    is full GDAT HUD/dungeon rendering and real save corpus, not synthetic
    menu/dungeon substitutes.

  - 2026-07-16 DM2 GDAT allocator helper update: `R_2BAD4` and `R_2D07D`
    are now closed through skproject-backed bounded receipts. Remaining
    adjacent work is still full real GDAT HUD/dungeon image-buffer ownership.

  - 2026-07-16 DM2 CPX compaction update: `R_2D8AD`, `R_2D8BA`, and
    `R_2D802` are now closed through skproject-backed bounded receipts for
    top-down CPX reservation, header-inclusive copy-span accounting, and
    active-block compaction with high-bit/free block skips. Remaining adjacent
    work is real CPX heap integration and live decoded GDAT image-buffer
    ownership, not synthetic payload copying.

  - 2026-07-16 DM2 palette/GDAT alias update: SKWIN
    `QUERY_GDAT_ENTRY_DATA_PTR` and `TRANSLATE_PALETTE` are now mapped to
    the existing real GDAT data-pointer and 256-byte dt07 palette-translation
    helpers. Remaining adjacent work is wider live palette capture across the
    full HUD/dungeon route.

  - 2026-07-16 DM2 mementi update: `FIND_FREE_MEMENTI` now has a
    skproject-backed cache-state receipt for next-free selection, fallback
    recycle, allocation count, and referenced-slot skipping. Remaining
    adjacent work is still full CPX/image-buffer ownership, not synthetic
    texture allocation.

  - 2026-07-16 DM2 inventory update: `EQUIP_ITEM_TO_INVENTORY` now has a
    skproject-backed receipt for OBJECT_NULL rejection, ObjectID direction-bit
    clearing, champion inventory placement, current-container overlay routing,
    and the `PROCESS_ITEM_BONUS` handoff boundary. Remaining adjacent work is
    real bonus semantics and broader live inventory interaction.

  - 2026-07-16 DM2 scalar/container possession update: `IS_NEGATIVE`,
    `IS_CONTAINER_MAP`, `DM2_IS_CONTAINER_MAP`,
    `FIND_POUCH_OR_SCABBARD_POSSESSION_POS`, and
    `DM2_FIND_POUCH_OR_SCABBARD_POSSESSION_POS` now have skproject-backed
    bounded receipts. Remaining adjacent work is full live item-action and
    container-map HUD consumption from real GDAT/record data.

  - 2026-07-16 DM2 movement update: `DM2_move_12b4_0d75`,
    `DM2_move_075f_0af9`, and `DM2_move_2fcf_0b8b` now have narrow
    skproject-backed handoff receipts. Remaining adjacent movement work is
    still full `DM2_PERFORM_MOVE`, live map/record mutation, missile deletion,
    and runtime dungeon rendering with real GDAT state.

  - 2026-07-16 DM2 render update: `DM2_sub_blit_specialeffects` now has a
    narrow skproject-backed special-effects blit receipt. Remaining adjacent
    work is wiring the receipt into real GDAT/HUD/dungeon pixel consumers and
    avoiding any fallback/synthetic blit output.

  - 2026-07-16 DM2 GUI update: `DM2_DRAW_CMD_SLOT`, `DRAW_CMD_SLOT`,
    `DM2_DRAW_CHARSHEET_OPTION_ICON`, and `DRAW_CHARSHEET_OPTION_ICON` now
    have narrow skproject-backed GUI draw receipts. Remaining adjacent work is
    connecting these receipts to live HUD/menu drawing with real GDAT assets.

  - 2026-07-16 DM2 GUI/container update: `DM2_DRAW_MONEYBOX`,
    `DRAW_MONEYBOX`, `DM2_DRAW_ITEM_STATS_BAR`, `DRAW_ITEM_STATS_BAR`,
    `DM2_DRAW_CONTAINER_PANEL`, `DRAW_CONTAINER_PANEL`,
    `DM2_DRAW_CONTAINER_SURVEY`, `DRAW_CONTAINER_SURVEY`,
    `DM2_DRAW_ITEM_ON_WOOD_PANEL`, and `DRAW_ITEM_ON_WOOD_PANEL` now have
    narrow skproject-backed receipts. Remaining adjacent work is live
    HUD/menu/container pixel consumption from real GDAT, not synthetic
    container art.

  - 2026-07-16 DM2 champion/HUD panel update: `DM2_DRAW_CUR_MAX_HMS`,
    `DRAW_CUR_MAX_HMS`, `DM2_DRAW_PLAYER_3STAT_TEXT`,
    `DM2_DRAW_PLAYER_3STAT_PANE`, `DM2_DRAW_FOOD_WATER_POISON_PANEL`,
    `DRAW_FOOD_WATER_POISON_PANEL`, `DM2_DRAW_CRYOCELL_LEVER`,
    `DRAW_CRYOCELL_LEVER`, `DM2_DRAW_EYE_MOUTH_COLORED_RECTANGLE`, and
    `DRAW_EYE_MOUTH_COLORED_RECTANGLE` now have narrow skproject-backed
    receipts. Remaining adjacent work is live panel pixel consumption from
    decoded GDAT and broader runtime HUD wiring.

  - 2026-07-16 DM2 blit/text alias update: `DM2_DRAW_ICON_PICT_BUFF`,
    `DRAW_ICON_PICT_BUFF`, `DRAW_DEF_PICT`, `DRAW_GRAY_OVERLAY`,
    `DRAW_NAME_STR`, `DRAW_GUIDED_STR`, and `DRAW_LOCAL_TEXT` now have
    skproject-backed narrow receipts or verified text-route aliases. Remaining
    adjacent work is actual decoded-picture blit ownership in live GDAT
    dungeon/HUD consumers.

  - 2026-07-16 DM2 querydb text update: `DM2_QUERY_GDAT_ITEM_NAME`,
    `QUERY_GDAT_ITEM_NAME`, `DM2_QUERY_CMDSTR_NAME`, `QUERY_CMDSTR_NAME`,
    `DM2_QUERY_CMDSTR_ENTRY`, `QUERY_CMDSTR_ENTRY`, and
    `DM2_QUERY_CUR_CMDSTR_ENTRY` now read real `dtText` rows through the asset
    loader. Remaining adjacent work is broader live command-string consumers.

  - 2026-07-16 DM2 source-name helper update: `getSpellTypeName`,
    `getSkillName`, and `getStatBonusName` now have bounded source-name
    receipts over existing DM2 spell-type, base-skill, and champion stat
    constants. Unknown values fail closed without fallback labels. Remaining
    adjacent name-helper work includes `getXActrName`, `printDistMap`, and
    broader live HUD/debug consumers where exact source tables are available.

  - 2026-07-16 DM2 magic-map/attack-dir update: `DM2_DRAW_MAJIC_MAP`,
    `DRAW_MAJIC_MAP`, `DM2_DRAW_PLAYER_ATTACK_DIR`,
    `DRAW_PLAYER_ATTACK_DIR`, `DRAW_PLAYER_3STAT_PANE`, and
    `DRAW_PLAYER_3STAT_TEXT` now have skproject-backed receipts or aliases.
    Remaining adjacent work is full live pixel rendering for the magic map.

  - 2026-07-16 DM2 HUD/dialog/item GUI update:
    `DM2_DRAW_ICON_PICT_ENTRY`, `DM2_DRAW_DIALOGUE_PROGRESS`,
    `DM2_DRAW_DIALOGUE_PARTS_PICT`, `DM2_DRAW_DIALOGUE_PICT`,
    `DM2_DRAW_WAKE_UP_TEXT`, `DM2_DRAW_PLAYER_3STAT_HEALTH_BAR`,
    `DM2_DRAW_PLAYER_NAME_AT_CMDSLOT`, `DM2_DRAW_PLAYER_DAMAGE`,
    `DM2_DRAW_SPELL_TO_BE_CAST`, `DM2_DRAW_SPELL_PANEL`,
    `DM2_DRAW_ITEM_IN_HAND`, `DM2_DRAW_ITEM_ICON`, `DM2_DRAW_ITEM_SURVEY`,
    `DM2_DRAW_HAND_ACTION_ICONS`, and their covered SKWIN aliases now have
    narrow skproject-backed receipts. Remaining adjacent work is full live
    decoded GDAT pixel consumption in HUD/menu/dungeon.

  - 2026-07-16 DM2 HUD survey/attack-result update:
    `MONEY_BOX_SURVEY`, `DM2_MONEY_BOX_SURVEY`, `SHOW_ATTACK_RESULT`, and
    `DM2_SHOW_ATTACK_RESULT` now have source-named bounded HUD receipts over
    the existing moneybox/container-survey/player-damage GDAT draw routes.
    Remaining adjacent work is wiring these receipts into live panel redraw and
    decoded GDAT pixel blits, without synthetic menu or container art.

  - 2026-07-16 DM2 SKWIN HUD alias update:
    `DM2_DRAW_SQUAD_SPELL_AND_LEADER_ICON`,
    `DRAW_SQUAD_SPELL_AND_LEADER_ICON`, `DRAW_SQUAD_POS_INTERFACE`,
    `DRAW_POWER_STAT_BAR`, `DRAW_SCROLL_TEXT`, `DRAW_SIMPLE_STR`, and
    `DRAW_SKILL_PANEL` now have narrow skproject-backed receipts. Remaining
    adjacent work is live GDAT pixel consumption for the full HUD/menu/dungeon
    render path.

  - 2026-07-16 DM2 record-address update: `DM2_GET_ADDRESS_OF_RECORD`,
    `GET_ADDRESS_OF_RECORD`, typed `GET_ADDRESS_OF_RECORD0` through
    `GET_ADDRESS_OF_RECORDF`, `GET_ADDRESS_OF_RECORDX4`,
    `GET_ADDRESS_OF_GENERIC_CONTAINER_RECORD`, `GET_ADDRESS_OF_ACTU`, and
    `GET_ADDRESS_OF_DETACHED_RECORD` now have skproject-backed bounded
    address receipts. Remaining adjacent work is replacing more dungeon/HUD
    consumers with these real record-address routes.

  - 2026-07-16 DM2 tile/fill update: `GET_ADDRESS_OF_TILE_RECORD`,
    `GET_TILE_VALUE`, `FILL_ENTIRE_PICT`, and `FILL_RECT_SUMMARY` now have
    skproject-backed bounded receipts. Remaining adjacent work is wiring more
    live dungeon/HUD consumers to these tile and fill routes.

  - 2026-07-16 DM2 item/container classifier update: `GET_ITEM_NAME`,
    `IS_MISCITEM_CURRENCY`, `IS_CONTAINER_MONEYBOX`, `IS_CONTAINER_CHEST`,
    `GET_ITEM_ORDER_IN_CONTAINER`, `FMT_NUM`, and their SKULLWIN aliases now
    have skproject-backed receipts. Remaining adjacent work is live inventory
    and moneybox HUD consumption with real GDAT text/order data.

  - 2026-07-16 DM2 text/fill/mouse wrapper update: `FILL_STR`,
    `DRAW_STRONG_TEXT`, `HIGHLIGHT_ARROW_PANEL`,
    `IBMIO_FILL_HALFTONE_RECT`, `FIRE_FILL_HALFTONE_RECTV`,
    `FIRE_FILL_HALFTONE_RECTI`, `IBMIO_MOUSE_RELEASE_CAPTURE`,
    `FIRE_MOUSE_RELEASE_CAPTURE`, and SKULLWIN aliases
    `DM2_FILL_STR`, `DM2_FILL_HALFTONE_RECTV`,
    `DM2_FILL_HALFTONE_RECTI`, and `DM2_MOUSE_RELEASE_CAPTURE` now have
    skproject-backed receipts. Remaining adjacent work is live menu/HUD
    pixel consumption and broader GDAT dungeon rendering.

  - 2026-07-16 DM2 string helper update: `DM2_SKCHR_TO_SCRIPTCHR`,
    `DM2_LTOA10`, `SK_STRLEN`, `SK_STRSTR`, `SK_LTOA10`, `SK_STRCPY`,
    and `SK_STRCAT` now have skproject-backed receipts. Remaining adjacent
    work is broader command-string parsing and live consumers.

  - 2026-07-16 DM2 querydb real-text promotion: command-string and item-name
    receipts in `dm2_v1_asset_loader` are now promoted to verified mappings
    over real GDAT `dtText`, and container order parsing has a real
    CONTAINERS field-0x40 asset-loader test. Open count is unchanged because
    the affected rows were already outside the MISSING/UNCERTAIN queue.

  - 2026-07-16 DM2 c_map/c_record scalar update: `tile_to_ulong`,
    `tile_to_ubyte`, `mk_record`, `record_to_word`, and `record_to_long` now
    have skproject-backed dungeon-loader receipts. Remaining adjacent work is
    larger `DM2_ARRANGE_DUNGEON`, `DM2_PERFORM_MOVE`, and c_map runtime
    mutation/rendering.

  - 2026-07-16 DM2 helper-disposition update: 20 already implemented
    skproject-backed helper symbols were closed through
    `SYMBOL_DISPOSITIONS.tsv` with focused test evidence. Covered rows include
    FIRE/IBMIO row blits, mouse cursor/queue/pattern helpers, ability/bar/tile
    predicates, level/CPX/game-state helpers, wall ornate alcove lookup, and
    `GRAPHICS_DATA_OPEN`. Remaining adjacent work is live GDAT HUD/dungeon
    pixel consumption and the broader high-priority runtime symbols still
    shown by `python3 tools/symbol_backlog.py --game DM2 --limit 30`.

  - 2026-07-16 DM2 p130 helper-disposition update:
    `IS_MISSILE_VALID_TO_LAUNCHER`, `REMOVE_POSSESSION`,
    `LOAD_PROJECTILE_TO_HAND`, `PUT_OBJECT_INTO_CONTAINER`,
    `PROCESS_TIMER_0E`, `PROCEED_GLOBAL_EFFECT_TIMERS`,
    `SET_TILE_ATTRIBUTE_02`, and `SUMMARIZE_STONE_ROOM` are now closed through
    existing skproject-backed helper code and focused direct tests. Remaining
    adjacent work is live runtime adoption for these receipts and the larger
    still-open p130 entries such as `DM2_ARRANGE_DUNGEON`,
    `DM2_PERFORM_MOVE`, `TRANSLATE_PALETTE`, and right-panel refresh.

  - 2026-07-16 DM2 projectile-impact move update:
    `DM2_move_075f_06bd` is now closed through the existing source-named
    projectile impact attack receipt and focused C11 test. Remaining adjacent
    movement helpers that include `dm2_v1_dungeon_loader.h` stay open until
    the current dungeon-loader header transition is compile-clean again.

  - 2026-07-16 DM2 memory/mement helper update: `ZERO_MEMORY` and
    `ValidateMements` now have source-named bounded receipts for caller-owned
    memory clearing and mement table validation. Remaining adjacent work is
    wiring the validator into the live CPX/GDAT mement owner without adding
    synthetic cache or picture data.

  - 2026-07-16 DM2 palette driver disposition update: skproject
    `driver_setcolors` is now closed through the existing
    `dm2_v1_skproject_core` palette receipts and focused strict C11 coverage.
    Remaining adjacent work is still the real GDAT/HUD palette path blocked by
    the current asset-loader/header transition, plus `TRANSLATE_PALETTE` live
    consumer routing.

  - 2026-07-16 DM2 HUD panel-routing update: `QUERY_CMDSTR_TEXT`,
    `DM2_QUERY_CMDSTR_TEXT`, `TRANSMIT_UI_EVENT`, `DM2_TRANSMIT_UI_EVENT`,
    `UPDATE_RIGHT_PANEL`, and `DM2_UPDATE_RIGHT_PANEL` now have focused
    skproject-backed receipts. Command text must come from caller-provided real
    dtText bytes and command events cannot synthesize labels. Remaining work is
    live right-panel pixel redraw against decoded GDAT HUD assets.

  - 2026-07-16 DM2 HUD/item helper disposition update:
    `IS_ITEM_HAND_ACTIVABLE`, `RETRIEVE_ITEM_BONUS`, `PROCESS_ITEM_BONUS`,
    `QUERY_PLAYER_SKILL_LV`, and `REFRESH_PLAYER_STAT_DISP` are now closed
    through existing skproject-backed helpers and direct tests. Remaining work
    is live HUD/panel adoption and decoded GDAT pixel redraw, not synthetic
    action/stat content.

  - 2026-07-16 DM2 PICT/missile helper disposition update:
    `GET_MISSILE_REF_OF_MINION`, `QUERY_PICT_BITS`, `QUERY_PICST_IMAGE`, and
    `QUERY_PICST_IT` are now closed through focused skproject-backed helper
    tests. Remaining adjacent work is live GDAT image-buffer ownership,
    dungeon/HUD pixel consumption, and broader missile/minion runtime adoption.

  - 2026-07-16 Theron update: a verifier now turns instrumented Mednafen
    FIFO-origin logs into the required original consumer markers. Remaining
    work is running it against a real captured original session and feeding
    the marker output into `FIRESTAFF_THERON_ORIGINAL_CONSUMER_TRACE`.

  - 2026-07-16 update: `docs/reference/audits/SYMBOL_DISPOSITIONS.tsv` is
    now the required evidence file for closing symbol rows as implemented,
    external/host-owned, or source-nonapplicable without code work. The file
    is intentionally empty except for its schema until each disposition has
    real source evidence. Keep rows in TODO until either Firestaff code maps
    them or a reviewed disposition row excludes them from the open queue.

  - 2026-07-16 ReDMCSB platform-boundary disposition update: the callable
    queue now closes reviewed `NON_APPLICABLE` platform rows from
    `REDMCSB_MISSING_PLATFORM_BOUNDARIES.tsv` and all Atari ST/PRIM ABI aliases
    through `SYMBOL_DISPOSITIONS.tsv`. This removes 324 nonportable rows from
    the DM1/CSB open backlog without adding runtime shims or synthetic
    behavior. FIO/PRIM, host input, media I/O, and runtime families remain
    open until they have source-backed Firestaff contracts.

  - 2026-07-16 DM1 update: the next ReDMCSB callable-symbol bundle before
    CHAMPION is closed through source-backed dispositions for `F0230`,
    `F0252`-`F0258`, `F0260`, `F0262`, `F0263`, `F0266`, both `F0267`
    rows, and `F0514`. The closures point at existing DM1 runtime/helper
    code and focused tests. Narrow rows remain honest boundaries: F0253 is
    closed only for the bounded C11/F0259 action-enable handoff, F0256 only
    for the typed C53 original-save/runtime receipt with no copy-protection
    side effect, F0258 only through the F0259 quiver movement planner, F0260
    only at the HUD receipt boundary, and F0266/F0267 only for the currently
    implemented movement/projectile route breadth.

  - 2026-07-16 DM1 CHAMPION/HUD update: `F0287`-`F0290` are now closed
    through the DM1 champion-panel HUD owner. The shared `F0288` integer
    formatter is used by status/stat/load text, `F0289`/`F0290` consume it
    for HP/stamina/mana including stamina /10 and C550/C551/C552 routing,
    and `F0287` remains covered by the PC34 bar pixel-band model. Remaining
    adjacent work is live original/Mac pixel capture, not alternate fonts or
    host-scaled HUD text.

  - 2026-07-16 DM1 CHAMPION slot/portrait update: `F0293`, `F0295`-`F0302`,
    their Atari ST ABI aliases where present, and the remaining `F292_arzz_`
    alias are now closed through source-backed narrow dispositions over the
    existing DM1 champion-panel state redraw, changed-object-icon refresh,
    leader-hand/slot transfer, inventory/chest owner-change, and slot-box
    dispatcher receipts. Honest boundaries remain: `F0293` is a contract loop
    around F0292 rather than a fresh bitmap renderer, `F0299` covers only the
    Rabbit's Foot C30 chest exclusion plus modifier-apply cadence in the
    modeled owner-change route, and the broader all-item modifier matrix plus
    original pixel parity remain open.

  - 2026-07-16 DM1 CHAMPION skill/stamina/load update: `F0303`-`F0310`
    and their listed Atari ST ABI aliases are now closed through source-backed
    DM1 skill-experience, throw/shoot stamina, champion-stat, and combat
    receipts. Narrow boundaries remain deliberate: `F0303`/`F0304` cover the
    tested skill query/XP mutation paths rather than every live award site,
    `F0305` keeps object-weight ownership with object data callers, and
    `F0307`/`F0308` do not claim broader all-callers RNG replay.

  - 2026-07-16 DM1 CHAMPION stat/wake/scent/death update: `F0311`-`F0319`
    are now closed except no `F0323` row exists in this queue, with listed
    aliases through `F317_adzz_`. `F0315`-`F0317` now have a source-backed
    DM1 champion-needs scent owner for 1-based lookup, zero-based
    delete/shift/window adjustment, and existing-scent strength updates.
    Live Footprints/Thieves Eye/group smell consumers and movement scent
    append remain route-owned.

  - 2026-07-16 DM1 CHAMPION damage/poison/stamina update: `F0320`,
    `F0321`, `F0322`, `F0324`, `F0325`, and aliases `F320_akzz_`,
    `F321_AA29_`, `F322_lzzz_`, `F324_aezz_`, and `F325_bzzz_` are now
    closed through DM1 combat/action-tail receipts. Narrow boundaries remain:
    F0320 covers pending wound/damage mutation but not the damage-number
    bitmap or C12 hide-event renderer, F0321 covers the tested combat damage
    pipeline rather than every live projectile/caller variant, F0322 keeps
    panel redraw/poison UI route-owned, and F0325 models the stamina mutation
    plus pending-damage magnitude while callers own the actual enqueue/redraw
    side effects.

  - 2026-07-16 DM1 CHAMPION projectile/action-time update: `F0326`,
    `F0327`, `F0328`, `F0330`, `F0331`, and aliases `F326_ozzz_`,
    `F327_kzzz_`, `F328_nzzz_`, `F330_szzz_`, and `F331_auzz_` are now
    closed through source-backed DM1 throw/shoot, action-tail, scheduler,
    champion-needs, and clock-tick receipts. Narrow boundaries remain:
    projectile queue/render and deeper impact behavior stay route-owned,
    F0327 covers the tested spell-launch handoff rather than every live
    projectile insertion route, and `F0329` remains open because the strongest
    leader-hand throw proof is currently M11/engine-owned rather than a clean
    DM1-only owner for this pass.

  - 2026-07-16 DM1 portrait/bar-graph audit update: `F0515`, `F0516`,
    `F2104`, `F2105`, and `S0287` are now closed through focused portrait
    planar-conversion and champion bar-graph tests. File/save envelopes,
    arbitrary editor bitmap dimensions, and live bitmap blitter sequencing
    remain route-owned; the existing F0329 M11-owned mapping is not changed by
    this audit pass.

  - 2026-07-16 DM2 querydb equipment update: `DM2_QUERY_CREATURES_ITEM_MASK`,
    `QUERY_CREATURES_ITEM_MASK`, `DM2_IS_ITEM_FIT_FOR_EQUIP`, and
    `IS_ITEM_FIT_FOR_EQUIP` are now closed through real parsed GDAT text/word
    rows in the DM2 asset loader. The remaining querydb rows stay open until
    they have source-backed code or explicit non-applicability evidence.

  - 2026-07-16 DM2 GDAT interface update: `DM2_LOAD_GDAT_INTERFACE_00_0A`
    is now closed through the explicit Rect14 receipt over real
    INTERFACE_GENERAL dt07/0x0A rows and the existing runtime host placement
    proof. Adjacent GDAT loader rows remain open until they have the same
    named source-symbol boundary.

  - 2026-07-16 Nexus update: MENU.BPK PRS3 V3 now has a reviewed upload-path
    receipt that binds sidecars, ledger and producer attestation. Full runtime
    upload remains open until independent Saturn capture authentication and
    decoder promotion are proven.

  - 2026-07-16 Nexus PRS3/Structure2 intake update: the real retail
    `MENU.BPK` and `LEV00.DGN` corpus now has a joined no-draw admission
    receipt for BPPK/BMPD directory framing, 162 PRS3 stream plans, the opaque
    PALT trailer, and Structure2 descriptor/payload anchors. It intentionally
    keeps PRS3 decode, Structure2 pixel spans, palette addressing, M11 render,
    and fallback visuals blocked until an authenticated Saturn decoder/capture
    proves the opcode, texel, palette, and VDP1 semantics.

  - 2026-07-16 Nexus PRS3 decoder reverse-admission update: retail `DM.BIN`
    SH-2 V1 loader operations are now joined to real `MENU.BPK` PRS3 stream
    plans before decoder admission. The gate proves the nonzero byte path,
    zero-side two-byte merge corridor, and no direct zero-side output store,
    then differentially rejects the current LSB/MSB trial decoders across the
    real MENU.BPK corpus. Decoder promotion and Structure2 pixel/palette intake
    remain blocked until a source-bound expected-output sidecar, authenticated
    Saturn provenance, and reviewed opcode grammar all exist.

  - 2026-07-16 Nexus PRS3 loader control-flow update: retail `DM.BIN` callee
    `85376` now has a source-bound control-flow receipt that names the source
    cursor, output base/index, remaining-source counter, control word/mask,
    zero compare, and repeat registers around `85450`/`85460`/`85464`.
    The receipt binds the nonzero source-byte-to-output-store path and the
    zero-side two-source-byte merge/indexed-output-window corridor to the real
    162-stream `MENU.BPK` plan. DMWeb-compatible bounded decompression now
    covers the zero-side copy semantics and expected output lengths; remaining
    gaps are authenticated Saturn execution provenance, pixel/mode meaning,
    palette and VDP1 placement.

  - 2026-07-16 Nexus PRS3 SH-2 subset-trace update: a strict, retail-byte
    subset executor now runs the bound `DM.BIN` V1 loader control corridor
    over real `MENU.BPK` entry 1 body bytes and records dynamic R12/R13/R6/
    R11/R14 observations. The real run consumes all 140 body bytes, observes
    82 original nonzero output stores, 22 zero-side merges, 125 indexed
    zero-side reads, and stable output/control fingerprints, but it reaches
    `blocked-execution` before the declared 240-byte indexed output vector. This is
    an authenticated-code negative blocker, not a decoder: positive vectors
    still require an independently authenticated Saturn/emulator trace or an
    equally reviewed execution proof that explains zero-side output/copy
    semantics.

  - 2026-07-16 Nexus PRS3 SH-2 subset-vector update: the subset executor now
    models the missing real output stores (`2a20`, `2310`, `2a10`) and R10
    linear output pointer alongside the R13/R6 history window. `MENU.BPK`
    entry 5 now produces a full 1674-byte output vector from its own 560-byte
    real stream with stable FNV `290a9d13c0224cc6`, proving the zero-side
    copy/store path for that bounded vector. Entry 1 still stops at 237/240
    bytes from its own real span. Remaining work is an independently
    authenticated Saturn/emulator trace and reviewed opcode/pixel/palette ABI
    before PRS3 output can be promoted into Structure2 intake or rendering.

  - 2026-07-16 Nexus PRS3/Structure2 ABI update: the full entry 5 output
    vector now reaches a dedicated Structure2/PALT ABI gate beside real
    `LEV00.DGN`. The receipt binds entry 5's 54x31x1 output vector, PALT raw
    256-entry trailer hash, and LEV00's 82 Structure2 descriptors/anchors,
    while proving that loose auth booleans cannot open pixel submit, palette
    submit, M11 handoff, or fallback. Remaining work is still a real
    independent Saturn/emulator trace or equivalent reviewed capture that
    proves VDP1 consumer semantics, pixel format, palette application, and
    Structure2 placement for this output.

  - 2026-07-16 Nexus LEV00 scene geometry update: a new DGN scene/runtime-plan
    consumer now binds the real `LEV00.DGN` source identity and party/camera
    adjacent-cell route before any mesh promotion. On the verified retail
    LEV00 bytes it fails closed at `blocked-mesh-entry`: the parser exposes
    the 64x64 Structure1B cell envelope, but no bounded Structure1F-owned
    Structure3 mesh entry can be materialized from LEV00 for this consumer.
    Texture submit, raster submit, M11 handoff, fallback geometry, and
    fallback visuals remain denied. Remaining work is to move the same
    consumer to a DGN level/source route with positive Structure1F/Structure3
    mesh rows or supply original Saturn geometry trace evidence for LEV00.

  - 2026-07-16 Nexus PRS3/VDP1 consumer evidence update: the entry 5
    Structure2/PALT ABI now feeds a dedicated VDP1-consumer capture/evidence
    gate. It retains the exact 1674-byte PRS3 output hash, raw BE16 PALT hash,
    and LEV00 Structure2 descriptor counts as the capture target while
    requiring retail media, Saturn emulator, Saturn BIOS, authenticated raw
    trace, DM.BIN/MENU.BPK/LEV00 binding, VDP1 command, texture-window,
    BE16 palette-application, descriptor-selection, and placement lanes.
    Local preflight found Mednafen's `ss` module but no Saturn BIOS or raw
    Nexus VDP1 trace sidecar, so pixel format, palette application,
    Structure2 submit, M11 handoff, and fallback visuals remain blocked.

  - 2026-07-16 DM2 door querydb update: `DM2_GET_DOOR_STAT_0X10`,
    `DM2_GET_GRAPHICS_FOR_DOOR`, and `DM2_query_0cee_3275` are now closed
    through real DOORS dtWordValue rows. Broader live door/HUD consumers remain
    open until they consume these values through source-shaped runtime paths.

  - 2026-07-16 Nexus update: Structure3 DGN raw capture now has a reviewed
    material-upload gate binding capture, Saturn attestation, package/host
    route and producer workflow. Full DGN material semantics and runtime
    renderer handoff remain open.

  - 2026-07-16 Nexus DGN runtime materialization update: the production
    render capture plus reviewed PRS3/BPK/Structure1F palette, texel order,
    VDP1 command, and material semantics before visible dungeon rendering can
    open.

  - 2026-07-20 Nexus DGN mesh extractor update: the Structure3 mesh entry
    unit face/normal pairs, and the edge corpus against retail LEV00-LEV15.
    Face-plane coherence semantics, material raster, and geometry readiness
    remain open capture-bound work.

  - 2026-07-20 Nexus DGN geometry readiness follow-up: the
    test's engine-field setup (plan status stays
    `BLOCKED_STRUCTURE2_SOURCE`/`MISSING`, zero commands). Diagnosis
    recorded; the engine route-admission chain needs its own round.
    CHECK with the designed zero-command no-draw outcome. Remaining:

  - 2026-07-20 Nexus DGN material raster route-admission trace (round 14,
    three. Remaining: authenticated Saturn PRS3/VDP1 capture replay plus a
    reviewed Structure1B selector-transform proof before any of the four
    assertions can move.

  - 2026-07-16 DM1 CHAMPION pre-HUD update: `F0280`, `F0281`, `F0283`
    through `F0286`, and their Atari ST ABI aliases where present are now closed
    through existing DM1 resurrection, rename, party-direction, and target
    selection receipts. Narrow boundaries remain deliberate: `F0280` covers
    the source C080/C127 Hall portrait route, candidate append/ordinal
    publication, and a source-proven candidate text/stat materialization slice
    over caller-owned name/title plus A..P encoded vitals/statistics; `F0281`
    covers the reincarnate rename UI gate/input rules; `F0285` is closed only
    through the `F0286` ordered-cell living champion scan.

  - 2026-07-16 DM1 HoC mirror runtime update: live C127 mirror selection now
    consumes a DM1-owned F0871 receipt that requires the original mirror record
    and C026 portrait atlas before M11 opens C040, and C160/C161 finalization
    consumes F0872 so resurrect commits directly while reincarnate must pass
    through the real rename gate before world/HUD/save state is finalized.
    Real-data coverage now discovers two original HoC mirrors, verifies C026
    portrait bytes, disables the source mirror sensors, renames/reincarnates
    the second candidate, and quicksave-resumes the committed party. Remaining
    adjacent work is inscription/M648 redraw capture breadth and broader
    packaged PC/Mac capture, not synthetic mirror candidates or fallback HoC
    panels.

  - 2026-07-17 DM1 HoC C127 click-cell closure: M11 now carries the exact
    F0172-visible packed C127 wall cell into the F0280 selection receipt,
    instead of substituting cell zero after the source-backed D1C hit test.
    This keeps visible HoC mirrors selectable through their original raw
    sensor/click pairing and rejects missing or mismatched front receipts.

  - 2026-07-17 HoC pointer-sweep update: the real-data C127 runtime test now
    walks every map-0 source sensor chain, derives each front pose from the
    packed source wall cell, and clicks the M11-provided D1C zone. All 24
    locally staged source candidates reject an adjacent out-of-zone click and
    then reach the F0280 candidate panel through the positive click; no HoC
    coordinate table, champion list, or fallback route participates.

  - 2026-07-16 DM1 HoC C160/C161/C162 panel-close redraw update: the focused
    M11 HoC close regression is now source-strict around F0128/F0107
    invalidation. After C160 closes C040, M11 proves a fresh GRAPHICS.DAT M648
    repaint on a real source TextString and separately proves the now-disabled
    mirror wall publishes clear-only M648 when its original thing chain has no
    visible C02 TextString. Remaining adjacent work is broader PC/Mac capture
    over more HoC wall tuples, not fallback fonts, retained glyphs, or
    synthetic post-mirror text.

  - 2026-07-16 DM1 CHAMPION HoC/reset update: `F0278`, `F278_apzz_`, and
    `F0279` are now closed through the DM1 resurrection owner. `F0278` is a
    source-ordered reset-to-start-game side-effect plan for new-game hand
    clear, resume/restart leader-hand restore, champion dirty-attribute clear,
    F0293 action/status/icon redraw, leader restore, and magic-caster restore;
    actual global mutation remains caller-owned. `F0279` covers the REVIVE.C
    A..P nibble decoder used by F0280 for four-character vitals and
    two-character statistics, and the 2026-07-16 F0280 follow-up now consumes
    those decoded values for caller-owned candidate text/stat materialization.
    Remaining adjacent work is original save/capture breadth and broader live
    UI/global mutation, not the scalar reset/decode/materialization helpers.

  - 2026-07-16 DM2 update: skproject map/list helper rows
    `DM2_map_0cee_1815`, `DM2_map_0cee_185a`, `DM2_map_2066_1f37`,
    `DM2_map_2066_1ec9`, and `SKW_2066_1ea3` are now closed through
    source-backed narrow receipts in `dm2_v1_skproject_core`. Remaining
    adjacent map work starts at `DM2_ARRANGE_DUNGEON` and broader live
    dungeon arrangement/global mutation.

  - 2026-07-16 DM2 update: skproject other-level/cross-map rows
    `DM_LOCATE_OTHER_LEVEL` and `DM2_map_3BF83` are now closed through
    source-backed narrow receipts in `dm2_v1_skproject_core`. Remaining
    adjacent work is live `DM2_ARRANGE_DUNGEON`, `DM2_LOAD_NEWMAP`,
    full record relocation, and caller-owned map/global mutation.

  - 2026-07-16 DM2 update: skproject move/alcove helper rows
    `DM2_move_12b4_023f`, `DM2_move_12b4_099e`,
    `DM2_move_12b4_0092`, `DM2_move_12b4_00af`,
    and `DM2_0cee_317f` are now closed through source-backed narrow receipts.
    Remaining adjacent work includes `DM2_PERFORM_MOVE`, `DM2_ATTACK_WALL`,
    `DM2_ATTACK_DOOR`, and broader live movement mutation.

  - 2026-07-16 DM2 update: skproject door attack row `DM2_ATTACK_DOOR`
    is now closed through a source-backed receipt for byte gate, strength,
    tile-type, delayed timer, and immediate open routes. Remaining adjacent
    movement work is `DM2_PERFORM_MOVE`, `DM2_ATTACK_WALL`, and live mutation
    wiring.

  - 2026-07-16 DM2 update: skproject wall attack row `DM2_ATTACK_WALL`
    is now closed through a source-backed receipt for side scan, ornate alcove
    missile relocation, actuator class 0x22, and actuator class 0x23 teleport
    routes. Remaining adjacent movement work is `DM2_PERFORM_MOVE` and live
    record/timer/actuator mutation wiring.

  - 2026-07-16 DM2 update: skproject `c_gdatfile.cpp`/
    `c_querydb.cpp` raw GDAT query rows now have source-named Firestaff
    receipts over the parsed ENT1/raw tables for raw-data file position,
    raw-data length, raw-data loading, entry pointer/data-index/data-pointer/
    data-length/data-buffer/loadable checks, `LOAD_ENT1`,
    `LOAD_GDAT_ENTRIES`, `QUERY_NEXT_GDAT_ENTRY`, and the bounded GDAT sound
    payload rows `DM2_47eb_00a4`/`DM2_482b_0684`, plus existing image offset
    and local-palette queries. Remaining DM2 querydb/gdatfile work is
    allocation, graphics file I/O lifecycle, preserved-GFX cache behavior,
    image extraction/decode ownership, text/name formatting, and higher-level
    gameplay queries; these receipts must not be used to fabricate images or
    promote missing scalar entries into buffers.

  - 2026-07-16 DM2 querydb update: `DM2_QUERY_ORNATE_ANIM_FRAME`,
    `DM2_GET_ORNATE_ANIM_LEN`, `DM2_QUERY_DOOR_DAMAGE_RESIST`,
    `DM2_QUERY_DOOR_STRENGTH`, `DM2_QUERY_GDAT_CREATURE_WORD_VALUE`, and
    `DM2_QUERY_GDAT_FOOD_VALUE_FROM_RECORD` are now source-mapped through real
    parsed GDAT query rows. Remaining adjacent work is live HUD/dungeon
    consumers for those rows plus broader real GDAT material classes.

  - 2026-07-16 DM2 update: skproject `DM2_QUERY_GDAT_ENTRY_VALUE` and
    `DM2_LOAD_GDAT_ENTRY_DATA_TO` are now closed at the parsed-GDAT loader
    boundary. Firestaff extracts T/I/D/S/P/F/G values from ENT1 rows and
    copies real raw payload bytes into caller buffers while rejecting scalar
    rows and undersized destinations. Remaining adjacent work is decoded image
    ownership, higher-level renderer/HUD consumption, and CPX/cache lifecycle;
    do not turn scalar entries or missing payloads into synthetic buffers.

  - 2026-07-16 DM2 update: skproject `DM2_TRACK_UNDERLAY`,
    `DM2_READ_GRAPHICS_STRUCTURE`, and the bounded `DM2_EXTRACT_GDAT_IMAGE`
    route are now closed at the loader/image receipt boundary. Firestaff binds
    real parsed GDAT raw tables, optional dtRaw8 underlay pairs, direct IMG3/U4
    or IMG9 decode hashes, and source allocation-byte formulas. Remaining
    adjacent work is overlay pixel composition, CPX cache-node lifecycle,
    preserved-GFX list ownership, and renderer/HUD consumption.

  - 2026-07-16 DM2 palette update: skproject `c_gfx_pal.cpp` byte wrapper
    rows, `DM2_CONVERT_DRIVERPALETTE`, `DM2_SELECT_PALETTE_SET`,
    `DM2_UPDATE_BLIT_PALETTE`, and `DM2_xlat_palette` are now source-mapped
    in `dm2_v1_skproject_core`. Remaining adjacent work is live host palette
    upload, fade blitting, and full renderer/HUD consumption of converted
    palettes with real GDAT surfaces.

  - 2026-07-16 DM2 update: skproject `c_gdatfile.cpp`
    `DM2_ALLOC_PICT_BUFF`, `DM2_FREE_PICT_BUFF`, `DM2_ALLOC_NEW_BMP`, and
    `DM2_FREE_PICT_ENTRY` now have bounded source-named bitmap allocation/free
    receipts over row-byte, header-byte, pool, CPX raw-index, and
    preserved-GFX free-route accounting. Remaining adjacent work is real
    `GRAPHICS_DATA_OPEN`/`READ`/`CLOSE` file-handle lifecycle,
    GDAT overlay composition/decode ownership and renderer/HUD consumption of
    real decoded data; these receipts still do not allocate CPX nodes,
    preserved image nodes, or fallback visuals.

  - 2026-07-16 DM2 update: skproject `BETWEEN_VALUE`,
    `DM2_BETWEEN_VALUE`, `ALLOC_TEMP_RECT`, and `ALLOC_TEMP_ORIGIN_RECT` are
    now source-mapped in `dm2_v1_skproject_core`. Remaining adjacent
    `SkWinCore.cpp` cache/mement symbols still require real cache-index and
    mement-table state; do not fake cache hits or image buffers.

  - 2026-07-16 DM2 core/cache update: skproject `c_random.cpp`
    `DM2_RAND16`/`DM2_RANDBIT`/`DM2_RANDDIR` plus the bounded
    `FIND_ICI_FROM_CACHE_HASH`, `INSERT_CACHE_HASH_AT`,
    `QUERY_MEMENTI_FROM`, `ADD_CACHE_HASH`,
    `QUERY_MEMENT_BUFF_FROM_CACHE_INDEX`, `GET_TEMP_CACHE_HASH`, and
    `ALLOC_TEMP_CACHE_INDEX` subset are now source-mapped in
    `dm2_v1_skproject_core`. The adjacent bounded picture-mement receipt
    slice below covers `RECYCLE_MEMENTI`, `TEST_MEMENT`, and image/pict
    admission/free at receipt level. Remaining cache/mement work is full CPX
    setup/guarantee/pointer creation, indexed mement eviction/list ordering,
    and GDAT decode buffer ownership; do not fabricate image buffers.

  - 2026-07-16 DM2 picture-mement update: skproject `TEST_MEMENT`,
    `RECYCLE_MEMENTI`, `ALLOC_NEW_PICT`, `ALLOC_IMAGE_MEMENT`,
    `ALLOC_PICT_MEMENT`, `CALC_PICT_ENT_HASH`, `FREE_IMAGE_MEMENT`, and
    `FREE_PICT_MEMENT` now have source-named bounded receipts in
    `dm2_v1_skproject_core`. These receipts preserve skproject's picture
    allocation sizes, picture-entry hash packing, image-vs-cache picture
    routing, and the Y=-32/8bpp image mement admission gate. Remaining
    adjacent DM2 work is still full CPX setup/guarantee/pointer creation,
    indexed mement eviction, GDAT image decode buffers, and renderer/HUD
    consumption with real data; do not promote these receipts into decoded
    graphics.

  - 2026-07-16 DM2 CPX lower-heap update: skproject
    `ALLOC_LOWER_CPXHEAP`, `ALLOC_CPXHEAP_MEM`,
    `DM2_ALLOC_CPXHEAP_MEM`, `DM2_ALLOC_CPX_LINK_NODE`, and
    `DM2_ALLOC_CPX_UNLINK_NODE` now have isolated bounded receipts in
    `dm2_v1_skproject_cpx_heap`. The receipts allocate from the low edge of
    the first sufficient free span, preserve exact-fit unlink versus split
    behavior, insert freed spans in ascending offset order, and coalesce
    adjacent spans. Remaining adjacent work is full CPX setup, guarantee/free
    scavenging, pointer/index creation, and persistent CPX cache ownership;
    the GFX16/GFX256 material routes now retain real loader bytes but do not
    allocate cache nodes, decoded buffers, or pixels.

  - 2026-07-16 DM2 door GFX256 runtime update: `DM2_DRAW_DOOR_FRAMES` now
    binds each source-selected panel/ornament/frame/button raw interval to a
    bounded GFX256 material receipt before the existing M11 door plan can be
    admitted. The route does not substitute a GFX16 default for non-dtImage
    door rows. Remaining door work is CPX cache ownership and the final pixel
    blit consumer, not a synthetic frame or palette fallback.

  - 2026-07-16 DM2 viewport scene GFX256 update: `DM2_DISPLAY_VIEWPORT`
    floor/ceiling commands now retain the exact active GRAPHICSSET raw
    intervals through GFX256 material receipts before M11 presentation.
    Existing rect, decoded-pixel, local-palette, and c_light receipts remain
    required. Remaining work is persistent CPX-cache ownership and source
    pixel blits, not fallback plane buffers.

  - 2026-07-16 DM2 HUD GFX256 runtime update:
    `DM2_LOAD_GDAT_INTERFACE_00_02` now binds every M11 HUD chrome command
    to its exact INTERFACE_GENERAL or CHAMPIONS raw GDAT interval through a
    GFX256 material receipt. The source-gated viewport rejects an absent,
    mismatched, or unhashed receipt alongside the existing decoded-pixel and
    local-palette checks. Remaining HUD work is source-owned live state for
    dynamic panels and CPX cache ownership, not synthetic frames or cached
    replacement images.

  - 2026-07-16 DM2 dynamic creature GFX256 runtime update:
    `DM2_GET_CREATURE_ANIMATION_FRAME` now retains the exact FB/FC/FD-selected
    CREATURES raw image interval through a GFX256 receipt in addition to the
    existing decoded image and local palette evidence. The M11 creature plan
    keeps that material identity distinct from DB4/F9 and binds its own
    direction, position, and depth. Remaining work is a live DB4 animation
    state owner that can produce source-selected dynamic sprites, not a
    generic icon, map-chip substitution, or procedural frame.

  - 2026-07-16 DM2 wall GFX256 runtime update: `DM2_DRAW_WALL` now carries
    the exact GRAPHICSSET WALL_GFX raw interval through a GFX256 receipt with
    its local palette, source cell order, RAW4 placement row, movement offset,
    and mirror state before M11 consumption. Any altered receipt or unknown
    clip route blocks the complete wall pass. Remaining adjacent renderer
    work is source-proven ornament and static-object placement/scale/flip
    material, not a DM1 projection or generic object fallback.

  - 2026-07-16 DM2 c_map real-data update: `GET_TILE_VALUE`,
    raw tile-map offset, and column-index offset. Remaining DM2 dungeon/runtime
    work is the original DB8/DB10 payload/layout proof and HUD/runtime
    material that consumes real GDAT/dungeon state without fallback visuals.

  - 2026-07-16 DM2 c_record append update: skproject `DM2_APPEND_RECORD_TO`
    / `APPEND_RECORD_TO` now has a bounded source mutation receipt in
    `dm2_v1_dungeon_loader`. It rejects null/end append inputs, resets the
    appended record's `w0` to `OBJECT_END_MARKER`, appends to parent links or
    existing tile chains, and inserts into empty byte-square tiles by shifting
    the ground-stack root list and bumping later column offsets.

  - 2026-07-16 DM2 c_record cut update: skproject `DM2_CUT_RECORD_FROM` now
    has a bounded source mutation receipt in `dm2_v1_dungeon_loader`. It
    rejects null/end cut inputs, uses the source `0x3fff` ObjectID mask for
    address and link comparisons, unlinks from parent-owned chains or tile
    chains, replaces first tile roots with their next link, compacts
    single-root tile ground-stack entries, decrements later column offsets,
    and resets the cut record's `w0` to `OBJECT_END_MARKER`. Remaining
    adjacent work is indexed mement free-list ordering and using these
    mutations in broader real GDAT/HUD dungeon runtime.

  - 2026-07-16 DM2 item-charge update: skproject `DM2_ADD_ITEM_CHARGE` /
    `ADD_ITEM_CHARGE` now has a source-mapped bounded helper in
    `dm2_v1_skproject_core` for DB5 weapon, DB6 cloth, and DB10 miscellaneous
    item charge bitfields and clamps. Remaining adjacent item work is runtime
    HUD/item consumption from real GDAT and dungeon records.

  - 2026-07-16 DM2 item max-charge update: skproject `DM2_GET_MAX_CHARGE` /
    `GET_MAX_CHARGE` now has a source-mapped bounded helper in
    `dm2_v1_skproject_core` for the same DB5 weapon, DB6 cloth, DB10
    miscellaneous item, `OBJECT_NULL`, and unsupported-DB outcomes used by
    the charge/value path. This does not implement HUD/runtime consumption.

  - 2026-07-16 DM2 item value/weight update: skproject
    `DM2_QUERY_ITEM_VALUE`, `DM2_QUERY_ITEM_WEIGHT`, `QUERY_ITEM_VALUE`,
    `QUERY_ITEM_WEIGHT`, and `CALC_PLAYER_WEIGHT` now have bounded
    source-mapped receipts in `dm2_v1_skproject_core`. The receipts consume
    caller-supplied source-shaped record/GDAT word values, preserve charge
    value scaling, potion money scaling, recursive container value, moneybox
    weight/value aggregation, the item-weight wrapper, and the selected
    player's open-chest overlay. Remaining adjacent item work is wiring these
    receipts to real loader-owned GDAT/dungeon records and HUD/runtime
    consumers; do not fabricate item definitions, container contents, or
    player inventory state.

  - 2026-07-16 DM2 movement-vector update: skproject
    `DM2_CALC_VECTOR_W_DIR` / `CALC_VECTOR_W_DIR` now has a bounded
    source-mapped helper in `dm2_v1_skproject_core`. The receipt preserves
    the additive caller-owned accumulator behavior, signed forward/side
    operands, source X/Y delta tables, and direction wrap. Remaining adjacent
    movement work is wiring this into real `c_move`/map collision/runtime
    consumption with loaded dungeon state; do not treat this as movement
    timing, collision, input dispatch, or map mutation.

  - 2026-07-16 DM2 coin-count update: skproject
    `DM2_COUNT_BY_COIN_TYPES` / `COUNT_BY_COIN_TYPES` now has a bounded
    source-mapped helper in `dm2_v1_skproject_core`. The receipt preserves the
    ten-counter zeroing, moneybox contained-record chain walk, miscellaneous
    currency filter, distinctive item-type table match, and charge+1
    accumulation over caller-supplied source-shaped records. Remaining
    adjacent item/container work is wiring this to real loader-owned GDAT
    DBSPEC currency flags, distinctive item types, and moneybox/container
    chains; do not fabricate item definitions, GDAT flags, or container
    contents.

  - 2026-07-16 DM2 attribute/UI update: skproject `BOOST_ATTRIBUTE`,
    `DM2_ADJUST_UI_EVENT`, and `ADJUST_UI_EVENT` now have bounded
    source-mapped helpers in `dm2_v1_skproject_core`. Remaining adjacent work
    is wiring these receipts to real runtime champion state, menu/HUD input
    events, and command dispatch; do not fabricate champion state, UI events,
    hand action availability, or input routing.

  - 2026-07-16 DM2 utility update: skproject `DM2_ABS`,
    `DM2_CALC_SQUARE_DISTANCE`, `DM2_CALC_VECTOR_DIR`,
    `DM2_COMPUTE_POWER_4_WITHIN`, `DM2_FILL_I16TABLE`, and
    `DM2_ATIMESB_RSHIFTC` now have bounded source-mapped helpers in
    `dm2_v1_skproject_core`. The receipts preserve signed absolute values,
    Manhattan square distance, dominant-axis direction selection with
    `DM2_RANDBIT` diagonal tie break, nth set-bit power scanning, caller-owned
    i16 table fill, and unsigned-word multiply-before-shift behavior.
    Remaining adjacent work is wiring utility consumers into real movement,
    map, HUD, and GDAT paths without fabricating pathfinding, collision,
    bitmask semantics, table storage, or runtime input.

  - 2026-07-16 DM2 c_gfx_str update: skproject `DM2_QUERY_FONT` and
    Remaining adjacent work is live pixel blitting, palette/blitter ownership,
    dynamic `.Z000`--`.Z028`/0x01 substitutions, scrollbox screen mutation, and wiring
    these receipts to real runtime HUD/menu consumers without fallback visuals.

  - **2026-08-05 CAII inventory update:** both linked narrow
    `DM2_1c9a_38a8` adapters formerly returned the original routine's valid
    zero result without its `s350` action list, live CAII records, or
    `DM2_FIND_WALK_PATH` owner. They now reject explicitly. Port the complete
    source state and callback chain from `c_1c9a.cpp:9748-9894` before a
    runtime path-search result can be admitted.

  - 2026-07-16 DM2 graphics-data file lifecycle update: skproject
    `DM2_GRAPHICS_DATA_OPEN`, `DM2_GRAPHICS_DATA_CLOSE`, and
    `DM2_GRAPHICS_DATA_READ` now have bounded file-counter and split-read
    receipts in `dm2_v1_asset_loader`. Remaining adjacent work is real OS
    file-handle IO, seek/read failure propagation, and using these receipts in
    full `LOAD_GDAT_RAW_DATA` / `LOAD_GDAT_ENTRY_DATA_TO` consumers without
    synthetic buffers.

  - 2026-07-16 Theron update: the admitted US raw Track 02 FIFO/session
    handoff can now consume the bounded all-dungeon route into a startup-level
    anchor receipt. The receipt carries only the real Hall of Records anchor
    offsets, dimensions, seed/index, and object/level/all-dungeon route hashes;
    exact level semantics, object-table admission, non-startup level
    admission, payload semantics, visual semantics, and fallback visuals remain
    blocked. Remaining Theron work is a positive real capture/consumer path
    that proves the level/object consumers themselves rather than promoting
    this anchor metadata.

  - 2026-07-16 Theron object-table route-gap update: the runtime startup
    anchor can now consume a verified Track 02 object-table route receipt as
    explicit gap evidence. The receipt carries descriptor/object candidate
    masks, first candidate offsets/user-data/hash/counts, binding status, and
    the missing-real-object-evidence blocker only when the object route hash
    matches the admitted startup anchor and object decode/admission/fallback
    remain closed. Remaining Theron work is still real object-table decode and
    runtime object consumers from captured Track 02 data; do not treat this as
    object semantics, runtime objects, or visuals.

  - 2026-07-16 Theron capture-consumer route-gap update: the runtime
    admission surface now has a combined Track 02 capture-consumer gap receipt
    that consumes the startup anchor, non-startup level evidence, and
    object-table evidence only when all three agree on the admitted US raw
    Track 02 session and route hashes. It records candidate masks/counts and
    first opaque candidate hashes while keeping capture-consumer readiness,
    object-table decode/admission, non-startup level decode/admission, exact
    level/object semantics, payload/visual semantics, and fallback visuals
    closed. Remaining Theron work is still a positive original runtime
    capture/consumer that proves these bytes before any route promotion.

  - 2026-07-16 Theron consumer-semantic bridge update: the runtime admission
    surface now has a fail-closed bridge from the combined capture-consumer
    gap receipt to an already proven original post-$3800 consumer semantic
    receipt. The bridge refuses pre-opened gap receipts, non-US Track 02
    variants, mismatched records, missing consumer trace checksums, missing
    dungeon/object/bitmap/palette consumer proof, and any fallback visuals.
    Remaining Theron work is still the real original capture that produces the
    post-$3800 consumer semantic receipt for the admitted runtime session; no
    synthetic consumer route or fallback visual path is admitted.

  - 2026-07-16 Theron render-asset admission update: the runtime admission
    surface now has a second fail-closed gate after consumer semantics for
    decoded real render assets. The receipt requires the same admitted US raw
    Track 02 session, matching level/object/all-dungeon route hashes, matching
    payload/envelope/consumer trace checksums, nonzero decoded level,
    object-table, bitmap, and palette hashes, explicit level/object/bitmap/
    palette consumer proof, decoded bitmap-pixel and palette-word proof, and
    rejects any synthetic promotion or fallback observation. Remaining Theron
    work is the real decoder/capture producer that supplies those decoded
    level, object-table, bitmap, and palette proofs from captured Track 02
    data; fallback visuals remain closed.

  - 2026-07-16 Theron host-dungeon consumer update: the runtime admission
    surface can now consume the real Track 02 dungeon handoff into a host
    dungeon-consumer receipt. It opens dungeon draw only when the same capture
    has matching decoded level/object-table/bitmap/palette hashes, proven
    original host route, level-grid/object-table/bitmap-palette runtime
    consumers, host surface upload, and host capture frame. It rejects any
    already-open draw state, synthetic host/level/object/bitmap promotion, hash
    drift, or fallback visuals. Remaining Theron work is the real capture/
    decoder producer that supplies those proofs from original Track 02 bytes.

  - 2026-07-16 Theron host proof producer update: the host dungeon-consumer
    proof is now produced by runtime admission code from the already verified
    Track 02 dungeon handoff, not hand-assembled by the probe. The producer
    requires a non-placeholder, non-synthetic, non-fallback original host-route
    identity plus level-grid, object-table, bitmap/palette, host-upload, and
    host-capture proof flags, then copies only the matching real route,
    checksum, and decoded asset hashes from the admitted handoff. Remaining
    blocker: real Track 02 decoder/capture code must still supply those proof
    flags and decoded hashes from original ISO/BIN/CUE bytes across non-startup
    dungeons.

  - 2026-07-16 Theron decoded-route render proof producer update: runtime
    admission can now build the render-asset proof from decoded Track 02 route
    receipts instead of probe-injected hashes. The producer requires matching
    consumer/level/object route hashes, non-startup level decode-ready,
    object-table decode-ready, a complete startup bitmap atlas, promotable
    palette-window evidence, nonzero decoded hashes, and no synthetic or
    fallback visual flags. Remaining blocker: the real ISO/BIN/CUE decoder and
    capture path must still fill those decoded receipts from original media for
    broader non-startup dungeons.

  - 2026-07-16 Theron palette-window fail-closed update: explicit HuC6260 4bpp
    palette-window inspection now clears all evidence on copy/decode failure,
    and focused coverage proves valid US ISO palette bytes become format-only
    evidence while malformed palette bytes and real/raw optional palette
    probes cannot promote. Remaining blocker: a source-locked loader/capture
    binding must still identify which palette window feeds which bitmap route
    before RGBA promotion, dungeon draw, or fallback visuals can open.

  - 2026-07-16 Theron bitmap-atlas layout gate update: startup bitmap atlas
    construction now requires at least one nonzero source pixel after laying
    out the CD-backed tiles, and clears the atlas receipt when a caller supplies
    only empty/nonsensical bitmap samples. Focused coverage proves positive
    route layout preserves raw/user-data offsets, tile order, width, and
    checksum, while all-zero layout evidence cannot become atlas-ready.
    Remaining blocker: real loader/capture evidence must still bind bitmap
    layout to a promotable palette and runtime consumer before RGBA, dungeon
    draw, or fallback visuals can open.

  - 2026-07-16 Theron split-CUE resolver update: the Track02 media resolver
    now uses the documented exact `TQJP02.iso -> TQJP02End.iso` and
    `TQUS02.iso -> TQUS02End.iso` split-layout aliases instead of leaving that
    helper dormant. The route still accepts both CUE-declared `MODE1/2352`
    raw-sector payloads and verified `MODE1/2048` ISO payloads, while arbitrary
    missing members and duplicate Track02 `INDEX 01` entries fail closed.
    Remaining blocker: real ISO/BIN/CUE decoder/capture code still must bind
    non-startup level/object and palette windows before runtime admission or
    fallback visuals can open.

  - 2026-07-16 update: stale audit row `F0208` is now narrowed against the
    existing DM1-owned event-plan mapping:
    `F0208_DM1_GROUP_BuildAddEventPlan_Compat` preserves the ReDMCSB
    requested-time comparison, C38-C41 to C33-C36 promotion for earlier
    aspect updates, `EVENT.C.Ticks` delta, and caller-owned F0238 insertion
    boundary. Verification used the focused
    `dm1_v1_creature_ai_behavior_source_lock` CTest; no synthetic event data
    was added. The broader M10 initial-attack consumption follow-up remains
    open below.

  - 2026-07-16 update: stale audit row `F0224` is now closed against the
    DM1-owned endgame fluxcage action plan.
    `DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat` preserves
    ReDMCSB's wall/stairs no-op, unused C15 explosion Thing gate, C050
    fluxcage creation, C24 remove-event fields, adjacent Lord Chaos scan
    order, and C29 danger reaction condition. Verification used the focused
    `dm1_v1_endgame_system_source_lock` CTest; no synthetic dungeon/save/art
    data was added. Broader real-map Lord Chaos escape capture, M11
    action-stamina cleanup, and pixel evidence remain separate.

  - 2026-07-16 update: adjacent stale rows `F0240`, `F0261`, and `F0265`
  audit hits as implementation evidence. Note: `tools/symbol_backlog.py` was
  absent in this checkout during the 2026-07-16 pass, so the row selection used
  `docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv` instead.
    side-effect bodies remain separate backlog work.

  - 2026-07-17 TITLE.C capture update: the source-step owner now transitions
    from PRESENTS frame 59/step 1 to CHAOS frame 60/step 2, retains the
    source-owned full-CHAOS sample at frame 77/step 19, holds the CHAOS plan
    through frame 99/step 21, then presents STRIKES at frames 100-101/step

    22.
    The visual/runtime
    capture receipts consume those coherent frame/step pairs; they do not
    infer CHAOS from a stale frame or a wrapper route. Remaining handoff
    failures are separate visual title-presentation, entrance-input, and
    real-scan MD5-drift assertions.

  - 2026-07-22 CSB host-cadence correction: while the verified C001 title or
    C004/C002/C003 Entrance sequence is active, M11 advances exactly one
    source VBlank every 20 ms. It no longer applies the 200 ms gameplay tick
    or the user gameplay-speed multiplier to those source-owned startup
    frames. Normal CSB runtime returns to the ordinary gameplay cadence. The
    focused cadence regression and the opt-in real `GRAPHICS.DAT`/
    `DUNGEON.DAT` startup sequence pass; remaining work is a legible real
    app-window capture, not an alternate decoder or generated replacement.

  - 2026-07-28 title/Entrance timing correction: ReDMCSB `TITLE.C:451-463`
    proves 60 VBlanks of PRESENTS, 20 shrink frames, `Delay(20)` on the full
    CHAOS page, then `Delay(2)` on C426 STRIKES BACK. The former 101-tick
    model mislabeled the CHAOS delay and held STRIKES only once. The current
    102-tick source model retains frames 100-101 before ENTRANCE.C receives
    ownership. The real local launcher regression
    now passes 500/500; remaining CSB-010 work is visual app-window capture,
    not a title-state or frame-boundary repair.

  - 2026-07-17 presentation-receipt update: the route receipt forwards the
    same source-owned 20-tick CHAOS zoom duration as the title capture. The
    focused package-capture regression asserts this receipt value, preventing
    the title/entrance owner from diverging back to the obsolete 18-tick test
    expectation. Remaining handoff work starts at the separate CHAOS-hold
    receipt assertion.

  - 2026-07-17 CHAOS-hold update: the render-view receipt now identifies the
    full CHAOS source at the coherent frame-77/step-19 pair and derives its
    hold tick from that source step. The focused regression rejects a zoom
    classification there; no title or HUD fallback route is introduced.
    Remaining handoff work begins at the separate STRIKES title receipt.

  - 2026-07-17 STRIKES update: the render-view receipt accepts the source
    owner's frame-80/step-21 transition and derives the one-tick STRIKES
    phase from that step boundary. The focused regression prevents a stale
    step-20 fixture from crossing the title capture route. Remaining handoff
    work begins at the separate entrance-input receipt.

  - 2026-07-17 ENTRANCE.C input update: entering the dungeon now publishes
    pre-open delay with `opening_step=0`; the first actual C002/C003 frame is
    not claimed until the delay has elapsed. Pointer action, host-dispatch and
    post-input render receipts therefore retain one coherent source state.
    Remaining handoff work is limited to real-scan graphics/Dungeon MD5 drift.

  - 2026-07-17 title-capture admission update: the signed render-view receipt
    now admits C001 only when its `title_frame`, `title_source_step`,
    `title_stage`, source rectangle, blit kind, and palette are the exact
    TITLE.C F0437 plan. In particular, frame 79/step 21 remains CHAOS while
    frame 80/step 21 is STRIKES; the prior frame-100/step-22 STRIKES claim was
    a stale capture interpretation. A mismatched same-step phase is no-draw.
    The M11 launcher boundary uses the real `data/csb` C001/C004/C002/C003
    path and waits one source vblank after the F0806 pre-open delay before the
    first door frame. No synthetic frame or palette is admitted.

  - 2026-07-17 real-raster correction: the render-view adapter no longer
    infers STRIKES from `title_source_step >= 20`. That stale shortcut changed
    the real frame-78/79 CHAOS C001 crop into a STRIKES crop even though both
    waves can carry steps 20/21. It now follows the signed TITLE.C F0437
    stage, retaining the real CHAOS palette/0..79 crop through frame 79 and
    selecting the 80..136 STRIKES crop only at frame 80. The local PC34
    `GRAPHICS.DAT` runtime sequence verifies indexed raster admission and
    palette ids for both boundary frames plus C004/C002/C003 Entrance; no
    synthetic frame, palette, or fallback decoder was introduced.

  - 2026-07-17 F0438/F0807 palette admission update: startup host-surface
    admission now checks the source palette before it creates a session frame.
    The final C004+C003 opening page requires `C28_ENTRANCE_CSB`; the first
    PANEL.C C017/C040 runtime page requires the neutral palette after F0807
    releases Entrance state. A relabelled title palette or retained Entrance
    palette rejects without updating session presentation metadata. The local
    PC34 sequence covers positive real indexed rasters and both reject paths;
    no synthetic door, HUD, frame, or palette was introduced.

  - 2026-07-28 CSB C28 palette correction: the runtime had incorrectly
    presented CSB's `C28_ENTRANCE_CSB` page through DM's `C07/G8148` row,
    producing bright-green edge pixels. It now uses the VGA `G8174` row from
    ReDMCSB `VIDEODRV.C`; direct real-data title/Entrance regressions and a
    presented SDL capture are green. Remaining CSB startup work is broader
    app-window capture and the separate save/DSA runtime corpus.

  - 2026-07-17 graphics-MD5 receipt update: startup receipt recomputation now
    clears both hash forms on graphics-proof drift before a session can reuse
    it. The focused regression mutates only graphics MD5. Dungeon drift and
    explicit fresh re-admission remain the next separate scanner work.

  - 2026-07-17 Dungeon/re-admission update: any graphics or Dungeon metadata
    drift permanently invalidates that receipt emission. Restoring fields in
    place remains rejected; only a fresh scanner/profile emission can issue a
    new identity. The focused regression covers Dungeon drift, failed in-place
    repair, and successful fresh reissue. The CSB boot handoff fixture is now
    green; remaining startup work is external corpus/capture evidence, not a
    local MD5-repair path.

  - 2026-07-16 update: presented Mac/app capture now also rejects any frame
    whose route hash is not one of the receipt-owned title, HUD/door, or
    credits capture hashes. Remaining CSB evidence is still real Mac/app
    capture with local CSB data, not a synthetic title/door/HUD substitute.

  - 2026-07-16 update: the standalone CSB callable backlog item for
    HINTHTC `F1918_LoadGame_CPSX`/`F1919_Post_F1918_LoadGame_CPSX` is now
    source-mapped through a CSB-owned initial-load/post receipt. It consumes
    only the accepted ReDMCSB F1914 header and exact GLOBAL_DATA,
    ACTIVE_GROUPS, and PARTY spans, then blocks later handoff unless all
    three source parts were accepted. Remaining CSB work is still real
    packaged title/HUD/door capture breadth and positive DSA/save-runtime
    corpus proof, not inferred save-tail or DSA construction.

  - 2026-07-16 update: the CSBWin DSA/save-runtime corpus gate is now
    explicit. A staged CSBWin save can be marked DSA runtime-handoff ready
    only when the filename is recognised, the Extended Features DSA section
    authenticates with real actions/program words, the game-info/level-index
    tail verifies, and the next bytes are a valid GAMEBLOCK1 header. This is
    a gate for future operator-supplied corpus bytes; it does not vendor or
    fabricate a positive real save. Remaining CSB work is still real
    packaged title/HUD/door capture breadth and real local DSA-bearing corpus
    execution through the gate.

  - 2026-07-16 update: the boot save/import receipt now carries that
    CSBWin DSA corpus verdict into the runtime handoff surface, separately
    from the older CSBGAME roster-loader readiness. A plain CSBGAME.DAT can
    still be loader-ready, but it remains DSA-runtime-handoff blocked with
    `reject_dsa_corpus_no_extended_features`; runtime import now refuses
    staged CSBWin paths unless the filename/loader classifier says import is
    ready. Remaining CSB work is positive operator-supplied DSA-bearing corpus
    execution and real packaged title/HUD/door capture breadth.

  - 2026-07-16 update: `PANEL.C F0347_INVENTORY_DrawPanel` now has a
    CSB-owned HUD-panel receipt over the existing real C017/C040 startup
    panel blit. It proves only source-owned C017 plus optional C040 hashes
    and does not create pixels, panel state, save data, DSA state, or host HUD
    fallback. Remaining CSB work is unchanged: real packaged title/HUD/door
    capture breadth and positive DSA/save-runtime corpus proof.

  - 2026-07-16 update: the CSB startup packed-page presenter now accepts only
    a 320x200 indexed page named by the exact CSB host-surface receipt,
    matching host-surface hash, raster route hash, and recomputed pixel hash
    before F0692/F0693 presentation. This closes the stale/wrapper raster
    promotion gap for title/PRESENTS/opening/HUD pages; remaining evidence is
    still real Mac/app capture with local CSB data and positive DSA/save
    corpus proof.

  - 2026-07-16 update: the CSB host-surface receipt hash now incorporates the
    routed raster pixel hash, and the packed-page presenter recomputes that
    receipt hash from the receipt's own frame route, raster route, raster
    pixels, host-surface kind, HUD binding, and title/door palette fields.
    A caller-forged host hash can no longer promote a title/opening/HUD page
    if the underlying ReDMCSB-owned raster identity does not match; remaining
    evidence is still real Mac/app capture with local CSB data.

  - 2026-07-16 update: the CSB title-to-opening session receipt now requires
    PRESENTS, CHAOS, and STRIKES BACK host captures to keep distinct
    host-surface and route hashes, and requires the C004/C002/C003 opening
    host frame to carry nonzero frame route, raster route, and pixel hashes.
    Replayed title phases or route-less opening pages cannot satisfy the
    startup consumption receipts. Remaining CSB evidence is still real
    Mac/app capture with local CSB data and positive DSA/save corpus proof.

  - 2026-07-16 update: restored CSBWin DSA STKOP_DiscardText now has runtime
    evidence through the saved TIMER bridge: it clears only the existing
    DB2/F0168 one-message receipt, keeps generic text mutation blocked, and
    cannot create or route a host message log. Remaining CSB work is still
    real packaged title/HUD/door capture breadth, positive DSA/save corpus
    execution, and any broader message queue behavior not bound to an
    authenticated source text receipt.

  - 2026-07-16 update: the HUD/door/input package receipt now consumes the
    routed C017/C040 HUD host raster instead of accepting a bare HUD host
    hash. It requires the real HUD frame route, raster route, raster pixel
    hash, two source surfaces, verified HUD binding hash, and exact resident
    C017/C040 owners before the first live door/input receipt can satisfy the
    package handoff. Remaining CSB work is unchanged: real Mac/app capture
    breadth with local CSB data and positive DSA/save corpus proof.

  - 2026-07-16 update: the CSB D0L2/D0R2 viewport route now has an internal
    receipt over the existing ReDMCSB F0125/F0126/F0128, F0115, and F0111
    source contracts. It binds the D0 side lane, F0115 thing-pass cell order,
    F0111 rear/door/front split, C10 transparency, disabled item/projectile
    rows, and non-mutating door draw boundary without loading game data,
    creating viewport pixels, or using a legacy viewport wrapper. Remaining
    work is still real viewport bitmap consumption/capture breadth, not
    synthetic lane art.

  - 2026-07-16 update: the D0L2/D0R2 F0111 door-front route now has a
    fail-closed real-asset receipt for `GRAPHICS.DAT` bitmap 693 plus ornament
    view 0. Focused coverage reads the local DMCSB1 item table, hashes item
    693's original payload, and admits the route only with that real source
    evidence plus explicit no-synthetic/no-fallback flags. Remaining CSB
    viewport work is actual decoded-door pixel consumption/capture breadth and
    wider HUD state, not substitute door art or guessed bitmap decoding.

  - 2026-07-16 update: the D0L2/D0R2 F0115 thing-pass route now has a
    fail-closed real-asset receipt for `GRAPHICS.DAT` wall-frame rows/items
    10 and 11. Focused coverage reads the local DMCSB1 item table, hashes each
    original compressed payload, requires explicit no-synthetic/no-fallback
    flags, preserves the G2028 item/projectile suppression boundary, and
    rejects wrong item indices or zero hashes. The new F0115 target and the
    previously missing F0111 door-front target are both registered in CMake for
    Ninja/CTest. Remaining CSB viewport work is decoded wall/thing pixel
    consumption and capture breadth from real assets, not fallback art.

  - 2026-07-16 update: the D1C F0115 center thing-pass route now has a
    fail-closed native-object receipt backed by real `GRAPHICS.DAT` entries
    498..583. Focused coverage reads the local DMCSB1 item table, hashes
    original payloads for entries 498 and 583 across the BACK/FRONT D1C passes,
    requires native-object family binding plus explicit no-synthetic/no-fallback
    flags, and rejects item 584, missing source data, synthetic pixels,
    fallback visuals, and zero hashes. The D1C test is now registered in
    CMake for Ninja/CTest. Remaining CSB viewport work is decoded object pixel
    consumption, capture breadth, and wider HUD state from real assets, not
    substitute item art.

  - 2026-07-16 update: the D1C F0111 center door-front route now has a
    fail-closed real-asset receipt for `GRAPHICS.DAT` item 558, matching
    ReDMCSB `G0186_s_Graphic558_Frames_Door_D1C`. Focused coverage reads the
    local DMCSB1 item table, hashes item 558's original payload, preserves the
    96x88 native D1C door geometry, C2 D1LCR ornament view, M631 zone, and
    BACK/FRONT pass ordering, and rejects missing source data, items 557/559,
    synthetic pixels, fallback visuals, and zero hashes. The D1C F0111 test is
    registered in CMake for Ninja/CTest. Remaining CSB viewport work is
    decoded door/object pixel consumption and capture breadth from real assets,
    not substitute door art.

  - 2026-07-16 update: the D1L/D1R F0111 side door-front route now has a
    fail-closed real-asset receipt for the shared `StdDoorGraphicsF1`
    `GRAPHICS.DAT` item 558 path. Focused coverage reads the local DMCSB1 item
    table, hashes item 558's original payload, requires both D1 side routes,
    preserves side door zones 3780/3800, top-track zones 732/734, rear/front
    F0115 order words, and C10 transparency, and rejects missing source data,
    items 557/559, synthetic pixels, fallback visuals, zero hashes, and
    duplicate-side route attempts. The D1L/D1R F0111 test is registered in
    CMake for Ninja/CTest. Remaining CSB viewport work is decoded side-door
    pixel consumption and capture breadth from real assets, not substitute
    side-door art.

  - 2026-07-16 update: the D2C F0111 center door-front route now has a
    fail-closed real-asset receipt for `GRAPHICS.DAT` item 694, matching
    ReDMCSB `G0694_ai_DoorNativeBitmapIndex_Front_D2LCR`. Focused coverage
    reads the local DMCSB1 item table, hashes item 694's original payload,
    preserves the 64x61 D2C door geometry, M628 door zone 3760, C1 D2LCR
    ornament view, rear/front F0115 order words 0x0218/0x0349, and C10
    transparency, and rejects missing source data, items 693/695, synthetic
    pixels, fallback visuals, and zero hashes. The D2C F0111 test is
    registered in CMake for Ninja/CTest. Remaining CSB viewport work is
    decoded D2 door pixel consumption and capture breadth from real assets,
    not substitute door art.

  - 2026-07-16 update: the CSB first-frame viewport materialization gate now
    joins the real D0/D1/D2 F0111/F0115 door/thing receipts into the actual
    `csb_v1_viewport_render_frame` consumer. The render config carries a
    shared real `GRAPHICS.DAT` catalog/material hash plus D0 door, D0 thing,
    D1 door, D1 native-object, and D2 door payload hashes, publishes a
    consumed receipt only with a real graphics session, and fail-closes when
    any required route/material is missing. The new Ninja/CTest target proves
    the positive local DMCSB1 path and the no-real-session blocker. Remaining
    CSB viewport work is broader decoded pixel/capture parity for these real
    materials, not more standalone DUNVIEW receipt micro-slices or fallback
    viewport art.

  - 2026-07-16 update: the CSB DSA save-runtime handoff now has a separate
    boot-owned receipt after CSBWin import. It consumes the existing
    save/import receipt and remains invalid for a loader-ready plain
    `CSBGAME.DAT` unless the real Extended Features DSA corpus, runtime
    actions, GAMEBLOCK1, loaded party, and CSB import-source facts all pass.
    This closes the loader-ready-versus-DSA-ready handoff ambiguity without
    adding a synthetic DSA-positive save. Remaining CSB work is still positive
    operator-supplied DSA-bearing corpus execution and real Mac/app
    title/HUD/door capture breadth.

  - 2026-07-16 update: DSA/save admission now requires the complete
    checksum-verified CSBWin GAMEBLOCK1 body after the authenticated Extended
    Features DSA tail, not only its 512-byte header. The admitted save and
    body FNV receipts are consumed by the boot handoff before runtime can
    claim DSA ownership. Header-only fixtures remain blocked; positive work is
    still an operator-supplied checksum-valid DSA save/dungeon corpus, without
    inferring any additional DSA opcode semantics.

  - 2026-07-16 update: the registered CSBWin save-loader probe now scans a
    bounded set of real staged `csbgame`/`dmsave` candidates instead of only
    the first filename hit, and runs both the loader discovery verdict and the
    strict DSA save-runtime corpus receipt for each one. The local data root
    currently has no such save corpus, so this adds a green real-corpus scan
    gate but does not close the positive DSA-bearing execution requirement.
    Remaining CSB work is still operator-supplied DSA corpus execution plus
    real Mac/app title/HUD/door capture breadth.

  - 2026-07-16 update: the CSBWin package runtime handoff probe now performs
    its own real-data discovery under the user data root, selects only a
    strict `runtime_handoff_ready` Extended Features DSA save receipt, and
    then drives the production Dungeon.dat + CSBWin resume/tick/core-resume
    path. Hosts without a staged DSA-bearing save skip without creating a
    save, DSA record, timer action, or fallback runtime owner. Core-only
    resume now also clears stale saved-timer DSA receipt fields alongside the
    older Extended Features/level-index/DSA catalog ownership. Remaining CSB
    work is positive operator-supplied DSA-bearing package execution breadth
    and real Mac/app title/HUD/door capture breadth.

  - 2026-07-16 update: CSB runtime now exposes a single
    `csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34` production receipt
    for the CSBWin DSA resume path. It requires the verified Extended
    Features DSA catalog, a level-index entry that reaches an authenticated
    action, exact TimerQueue/event ownership, and, after a saved timer fires,
    an exact action ordinal/DSA/state/column receipt. The package and
    Extended-DSA probes use this same runtime receipt and remain skip-safe
    without a staged checksum-valid DSA save; remaining CSB work is positive
    operator-supplied DSA-bearing package breadth and broader CSBWin event
    family execution.

  - 2026-07-16 update: the authenticated DSA transfer/stack-core runtime now
    publishes `csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34` after
    a successful transactional commit. It reports the exact DSA
    id/state/column/action ordinal, verifier class, stack/transfer counters,
    rollback guard, and committed mutation classes for globals, local DSA state,
    EXPOOL/save policy/text/party/dungeon bytes. Unsupported or unverified
    opcodes still fail closed before publication. Remaining CSB work is positive
    operator-supplied DSA-bearing corpus execution for this receipt and broader
    CSBWin event/opcode families.

  - 2026-07-16 update: the DSA verifier/runtime receipt now separates the
    admitted VM families for conditionals, arithmetic, local/global variables,
    timer-owned effects, and dungeon/save mutation callbacks. The executed VM
    remains transactional and fail-closed: these family flags are published only
    for exact checksum-imported `DSAAction` programs after the candidate profile,
    EXPOOL, local state, and dungeon byte commits all succeed. Remaining CSB
    work is still positive operator-supplied DSA-bearing corpus coverage and any
    future CSBWin opcode family not yet admitted by the verifier.

  - 2026-07-16 update: `DSACMD_QUESTION` now executes its selected
    branch-transfer path instead of stopping at receipt-only admission. The VM
    decodes branch operands in source order, pops the source condition, then
    enters only the selected exact imported JUMP/GOSUB action chain through the
    existing transfer subset. A missing or mismatched branch owner remains
    fail-closed and publishes no staged runtime mutation. Remaining CSB work is
    positive operator-supplied DSA-bearing corpus breadth and later unreviewed
    opcode families.

  - 2026-07-23 update: `DSACMD_CASE` now admits CSBWin's complete source
    `DSAcaseCmd` span: signed compact `NextState`, its raw ui16 MAXSTATE
    extension, raw case count, and every ui32
    key/packed state-column pair. Runtime execution mirrors `EX_CASE`'s
    binary search and reaches only a checksum-imported JUMP/GOSUB target via
    the bounded transfer owner. Truncated tables, missing targets, and other
    target opcode families reject before state publication. The opt-in real
    save probe remains the only positive corpus route; no DSA case table is
    generated when CSBWin save/Dungeon data is unavailable.

  - 2026-07-23 update: `DSACMD_OVERRIDE` now admits the exact CSBWin
    `DSAoverrideCmd` grammar for `OVERRIDE_P`: compact fields, optional raw
    `Override_Pos`, then the raw ui16 `MAXSTATE` extension in source order.
    Execution publishes only through an explicit ProcessTimers-scope owner;
    missing owner callbacks, unknown selectors, malformed words, and absent
    real CSBWin save/Dungeon identity fail closed. Remaining work is binding
    that owner to a positively identified live CSBWin session and expanding
    external DSA-bearing corpus coverage.

  - 2026-07-23 update: the dynamic loop/control-flow pair
    `STKOP_JumpGear`/`STKOP_GosubGear` now consumes source stack-selected
    `(state,column)` targets in CSBWin pop order. It enters only an exact,
    checksum-imported JUMP/GOSUB transfer chain; missing states, malformed
    stack operands, unreviewed target opcodes, and absent real save/Dungeon
    identity fail closed. Remaining work is broader real CSBWin DSA corpus
    coverage for dynamic targets outside the bounded transfer family.

  - 2026-07-16 update: authenticated JUMP/GOSUB transfer execution now exposes
    CSBWin's implicit RETURN boundary as runtime state: missing
    `Program(state,column)` entries produce counted returns, GOSUB pushes and
    pops explicit call frames, and the committed runtime receipt reports
    transfer count, return count, frame push/pop balance, maximum subroutine
    depth, and final state only after rollback-guarded commit. Remaining CSB
    work is still positive operator-supplied DSA-bearing corpus breadth and
    unproved action forms outside this control-flow subset.

  - 2026-07-16 update: `DSACMD_MESSAGE`, `DSACMD_MESSAGE32`, and
    `DSACMD_DESSAGE32` now execute through the production DSA VM as the first
    post-control-flow message/timer action family. The interpreter decodes
    `DSAmessageCmd` next-state, delay, and target operands in CSBWin source
    order, then schedules via a runtime-owned `QueueDSASwitchAction` callback
    that validates the real dungeon cell and publishes a rollback-guarded
    timer scheduling receipt. Missing dungeon ownership, unsupported cells,
    malformed operands, and unreviewed message routes still fail closed. The
    remaining DSA action backlog is positive external DSA-bearing corpus
    execution plus broader text/ex-pool, party/champion, cancellation, and
    dungeon mutation action families that are not proven by this MESSAGE
    subset.

  - 2026-07-16 update: `DSACMD_COPYTELEPORTER` and
    `DSACMD_COPYTELEPORTER32` now execute as a single authenticated
    switch/actuator/dungeon-mutation path. The VM decodes source and
    destination targets in CSBWin operand order, then the runtime owner copies
    only a real source DB1 teleporter payload plus source CELLFLAG byte onto an
    existing destination teleporter in the profile candidate. Missing
    teleporter records are source no-ops; unavailable or malformed original
    dungeon ownership remains fail-closed. Remaining DSA work is positive
    external DSA-bearing corpus execution plus unreviewed actuator/switch
    action forms beyond this bounded teleporter-copy family.

  - 2026-07-16 update: CSB now has a runtime-owned receipt for ReDMCSB
    `TIMELINE.C F0240_IsFirstEventExpired`. It inspects only the live CSB
    `timeline_queue` heap root and runtime `game_time`, compares the low
    24-bit `Map_Time` value, and rejects malformed heap roots instead of
    fabricating a timer/event. Remaining CSB TIMER work is the broader F0261
    event body/corpus execution surface and positive DSA-bearing save breadth,
    not the first-event expiry predicate.

  - 2026-07-16 update: the CSB runtime now has a source-named
    `F0261_TIMELINE_Process` receipt over the live V1 tick path. It records
    the real heap before/after `csb_v1_runtime_tick_v1()`, drains only expired
    events through the existing ReDMCSB queue processor, preserves future
    events, and rejects malformed heaps before ticking. Remaining CSB TIMER
    work is positive DSA-bearing corpus breadth and broader event-family
    execution evidence, not another synthetic timer wrapper.

  - 2026-07-16 update: skproject `c_map.cpp` `GET_TILE_VALUE`,
    `GET_ADDRESS_OF_TILE_RECORD`, and `IS_TILE_PASSAGE` now have bounded
    Firestaff wrappers over the parsed byte-map, column-index, and
    ground-stack tables. Remaining DM2 c_map work is broader source proof for
    record-chain traversal, DB8/DB10 roots, map transitions, and complete live
    dungeon/HUD material; these wrappers must not substitute GDAT pixels or a
    playable record graph.

  - 2026-07-16 update: the MENU.BPK renderer handoff test is now a CMake/CTest
    target and proves `READY_DECODED` still maps to `blocked-prs3` unless an
    authentic PRS3 route exists; parseable MENU.BPK receipts without the
    canonical source hash also block with no fallback. The DGN
    face/material provenance gate now requires static Structure3 material
    selectors to resolve into the same canonical Structure2 descriptor table
    before publishing a package/host no-draw boundary. Remaining work is still
    real PRS3 opcode/output proof, Saturn presentation capture, runtime DGN
    material decode semantics, Track 02 SFX/script blockers, and broad
    startup/menu/DGN host-route completion.

  - 2026-07-16 update: MENU.BPK PRS3 now has a bounded opcode-prefix witness
    over validated PRS3 frames. It records the control byte, literal/backref
    command counts, first backref operands, consumed control/operand bytes,
    bit order, and stop reason without allocating decoded output or changing
    runtime route admission. Remaining PRS3 work is still original Saturn
    opcode semantics, exact decoded output proof, palette/output ordering,
    and a reviewed host upload route; renderer and DGN material handoffs
    remain blocked with no fallback visuals.

  - 2026-07-16 update: the DGN face/material package-host boundary now
    distinguishes source-bound Structure2/Structure3 selector proof from
    drawable material semantics. Even canonical retail DGN bytes with complete
    selector/descriptor binding now report original Saturn capture required,
    no material semantics proven, no fallback visuals, and no raster-input
    submission. Remaining Nexus work is still real PRS3 decoded-output proof,
    Saturn VDP1/material/palette capture, and the reviewed DGN/menu host
    upload route before any visual handoff can open.

  - 2026-07-16 update: MENU.BPK PRS3 decoded-output proof now has a
    fail-closed gate over a caller-supplied output sidecar. The gate binds the
    sidecar to one real MENU.BPK PRS3 stream by exact expected byte count and
    FNV, but still reports provenance required unless the capture source is
    bound and independently verified as original Saturn execution. Even a
    proof-ready receipt does not promote opcode grammar, a decoder, runtime
    upload, or fallback visuals. Remaining Nexus work is still real original
    Saturn PRS3 opcode/output proof with reviewed producer provenance,
    Saturn VDP1/material/palette capture, and the reviewed DGN/menu host
    upload route before any visual handoff can open.

  - 2026-07-16 update: the PRS3 decoded-output gate now has focused coverage
    for the positive original-Saturn provenance case. A matching real MENU.BPK
    stream sidecar with capture source and original provenance can become
    `source-bound-no-runtime`, but opcode grammar, decoder promotion, runtime
    upload, rendering, and fallback visuals remain false. Remaining Nexus work
    is the real producer that supplies authenticated Saturn output/VDP1/palette
    sidecars, plus reviewed DGN/menu upload before visual handoff.

  - 2026-07-16 update: the DGN package/host consumer now requires the
    validated real-DGN face/material receipt plus an explicit host-route
    request, package-consumed bit, level index, canonical DGN size, face count,
    and Structure2 descriptor count before it records source-route consumption.
    The accepted receipt is still `ready-no-draw`: original Saturn rendering,
    material semantics, raster-input submission, and fallback visuals remain
    false. Remaining Nexus work is the reviewed menu upload consumer, original
    Saturn PRS3/output producer provenance, Saturn VDP1/material/palette
    capture, and broad startup/menu/DGN route completion.

  - 2026-07-16 update: the real LEV00 package/host-route hash test now asserts
    fail-closed DGN readiness at the host boundary: the package is consumed,
    runtime DGN remains blocked, presentation is denied, rasterized command
    count and written pixels stay zero, fallback visuals stay false, and the
    stable host-route status is `blocked-handoff`. Remaining Nexus work is
    still original Saturn DGN capture/admission, reviewed DGN/material upload,
    PRS3 output provenance, and the menu/title/save route capture producer.

  - 2026-07-16 update: the current startup/champion/menu focused WIP is green
    with fail-closed real-DGN behavior. `m11_nexus_startup_runtime_handoff` and
    `nexus_v1_startup_menu_pc34_compat` now require
    `blocked-dgn-capture-required` behavior, zero copied/cached DGN commands,
    zero DGN pixels, and no fallback/synthetic Saturn visuals while verified
    WARNING.BIN/TITLE.CG/SAVE startup receipts remain consumable. Remaining
    Nexus work is still the real original-Saturn PRS3/output producer
    provenance, Saturn VDP1/material/palette capture, and reviewed DGN/menu
    upload route before any visual handoff can open.

  - 2026-07-16 update: the Track 1 screen-capture readiness probes now prove
    real DM.BIN/FONT256.S2D/LEV00.DGN/SCORPION.MNS reach the Nexus capture
    path while DGN remains no-draw/capture-required. The updated gates require
    deterministic local BMP/PPM receipts with zero DGN/fallback pixels, block
    runtime host presentation before original Saturn capture/admission, and
    keep MENU.BPK PRS3 and Structure3 evidence non-promoting. Remaining Nexus
    work is still the real original-Saturn PRS3/output producer provenance,
    Saturn VDP1/material/palette capture, and reviewed DGN/menu upload route
    before any visual handoff can open.

  - 2026-07-16 update: the DGN package/host consumer now also rejects an
    explicit synthetic material route request while preserving the real-DGN
    Structure2/Structure3 admission boundary. Accepted host consumption marks
    the real DGN source route and descriptor/selector admission as consumed,
    but material pixel promotion, raster input, Saturn rendering, and fallback
    visuals remain blocked. Remaining Nexus work is still original-Saturn
    PRS3/output producer provenance, Saturn VDP1/material/palette capture,
    reviewed DGN/menu upload, and the adjacent Structure3 source-path API
    cleanup.

  - 2026-07-16 update: the adjacent Structure3 source-path API cleanup is now
    closed. Real DGN Structure3 face/material collection emits only
    static/animated selector bindings, skips non-textured color-fill faces
    instead of creating synthetic material routes, and reaches the current
    package/host no-draw receipt without pixel/raster/fallback promotion.
    Remaining Nexus work is still original-Saturn PRS3/output producer
    provenance, Saturn VDP1/material/palette capture, reviewed DGN/menu
    upload, and broader startup/menu/DGN route completion before any visual
    handoff can open.

  - 2026-07-16 update: PRS3 decoded-output proof and reviewed MENU.BPK
    VDP1/producer upload evidence now meet at an explicit no-runtime gate.
    The joined receipt preserves the real stream/output identity and reviewed
    upload-path facts, but rejects pre-promoted runtime upload or fallback
    visuals and still requires independent Saturn capture authentication plus
    decoder review before any visual handoff can open. Remaining Nexus work is
    still authenticated Saturn producer/capture evidence, reviewed PRS3
    decoder semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the DGN package/host material route and the PRS3
    output/upload evidence now have a joint route-proof gate. It accepts only
    source-bound real-DGN host consumption plus source-bound PRS3
    no-runtime upload facts, requires explicit startup and DGN route requests,
    and still blocks DGN rendering, startup menu rendering, PRS3 runtime
    upload, material pixel promotion, and fallback visuals. Remaining Nexus
    work is authenticated Saturn capture/producer evidence, reviewed PRS3
    decoder semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the joint DGN/PRS3 route proof now carries concrete
    MENU.BPK PRS3 stream identity through the source-path boundary: entry
    index, stream offset/size, expected output byte count, and output FNV.
    The route blocks if that output fingerprint is missing, while still
    denying DGN rendering, startup menu rendering, PRS3 runtime upload, and
    fallback visuals. Remaining Nexus work is authenticated Saturn
    capture/producer evidence, reviewed PRS3 decoder semantics, Saturn
    VDP1/material/palette capture, and broader startup/menu/DGN route
    completion.

  - 2026-07-16 update: the joint DGN/PRS3 route proof now also carries the
    DGN package/host identity through the same source-path boundary: level
    index, canonical DGN byte count, material-face count, and Structure2
    descriptor count. The join blocks if that retained DGN identity is lost,
    while still denying DGN rendering, startup menu rendering, PRS3 runtime
    upload, material pixel promotion, and fallback visuals. Remaining Nexus
    work is authenticated Saturn capture/producer evidence, reviewed PRS3
    decoder semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the joint DGN/PRS3 route proof now requires the PRS3
    decoded-output sidecar binding in addition to the output fingerprint and
    reviewed upload path. Missing sidecar evidence keeps the route no-draw even
    when the DGN package/host identity and PRS3 stream identity are otherwise
    present. Remaining Nexus work is authenticated Saturn capture/producer
    evidence, reviewed PRS3 decoder semantics, Saturn VDP1/material/palette
    capture, and broader startup/menu/DGN route completion.

  - 2026-07-16 update: the joint DGN/PRS3 route proof now also carries the
    reviewed PRS3 upload-path facts that made the route source-bound:
    reviewed upload path, MENU.BPK upload review, original Saturn provenance,
    and independent-authentication requirement. Missing MENU.BPK upload review
    keeps the route no-draw even when DGN identity, PRS3 stream identity, and
    decoded-output sidecar evidence are present. Remaining Nexus work is
    authenticated Saturn capture/producer evidence, reviewed PRS3 decoder
    semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the joint DGN/PRS3 route proof now also carries and
    requires the PRS3 no-runtime source boundary itself: decoded-output proof
    bound plus source-bound-no-runtime. Missing no-runtime PRS3 source
    evidence keeps the route no-draw even when DGN identity, PRS3 stream
    identity, decoded-output sidecar evidence, and reviewed upload facts are
    present. Remaining Nexus work is authenticated Saturn capture/producer
    evidence, reviewed PRS3 decoder semantics, Saturn VDP1/material/palette
    capture, and broader startup/menu/DGN route completion.

  - 2026-07-16 update: the DGN package/host route and joint DGN/PRS3 proof now
    preserve the Structure3 material selector census and matching geometry
    material-face count: static selector count, animated selector count, and
    geometry material-face count must still sum/match the retained DGN face
    identity before the join is source-bound. Drift keeps the route no-draw.
    Remaining Nexus work is authenticated Saturn capture/producer evidence,
    reviewed PRS3 decoder semantics, Saturn VDP1/material/palette capture, and
    broader startup/menu/DGN route completion.

  - 2026-07-16 update: the same DGN package/host and DGN/PRS3 route proof now
    verifies that retained static Structure3 material selectors still fit
    within the retained Structure2 descriptor count. A later mismatch between
    static selector census and Structure2 descriptor identity blocks the join
    without promoting material semantics, pixels, runtime upload, rendering, or
    fallback visuals. Remaining Nexus work is authenticated Saturn
    capture/producer evidence, reviewed PRS3 decoder semantics, Saturn
    VDP1/material/palette capture, and broader startup/menu/DGN route
    completion.

  - 2026-07-16 update: the retained static-selector/Structure2 relation is now
    carried as an explicit package/host and DGN/PRS3 route receipt flag, so
    later consumers can audit the source-bound relationship directly instead
    of recomputing it from counts. Missing or false relation evidence blocks
    the join no-draw. Remaining Nexus work is authenticated Saturn
    capture/producer evidence, reviewed PRS3 decoder semantics, Saturn
    VDP1/material/palette capture, and broader startup/menu/DGN route
    completion.

  - 2026-07-16 update: the DGN package/host and DGN/PRS3 route proof now also
    carries the retained selector-binding completeness bit from the material
    source receipt. If a later package/host consumer loses that complete
    Structure3 selector binding evidence, the joint route blocks no-draw even
    when DGN counts and PRS3 source evidence still match. Remaining Nexus work
    is authenticated Saturn capture/producer evidence, reviewed PRS3 decoder
    semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the same route now carries the retained Structure2
    descriptor source-route bit from the material receipt into package/host
    and DGN/PRS3 receipts. Losing that descriptor route evidence blocks the
    join even when DGN identities, selector counts, binding completeness, and
    PRS3 no-runtime source facts still match. Remaining Nexus work is
    authenticated Saturn capture/producer evidence, reviewed PRS3 decoder
    semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the DGN package/host and DGN/PRS3 route proof now
    retains the renderer-neutral geometry source facts from the material
    receipt: source-bound geometry, geometry admission, and textured-raster
    blocking. Losing the geometry source route blocks the join while DGN
    rendering, pixel promotion, runtime upload, and fallback visuals stay
    denied. Remaining Nexus work is authenticated Saturn capture/producer
    evidence, reviewed PRS3 decoder semantics, Saturn VDP1/material/palette
    capture, and broader startup/menu/DGN route completion.

  - 2026-07-16 update: the retained geometry admission and textured-raster
    blocker now have explicit DGN/PRS3 fail-closed coverage after package/host
    consumption. Losing either fact blocks the join even when DGN identity,
    PRS3 no-runtime evidence, and the remaining geometry source facts still
    match. Remaining Nexus work is authenticated Saturn capture/producer
    evidence, reviewed PRS3 decoder semantics, Saturn VDP1/material/palette
    capture, and broader startup/menu/DGN route completion.

  - 2026-07-16 update: the joint DGN/PRS3 route receipt now also exposes the
    DGN package/host no-raster blockers: material pixel promotion blocked,
    raster input still denied, and DGN fallback visuals still denied. Losing
    any of those retained host facts blocks the join while the PRS3
    no-runtime route and DGN source identities remain visible. Remaining Nexus
    work is authenticated Saturn capture/producer evidence, reviewed PRS3
    decoder semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the PRS3 reviewed output/upload join now rechecks the
    retained decoded-output proof facts before declaring `source-bound-no-runtime`:
    capture source binding, output length match, output hash match, exact
    observed/expected byte count, and nonzero output FNV. Losing any of those
    real-data proof facts blocks the reviewed MENU.BPK upload join without
    promoting a decoder, runtime upload, or fallback visuals. Remaining Nexus
    work is authenticated Saturn capture/producer evidence, reviewed PRS3
    decoder semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the DGN/PRS3 route receipt now also carries the
    retained DGN no-draw host boundary and real-DGN mesh-render blocker after
    package/host consumption. Losing either host fact blocks the join even
    when DGN identity, selector/geometry evidence, no-raster blockers, and
    PRS3 no-runtime evidence still match. Remaining Nexus work is
    authenticated Saturn capture/producer evidence, reviewed PRS3 decoder
    semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the same route now has fail-closed coverage for DGN
    package/host drift that would erase the original-Saturn capture
    requirement or pre-promote Saturn rendering/material semantics. Any of
    those states blocks the DGN/PRS3 join while PRS3 no-runtime evidence and
    the retained DGN no-draw/mesh blockers remain visible. Remaining Nexus work
    is authenticated Saturn capture/producer evidence, reviewed PRS3 decoder
    semantics, Saturn VDP1/material/palette capture, and broader
    startup/menu/DGN route completion.

  - 2026-07-16 update: the DGN package/host consumer now has explicit
    fail-closed coverage for host-route request, retained level identity,
    canonical DGN byte-size identity, and Structure2 descriptor-count identity.
    Any drift blocks source-route consumption before package-host route
    publication, with raster input, material pixel promotion, and fallback
    visuals still denied. Remaining Nexus work is authenticated Saturn
    capture/producer evidence, reviewed PRS3 decoder semantics, Saturn
    VDP1/material/palette capture, and broader startup/menu/DGN route
    completion.

  - 2026-07-16 update: game-owned FIFO payload consumer traces now have a
    bounded file import/corpus gate. A staged local US raw Track 02 plus
    original Mednafen consumer transcript must rehash, parse through the
    `$3840` -> `$e009`/READ(6)/FIFO/main-RAM consumer chain, and reject
    mutated media before it can publish an opaque byte receipt. Remaining
    Theron work is still positive operator-supplied corpus breadth and runtime
    wiring from that consumer receipt into startup/admission; the receipt does
    not prove level, object, bitmap, palette, or visual semantics.

  - 2026-07-16 update: the runtime-admission surface now consumes that
    game-owned FIFO payload receipt directly. Only the verified US raw Track
    02 `$3840` -> READ(6) -> FIFO -> game-RAM consumer chain can set the
    opaque runtime admission bit; payload semantics, visual semantics, and
    fallback visuals remain explicitly false. Remaining Theron work is
    positive operator-supplied corpus breadth and broader startup/session
    plumbing from admitted opaque bytes into real runtime capture, not
    generated presentation.

  - 2026-07-16 update: admitted FIFO bytes now publish a bounded
    runtime-session handoff receipt. The receipt preserves the US raw Track
    02 record, source offset/byte, READ(6), FIFO-to-game-RAM, and game-RAM
    consumer facts, requires a future real runtime capture, and keeps object
    table admission, level admission, payload semantics, visual semantics, and
    fallback visuals closed. Remaining Theron work is still operator-supplied
    corpus breadth and a reviewed startup/runtime capture consumer that uses
    this opaque session receipt without inventing presentation data.

  - 2026-07-16 update: the session handoff can now consume a bounded Track 02
    all-dungeon route receipt and publish only corpus evidence: capture mask,
    no-fallback role mask, startup-level anchor, blocked object-table anchors,
    blocked non-startup-level anchors, and route hashes. This remains
    `runtime-capture-required`: exact level/object semantics, object-table
    admission, level admission, payload semantics, visual semantics, and
    fallback visuals all stay closed. Remaining Theron work is a real capture
    consumer that validates the bounded receipt against operator-supplied media
    breadth before any object/table/level promotion.

  - 2026-07-16 update: the runtime startup-level anchor can now consume the
    detailed Track 02 level-route receipt's non-startup candidate evidence
    without opening admission. The new receipt requires verified descriptor
    anchors, matching startup-level anchor/hash evidence, first opaque
    post-descriptor candidate raw/user-data offsets, byte count, hash, header
    view, and the existing missing-non-startup-evidence blocker. Non-startup
    level admission, exact level/object semantics, payload semantics, visual
    semantics, and fallback visuals remain closed. Remaining Theron work is a
    positive original runtime consumer/capture that proves what these
    non-startup bytes are before any route promotion.

  - 2026-07-16 update: the post-$3800 object/dungeon consumer gates now require
    explicit same-capture consumer PCs plus byte-window evidence for the
    initial level envelope and post-envelope object candidate span. The windows
    must match the already hash-verified loader slices by payload offset, byte
    count, and checksum before object/dungeon grammar or broader consumer
    semantics can bind. Bitmap, palette, RGBA, runtime handoff, synthetic
    promotion, and fallback visuals stay blocked. Remaining Theron work is the
    real original capture/transcript producer that supplies those PCs and
    windows from BIN/CUE Track 02 execution rather than hand-authored facts.

  - 2026-07-16 update: those same post-$3800 consumer facts must now also carry
	    and object state can be proven without promoting palette/pixels. Dungeon
	    runtime admission, draw, synthetic dungeon/object data, and fallback
	    visuals remain denied.
    text, runtime, or fallback-visual semantics. Remaining blocker: a real

  - 2026-07-23 update: the public `DM1_SaveGamePC34()` path now emits an
    untagged PC3.4 envelope through F0803; the former manifest-bearing F0802
    route remains diagnostic-only. The focused F0435 handoff regression proves
    the untagged header is admitted. Remaining proof is an original DOS
    executable roundtrip and an operator-provided original-save corpus.

  - 2026-07-16 update: the low-level V1 title source selector now also skips
    malformed or missing C001 even when TITLE.DAT is available, so TITLE.DAT
    is retained only as rejected evidence rather than a drawable fallback.
    Focused Ninja/CTest passed. Remaining work is unchanged: real PC34/Mac
    title-to-HoC capture plus operator-staged external original-save corpus
    evidence.

  - 2026-07-16 update: the legacy DM1 title state helper now also requires
    an exact 320x200 C001-sized source before loading or publishing a
    real-asset receipt, and its draw pass follows the last requested source
    zoom step instead of the double-buffer index. Focused Ninja/CTest passed.
    Remaining work is unchanged: real PC34/Mac title-to-HoC capture plus
    operator-staged external original-save corpus evidence.

  - 2026-07-16 update: the external original-save corpus probes now compile
    and are CTest-registered across discovery/import/export/roundtrip and
    HoC/runtime materialization. With no local save corpus staged they skip
    cleanly; positive title/HoC completion still requires real PC34/Mac capture
    plus operator-staged original-save files.

  - 2026-07-16 update: Structure1Fa ITEM.IBS descriptor-0008 material now
    reaches the exact DGN floor-command receipt as authenticated packed 4bpp
    source bytes, while explicitly carrying
    `blocked_missing_vdp1_command_provenance` and zero verified VDP1 captures.
    Remaining work is an original Saturn VDP1 command/state/palette capture
    that matches ITEM.IBS and the DGN command route before texel order,
    palette semantics, or any draw can be authorized.

  - 2026-07-16 update: descriptor-0008 command consumption now also rejects
    any drift that mixes the special floor-image lane with regular
    inventory-material fields: palette index, image index, or regular packed
    texel pointers must stay at their sentinel values before the packed
    source bytes can reach the DGN command receipt. Non-floor command kind,
    source-cell mismatch, and out-of-range command index still fail closed
    with no fallback. Remaining work is unchanged: real original-Saturn VDP1
    command/state/palette capture tied to ITEM.IBS and the DGN command route
    before texel order, palette semantics, or drawing can open.

  - 2026-07-15 raster-source binding update: both F0168 world discovery and
    F0172 selected-wall consumption now prove the same raw M648 receipt,
    with each decoded byte bound to `byte << 3`, an unscaled native 8x8 cell,
    original GRAPHICS.DAT entry 258, and C10 transparency. The gate rejects
    padded/scaled font dimensions as well as altered source or destination
    cells. Remaining work is real PC34/Mac capture across every HoC and
    non-HoC projection; side/depth continues to use its original unreadable
    ornament route, not host text.

  - 2026-07-15 fail-closed receipt update: malformed F0172 selected-wall and
    F0168 world TextString lookups now clear the caller's M648/C10 material
    receipt on failure, so stale readable wall text cannot survive a rejected
    raw record. Focused material and M11 inscription CTests pass. Remaining
    work is unchanged: real PC34/Mac capture across every HoC and non-HoC
    projection; side/depth continues to use its original unreadable ornament
    route, not host text.

  - 2026-07-16 selected-wall native-material update: the M11 D1C readable and
    D0-D3 side/depth unreadable consumers now enter the DM1 F0172-selected
    wall TextString receipt instead of the broader F0168 world scan. The
    material gate proves that a hidden selected TextString cannot borrow a
    neighbouring visible record, while real PC34 GRAPHICS.DAT pixel probes
    still prove M648 byte<<3, native 8x8 cells, C10 transparency, and the
    original unreadable ornament palette/clip path. Remaining work is broader
    packaged PC34/Mac capture, not host-font substitution or synthetic text.

  - 2026-07-15 update: the public champion-status layout now exposes the
    F0287 PC34 bar container at y=2, matching the existing live bar-fill model
    and ReDMCSB's 4x25 split. This closes the stale y=0 HUD geometry path;
    remaining work is still real PC34/Mac capture breadth.

  - 2026-07-15 all-C127 source proof update: the C026 portrait source is now
    explicitly bounded to ReDMCSB's 8x3 (24-cell) 32x29 atlas. A malformed
    C127 `sensorData` clears the current mirror route and its materialized
    payload instead of sampling outside C026 or using a fallback. The
    real-PC34 directional gate now walks every C127 sensor in HoC map 0,
    proves its visible-wall-only C346->C026 route, rejects all three other
    wall cells, and records the number of sensors and distinct source
    portraits. Remaining work is an operator-captured Mac/release frame for
    these genuine routes, not a host portrait substitute.

  - 2026-07-15 direct-plan atlas update: the lower C346/C026 front-mirror
    render plan now rejects any portrait index outside ReDMCSB's 24-cell C026
    atlas before computing source coordinates, matching the runtime receipt's
    fail-closed route. Remaining work is still the operator-captured
    Mac/release frame for these genuine routes, not a host portrait
    substitute.

  - 2026-07-17 D1 visibility update: the C127 viewport projection receipt
    distinguishes D1C portrait overlay from the D1L/D1R F0107 backing routes.
    It admits global ornament 43 only at D1L view-wall 10 or D1R view-wall 11;
    D2/D3 or mismatched-view C127 facts suppress generic ornament material and
    cannot synthesize C346/C026. Focused DM1 and M11 HoC receipt tests and the
    full app build pass.

  - 2026-07-31 F0107 side/depth correction: C127 selects the original
    C345/C346 mirror-frame pair in every source-defined D1-D3 F0107 ornament
    projection, not just D1L/D1R. C026 remains strictly D1C-only. The live
    renderer now routes C127 before map-local ornament lookup, because the
    sensor owns C346 directly. Real PC34 material tests cover D1 side plus D2
    and D3 backing, while the separate D1C suite retains C346/C026 ownership.
    Remaining work is Mac/release-frame capture, not a missing side/depth
    mirror-frame route.

  - 2026-07-15 update: the live M11 app boundary now re-composes the current
    C001--C005 plan and requires byte-for-byte indexed-raster equality plus
    its exact F0437/F0441 palette selection before accepting SDL RGBA output.
    A stale title phase, foreign door step, or host wrapper page is no-draw.
    Remaining work is an external Mac window capture of the complete sequence.

  - 2026-07-15 update: `FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR` records one
    post-present SDL RGBA BMP per source palette phase only after the live
    C001--C005 raster and palette gate pass. It cannot capture a stale phase,
    foreign door step, or host wrapper page. External Mac-window comparison
    remains required for original pixel parity.

  - 2026-07-15 update: fixed the startup presentation receipt off-by-one that
    compared `TITLE.C F0437` source step against the previous title frame.
    First and last CHAOS zoom frames now produce valid real C001 render plans.
    Remaining work is still the real Mac/window capture and broader app route,

  - 2026-07-15 update: the title/opening consumption receipt now requires the
    host opening raster to prove C004 entrance plus C002/C003 door-strip
    composition from the same package session. Remaining work is still external
    original-window capture and broader live app coverage, not host-wrapper or
    partial opening-surface admission.

  - 2026-07-15 update: the startup session contract now requires the resident
    decoded C001 title, PRESENTS/CHAOS/STRIKES crops, C004 entrance, and
    C002/C003 door surfaces to retain their exact PC34 dimensions and source
    asset ids before any terminal, title/opening, or opening-door receipt can
    cross into HUD/runtime. Remaining work is still external original-window
    capture and broader app-route parity, not one-byte or host-owned package
    stand-ins.

  - 2026-07-15 update: the terminal F0806/F0807 door-to-HUD handoff now gates
    on that same full resident surface contract before HUD playback, so a
    valid C017/C040 pair cannot bypass malformed C001/C004/C002/C003 startup
    media. Remaining work is still external original-window capture and
    app-route parity, not synthetic title/entrance replacement.

  - 2026-07-15 update: the title/opening consumption receipt now also binds
    each host C001 title capture to its F0437 source step, phase mask,
    palette, nonzero route hash, and distinct indexed raster hash, so
    PRESENTS cannot be relabeled as CHAOS/STRIKES and one stale title page
    cannot satisfy the complete sequence. Remaining work is still external
    original-window capture and app-route parity, not a host-wrapper,
    duplicated-phase, or synthetic title replacement.

  - 2026-07-15 update: CSB C001 title admission now keeps the decoded
    GRAPHICS.DAT surface at its full 320x200 size, with PRESENTS, CHAOS, and
    STRIKES BACK remaining separate cropped F0437 regions. A cropped 320x153
    C001 stand-in no longer satisfies the startup session or package
    consumption receipts. Remaining work is still external original-window
    capture and broader app-route coverage, not a synthetic or cropped title
    replacement.

  - 2026-07-15 update: opening-door and title/opening consumption receipts
    now require playback to still be in the active ENTRANCE stage, after the
    complete C001 title phases and before F0807 promotes the session to HUD.
    A stale HUD-stage C004/C002/C003 host frame can no longer be recorded as
    a valid opening capture. Remaining work is unchanged: external
    original-window capture and broader app-route parity.

  - 2026-07-16 update: opening-door and title/opening consumption receipts
    now also require the package receipt and opening host frame to match the
    active startup session source tick. A stale package tick or reused
    C004/C002/C003 host frame from another startup tick cannot be promoted
    into the entrance receipt. Remaining work is unchanged: external
    original-window capture and broader app-route parity, not a tick-stale
    host capture or synthetic startup replacement.

  - 2026-07-16 update: the title/opening consumption receipt now pins each
    host C001 title frame to the resident session title surface and pins the
    C004/C002/C003 opening host frame to the resident entrance and door
    surfaces, matching the narrower opening-door receipt. Swapped owner
    pointers with plausible raster facts are rejected. Remaining work is
    unchanged: external original-window capture and broader app-route parity,
    not a wrapper-owned startup surface.

  - 2026-07-16 update: the title/opening consumption receipt now also requires
    PRESENTS, CHAOS, and STRIKES BACK host captures to carry a non-stale,
    ordered source-tick sequence immediately before the active C004/C002/C003
    opening tick. A replayed old C001 phase can no longer satisfy the current
    title-to-opening receipt. Remaining work is unchanged: external
    original-window capture and broader app-route parity; the local opt-in
    real-package launch probe still has broader real-data/hash failures.

  - 2026-07-17 update: the F0217 Ven/Ful impact consumer now admits only a
    byte-identical C14 Slot and C05 power/type record, publishes its C15/C25
    owner before runtime F0213, and rolls C15/SFT/runtime/timeline state back
    on drift, pool exhaustion, or schedule failure. Other projectile impact
    families remain separate source-owned work; they do not inherit a
    synthetic C15/C25 fallback.

  - 2026-07-17 update: F0218's deferred C38 impact admission now walks the
    loaded SFT C14 chain and requires raw/decoded/runtime identity per cell
    before F0190 compaction. A missing or drifted C14 owner is a no-mutation
    rejection, never a scan of host-only projectile slots.

  - 2026-07-17 update: F0214 queue compaction now writes every shifted live
    C14 `EventIndex` back to raw bytes before any later save/export boundary.

  - 2026-07-17 update: loaded DM1 spell and creature F0212 callers now
    publish the C14/C49 pair atomically through the verified transaction
    boundary; missing C14 ownership, duplicate C49 ownership, or any publish
    failure restores the runtime/timeline state. Remaining F0212 work is
    caller breadth, not a synthetic projectile fallback.

  - 2026-07-17 update: F0221 now supplies F0219's fluxcage blocker only from
    the loaded destination SFT chain. Each C15 is raw/decoded-verified before
    its C050 type can consume a projectile; a malformed C15 rejects before
    any C14 or runtime mutation. Remaining F0221 work is broader caller
    adoption, not a runtime-list substitute.

  - 2026-07-16 update: the DM1 wall-material scheduler now source-locks the
    F0128 D0-D3 wall renderplan against ReDMCSB draw-order rows and keeps
    D2L/D2R C710/C711 side-wall receipts alive even when nearer D1L/D1R side
    lanes are closed. Same-lane side occupancy remains an occlusion rule for
    later floor/content/effect passes, not for F0116/F0117/F0119/F0120 wall
    materialization. Remaining viewport work is broader real GRAPHICS.DAT
    capture parity across doors, mirrors, inscriptions, things and fields.

  - 2026-07-19 update (Jobb E part 1, first slice): the complete per-square
    passes 86/86 contract assertions data-free. Remaining part-1 work:
    broaden real PC34/Mac capture parity (the live M11 wiring landed
    2026-07-19, see DONE).

  - 2026-07-16 update: the live CSBWin `SetTimer` duplicate-policy matrix for
    C05..C10 map timers now stays inside the CSBWin owner. Matching timer
    functions on the same square/time replace only the action byte, TT_STONEROOM
    still requires the same cell/position, and different timer functions on the
    same square append as separate source-owned timers instead of falling
    through the shared DM1 merge helper. Remaining work is full requeue
    semantics from a real CSBWin save corpus.

  - 2026-07-22 update: boot now publishes an inventory receipt only after a
    second classification exactly matches the manifest-admitted cache index.
    Missing or mismatched media is clear-only. The remaining blocker is still
    real type/id-to-entry capture, not a generated bitmap or palette fallback.

  - 2026-07-22 update: FIO1's portable caller-owned routes now map F1321,
    F1323, F1328-F1336, F1338-F1339, and F1341-F1342 over existing PC34
    FILE.C callbacks with source-style success/error contracts. Floppy, format,
    lock, and drive-specific operations remain outside the portable boundary.

  - 2026-07-22 update: the FILLBOX-specific F0692 now delegates caller-owned
    planar bitmap, inclusive box, and color to F0135. IMAGE3's packed-raster
    F0692 stays a separate route; no video data is generated by this bridge.

  - 2026-07-15 follow-up: `STKOP_Mastery` now binds `DSA.cpp:3389-3409` to
    the same loaded CHARDESC rows, including selector-four hand character,
    temporary-XP suppression, hidden-skill averaging, and transient
    `PartySleeping`. Only callers that request CSBWin's possession suppression
    are accepted: Firestaff has no verified original name-index owner for
    possession bonuses, so the unflagged route remains unavailable rather
    than mapping a host item.

  - 2026-07-15 update: M11 now binds source-required item/icon material
    receipts to the same G1 scene-control identity as the viewport frame, so
    a matching item material hash/count cannot be replayed from another scene.
    Remaining object-icon work is save-corpus state binding and broader
    inventory/hand/possession capture, not a local icon fallback.

  - 2026-07-17 update: `DRAW_DOOR_FRAMES`' bounded `yy & 1` roof-slit
    `DRAW_DUNGEON_GRAPHIC` transaction now has an M11 consumer for only the
    source table pairs `table1d6efd[3..8]` (`0x12..0x17`) and
    `table1d6f0b[3..8]` (`0x2f2/0x2f1/0x2f3/0x2ef/0x2ee/0x2f0`). It requires
    authenticated GRAPHICSSET raw/IMG3/PAL16, RAW4 table/row, scene key,
    composition and live surface identities. The missing broader work remains
    source-selected roof-slit flag delivery, stretch/light-palette execution,
    centre-frame placement and unproven door transforms; no generic roof art,
    inferred cell, scaling or fallback is permitted.

  - 2026-07-17 update: the default D1C door-button route is now executable
    only at the source identity scale `table1d6b71[1] == 0x40`, with
    `DOOR_BUTTONS/0` field 0 or 5 and `table1d6ed3[3] + 0x79e == 0x7a1`.
    Remaining button work is D0/D2/D3 scaling, custom WALL_GFX buttons and
    source-selected live geometry delivery; none may borrow this D1C path.

  - 2026-07-23 update (Lane C, cycle 10): source-locked door panel/button
    Remaining: distance stretch/light-palette execution, `DRAW_DOOR_FRAMES`,
    D3/custom WALL_GFX buttons, door opening transforms, and verified GDAT
    material.

  - 2026-07-15 update: the CSB startup presentation receipt now rejects an
    active door-opening frame until the ReDMCSB C002/C003 animation has a
    real step in the 1..31 range. The pre-open delay may still retain the
    closed C004/C002/C003 surface, but step 0 and step 32+ no longer publish
    an opening-frame receipt. Remaining work is still external Mac/app
    capture.

  - 2026-07-15 update: the lower-level CSB source render-plan route now uses
    the same C002/C003 step gate directly, so a host cannot bypass the receipt
    path and obtain an opening-frame plan with step 0, step 32+, or a pre-open
    delay carrying a real door step. Remaining work is still external
    Mac/app capture.

  - 2026-07-15 owner-corpus update: `firestaff_nexus_v1_structure1f_owner_corpus_probe`
    now rechecks every canonical Structure1F owner row against its exact
    Structure1A record and emits one all-level raw-owner fingerprint. The
    documented Structure1A model byte remains only a source field: it does not
    prove Structure3 entry selection, mesh placement, texture, palette, VDP1,
    transform, decoder, or drawing semantics.

  - 2026-07-15 static descriptor-face corpus update: all canonical `00xx`
    Structure3 textured faces now revalidate their local Structure2 descriptor
    and bounded raw anchor fields under one corpus fingerprint. The anchors
    remain opaque: pixel codec, palette format, UVs, VDP1, transforms and draw
    behavior still require original Saturn evidence.

  - 2026-07-15 static capture-window update: each static face descriptor now
    carries a bounded raw image-anchor interval and, when present, palette
    anchor interval derived only from the next original descriptor anchor or
    payload end. These are capture windows, never pixel/palette lengths or
    decoder/VDP1 semantics.

  - 2026-07-15 raw-window witness update: every bounded static image/palette
    window now contributes its original bytes to the corpus hash. This proves
    source identity for future capture comparison, not payload grammar.

  - 2026-07-16 Nexus Structure1A relation hardening: the host provenance
    receipt no longer accepts a cached Structure1F relation flag unless the
    live owner table still has exactly one matching source cell and the
    cached model/rotation bytes still match the Structure1A row. Stale
    owner coordinates or stale row bytes now block before runtime DGN
    preparation, so cached relation data cannot replace source-owned DGN
    records. Remaining blocker is unchanged: original Saturn evidence must
    still prove Structure1A selector semantics, transforms, texture/palette,
    VDP1, culling, and draw behavior.

  - 2026-07-14 update: the full skproject `LOAD_GDAT_INTERFACE_00_02` HUD
    plan now carries its exact consumed command hash from viewport through
    runtime and boot to M11. A source-required HUD frame is rejected if any
    plan command is missing; no generic HUD lookup can promote a partial
    plan. Remaining weather work still requires original timer destination
    and clipping evidence before any overlay is admitted.

  - 2026-07-14 update: the visible direct-DB4 creature plan now carries the
    exact `CREATURES/type/F9` raw bytes, local palette, object ID, tile,
    direction, and atlas placement identity through runtime, boot, and M11.
    Mixed dynamic-creature frames stay no-draw at M11 until their original
    GDAT owner route is equally proven; no type-index substitute is accepted.

  - 2026-07-15 update: an unowned local creature candidate cannot derive a
    drawable GDAT key from its type/frame, and Rect14 cannot promote that
    candidate into a sprite. Only a proven live `QUERY_CREATURE_PICST` field
    or an exact G1/DB4 material receipt may draw. Remaining work is binding
    dynamic AI/animation ownership to an equally source-backed live route.

  - 2026-07-15 update: the final DB4 `CREATURES/type/F9` viewport gate now
    also consumes the direct record's `b15_0_1` direction. A material receipt
    cannot be replayed after a direction change even when object ID, tile,
    type, decoded dimensions, and local palette still match. The canonical
    G1 regression proves both the accepted source direction and the blocked
    mismatch; dynamic AI-owned orientation remains separately unavailable.

  - 2026-07-15 update: every successful creature GDAT blit now records its
    source key in draw order. Runtime folds the complete list into the M11
    material receipt and rejects a count/hash mismatch, so a multi-creature
    frame cannot be represented by the last blit alone. Remaining work is
    authentic dynamic CCM/DB4 ownership and mutable animation state.

  - 2026-07-15 update: every visible `DRAW_WALL` GRAPHICSSET panel now also
    reaches frame ownership in source draw order, with a separate visible
    command count/hash. The receipt distinguishes the ten consumed viewport
    fields from the broader cached material plan; remaining work is exact
    original movement/light clipping and additional scene families.

  - 2026-07-15 update: every successful skproject
    `QUERY_DUNGEON_MAP_CHIP_PICT`/`DRAW_CHIP_OF_MAGIC_MAP` missile or cloud
    blit now carries its real GDAT key in draw order into frame ownership and
    the M11 receipt. A count/hash mismatch fails closed; the last projectile
    diagnostic can no longer stand in for a mixed overlay frame. Remaining
    work is original dynamic projectile/AI ownership and timing, not a
    generic spell-image fallback.

  - 2026-07-15 update: every successful source-owned floor-object,
    creature-possession, and leader-hand map-chip blit now carries its GDAT
    key and source pass in presentation order to frame ownership and M11.
    A count/hash mismatch fails closed, so a final object diagnostic cannot
    represent a partial mixed object pass. Remaining work is original mutable
    object-chain ownership and timing, not a generic `MISC` image fallback.

  - 2026-07-15 update: a visible skproject `DM2_DRAW_DOOR` transaction now
    reaches M11 only when every exact plan command was consumed, rather than
    when any one door material happened to resolve. The command count joins
    the existing source-plan hash; partial panel/frame/overlay/button plans
    fail closed. Remaining work is original light-palette and split-panel
    behavior, not a generic door surface.

  - 2026-07-15 update: the complete door-plan command count now crosses the
    boot/M11 host boundary with its hash. Host presentation rejects stale or
    mismatched counts; it cannot reduce a multi-command door transaction to
    a boolean consumed bit.

  - 2026-07-15 update: the visible `DRAW_WALL` command count now crosses the
    boot/M11 host boundary with its source hash. M11 rejects a stale count,
    so a shorter wall panel list cannot reuse an otherwise valid material hash.

  - 2026-07-15 update: ordered projectile GDAT receipts now cross boot/M11,
    including their exact count. A stale missile/cloud count is rejected at
    presentation; no generic projectile image can fill the gap.

  - 2026-07-15 update: ordered object GDAT receipts now cross boot/M11 with
    their exact count. A stale floor-object, possession, or leader-hand pass
    is rejected before presentation; no generic `MISC` image can fill it.

  - 2026-07-15 update: the synthetic injected-provider leader-hand path now
    proves its bounded item map-chip blit, receipt, and M11 material handoff
    without a boot GDAT parser. Remaining `dm2_v1_runtime_handoff_smoke`
    blockers are independent palette, wall-count, DB0 door/button/overlay,
    destroyed-door mask, and wall-gfx button material gaps.

  - 2026-07-22 update (Lane B, cycle 3): the remaining
    `dm2_v1_runtime_handoff_smoke` blockers from the 2026-07-15 list are
    closed.  DB0 closed-door panel GDAT index selection, CCM creature-tick
    door open-percent reporting, deterministic timeline display-message
    target, and wall-gfx custom-button discovery for both DB2 text records
    and DB3 actuator records now pass.  `test_dm2_v1_runtime_handoff_smoke`
    reports 167/0 PASS/FAIL.  Broader real-map custom wall-gfx/source-material
    proof and DM2 real-corpus capture remain open.

  - 2026-07-15 update: multi-creature GDAT receipts now cross boot/M11 with
    their exact count. A shortened creature list is rejected before frame
    presentation; no inferred type/frame sprite is admitted.

  - 2026-07-15 update: source-backed weather renderer hash/count now cross
    boot/M11. A mismatched weather command list is rejected before host
    presentation; no intensity-only overlay is admitted.

  - 2026-07-15 update: every source-owned `DRAW_DOOR` panel, frame, button,
    ornate, and destroyed-mask plane is rehashed at the viewport boundary.
    A modified decoded image or local palette blocks the complete door pass
    before the first blit; same-sized material cannot impersonate GDAT.

  - 2026-07-15 update: the final weather viewport fetch now also matches the
    receipt's decoded ENVIRONMENT pixel and local-palette hashes. A same-sized
    altered image or palette blocks the complete cloud/rain transaction before
    any layer draws; no host-side substitute can satisfy the receipt.

  - 2026-07-14 update: `DM2_DRAW_DOOR`/`DRAW_DOOR_FRAMES` now transfers the
    exact consumed multi-category door-material plan hash through runtime,
    boot, and the M11 gate. Doorless frames deliberately carry no invented
    identity; a frame that drew a door is rejected when its real plan was not
    consumed. The remaining weather work is still blocked on source-owned
    live timer destination/clipping state, not on a synthetic overlay.

  - 2026-07-14 update: the live dungeon candidate list now comes only from
    the committed direct-DB4 `CREATURES/type/F9` material receipt. It retains
    the original ObjectID, tile, direction, decoded dimensions, and local
    palette owner through M11; generic record-link traversal cannot nominate
    a creature sprite. This is a static map-chip handoff only: animation
    selection and dynamic lighting remain unavailable.

  - 2026-07-14 update: the active G1 map's original offset bytes now reach
    SKProject `SET_GRAPHICS_FLIP_FROM_POSITION` for decoded plane rects
    700/701. No unproven tile/material route is enabled by this binding.

  - 2026-07-14 update: the bounded D0/D1/D2 center ray and matching side
    rays now convert the
    active G1 byte square through SKProject `DME.h::tileTypeIndex` before it
    reaches the existing GDAT wall/floor consumer. G1 `0=wall` and `1=floor`
    cannot be confused with the host enum; class `4` remains on the direct
    DB0 door route and every other class stays unavailable.

  - 2026-07-14 update: D3L/D3R now use SKProject's deep projection
    coordinates (five forward, two lateral) and feed the same GDAT wall
    consumer. D3C remains unavailable because the active GRAPHICSSET exposes
    no source wall field for it; no fallback surface is drawn. Dynamic light
    and non-wall/floor terrain remain open.

  - 2026-07-14 update: M11 now also consumes the live
    `INTERFACE_GENERAL/0/dt07/0x02` action-palette identity alongside the
    interface palette. Remaining work is the source-owned dynamic light
    input, not a substitute HUD palette.

  - 2026-07-14 update: recursive corpus discovery now also retains a renamed
    artifact only after its exact 42-byte SKSave header and full source-bound
    payload parser both succeed. Filename or extension alone cannot consume
    a scan slot or promote a candidate; runtime import still revalidates the
    complete-file hash. Remaining work is authentic corpus breadth plus
    rebuilt-dungeon byte ownership.

  - 2026-07-15 update: raw original candidates now retain the exact encoded
    SUPPRESS timer byte window as offset, byte count, and hash after the
    complete file hash and raw importer agree on the candidate. This is byte
    ownership only; timer dispatch semantics and rebuilt dungeon DB graph
    ownership remain separate.

  - 2026-07-16 update: DM2 complete-support publication now also requires
    the original-save state corpus probe to find and parse at least one
    source-owned original SKSave candidate with no rejected original
    candidates. A scan with zero parsed original states remains observable
    as incomplete evidence and cannot promote `complete-support-ready`.
    Remaining work is an operator-staged real original-save corpus plus the
    rebuilt dungeon DB/timer semantics needed to consume that parsed state.

## DM1 V2.2 Finished-Art Follow-up (2026-07-12)

The live V2.2 selection path now rejects synthetic and partial modern-art
Remaining work is those runtime material bindings and real Mac/app capture,
not pack admission. The selected-menu regression still rejects missing or
partial packs rather than silently using a placeholder or synthetic pack.
also applies palette interpolation without requiring dither cleanup. Remaining

## DM1 PC34 Original Save Follow-up (2026-07-11)

The native PC34 importer now transactionally validates the five source save
parts, including the `GLOBAL_DATA.MaximumActiveGroupCount`-sized ACTIVE_GROUP
block, the fixed `M516_CHAMPIONS + PARTY_INFO` PC34 PARTY block, and the fixed
four-portrait section before publishing header, party, or timeline state. The
EVENT/TIMELINE handoff also validates every decoded index and commits a fully
staged queue, so a malformed report cannot alter a live queue. The DM1 world
handoff restores ACTIVE_GROUP records through the same candidate-world path.
Remaining original-save interop work is original dungeon-tail import breadth
and real community-save corpus round-trip evidence.

  - 2026-07-14 update: F0435 now materializes the source-owned one-byte C73
    Thieves Eye and C79 Footprints counters from `PARTY_INFO` offsets 2 and
    3 into the existing M10 magic state; C79 also restores its active
    footprint flag. F0433 writes only those bounded counters back to the
    retained source bytes.

  - 2026-07-14 update: F0435 now restores ReDMCSB `PARTY.FreezeLifeTicks`
    from `PARTY_INFO` byte 11 into the M10 world and magic-state owners. The
    live periodic runtime decrements both owners together, and F0433 writes
    the bounded byte back to the retained source record. The targeted
    authenticated-PC34 regression covers import, a live periodic tick, and
    export/reimport. F0435/F0433 now also own the one-byte `ScentCount` at
    offset 10 through the existing F0412 Footprints-window runtime owner.
    F0435/F0433 now also own `FirstScentIndex` at byte 84 through that same
    runtime field; scent arrays, `LastScentIndex`, Event71, and unreferenced
    `PARTY_INFO` bytes remain opaque pending their own owners.

  - 2026-07-14 update: the fixture-free external-corpus M11 runtime gate now
    drives every F0435-admitted original PC34 save through the live game view,
    checks the source party pose/tick and owned dungeon against an independent
    F0435 materialization, draws an M11 frame, and rejects any invented C040
    HoC panel or stale sidecar state. It requires operator-staged original
    save bytes plus original DM1 media and skips when either is absent; no
    synthetic save, dungeon, or HoC fallback can satisfy it. Remaining work
    is independently collected community saves with broader tail/map coverage.

  - 2026-07-14 update: the focused M11 runtime interop route now proves a
    checksum-qualified original PC34 F0435 load can be quicksaved by the live
    host, exported through F0433 as a new PC34 envelope, reclassified, and
    loaded through M11 again with party pose, tick, champion count, and live
    dungeon ownership intact. This is a bounded host load-save bridge; it
    neither admits an external corpus nor substitutes a tail/dungeon.

  - 2026-07-14 update: checksum-authenticated C4 data must now also preserve
    ReDMCSB `TIMELINE.C F0234` heap order, not only its C3 membership. The
    importer rejects the first parent/child ordering violation before staged
    PARTY or runtime state can commit. Remaining work is authentic tail/save
    corpus coverage, not accepting reordered timeline records.

  - 2026-07-14 update: each external PC34 snapshot now also receives a
    no-fallback F0435 runtime-stage receipt before transient export. A source
    tail must materialize its own owned dungeon and carry its C13 count into
    the staged timeline; a tail-less source remains explicitly not runtime
    ready and never borrows a host start dungeon. Remaining work is broader
    authentic tail coverage and independently collected community saves.

  - 2026-07-14 update: startup, boot-summary, host-field, expectation, and
    diagnostic receipts now carry the F0435 candidate-to-live adoption counts
    separately from byte-roundtrip evidence. Only an external corpus whose
    every qualified PC34 candidate adopts its owned dungeon/timeline can pass
    the explicit runtime-corpus expectation; a tail-less or failed adoption
    remains visible as non-runtime-ready. Remaining work is still an
    independently collected, provenance-recorded PC34 corpus and original
    executable load evidence, not synthetic saves.

  - 2026-07-14 update: the F0435 candidate-to-live commit now rejects an
    incoherent F0238 queue before moving either live owner. The staged queue
    must retain the restored GameTime, C4 active-entry uniqueness, and event
    count shared with the materialized timeline. Remaining work is authentic
    external tail/save coverage, not repairing or synthesizing a queue after
    adoption.

  - 2026-07-14 update: the focused C13 runtime regression now follows
    `TIMELINE.C F0255` from step 2 through the matching-bones step 1 unlink
    and the one-tick-later step 0 rebirth. A bones-missing live step 1 ends
    without a fallback rebirth, while a saved step 1 still requires its owner
    at F0435 handoff. Remaining work is authentic external tail/save coverage,
    not a synthetic C13 completion path.

  - 2026-07-13 update: every classifier-qualified external PC34 corpus file
    now emits a separate source path/hash/size, game-id, transient export
    hash/size, and result receipt for the F0435 -> F0433 -> F0435 handoff.
    Firestaff manifests and nonoriginal envelopes remain explicit rejected
    rows and never enter import/export. Remaining work is broader original
    dungeon-tail import breadth and independently collected community saves.

  - 2026-07-13 update: corpus receipts now have an explicit transaction
    commit boundary. A failed F0435 -> F0433 -> F0435 candidate may retain
    raw C13/C24/C25 diagnostics for investigation, but cannot publish those
    fields as committed corpus evidence; only a successful core match with a
    transient exported hash commits them. This follows ReDMCSB `LOADSAVE.C`
    F0433/F0435's whole-save staging contract. Remaining work is authentic
    community-save coverage, not a partial-receipt fallback.

  - 2026-07-13 update: rejected external PC34 rows now retain their local
    F0435 staging boundary: handoff result, importer result, and the number
    of checksum-qualified parts. The probe uses only candidate-local
    `SaveGame`/party/timeline objects and does not publish a rejected import.
    This distinguishes malformed post-portrait tail bytes after all five
    source parts from a later roundtrip mismatch while keeping C13/C24/C25
    evidence uncommitted. Remaining work
    is authentic community-save coverage, not a fallback import path.

  - 2026-07-13 update: C4 TIMELINE now rejects a duplicate active EVENT
    index before a candidate party or timeline can commit. The failure receipt
    names both C4 slots and their repeated raw EVENT index; all five F0435
    parts may be checksum-valid, so this is separate from byte corruption.
    Remaining work is authentic community-save coverage, not duplicate-event
    normalization.
    The runtime entry regression additionally proves its candidate world,
    queue, and caller receipt all remain unchanged on this rejection.

  - 2026-07-13 update: a C4 entry outside the decoded F0433 EVENT array now
    fails with its source slot and raw index receipt. Sparse valid EVENT
    indexes remain admissible; only a non-existent or `EVENT_NONE` reference
    is rejected before runtime commit. Remaining work is authentic community
    save coverage, not timeline compaction.

  - 2026-07-13 update: an in-range C4 index whose C3 record is `EVENT_NONE`
    now has separate exact provenance (`slot`, `index`, `is_none`) from an
    out-of-range index. It fails before party/timeline commit; the runtime
    candidate and caller receipt retain their prior state.
    The focused fixture also locks the adversarial relation where that C4
    index is exactly `FirstUnusedEventIndex`: a tombstone can own the free
    list but can never be scheduled through the active C4 heap.

  - 2026-07-13 update: `GLOBAL_DATA.FirstUnusedEventIndex` now rejects an
    active C3 owner. ReDMCSB `TIMELINE.C` F0651 rebuilds that free-list only
    from `EVENT_NONE` tombstones after F0435; Firestaff records the active
    owner index and type and refuses to publish the malformed candidate.

  - 2026-07-13 update: the raw two-byte `UNUSED_EVENT.NextUnusedEventIndex`
    overlay in an `EVENT_NONE` C3 slot is now regression-covered as ignored
    staging data. A checksum-valid tombstone link aimed at the active C4 Door
    event leaves the Door scheduled and the tombstone free, matching
    `TIMELINE.C` F0651's post-F0435 free-list rebuild. Remaining work is
    authentic community-save coverage, not raw free-list-link import.
    A two-slot raw chain whose final saved link names a valid active C4 event
    is also covered: queue insertion consumes the two `EVENT_NONE` slots in
    order and never follows the stale raw link into the active owner.

  - 2026-07-13 update: each external corpus candidate now also records the
    source/exported PC34 header identity (`FormatID`, `GameID`, `Platform`,
    `DungeonID`) and all five F0433/F0435 length-prefixed part byte counts.
    The opt-in real-corpus probe requires those source-owned values to match.
    Header Noise, regenerated keys/checksums, and AdditionalData manifest
    bytes are intentionally not treated as original mirrors. Remaining work
    is independently collected external PC34 saves, not a synthetic header
    or part layout.

  - 2026-07-13 update: the external corpus receipt now records the complete
    decrypted C2 `M516_CHAMPIONS` prefix as four fixed 319-byte records with
    source/export counts and fingerprints. The opt-in real-corpus probe
    requires raw byte identity. Remaining work is independently collected
    external PC34 saves, not a synthetic champion block.

  - 2026-07-13 update: the external corpus receipt now records the complete
    decrypted C4 `TIMELINE` index array with exact index count, byte count,
    and fingerprint. The opt-in real-corpus probe requires raw C4 identity;
    this extends the existing C13-only reference receipt without changing
    importer or exporter behavior.

  - 2026-07-13 update: opt-in corpus discovery is now scoped to its explicit
    directory only. Each inspected regular file records size, header-prefix
    bytes fingerprint, classifier shape/reason, eligibility, and final error
    or roundtrip result; missing roots are explicit errors rather than empty
    successes. Real-corpus verification rejects discovery truncation and an
    empty candidate set. It neither searches game-data roots nor opens other
    games. Remaining work is independently collected external PC34 saves,
    not broader filesystem discovery or a synthetic corpus.

  - 2026-07-13 update: discovery now receipts ReDMCSB save `FormatID`,
    `Platform`, `DungeonID`, and `GameID` per external candidate. Only
    checksum-valid PC34 DM `(5,9,10)` identities can enter F0435; all other
    decoded save-version/platform combinations remain hard rejected. Remaining
    work is external PC34 corpus coverage, not a compatibility fallback.

  - 2026-07-13 update: corpus verification now compares the optional F0433/
    F0435 dungeon tail as raw original bytes after the four portrait payloads.
    A source tail must retain checksum-qualified size, fingerprint, and exact
    bytes through transient export; a tail-less source must remain tail-less.
    The opt-in real-corpus test rejects an empty candidate set, so it cannot
    claim a green real-save pass without external PC34 bytes. Remaining work
    is independently collected community saves and runtime ownership for
    still-opaque tail structures, not a synthetic tail format.

  - 2026-07-13 update: C13 Vi Altar records now receive a byte-preservation
    receipt during the F0435 -> F0433 -> F0435 verification path. ReDMCSB
    `LOADSAVE.C` saves and reloads the whole ten-byte EVENT part; Firestaff
    therefore matches `Map_Time`, Type, Priority, `B.Location`, and
    `C.Cell/Effect` as an order-independent raw-record set after timeline
    rebuilding. A truncated EVENT part or any C13 byte mismatch fails the
    round trip; no generic timer substitution is permitted. Remaining work
    is authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: C13's companion C4 TIMELINE record now has an
    independent raw-16-bit-index receipt. ReDMCSB `LOADSAVE.C` F0433 writes
    `EventMaximumCount * sizeof(int16_t)` and F0435 reads it before F0651;
    Firestaff compares each active C13 index byte-for-byte after the native
    handoff. A compacted, reordered, missing, or extra C13 reference fails
    closed instead of being normalized into a new save layout. Remaining work
    is authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: opt-in external-corpus receipts now retain canonical
    ten-byte C13 EVENT payload counts, byte lengths, and fingerprints plus
    the raw C4 little-endian C13 reference sequence. EVENT receipt rows sort
    only for the receipt because `TIMELINE.C` F0651 may reorder storage; C4
    references remain in original order. A real corpus row must preserve both
    fingerprints exactly. Remaining work is authentic community-save coverage
    and broader dungeon-tail import.

  - 2026-07-13 update: the independent post-part portrait section now has an
    exact F0435 -> F0433 -> F0435 byte receipt. ReDMCSB `LOADSAVE.C` F0433
    writes four fixed 32x29 payloads after the five parts and F0435 reads all
    four before publishing the save. Firestaff records offset, length, and
    fingerprint, then compares the transient source/export bytes directly.
    A missing, short, or changed portrait block fails closed. Remaining work
    is authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: C2 PARTY now retains each inactive `M516_CHAMPIONS`
    319-byte record as opaque F0435 provenance and F0433 writes it back only
    when that original record exists. The verifier decrypts each source and
    transient-export PARTY part with its own original key and compares every
    inactive slot byte-for-byte. No inactive record creates a live champion;
    missing, malformed, or changed records fail closed. Remaining work is
    authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: C13 materialization now consumes its original
    `CLIKVIEW.C F0374` owner relation at ReDMCSB `TIMELINE.C F0255` step 1:
    the saved `B.Location`/`C.Cell` must resolve to a live PC34 JUNK bones
    record (`type=5`) whose two-bit `ChargeCount` equals `EVENT.Priority`.
    An absent or different record rejects that dereferencing branch; C13 can
    no longer reinterpret Priority as a free champion selector. Saved steps
    2 and 0 retain F0255's source behavior: step 2 has not yet read bones,
    while step 0 follows step 1's unlink and therefore resumes without them.
    Remaining work is authentic community-save coverage and broader
    dungeon-tail import.

  - 2026-07-13 update: C13 now has a separate C2 PARTY byte receipt.
    For every saved C13, F0435 -> F0433 -> F0435 decrypts each save's own
    C2 part and compares the exact 319-byte active `M516_CHAMPIONS` record
    selected by `EVENT.Priority`. Missing active slots or altered selected
    records fail closed; the corpus receipt exposes the same count/result.
    Remaining work is authentic community-save coverage and broader
    dungeon-tail import.

  - 2026-07-13 update: the C2 tail is now retained as the original 128-byte
    `PARTY_INFO` record. ReDMCSB `DEFS.H` places magical light, shield,
    scent and unreferenced bytes there; it is not `GLOBAL_DATA`, so F0433
    re-emits imported provenance instead of writing party coordinates into
    that region. The roundtrip and corpus receipts compare all 128 bytes.
    F0435 now materializes the proven leading signed light and three shield
    words into existing M10 owners, and F0433 writes only those owned words
    back with signed-16-bit range rejection. The C73/C79 counters, scent,
    freeze-life, and unreferenced bytes remain opaque pending their own
    source-backed runtime owners and authentic community-save coverage.

  - 2026-07-13 update: the atomic C2 `PARTY_INFO` receipt now records the
    full 128-byte source/export fingerprints after each save's own F0417
    decode key. The existing C3 receipt already fingerprints every raw EVENT
    byte at the F0433/F0435 boundary, so C2 and C3 identity can now be audited
    together without interpreting opaque PARTY_INFO bytes. Remaining work is
    authentic community-save coverage and source-backed owners for the opaque
    PARTY_INFO fields.

  - 2026-07-13 update: external corpus certification now requires raw C3
    EVENT and C4 TIMELINE identity even when C13/C24/C25 subtype counts are
    zero. Failed rows retain their source provenance and raw receipts but are
    never promoted as passing corpus evidence. Remaining work is authentic
    community-save coverage and source-backed owners for opaque fields.

   - 2026-07-13 update: corpus rows now retain positive C13 EVENT and C13
    timeline-reference receipt availability, and require those plus C24/C25
    availability even when all three subtype counts are zero. An absent
    subtype receipt is therefore an explicit failed external-corpus result,
    never vacuous evidence. Remaining work is authentic community-save
    coverage and source-backed owners for opaque fields.

  - 2026-07-13 update: direct F0435 -> F0433 -> F0435 state identity now
    applies the same fail-closed rule to C13, C24, and C25: any source or
    exported optional row requires its available, byte-preserved receipt.
    Empty rows remain valid only with positive zero-count receipts. Remaining
     work is authentic community-save coverage and source-backed owners for
     opaque fields.

  - 2026-07-13 update: F0802 now round-trips live C48/C49 projectile events
    with original `EVENT.B.Slot`/packed `EVENT.C.Projectile` bytes and updates
    the corresponding exported C14 `EventIndex` without changing the live
    world. Unbound runtime slots fail export rather than becoming synthetic
    square events. Remaining work is broader original dungeon-tail import
    breadth and real community-save corpus evidence.

  - 2026-07-13 update: F0219 now writes a successfully moved live C14
    projectile's decremented kinetic energy and attack back to its decoded
    original record before the next C49 is queued. F0802 continues to own
    rebuilt PC34 event indices because M10's timeline deliberately has no
    raw EVENT-slot allocator. Remaining work is broader original dungeon-tail
    import breadth and real community-save corpus evidence.

  - 2026-07-13 update: F0435 now binds tail-less PC34 saves to the existing
    original start dungeon before materializing EVENTS/TIMELINE. Source
    C29-C41 group reactions resolve `B.Location` through the live raw SFT
    chain, retain `C.Ticks`, and export back to their original event union;
    an absent group remains a failed import rather than a synthetic reaction.
    The same ordering makes C48/C49 resolve against their real C14 records.

  - 2026-07-13 update: C20 delayed sound events now preserve the distinct
    ReDMCSB `EVENT` union (`B.Location`, signed `C.SoundIndex`) through
    native PC34 export/import and F0435 materialization. Remaining save
    breadth is other source union families, never generic Cell/Effect
    substitution.

  - 2026-07-13 update: C22 CPSE events now preserve only their source-owned
    `Map_Time` through F0435 materialization and F0802 export. Firestaff
    consumes the typed receipt through the ReDMCSB `NOCOPYPROTECTION` no-op
    path; priority and B/C union bytes are unowned and therefore canonicalized
    rather than used to invent a fuzzy-sector result.

  - 2026-07-13 update: C25 explosion events now preserve ReDMCSB
    `B.Location` plus `C.Slot` through F0435 materialization and F0802
    export. The C15 Thing must resolve in the original square chain and a
    live F0220 instance must map back to exactly one original C15 record;
    missing or ambiguous references reject rather than becoming synthetic
    Cell/Effect events. Remaining save breadth is other source union
    families, original dungeon-tail import breadth, and real community-save
    corpus evidence.

  - 2026-07-13 update: opt-in external-corpus rows now retain a separate
    C25 receipt for the exact four source-owned union bytes: `B.Location`
    followed by `C.Slot`. The receipt canonicalizes only EVENT storage order
    after F0651, records count/byte length/fingerprint, and requires exact
    source/export identity for every observed C25. It does not infer an
    explosion from `Cell/Effect`, or alter F0435/F0802 runtime behavior.
    Remaining work is authentic community-save coverage and broader
    dungeon-tail import.

  - 2026-07-13 update: opt-in external-corpus rows now retain the separate
    C24 fluxcage `B.Location` plus `C.Slot` union as a four-byte canonical
    receipt. It records count, byte length, and fingerprint after F0651 may
    reorder EVENT storage, but keeps no host explosion-index interpretation.
    Every observed external C24 must retain source/export identity. Remaining
    work is authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: C24 and C25 corpus receipts now share one raw C15
    union serializer for `B.Location` plus `C.Slot`, preventing their
    four-byte receipts from diverging. The helper copies raw EVENT bytes only;
    it neither reads nor emits a host explosion-list index. The existing C15
    square-chain validators remain the only runtime owner. Remaining work is
    authentic community-save coverage and broader dungeon-tail import.

  - 2026-07-13 update: the C24/C25 runtime handoff is now regression-locked
    as one F0435 candidate stage. A valid C25 followed by an authenticated C24
    `C.Slot` on the wrong `B.Location` rejects before publication, preserving
    the destination world, prior report, borrowed start-world explosion list,
    and raw PC34 source bytes. Remaining work is authentic community-save
    coverage and broader dungeon-tail import.

  - 2026-07-13 update: the opt-in corpus now also retains an atomic decrypted
    C3 EVENT-array receipt from the F0433/F0435 part boundary: record count,
    byte count, and fingerprint cover every raw EVENT slot, including C13,
    C24, and C25. The real-corpus probe requires full C3 identity in addition
    to the canonical subtype receipts. Remaining work is authentic community
    save coverage and broader dungeon-tail import.
    Remaining work is broader original dungeon-tail import breadth and real
    community-save corpus evidence.

  - 2026-07-13 update: F0435 dungeon-tail preflight now also enforces the
    F0434 map raw-data span boundary before publishing a tail receipt. A
    checksum-correct descriptor that reaches outside the saved raw-map block
    fails closed, matching the later materializer. The opt-in regression uses
    only the local original `DUNGEON.DAT` through the production F0802 export
    path; it skips without user data. Remaining work is broader original
    dungeon-tail import breadth and real community-save corpus evidence.

  - 2026-07-13 update: accepted F0433/F0435 dungeon tails now publish a
    stable full-tail provenance fingerprint only after size, map-span, and
    checksum validation. The real-data regression proves repeated production
    exports of the same local original dungeon retain that identity; rejected
    tails publish no accepted receipt.

  - 2026-07-13 update: F0435 runtime materialization now rejects a candidate
    save when its live source `EVENTS`/`TIMELINE` heap contains an event
    family without an M10 materializer. It never resumes by silently dropping
    an original event; remaining work is source-complete event-family breadth
    plus real community-save corpus round-trip evidence.

  - 2026-07-14 update: source-shaped C11 `ENABLE_CHAMPION_ACTION` records now
    have focused F0435 runtime coverage: all three valid Priority/SlotOrdinal
    forms survive the C3/C4 handoff and emit only `EMIT_ACTION_ENABLED` on
    their saved ticks, without a dungeon, action, or follow-up being invented.
    An unknown active C80 still rejects transactionally. This is source-bound
    host-runtime evidence, not an original PC34 trace; remaining work is
    authentic community-save corpus coverage.

  - 2026-07-14 update: F0435 candidate materialization now rejects a restored
    PartyMapIndex/X/Y outside its checksum-qualified, materialized dungeon
    tail before world/report commit. The focused local-original-data regression
    rechecks F0417/F0430 checksums for each invalid map, X, and Y mutation and
    proves no destination state is published. Remaining work is authentic
    community-save corpus coverage.

  - DONE 2026-07-14: F0435 now materializes saved C05/C06/C08/C09/C10
    square-state records through M10's existing ReDMCSB `TIMELINE.C`
    F0242/F0244/F0245/F0250/F0251 consumer. It retains the source
    `B.Location` and `C.Cell/Effect` union and leaves source-square validation
    to that existing dispatch owner, rather than substituting a host event.
    The focused authenticated-PC34 regression
    covers all five records and their F0433/F0435 round trips. Remaining work
    is authentic community-save corpus coverage and other source event unions.

  - DONE 2026-07-14: F0435 C02 door-destruction materialization now requires
    the saved `B.Location` to name a loaded door square with source Priority
    zero, matching PROJEXPL.C F0232's only C02 producer before TIMELINE.C
    F0243 consumes Location. M10 emits the destruction receipt only after it
    changes that door. The focused authenticated-PC34 regression preserves
    C02 Location through F0433/F0435, rejects a corridor target, and proves
    the due event reaches C5_DOOR_STATE_DESTROYED. Remaining work is authentic
    community-save corpus coverage and other source event unions.

  - DONE 2026-07-14: immutable original-save imports now use the same F0435
    candidate-world materializer as file imports. Tail-less byte imports borrow
    the live original DM1 dungeon/Thing backing before C3/C4 processing, so
    M11 cannot adopt a save that lost its dungeon. The focused regression proves
    backing retention and atomic rejection without that source backing.
    Remaining work is authentic community-save corpus coverage and other
    source event unions.

  - 2026-07-13 update: saved C040 HoC panel state now reopens only for the
    last appended party slot, the same candidate position consumed by
    `REVIVE.C` F0282. A stale sidecar cannot bind a mirror panel to an older
    champion after F0435 resume; broader original-save and HoC runtime proof
    remains open.

## DM1 F0248/F0213 timeline ownership follow-up

- 2026-08-06 source-audio audit: authenticated DM1 event calls no longer use
  the procedural marker path when SND3 data is unavailable. The remaining
  audio scope is real original capture/corpus breadth, not replacement audio.

- [ ] Capture broader original PC34 launcher and impact sequences. F0248 now
  publishes C007/C008/C009/C010/C014/C015 launcher projectiles only through
  a reserved raw C14 and its C48/C49 owner; loaded worlds reject an exhausted,
  malformed, or absent C14 pool. Loaded impact explosions likewise require a
  reserved C15 plus live C25 receipt. Remaining work is original corpus/capture
  breadth, not an in-memory projectile or explosion fallback.

## DM1 F0134/F0135 production material follow-up

- [ ] Capture the full original DM1 panel-draw sequence around F0345/F0344 on
  an operator GRAPHICS.DAT corpus. The production C020/C030/C031 consumer now
  requires the authenticated LIGHT0 palette, pixel receipts, and exact
  C101/C500/C501 placement; C038 status-border selection uses its live 67x29
  source destination. Remaining work is external capture comparison only.

  - 2026-07-30: CLI `--game dm1 --save <original DMSAVE.DAT>` now reaches the
    already validated M12/M11 resume path, making this capture reproducible
    without synthetic save fixtures. The full F0344/F0345 visual sequence is
    still required before this item can close.

  - 2026-07-30 runtime proof: the fixture-free external PC34 save test now
    opens C017, sends the real C545 mouth click at `(64,54)`, and verifies the
    resulting F0345 C020/C030/C031 food-water pixels. This closes neither an
    original-frame comparison nor the broader F0344 interaction sweep.

  - 2026-07-31 capture recheck: the original PC3.4 `DM.EXE` selector ran in
    DOSBox Staging and reached the automation route, but the current macOS
    `CGWindowListCreateImage` permission context returned all-black frames.
    Those images are rejected as invalid evidence. Earlier operator-local
    DOSBox rawshots are valid dungeon-movement evidence only; they do not
    cover the F0344/F0345 panel sequence. A raw DOSBox capture or an allowed
    macOS window-capture context remains required.

  - 2026-08-06 README refresh: three new Firestaff DM1 v1 frames were captured
    from the real PC34 data and operator save route. They are public runtime
    documentation only and deliberately do not close this original-vs-Firestaff
    panel capture requirement.

  - 2026-07-31 screencapture recheck: the same route reported `Claude` as
    the active macOS app after every reactivation attempt, while its complete
    `320x200` image was black and rawshot recovery remained unavailable. The
    captured abort receipt is operator-local under
    `/tmp/dm1-screencapture-route.fDed0F`; do not retry until a capture
    context can make DOSBox frontmost and expose real pixels.

  - 2026-07-31 direct-window recheck: Quartz found the live `DM.EXE` DOSBox
    window (`1024x796`), but `screencapture -l <window-id>` returned `could
    not create image from window`. This confirms the fault is a host capture
    entitlement/context issue, not the PC34 selector sequence.

  - 2026-07-31 receipt-integrity repair: M11 now rejects an SDL `dummy`
    window as macOS/release-app capture evidence. Headless runs retain their
    internal presented-frame receipt for deterministic tests, but cannot
    satisfy any Mac-window or release-app readiness gate. This prevents an
    internal framebuffer from standing in for the still-required external
    original-frame comparison.

  - 2026-07-31 DOSBox-X Resume input recheck: the capture runner can now
    mount an operator-owned `DMSAVE.DAT` as PC34 `A:` and records a visual
    checkpoint after the documented option-4 `Alt`+numeric-keypad cursor
    route. The real `C409_ZONE_ENTRANCE_RESUME` attempt remained on the
    entrance wall for both System Events and `cliclick` keypad event paths;
    no F0433/panel capture was promoted. This is a host DOSBox-X event-ingest
    blocker, not a missing save, path, or Firestaff save-runtime fallback.

  - 2026-08-09 capture-route correction: the failed option-4 runs above are
    retained as rejected evidence. The live resume plan now selects the
    source-documented PC34 mouse mode (option 1) and dispatches the real
    `C409_ZONE_ENTRANCE_RESUME` click. A new operator-owned raw capture at
    `/Volumes/Extern-disk/Documents/Firestaff/dm1-original-resume-c13-mouse1.v7hhJs`
    reaches stable `dungeon_gameplay` frames and records the subsequent
    forward-click plus keypad fallback. This closes the capture harness's
    entrance-menu stall; it does not prove C13, and its save remains excluded
    from the C13 corpus until the decoded file contains a C13 event.

## DM1 C13 F0435 stale-fence follow-up

- [ ] Run the fixture-free PC34 C13 corpus target with operator-owned saves
  covering changed timeline and active-group states. The runtime fence now
  revokes stale C13 identity before presentation; generated saves are not
  accepted as positive evidence.

  - 2026-08-06 runtime-fixture correction: the focused C13/M11 regression now
    uses a live admitted champion (`hp.current == hp.maximum`), so its party-
    death gate no longer masks the source C13 progression. The test passes;
    this does not close the real-data C13 corpus requirement.

  - 2026-07-30 diagnostic update: the opt-in real-corpus probe now emits the
    per-file C3 record/byte count, C4 timeline index/byte count, C13 event
    count/bytes, staged event/C13 counts, and active-group count. A passing
    corpus result therefore cannot be mistaken for C13 coverage when its
    original file contains zero C13 events.

  - 2026-07-31 re-verification: the operator-owned DOSBox `DMSAVE.DAT`
    (`80a50d66`, 48,561 bytes) passed the fixture-free F0435 -> F0433 ->
    F0435 corpus route with 467 C3 records and 467 C4 indices, but reports
    zero C13 events and zero active groups. It remains valid original-save
    interop evidence only; it does not close the C13 or changed-layout corpus
    requirements below.

  - 2026-07-31 corpus expansion: a third operator-supplied DOSBox PC3.4
    save (`5bcee58c`, 48,561 bytes) is provenance-attested and passes the
    backing-aware F0435 -> native quicksave -> F0433 -> F0435 route against
    the installed original `DUNGEON.DAT`. Like the earlier first save, it has
    zero C13 and active-group entries, so it adds real interop coverage but
    does not close the C13 requirement.
  - 2026-08-08 scope decision: intentionally skipped for this pass. HoC is
    the Hall of Champions startup/presentation path, not a source of C13
    records; C13 remains the dungeon-timeline Vi Altar rebirth event. The two
    PC34 `DMSAVE.DAT` files found in `~/Downloads` are duplicates of the
    existing operator saves (`26ccd159…` and `ab7bb4a3…`) and both report
    `c13_events=0`. Do not synthesize or promote a replacement. Reopen this
    gate only when an operator-owned C13-bearing PC34 save is available.
## DM1 C03/C04 runtime identity follow-up

- [ ] Run the fixture-free PC34 corpus target with operator-owned saves that
  contain different C03 timer/event and C04 heap layouts. Stage/adopt now
  preserve their raw identities; generated saves remain invalid as positive
  corpus evidence.

  - 2026-08-09 verification: the two provenance-attested saves in
    `/Users/bosse/Downloads` were run through the backing-aware production
    corpus target against the real PC34 `DUNGEON.DAT`. The pair passes
    transactionally; the second save supplies 15 active groups and 15 live
    C03 records while the first supplies the empty-layout comparison. This
    closes the current local C03/C04 corpus gap. Broader independently
    collected layouts remain open.

  - 2026-08-09 external-disk audit: three additional provenance-attested
    `DMSAVE.DAT` files under `/Volumes/Extern-disk/Documents/Firestaff/
    dm1-resume-*` are byte-identical and pass the same backing-aware
    round-trip. Their directory names include `c13`, but the decoded files
    contain zero C13 events; they are therefore C03/C04 evidence only and do
    not close the C13 requirement.

  - 2026-08-09 runtime verification: the external-disk save from
    `dm1-resume-c13-diskette.Y0dbXx` passes a real headless M11 resume with
    `--platform auto`, restores map 1 / party `(6,2,2)` at tick 1674 and
    reaches `dm1-runtime` without the entrance-menu stall. The remaining
    save gap is specifically an authenticated C13-bearing file, not the
    generic resume handoff.

  - 2026-07-31 DOSBox-X corpus follow-up: a second operator-owned PC34
    DMSAVE.DAT (48,561 bytes) restored map 1, party (6,2,2), tick 1664,
    15 active groups, and 15 live C03 records (types C32, C37 and C38),
    versus the first corpus save's map 0, party (4,11,2), tick 1487, and
    empty C03/C04 live set. Authentic ACTIVE_GROUP.GroupThingIndex values
    are raw GROUP-table indexes, not always type-4 packed THING values; the
    shared F0435/F0145/F0146/F0147/F0196 handoff now admits both original
    raw and legacy encoded forms while resolving only the loaded GROUP table.
    V1/V2.0/V2.1/V2.2 --game dm1 --save boot probes passed against the
    second save. The standalone corpus preflight remains open because this
    authentic save has no dungeon tail; it must be staged against its real
    DUNGEON.DAT before F0435 -> F0433 -> F0435 can certify the full route.

  - 2026-07-31 backed-runtime update: both operator-owned DOSBox saves now
    a live C03/C04 save must never be certified without its matching dungeon
    backing. Reference: ReDMCSB `LOADSAVE.C` F0433/F0435 and DMweb
    saved-game file-format documentation.

  - 2026-07-31 corpus expansion: all three provenance-attested DOSBox saves
    now pass the backing-aware corpus target. The additional save has the
    same empty C03/C04 runtime layout as the first one, so the existing
    non-empty DOSBox-X save remains the sole current real-data proof for
    active-group and live C03 coverage.

  - 2026-07-31 test-route update: legacy self-contained corpus probes now
    explicitly skip a provenance-attested save whose runtime stage is not
    self-contained. The backing-aware M11 corpus target is mandatory for
    that media and passes both current DOSBox saves. The split prevents a
    valid original save from being mislabeled as malformed while retaining
    the older no-backing proof for saves that own their dungeon tail.

  - 2026-07-31 startup-census update: when an operator explicitly configures
    `FIRESTAFF_DM1_PC34_SAVE_CORPUS` or
    `FIRESTAFF_DM1_ORIGINAL_SAVE_DIR`, startup now publishes its classified
    PC34 corpus facts even if the unbacked F0435 preflight cannot certify a
    live save. The receipt therefore distinguishes discovered envelopes from
    successful roundtrips and never treats a resume-parent directory or a
    synthetic fixture as external corpus evidence. The actual M11 selected-
    save path remains the backing-aware authority.
  - 2026-07-31 local recheck: the two operator-owned `Downloads/DMSAVE*.DAT`
    files completed `dm1_v1_original_save_pc34_backed_corpus_roundtrip` when
    explicitly bound to the installed original `DUNGEON.DAT`. This confirms
    the backing-aware route for the currently available real saves only; it
    does not replace the required broader, provenance-attested corpus.

## ReDMCSB DM1 Reference Boundaries (2026-07-13)

These are reference limits, not claims that ReDMCSB logic is wrong. Each item
marks a place where a Firestaff PC34 claim needs evidence in addition to the
ReDMCSB WIP 2021-02-06 source tree.

- REDMCSB-DM1-GAP-001 — **ReDMCSB `Documentation/Readme.htm`, scope and
  terminology sections.** The project states that it is reverse engineered,
  not FTL's original source, and that names are reconstructed from binaries
  and secondary material. **Firestaff risk:** a plausible identifier or C
  expression can be mistaken for a proven PC34 ABI, byte layout, or side
  effect. **Required independent evidence:** a hash-identified original PC
  3.4 executable/disassembly and a minimal runtime trace for every ABI or
  save-layout assertion that depends on reconstructed naming.

- REDMCSB-DM1-GAP-002 — **ReDMCSB `Documentation/Readme.htm`, Accuracy /
  Atari ST.** The supplied Megamax 1.1 rebuilds are deliberately only
  near-identical to FTL's Atari binaries and fail some copy-protection checks;
  the original compiler/linker is unavailable. **Firestaff risk:** source
  control-flow is not binary proof for checksums, address-sensitive code, or
  instruction timing. **Required independent evidence:** the original PC34
  executable plus DOS/emulator execution trace; do not use a ReDMCSB rebuild
  as a checksum or timing oracle.

- REDMCSB-DM1-GAP-003 — **ReDMCSB `Documentation/Engine.htm:10` and
  `Documentation/Readme.htm`, Accuracy / Other platforms.** The engine page
  is explicitly written from Atari ST source, while the other-platform
  accuracy section is marked "TO BE COMPLETED". **Firestaff risk:** an ST
  render, palette, disk, or input branch can be silently promoted to PC34.
  **Required independent evidence:** PC 3.4 English media, executable
  disassembly, and frame/input captures; cross-platform branches are
  explanatory only until those agree.

- REDMCSB-DM1-GAP-004 — **ReDMCSB `DUNVIEW.C` F0115 setup, around line 2464
  original-PC custom-dungeon capture remains the blocker for the legacy
  memory-limit behavior.
  the documented platform-memory gap: an original-PC custom-dungeon capture is

- REDMCSB-DM1-GAP-005 — **ReDMCSB `CHAMPION.C` F0306/F0319/F0320/F0321.**
  Firestaff now locks the PC34 F0306 compiler-order branch and M11 owns the
  F0319 one-shot death record: inventory/bones are not duplicated across host
  ticks or an original-save reload that already has its bones record, poison
  lifecycle records clear with the champion, and the champion direction plus
  C026 portrait record remain source-backed. M11 creature melee now publishes
  the actual F0320 C015/C016 champion-damage receipt after its F0321 result.
  **Still required independent evidence:** PC34 executable capture covering
  nonlethal F0321 damage, death, save/reload, bones pickup, and resurrection,
  with recorded portrait/HUD frames and save bytes.

- REDMCSB-DM1-GAP-006 — **ReDMCSB `DUNGEON.C` thing allocation, around line
  2099 (`BUG0_10`).** The reserved champion-bones type uses bit 15 and the
  legacy compiler's shift happens to discard it before indexing; the source
  documents that a normal compiler can index out of bounds. **Firestaff risk:**
  source-shaped C alone cannot define the PC34 allocation semantics for dead
  champion bones. **Required independent evidence:** real PC34 saves and
  runtime traces covering party death, bone allocation, pickup, save, load,
  and export byte comparison.

- REDMCSB-DM1-GAP-007 — **ReDMCSB `LOADSAVE.C` load branch around
  lines 2860-2895.** The code documents a broken historical DM/CSB dungeon
  detector and format/header changes that make older saves impossible to load.
  **Firestaff risk:** ReDMCSB's broad multi-platform loader is not a complete
  PC34 interchange specification, especially for damaged, backup, and
  version-mismatched files. **Required independent evidence:** provenance
  recorded PC34 save corpus spanning new game, HoC selection, deaths/bones,
  active groups, pending events, backup, and rejected/corrupt files, with
  original-load and byte round-trip results.

  - 2026-07-13 update: external-PC34 corpus discovery and per-file receipts
    now retain the exact F7057 five-part envelope endpoint and the untouched
    trailing-byte count that F0435 must consume as portraits/dungeon tail.
    This distinguishes a valid envelope with a later corrupt suffix from a
    malformed F7057 body without decoding or promoting tail bytes. Remaining
    evidence is a provenance-recorded real PC34 corpus across the listed
    gameplay states and original executable load results.

  - 2026-07-13 update: corpus certification now validates the populated
    receipt itself: header/part shape, F7057 boundary, atomic C3 EVENT bytes,
    raw C4 TIMELINE bytes, and the optional F0433 dungeon tail. C13/C24/C25
    are subtype receipts only when source rows exist, so an absent optional
    subtype cannot block independent C3/C4/tail evidence.

  - 2026-07-13 update: PC34 import now rejects any active C3 EVENT omitted
    from C4 before runtime materialization. The focused C13 regression keeps
    a valid rebirth EVENT but substitutes a different active C4 index, proving
    exact rollback provenance instead of silently losing the timer. Remaining
    evidence is still a provenance-recorded real PC34 corpus and original
    executable load results.

  - 2026-07-13 update: corpus discovery and F0435 import now bind to the
    same reclassified byte snapshot. A DMSAVE.DAT replaced after recursive
    discovery is rejected with a diagnostic instead of inheriting stale
    header/envelope provenance. This is transaction hardening only; it does
    not replace the required original PC34 corpus.

  - 2026-07-14 update: `dm1_v1_original_save_pc34_external_corpus` is a
    fixture-free admission target for an explicitly staged corpus. It reports
    each admitted file's source/export hashes, F7057 envelope boundary,
    trailing-tail size, and no-fallback runtime-stage/adoption results; an
    unset corpus root is a non-promoting skip. Remaining evidence is still
    provenance-recorded original PC34 saves and original executable results.

  - 2026-07-14 update: fixture-free external-corpus promotion now requires
    every qualified PC34 candidate to stage an owned F0435 dungeon and adopt
    its EVENTS/TIMELINE queue before it is called runtime-admitted. A
    tail-less byte roundtrip remains diagnostic-only; it cannot borrow a host
    dungeon or satisfy the corpus runtime gate.

  - 2026-07-14 update: the external HoC runtime gate now requires the live
    M11 runtime to retain the `ORIGINAL_SAVE_PC34` viewport origin and emit a
    nonblank, byte-stable 224x136 PC34 viewport crop. HUD chrome cannot
    satisfy this rendering receipt. This records Firestaff consumption of an
    admitted save's live dungeon state; it remains neither a DOS pixel-parity
    claim nor a replacement for provenance-recorded original executable runs.

  - 2026-07-14 update: the same fixture-free HoC runtime gate now requires
    M11's canonical post-adoption world hash to match an independently staged
    F0435 world from the identical external save snapshot. This binds the
    live handoff to its restored party, active state, timeline, and
    dungeon-backed runtime rather than only its pose or viewport. It remains
    host-runtime evidence, not original-PC execution or pixel-parity proof.

  - 2026-07-13 update: F0435 tail validation now verifies the persisted
    per-column cumulative SquareFirstThings table against raw-map thing-list
    flags before M10 can reconstruct its lookup. The remaining requirement is
    still provenance-recorded original PC34 saves and original executable
    results, not a different tail format.

- REDMCSB-DM1-GAP-008 — **ReDMCSB `LOADSAVE.C`
  F1057/F0433/F1059 and `COMMAND.C` save-command checksum gates.** Save
  control flow is wrapped by platform-specific checksum/copy-protection
  helpers such as F0464; the ReDMCSB accuracy note says non-identical rebuilt
  Atari binaries have incorrect checksum values. **Firestaff risk:** source
  call order does not prove original PC34 save UI acceptance, protection, or
  failure presentation. **Required independent evidence:** original PC34
  executable and save-media corpus with recorded save command, produced bytes,
  reload result, and dialog/frame capture.

- REDMCSB-DM1-GAP-009 — **ReDMCSB `MEMORY.C` graphic loading calls to
  F0497_LZW_Decompress and `Documentation/Readme.htm`, graphics compression
  and caching.** The source explains the codec and cache policy, but carries
  no Firestaff-owned proof that a particular PC34 `GRAPHICS.DAT` bitmap,
  palette, title, or sound entry has the assumed identity. **Firestaff risk:**
  valid decoding can still select the wrong asset, palette, frame, or cached
  representation. **Required independent evidence:** hash-identified PC34
  `GRAPHICS.DAT` plus asset-offset/decoded-pixel corpus and original frame
  captures for title, entrance, HoC, HUD, inscriptions, and dungeon cells.

- REDMCSB-DM1-GAP-010 — **ReDMCSB `GRF1.C`, `IBMIO.C`, `IO.C`
  S0075/S0076, and `SOUND.C` F0061.** Low-level graphics, input interrupts,
  mouse state, and sound routes include platform assembly/system calls or
  platform-specific tables. **Firestaff risk:** the high-level source cannot
  prescribe SDL event coalescing, audio scheduling, palette latch timing, or
  host frame presentation. **Required independent evidence:** PC34 DOS/emulator
  input/audio/frame capture with tick markers, then Mac/SDL packaged-app
  comparison; no synthetic timing or sound substitute may be promoted.

  - 2026-07-14 IBMIO.C F8099/F8100/F8111/F8112 now have a source-locked
    PC34 state adapter for lock depth, cursor coordinates and formatted button
    state. Host interrupt scheduling, SDL event coalescing and cursor drawing
    timing remain outside this narrow implementation and still require the
    recorded PC34 evidence above.

  - 2026-07-14 IBMIO.C F8101/F8108/F8109 now preserve handler registration,
    72-byte pointer-slot registration and visible-pointer transition ordering.
    The DOS interrupt, deferred mouse history and host video drawing remain
    callback boundaries and still require the recorded PC34 evidence above.

  - 2026-07-14 IBMIO.C F8123/F8124 now preserve the empty CD-track route and
    the device-specific raw sound-progress query. F8128 remains unavailable:
    its hardware/ISR polling has no portable progress contract without the
    PC34 timing evidence required above.

  - 2026-07-14 IBMIO.C F8129/F8130 now preserve the defined device-type table
    and three-attempt first-sector probe through host I/O callbacks. The DOS
    IOCTL/BIOS transports and source-undefined device types outside 0–7 remain
    external evidence boundaries.

  - 2026-07-14 IBMIO.C F8131/F8132/F8133 now preserve FAT-label filtering,
    DOS-time `DX` seed packing and the empty floppy route. DOS FCB/DTA lookup
    and clock provenance remain host callback boundaries requiring PC34 proof.

  - 2026-07-14 IBMIO.C F8134 now preserves the defined DOS EXEC command-tail
    prefix and normal-termination result. Program loading and DOS exit status
    remain callback boundaries requiring PC34 proof.

  - 2026-07-14 NEC816.C F8137 now preserves the MEDIA457_P20JA 4bpp packed
    nibble fill, including odd/even start and bounded write behavior. The
    MEDIA472 byte-per-pixel path and the live framebuffer consumer remain
    separate, unproven work.

  - 2026-07-14 NEC816.C F8140/F8162 now preserve the PC 3.4 source-defined
    overlap-copy direction and 160-byte-stride multi-plane message-area
    transfer. Binding those logical planes to the live Mac/SDL framebuffer
    remains a separate presentation/capture task; no hardware aperture is
    claimed by this narrow adapter.

  - 2026-07-14 VIDEODRV.C/NEC816.C F8151 now preserves the PC 3.4 C25
    source-bitmap-to-aperture rectangle path, including source/destination
    even strides, transparency and vertical flip. C25 F0681/F0683 are empty,
    so horizontal flip deliberately remains a source-defined no-op. The F8143
    C25 aperture-to-bitmap primitive is now separately ported; live framebuffer
    presentation remains unproven.

  - 2026-07-14 VIDEODRV.C F8152 now preserves the PC 3.4 C25 inclusive
    rectangle fill through F8137, including fixed 320-byte rows and the
    viewport color-index offset. The live SDL/Mac aperture binding remains
    separate.

  - 2026-07-14 VIDEODRV.C F8143 now preserves the PC 3.4 C25 aperture-to-
    packed-bitmap readback with low-nibble extraction and retained opposite
    boundary nibbles. It is a standalone aperture primitive until the live
    host framebuffer supplies the C25 surface.

  - 2026-07-14 VIDEODRV.C F8154 now preserves the PC 3.4 C25 inclusive
    aperture inversion rectangle, XORing exactly `0x04` at `(y * 320) + x`.
    Its live Mac/SDL aperture consumer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8155 now preserves the PC 3.4 C25 hatch box:
    `((x ^ y) & 1) == 0` aperture pixels clear to zero and the others are
    retained. Live Mac/SDL aperture consumption remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8167/F8168 now preserve the C25 mouse-pointer
    background lifecycle: a real aperture snapshot clamped to 18x18 and the
    screen edge, then direct F8166 restoration. Live mouse compositing and
    host framebuffer binding remain separately unproven.

  - 2026-07-14 VIDEODRV.C F8169 now preserves the C25 blackening animation's
    real 16-bit LFSR order, including its explicit final write to pixel zero
    and `G8177 | black` aperture value. The live host framebuffer consumer
    and frame pacing remain separately unproven.

  - 2026-07-14 VIDEODRV.C F8163 now preserves the C25 caller-bitmap transfer:
    the original packed 4bpp source indices and destination aperture indices
    flow unchanged into F0680. Binding that aperture to the live SDL/Mac
    framebuffer remains separately unproven.

   - 2026-07-14 VIDEODRV.C F8213 now preserves C25's direct one-byte aperture
    write: `G8177 | color` goes to the requested `G8134` pixel index. Binding
    this logical aperture to live SDL/Mac presentation remains separately
    unproven.

  - 2026-07-14 VIDEODRV.C F8153 now preserves C25's two-phase 0x3DA vertical
    blank poll: leave an active blank, then wait for the following one. The
    live SDL/Mac VBlank-status provider and frame presentation remain
    separately unproven.

  - 2026-07-14 VIDEODRV.C F8139 now preserves C25's direct packed-4bpp
    source-to-aperture loop, including source parity and bytewise G8177 OR.
    Binding the logical aperture to live SDL/Mac presentation remains
    separately unproven.

  - 2026-07-14 VIDEODRV.C F8137 now has its C25 VGA aperture-fill variant:
    direct `G8177 | color` bytes over the real 320x200-compatible aperture.
    Its runtime call-site consumption and live capture remain separately
    unproven.

  - 2026-07-14 IMAGE3.C F0684 now preserves the PC 3.4 C25 packed-bitmap
    source-to-aperture route for all four flip modes and transparency. It
    consumes the direct 320x200 host-compatible aperture; live game call-site
    capture remains separately unproven.

  - 2026-07-14 DUNVIEW.C F0675 now preserves native/derived/temporary bitmap selection before real F0129 scaling; M11 call-site binding remains unproven.

  - 2026-07-14 VIDEODRV.C F8216 now preserves C25's forward aperture copy
    from one 320-byte row above, including its source-visible propagation for
    spans larger than a row. Binding this logical aperture effect to live
    SDL/Mac rendering remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8230 now preserves the C25 single-colour palette
    mutation: real RGB4 components become `(component << 2) + 3` RGB6 bytes,
    then publish through F8156 only while the curtain is active. The live
    SDL/Mac DAC consumer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8166 now consumes the real C25 F8165 aperture
    snapshot format directly: three 16-bit prefix words and raw 320-stride
    rows are replayed without palette conversion. Binding the host aperture to
    the live SDL/Mac framebuffer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8165 now preserves C25's real aperture snapshot
    layout: a 6-byte partial-box prefix followed by raw byte-per-pixel rows at
    the original 320-byte stride. The capture source is still an explicit host
    aperture; live SDL/Mac framebuffer binding remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8156/F8157 now preserve the PC 3.4 C25 RGB6
    palette bytes: F8157 updates only terminated table entries whose index is
    below 32, and F8156 publishes all 32 rows only after a host VBlank gate
    when the curtain is active. This does not fabricate a VBlank or a palette:
    M11/SDL consumption of the verified DAC rows remains separate work.

  - 2026-07-14 VIDEODRV.C F8159 now preserves the PC 3.4 C25 RGB6 curtain:
    black waits at the caller-owned VBlank boundary and writes 32 zero rows;
    normal restores verified source RGB6 bytes through F8156. SDL/Mac DAC
    consumption remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8160 now preserves the PC 3.4 C25 creature palette
    mutation: six existing palette-table rows receive only the RGB6 bytes from
    one real `G8175_CREAT_PAL` set. Publishing that altered logical palette to
    the live SDL/Mac DAC remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8161 now preserves the PC 3.4 C25 viewport
    source-to-aperture consumption: it applies the original `0x10` RGB bank
    only to an opaque, unflipped `224 -> 320` F8151 blit. Binding this logical
    aperture to the live SDL/Mac framebuffer remains separately unproven.

  - 2026-07-14 VIDEODRV.C F8158 is not a PC 3.4 C25 task: its source body is
    guarded to EGA/Tandy builds only, so no C25 behavior is invented.

  - 2026-07-14 DM1 V1 spell HUD now routes only through CASTER.C/MENUDRAW.C
    C009/C011/C013 with real GRAPHICS.DAT and original-font gates. Palette
    capture parity remains subject to the PC34 evidence requirement above.

- REDMCSB-DM1-GAP-011 — **ReDMCSB `GAMELOOP.C` lines 171-181 and `IO.C`
  mouse interrupt path.** The source records a platform/version race fix for
  eye/mouth press state between interrupt and command-queue processing.
  **Firestaff risk:** polling SDL input can look correct while diverging on
  press/release ordering, dialogs, chest panels, or save-menu entry. **Required
  independent evidence:** original PC34 input traces for click/hold/release
  around eye, mouth, chest, scroll, resurrection and save commands, plus the
  same scripted sequence through the packaged host app.

- REDMCSB-DM1-GAP-012 — **ReDMCSB `ENTRANCE.C`
  F0438_STARTEND_OpenEntranceDoors, `TITLE.C` F0437, and platform headers.**
  The common source supplies control flow but timing, bitmap presentation,
  palette behavior, and sound backend are selected by platform conditionals.
  **Firestaff risk:** copying the sequence can produce a visually plausible
  but wrong PC34 title/swoosh/entrance cadence or palette. **Required
  independent evidence:** frame-numbered, audio-synchronised PC34 startup
  captures and raw title/animation asset corpus, compared against the packaged
  Firestaff frame stream.

- REDMCSB-DM1-GAP-013 — **ReDMCSB's documented bugs are observations, not a
  modern-engine policy.** `Documentation/BugsAndChanges.htm` records original
  behaviors such as object cloning, timeline exhaustion, and malformed custom
  dungeon crashes, but does not establish whether a safe host should emulate,
  contain, or reject each failure. **Firestaff risk:** either erasing a
  source-visible PC34 behavior or reproducing memory corruption under C11.
  **Required independent evidence:** a PC34 reproduction for the claimed
  release and an explicit per-route emulate/guard/reject decision backed by a
  regression; without it, keep the input bounded and mark the route
  unavailable rather than synthesising a result.

- REDMCSB-DM1-GAP-014 — **Copy-protection and physical-media behavior is not
  a portable game contract.** `CopyProtection.htm`, `GRAPH21.C`, and platform
  disk/I/O paths expose fuzzy-sector and protection control flow, but do not
  provide an authentic PC disk signal, DOS driver timing, or a normalised
  failure contract for a modern filesystem. **Firestaff risk:** a fabricated
  success/failure result can alter startup, free memory, event scheduling, or
  endgame presentation. **Required independent evidence:** an archived
  original PC media image with emulator/real-machine trace; otherwise the
  protection branch remains explicitly unavailable and cannot be replaced by
  synthetic state.

- 2026-07-13 CSBWin restored `TT_FALSEWALL` follow-up: authenticated SET now
  and DSA-free CLEAR/TOGGLE now update the original falsewall cell flag.
  Portrait/DSA-owned squares and parameter-message payloads remain fail-closed.
  CLEAR and the open-wall TOGGLE branch defer only where the saved timer's
  party/nonmaterial-group owner can be retained.

- 2026-07-13 CSBWin restored `TT_24` follow-up: exact saved-object removal
  and free are live only for a validated source Thing chain. Timer-owned
  clouds, source sound/party effects, malformed chains, and all broader
  object-lifetime routes remain fail-closed until their CSBWin owners exist.

- 2026-07-13 CSBWin restored `TT_13` follow-up: the exact final Vi Altar life
  stage and the old-save no-EXPOOL DB10-bones state-1 handoff are live. The
  `packedState` 2 cloud and Wings state-1 branches remain fail-closed until
  those original owners are retained by the imported runtime record. The
  current-save `EDT_ChampionBones` EXPOOL state-1 branch is complete and
  tracked in DONE.

- 2026-07-13 CSBWin restored `TT_53` follow-up: live runtime now retains the
  exact saved watchdog TIMER/queue owner while requeueing its source `+300`
  level-zero successor. Broader watchdog diagnostics remain blocked without
  their complete CSBWin runtime state.

- 2026-07-13 CSBWin restored `TT_65` follow-up: the live queue now restores
  only the saved generator's exact `timerObj8` actuator identity, retaining
  CSBWin's documented first-disabled-actuator fallback for old saves. Broader
  C65 sensor execution and generator materialization remain blocked unless
  their complete CSBWin timer ownership is preserved.

- 2026-07-13 CSBWin restored `TT_75` follow-up: exact saved 8-bit poison
attacks now enter the existing source C75 damage and `+36` requeue chain.
2026-07-14 update: the live event-slot receipt now retains the complete
16-bit `timerWord6` through every `+36` continuation, clearing only when its
exact event is consumed. Source panel redraws and candidate-champion UI
effects remain blocked without a restored HUD owner.

- 2026-07-13 CSBWin restored `TT_78`/`TT_79` follow-up: Fire Shield and
  Magic Footprints now consume exact saved timer queue/event identities and
  update only their authenticated character-tail counters. Portrait redraw,
  footprint cleanup, and visual footprint material stay blocked without their
  corresponding CSBWin runtime/HUD owners.

- 2026-07-13 CSBWin restored `TT_77` follow-up: live dispatch now expires an
  exact saved Spell Shield receipt only for a positive, non-underflowing
  signed defense delta. The source all-portrait redraw remains blocked without
  a restored M11 HUD owner.

- 2026-07-13 CSBWin restored `TT_74` follow-up: live dispatch now expires an
  exact saved Party Shield receipt only for a positive, non-underflowing
  signed defense delta. The source all-portrait redraw remains blocked without
  a restored M11 HUD owner; timer families needing omitted object identity
  remain blocked pending a source-backed timer-record expansion.

- 2026-07-13 CSBWin restored `TT_73` follow-up: live dispatch now expires an
  exact saved Thieves' Eye receipt only while its imported party count is
  positive. Timer families requiring unpreserved object-word identity remain
  blocked pending a source-backed timer-record expansion.

- 2026-07-13 CSBWin restored `TT_72` follow-up: live dispatch now applies an
  exact saved champion-shield expiry only when its imported champion and
  unsigned defense delta are coherent. Underflowing records and the source
  status-panel redraw remain blocked; timers requiring omitted `timerWord8`
  object identity remain blocked as well.

- 2026-07-13 CSBWin restored `TT_71` follow-up: live dispatch now expires an
  exact saved invisibility receipt only while its imported party count is
  positive. The source inventory/status redraw has no restored UI owner, and
  timer families requiring omitted `timerWord8` object identity remain blocked.

- 2026-07-13 CSBWin restored `TT_1` timer follow-up: collision-free original
  door stepping and saved-owner requeue are live. Party damage, material-group
  damage/reaction, source sound data, malformed Thing chains, and nonterminal
  collision ownership remain blocked until their complete saved runtime state
  is source-backed.

- 2026-07-13 DM2 viewport source-material follow-up: source-required wall and
  door passes now reject a decoded GDAT image when its own IMG3 local-palette
  receipt is absent. Extend that same per-image ownership rule to remaining
  map-chip consumers only with proven skproject lookup and palette evidence;
  do not borrow `INTERFACE_GENERAL` colors or fabricate a fallback plane.

  - 2026-07-13 update: direct G1 DB2 Text and DB3 Actuator `WALL_GFX` routes
    now carry `dtImage/1` metadata plus the matching
    `QUERY_GDAT_IMAGE_LOCALPAL` receipt into `DRAW_DEFAULT_DOOR_BUTTON`.
    The source-required viewport fetches that exact IMG3 before comparing its
    palette hash and blocks any absent/mismatched ornate/button. The lookup
    now also consumes skproject's real `MISCELLANEOUS/FE/FE` GDAT default
    palette when an otherwise valid source image lacks a four-bit tail. This covers
    only the proven field-1 button route; broader ornate placement and other
    fields remain unavailable rather than inferred.

- 2026-07-13 DM2 viewport material follow-up: source-required creature,
  floor-object, projectile, carried-item, possession, and CHAMPIONS portrait
  GDAT drawing, plus T600 outdoor sky and ground planes, now require decoded
  IMG3 pixels and their exact local-palette receipts. Weather now verifies its
  GRAPHICSSET environment IMG3 address and local palette and carries its
  receipt into live frame ownership, but remains no-draw until skproject proves
  the destination clip; do not borrow interface colors or synthesize
  replacement art.

  - 2026-07-13 update: PC G1 DB4 creature map-chip receipts now bind the
    exact `CREATURES/type/F9` local palette alongside the decoded image. The
    viewport rejects a palette-hash mismatch; this does not infer animation,
    clipping, or a new draw route.

  - 2026-07-13 update: weather ENVIRONMENT commands now additionally require
    a bounded decoded IMG3 pixel receipt that matches their metadata and local
    palette. The decoded material remains no-draw until its complete source
    `QUERY_TEMP_PICST`/`DRAW_TEMP_PICST` execution route is consumed.

  - 2026-07-13 update: the DM2 runtime now carries the verified indoor
    floor/ceiling required and consumed masks into its M11 handoff receipt.
    A source-required indoor frame is invalid unless both GRAPHICSSET planes
    completed their renderer-owned material transactions; an incomplete plane
    is not presented through a substitute surface.

  - 2026-07-13 update: source-required `DM2_DRAW_DOOR` now prebinds every
    visible panel, ornament/destroyed-mask, frame, and button IMG3 together
    with its local palette before the first door blit. A missing component
    blocks the complete door pass instead of leaving partial or fallback door
    pixels. Remaining door work is exact source placement/clipping breadth.

  - 2026-07-13 update: source-required skproject T600 now prebinds both
    active `GRAPHICSSET` sky and ground IMG3s with their own local palettes
    before either outdoor scene plane is drawn. Weather stays no-draw because
    its real `DRAW_TEMP_PICST` image/destination route remains unproven.

- 2026-07-13 CSBWin saved-DSA parameter-message follow-up: the bounded
  `TT_ParameterMessage` runtime path now owns authenticated EXPOOL payloads
  through the 26-word stack ABI for source stone/open-room dispatch. Larger
  source records, non-DSA timer effects, master-state persistence, and DSA
  world/filter opcodes remain intentionally unavailable until each has an
  independently source-owned runtime surface; do not truncate parameters or
  route a timer by inferred room state.

- 2026-07-13 CSBWin saved-DSA LocalState follow-up: normal saved queue
  entries now execute source `LocalState=2` only when compact DB3 `ParameterB`
  has no widened high bits. Widened ParameterB values, slave-master routing,
  master-state writes, timer cell effects, and all world/filter opcodes remain
  blocked pending complete source-owned runtime records; do not coerce a
  compact actuator into a widened state value.

- 2026-07-13 CSBWin saved-DSA tick follow-up: restored timer queue entries
  remain save-owned after the tick. It still skips without an explicit original
  package and does not create a substitute save, DSA, or timer fixture.
  inventory/status-panel redraw branches remain blocked until their live

- 2026-07-13 CSB F0282 probe follow-up: keep the repaired probe-local C040
  candidate-panel receipt fail-closed as additional real-save variants are
  staged. Do not reintroduce the removed M11 diagnostic export or admit any
  state beyond source-proven F0280/F0282 panel facts.

- 2026-07-13 DM1 F0407 action-enable receipts: `THROW` and the real
  action-hand `SWING` route now complete through their F0330 C11 owners.
  Other F0407 action families still require separate source-owned live
  receipt work; do not generalize C11 scheduling from a UI cooldown or add a
  fallback timer. In particular, a delayed SWING C11 must remain locked until
  the authentic receipt reaches F0253. C11 receipts must retain their
  original ordinal and be rejected once their live owner is consumed.

- 🔧 2026-07-13 Nexus Structure3 follow-up: documented `0x800`-byte block
  requires an unavailable original Saturn capture and continues to block
  normal-plane, transform, texture/palette, and draw behavior.
  semantics remain unproved. The documented entry-local
  1,144 entries / 18,478 pairs. Next remains original Saturn evidence for

  - 2026-07-14 update: Structure1A byte 0 now reaches the DGN handoff and
    render-plan receipts only through complete Structure1F owner relations.
    Its raw reuse is counted, but its grammar remains unassigned; it cannot
    select a face, model, transform, mesh, material, or draw route.

  - 2026-07-14 update: the renderer-facing DGN plan now retains the same
    bounded Structure3 texture-selector and face/normal ordinal receipts as
    the handoff, including the complete retail LEV00--LEV15 selector joins.
    This remains no-draw provenance: original Saturn capture/executable
    evidence must still establish payload/palette decode, transforms, and
    VDP1 ordering before any mesh command can render.

  - 2026-07-14 update: those bounded face selectors now retain per-level
    unique/reused occurrence accounting for both Structure2 and Structure1G
    joins. This records raw source selector reuse only; it does not assign
    payload contents, texture dimensions, UVs, palette semantics, animation,
    transforms, or a draw route.

  - 2026-07-14 update: the hash-verified LEV00--LEV15 corpus now exercises
    the Structure3 capture binder for every retail level with source-only
    input. All 16 remain explicitly blocked before candidate framing, source
    binding, or renderer handoff. This is a no-draw regression guard, not
    Saturn capture evidence; texture/palette decoding, transforms, and VDP1
    draw ordering still require an authenticated original executable trace or
    capture containing every bound span.

  - 2026-07-14 update: the bounded Structure3 grammar now has a caller-owned,
    source-hash-checked typed entry extractor for documented signed-16.16
    vertex/normal rows and entry-local face rows. It rejects partial buffers
    and mutated payloads, retains raw byte 9 without assigning it a role, and
    never grants transform, palette, texture, VDP1, or draw semantics. The
    retail LEV00--LEV15 corpus extracts every 1,144 entries while preserving
    the 18,478 face/normal totals. Next remains original Saturn evidence for
    payload/palette decoding and VDP1 ordering, not fallback visuals.

  - 2026-07-14 update: hash-verified retail Structure1A/Structure1F records
    now bind their documented Structure3 model and face selectors to one
    bounded entry-local face row and its same-ordinal normal row. Any
    out-of-range model or face selector rejects the complete attachment
    receipt. This does not establish placement, transform, normal-plane use,
    texture/palette behavior, culling, VDP1 state, or a draw route. The next
    boundary remains original Saturn execution/capture evidence for those
    behaviors; do not promote the attachment receipt into rendering.

  - 2026-07-17 update: the parser now exposes source-bound opaque 12-byte
    ordinal rows for all three counted Structure3 entry regions, plus
    face-indexed first-region rows, source-order face-index sets,
    same-ordinal third-region rows, and raw face prefix/tail slices. Each
    admission rechecks the direct DGN identity, entry/region FNV, and row
    bounds; it grants no coordinate, topology, normal, material, texture,
    geometry, or draw semantics. The next concrete intake must be a separately
    evidenced raw relation, while original Saturn evidence remains required
    before any geometry or renderer promotion.

  - 2026-07-17 update: a 0x21-tagged Structure1F wall-decoration record can
    now join its already-admitted raw selector byte to one admitted Structure3
    second-region row ordinal. The join rechecks direct identity, package and
    retained record/entry/region/row FNV witnesses and rejects selector or
    offset drift. It is an opaque equality witness only, not a face, owner,
    topology, geometry, material, texture, placement, or draw relation.

  - 2026-07-17 update: the equivalent 0x20 Structure1F alcove selector path
    is now admitted against the same bounded Structure3 second-region row
    ordinal. Its independent record FNV, source tag, selector byte, direct
    identity, entry/region/row witnesses, and offset all fail closed. This is
    not portal, face, owner, topology, geometry, material, texture, placement,
    or draw evidence; floor-decoration and floor-sensor payload bytes remain
    unlinked until a separate source-backed relation exists.

  - 2026-07-17 update: Structure1F directory evidence now gives each 0x11
    floor-decoration and 0x12 floor-sensor opaque payload tail a strict source
    owner: family, tag, record ordinal/span/FNV, payload span/FNV, and direct
    package identity. Other families reject, and the receipt cannot grant a
    Structure3 relation or object, sensor, placement, geometry, material,
    texture, or draw semantics. A future relation requires separate source
    evidence rather than payload-byte inference.

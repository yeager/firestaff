# Firestaff TODO - DM2

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Active Cycle 16 Jobs (DM2 only — continuous operation)

Per directive: DM2 only, auto mode. Lanes pull jobs from this file; the
orchestrator keeps them fed, assembles, and pushes. Fix synthetic paths when
real game data is available; batch small jobs into larger ones. Source-lock
against skproject (SKULLWIN/SKWIN); keep fail-closed where evidence is
missing. Do not push — the orchestrator pushes after assembly. Update this
file and DONE.md after every completed job.

- **Lane A — DM2 SkWinCore symbol audit batch 16 (cycle 16):** Done — see
  "Recently Completed" below.

- **Lane A — DM2 SkWinCore symbol audit batch 17 (cycle 16):** Done — see
  "Recently Completed" below.

- **Lane A — DM2 SkWinCore symbol audit batch 18:** Done (v3.0.182). Ported
  16 symbols from `SKULLWIN/c_1c9a.cpp`: 14 fully ported, 2 fail-closed stubs.
  Source-locked helpers in `src/dm2/dm2_v1_skproject_core.c`, declarations in
  `include/dm2_v1_skproject_core.h`, focused regression tests in
  `tests/test_dm2_v1_skproject_core.c`, and audit updates in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` plus
  `SYMBOL_DISPOSITIONS.tsv`.

- **DM2 legacy SKProject sound-model reachability audit (2026-08-07):** Done.
  The caller-authored regression model has no M10/M11 call site; the
  production-boundary verifier now rejects one if added. Real sound remains
  limited to verified GDAT/DYN4, SDL playback and native FM Towns CDDA media.

- **DM2 selected-corpus i18n verification (2026-08-07):** Done. The real-data
  test now requires the selected PC-DOS `GRAPHICS.DAT` when configured and
  verifies every extracted GDAT text key through its lookup route; stale
  home-directory paths and success-after-skip output are removed.

- **Lane C — DM2 real-data startup/dungeon gate repair (cycle 16):** Done
  (v3.0.181). Fixed 5 of 7 target tests: boot_profile_smoke,
  startup_audio_menu, dungeon_loader_first_map_gate, c_map_tile_access,
  asset. Refined DM2-001 g1_w0_chains_disabled flag so synthetic skproject
  fixtures preserve w0 chain traversal. Enabled record_graph_complete for
  skproject-loaded fixtures. DM2 failures: 2 (was 13). Remaining:
  m11_startup_profile_gate (30 deep runtime failures, pre-existing) and
  utility_import (timeout, pre-existing).

- **Lane D — DM2-010 creature/cloud passes (cycle 16):** Done (see
  "Cycle 16 Completed Lanes" below).

## Cycle 16 Completed (DM2 only — lanes report here; orchestrator pushes)

- **Lane E — DM2 real-data combat and drops mechanics (cycle 16):** Done
  2026-07-23; committed on `cycle16-lane-E`, not pushed.
  - Drops: new `dm2_v1_drops_resolve_gdat_creature_drops()` reads the eleven
    CREATURES drop words (fields 0x0A..0x14) straight from a verified
    GRAPHICS.DAT loader and resolves them in source order
    (skcrture.cpp:2092-2100 DROP_CREATURE_POSSESSION).  Proven against the
    local canonical GDAT: GLOP/24 drops items 284/314 (words 0x8E10/0x9D10),
    ATTACK MINION/14 drops items 8/264 (0x0410/0x8412), TREE/0 drops item
    292 (0x9241); the live death path
    (`dm2_v1_creature_load_ai_table_from_gdat` → `dm2_v1_creature_death_check`)
    reports the real item through the observer with replica-exact RNG draws.
    A creature without imported GDAT drop words now produces no generated
    loot, exactly as `DROP_CREATURE_POSSESSION` does for eleven zero words.
  - Combat: melee/ranged resolution gained the real-data defense route —
    `dm2_v1_combat_bind_creature_defense_fn()` (caller-owned provider hook,
    same pattern as the CAII word providers) and
    `dm2_v1_combat_resolve_attack_on_creature()` resolve the source damage
    formula against the creature's AIDefinition Defense byte @8
    (c_engage.cpp via c_record.cpp:1351-1354), with kill threshold
    (damage >= hp) receipted.  New data-backed accessor
    `dm2_v1_creature_ai_defense()` mirrors `dm2_v1_creature_ai_base_hp`.
    Fail-closed: without a provider, or when the session did not prove the
    creature's defense — the local PC English GDAT has no CREATURE_AI
    (0x19) category, so the route rejects explicitly locally instead of
    inventing a defense value.
  - Tests: `test_dm2_v1_combat_pc34_compat` 49 → 56 checks (provider-bound
    resolution, kill threshold, undestroyable Defense=255, invalid-weapon
    rejection, out-of-range zero damage, both fail-closed gates); new
    `test_dm2_v1_drops_gdat_real_data` (explicit-corpus; proven drop words, RNG
    replica, death-observer chain, fail-closed defense); creature/combat
    probe 158 → 166 assertions (fail-closed gates for all three modules).
  Verify: `ctest --test-dir build -R 'dm2_v1_(combat|drops|creature_death_drop|creature_combat_probe)'`
  7/7 PASS incl. `dm2_v1_combat_probe` and `dm2_v1_creature_death_drop_probe`;
  strict `-Wall -Wextra -Werror` clean on all touched files.
  Remaining: a CREATURE_AI-proven graphics session to light up the
  defense/BaseHP route locally, DUNGEON.DAT door-record evidence for the
  door-destruction table, and ALLOC_NEW_DBITEM item-record creation for
  admitted drop slots.

- **Lane B — DM2-008 audible playback backend (cycle 16):** Done 2026-07-23;
  committed on `cycle16-lane-B`, not pushed.  Voice allocation, PCM decode,
  and a real SDL3 playback backend now sit behind the existing fail-closed
  contract in `src/dm2/dm2_v1_sound.c`:
  - `dm2_v1_sound_decode_gdat_pcm()` decodes the unsigned 8-bit mono PCM of a
    verified `GRAPHICS.DAT` sound raw entry (payload = raw bytes after the
    two-byte format header, converted `byte ^ 0x80`, exactly the SKWin
    `0x80 + raw_byte` alloc-time conversion; playback rate 6000 Hz per
    SKWIN/SkwinSDL.cpp).  Verified against real data: 292 loadable SOUND
    entries locally; entry 3/0/129 decodes byte-for-byte.
  - Voice allocation owns MAX_SB = 16 voice slots (SKWin `MAX_SB`); voices
    free when the backend reports playback complete, no stealing, the 17th
    simultaneous request is explicitly rejected (`rejected_no_free_voice`).
  - `dm2_v1_sound_bind_playback_backend()` binds an SDL-free vtable;
    `src/dm2/dm2_v1_sound_sdl_backend.c` (+ header) is the concrete SDL3
    backend (6000 Hz U8 mono stream, additive sdlAudMix-shaped mixing,
    per-voice volume).  `dm2_v1_sound_play_gdat_entry()` /
    `_positional()` start audible playback only when the sample decodes from
    a verified GDAT entry AND the backend reports ready; attenuation is the
    source-locked R_928 metric (c_sound.cpp:256-308), never synthesized.
    `dm2_v1_sound_play()` / `dm2_v1_sound_play_positional()` now resolve
    the sound_id as the GDAT raw sample binding (xsndptr2 `w_00`) and stay
    fail-closed (-1) without loader, backend, or a matching SOUND entry.
  - Title music stays fail-closed: no verified music asset root is proven
    locally (no `SKWIN/data/*.hmp.mid` present; the DOS zip only ships
    `test.hmp`, which is not the title cue).
  - Tests: `test_dm2_v1_sound_gdat_real_data` gained PCM-decode and
    no-backend rejection checks; new `test_dm2_v1_sound_playback_sdl`
    (SDL_AUDIODRIVER=dummy) proves audible playback start, voice completion,
    legacy sound_id routing, R_928 positional attenuation, 16-voice
    exhaustion, and stop_all reuse against real GRAPHICS.DAT;
    `test_dm2_v1_sound_source_gate` gained fail-closed decode/playback
    checks.  `firestaff_dm2_v1_creature_combat_probe` unchanged (158/158).
    `firestaff_dm2` now links SDL3::SDL3 publicly (glob picked up the new
    backend source).
  Verify: `ctest --test-dir build -R dm2_v1_sound` 4/4 PASS;
  `firestaff_dm2_v1_creature_combat_probe` 158/158 PASS.
  **2026-07-31 update:** M11 now describes and binds the SDL backend only
  after `dm2_v1_boot_startup_launch_alloc()` admits the verified boot
  profile, then unbinds it during DM2 shutdown. The real-data M11 startup
  gate proves that lifecycle. Remaining: bind a verified music asset root
  when `*.hmp.mid` assets are proven, and prove wall-occlusion/facing routing
  before positional cues leave the queue.

- **Lane E (next) — DM2 combat follow-ups (cycle 16/17):** bind a verified
  music asset root; ALLOC_NEW_DBITEM drop-slot materialization (c_dballoc);
  DUNGEON.DAT door-record evidence for the door-destruction table; a
  CREATURE_AI-proven graphics session to light the defense/BaseHP route locally.

## Cycle 15 Completed (DM2 only — both lanes pushed)

- **Lane A — DM2 SkWinCore symbol audit batch (cycle 15):** Done 2026-07-23;
  pushed. Backlog 891 → 883 `MISSING`; `c_querydb.cpp` fully drained.

- **Lane B — DM2-010 static-object pixel draw (cycle 15):** Done.
  Source-locked the remaining DRAW_ITEM floor-object chain against skproject
  SKWIN/SkWinCore.cpp (DRAW_ITEM _32cb_3672, DRAW_PUT_DOWN_ITEM _32cb_3991,
  DRAW_STATIC_OBJECT _32cb_3b9d, QUERY_GDAT_ENTRY_DATA_INDEX /
  QUERY_TEMP_PICST) and DME.h ExtendedPicture w28/w30:
  - `dtImageOffset` is now source-owned: the boot selectors read it at the
    default item index 0xFE exactly like DRAW_ITEM (tt == 0), and a
    proven-absent entry binds offset 0 (QUERY_GDAT_ENTRY_DATA_INDEX returns 0
    for an absent fmtPicOff entry) instead of blocking the object.  The
    per-type offset entries are inventory-icon material the floor route never
    consumes.  The offset flows selector -> viewport sprite -> render row ->
    blit (signed high byte to x, low byte to y).
  - The expanded-clip receipt (raw4 rect) plus raw GDAT image receipt and
    local palette now travel from the admitted static-object material onto
    the viewport sprite (`dm2_runtime_admit_static_object_draw_item_material`),
    and `dm2_runtime_bind_g1_scene_item_material` binds the decoded F0/F4
    image through `dm2_v1_viewport_set_g1_scene_static_item_material_direct`
    with matching raw identities, so admitted static objects leave `no_draw`
    and actually blit in `dm2_v1_render_items` where the GDAT evidence is
    complete (canonically: the map-26 WEAPONS/0/F0 record; WEAPONS/126 and
    palette-less records stay fail-closed).
  - Per-square chain-slot ordinals are proven rather than assumed: the G1
    materializers only admit tile chain heads (square-first-thing), and
    DRAW_PUT_DOWN_ITEM draws the head of a matching direction group first, so
    draw_slot 0 and record_list_ordinal 1 are bound with that evidence
    (replacing the synthetic global (i+1) ordinal).
  - Side/deep cells 1..15 are admitted (glbTabYAxisDistance, _4976_418e rows
    0..3 and the 16-cell display-order table prove their placement); cell 0
    (no table1d7029 pass) and D4 cells 16+ (DRAW_PUT_DOWN_ITEM distance
    guard) stay fail-closed.  The chest mirror rule now follows the source
    (x-distance 1 always mirrors, x-distance 0 mirrors right-column anchors).
  - Fixed a cycle-13 source-lock bug: the draw-slot deltas were assigned to
    the wrong axes — DRAW_ITEM adds _4976_41de[_4976_41b0[vv][0]] to the x
    anchor (offx -> ExtendedPicture.w28) and [vv][1] to the y anchor (w30).
  - Tests: `test_dm2_v1_draw_item_source_placement` 106/106 (was 78/78),
    `test_dm2_v1_g1_static_object_visibility_real_data` 39/39 with the real
    pixel-draw chain, `test_dm2_v1_g1_weapon_viewport_material_gate` 9/9,
    `test_dm2_v1_viewport_door_state_side_cells` 25/25; new probe
    `probes/dm2/firestaff_dm2_v1_static_object_pixel_probe.c` (11/0, 1 record
    admitted, 3 fail-closed on the canonical corpus); updated probes
    `firestaff_dm2_v1_draw_item_source_probe` (10/10) and
    `firestaff_dm2_v1_draw_item_source_pass_probe` (135/0).
  - Verify: `ctest --test-dir build -R dm2_v1` 227/237; the 10 failures are
    byte-identical to the fully rebuilt cycle-15 baseline (pre-existing).
  Remaining: only the first admitted static object binds scene material per
    frame (single g1_scene_item_material slot, same bounded pattern as the
    G1 creature route); M11 delivery plans keep `no_draw` for the host handoff;
    creature/cloud passes keep their existing map-chip routes.

## Skproject Audit (DM2)

- **SKPROJECT-DM2-FUNCTION-COVERAGE-2026-08-06:** The earlier informal
  “31 missing functions” count is stale. The current named-symbol audit
  (`docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`) records
  `DM2_SOUND1` through `DM2_SOUND7`, the applicable `c_move.cpp` paths, and
  all source-owned `c_map.cpp` paths as `IMPLEMENTED_PARITY` (1,118 total
  symbols). The remaining `c_dialog.cpp` and `c_eventqueue.cpp` entries are
  explicitly `NOT_APPLICABLE_ARCH`: they are DOS UI/event-loop owners
  replaced by M11, not callable game-data substitutes. Keep auditing the
  real M11/GDAT path for missing ownership; do not revive any retired
  callback transcript merely to reduce a function-count metric. **2026-08-06
  eventqueue correction:** the retained test transcript now matches the
  source `0x02`/`0x04` capacity edge, keyboard seven-entry cap and init-vs-
  flush sentinel split; it remains excluded from every production path.

- SKPROJECT-DM2-STARTUP-001 — `SKWIN/SkWinCore.cpp::SHOW_MENU_SCREEN`
  (`TITLE/0 dt07/4`): Firestaff now treats the menu as one static GDAT draw
  command owned by DM2 startup presentation; `TITLE/0 dt07/1` is retained as
  title/credit query receipt evidence, not a second host menu draw or
  synthetic overlay. Verification is now executable: the current
  `test_dm2_v1_m11_startup_profile_gate` passes against the hash-verified
  PC-DOS data and `firestaff --game dm2 --boot-probe` reaches the active
  `dm2-startup-menu` phase. An installed v3.0.288 app was inspected only as
  a stale external comparison, not as evidence for this v3.0.290 build.
- SKPROJECT-GAP-001 — `SKULLWIN/c_weather.cpp::DM2_SET_TIMER_WEATHER` and
  `DM2_UPDATE_WEATHER` identify scheduling but not a serialised timer-record
  layout or save offset. Risk: Firestaff could bind a random SKSave region as
  weather state. A per-file original-save timer-format receipt now preserves
  only verified candidate type/path/size/hash and explicitly rejects each
  unowned envelope/raw payload. `DM2_SET_TIMER_WEATHER` and
  `DM2_weather_3df7_0037` now have source-mapped runtime receipts for the
  outdoor 182-tick scheduling and reseed/weather transaction, but the saved
  timer-record owner/byte layout is still not proven. Required: original
  timer/save trace and corpus with known weather transitions that identify
  record owner and byte layout.
  2026-07-18 update: the generic saved timer-record byte layout half is now
  source-proven by the verified `v1d6463 = vsgame+0x00` mask
  (dm2data.cpp:97-99) over `c_tim` (c_timer.h:8-46) and materialised by
  `dm2_v1_save_timers_pc34_compat` (DM2-009). Still open: the weather-timer
  record OWNER (which saved record is the weather timer) and corpus traces
  with known weather transitions.
- SKPROJECT-GAP-002 — `SKWIN/DME.h::DistantEnvironment` fixes the ten-byte
  in-memory shape but not allocation owner, persistence location, or save
  encoding. Risk: ENVIRONMENT material could pair with stale slot bytes.
  Required: DOS memory/save snapshots across weather updates.
- SKPROJECT-GAP-003 — `SKULLWIN/c_sound.cpp` retains TODOs around MIDI calls,
  sample-state returns, and queue fields. Risk: voice lifetime/music semantics
  can diverge. Required: original executable trace and sound corpus.
- SKPROJECT-GAP-004 — `SKULLWIN/c_map.cpp` marks map globals and ground-stack
  table meanings unresolved. Risk: over-promoted G1 record/tile ownership.
  A raw-only G1 receipt now preserves verified column-index, ground-stack,
  and trailing map-data bounds, counts, and hashes from hash-verified corpus.
  A second raw-only receipt correlates every verified `Map_definitions` row
  to its bounded trailing-map span and hash, without assigning tile meaning.
  Both receipts intentionally leave the table and tile semantics absent.
  Direct DB0 and DB3 root receipts now read only their independently defined
  payload words after runtime admission; DB3 extension records and every
  `GenericRecord::w0` route remain unread and untraversed.
  Required: multi-map original DUNGEON.DAT corpus plus debugger traces that
  define `v1e03f4`, `dunGroundStacks`, and bit `0x10` beyond their observed
  indexing contract.
- SKPROJECT-GAP-005 — `SKWIN/DME.h` labels CCM `0x32..0x34` unknown. Risk:
  fabricated creature behaviour. The corpus receipt now hashes only verified
  `CREATURE_AI/row/dt00` AIDefinition rows, rejects all adjacent fields, and
  records no 0x32..0x34 stream bytes. Required: original opcode streams and
  instruction-level traces that bind a file/save owner and grammar.
- SKPROJECT-GAP-006 — `SKWIN/SkWinCore.h::_44c8_0f29` is unresolved blitting.
  Risk: local-palette clipping/mirroring differs despite decoded GDAT pixels.
  Required: original framebuffer captures and DOS blitter trace.
- SKPROJECT-GAP-007 — **The named-symbol inventory has no verified behavior
  mappings yet.** `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv`
  records 1,751 skproject callable definitions: 142 exact-name candidates,
  1,540 missing names, 69 desktop-variant exclusions, and zero implementation
  claims. Risk: promoting a literal identifier collision as a DM2 port.
  Required: per-family call-path evidence, owned input/GDAT/save data, and a
  focused Firestaff regression before any `UNCERTAIN` row becomes
  `IMPLEMENTED`.
- SKPROJECT-GAP-008 — **Title/menu GAME_LOAD remains a source boundary, not a
  session constructor.** Candidate names found by the inventory do not prove
  `SHOW_MENU_SCREEN` input routing or `GAME_LOAD` data admission. Risk:
  reintroducing a synthetic party or dungeon after NEW GAME. Required:
  skproject input event trace plus a hash-verified original DUNGEON.DAT load
  receipt consumed by M11 before runtime activation.
- SKPROJECT-GAP-009 — **Two skproject source files are presently unreadable
  locally.** `SKULLWIN/c_music_wav.cpp` and `SKULLWIN/c_rect.cpp` are retained
  as explicit audit sentinels rather than guessed symbols. Risk: treating a
  partial source tree as exhaustive coverage. Required: readable local source
  copies followed by a regenerated inventory and reviewed mappings.

- 2026-07-13 CSBWin restored `TT_60`/`TT_61` follow-up: only the exact
  party-square, non-Lord-Chaos `+5` successor is live before M10 can mutate
  `timerObj8`. Object movement, TT_61 sound, occupied-square checks, and the
  Lord Chaos random detour remain fail-closed without their CSBWin owners.
  - 2026-07-14 hardening: every queue-owned saved `TT_60`/`TT_61` receipt is
    now consumed before M10's incompatible C60/C61 group path. Unsupported,
    malformed, off-party-square, and Lord-Chaos shapes create no successor
    and retain no generic movement or sound behavior.
  - 2026-07-15 queue-retirement correction: the source-owned receipt is now
    neutralized only after the common F0239 extraction. Marking the queued
    event as `NONE` before extraction left the original TIMER in the heap and
    could duplicate the authenticated `+5` successor. The source gate remains
    fail-closed for every unsupported receipt.

- 2026-07-13 CSBWin restored `TT_22` follow-up: the imported restart timer's
  exact source no-op is live. Its original creation context and the removed
  historical restart work remain unavailable; do not infer a C22 action.
## DM2 V2.0 Runtime Follow-up (2026-07-13)

M11 now binds the selected DM2 V2 presentation mode to the persistent V2
phase gate and boot-owned, hash-verified GDAT provider. The runtime HUD now
consumes only decoded `INTERFACE_GENERAL` and `CHAMPIONS` pixels, mapping each
IMG3 through its paired boot-owned `dtPalette16` after the source viewport
frame; absent pixels or palettes remain absent. Remaining V2.0
work is real packaged Mac capture and wider GDAT/save corpus coverage, not a
new fallback HUD or menu.

DM2 V1 scene planes now bind and consume each decoded `GRAPHICSSET` IMG3 with
its own `QUERY_GDAT_IMAGE_LOCALPAL` result before the next material fetch.
Ceiling/sky can no longer inherit floor/ground palette entries, and a missing
source outdoor plane records a blocked no-draw receipt rather than silently
accepting an existing framebuffer fill as source-backed. Remaining scene work is the separately
unproven weather `QUERY_TEMP_PICST`/`DRAW_TEMP_PICST` destination path.

DM2 V2.1 now keeps its internal per-surface pipeline in lockstep with M11:
the selected upscaled mode enables source-preserving EPX while retaining the
original palette. Remaining V2.1 work is real packaged capture and broader
GDAT material coverage, not a separate synthetic upscale route.

DM2 V2.2 is fail-closed in M11. A V2.2 request records the finished-art
manifest gate but resolves to V2.1 because the legacy modern cache has no
live real-material renderer. Remaining V2.2 work is a renderer that consumes
a complete, operator-reviewed pack through every active GDAT scene route;
placeholder or partial assets must never be promoted.

DM2 V1 weather CMDSTR receipts now require the matching original
`ENVIRONMENT` `dtImage` in the active graphics set, so `CD/FW` metadata
cannot authorize a draw by itself. The selected IMG3 now has a strict
source-bounds receipt after its real `dtImageOffset` pair, but destination
clipping is still unproven. Remaining weather work is to bind real original
rain/cloud state and final `QUERY_TEMP_PICST`/`DRAW_TEMP_PICST` placement to
that verified image route; do not derive overlays from Firestaff weather
enums or intensity.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- DM2-001 — `skproject/SKULLWIN/c_gdatfile.cpp` GDAT query/load path and `c_loadlevel.cpp` level materialisation: the hash-verified DOS EN/FR shared dungeon member is discovered and materialized through the normal scanner, and its typed GDAT ENT1 payload graph validates. PC G1 parsing bounds the real pre-map extension and exposes the proven `c_map.cpp` route: its 256-byte post-descriptor G1 block precedes the 480-word column-prefix table, which reaches the bounded 2360-word ground-stack table. The source-ordered `c_record.cpp` pool transform and DB3/DB4 continuation addresses are proven.
  - 2026-07-27 update: **Record graph complete.** [v3.0.180] Diagnostic proved that G1 byte-square format stores game data in w0 (not next-links). Validator now validates ground-stack → record resolution (including G1 extension pools for DB3/DB4). `get_thing_record` resolves extension records. `get_next_thing` returns END_MARKER for G1 (no w0 chains in file). `record_graph_complete=1` on real 39,437-byte DUNGEON.DAT. Fixed 3 pre-existing test failures (arrange_dungeon_receipt, first_map_real_data_probe, object_model_probe).
- DM2-002 — `skproject/SKULLWIN/c_dballoc.cpp`, `c_record.cpp`, `c_map.cpp`, and `c_moverec.cpp` database-record ownership: `src/dm2/dm2_v1_world_model.c`, `dm2_v1_world_state.c`, and `dm2_v1_runtime.c` retain reduced Firestaff records, including a stub save-state layout. Replace the parallel model with validated original record pools, links, maps, and relocation semantics.
  - 2026-07-18 update: the world now owns a source-ordered c_record pool
    set (`dm2_v1_record_pool_pc34_compat`) populated from the validated G1
    spans with exact `table_recordsizes`, GET_ADDRESS_OF_RECORD handle
    decode, GET_NEXT_RECORD_LINK, APPEND/CUT list paths, and a
    MOVE_RECORD_TO list-relocation boundary; tile-rooted relocation and
    full cross-map moves remain fail-closed until c_map ground-stack link
    state is proven. CTest `dm2_v1_record_pool_pc34_compat` PASS; dm2_v1
    lane keeps the same 27 known baseline failures with zero new failures.
    Remaining: tile-rooted link state, cross-map relocation, save-state
    relocation, and retiring the remaining parallel record reads.
  - 2026-07-16 update: `DM2_ARRANGE_DUNGEON` now has a DM2-owned
    source-named receipt over the already proven dungeon-loader arrangement.
    It admits only real `DUNGEON.DAT` map descriptors, square layout,
    per-map dimensions, map offsets, `MapGraphicsStyle`, ground-stack/text
    table addresses, and record-graph completion state. The local PC G1 data
    remains explicitly incomplete when the full c_record graph is not proven;
    no record links, objects, or runtime semantics are fabricated. Remaining
    work is the complete `c_record` ownership/relocation contract and runtime
    handoff after the graph is source-proven.
  - 2026-07-16 update: `DM2_PERFORM_MOVE` now has a source-named
    movement-admission plan consumed by `dm2_v1_runtime_move`. It records
    normalized direction, source/target cell, target tile facts, door state,
    cooldown blocking, dungeon impassable square classes, open-door admission,
    and outdoor no-dungeon-tile admission without changing the existing
    trigger/plate/actuator post-step path.
  - 2026-07-22 update (Lane B, cycle 5): fixed a regression where
    `dm2_v1_runtime_move` passed raw DM1/DM2 tile encodings (wall=0, floor=1)
    into the `DM2_PERFORM_MOVE_plan` admission path, which expects
    `DM2_SquareType` enum values (wall=1, floor=0). A new
    `dm2_runtime_normalize_square_type` helper maps raw tile classes to the
    enum before both the local impassability check and the plan request.
    `test_dm2_v1_movement_collision_gate_pc34_compat` now passes 7/7,
    including `runtime_blocked_step_turn_state`. Remaining `c_move.cpp` work
    is still broad: chained object/group/projectile movement, ladders,
    missiles, cross-map relocation, source timing/audio, and full record
    ownership.
  - 2026-07-16 update: `FIND_LADDER_AROUND` now has a DM2-owned
    source-named dungeon receipt over loaded `DUNGEON.DAT` square facts. It
    scans the origin and surrounding eight cells in a bounded source-order,
    accepts only real stairs-up/stairs-down tile candidates, records the
    target cell, raw tile, square type, vertical direction, distance, and
    search hash, and returns an explicit not-found receipt instead of
    fabricating ladder/hole semantics. Remaining work is full vertical
    transport, hole/ladder object semantics, chained movement effects, timing,
    and runtime handoff into the broader `c_move.cpp` path.
  - 2026-07-16 update: `DM2_move_075f_06bd` now has a DM2-owned
    source-named projectile impact attack receipt matching skproject's
    `_075f_06bd` / `PROJECTILE_GET_IMPACT_ATTACK` boundary. It consumes
    missile energy, cloud effect IDs, GDAT throw strength word `0x09`,
    poisonous word `0x0D`, item weight, and injected RNG terms without
    synthetic gameplay data. Remaining projectile work is full missile impact
    integration through object/group/champion runtime effects, timing, audio,
    and record ownership.
  - **2026-08-05 c_move inventory correction:** the former
    `DM2_move_075f_1bc2` target-cell and `DM2_move_2c1d_028c` commit receipts
    were synthetic. In SKProject `c_move.cpp:2861` selects four candidate
    player positions using party state and `DM2_RANDBIT`; `:2914` searches an
    adjacent party member and returns its index or `-1`. Neither routine is
    collision nor movement commit. Both adapters now reject explicitly until
    the real party-position, RNG, and caller state are bound. Keep collision
    in the separately source-scoped runtime route; do not reuse these names
    to admit a DUNGEON.DAT movement result.
  - 2026-07-16 update: `DM2_move_2fcf_0434` now has a DM2-owned
    source-named teleporter transition gate for the `c_move.cpp:2152`
    boundary. It consumes either loaded `DUNGEON.DAT` square facts or explicit
    square facts, requires DB1 `Teleporter`, source byte-square type 5 with
    bit `0x08` enabled, a complete record graph, party scope bit `0x02`, and
    a bounded destination before admission. It records all block reasons and
    remains non-mutating: no `GenericRecord::w0`, DB3/DB4/DB8/DB10 traversal,
    timing, sound, or map switch is fabricated.
- DM2-003 — `skproject/SKULLWIN/c_timer.cpp`, `c_tim_proc.cpp`, `c_events.cpp`, and `c_eventqueue.cpp` timer order: `src/dm2/dm2_v1_timeline.c`, `dm2_v1_runtime.c`, `src/memory/`, and `src/engine/m11_game_view.c` do not execute the original timer-type matrix and still contain an M11 creature-tick simulation. Route every DM2 timer through a DM2-owned source-order dispatcher and remove host-side behavioural substitution.
  - 2026-07-18 update: new DM2-owned source-order dispatcher
    `dm2_v1_proceed_timers_pc34_compat` mirrors `c_tim_proc.cpp:3980-4230`
    DM2_PROCEED_TIMERS: pop-before-map-switch over the c_timer.cpp:31-47
    heap order, the full 26-entry type matrix (0x01, 0x02, 0x04 tile
    subdispatch 0-6, 0x0c, 0x0d, 0x0e, 0x15, 0x19, 0x1e, 0x21/0x22, 0x3d,
    0x46, 0x47, 0x48, 0x4b, 0x54, 0x55, 0x56, 0x58, 0x59, 0x5a, 0x5b,
    0x5c, 0x5d, 0x5e), unknown types skipped via the source's `continue`,
    and known types without a bound DM2-owned handler acknowledged
    fail-closed, never simulated. The unconditional host creature-tick
    simulation in `dm2_v1_runtime_tick` is removed: creature state now
    advances only through dispatched 0x21/0x22 DM2_THINK_CREATURE timers
    via the DM2-owned runtime queue (`dm2_v1_runtime_enqueue_source_timer`,
    `dm2_v1_runtime_last_proceed_timers_receipt`). CTest
    `dm2_v1_proceed_timers_pc34_compat` PASS (matrix membership, heap
    order, not-due boundary, unknown skip, fail-closed, handler rejection,
    0x04 tile subdispatch). dm2_v1 lane 191 tests, same 27 known baseline
    failures, zero new failures. Remaining: bind proven timer producers
    (doors, missiles, weather, creature scheduling) to the queue, per-cell
    DM2_THINK_CREATURE binding after DM2-005 record ownership, and removal
    of the DM1-generic M11 creature-group pass for DM2 sessions.
  - 2026-07-19 update: the weather timer producer is now bound to the
    DM2-owned source queue. `dm2_v1_runtime_tick` enqueues a type-0x54
    c_tim (actor 0, mticks = gametick + delay) through
    `dm2_v1_runtime_enqueue_source_timer`, mirroring
    skproject/SKULLWIN/c_weather.cpp:20-30 DM2_SET_TIMER_WEATHER; the
    182-tick cadence stays owned by the existing
    DM2_SET_TIMER_WEATHER receipt, each pop is acknowledged fail-closed
    by the dispatcher (no 0x54 handler bound yet), and the producer
    re-schedules the next cycle after the pop. The host weather
    transition path remains the transition owner until
    DM2_UPDATE_WEATHER (c_weather.cpp:33+) is bound. New CTest
    `dm2_v1_weather_timer_producer_pc34_compat` PASS (enqueue on first
    outdoor tick, not-due before the boundary, pop at the 182-tick
    boundary, re-schedule after pop, indoor never enqueues, host
    transition path unchanged). dm2_v1 lane 198 tests, same 27 known
    baseline failures, zero new failures. Remaining: door/missile
    producers (c_tim_proc.cpp / c_move.cpp DM2_QUEUE_TIMER sites) and
    the 0x54 DM2_UPDATE_WEATHER handler binding.
  - 2026-07-19 update: per-cell DM2_THINK_CREATURE is now bound over the
    DM2-002 record pool. New module `dm2_v1_think_creature_pc34_compat`
    mirrors c_querydb.cpp:1486-1507 DM2_GET_CREATURE_AT (tile record
    link via `dm2_v1_dungeon_get_first_thing`, bounded next-link walk,
    first DB4 record, direction bits preserved) and the
    c_tim_proc.cpp:4079-4088 0x21/0x22 payload decode (x = getxA,
    y = getyA, type word, timer map) as a DM2-owned dispatcher handler;
    the c_ai.cpp:5670-5673 no-creature early return consumes the timer
    without simulating, and the think body stays fail-closed behind an
    explicit DM2_V1_ThinkCreatureBody boundary until the CCM stream
    owner/grammar is proven. New CTest
    `dm2_v1_think_creature_pc34_compat` PASS. dm2_v1 lane 199 tests,
    same 27 known baseline failures, zero new failures. Remaining:
    session-owned record pool set in the runtime so the live 0x21/0x22
    dispatch resolves per-cell, and the creature-scheduling producer.
  - 2026-07-19 update: the 0x54 DM2_UPDATE_WEATHER handler is now bound
    as a bounded slice. New module `dm2_v1_update_weather_pc34_compat`
    mirrors the c_tim_proc.cpp:4179-4183 dispatch into
    DM2_UPDATE_WEATHER(1) and the arg==1 branch of
    c_weather.cpp:33-90: the zone weather-flag read
    (table1d6b76[4*v1e1472 + 0x70], table bound verbatim from
    dm2data.cpp:889-896), the byte-arithmetic ++v1e147b retry with the
    >0x1f forced transition (DM2_weather_3df7_0037(0) stays host-owned,
    receipt-flagged, no requeue), the previous-intensity snapshot, the
    intensity step v1e1474 += (u8)v1e1484 * (i8)v1d7108[(row<<5)+retry]
    with clamp 0..0xff, and the requeue delay RAND16(256)+50 (50..305)
    on the shared DM2_V1_DropRng LCG binding (c_random.cpp:13-31). The
    128-byte v1d7108 pattern table is bound verbatim from the extracted
    v1d7108.dat (loaded by the source via DM2_READ_BINARY,
    dm2data.cpp:1371). Zone/pattern-row out of bounds is fail-closed
    with no mutation. New CTest `dm2_v1_update_weather_pc34_compat`
    PASS (normal step with reference-LCG cross-check of the requeue
    draw, forced transition without requeue/RNG advance, clamp at 0 and
    0xff, fail-closed bounds, retry byte wrap, requeue delay bounds).
    dm2_v1 lane 201 tests, same 27 known baseline failures, zero new
    failures. Remaining: runtime wiring of the 0x54 dispatch to this
    handler — the runtime producer still owns its fixed 182-tick
    cadence while the source re-queues RAND16(256)+50 inside the
    handler, and v1e1478/v1e1484 (pattern row/step) need map-load
    provenance — plus the DM2_weather_3df7_0037 transition owner and
    the arg==0 day-rollover branch (c_weather.cpp:91+).
  - 2026-07-19 update: the 0x54 weather chain is now fully wired in the
    runtime and the synthetic 182-tick cadence is retired. New
    `dm2_v1_weather_transition` binds DM2_weather_3df7_0037
    (c_weather.cpp:509-567): the arg==0 full transition (host-flagged
    DM2_UPDATE_GLOB_VAR light request, day_tick = gametick + 0x555,
    normal reseed delay RAND16(8000)+500 with pattern_row = RANDDIR and
    step = RAND16(3)+1, the v1d7188 storm-forced branch with delay
    RAND16(500), row 3, step 1 and rain-counter clear, the common reset
    of cloud/lightning/intensity/retry plus wind_dir = RANDDIR and the
    source re-queue), the arg!=0 keep-current branch (previous cleared,
    step floored to 1, no requeue), and the common tail
    (cloud_timer = RAND16(4)+4, day_word = table1d70f0[hour] from the
    dm2data.cpp:182-191 table bound verbatim, days/hour from
    (gametick+v1e1438)/0x555, v1d7188 cleared). The transition's
    RAND16 draws use the source's CUTX16-then-modulo semantics via the
    new raw dm2_v1_drops_rand24 accessor (dm2_v1_drops_rand16 modulo on
    the full 24-bit draw is only identical for moduli dividing 2^16).
    In `dm2_v1_runtime_tick` the retired producer block is replaced by
    the self-perpetuating source chain: outdoor sessions start the
    chain with the session-seeded transition (arg=0, mirroring the
    c_savegame.cpp:546 session-start call — the v1d652d arg-selecting
    flag is unproven, bounded choice documented), the 0x54 dispatch is
    bound to dm2_runtime_update_weather_timer, which steps the
    session-owned v1e14xx state through dm2_v1_update_weather_1,
    re-queues the source delay, and runs the bound transition when the
    handler forces one. The presentation weather intensity is derived
    from v1e1474 (bounded 0..255 -> 0..100); the weather enum stays a
    host presentation selector. `dm2_v1_weather_timer_producer_pc34_compat`
    and `dm2_v1_weather_seed_regression` are rewritten for the source
    chain (reference-LCG-derived delays, reseeded state, per-pop
    intensity steps, indoor never starts, deterministic restart);
    `dm2_v1_update_weather_pc34_compat` gains transition coverage
    (normal reseed, storm path, keep-current, NULL-RNG fail-closed).
    dm2_v1 lane 201 tests, same 27 known baseline failures, zero new
    failures. Remaining: the arg==0 DM2_UPDATE_WEATHER day-rollover and
    weather-visuals branch (c_weather.cpp:91-507 — needs the
    light/cloud/SFX/creature-strike subsystems), the v1d652d
    saved-weather flag semantics for the session-start arg, and the
    weather-timer saved-record owner (SKPROJECT-GAP-001).
  - 2026-07-19 update: per-cell DM2_THINK_CREATURE is now wired in the
    runtime over a session-owned DM2-002 record pool set. New
    `dm2_v1_record_pool_set_init_from_dungeon` exposes the pool
    population directly from dungeon data (the world wrapper moved to
    dm2_v1_world_model.c so record-pool consumers no longer link the
    world model). `dm2_v1_runtime_tick` lazily populates the session
    pool set from the boot dungeon data once its G1 candidate evidence
    validates and binds the 0x21/0x22 dispatch to
    `dm2_v1_think_creature_timer_handler`: a popped think timer
    resolves the DB4 creature record AT THE TIMER CELL via
    DM2_GET_CREATURE_AT (c_querydb.cpp:1486-1507) against the session
    pools, the no-creature early return consumes the timer without
    simulating, and the think body stays unbound (receipted) until the
    CCM stream owner/grammar is proven. The former unconditional
    CCM-instance step handler and its door-reader bridge are retired
    (they were dead code — no creature-scheduling producer exists yet);
    without validated dungeon evidence the 0x21/0x22 handlers stay
    unbound and the dispatcher acknowledges those timers fail-closed.
    New CTest `dm2_v1_think_creature_runtime_pc34_compat` PASS (lazy
    population, per-cell resolution, early return, unbound body,
    fail-closed without dungeon data). dm2_v1 lane 202 tests, same 27
    known baseline failures, zero new failures. Remaining: the
    creature-scheduling producer (map-load timer list + c_ai re-queue),
    the CCM stream owner/grammar for the think body, and the
    possession chain walk / tile-rooted ground-stack mutation for
    DM2-002.
  - 2026-07-20 update: the creature-scheduling producer DM2_1c9a_0cf7
    (c_1c9a.cpp:5695-5728) is now bound as a bounded slice in new module
    `dm2_v1_creature_schedule_pc34_compat` and wired into the runtime as
    `dm2_v1_runtime_schedule_creature_at` — the end-to-end chain is
    active. The producer resolves the creature record at (x, y) via
    DM2_GET_CREATURE_AT, derives the source timer tuple (type 0x22 when
    the record group/leader link word@8 != 0xffff, else 0x21,
    c_1c9a.cpp:5708-5712; owner = creature-type byte@4; due = gametick +
    1 via setmticks; payload = setxyA(x, y); map = caller-owned stand-in
    for ddat.v1d3248 until DM2_CHANGE_CURRENT_MAP_TO is proven) and
    enqueues it on the runtime source queue; the next
    dm2_v1_runtime_tick dispatches it through dm2_v1_proceed_timers to
    the per-cell DM2_THINK_CREATURE binding, which resolves the same
    record. The CAII creature-array slot timer word and the
    DM2_1c9a_0db0 delete stay host-owned until the CCM body is proven
    (receipted replaced_existing, never simulated); the spawn-site
    callers (DM2_ALLOC_CAII_TO_CREATURE map-load instantiation,
    c_creature.cpp:648, c_move.cpp:700, c_ai.cpp:5958) remain future
    wiring behind the new DM2-owned boundary. New CTests
    `dm2_v1_creature_schedule_pc34_compat` (timer derivation, due
    semantics, payload packing, fail-closed paths) and
    `dm2_v1_creature_schedule_runtime_pc34_compat` (end-to-end
    producer→queue→dispatch→think resolution, fail-closed without
    dungeon data) PASS. dm2_v1 lane 204 tests, same 27 known baseline
    failures, zero new failures. Remaining: the c_ai re-queue inside the
    DM2_PROCEED_CCM end (c_ai.cpp:5609-5614 + 5644) behind the CCM body,
    the ALLOC_CAII map-load spawn path (c_creature.cpp:384), the CCM
    stream owner/grammar for the think body, and the possession chain
    walk / tile-rooted ground-stack mutation for DM2-002.
  - 2026-07-20 update: the CAII creature-array slot allocator
    DM2_ALLOC_CAII_TO_CREATURE (c_1c9a.cpp:5772-5894) is now bound as a
    bounded slice in new module `dm2_v1_caii_alloc_pc34_compat` and wired
    into the runtime as `dm2_v1_runtime_caii_init` +
    `dm2_v1_runtime_alloc_caii_at` — the lazy creature-activation chain
    is active end-to-end. Source research correction: there is NO
    map-load CAII loop; activation is event-driven (DM2_ATTACK_CREATURE
    resolves the record via DM2_GET_CREATURE_AT at the activation cell
    when its record argument is -1, c_creature.cpp:347-352, then allocs;
    c_moverec.cpp:983, c_tim_proc.cpp:2887, c_1c9a.cpp:9982 call the
    allocator directly), so the runtime boundary mirrors that reach path
    instead of inventing a spawn walk. The session-owned CAII array
    (34-byte slots, source stride 0x22; capacity caller-owned stand-in
    for ddat.v1e08a0 until DM2_1c9a_3c30/DM2_INIT is proven; alloc
    counter stand-in for ddat.v1d4020) binds the observable slice: the
    record byte@5 early return, the word@0xe bit-10 rewrite receipted as
    a no-op, the free-slot scan (signed word@0 < 0), the full slot init
    (word@0 = bare record index — DM2_1c9a_0fcb rebuilds the DB4 handle
    by OR-ing 0x1000, c_1c9a.cpp:5915; word@2 = -1; byte@6 =
    (gametick >> 2) - 1; byte@4 = gametick - 0x7f; word@0xc =
    x | y<<5 | map<<10; byte@0x16/0x17 = -1; byte@7 = 0; record byte@5 =
    slot index), the bound DM2_1c9a_0cf7 producer queueing the first
    0x21/0x22 timer, and slot byte@1a = 0x00 grouped / 0x11 ungrouped.
    The no-free-slot path fails closed without mutation (the source
    recycle DM2_RECYCLE_A_RECORD_FROM_THE_WORLD c_1c9a.cpp:5880-5891 is
    unproven); DM2_PREPARE/UNPREPARE_LOCAL_CREATURE_VAR, DM2_14cd_0802
    and the s350.v1e0552/v1e054e group scan with
    DM2_CREATURE_SOMETHING_1c9a_0a48 stay host-owned until the CCM body
    is proven (receipted, never simulated). New CTests
    `dm2_v1_caii_alloc_pc34_compat` (slot init fields, early return,
    grouped/ungrouped modes, no-free-slot fail-closed, argument
    validation) and `dm2_v1_caii_alloc_runtime_pc34_compat` (activation
    → slot → timer → next-tick think resolution end-to-end,
    re-activation early return, fail-closed without CAII array or
    dungeon data) PASS. dm2_v1 lane 206 tests, same 27 known baseline
    failures, zero new failures. Remaining: the c_ai re-queue inside the
    DM2_PROCEED_CCM end (c_ai.cpp:5609-5614 + 5644) behind the CCM body,
    the CCM stream owner/grammar for the think body, the
    DM2_1c9a_0db0/DELETE_TIMER replacement path (needs stable timer
    indices in the source queue), the event-driven activation callers
    (ATTACK_CREATURE body, c_moverec.cpp:983, c_tim_proc.cpp:2887), and
    the possession chain walk / tile-rooted ground-stack mutation for
    DM2-002.
  - 2026-07-20 update: the DM2_1c9a_0db0/DELETE_TIMER replacement path
    is now bound — the creature timer chain is self-maintaining. The
    source queue gained stable session-issued tickets mirroring the
    timerarray slot index (DM2_QUEUE_TIMER returns the stable index,
    c_timer.cpp:235-257; DM2_DELETE_TIMER frees it,
    c_timer.cpp:215-232): additive `tickets[]`/`next_ticket` fields on
    DM2_V1_SourceTimerQueue, `dm2_v1_source_timer_enqueue_ticketed`
    (legacy enqueue delegates), and `dm2_v1_source_timer_cancel`
    (fail-closed on zero/unknown/stale tickets). The creature-schedule
    receipt now carries `timer_ticket`. New in
    `dm2_v1_caii_alloc_pc34_compat`: `dm2_v1_caii_delete_timer` binds
    DM2_1c9a_0db0 (c_1c9a.cpp:5734-5763 — DB4 check, record byte@5 slot,
    slot word@2 pending-timer delete + write-back -1) over the session
    tickets, and `dm2_v1_caii_schedule_creature_at` binds the COMPLETE
    DM2_1c9a_0cf7 (c_1c9a.cpp:5695-5728): replace-first when the slot
    timer word references a live ticket (receipt.replaced_existing == 1,
    stale post-dispatch references fail safe), enqueue, then store the
    issued ticket in slot word@2 (c_1c9a.cpp:5724-5728); the CAII alloc
    path now writes word@2 as well. A record without a CAII slot fails
    closed (no_caii_slot) — the source would index the creatures array
    out of bounds. Runtime boundary
    `dm2_v1_runtime_reschedule_creature_at` exposed for the source's
    direct callers (c_creature.cpp:648, c_move.cpp:700). New CTests
    `dm2_v1_caii_timer_replace_pc34_compat` (ticket issue/cancel/
    stale-guard, 0db0 paths, complete replacement without duplicate
    accumulation) and `dm2_v1_caii_reschedule_runtime_pc34_compat`
    (activation → reschedule → exactly one think timer dispatched,
    post-dispatch reschedule still schedules) PASS. dm2_v1 lane 208
    tests, same 27 known baseline failures, zero new failures.
    Remaining: the c_ai re-queue inside the DM2_PROCEED_CCM end
    (c_ai.cpp:5609-5614 + 5644) behind the CCM body, the CCM stream
    owner/grammar for the think body, the event-driven activation
    callers (ATTACK_CREATURE body, c_moverec.cpp:983,
    c_tim_proc.cpp:2887), DM2_1c9a_0fcb (CAII slot free), and the
    possession chain walk / tile-rooted ground-stack mutation for
    DM2-002.
  - 2026-07-20 update: DM2_1c9a_0fcb (CAII slot free,
    c_1c9a.cpp:5896-5944) is now bound as `dm2_v1_caii_free_slot` in
    `dm2_v1_caii_alloc_pc34_compat` with runtime boundary
    `dm2_v1_runtime_free_caii_slot` — the slot lifecycle is complete
    (alloc → schedule → delete → free). The bounded slice fails closed
    for out-of-range indexes (the source compares slot > ddat.v1e08a0
    unsigned and would index out of bounds at slot == capacity,
    c_1c9a.cpp:5905), takes the already-free early return, rebuilds the
    DB4 handle as slot word@0 | 0x1000 (c_1c9a.cpp:5915), clears slot
    byte@1a, deletes the pending timer through the round-7 bound
    DM2_1c9a_0db0 path (c_1c9a.cpp:5933), decrements the alloc counter,
    clears record byte@5 and marks the slot free. The
    DM2_DELETE_CREATURE_RECORD branch (c_1c9a.cpp:5930-5944, including
    the timer payload read) stays unbound — its flag derives from
    DM2_QUERY_CREATURE_AI_SPEC_FLAGS whose AI-spec table owner is
    unproven (receipted record_delete_unbound, never simulated). The
    source's despawn/cleanup callers (c_ai.cpp:5775,
    c_moverec.cpp:684 + 997, c_savegame.cpp:2049) remain future wiring.
    New CTests `dm2_v1_caii_free_pc34_compat` (guard paths, free
    semantics, slot reuse lifecycle) and
    `dm2_v1_caii_free_runtime_pc34_compat` (activate → free → no
    dispatch for the freed creature → re-activation reuses the slot and
    the think chain resumes end-to-end) PASS. dm2_v1 lane 210 tests,
    same 27 known baseline failures, zero new failures. Remaining: the
    c_ai re-queue inside the DM2_PROCEED_CCM end (c_ai.cpp:5609-5614 +
    5644) behind the CCM body, the CCM stream owner/grammar for the
    think body, the event-driven activation callers (ATTACK_CREATURE
    body, c_moverec.cpp:983, c_tim_proc.cpp:2887), the AI-spec table
    owner (gates the 0fcb record-delete branch and the ATTACK_CREATURE
    alloc guard), and the possession chain walk / tile-rooted
    ground-stack mutation for DM2-002.
  - 2026-07-20 update: the AI-spec table owner is now proven and bound.
    The source chain DM2_QUERY_CREATURE_AI_SPEC_FLAGS
    (c_record.cpp:1346-1349) → DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD
    (c_record.cpp:1351-1354) resolves the creature type's GDAT CREATURES
    word field 0x05 into the 36-byte AIDefinition table
    (table1d296c) whose word@0 holds the flags — exactly the indirection
    the proven EXTENDED_LOAD_AI_DEFINITION GDAT path
    (SkWinCore.cpp:233-400) already captures. New accessor
    `dm2_v1_creature_ai_spec_flags` in `dm2_v1_creature` follows that
    captured indirection (fail-closed 0 for types the session did not
    define) without touching the legacy capped-index
    `dm2_v1_creature_ai_spec` view its combat/projectile consumers rely
    on. The CAII module consumes the flags through a session-wired
    provider hook `dm2_v1_caii_set_ai_spec_flags_fn` (the CAII module
    must not depend on the creature translation unit; with no provider
    wired both sites fail closed with "unknown provenance"). Two gates
    are now data-backed: (1) DM2_1c9a_0fcb's record-delete flag
    (c_1c9a.cpp:5917-5929) is computed as ((flags & 0x1) == 0 && slot
    byte@1a == 0x13) and receipted `record_delete_flag` (1/0, -1
    unknown) — the DM2_DELETE_CREATURE_RECORD branch itself stays
    unbound (receipted record_delete_unbound, never simulated); (2) new
    accessor `dm2_v1_caii_attack_guard_allows_alloc` binds the
    ATTACK_CREATURE vl_18 gate (c_creature.cpp:370-385: AIDefinition
    word@0 & 1; the source allocs the CAII slot only when set and
    returns early when clear) returning 1/0/-1 (unknown). New CTest
    `dm2_v1_caii_ai_spec_pc34_compat` drives a fully synthetic
    dtWordValue GDAT fixture (type 12 → AI row 5 → flags 0x0001; type 7
    → AI row 9 → flags 0x0200) and covers the accessor, the fail-closed
    no-session/no-provider paths, the attack-guard polarity, and the
    full 0fcb record-delete matrix (bit0 × byte@1a). PASS. dm2_v1 lane
    211 tests, same 27 known baseline failures, zero new failures.
    Remaining: the DM2_DELETE_CREATURE_RECORD body (c_1c9a.cpp:5930-5944
    timer payload read), the ATTACK_CREATURE body itself, the
    event-driven activation callers (c_moverec.cpp:983,
    c_tim_proc.cpp:2887), the c_ai re-queue inside the DM2_PROCEED_CCM
    end, the CCM stream owner/grammar, and the possession chain walk /
    tile-rooted ground-stack mutation for DM2-002.
  - 2026-07-20 update: the DM2_DELETE_CREATURE_RECORD branch head of
    DM2_1c9a_0fcb (c_1c9a.cpp:5936-5957) is now TAKEN data-backed, and
    the delete body's provable decision head is bound. New read-only
    queue accessor `dm2_v1_source_timer_peek_ticket` binds the source's
    timerarray slot read (c_1c9a.cpp:5943-5944): when the round-9
    record-delete flag is set AND the slot timer word holds a live
    ticket, the payload is read BEFORE the timer is deleted (source
    order: payload 5939-5945, byte@1a 5950, DM2_1c9a_0db0 5952) —
    valueA lo/hi bytes are the branch's (x, y) arguments (c_timer.h:80-82
    getxA/getyA). Flag set without a pending timer takes the source's
    RG3L = 0 outcome (receipted record_delete_no_timer). After the slot
    is marked free the branch runs the new bounded decision head
    `dm2_v1_caii_delete_creature_record_head` (c_record.cpp:1357-1425):
    DM2_GET_CREATURE_AT(x, y) resolution with the source early return,
    the jz_test8 AI gate (aidef byte@0 & 1, c_record.cpp:1385) computed
    data-backed through the wired provider, and the BOUND CAII slot
    byte@1a clear for creatures still owning a slot
    (c_record.cpp:1408-1413). The mutating tail stays unbound behind
    named receipts: the table1d607e/GDAT-word@1 probe + map swap +
    DM2_INVOKE_MESSAGE (c_record.cpp:1387-1406), the tile-rooted
    DM2_MOVE_RECORD_TO cut (c_record.cpp:1419, DM2-002),
    DM2_DROP_CREATURE_POSSESSION (c_record.cpp:1422, DM2-002),
    DM2_1c9a_0247 tagged-dballoc cleanup (c_record.cpp:1423,
    c_1c9a.cpp:5135-5160) and DM2_DEALLOC_RECORD's pool free-chain
    (c_record.cpp:1424). `dm2_v1_caii_free_slot` gained a dungeon
    parameter for the head (runtime boundary and all callers updated);
    the runtime now wires the proven provider
    (dm2_v1_creature_ai_spec_flags) in its think-binding init. New CTest
    `dm2_v1_caii_record_delete_pc34_compat` covers the peek accessor,
    the branch-taken path with payload coords, the no-timer RG3L = 0
    path, the bit0-set closed branch, the no-provider closed branch, and
    the direct head matrix (early return, gate polarity, slot mode-byte
    clear, NULL-CAII fail-closed). PASS. dm2_v1 lane 212 tests, same 27
    known baseline failures, zero new failures. Remaining: the
    delete body's mutating tail (DM2-002 ground-stack/possession walk,
    dballoc free chain, message system), the ATTACK_CREATURE body, the
    event-driven activation callers (c_moverec.cpp:983,
    c_tim_proc.cpp:2887), the c_ai re-queue inside the DM2_PROCEED_CCM
    end, and the CCM stream owner/grammar.
  - 2026-07-20 update: the ATTACK_CREATURE message body
    (c_creature.cpp:318-649) is now TAKEN data-backed as
    `dm2_v1_caii_attack_creature` in the CAII module. The bound slice
    covers: handle -1 resolution through DM2_GET_CREATURE_AT with the
    source early return (c_creature.cpp:345-352), the unknown-AI-flags
    guard via the wired provider, the CAII-slot-less static denial, the
    vol_00 bit cuts (and16 0xbfff / poke16 with the x86 SHL mod-32
    count), the RANDBIT vol&0x4000 survival, the hp word write, the
    three-band aggro block (c_creature.cpp:394-435: hp > 30 sets, hp
    <= 4 probes 100*hp/aidef word@4 > 0xf, the 5..30 middle band
    consumes RANDDIR), the champion bit set/clear into record word@0xa
    gated by vl_14 = strength > RAND16(100) (c_creature.cpp:539-563),
    and the full reschedule gate (c_creature.cpp:566-648): the
    vl_10+strength-0 forced rg1, the skip00254 cut, the
    table1d613a[slot byte@1a] chain into table1d607e[GDAT word@1]
    t6 & 0x410 with the & 2 tail, the dying-mode 0x13 and
    below-word@6-threshold early returns, and the closing
    DM2_1c9a_0db0 + DM2_1c9a_0cf7 cancel-and-reschedule. The c_ai turn
    block (c_creature.cpp:438-536) stays host-owned: its entry gate
    (table1d607e[w1].uc[0] & 0x80) is computed data-backed, and a
    passing gate declares the RNG stream diverged and stops BEFORE the
    reaction roll, fail-closed — the block consumes a variable number
    of draws the bounded slice cannot reproduce. RNG draws run on the
    session DM2_V1_DropRng through LOCAL LCG helpers with CUTX16
    semantics: RAND16(n) = CUTX16(draw) % n and 100 does not divide
    2^16, so the drops.h RAND16 macro would yield wrong values
    (drops.h:80-86 documents this). table1d607e (47x4) and
    table1d613a (86 bytes, proven span 0x00-0x55) are verbatim
    per-module copies (mdata.c:1564-1639); out-of-span mode bytes fail
    closed. New creature-module accessors `dm2_v1_creature_ai_base_hp`
    (aidef word@4) and `dm2_v1_creature_gdat_word1` (CREATURES word@1,
    loader field 0x01 capture) feed the runtime think-binding. The
    delete head gained a data-backed `invoke_message_would_run`
    receipt (uc[0] & 4 == 0). New CTest
    `dm2_v1_caii_attack_pc34_compat` covers fifteen scenarios (a-o):
    resolve/early-return, static denial, alloc failure, aggro bands
    with and without a stream, vl_10 forced rg1, dying mode, threshold,
    champion bit set/clear, the table chain to rg1 = 1, the span
    guard, and the diverged-stream fail-close. PASS. dm2_v1 lane 213
    tests, same 27 known baseline failures, zero new failures.
    Remaining: the c_ai turn block itself (DM2_ai_13e4_0360 +
    CALC_VECTOR_DIR), the delete body's mutating tail (DM2-002), the
    event-driven activation callers (c_moverec.cpp:983,
    c_tim_proc.cpp:2887), the c_ai re-queue inside the DM2_PROCEED_CCM
    end (c_ai.cpp:5609-5614, 5644), and the CCM stream owner/grammar.
  - 2026-07-20 update: the c_ai turn block inside ATTACK_CREATURE
    (c_creature.cpp:438-536) is now BOUND — the round-11 diverged-stream
    stop is replaced by the full data-backed direction dance. The bound
    slice covers: the entry RANDBIT (c_creature.cpp:444),
    DM2_CALC_VECTOR_DIR (util.cpp:30-46, verbatim — including its
    tie-break RANDBIT draw) from the creature's CCM dispatch coordinates
    toward the attack origin, the word@0xa & 8 branch, the
    skip00247/skip00248/skip00251 ladder over the record's facing bits
    (word@0xe >> 8 & 3) with the exact source RNG draw sequence
    (RANDDIR/RANDBIT draws in source order), and DM2_ai_13e4_0360 with
    argl0 == 0 (c_ai.cpp:5912-5960): the slot byte@0x17/0x1a == 0x13
    guards and the byte@0x17 direction write (c_ai.cpp:5946). Final
    turn values 0-3 (absolute), 6/7 (relative) and -1 (no turn) are
    receipted. The creature's position (ddat.v1e0270/v1e0272,
    c_dballoc.cpp:438-440 — globals the source reads) enters the
    bounded slice as the new target_x/target_y parameters of
    `dm2_v1_caii_attack_creature`. Fail-closed paths kept: gate
    unknown/out of span, or gate passed without a bound session stream,
    still stop BEFORE the reaction roll (rng_stream_diverged). The
    argl0 != 0 tail of DM2_ai_13e4_0360 (byte@0x21 flag / 0db0+0cf7
    requeue, c_ai.cpp:5949-5959) belongs to OTHER callers
    (c_ai.cpp:2114, c_tim_proc.cpp:2988) and stays unbound.
    `dm2_v1_caii_attack_pc34_compat` reworked: scenario (h) now proves
    the bound turn (seeded LCG draws: survival bit 0, entry bit 1,
    RANDDIR 2, reaction roll 60 -> turn dir 7 written to slot byte@0x17,
    body completes), and new scenarios (p)-(s) cover the entry-flip-0
    no-turn with an aligned reaction roll, the skip00248 reversal
    (turn 6), the byte@0x17 write guard with the dying-mode tail, and
    the diverged-stream stop without a bound RNG. 19 scenarios PASS.
    dm2_v1 lane 213 tests, same 27 known baseline failures, zero new
    failures (the broad -R dm2 dm2_v2 probe failures pre-exist the
    lane merges — verified identical with the changes stashed).
    Remaining: the DM2_ai_13e4_0360 argl0 != 0 tail callers
    (c_ai.cpp:2114, c_tim_proc.cpp:2988), the delete body's mutating
    tail (DM2-002), the event-driven activation callers
    (c_moverec.cpp:983, c_tim_proc.cpp:2887), the c_ai re-queue inside
    the DM2_PROCEED_CCM end (c_ai.cpp:5609-5614, 5644), and the CCM
    stream owner/grammar.
  - 2026-07-20 update: the c_ai re-queue at the DM2_PROCEED_CCM end
    (c_ai.cpp:5608-5614 + 5641-5646) is now TAKEN data-backed as
    `dm2_v1_caii_ccm_end_requeue`. The CCM message loop itself (the
    stream grammar) stays host-owned; its outputs enter the bounded
    slice as explicit parameters: s350.v1e0562's loop-owned payload
    fields (actor/valueA/valueB pass through), RG4W at m_15785
    (loop_result), s350.v1e0570 (suppress_requeue), s350.v1e0571
    (mticks_map) and the DM2_CREATURE_SOMETHING_1c9a_0a48() result
    (mticks_delta — that animation-frame reader, c_1c9a.cpp:5434+,
    stays host-owned). Bound in source order: the timer type
    (loop_result != 1 ? 1 : 0) + 0x21 (c_ai.cpp:5609-5611), the
    v1e0570 suppression return (c_ai.cpp:5612-5613, receipted
    suppressed), the setmticks word rebuild (c_ai.cpp:5614,
    c_timer.h:66 — the delta OR-ed unmasked, kept verbatim), the
    slot word@2 != -1 cancel through the bound DM2_1c9a_0db0
    (c_ai.cpp:5641-5643), DM2_QUEUE_TIMER over the session queue
    (c_ai.cpp:5644, c_timer.cpp:235-257), and the ticket store into
    slot word@2 (c_ai.cpp:5646). New scenarios (z)-(cc) in
    `dm2_v1_caii_attack_pc34_compat` cover the full requeue with
    peek-verified type/setmticks/payload, the 0x21 type, the
    suppression path (no cancel, no enqueue), and the slot-less
    fail-close. 29 scenarios PASS. dm2_v1 lane 213 tests, same 27
    known baseline failures, zero new failures. Remaining: the CCM
    stream owner/grammar itself (the message loop + the s350
    context), DM2_CREATURE_SOMETHING_1c9a_0a48, the XACT/timer-proc
    AI-stop callers (c_ai.cpp:2078-2117, c_tim_proc.cpp:2907-2988+),
    the delete body's mutating tail (DM2-002), the event-driven
    activation callers (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-20 update (round 14): DM2_CREATURE_SOMETHING_1c9a_0a48 is
    now BOUND data-backed in new module
    `dm2_v1_creature_something_pc34_compat`, including its animation
    core DM2_GET_CREATURE_ANIMATION_FRAME + DM2_4FCC
    (c_creature.cpp:3217-3278 + 3285-3378). The data path is fully
    proven: the dtRaw8/0xfb attribution scan and dtRaw7/0xfc info
    sequence resolve through the real GDAT asset loader, the
    AIDefinition static/dynamic gate (aidef byte@0 bit0) resolves
    through the session AI table (dm2_v1_creature_ai_spec_def), and
    every RAND16/RANDBIT/RAND draw consumes the session LCG in source
    order. The s350 context enters as explicit parameters (record
    handle for v1e054e, CAII slot for s350.creatures, adj pair for
    v1e055e, row pointer for v1e055a, ddat map/home/v1e0238/b03 and
    s350.v1e0584 scalars). Bound in source order: the GAF fetch with
    parl01 = 0 / packed word@0xc (c_1c9a.cpp:5462-5477), the source's
    own zeroed 4-byte fallback row with the NULL reset
    (c_1c9a.cpp:5478-5481 + 5668-5670), the jitter/bit6 frame-byte
    dance with the 0x23/0x24/0x25 mode guard writing slot byte@7 and
    the adj pair (c_1c9a.cpp:5484-5550), and the complete delta
    arithmetic — dying-mode *3, flee *4+RANDBIT, the 75x/100 max-1
    band (the source's dead modulo kept as evidence), the map *2/*4
    band, the big-creature min(1, hi) band, and the signed 16-bit
    truncation — returning gametick + delta exactly as the CCM end
    re-queue's mticks_delta consumer expects (c_ai.cpp:5614).
    DM2_QUEUE_NOISE_GEN1 stays unproven and is receipted
    (noise_would_queue + index), never simulated; the unchecked
    table1d607e[v1e0584] probe fails closed outside the proven 0x2f
    span. New test `dm2_v1_creature_something_pc34_compat` covers the
    GAF static/dynamic/fail-closed paths plus twelve 1c9a_0a48
    scenarios including LCG determinism. dm2_v1 lane: 208 passed,
    same 33 known baseline failures (verified identical on the
    pristine tree), zero new failures. Remaining: the CCM stream
    owner/grammar itself (the message loop + the s350 context), the
    XACT/timer-proc AI-stop callers (c_ai.cpp:2078-2117,
    c_tim_proc.cpp:2907-2988+ — both now fully unblocked: their
    DM2_ai_13e4_0360 and DM2_ATTACK_CREATURE primitives are bound,
    XACT_85 additionally needs the DM2-002 tile record-link walk),
    the delete body's mutating tail (DM2-002), the event-driven
    activation callers (c_moverec.cpp:983, c_tim_proc.cpp:2887), and
    a canonical GRAPHICS.DAT real-data companion test for the new
    animation reader.
  - 2026-07-20 update: DM2_ai_13e4_0360 (c_ai.cpp:5912-5960) is now
    bound COMPLETE as the public `dm2_v1_caii_ai_13e4_0360` — including
    the argl0 != 0 AI-stop tail its other callers use
    (c_creature.cpp:233, c_ai.cpp:2114 DM2_PROCEED_XACT_85,
    c_tim_proc.cpp:2988 DM2_ACTIVATE_CREATURE_KILLER — all three pass
    dir 0x13 with argl0 == 1). The complete slice: handle -1 resolution
    via DM2_GET_CREATURE_AT with the source early return
    (c_ai.cpp:5925-5931), the record byte@5 == 0xff guard
    (c_ai.cpp:5934-5936), the slot byte@0x17/0x1a == 0x13 guards
    (c_ai.cpp:5941-5944 — once the AI-stop marker is written, further
    turns are blocked, receipted guard_denied), the byte@0x17 direction
    write (c_ai.cpp:5945-5946), the argl0 == 0 return, and the argl0
    != 0 tail (c_ai.cpp:5949-5959): table1d613a[slot byte@1a] & 0x10
    sets slot byte@0x21 = 1, otherwise the bound DM2_1c9a_0db0 +
    DM2_1c9a_0cf7 pair cancels the pending timer and re-queues the
    think timer at (x, y). The table's proven span 0x00-0x55 is
    enforced fail-closed AFTER the dir write, exactly like the source's
    out-of-bounds read order (mode_b1a_out_of_span). The x/y parameters
    are the creature's coordinates (the source's edxl/ebxl, reused for
    the requeue); map_id/game_tick carry the CCM dispatch context. The
    ATTACK_CREATURE c_ai turn block keeps its own inline argl0 == 0
    binding (round 12) — same source, same guards. New scenarios
    (t)-(y) in `dm2_v1_caii_attack_pc34_compat` cover the direct
    argl0 == 0 write, the byte@0x21 flag path with the follow-up 0x13
    guard denial, the cancel-and-requeue tail over the live queue, the
    handle -1 resolution (direction bits kept, as the tile link word
    carries them) with the empty-cell early return, the byte@5 guard,
    and the tail span guard. 25 scenarios PASS. dm2_v1 lane 213 tests,
    same 27 known baseline failures, zero new failures. Remaining: the
    XACT/timer-proc CALLERS themselves (DM2_PROCEED_XACT_85
    c_ai.cpp:2078-2117, DM2_ACTIVATE_CREATURE_KILLER
    c_tim_proc.cpp:2907-2988+), the delete body's mutating tail
    (DM2-002), the event-driven activation callers (c_moverec.cpp:983,
    c_tim_proc.cpp:2887), the c_ai re-queue inside the DM2_PROCEED_CCM
    end (c_ai.cpp:5609-5614, 5644), and the CCM stream owner/grammar.
  - 2026-07-20 update (round 15): the CCM stream owner/grammar itself
    is now BOUND as a bounded slice in new module
    `dm2_v1_ccm_loop_pc34_compat` — DM2_13e4_0982 (c_ai.cpp:5341-5647),
    the message loop DM2_THINK_CREATURE runs for a living creature.
    Bound in source order: the pre-check (c_ai.cpp:5346-5383 — body
    when savegames1.b_03 == 0, aidef byte@1 & 0x10, or a 0x13
    command/pending command; otherwise adddata(4) on the payload long,
    c_timer.h:68, and NO body); the !flag branch's standalone DM2_4FCC
    (newly exported `dm2_v1_creature_anim_4fcc` from the round-14
    module — GAF's dynamic tail refactored into the shared
    dm2_v1_anim_4fcc_walk helper, zero behavior change, round-14 tests
    PASS unchanged); the flag branch's b_1a = b_17 dance with the
    DM2_14cd_09e2 AI-goal boundary (fail-closed AFTER the source's
    b_1a write), the bound DM2_14cd_062e byte@0x12 == 0xff head (the
    table1d5f82 s_seven chain stays unproven, fail-closed), the
    table1d613a & 4 bitmap-state receipt and the mode 6/7 facing write
    to slot byte@0x1d; the dying branch (slot words 0xe/0x10 = adj,
    the data-backed table1d607e[GDAT word@1].uc[0] & 8 gate with
    fail-closed span guard, the aidef byte@0x23 cloud selector
    0xbe/0xff/0x6e receipted cloud_would_create, never simulated); the
    0x32..0x34 special setmticks(v1e0571, b_1a + gametick - 50) with
    NO loop; the bound round-14 GAF; and the loop grammar itself
    (c_ai.cpp:5567-5606): the !flag/byte@0x21/row byte@2 & 0x40
    handler skip, the row byte@2 & 0x80 handler gate with the bound
    DM2_13e4_01a3 slice (v1e07eb once-guard, lazy v1e0584 GDAT word@1,
    the v1e058d RAND16(2*(0xf-(aidef w16 & 0xf))+1) draw with
    CUTX16-then-modulo semantics against the gametick-minus-slot-byte@4
    window), the DM2_PROCEED_CCM dispatch RECEIPTED through the proven
    DM2-005 matrix and fail-closed (handler bodies stay host-owned),
    and the bound DM2_50CB deterministic stream step
    (c_ai.cpp:5275-5338) whose result 2 BREAKs the loop while 0/1
    CONTINUE it (c_ai.cpp:5602-5604). A NULL animation row fails
    closed (anim_row_null — the source dereferences it unchecked).
    The m_15785 end honors the v1e0570 suppression BEFORE the delta
    computation, calls the bound round-14 1c9a_0a48 for the mticks
    delta and hands the type rebuild + cancel/queue/store to the
    round-13 end_requeue; the non-loop exits (payload skip, 0x32..0x34)
    reach the m_15843 tail (c_ai.cpp:5642-5647) composed from the
    bound delete-timer + enqueue-ticketed primitives with the ticket
    stored in slot word@2. The m_157BC bitmap block stays unbound
    (structurally unreachable: its v1e07eb guard needs a completed
    13e4_01a3, and the handler boundary fails closed first). New CTest
    `dm2_v1_ccm_loop_pc34_compat` covers nine scenarios (a-i): the
    !flag full path (4FCC + two 50CB steps + end requeue with
    peek-verified queue state), the payload skip, the 0x32..0x34
    special, the dying branch, the handler boundary with the bound
    01a3 slice, the AI-goal and s_seven fail-closes, the static
    NULL-row fail-close, and the mode 6 facing write. PASS. New
    canonical companion test `dm2_v1_creature_something_real_data`
    proves the animation reader's data path against the real
    GRAPHICS.DAT through the actual loader: the dtRaw8/0xfb
    attribution terminator scan + dtRaw7/0xfc info table are admitted
    for 57 creature types locally (the AI-gated GAF/4FCC/1c9a_0a48
    identity checks run when the profile admits the AI classification;
    the established real-data test SKIPs that gate the same way).
    dm2_v1 lane 216 tests, same 27 known baseline failures (verified
    identical with the changes stashed), zero new failures.
    Remaining: runtime wiring of the bound loop into the think-binding
    (the s350 context owners PREPARE/UNPREPARE stay host-owned), the
    per-command CCM handler bodies (currently receipted at the
    dispatch-matrix boundary), DM2_14cd_09e2 (the AI goal picker) and
    the table1d5f82 s_seven chain, the m_157BC bitmap block, the
    XACT/timer-proc AI-stop callers (c_ai.cpp:2078-2117,
    c_tim_proc.cpp:2907-2988+ — XACT_85 needs the DM2-002 tile
    record-link walk), the delete body's mutating tail (DM2-002
    tile-rooted ground-stack + possession walk), and the event-driven
    activation callers (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-20 update (round 16): the DM2-002 tile record-link walk is
    now a first-class bounded primitive, and both AI-stop callers that
    were blocked on it are BOUND. New module
    `dm2_v1_tile_record_walk_pc34_compat` binds DM2_GET_TILE_RECORD_LINK
    (c_map.cpp:61-69 — the bit-0x10 object flag + column-index
    ground-stack head, over the proven loader binding) and the bounded
    next-link walk (c_record.cpp:54-57 — OBJECT_END_MARKER terminates,
    corrupt chains bounded by the declared record count, fail-closed).
    On top of it: DM2_PROCEED_XACT_85 (c_ai.cpp:2078-2117) — the cell
    chain walk with the DB-index > 3 break, the DB2 word@2 probe
    ((w & 0x6) == 0x2 and (w >> 11) == 1) ending with slot byte@0x1e =
    1, byte@0x1a = 59 and the source return -2, and the walk-end tail
    running the bound DM2_ai_13e4_0360 AI-stop (dir 0x13, argl0 1)
    before the unconditional byte@0x1a = 51 write and return -3 (the
    slot writes land through record byte@5 exactly like
    s350.creatures); and DM2_ACTIVATE_CREATURE_KILLER
    (c_tim_proc.cpp:2907-2988) — the rectangular sweep around
    (ebx, ecx) with radii |ebx - argl0| / |ecx - argl1|, the map-bounds
    skip, per-cell DM2_GET_CREATURE_AT, the DM2_1c9a_09b9 record word@8
    filter (c_1c9a.cpp:5404-5414), action 0xb mode-word semantics (0/1
    skip, 2 bound AI-stop, above 2 aborts the whole sweep), and action
    0x28 running the bound DM2_ATTACK_CREATURE with attack word
    (RG6 low 16 bits) | (argw3 ? 0x8000 : 0), strength 0x64, hp delta
    0. New CTest `dm2_v1_tile_record_walk_pc34_compat` covers the walk
    primitive (order, end marker, empty chain, bounded corrupt
    self-loop), the XACT_85 match/walk-end/fail-closed paths, and the
    killer sweep (filter, modes 1/2/3, attack with unwired providers
    failing closed, out-of-bounds edge sweep, unknown action). PASS.
    dm2_v1 lane 219 tests, same 27 known baseline failures (the two
    merged-lane "Not Run" entries pass once their targets are built),
    zero new failures. Remaining: the delete body's mutating tail
    (DM2-002 tile-rooted ground-stack + possession walk), runtime
    wiring of the bound loop/callers into the think-binding (the s350
    context owners PREPARE/UNPREPARE stay host-owned), the per-command
    CCM handler bodies, DM2_14cd_09e2 and the table1d5f82 s_seven
    chain, the m_157BC bitmap block, and the event-driven activation
    callers (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-20 update (round 17): the DM2_DELETE_CREATURE_RECORD
    mutating tail (c_record.cpp:1416-1424) is now bound as
    `dm2_v1_caii_delete_creature_record_tail` in the CAII module,
    composing with the round-12 decision head. The tile-rooted
    ground-stack cut (c_record.cpp:1419 — the DM2_MOVE_RECORD_TO x == -4
    skip00823/3CE7D path's observable end state, c_moverec.cpp:630-683)
    is BOUND: a bounded membership pre-walk (same budget discipline as
    the round-16 walk primitive) guarantees the source splice cannot
    spin on a corrupt chain, the record-pool list-cut mirrors
    DM2_CUT_RECORD_FROM exactly, and the chain head is rewritten in the
    dungeon ground-stack table through the new loader setter
    `dm2_v1_dungeon_set_first_thing` (byte-square cells with the 0x10
    object flag only, same index computation and bounds discipline as
    the getter — additive, all loader-dependent tests unchanged).
    DM2_DEALLOC_RECORD (c_record.cpp:1424 via c_record.cpp:1205-1208) is
    BOUND: the record's first word becomes the 0xffff free marker. The
    3CE7D timer/text side effects and the recursive DM2_1c9a_0fcb slot
    free inside the cut, DM2_DROP_CREATURE_POSSESSION
    (c_record.cpp:1422 — RAND16 scatter + ALLOC_NEW_DBITEM body) and
    the DM2_1c9a_0247 tagged-dballoc cleanup (c_record.cpp:1423,
    c_1c9a.cpp:5135-5160) stay unbound behind named receipts, never
    simulated. New CTest `dm2_v1_delete_creature_tail_pc34_compat`
    covers the setter discipline, the head+tail composition mid-chain
    cut, the head cut with ground-stack rewrite, dealloc, and the
    fail-closed paths (re-cut, bounded self-loop, unresolvable record,
    end-marker handle). PASS. dm2_v1 lane 220 tests, same 27 known
    baseline failures, zero new failures. Remaining: the possession
    drop body, the 3CE7D cut side effects, the 1c9a_0247 dballoc tag
    system, DM2_INVOKE_MESSAGE (host message system), runtime wiring of
    the bound loop/callers into the think-binding, the per-command CCM
    handler bodies, DM2_14cd_09e2 and the table1d5f82 s_seven chain,
    the m_157BC bitmap block, and the event-driven activation callers
    (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-21 update (round 18): DM2_DROP_CREATURE_POSSESSION
    (c_record.cpp:1537-1752) is now BOUND as a bounded slice in new
    module `dm2_v1_drop_possession_pc34_compat` — the delete tail's
    possession-drop receipt is retired. Bound in source order: the
    mode == 2 immediate return; the mode == 0 generated-drops loop
    (GDAT CREATURES drop fields 0x0A..0x14) delegated to the proven
    `dm2_v1_drops_place_source_slots` binding with the destination head
    now TILE-ROOTED in the dungeon ground-stack table through the
    round-17 setter (written back only when the head changed); and the
    possession chain walk from the creature's word@2 link — next link
    prefetched BEFORE the move, the AI-flags bit0 direction
    randomization ((party_dir + RANDBIT) & 3 on the party cell,
    RANDDIR elsewhere) folded into the handle ((dir << 14) |
    handle & 0x3fff), DB != 0x0e items appended to the drop cell (the
    MOVE_RECORD_TO from-nowhere end state) and DB 0x0e records
    deallocated (word@0 = 0xffff). Fail-closed discipline: flag-less
    drop cells (map-table growth unproven), corrupt destination chains
    (bounded end pre-walk before every unbudgeted source splice),
    unwired AI flags (stops BEFORE the walk's first RNG draw), and a
    missing LCG (stops before any mutation). DM2_QUEUE_NOISE_GEN2 stays
    host-owned, receipted per dropped item. New CTest
    `dm2_v1_drop_possession_pc34_compat` covers the full slice
    (generated drops + walk with link-chain verification incl.
    direction bits), AI bit0 set, the empty-cell head write-back,
    mode 2, and the fail-closed paths. PASS. dm2_v1 lane 221 tests,
    same 27 known baseline failures, zero new failures. Remaining: the
    3CE7D cut side effects, the 1c9a_0247 dballoc tag system,
    DM2_INVOKE_MESSAGE (host message system), wiring the bound drop
    into the delete tail, runtime wiring of the bound loop/callers
    into the think-binding, the per-command CCM handler bodies,
    DM2_14cd_09e2 and the table1d5f82 s_seven chain, the m_157BC
    bitmap block, and the event-driven activation callers
    (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-21 update (round 19): DM2_INVOKE_MESSAGE
    (c_tim_proc.cpp:4332-4367) is now BOUND as a bounded slice in new
    module `dm2_v1_invoke_message_pc34_compat` — setmticks
    (c_timer.h:66), settype 0x4, the RG3UW actor mapping (0->1, 1->3,
    2->2, else the c_tim init default 0), setxyA/setxyB
    (c_timer.h:82,90), and the ticketed DM2_QUEUE_TIMER enqueue with the
    producer-module source_index 0u convention; the message dispatch at
    processing stays host-owned. The COMPLETE DM2_DELETE_CREATURE_RECORD
    (c_record.cpp:1357-1425) is now bound as a source-ordered
    composition in new module `dm2_v1_delete_creature_full_pc34_compat`:
    GET_CREATURE_AT early return; the jz_test8 AI gate and the
    table1d607e[GDAT word@1] &4 probe data-backed through the caii
    module's new read-only provider/table accessors (the providers and
    the verbatim mdata table stay module-owned — the composition lives
    in its own translation unit to keep the link boundary); the
    word@0xc decode (map = (w >> 10) & 0x3f, the y word w >> 5 with
    setxyA keeping the low byte, x = w & 0x1f), the map swap receipted
    (single-map session) and DM2_INVOKE_MESSAGE(x, y, 0, 0,
    gametick + 1) BOUND; the CAII slot byte@1a clear
    (c_record.cpp:1408-1413); the tile-rooted cut (bounded membership
    pre-walk BEFORE any mutation, cut + ground-stack head rewrite); the
    round-18 bound DM2_DROP_CREATURE_POSSESSION wired into the tail with
    the session's wired AI flags provider (a fail-closed drop skips the
    dealloc); DM2_1c9a_0247 receipted host-owned (the preserved-GFX
    cache); DM2_DEALLOC_RECORD bound (word@0 = 0xffff). Gate-open with
    unknown/out-of-span GDAT word@1 fails closed before any mutation
    (the invoke branch is then unprovable). The standalone head/tail
    slices remain for their granular callers. New CTest
    `dm2_v1_delete_creature_full_pc34_compat` covers the invoke unit
    (actor mapping, field decode, distinct tickets, queue-full
    rejection) and the full flow (queued type-0x4 timer verification,
    slot-mode clear, cut, drop with chain verification, dealloc), the
    &4-set skip, the bit0-set gate skip, the fail-closed word@1 path,
    the early return, the fail-closed drop, and invalid input. PASS.
    dm2_v1 lane 222 tests, same 27 known baseline failures, zero new
    failures. Remaining: the 3CE7D cut side effects (tile-content
    analysis + actuator/sensor evaluation — needs the actuator
    subsystem), the 1c9a_0247 dballoc tag system (host-owned
    preserved-GFX cache), runtime wiring of the bound loop/callers
    into the think-binding, the per-command CCM handler bodies,
    DM2_14cd_09e2 and the table1d5f82 s_seven chain, the m_157BC
    bitmap block, and the event-driven activation callers
    (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-21 update (round 21): the 0fcb branch
    (c_1c9a.cpp:5956-5957) is now wired to the COMPLETE
    DM2_DELETE_CREATURE_RECORD composition through a session-owned
    hook — `dm2_v1_caii_set_delete_creature_full_fn` keeps the caii
    module's link boundary (the composition lives in its own
    translation unit); when wired, `dm2_v1_caii_free_slot` runs the
    composition instead of the standalone decision head (the head call
    remains as the unwired fallback; the source call is
    DM2_DELETE_CREATURE_RECORD(x, y, 0, 1)). New runtime
    session/test-support accessor `dm2_v1_runtime_caii_set_slot_mode_byte`
    mirrors the source's slot-mode writers (the 0x13 dying mode that
    gates the branch). CTest `dm2_v1_caii_free_runtime_pc34_compat`
    extended: a synthetic GDAT session (type 0x0C -> AI row 5, bit0
    clear, word@1 = 0) drives the full lifecycle through the runtime
    boundary — activation, dying-mode slot, the branch taken
    data-backed, the composition running end-to-end (invoke timer
    queued, tile-rooted cut, drop, dealloc), and no think timer
    dispatching afterwards. 18/18 PASS. dm2_v1 lane 222 tests, same
    27 known baseline failures, zero new failures. Remaining: wiring
    the hook inside dm2_v1_runtime.c itself (needs the composition
    sources added across the ~20 runtime test targets), the 3CE7D cut
    side effects (needs the actuator subsystem), the 1c9a_0247 dballoc
    tag system (host-owned preserved-GFX cache), runtime wiring of the
    bound CCM loop/callers into the think-binding, the per-command CCM
    handler bodies, DM2_14cd_09e2 and the table1d5f82 s_seven chain,
    the m_157BC bitmap block, and the event-driven activation callers
    (c_moverec.cpp:983, c_tim_proc.cpp:2887).
  - 2026-07-21 update (round 22): the 0fcb delete composition is now
    PRODUCTION-WIRED inside dm2_v1_runtime.c —
    dm2_runtime_ensure_think_binding wires the session-owned hook
    (dm2_runtime_delete_creature_full) so the runtime boundary runs the
    COMPLETE DM2_DELETE_CREATURE_RECORD composition without any
    test-side wiring.  The hook mirrors the source call
    DM2_DELETE_CREATURE_RECORD(x, y, 0, 1), uses the runtime session's
    tick counter and party accessors, a session-owned DropRng, and the
    creature module's GDAT drop-word accessors (drop slots passed only
    when the session loaded them for the creature's type).  New
    read-only accessor dm2_v1_runtime_last_delete_full_receipt exposes
    the last composition receipt.  The composition sources
    (delete_creature_full + invoke_message + drop_possession +
    dbitem_alloc) were added to all 22 remaining build targets that
    compile dm2_v1_runtime.c.  CTest dm2_v1_caii_free_runtime_pc34_compat
    now verifies the production wiring through the runtime accessor
    (18/18 PASS).  Full project rebuild clean; dm2_v1 lane 222 tests
    with 19 environment baseline failures (missing game assets — the
    merge of the sibling branches retired 8 of the former 27), zero
    failures in any CAII/delete/drop/possession test.  Remaining:
    runtime wiring of the bound CCM loop/callers into the
    think-binding, the per-command CCM handler bodies, DM2_14cd_09e2
    and the table1d5f82 s_seven chain, the m_157BC bitmap block, the
    event-driven activation callers (c_moverec.cpp:983,
    c_tim_proc.cpp:2887), the 3CE7D cut side effects (needs the
    actuator subsystem), and the 1c9a_0247 dballoc tag system
    (host-owned preserved-GFX cache).
  - 2026-07-21 update (round 23): the event-driven activation callers
    are now BOUND.  The two direct DM2_ALLOC_CAII_TO_CREATURE call sites
    are bounded slices in the CAII module:
    dm2_v1_caii_animate_activation (DM2_ANIMATE_CREATURE,
    c_tim_proc.cpp:2859-2900 — flags bit0 SET + record byte@5 == 0xff
    allocates; the CCM tail PREPARE/UNPREPARE_LOCAL_CREATURE_VAR +
    DM2_ai_13e4_0806/071b stays host-owned, receipted) and
    dm2_v1_caii_moverec_activation (DM2_moverec_3CE7D,
    c_moverec.cpp:960-985 — byte@5 != 0xff updates the pending think
    timer IN PLACE through the new dm2_v1_source_timer_update_payload
    timeline primitive (setxyA(x, y) + setmticks(map, getticks()),
    c_timer.h:66/82); byte@5 == 0xff with flags bit0 CLEAR allocates —
    the OPPOSITE gate; SET_MINION_RECENT_OPEN_DOOR_LOCATION stays
    host-owned).  Runtime wiring: the 0x04 actuator dispatch reads the
    square class through a bound tile_class_at provider and square
    class 1 runs a bounded DM2_ACTUATE_FLOOR_MECHA chain walk
    (c_tim_proc.cpp:3009-3532 + 4297-4299) whose DB3 type-0x3a records
    fire the animate activation; DB > 3 takes the source's
    whole-function return, corrupt chains fail closed bounded.  Session
    receipt via dm2_v1_runtime_floor_mecha_receipt.  New test
    dm2_v1_caii_activation_sites_pc34_compat (all checks passed);
    dm2_v1 lane 223 tests, 19 environment baseline failures (missing
    game assets), zero new failures.  Remaining: runtime wiring of the
    bound CCM loop/callers into the think-binding, the per-command CCM
    handler bodies, DM2_14cd_09e2 and the table1d5f82 s_seven chain,
    the m_157BC bitmap block, a dedicated runtime test for the
    floor-mecha dispatch (the slices themselves are covered by the
    activation-sites test), the 3CE7D cut side effects (needs the
    actuator subsystem), and the 1c9a_0247 dballoc tag system
    (host-owned preserved-GFX cache).
- DM2-004 — `skproject/SKULLWIN/c_input.cpp`, `c_keybd.cpp`, `c_tmouse.cpp`, `c_clickrect.cpp`, `c_buttons.cpp` UI event routing: `src/engine/m11_game_view.c`, `src/dm2/dm2_v1_startup_menu.c`, and `dm2_v1_inventory_panel.c` cover only bounded menu/viewport actions. The original `INTERFACE_GENERAL dt07/2` group spans are now materialized as typed primary/secondary/tail data; default door-button receipts now expose skproject `MAKE_BUTTON_CLICKABLE` rectnos 3/4 and reject custom wall-GFX buttons as non-clickable. The title-menu NEW path expands original `INTERFACE_GENERAL/0/dt04/0` rectangle `0xD7` and consumes it through M11; the hard-coded startup panel no longer accepts M11 clicks. The matching `0xD9` surface has a source-owned pointer receipt and is explicitly selector-unavailable, so it cannot fall through into a synthetic resume row. The title/menu indexed presentation now expands `dtPalIRGB`'s source 6-bit DAC channels to SDL's 8-bit RGBA after `DM2_CONVERT_DRIVERPALETTE`, while retaining raw GDAT palette bytes for receipts. Bind the original resume-selector state machine before it can create a resume action. Consume the remaining original click-rectangle, keyboard, mouse, held-button, and modal-dialog ordering. Unsupported controls must remain unavailable.
- DM2-004 — `skproject/SKULLWIN/c_input.cpp`, `c_keybd.cpp`, `c_tmouse.cpp`, `c_clickrect.cpp`, and `c_buttons.cpp` UI event routing: `src/engine/m11_game_view.c`, `src/dm2/dm2_v1_startup_menu.c`, and `dm2_v1_inventory_panel.c` cover only bounded menu/viewport actions. The original `INTERFACE_GENERAL dt07/2` group spans are now materialized as typed primary/secondary/tail data; default door-button receipts now expose skproject `MAKE_BUTTON_CLICKABLE` rectnos 3/4 and reject custom wall-GFX buttons as non-clickable. The title-menu NEW path expands original `INTERFACE_GENERAL/0/dt04/0` rectangle `0xD7` and consumes it through M11; the hard-coded startup panel no longer accepts M11 clicks. The matching `0xD9` surface has a source-owned pointer receipt and is explicitly selector-unavailable, so it cannot fall through into a synthetic resume row. The title/menu indexed presentation now expands `dtPalIRGB`'s source 6-bit DAC channels to SDL's 8-bit RGBA after `DM2_CONVERT_DRIVERPALETTE`, while retaining raw GDAT palette bytes for receipts. Bind the original resume-selector state machine before it can create a resume action. Consume the remaining original click-rectangle, keyboard, mouse, held-button, and modal-dialog ordering. Unsupported controls must remain unavailable.
  - 2026-07-15 verification: the M11 logical-window FIT/content inverse now
    has a real-GDAT `0xD7` test through the actual startup state route. It
    reaches the source NEW surface and emits the existing GAME_LOAD boundary;
     no coordinate remap or synthetic menu action was added.
- DM2-005 — `skproject/SKULLWIN/c_creature.cpp` `DM2_PROCEED_CCM`, `DM2_CREATURE_ATTACKS_PARTY`, and `DM2_CREATURE_CAST_SPELL`: `src/dm2/dm2_v1_ccm.c` implements a small opcode subset and returns `UNKNOWN_OPCODE` for the remaining program. `EXTENDED_LOAD_AI_DEFINITION` proves only `CREATURE_AI` `dtWordValue` fields 0–35 as AIDefinition data; it does not prove any adjacent CCM bytecode field, so boot field probing is fail-closed and no decoded candidate can reach runtime. Decode and execute the complete CCM instruction/data contract only after its original stream owner/grammar is proven, including control flow, creature state, spells, and summon branches.
  - 2026-07-18 update: the DM2_PROCEED_CCM compare chain
    (c_creature.cpp:2930-3212) is now bound verbatim as
    `dm2_v1_ccm_dispatch_pc34_compat`: every b_1a byte maps to its source
    handler group or NONE, table1d613a (mdata.c:1615-1639) is bound with
    fail-closed out-of-span reads, and the gametick writeback gate
    (flags & 3) is exposed. CTest `dm2_v1_ccm_dispatch_pc34_compat` PASS.
    Note: the legacy `dm2_v1_ccm.c` opcode numbering diverges from the
    source b_1a mapping (e.g. source 0x17 = PLACE_MERCHANDISE, 0x27/0x28
    = CAST_SPELL); aligning the interpreter subset to the source matrix
    is follow-up work. Remaining: prove the CCM stream owner/grammar,
    execute handler bodies beyond receipts, per-cell DM2_THINK_CREATURE
    binding over the DM2-002 record pool.
  - 2026-07-19 update: the legacy interpreter subset is now aligned to
    the source b_1a matrix. `dm2_v1_ccm.c` opcode values ARE the source
    command bytes (c_creature.cpp:2930-3212): 0x01/0x02/0x09 WALK_NOW,
    0x03/0x04 CCM03, 0x05 JUMPS, 0x06/0x07 CCM06, 0x08/0x26
    ATTACKS_PARTY, 0x0A STEAL_FROM_CHAMPION, 0x0E/0x0F SHOOT_ITEM,
    0x13 KILL_ON_TIMER_POSITION, 0x15/0x16 ROTATES_TARGET_CREATURE,
    0x17 PLACE_MERCHANDISE, 0x18 TAKE_MERCHANDISE,
    0x19/0x29/0x2A/0x2D/0x2E PUTS_DOWN_ITEM, 0x1A/0x2B/0x2C TAKES_ITEM,
    0x27/0x28 CAST_SPELL, 0x3D-0x40 EXPLODE_OR_SUMMON. Every table row
    carries its DM2_V1_CcmSourceHandler group; source "no branch taken"
    bytes (0x00, 0x10-0x12, 0x14, 0x1B-0x25, 0x32-0x34, 0x41-0x54,
    0x56-0xFE) and unproven handler bodies (CCM0B 0x0B, CCM0C 0x0C/0x0D,
    ACTIVATES_WALL 0x2F-0x31, USES_LADDER_HOLE 0x35-0x3A, TRANSFORM
    0x3B/0x3C, DM2_1B7D5 0x55) stay fail-closed UNKNOWN_OPCODE. The
    diverged legacy macros in `dm2_v1_creature.h` (DM2_CCM_*) and the
    creature runtime bridge + combat probe assertions are re-based on
    the same matrix. New CTest `dm2_v1_ccm_source_alignment_pc34_compat`
    PASS cross-checks every legacy row against
    `dm2_v1_ccm_dispatch_source_group` so the modules cannot drift;
    `dm2_v1_ccm_pc34_compat` and `dm2_v1_creature_ccm_runtime_pc34_compat`
    re-based and PASS. dm2_v1 lane 197 tests, same 27 known baseline
    failures, zero new failures. Remaining: prove the CCM stream
    owner/grammar, execute handler bodies beyond receipts (the per-cell
    DM2_THINK_CREATURE binding over the DM2-002 record pool landed
    2026-07-19 as `dm2_v1_think_creature_pc34_compat`; its think body
    plugs into that module's DM2_V1_ThinkCreatureBody boundary once the
    stream owner/grammar is proven).
  - 2026-07-15 update: `DRAW_MAP_CHIP` takes a concrete record link, dereferences
    DB4 `Creature`, and only then reads AI animation state. Firestaff's local
    CCM instances have no source-owned DB4 handle, so the former `source_kind=1`
    viewport path is removed. They advance simulation only; neither GDAT nor
    fallback art may be emitted. Remaining: bind a live CCM instance to the
    exact G1 DB4 record/AI state before admitting a dynamic creature sprite.
  - 2026-07-15 update: direct DB4 map-chip receipts now take creature facing
    from `DME.h::Creature::b15_0_1`, matching `DRAW_MAP_CHIP`; ObjectID
    direction bits are no longer substituted for atlas selection. Remaining:
    bind authentic mutable command state before dynamic frame selection.
- DM2-006 — `skproject/SKWIN/c_creature.cpp` AI/death paths and `c_ai.cpp`: the bounded real-data chain `CREATURES[type] dtWordValue(0x05) -> CREATURE_AI row -> AIDefinition.w30/w32` is available as evidence for `DRAW_PUT_DOWN_ITEM`; it preserves the source w30 eligibility gate and still does not create a click target until owner records and rect expansion are both proven. Bind real GDAT AI records and reproduce source eligibility, possession, death, and cooldown ordering.
  - 2026-07-23 update (Lane E, cycle 16): the real-data drop route is now
    public and proven against the local canonical GRAPHICS.DAT:
    `dm2_v1_drops_resolve_gdat_creature_drops()` reads CREATURES word
    fields 0x0A..0x14 straight from a verified loader and resolves them in
    source order (GLOP/24: 0x8E10+0x9D10 → items 284/314; ATTACK MINION/14:
    0x0410+0x8412 → items 8/264; TREE/0: 0x9241 → item 292), and the death
    path drops the real GDAT item through the observer.  Melee resolution
    gained a real-data defense route: `dm2_v1_combat_bind_creature_defense_fn()`
    + `dm2_v1_combat_resolve_attack_on_creature()` consume the AIDefinition
    Defense byte @8 through the new data-backed
    `dm2_v1_creature_ai_defense()` accessor and reject explicitly when the
    defense is unproven — which is the case for the local PC English GDAT
    (no CREATURE_AI/0x19 category), so combat damage stays fail-closed
    locally. Remaining: a CREATURE_AI-proven graphics session to light
    up the defense/BaseHP route, DUNGEON.DAT door-record evidence for the
    door-destruction table, and ALLOC_NEW_DBITEM item-record creation.
  - 2026-07-18 update: `dm2_v1_drops_resolve_source_slots` now binds
    DROP_CREATURE_POSSESSION's generated-drops loop
    (skcrture.cpp:2084-2118): CREATURES fields 0x0A..0x14 in ascending
    slot order, word 0 skipped, base=(w&15)+1, extra=(w&0x70)>>4, count
    += RAND16(extra+1), item = w>>7, over the source LCG
    (c_random.cpp:13-31). The GDAT AI-table loader imports CREATURES drop
    words per type; the death path resolves them source-ordered and
    receipts the result in the death-drop observer. The fixed Thorn Demon
    fallback is preserved for data-free sessions. CTest
    `dm2_v1_drops_source_order_pc34_compat` PASS. Remaining: bind
    ALLOC_NEW_DBITEM + MOVE_RECORD_TO so resolved drops become real DB
    item records (with RAND01/RAND02 direction draws), the possession
    chain walk (skcrture.cpp:2120+), and source cooldown/eligibility
    ordering.
  - 2026-07-19 update: ALLOC_NEW_DBITEM + the bounded from-nowhere
    MOVE_RECORD_TO are now bound, so resolved drops become real DB item
    records. New module `dm2_v1_dbitem_alloc_pc34_compat` mirrors
    c_record.cpp:367-401 GET_ITEMDB_OF_ITEMSPEC_ACTUATOR (itemspec &
    0x1ff, groups 0/1/2 -> dbWeapon/dbCloth/dbMisc, group 3 split
    0x1fc scroll / >=0x1e0 container / >=0x1b0 creature / else potion,
    >0x1fc invalid), c_record.cpp:403-444 GET_ITEMTYPE_OF_ITEMSPEC_
    ACTUATOR, c_record.cpp:1076-1139 ALLOC_NEW_RECORD (forward scan for
    w0 == OBJECT_NULL, zero + OBJECT_END_MARKER init, dbContainer w2,
    bones 0x800A without the dbMisc 3-record reserve; the source's
    RECYCLE_A_RECORD_FROM_THE_WORLD fallback stays unproven so an
    exhausted pool returns OBJECT_NULL fail-closed), c_record.cpp:284-345
    SET_ITEMTYPE (db5/6/10 word@2 low 7 bits, db8 word@2 high 7 bits,
    db9 container charge split over word@4 with the (w&6)==2 word@6
    mark, db4 byte@4, db7 scroll no-op), and c_record.cpp:1142-1165
    ALLOC_NEW_DBITEM itself. `dm2_v1_drops_place_source_slots` binds the
    c_record.cpp:1537-1634 generated-drops loop with the source's
    interleaved RNG order (slot count roll, then that slot's per-item
    direction draws): OBJECT_NULL breaks the slot loop BEFORE the
    direction draw, the party cell draws (party_dir + RANDBIT) & 3 and
    other cells draw RANDDIR, and the direction folds into the record
    word (dir << 14 | handle & 0x3fff) before the bounded append to a
    caller-owned destination list (tile-rooted ground-stack mutation
    stays unproven). New CTest `dm2_v1_dbitem_alloc_pc34_compat` PASS
    (itemspec mapping, pool scan/reserve/bones/exhaustion, per-DB
    SET_ITEMTYPE, interleaved RNG cross-check against a reference LCG,
    party-cell rule, OBJECT_NULL break without a draw, source-ordered
    ground chain). dm2_v1 lane 200 tests, same 27 known baseline
    failures, zero new failures. Remaining: the possession chain walk
    (c_record.cpp:1640+, missile dealloc + AI-spec direction gate),
    tile-rooted ground-stack mutation, runtime wiring of the death path
    to a session-owned pool set, and source cooldown/eligibility
    ordering.
  - 2026-07-22 update: `dm2_v1_creature_load_ai_table_from_gdat` now
    admits both the source word-value path
    (SkWinCore.cpp:233-400 EXTENDED_LOAD_AI_DEFINITION, 36 individual
    `dtWordValue` fields per CREATURE_AI row) and a bounded raw-record
    fallback for synthetic fixtures that materialise a 36-byte
    AIDefinition block under `CREATURE_AI` (DME.h:1505-1545 layout).
    The fallback only fills rows the word-value path did not already
    load, never overrides a proven GDAT session, and sets the
    creature-type-to-AI-row indirection so `dm2_v1_creature_ai_spec`,
    `dm2_v1_creature_attacks_party`, and `dm2_v1_creature_resolves_spell`
    work for imported rows. CTest `test_dm2_v1_creature_gdat_ai_table`
    now passes 13/13. dm2_v1 lane 205 tests, same known baseline
    failures, zero new failures. Remaining: possession chain walk,
    tile-rooted ground-stack mutation, runtime death-pool wiring,
    source cooldown/eligibility ordering.
- DM2-007 — `skproject/SKULLWIN/c_events.cpp` `DM2_TRY_CAST_SPELL`, `DM2_FIND_SPELL_BY_RUNES`, `DM2_CAST_SPELL_PLAYER`, and `DM2_PROCEED_SPELL_FAILURE`: `EXTENDED_LOAD_SPELLS_DEFINITION` is a bounded GDAT `SPELL_DEF` receipt over exact dtWordValue fields 1-7 plus dtText field `0x18`. The fixed original table and live rune lookup are now source-exact, but DB object-effect resolution, projectile creation, timer effects and final UI feedback remain unbound. Unsupported object effects must remain unavailable rather than use a spell-index mapping.
  - 2026-07-31 update: the fixed 34-record route now copies the exact
    `dSpellsTable` bytes from `SKWIN/SkGlobal.cpp:968-1007`: raw rune
    codes (`0x66..0x77`), difficulty, required-skill byte and packed `w6`.
    Live fixed-table lookup takes the source power rune plus its one-to-three
    rune tail, with power derived as `rune[0] - 0x5f` and cost/cooldown from
    `w6`; the former ordinal runes and reconstructed mana/cooldown fallback
    are gone. `test_dm2_v1_spell_pc34_compat` checks every source record.
  - 2026-07-18 update: the DM2_FIND_SPELL_BY_RUNES contract
    (c_events.cpp:2211-2264) is now bound in `dm2_v1_spell.c`: source
    query-key packing (rune[0]<<24...rune[3], zero-terminated, max four
    runes), reverse table scan, 24-bit masked compare for
    top-byte-zero records (power rune stripped), full 32-bit compare for
    exact-power-locked records, plus the record mana formula
    ((w6>>10)&0x3f)*(cast_power+0x12)/0x18 (c_events.cpp:2282-2289) and
    the DM2_PROCEED_SPELL_FAILURE classes 0x10/0x20/0x30 with the
    TRY_CAST_SPELL rune-clear/panel rule (c_events.cpp:2687-2786).
    DM2_UPDATE_GLOB_VAR / v1e0b6c stay receipted-pending. CTest
    `dm2_v1_spell_rune_lookup_pc34_compat` PASS. Remaining: bind live
    hero rune strings to validated GDAT SPELL_DEF records, resource
    spending, flask/missile/summon execution branches
    (DM2_CAST_SPELL_PLAYER cases 0-3), and timer effects.
  - 2026-07-23 update: DM2-007 runtime spell cast slice is now bound in
    `dm2_v1_spell_cast_player.c`.  It builds a unified runtime table from
    the fixed 34-spell `dSpellsTable` and the bounded GDAT `SPELL_DEF`
    receipt, performs source-ordered reverse lookup with the power-locked
    vs. power-stripped key compare, computes mana cost via the existing
    `dm2_v1_spell_record_mana_cost` formula, applies the source cast-chance
    math (`bp08 = difficulty + power`, `bp0c = skill + 15 - bp08`),
    classifies execution into POTION/MISSILE/GENERAL/SUMMON, gates potion
    casts on an empty flask, and emits bounded timer-effect requests (light,
    aura, enchantment, cloud, summon, projectile) without mutating champion
    state, creating objects, or queuing timers.  CTest
    `dm2_v1_spell_cast_player_pc34_compat` PASS (49/49).  Remaining:
    wire the receipt into live champion/UI state, consume mana and flasks,
    instantiate missiles and summon creatures, apply timer payloads, and
    hook the failure classes into M11 feedback.
  - 2026-07-23 update (Lane B, cycle 10): DM2-007 champion/UI state writeback
    slice is now bound in `dm2_v1_spell_cast_player.c`.  New
    `dm2_v1_spell_cast_player_apply()` takes a populated
    `DM2_V1_SpellCastPlayerReceipt` and mutates a `DM2_ChampionRecord` on
    success: mana is reduced by the receipt's computed cost, the casting hand
    receives `cooldown_ticks`, the rune tail is cleared, and a provided empty
    flask object (via `DM2_LeaderPossession`) is consumed for POTION casts.  On
    failure the rune tail is cleared only when the failure receipt says so
    (class 0x30 keeps runes so the player can supply a flask), and a failure
    feedback flag is set for M11/UI.  Successful non-potion casts optionally
    enqueue a source-order timer request on a caller-provided
    `DM2_V1_SourceTimerQueue`: LIGHT (0x46), AURA/ENCHANTMENT (0x47), CLOUD
    (0x19), SUMMON (0x5e placeholder), and PROJECTILE (0x1e), carrying the
    source-derived duration, champion actor, map id, and party cell.  CTest
    `test_dm2_v1_spell_cast_player_pc34_compat` grows to 86/86 checks.
  - 2026-07-23 update (Lane B, cycle 11): DM2-007 spell-effect timer handler
    bodies are now bound in `dm2_v1_spell_timer_handlers_pc34_compat.c`.
    Proven handlers: `0x46 DM2_PROCESS_TIMER_LIGHT` (requeues while remaining
    duration > 0), `0x47` hero enchantment flag set/clear, `0x48` enchantment
    power decay, and `0x4b` poison tick.  `0x19` cloud, `0x1e` missile step,
    and `0x5e` summon remain fail-closed until their DB-record owners are
    proven.  `test_dm2_v1_spell_cast_player_pc34_compat` grows to 110/110
    checks; lane-relevant tests `dm2_v1_proceed_timers_pc34_compat`,
    `dm2_v1_spell_rune_lookup_pc34_compat`, and `dm2_v1_spell_pc34_compat`
    all pass.  Remaining: instantiate real missile DB records / flying items,
    create summon creature records, implement the cloud handler, wire the
    handlers into live `dm2_v1_runtime.c` timer dispatch, and route failure
    feedback through M11's DM2 status scope.
- DM2-008 — `skproject/SKULLWIN/c_sound.cpp` `DM2_PLAY_MUSIC`, `DM2_PLAY_SOUND`, `DM2_QUERY_SND_ENTRY_INDEX` and `c_sfx.cpp` queueing: **cycle 16 (Lane B) update:** voice allocation, PCM decode, and a real SDL3 playback backend are now implemented behind the fail-closed contract — `dm2_v1_sound_decode_gdat_pcm()` decodes verified GDAT sound raw entries (payload ^ 0x80, 6000 Hz U8 mono per SKWIN/SkwinSDL.cpp), 16 MAX_SB voices allocate/free without stealing, `src/dm2/dm2_v1_sound_sdl_backend.c` mixes them through a real SDL3 stream, and attenuation is the source R_928 metric only. Playback is audible only when the sample decodes from a verified GDAT entry and the backend reports ready; the title music cue stays fail-closed because no verified music asset root is proven locally. Remaining: a verified music asset root and proven wall-occlusion/facing routing for positional cues.
  - 2026-08-06 update: M11 binds the SDL backend only after
    `dm2_v1_boot_startup_launch_alloc_with_language()` succeeds, then
    unbinds it in `M11_GameView_Shutdown()`. The real-data M11 startup gate
    now covers both sides, preventing a verified DM2 backend from leaking
    into an unverified or later game launch.
  - 2026-07-31 update: `dm2_v1_sound_stop_music()` now closes the MIDI
    backend and clears the admitted event/loop schedule. A completed stop
    cannot leave an old source stream schedulable.
  - 2026-07-23 update: **cycle 14 (Lane B):** the source-locked paths now live in `src/dm2/dm2_v1_sound.c` — a verified GDAT loader binds via `dm2_v1_sound_bind_gdat_loader()`, `DM2_SOUND9` populates the `dm2sound.xsndptr2` seven-byte runtime queue with GDAT-resolved sample bindings, and `DM2_QUERY_SND_ENTRY_INDEX` keeps the original 1-based scan with a GDAT fallback in original queue/query order.  Unavailable audio is explicit (no synthesized attenuation or playback).  Remaining: voice allocation, decoding, and a proven SDL playback backend plus verified music asset root before audible playback leaves the fail-closed state.
  - 2026-07-14 update: without the original runtime `xsndptr2` queue and its
    resolved payload, direct and positional playback now report unavailable.
    Firestaff no longer treats an arbitrary SFX identifier as a GDAT result or
    synthesizes attenuation and successful playback. Remaining: bind the
    source `DM2_SOUND9` queue mutations and a verified sample backend.
  - 2026-07-18 update: `dm2_v1_sound_queue_pc34_compat` now owns the original
    queue/query/change-detection order: `DM2_SOUND9` seven-byte `s_ssound`
    runtime queue mutation (c_sound.cpp:650-662), 1-based
    `DM2_QUERY_SND_ENTRY_INDEX` (c_sound.cpp:664-673), `DM2_QUEUE_NOISE_GEN1`
    gates/rotation/`R_1FB7D` occlusion clamp/duplicate suppression
    (c_sfx.cpp:138-331), `DM2_QUEUE_NOISE_GEN2` remap (c_sfx.cpp:334-345),
    exact `R_928` metric incl. negative-x branches (c_sound.cpp:256-308),
    `R_8FE` precedence, `DM2_PLAY_SOUND` permutation bubble sort and 64-slot
    free-sample scan with whole-pass early return (c_sound.cpp:342-434), and
    `DM2_SOUND8` flush (c_sound.cpp:633-647). Unproven samples reject, the
    delayed path receipts its type-0x15 timer pending instead of simulating
    `DM2_QUEUE_TIMER`, and playback stays explicitly unavailable without
    mutating the slot table. CTest `dm2_v1_sound_queue_pc34_compat` PASS.
    Remaining: a verified sample backend (`do_sound`, c_sfx.cpp:47-77), the
    source's secondary `s54p_00->s54p_00` duplicate comparison once
    sample-record ownership is proven, and the `DM2_PROCESS_SOUND` delayed
    release once the type-0x15 timer binding exists.
- DM2-009 — `skproject/SKULLWIN/c_savegame.cpp` `DM2_SELECT_LOAD_GAME` and restore flow: source save candidates must not enter world state until complete original payload restoration and post-load rebuild are implemented. `D2RS` is diagnostic-only and rejected by all player-facing admission paths. The hash-revalidated original-save corpus census receipts only decoded `GAME_LOAD` facts (tick, RNG, party pose/map, champion/timer counts, rain intensity, and the source 263-byte champion records); it is not restore input and assigns no DB, timer-payload, possession, or inventory semantics. The raw prefix validates and receipts source-owned descriptor, column-index, ground-stack, text, DB-pool, and map-data spans; map-data length is `savegamep4->warr_00[1]` exactly, never a derived descriptor size. Remaining: SKProject-equivalent record allocation/ownership, full DB and timer materialisation, original read/write, map state, and post-load rebuild order.
  - 2026-07-15 update: Firestaff live-sidecar restore now validates its full
    creature payload before `GAME_LOAD`-owned session publication, then
    restores the source-owned G1 bytes and refreshes derived scene controls.
    A wire-valid session whose SKProject timer-owner reconstruction rejects is
    covered as a non-mutating failure: live CCM and dungeon bytes remain
    unchanged. This changes no original raw corpus bytes or provenance.
  - 2026-07-15 update: original raw-SKSave restore now runs the same
    `GAME_LOAD` session/timer-owner preflight before it swaps parsed G1 or
    clears the Firestaff-only CCM cache. A decoded `tty0C` timer whose actor
    is outside the saved squad rejects without changing the live party,
    dungeon, or creature pool. Remaining work is still corpus-verified full
    DB/timer materialisation and source-complete post-load rebuild order.
  - 2026-07-18 update: `dm2_v1_save_timers_pc34_compat` now materialises the
    timer payload in the source's exact `GAME_LOAD` order
    (c_savegame.cpp:1517-1527): per-record SUPPRESS decode through the
    verified `v1d6463 = vsgame+0x00` 12-byte mask (dm2data.cpp:97-99,
    dm2data.h:608) over the 12-byte `c_tim` wire layout (c_timer.h:8-46),
    `clrtype()` for `[num_timers, max_timers)`, the `DM2_SORT_TIMERS`
    identity+heapify order with `DM2_cmp_timers`' full tiebreak chain
    (c_timer.cpp:31-48, 126-194), and the `DM2_REARRANGE_TIMERLIST`
    free-chain rebuild (c_timer.cpp:97-122). Underflow decodes into scratch
    and leaves caller state untouched; `dummya` is never restored. This
    proves the saved timer-record byte layout half of SKPROJECT-GAP-001.
    CTest `dm2_v1_save_timers_pc34_compat` PASS. Remaining: corpus-verified
    full DB-record materialisation and the post-load
    `DM2_READ_SKSAVE_DUNGEON` / `DM2_PROCEED_GLOBAL_EFFECT_TIMERS` rebuild
    order (receipted pending).
  - 2026-07-15 update: each DB pool and the `warr_00[1]` map span now retain
    their exact source-order byte offsets. This is address evidence only;
    unknown records, links, timer payloads, and rebuild semantics stay blocked.
  - 2026-07-15 update: callers can now obtain one hash-bound raw DB record
    receipt only by `(pool,index)` after the complete original prefix validates.
    It exposes pool/record size and source offset, never decoded fields or
    links. Full DB semantics and post-load reconstruction remain open.
  - 2026-07-15 update: DB0 `Door::w2` is now decoded from that receipt with
    only skproject's documented button/type/state/opening/ornate/damage bits.
    `w0`, tile attachment, links, and all other DB families remain blocked.
  - 2026-07-15 update: DB3 `Actuator::w2/w4/w6` is now decoded only through
    its hash-bound raw record receipt. It preserves documented control and
    target fields, but executes no actuator and follows no link.
  - 2026-07-15 update: DB4 `Creature::b4/w6` now has a corpus receipt for
    source type and first HP word only. Possession `w2`, further HP words,
    animation state, AI, links, and tile ownership remain unavailable.
  - 2026-07-15 update: DB5 `Weapon::w2` now has a corpus receipt for source
    item type, important flag, and charges only. `w0`, item location, links,
    and any GDAT visual promotion remain blocked until a real source material
    route proves them.
  - 2026-07-15 update: raw runtime handoff now validates every map-reachable
    `GenericRecord::w0` chain after the receipt-proven pool layout has parsed.
    A thing-bearing square without a bounded ground-stack root, invalid
    ObjectID, cycle, or nonterminating link rejects before the atomic dungeon
    swap. Unused pool slots remain opaque allocation state. Remaining: real
    corpus coverage for valid linked record families and the source post-load
    rebuild owners.
  - 2026-07-15 update: an admitted raw layout now carries its map/pool/hash
    identity through DM2 runtime into the first V1/M10 frame receipt. The
    frame marks it consumed only while the active dungeon bytes and saved party
    pose still match; rejected candidates retain the previous receipt/model.
    Remaining: authentic multi-variant corpus captures and source-complete
    post-load record/timer materialization.
  - 2026-07-15 update: a selected original corpus census row now has its own
    `DM2_SELECT_LOAD_GAME`-shaped runtime handoff. It rescans and completely
    reparses the original-only census, admits only the exact selected
    envelope/raw identity and decoded state hash, then reuses the existing
    complete-file-hash restore boundary. A stale row is rejected even when a
    valid Firestaff session remains in the same directory; this route has no
    first-importable or synthetic-save fallback. Remaining: real multi-save
    corpus coverage plus full DB/timer materialisation and post-load order.
  - 2026-07-15 update: selected raw rows now carry the independently parsed
    map count, all 16 DB-pool record counts, and dungeon-prefix/map-data FNV
    identities through the original-only corpus handoff. The runtime rejects
    a raw row without this exact layout receipt before restore; this admits no
    DB links, record behavior, or graphics semantics. Remaining: positive
    multi-save corpus breadth and source-complete DB/timer materialisation.
  - 2026-07-15 update: raw original-save DB5 Weapon, DB6 Cloth, DB7 Scroll,
    and DB10 Miscellaneous_item records now expose only their common
    SKProject `DME.h::ItemType()` owner, `w2` bits 0..6, through a
    hash-bound record receipt. The parser verifies exact pool/index/stride
    boundaries before reading the field and rejects DB9 or any absent row;
    GenericRecord links, item contents, charges outside DB5, and all visual
    consumers remain unowned. Remaining work is real corpus breadth and the
    source-complete DB graph/post-load order.
  - 2026-07-15 update: raw DB2 Text and DB9 Container rows now have separate
    hash-bound SKProject field receipts. DB2 exposes only `w2` visibility,
    mode, and 13-bit text index; DB9 exposes only `b4` open/type. Both reject
    absent rows before runtime import, while `GenericRecord::w0`, text-table
    decoding, contained-object `w2`, record chains, and any visual route stay
    explicitly unavailable. Remaining work is positive real-corpus record
    graph evidence and source-complete post-load ordering.
  - 2026-07-15 update: raw DB1 Teleporter rows now expose their verified
    `DME.h` `w2/w4` destination, scope, sound, rotation, and rotation-type
    fields through a six-byte hash-bound receipt. The parser rejects a missing
    row and does not read `GenericRecord::w0`, invoke movement, mutate maps,
    or infer a record chain. Remaining work is positive original-corpus link
    and runtime-dispatch evidence before any teleporter action can be used.
- DM2-010 — `skproject/SKULLWIN/c_gui_vp.cpp` `DM2_DRAW_WALL`, `DM2_DRAW_DOOR`, `DM2_DRAW_DOOR_FRAMES`, and `DM2_DRAW_DUNGEON_TILES`, plus `c_gfx_blit.cpp`/`c_gfx_stretch.cpp`: `src/dm2/dm2_v1_viewport_renderer.c` still permits fallback rectangles/colours. Door panel, frame, button, ornament, destroyed-mask, dynamic champion HUD pixels, walls, planes, and map chips now consume the exact 16-byte IMG3 local palette returned by `QUERY_GDAT_IMAGE_LOCALPAL`; a source-owned runtime image is blocked when that palette cannot be proven. Original `dt07/0x0A` Rect14 metadata reaches the host receipt and gates runtime viewport consumption. Champion names consume boot-owned `INTERFACE_GENERAL dt07/0`, while portraits and three status bars consume expanded original `dt04/0` IDs 173–176, 165–168, and 185–204. Direct G1 DB5/DB9 F9 map-chip receipts are retained only as authenticated `DRAW_MAP_CHIP` evidence; the viewport rejects them as static-object pixels until the separate `DRAW_ITEM` F0/F4 geometry and raw receipts are complete. D0C and all side/deep routes are unavailable instead of using Firestaff's old generic projection. `SUMMARIZE_STONE_ROOM` calls the original random-decoration helper before it populates ornament details; Firestaff has no proven decoration table/seed contract, so that stage remains unavailable rather than randomized. Remaining: complete source cell ordering, `DRAW_ITEM` clipping/placement, door states beyond source-locked closed-panel/button placement, object/creature/cloud passes, scale/flip rules, and verified GDAT material.
  2026-08-06: a source-scheduled door now rejects an unresolved original
  `RAW4` panel or button rectangle instead of falling through to the historic
  hard-coded rectangle. The compact rectangle table remains limited to
  diagnostic/non-source consumers; real GDAT/scene sessions are no-draw
  until their selected source owner is available.
  2026-08-06: source-scheduled teleporter fields likewise reject the compact
  placement table. SKProject `DRAW_TELEPORTER_TILE` selects a per-cell RAW4
  rectangle through `tblGraphicsTeleporterWords` and applies
  `tblGraphicsTeleporterBytes4` copy/flip and offsets; full command
  materialization remains required before those fields can be drawn.
  2026-08-06: Greatstone's PC English catalogue now proves decoded pixels for
  all 4,031 IMG3/IMG9/IMG11 rasters. FNT1 0203 remains a genuine font record
  pending source font rasterisation; do not substitute generated glyphs.
  2026-08-06: an admitted DB5/DB9 root now carries the matching
  `INTERFACE_GENERAL dt07/0x0A` Rect14 placement from its verified
  `DRAW_STATIC_OBJECT` cell/pass/clip plan into the final item blit. Source
  scale, lateral offset and mirror flag now replace the former host
  frame-index lookup. A mismatched Rect14 image field stays outside that join;
  unsupported item classes and missing source plans remain no-draw.
  - 2026-07-16 update: `DRAW_STATIC_OBJECT -> DRAW_PUT_DOWN_ITEM -> DRAW_ITEM` now has a bounded DB5/DB9 source-plan receipt for D1C/D2C: source 5x5 direction position, F0/F4 image field, `QUERY_CREATURE_BLIT_RECTI` key, distance stretch, slot delta, and container mirror. This corrected the earlier F9 conflation: DB5/DB9 F9 is `DRAW_MAP_CHIP`, not static-object material, and is now deliberately no-draw in this renderer path. Remaining before a positive static-object blit: the source 5x5 visibility mask, record-list ordinal, expanded clipping rectangle, `dtImageOffset`, and exact F0/F4 raw image/local-palette receipts through M11.
  - 2026-07-17 update: the bounded D1C/D2C static DB5/F0 and DB9/F0/F4 route now carries `dtImageOffset`, GFX256 raw bytes/receipt, IMG3 palette and RAW4 clip receipt through M11; frame-plan consumption requires all identities to match. F9 and unsupported cells remain no-draw. Remaining: source 5x5 visibility mask and record-list ordinal for wider static-object coverage.
  - 2026-07-17 update: DB5/F0 and DB9/F0/F4 now additionally have a separate
    source-owned M11 delivery plan. It invalidates on session, selector/data,
    source cell/pass/clip, GFX256, IMG3, or RAW4 drift and remains no-draw
    until original pixel-decoder evidence exists. The earlier direct blit
    fixture and all F9/map-chip routes are not delivery-plan owners.
  - 2026-07-17 update: `QUERY_CREATURE_BLIT_RECTI` now has an exact SKWIN
    5x5-rotation/RAW4 rect-id receipt, and the bounded DB14 flying-item source
    route consumes its `dir=0` result. DB14 now also accepts a timer direction
    only from the matching raw ten-byte `DME.h::Timer` row (`w8` bits 10--11),
    hash-bound to the DB14 timer index. A session adapter now reads only the
    original contiguous timer table and rejects an index outside its saved
    count. Runtime can now retain a complete identity-matched DB14 receipt but
    marks it no-draw. A separate viewport/M11 delivery plan now retains the
    exact source rect/rotation and GFX256/IMG3/RAW4/timer identities while
    explicitly denying pixels. Remaining: an original pixel-decoder trace and
    source-proven live viewport enumeration; it never reuses a legacy
    projectile/map-chip material.
  - 2026-07-17 update: the DM2-owned M11 receipt gate now composes complete
    scene/light, wall, door, weather, DB5/DB9 static-object, and DB14
    flying-item source plans under one nonzero session/data epoch and fixed
    source order. Any absent or drifted member rejects; the resulting receipt
    remains no-draw until authentic per-pixel decoder evidence exists.
    Remaining: source-proven live plan enumeration and the original pixel
    decoder, not a generic M11 or map-chip fallback.
  - 2026-07-17 update: live plan enumeration now binds that gate to a valid,
    serialized original `DM2_V1_SessionState` and its G1 map token. Active
    GRAPHICSSET must agree across the G1/GDAT scene, light, wall, and weather
    plans; static material must carry the same session identity. Session/data
    drift, missing plans, and GDAT hash drift reject deterministically. The
    receipt remains no-draw. Remaining: runtime-owned population of each
    optional DB5/DB9 and DB14 plan from the current viewport record scan, then
    authentic pixel-decoder evidence.
  - 2026-07-17 update: DB14 now resolves `Missile::missile_object` through
    the exact `DM2_DRAW_FLYING_ITEM` class1/class2 chain from
    `SKULLWIN/c_gui_vp.cpp:3545-3770`. The `class1==0x0d` receipt requires
    DB14 byte+4 not to be `0xff` and the real GDAT `(0x0d,class2,0x0b,1)`
    data-index query; non-0x0d remains an opaque `DM2_DRAW_ITEM` branch.
    Remaining: source table proof that converts this data-index and viewport
    state into the exact `8/9/10/12` image field, before any source receipt
    or plan can consume it.
  - 2026-07-17 update: the bounded `c_gui_vp.cpp:3610-3745` branch matrix
    now resolves `vb30` only from source-owned query state, timer direction,
    viewport direction, 5x5 direction, and validated viewport-table inputs.
    The `class1==0x0d` route then calls the existing
    `QUERY_GDAT_SUMMARY_IMAGE` receipt with exactly `(class1,class2,vb30)`;
    an IMG3 metadata/local-palette receipt is required. All unknown branch
    values, unavailable table rows, palette-less images, and the source's
    blocked TEMP_PICST case reject. Remaining: bind authenticated decoded
    pixels and all runtime receipt identities before any DB14 plan can draw.
  - 2026-07-17 update: `dm2_v1_g1_flying_item_decoded_material_receipt` now
    binds that exact tuple to `dm2_v1_gdat_image_raw_material_receipt()` and
    `dm2_v1_asset_load_image_field()`; the raw GFX256, decoded IMG3, and
    local-palette hashes must agree with the SUMMARY_IMAGE receipt. A separate
    DM2 runtime plan adds live session/map/timer plus selector/vb30/table
    drift invalidation. Decoded bytes are discarded after hashing and the
    resulting plan is `no_draw`. Remaining: source proof for an authentic
    frame consumer before any DB14 pixel delivery.
  - 2026-07-17 update: a DM2-owned M11 material consumer now re-decodes and
    hash-checks the exact indexed IMG3 bytes, local palette, clip id, flip
    fact, and `PUT_DOWN_ITEM -> creature -> DRAW_FLYING_ITEM` source order.
    It composes only a `no_draw` receipt; the source destination transform is
    still unavailable. Remaining: an indexed framebuffer blit may open only
    with explicit source-backed destination/clip/orientation evidence.
  - 2026-07-17 update: SUMMARY_IMAGE now retains its ordered `dtImageOffset`
    receipts (`0xfe`, then `vb30`), and DB14 binds their signed x/y sum to the
    matching RAW4 `QUERY_EXPANDED_RECT` clip receipt. Changed rect, offset,
    clip id, or flip fact rejects. This is still a source-owned `no_draw`
    destination receipt: the final QUERY_PICST_IT scale/orientation transform
    remains unproven, so no framebuffer blit is admitted.
  - 2026-07-17 update: a complete DB14 timer/GDAT receipt can now be joined
    with the verified selector and table-geometry receipts only under one
    session/map token. The resulting viewport evidence and M11 delivery plan
    retain all identities and are no-draw; changed timer/GDAT/selector/table,
    session, or map evidence rejects. Remaining: exact source table proof for
    the actual `8/9/10/12` image field and original pixel decoding.
  - 2026-07-15 update: leader-hand rendering now follows SKProject
    `_2405_014a`/`DRAW_ITEM_IN_HAND`: it reads the held record's exact
    `dtWordValue(6)` selector and resolves the selected `dtImage` field from
    game tick, record index, or party direction only for source-proven modes
    0/2/5. The packed M11 item key now preserves that field through both the
    boot pixel and palette providers; it no longer silently collapses every
    held object to `dtImage/F9`. Random, charge-dependent, or
    equipment-context selectors remain unavailable until their source record
    state is modeled. This corrects the old hand-only field substitution;
    cursor-buffer placement and its full source blit contract remain open.
  - 2026-07-15 update: a source-admitted DB5/DB9 row now carries its exact
    decoded WEAPONS/CONTAINERS F9 pixels and IMG3 local palette through the
    M11 frame with ObjectID, coordinate, category/type, and pixel-hash proof.
    The renderer consumes that receipt directly and cannot re-query a changed
    GDAT provider. Remaining `DRAW_ITEM` geometry is still explicitly gated.
  - 2026-07-15 update: an admitted DB2 Text or DB3 Actuator custom door
    button now carries its exact `WALL_GFX/index/dtImage/1` pixels and local
    palette directly into M11. The direct handoff retains map coordinate,
    ObjectID, WALL_GFX index/field, packed GDAT key, and an indexed-pixel
    hash; `DRAW_DEFAULT_DOOR_BUTTON` consumes it only after the existing
    root receipt matches. Absent or changed bytes remain no-draw. This does
    not treat `WALL_GFX/F9` or `DOORS/F9` map chips as visible door art: their
    source placement semantics are still unproven. A button-only DB2/DB3
    route is not a DB0 door: it does not borrow a DoorType, GRAPHICSSET frame,
    panel, or side jamb when those source owners are absent.
  - 2026-07-16 update: this bounded `DRAW_DEFAULT_DOOR_BUTTON` route now also
    requires the exact `WALL_GFX/index/dtImage/1` raw `GRAPHICS.DAT` interval
    and receipt hash alongside the existing DB2/DB3 ObjectID, map-cell,
    palette, and decoded-pixel evidence. The source-selected direct M11
    surface is rejected if that raw provenance is incomplete. This admits no
    other ornament or static-object transform: unproven clip, scale, flip,
    side, and placement branches remain no-draw.
  - 2026-07-15 update: HUD HP, stamina, and mana bars now use the same
    source-owned `glbChampionColor[player]` value, initialized by
    `SkWinCore::INIT` as `7,11,8,14`; the prior independent `2,11,12`
    resource colors were Firestaff-only. A later authenticated mutation of
    `glbChampionColor` remains required before any non-bootstrap override is
    admitted.
  - 2026-07-15 update: the live M11 DM2 branch no longer sends a verified
    boot profile through `dm2_v2_runtime_render_frame`'s
    procedural dungeon path. It now presents `dm2_v1_boot_runtime_render_frame`
    directly with no V2 callback, consuming the mounted G1 pose and GDAT scene;
    the optional V2 HUD remains limited to decoded original GDAT images. The
    direct boot smoke route proves V2 was not attempted and the resulting
    frame has the complete real-material/no-fallback receipt. Remaining work
     is the unproven source geometry and material cases above, not a generated
     substitute.
  - 2026-07-15 update: the canonical G1 runtime now materializes the active
    map's direct DB0 Door roots once and uses only that cached `DME.h::Door`
    `w2` receipt to populate visible door type/button/state/opening/ornate
    fields. A front-door scene may consume a D0C GDAT panel only when the
    matching direct root and source-owned M11 panel command both exist. The
    former generic `w0`/next-link lookup is not used by G1, and unknown wall
    button owners remain unavailable. Remaining work is source-proven full
    cell ordering, decoration, object passes, and all unproven door routes.
  - 2026-07-15 update: source-required doors now require the active G1
    `MapGraphicsStyle` scene receipt before `DRAW_DOOR_FRAMES` may resolve a
    `GRAPHICSSET` frame. The old set-one renderer convenience is no longer a
    valid material owner; absent scene routing blocks the entire door pass
    before any GDAT fetch or pixel draw.
  - 2026-07-15 update: the wall M11 planner now has the same G1 ownership
    rule. It refuses to emit any `GRAPHICSSET` plan without a live
    `MapGraphicsStyle` receipt, rather than encoding the historical set-one
    default for later consumption.
  - 2026-07-15 update: a presented floor/ceiling M11 plan now stays attached
    only while its exact `GRAPHICSSET` index and command hash match the active
    G1 scene-control transaction. A changed map/style or a mismatched plan is
    detached before rendering and therefore remains no-draw.
  - 2026-07-15 update: creature scene rendering now has only the selected
    `DRAW_MAP_CHIP`/Rect14 GDAT bitmap route. Generated fallback rectangles,
    colours, and health bars are removed; absent material is blocked and
    leaves no creature overlay. Remaining work is authentic DB4/CCM ownership
    and source placement/animation semantics.
  - 2026-07-15 update: the direct G1 DB4 creature handoff now binds decoded
    `CREATURES/type/F9` bytes with a row-wise FNV receipt in addition to
    dimensions and its IMG3 local palette. The viewport recomputes that
    receipt immediately before blitting, so a mutated/stale provider buffer
    is blocked rather than replayed under old provenance.
  - 2026-07-15 update: missile and cloud map chips now require the source-owned
    GDAT category selected by the runtime record. Category zero no longer
    becomes a generic spell-missile (`0x0d`) image; it is no-draw and strict
    profiles record a blocked projectile material. Remaining work is authentic
    missile/cloud owner records and exact source placement/timing.
  - 2026-07-15 update: floor objects, creature possessions, and leader-hand
    overlays now require their record-owned GDAT category. Category zero and
    unknown DB pools no longer normalize to `MISC/0x15`; they are no-draw and
    strict profiles record the appropriate blocked source material. Remaining
    work is original record ownership and exact source placement, not a
    substitute category.
  - 2026-07-15 update: direct G1 DB5 weapon roots now traverse the complete
    `DRAW_MAP_CHIP` viewport path as `WEAPONS/itemType/F9`. The runtime admits
    only a visible receipt row, and the final blit rechecks ObjectID, map
    coordinate, item type, decoded dimensions, decoded pixel hash, and IMG3
    local-palette hash. Boot admits that pixel hash only after the same exact
    virtual GDAT entry has raw-byte and decoded-pixel evidence. A mismatched
    or absent receipt is no-draw. The current canonical corpus
    correctly remains blocked because it lacks `WEAPONS/126/F9`; broader DB6--
    DB10 object ownership and mutable floor-chain semantics remain open.
  - 2026-07-15 update: direct G1 DB9 containers now use the same authenticated
    `DRAW_MAP_CHIP` admission with `CONTAINERS/ContainerType/F9`. The viewport
    rechecks ObjectID, tile, container type, decoded dimensions, decoded pixel
    hash, and local palette before a blit, while container contents remain
    unread. Canonical G1 has the source container root `0xe408` but no exact
    `CONTAINERS/0/F9`; it is therefore blocked rather than borrowing another
    item's art. DB6--DB8/DB10 have no direct G1 roots and remain unavailable
    without a separately proven source route.
  - 2026-07-15 update: the static `INTERFACE_GENERAL` HUD chrome (top/action/gold
    areas, action icons, portrait surround, dividers, coin/label, slot fills,
    and unavailable name pixels) is now GDAT-only. Missing material leaves the
    surface blank; only live champion state bars and leader state remain runtime
    overlays. Remaining work is source-backed HUD command coverage and the
    original cell/pass ordering, not substitute artwork.
  - 2026-07-15 update: `DRAW_CHAMPION_PICTURE` now accepts only its selected
    `CHAMPIONS` GDAT bitmap. Missing portrait material is blank (and blocked
    for strict source profiles), not a generated champion-colour rectangle.
  - 2026-07-15 update: floor/ceiling planes and all `DRAW_WALL` cells now
    have the same source-only rule. Missing GDAT material produces no plane or
    wall pixels; strict source profiles record the appropriate blocked class.
  - 2026-07-15 update: `DRAW_DOOR` no longer replaces a failed `DOORS`
    panel query with its generated coloured rectangle. The selected GDAT panel
    is now the only visual route; strict source profiles record the missing
    panel as blocked, while non-source profiles leave it blank.
  - 2026-07-15 update: the complete M11 wall-material receipt now hashes each decoded `GRAPHICSSET` panel, not only raw bytes and its local palette. Canonical-data coverage explicitly requires D3L/D3R/D2L/D2R to retain the skproject viewport field, decoded pixels, and palette before the renderer may consume the plan. A missing side panel blocks the source frame.
  - 2026-07-15 update: source-required `DRAW_WALL` now consumes its own
    `QUERY_TEMP_PICST` geometry rather than a `G0163` approximation. The
    command retains `0x2be + cell` RAW4 table/row receipts, IMG3 offsets,
    exact `DM2_QUERY_BLIT_RECT` crop/destination, and side mirror. The
    canonical route covers the observed nested RAW4 grammar and fails closed
    for an unproved global-clip branch. Remaining: live movement/light
    clipping, complete source cell ordering, and dynamic tile/object passes.
  - 2026-07-15 update: stationary `DRAW_WALL` now follows SKProject
    `DM2_DRAW_DUNGEON_TILES`' exact `table1d7029` cell scheduler rather than
    the former DM1-like depth loop. The admitted wall cells are consumed in
    source pass order `11,10,7,8,6,4,5,3,1,2`; D3C and D0C remain unavailable
    because Firestaff has no source `DRAW_WALL` geometry for those centers.
    This proves only the wall subpass. The tile-type branches that interleave
    doors, creatures, static objects, pits, and teleporters still need real
    per-cell records.
  - 2026-07-15 update: source-required center doors now follow the same
    `table1d7029` scheduler: D3C/cell 11, D2C/cell 6, then D1C/cell 3.
    D0C/cell 0 is deliberately no-draw in this pass because the source table
    does not dispatch it; `DM2_DRAW_PLAYER_TILE` is a separate later route.
    Remaining: bind that route and the tile-type record decision before
    admitting front-cell doors or any interleaved object pass.
  - 2026-07-15 update: moving source-required frames now reject the cached
    stationary wall plan. `DM2_DRAW_WALL` changes the signed RAW4 request by
    the live `table1d6b15[iViewportCell]` distance; until that dynamic RAW4
    receipt is bound, M11 draws no wall rather than replaying a stationary
    crop at an invented movement position.
  - 2026-07-15 update: `DRAW_DOOR`'s admitted D0/D1/D2 center-cell distance routing now reaches the M11 GDAT door command: skproject distances 0/1/2 select verified `DOORS` image fields 0/0/1. D0 follows the source's mandatory image-zero branch and records `iStretchDual=0x71`; D1/D2 retain `0x40`, with `iLightPalette=0`. The viewport revalidates this source control tuple before consuming the command, so an initial-`0x40` D0 receipt or altered retry route is no-draw. D3 (cell 11, distance 3) now reaches the source-material viewport as a panel-only D3C draw at the skproject `G0163` geometry `[74..149] x [25..75]`; it carries field 2, or only the documented field-0 GDAT retry with stretch `0x1c` and light palette 3 when field 2 is unavailable. D3 has no invented frame. The same RAW4 `QUERY_BLIT_RECT` route admits vertical state-1..3 positions as `tlbRectnoDoorPosition + state`. Horizontal state-1..3 now carries its original two-command transaction: right half first at `base + state + 6`, then left half at `base + state + 3`, each with its own RAW4 rectangle and source crop. A missing half, palette, raw row, decoded hash, or geometry blocks the whole door. `INTERFACE_GENERAL dt04` expansion remains unavailable.
  - 2026-07-15 update: the D3 field-0 retry (`stretch 0x1c`, palette 3)
    now consumes SKProject `_32cb_0804`'s stationary `_4976_4226` darkness
    table and the already decoded `INTERFACE_GENERAL/0/dt07/2` `_0b36_037e`
    palette remap. Its command binds the exact `c_light` receipt, effective
    darkness, remapped palette hash, and transform hash; M11 rejects any
    missing or mismatched receipt. The live inventory/record accumulator and
    darkness-state capture needed to publish `c_light` remain open, so this
    route stays no-draw when authentic runtime evidence is absent.
  - 2026-07-15 update: source-required M11 square projection no longer assigns a synthetic per-tile full-light value (`15`). The active G1 `GRAPHICSSET` receipt supplies only proven ambient controls; until `c_light` produces an authenticated per-square result, projected squares retain light level zero and cannot claim a fully-lit source route. Compatibility-only callers retain the old default outside the original-data path.
  - 2026-07-15 update: the source-bound terminal `c_light.cpp::DM2_RECALC_LIGHT_LEVEL` receipt is now implemented: dynamic maps consume an authenticated pre-darkness `v1e0974` value, non-dynamic maps use the original base level one, then the receipt subtracts authenticated `v1e0978` and clamps to 0..5. It refuses a missing raw-state hash and does not derive brightness from GRAPHICSSET. The authenticated result now has a fail-closed M11 dungeon-square metadata consumer, the source `DISPLAY_VIEWPORT` `light_level * 10` palette parameter, a source-frame identity binding, and a raw `Map_definitions::Difficulty()` receipt that rejects the wrong fixed/dynamic state branch. Remaining: bind the inventory/record accumulator and darkness state from a real PC runtime/save before live M11 can publish or remap a palette.
  - 2026-07-15 update: live legacy DB0-door wall-button material now resolves
    SKProject `QUERY_ORNATE_ANIM_FRAME(WALL_GFX, index, tick, 0)` before M11
    selects its `dtImage` field. The exact `dtWordValue/0x0d` length/high-bit
    base or NUL-terminated `dtText/0x0d` base-36 sequence is required; bad
    sequence bytes, missing data, or an unavailable selected image remain
    no-draw. G1 direct-door roots and wall/floor ornament placement still
    need their own source-backed animation-record handoff.
  - 2026-07-15 update: `DRAW_DOOR_FRAMES` side jambs now consume their
    separate `GRAPHICSSET` fields through exact `QUERY_TEMP_PICST` inputs:
    `QUERY_GDAT_SUMMARY_IMAGE` offsets, RAW4 `QUERY_CREATURE_BLIT_RECTI`
    rows, source left/right override modes, right mirror, local IMG3 palette,
    and the live scene colorkey. The older centre-frame wall rectangle is not
    used for this route. Remaining: nonzero light-palette translation and
    unproved `DRAW_DUNGEON_GRAPHIC` centre-frame placement.
  - 2026-07-14 update: the active G1 map's exact `Map_definitions::FloorGraphics()` byte list is now admitted through the skproject `LOAD_LOCALLEVEL_DYN` ordering (creatures, wall graphics, floor graphics). Every listed `FLOOR_GFX/index/F9` must have raw/decoded evidence and its own `QUERY_GDAT_IMAGE_LOCALPAL` result before its hash reaches M11; missing material invalidates the plan. This is map-chip material admission only. Exact `SUMMARIZE_STONE_ROOM` floor-decoration predicates and 3D placement remain unavailable until their full actuator/animation semantics are source-bound.
  - 2026-07-14 update: the same active-map `WallGraphics()` list now binds each exact `WALL_GFX/index/F9` map chip and local palette into a separate M11 material plan. A missing listed chip or palette invalidates the plan; it cannot fall back to a generic wall surface. This admits source material only, not the unresolved random-decoration, clipping, or 3D ornament-placement semantics.
  - 2026-07-14 update: the `DRAW_MAP_CHIP` teleporter branch now consumes only
    `TELEPORTERS/0/F9`, slices the real horizontal map-chip atlas by live tick,
    applies its own local IMG3 palette, and carries raw/decoded/palette identity
    into the runtime/M11 receipt. Missing material blocks the frame and paints
    nothing. Remaining: source-captured exact `SUMMARIZE_STONE_ROOM` placement,
    clipping, and teleporter animation timing across real dungeon views.
  - 2026-07-13 update: M11 now consumes the exact boot-owned floor/ceiling
    receipt before presentation: two real GRAPHICSSET plane blits, zero plane
    fallback draws, zero blocked source-material passes, and the existing
    atomic map/scene/palette identity are all mandatory. Remaining DM2-010
    work is the broader viewport-cell and dynamic-material coverage above.
  - 2026-07-15 update: `DM2_DISPLAY_VIEWPORT` plane composition now retains
    its source order: ceiling `GRAPHICSSET/1` through rect `0x2bc` before
    floor `GRAPHICSSET/0` through rect `0x2bd`. The transaction binds each
    decoded image, its own local palette, geometry, and ordering hash; a
    swapped order or changed material is callback-free no-draw. The live D1/D2
    three-wall occlusion route now consumes GRAPHICSSET 0x70/0x71 low-byte
    ceiling and high-byte floor trims; a missing optional word is the source
    zero/no-trim result, not a borrowed control. Remaining: later tile/object
    passes.
  - 2026-07-13 update: source-selected `DOORS` and `WALL_GFX` image fields
    now publish no-draw material receipts only after the original image,
    metadata, decoded pixels, and `QUERY_GDAT_IMAGE_LOCALPAL` palette agree.
    `DRAW_DOOR` keeps its required non-zero `GDAT_IMG_COLORKEY_1` gate, and
    `DRAW_WALL_ORNATE` receives its exact selected image field from the caller;
    neither route selects a replacement image or paints a fallback. Remaining
    work is to consume these receipts in source-order runtime draw placement.
  - 2026-07-16 update: `GET_WALL_ORNATE_ALCOVE_TYPE` now has a DM2-owned
    source-named receipt over `WALL_GFX` `dtWordValue 0x0A`, bounded to the
    real skproject alcove range before `DRAW_WALL_ORNATE` material admission.
    The wall-ornament material receipt consumes that named alcove result plus
    the selected original image field, decoded pixels, local palette, position,
    flip, colorkey, and item-displacement evidence. Real DM2 `GRAPHICS.DAT`
    verifies existing alcove records and at least one complete selected-field
    material chain. Remaining work is runtime placement/source-order draw
    consumption and the broader blit/stretch/palette coupling.
  - 2026-07-13 update: source-selected inventory item addresses now produce a
    no-draw HUD material receipt only when the exact item-category `dtImage`,
    decoded four-bit pixels, and `QUERY_GDAT_IMAGE_LOCALPAL` payload agree.
    The selected item snapshot must match the receipt ObjectID. Firestaff does
    not infer an ObjectID-to-GDAT mapping, choose another icon, or draw a
    substitute; original record routing and source-order renderer consumption
    remain open.
  - 2026-07-13 update: the panel now directly consumes a verified selected
    item receipt into a caller-provided HUD surface using skproject's exact
    local palette and transparency key 12. It accepts only a source-provided
    `QUERY_BLIT_RECT` result and rejects changed material, palette, key,
    source span, or destination span. Live M11 ownership of the original
    record-to-GDAT and rectangle route remains open; this panel API supplies
    no default icon, position, clip, scale, or fallback pixels.
  - 2026-07-13 update: selected-item survey preview now follows
    `DRAW_ITEM_SURVEY` exactly: it admits only the selected source item's
    optional `dtImage/0x11`, requires original rect `0x1EE` and key 12, and
    consumes it through the verified panel blit. Missing `0x11` or a different
    rect/key produces no preview; there is no generic item illustration.
  - 2026-07-13 update: selected leader-hand consumption now follows
    `DRAW_ITEM_IN_HAND`: it admits only the caller-selected item `dtImage`,
    copies its exact local palette, and blits the complete decoded image into
    an origin surface with identical dimensions. A material/palette/pixel
    mismatch or a different destination size produces no hand image; no
    generic hand icon, transparency substitution, or scaling is supplied.
  - 2026-07-13 update: `DRAW_HAND_ACTION_ICONS` now binds its hand-slot
    backdrop to the source-selected INTERFACE_GENERAL/4 `dtImage`, local
    palette, and direction-derived expanded rect. A changed field, palette,
    pixels, rect, or transparency mode produces no backdrop; no generic panel
    tile or replacement image is available.
  - 2026-07-23 update (Lane C, cycle 8): DM2 V1 door overlay real-data plan
    and live M11 plan enumeration fixes. `dm2_v1_viewport_build_door_render_plan`
    now routes unrecorded doors (door_record_type == 0) through the square panel
    (`dm2_v1_viewport_door_panel_graphic_index_for_square`) instead of the
    DoorType/OpeningDir record panel, matching SKWIN `DM2_DRAW_DOOR` semantics
    and unblocking the canonical `DOORS`/`DOOR_GFX`/`DOOR_BUTTONS` overlay plan.
    `test_dm2_v1_gdat_door_overlay_plan_real_data` now passes. Fixed a live
    composition regression where `dm2_v1_runtime_enumerate_dm2_viewport_m11_live_plans`
    passed `&receipt.composition` to `dm2_v1_runtime_build_dm2_viewport_m11_composition`,
    which zeroed the previously captured `surface_before` snapshot; the snapshot
    is now saved and restored around the build. `test_dm2_v1_dm2_viewport_m11_live_enumeration`
    is updated to provide a source-valid door panel command (kind, view_square,
    field, draw_distance, stretch_dual, light_palette). Remaining DM2-010 work:
    complete source cell ordering, `DRAW_ITEM` clipping/placement, door states
    beyond the overlay plan, object/creature/cloud passes, scale/flip rules, and
    verified GDAT material.
  - 2026-07-23 update (Lane C, cycle 10): source-locked door panel/button
    placement. Closed D0C/D1C/D2C/D3C panels and D0C/D1C/D2C default buttons now
    resolve their destination rectangles from `INTERFACE_GENERAL/0/RAW4/0`
    (`tlbRectnoDoorPosition` / `tlbRectnoDoorButton`) when source materials are
    required, falling back to the prior hard-coded rectangles only when no asset
    loader is available. Added `asset_loader` to `DM2_V1_ViewportState`,
    `dm2_v1_viewport_set_asset_loader`, source-aware helpers
    `dm2_v1_viewport_door_panel_rect_for_square_from_source` and
    `dm2_v1_viewport_door_button_rect_for_square_from_source`, and
    `dm2_v1_boot_asset_loader` with runtime wiring. Exposed
    `dm2_v1_gdat_door_overlay_panel_rect_number`,
    `dm2_v1_gdat_door_overlay_button_rect_number`, and
    `dm2_v1_gdat_door_overlay_query_raw4_destination_rect`. New test
    `tests/test_dm2_v1_door_panel_source_placement_real_data` proves panel and
    button rects match RAW4-derived M11 commands and differ from the fallback.
    Remaining DM2-010 work: complete source cell ordering, `DRAW_ITEM`
    clipping/placement, door states beyond closed-panel/button placement,
    object/creature/cloud passes, scale/flip rules, and verified GDAT material.
- DM2-011 — `skproject/SKULLWIN/c_weather.cpp` `DM2_UPDATE_WEATHER`, `c_light.cpp`, and `c_cloud.cpp`: `DM2_SET_TIMER_WEATHER` and `DM2_weather_3df7_0037` are now mapped as source receipts for outdoor 182-tick scheduling and reseed/weather transition. The runtime forwards its exact live weather state to the outdoor viewport and records the handoff. `QUERY_GDAT_TEXT(ENVIRONMENT, MapGraphicsStyle, 0x67..0x6c)` now retains all six exact raw `dtText` receipts and decodes only the bounded, source-proven `QUERY_CMDSTR_TEXT` `CD`/`FW` values used by `c_bkgrnd.cpp::RETRIEVE_ENVIRONMENT_CMD_CD_FW`; a missing NUL, missing/zero CD, or out-of-range FW clears the material bit and cannot cause a substitute draw. `DM2_UPDATE_WEATHER(0)` is now executed by the runtime tick, its selected cloud/rain/bolt commands are converted into live ten-byte `DistantEnvironment` slots, and the slots are bound to the renderer after validation against the current `MapGraphicsStyle`/GDAT weather receipt. Real-data outdoor frame capture now passes: `tests/test_dm2_v1_outdoor_weather_frame_capture` proves the renderer consumes the bound live slots and M11 accepts the frame. This closes DM2-011; see DONE.md for details. Do not add a procedural visual substitute.
  - 2026-07-15 update: presented weather now consumes the live ten-byte
    `DistantEnvironment` register image rather than reinitializing it at the
    renderer. `cmFW`, `cmCD`, `w4/w6`, and `b8/b9` are hash-validated against
    the selected GDAT command and drive the original mirror, offset, and
    scale branches through the final `QUERY_TEMP_PICST` receipt. Altered,
    mismatched, or out-of-range slot data is no-draw. Remaining: source timer
    dispatch/reseed and corpus-backed live slot production.
  - 2026-07-15 update: a live `DistantEnvironment` slot now binds only after
    it is revalidated against the current map's `MapGraphicsStyle` and exact
    GDAT weather receipt. Every scene-control refresh clears prior slots, so
    a matching command byte from another level cannot inherit a weather image
    or destination. Remaining: source timer dispatch/reseed and corpus-backed
    live slot production.
  - 2026-07-15 update: a presented source weather transaction now carries its
    exact `WeatherRendererReceipt` hash and command count through the viewport
    into M11. M11 accepts it only after every selected `DistantEnvironment`
    layer drew from its matching GDAT material; a missing layer clears the
    receipt and makes the source frame unavailable. Remaining work is the
    original timer/reseed/light/cloud state machine and broader real capture.
  - 2026-07-15 update: runtime now accepts only validated live
    `DistantEnvironment` slots and joins them with the selected GDAT/dt04
    receipts for stationary outdoor frames. No slot, malformed slot, movement,
    or missing source material stays no-draw. Remaining: original timer/save
    ownership that produces the live slots, moving-frame transforms, and
    corpus capture; do not derive slots from generic weather intensity.
  - 2026-07-13 update: `DM2_UPDATE_WEATHER` cloud-then-rain command order now
    has a DM2-owned execution plan. It preserves the source's ten-byte slot
    sequence and only publishes a selected `CD`/`FW` command when its original
    ENVIRONMENT `dtText` receipt remains valid. The actual
    `QUERY_TEMP_PICST` image resolver is still unproven and intentionally not
    inferred; a missing material returns no plan rather than an overlay.
  - 2026-07-13 update: each selected plan entry now carries the exact
    `DistantEnvironment` defaults written by
    `RETRIEVE_ENVIRONMENT_CMD_CD_FW` (`w4/w6=0`, `b8/b9=0x40`) alongside the
    original `CD`/`FW` values. This closes no image or placement gap: actual
    `ENVIRONMENT_DRAW_DISTANT_ELEMENT` interpolation and `QUERY_TEMP_PICST`
    realization remain blocked on the original rect/image route.
  - 2026-07-14 update: M11 now consumes the already source-owned
    `DistantEnvironment` transform at the final GDAT weather draw: `FW`
    mirror, movement offset, and the verified 0x40/0x34 fixed-point scale are
    passed to the indexed `DRAW_TEMP_PICST` equivalent. Invalid scale values
    block the complete source weather pass; no procedural overlay or 1:1
    substitute is retained. The original timer/reseed/light/cloud dispatcher
    and real-corpus runtime capture remain open.
  - 2026-07-13 update: the selected `QUERY_TEMP_PICST` material now carries
    its exact GDAT IMG3 width, height, bpp, category-wide `dtImageOffset`
    (`0xfe`), and image-specific `dtImageOffset`, in source order. Absent
    offset fields remain the original zero offset; no rect coordinates or
    pixels are inferred. Remaining weather work is still source timer/light
    behavior, actual original rect expansion, and real-data capture.
  - 2026-07-13 update: `CD` now reaches a bounded source-only
    `INTERFACE_GENERAL/0/dt04/0` destination-clip receipt through
    `QUERY_TEMP_PICST`/`QUERY_BLIT_RECT`, and the active runtime carries its
    all-material clip hash and mask. Unsupported compressed rect forms, a
    missing exact IMG3/local palette, or any missing rect route fail closed;
    the viewport still draws no weather pixels. Remaining weather work is the
    complete compressed-rect grammar, timer/light/cloud execution, and
    original runtime capture.
  - 2026-07-13 update: a validated post-save `WeatherState` now has a
    fail-closed renderer receipt only when every source-owned ten-byte
    `DistantEnvironment` slot still names its exact `0x67..0x6c` command,
    owns decoded GDAT material, and resolves through the original `dt04`
    destination clip. The receipt does not infer cloud/rain selection from
    generic state and does not inspect or invent timer bytes. Remaining work
    is still the original timer/reseed/light/cloud dispatcher and real-corpus
    runtime capture.
  - 2026-07-13 update: the separate `c_gui_vp.cpp` dialogue path now has a
    source-only `GRAPHICSSET` receipt for its selected shell (`-4..-2`) and
    glyph field `3`, including each exact IMG3 local palette. It is not a
    dialogue renderer: message layout, text, and `DRAW_PICST` remain blocked
    until their source contracts are wired.
  - 2026-07-13 update: the active runtime now carries the verified
    `GRAPHICSSET` dialogue-shell/glyph IMG3-plus-local-palette receipt for
    field `0xfd` into frame ownership. It remains evidence-only and no-draw.
  - 2026-07-20 update (job/w3): the `DM2_UPDATE_WEATHER(0)` frame update
    (`c_weather.cpp:91-506`) is now bound as `dm2_v1_update_weather_0` in
    `dm2_v1_update_weather_pc34_compat` — day rollover through
    `table1d70f0[(gametick+v1e1438)/0x555 % 0x18]`, the exact lightning
    evaluation RNG order (rain decay, `u16(0x100 - intensity +
    (RAND&0xf))` threshold, `RG51w` 7/0x28 at 0xcd, flag latch, rain
    increment gating, `v1e1481`-gated flash), and the light/cloud command
    selection: cloud `0x67/0x68/0x69` (+`storm_active`) at
    0x10/0x40/0x80, rain `0x6a/0x6b/0x6c` at 0x40/0x80/0xc0, lightning
    bolt `100+RAND16(3)` via RANDBIT, with source slot semantics (a
    failed `RETRIEVE_ENVIRONMENT_CMD_CD_FW` does not advance the 10-byte
    slot, so the next command or the `0xff` terminator overwrites it)
    reported as a compacted live chain. The thunder paths hand off
    `CREATE_CLOUD`/`INVOKE_MESSAGE` as receipt flags with an explicit
    `rng_diverges` marker; the thunder-sound latch (`v1d718c`), clamped
    volume and final `v1e024c = 1` light-recalc (m_4A899) are bound.
    `RECALC_LIGHT_LEVEL`, `UPDATE_GLOB_VAR`, noise queues, the bolt rect
    geometry and the RETRIEVE calls themselves stay host-owned; the
    slice fails closed on zone > 31 and on intensity != 0 with step == 0
    (source division guard). Remaining: saved timer
    owner proof, and real-data capture.
  - 2026-07-20 update (job/w3, round 14): the slot commands 0x64..0x6c
    are now bound against the `QUERY_TEMP_PICST` execution chain. The
    source-owned command range covers the lightning bolt 100+RAND16(3)
    (0x64..0x66) alongside cloud 0x67..0x69 and rain 0x6a..0x6c
    (`DM2_V1_WEATHER_COMMAND_COUNT` 9, renderer/distant slots up to 3,
    matching the source's cloud-then-rain-then-bolt slot order and
    m_4A8A8 terminator). Bolt slots carry the c_weather.cpp:471
    RANDDIR byte in cmFW position (0..3; only value 2 evaluates the
    0x20 mirror, everything else draws unflipped) instead of a GDAT FW
    key; the host still owns the bolt w4/w6 rect interpolation
    (`DM2_rect_098d_04c7`). `QUERY_GDAT_TEXT`'s source decode
    (SkWinCore.cpp 2636:0377, gated on `dtWordValue(0,0,0)` bit 3 per
    55629) is now applied before the CMDSTR parse, with the lowercase
    `cd`/`fw` keys of EnvCM_CD/EnvCM_FW (SkGlobal.cpp:755). Verified
    against the real DM2 GRAPHICS.DAT (set 5): all nine commands carry
    dtText+dtImage; bolt text is "cd6002" (no FW), cloud "cd6004fw32",
    rain "cd6005fw8", and every image decodes through the
    source-faithful IMG9 path (bolts 16x36/23x33/28x38, clouds 224x39,
    rain 224x62, all 8bpp) — the decoded-pixel receipt now records
    these real extents. The remaining material gap is exactly named:
    the real 8bpp IMG9 command images carry no 16-color local palette,
    and their `QUERY_GDAT_SUMMARY_IMAGE` global-palette identity is not
    yet proven, so they keep the decoded-pixel receipt but stay
    material-invalid (no draw) until that palette receipt is bound.
    GRAPHICS_DATA_OPEN now hashes the full 0x64..0x6c text range.
  - 2026-07-20 update (job/w3, round 15): the IMG9 global-palette
    identity is now bound against actual GDAT data, closing the last
    material gate.  Source rule: `QUERY_GDAT_IMAGE_LOCALPAL`
    (SkWinCore.cpp 3e74:521A, DM2_EXTENDED_MODE == 1) returns NULL for
    any non-4bpp image, and `QUERY_GDAT_SUMMARY_IMAGE` (0B36:0520) then
    installs the 256-entry identity translation (ref->b58[i] = i,
    ref->w56 = 256) — each decoded pixel byte indexes the global screen
    palette directly.  `DM2_V1_WeatherCommandReceipt` now carries that
    exact palette-translation receipt (16-entry local for 4bpp IMG3/U4,
    256-entry identity hash for 8bpp IMG9), the draw plan and renderer
    gate compare the translation identity, and the real-data test
    `dm2_v1_weather_img9_global_palette_identity_real_data` proves all
    nine set-5 commands material-valid with the identity hash computed
    independently from the source table.  The saved timer owner proof
    also landed this round: `dm2_v1_save_timer_weather_owner_receipt`
    binds a restored 12-byte wire record (type 0x54, actor 0, map 0 per
    c_weather.cpp:22-30) to its `DM2_UPDATE_WEATHER(1)` owner
    (c_tim_proc.cpp:4179-4183) with the signed schedule delta against
    the restored gametick (c_savegame.cpp:1486-1487);
    `dm2_v1_save_timer_weather_owner_pc34_compat` covers owner identity,
    overdue-fire, non-chain rejects, sorted-queue dispatch order, and
    the RAND16(256)+50 reschedule bounds.  Remaining: real-data capture.
  - 2026-07-23 update (Lane C, cycle 6): runtime weather frame update now
    produces and binds live `DistantEnvironment` slots. `dm2_v1_runtime_tick()`
    runs `DM2_UPDATE_WEATHER(0)`, converts the resulting `live_cmds` into ten-byte
    register images, and admits them through the runtime GDAT receipt.
    `tests/test_dm2_v1_runtime_weather_frame_slot` passes against canonical DM2
    data.
  - 2026-07-23 update (Lane C, cycle 6, DM2-011 completion): real-data outdoor
    frame capture now passes. `tests/test_dm2_v1_outdoor_weather_frame_capture`
    proves the renderer consumes bound live `DistantEnvironment` slots and the
    M11 runtime-frame gate accepts the resulting frame. This closes DM2-011;
    moved to DONE.md.
- DM2-012 — `skproject/SKULLWIN/c_item.cpp`, `c_hero.cpp`, `c_dialog.cpp`, and `c_engage.cpp`: `src/dm2/dm2_v1_inventory_panel.c`, `dm2_v1_shop.c`, `dm2_v1_companion.c`, and M11 expose catalog-driven panels and simplified interactions. `c_dialog.cpp::DM2_dialog_2066_3820` now carries the real `DIALOG_BOXES/0x81/0` pixels and local palette to the viewport through its expanded `RECT_453` host command, and remains no-draw unless the source dialogue owner marks it active. Remaining: original modal state/event, text, button and cancellation semantics; no catalog panel or fallback dialogue may replace them.
  - 2026-08-06 update: the exact static material half of inventory survey and
    hand-action rendering is now real-data covered. The receipts accept only
    `INTERFACE_CHARSHEET/0/dtImage/1` at `RECT_1EE` and
    `INTERFACE_GENERAL/4/dtImage/2..5` at their source direction rectangles;
    local palette, raw payload and decoded-pixel identity are rechecked when
    consumed. `test_dm2_v1_inventory_gdat_real_data` verifies all 64 source
    hand routes and the survey frame in the mounted PC English corpus. This
    is not permission to reopen the M11 inventory panel: the original
    layout/event/modal route is still incomplete and therefore remains
    unavailable rather than falling back to a DM1 panel or host UI.
  - 2026-07-22 update: `dm2_v1_dialogue_open_panel_receipt` and `dm2_v1_dialogue_save_input_apply` now bind the save/load panel's source GDAT label receipt, state/event, text, button and cancellation semantics for synthetic fixtures. `dm2_dialogue_open_panel_text_decode` treats a missing GDAT 0/0/dtWordValue/0 as unencrypted, preserves the source payload size in the receipt, and rejects empty labels. **2026-08-07 correction:** `dm2_v1_asset_load_image_metadata` now follows `DME.h::IMG3::Getpf()` exactly: only `OffsetY() == -32` may read `w4` as 4/8bpp, `OffsetY() == 31` is C8/8bpp, and all other compressed records are 4bpp. A caller-authored `w4` no longer changes production image depth.
  - 2026-07-22 update: `src/dm2/dm2_v1_boot.c` `dm2_v1_boot_parse_interface_action_table` now decodes INTERFACE_GENERAL/0/dt07/2 exactly like `skproject/SKWIN/SkWinCore.cpp` `LOAD_GDAT_INTERFACE_00_02`: a leading group-count byte, one length byte per group, the primary value block, the secondary value block, and then the command tail. `dm2_v1_interface_action_table_remap_palette` now uses the corrected tail offset and the 256 (group, threshold) pairs, matching `_0b36_037e`. CTest `test_dm2_v1_dialogue_box_viewport_real_data` now passes; `test_dm2_v1_boot_profile_smoke` now also passes the dt07/2 span materialization check. Remaining work is broader real-dialogue runtime wiring and consuming the remapped action palette in the live M11 text path.
  - 2026-07-15 update: removed the active M11 leader-hand cursor icon route.
    Its icon bytes were GDAT-backed, but it scaled them into an arbitrary
    14x14 host cursor rectangle. SKProject `DRAW_ITEM_ICON` instead derives
    an object-selected field and `QUERY_EXPANDED_RECT`/`QUERY_BLIT_RECT`
    geometry before its palette-aware blit. The verified inventory material
    consumer is retained, but no icon is drawn until that exact rect route is
    bound; do not use pointer coordinates as replacement geometry.
  - 2026-07-15 update: the active M11 DM2 leader-hand name overlay is now
    no-draw. The previous DM1 rectangle/host-font path converted ObjectIDs to
    catalog labels such as `DM2 MISC 51`; SKProject `GET_ITEM_NAME` instead
    resolves `QUERY_GDAT_ITEM_NAME(category, class)` through `dtText/0x18`
    and `FORMAT_SKSTR`. Do not show a name until that exact GDAT text and
    formatting route is bound; source-backed object icons remain separate.
  - 2026-07-15 update: DM2's M11 inventory-toggle route now fails closed rather
    than opening the shared DM1 `GRAPHICS.DAT` inventory workbench. SKProject
    `CHANGE_VIEWPORT_TO_INVENTORY` owns DM2's `CHAMPIONS`/
    `INTERFACE_GENERAL` GDAT surfaces and click table; those bytes have not
    yet been bound into a complete inventory layout. Existing DM2 GDAT object
    icons stay source-owned, but no DM1 slot panel, host text, or input
    transaction may be presented around them. Remaining: decode and bind the
    complete original DM2 inventory layout/event route.
  - 2026-07-15 update: M11 no longer presents or drives the former fixed-coordinate
    catalog shop overlay. SKProject `SkWinCore::_32cb_0f82_SHOP_GLASS` binds a
    shop through the active G1 wall actuator, `WALL_GFX` image offsets, `dt08`
    item-list bytes, and `WALL_ORNATE__OVERLAY`; Firestaff has not yet proven
    that complete chain. A front-cell shop action now closes the temporary
    catalog state and reports `DM2 SHOP GDAT REQUIRED`, leaving the verified
    GDAT dungeon frame untouched. Remaining: bind the real actuator/ornament
    records and source images before enabling a shop interaction or pixels.
  - 2026-07-15 update: `DM2_dialog_OPEN_DIALOG_PANEL` now also decodes its two original `DIALOG_BOXES/0x81/dtText/0..1` labels using the source `GDAT 0/0/dtWordValue/0` bit-`0x08` `QUERY_GDAT_TEXT` transform, and places them through the raw4 `DM2_COMPRESS_RECTS`/`DM2_QUERY_BLIT_RECT` chains at `RECT_1D2` and `RECT_1D3`. The source panel is rendered at its native 320x200 coordinates. Literal labels are admitted; unknown `DM2_FORMAT_SKSTR` substitutions remain fail-closed. Remaining: source modal state/event loop, version-heading source string, button construction, and cancellation semantics.
  - 2026-07-15 update: save/load pointer selection now accepts only the original event's supplied `event_unk05`/`event_unk09` RAW4 rectangle IDs, expands and measures them through the same `QUERY_EXPANDED_RECT`/`QUERY_TOPLEFT_OF_RECT` grammar, and preserves `c_gfx_str.cpp`'s `strxplus = 7` row stride plus the source maximum slot 10. Coordinates before the measured source baseline fail closed; no menu click geometry or row list is inferred. Remaining: the original modal event producer, save-record text/state, button construction, and cancellation semantics.
  - 2026-07-15 update: `DM2_dialog_OPEN_DIALOG_PANEL` now carries its exact `dm2data.cpp::v1d10eb` compiled `V1.0` heading with palette slot 12 and its `RECT_1C2` raw4 placement into the active viewport command. It is not a localized or synthetic host label. Missing version bytes, hash, rectangle, panel, GDAT labels, font, or palette block the whole source dialogue draw. Remaining: the original modal event producer, save-record text/state, button construction, and cancellation semantics.
- DM2-013 — `skproject/SKWIN/SkWinCore.cpp` startup/render orchestration and `skproject/SKULLWIN/c_gui_draw.cpp`: M11 now rejects incomplete DM2 boot before clearing or drawing its viewport, keeping the launcher frame and publishing `DM2 ORIGINAL DATA REQUIRED` instead of the former red/brown ceiling/floor substitute. Startup presentation also stops if the source-owned `TITLE/0 dt07/4` draw fails. The complete-support receipt accepts only the source's static title/menu surfaces: a hash-verified raw menu screen or a decoded original image field, both with the existing no-synthetic-overlay composite proof. `src/dm2/dm2_v2_*.c` still contains procedural HUD/asset fallbacks and placeholder stamps. Block those remaining paths until verified original data is available; no synthetic DM2 screen may stand in for an original surface.
  - 2026-08-06 update: the V2 palette-control LUT is no longer a stub. It
    reads only the immutable V1 palette table, preserves every source RGB
    byte at neutral settings, and applies a bounded user-requested
    presentation transform only after source rendering. It does not admit
    generated art, a replacement palette, or an unverified V2 surface.
  - 2026-08-06 follow-up: the former static action-icon row was removed from
    the HUD material plan. SKProject selects those hand backdrops dynamically
    through `INTERFACE_GENERAL/4` and `RECT_46..RECT_4d`; static `/3/2..6`
    keys and host coordinates could not represent that source route.
  - 2026-08-06 follow-up: the production GDAT fetch and RAW4 crop route for
    those four hand backdrops is now source-bound and fails closed. It remains
    deliberately unwired from normal gameplay until the live original
    champion formation, possession and hand-selection state is recovered;
    M11 must not infer that tuple from party order, pointer position or a
    Firestaff default.
  - 2026-08-06 follow-up: command dispatch now propagates a rejected
    source-GDAT image callback, so a failed title/menu blit aborts the
    presentation transaction instead of being reported as successfully drawn.
  - 2026-07-14 update: `SkWinCore::INIT` loops `SHOW_MENU_SCREEN()` until
    `GAME_LOAD()` accepts a menu action. `SHOW_MENU_SCREEN` owns one static
    `TITLE/0/dt07/4` surface and `MessageLoop(true)`; DM2 boot/M11 now force
    all title timing fields to that static phase, even when the host supplies
    a non-zero tick. Missing title GDAT remains fail-closed with no dungeon
    render. `UI_EVENTCODE_START_NEW_GAME` now records the source-owned
    `GAME_LOAD` boundary and remains in the title menu until a verified
    `LOAD_NEW_DUNGEON` receipt exists; it no longer creates a local party.
    Remaining: source-proven complete `MAIN_LOOP` control routing and
    `GAME_LOAD`/`LOAD_NEW_DUNGEON` data receipt.
  - 2026-07-15 update: the boot layer now performs the source-owned
    `DM2_LOAD_NEW_DUNGEON` G1 reload transaction from the selected verified
    dungeon path, rechecks its selected MD5 when present, publishes its byte
    hash/seed/map-count receipt, and swaps the parsed dungeon only on complete
    success. It explicitly records the
    source party/leader resets as still required and creates no starter party.
    Runtime HUD now preserves the exact 8-bit `Champion::HeroType` supplied
    by an admitted session and emits one source GDAT portrait command per
    occupied source-bound squad slot. Missing HeroType material retains the
    static source HUD and leaves only that portrait no-draw. Remaining: bind
    the original party/leader records, timers, and subsequent `GAME_LOAD`
    control flow before NEW may leave the title screen.
  - 2026-08-06 update: the real PC English corpus regression now opens the
    direct DM2 data root without extraction and locks its `PAL_IRGB` route to
    Greatstone's documented system palette for IMG9 raw 0174/0175 credits and
    menu. The M11 startup test also proves the original Credits click,
    countdown, and either-button dismissal with that global palette. This
    closes palette provenance for the static menu/credits route; interactive
    packaged-app capture and the wider `GAME_LOAD` flow remain open.
- DM2-014 — `skproject/SKULLWIN/c_savegame.cpp`, `c_loadlevel.cpp`, `c_gui_vp.cpp`, `c_sound.cpp`, `c_events.cpp`, and `c_creature.cpp` integration order: `tests/`, `probes/`, and the DM2 receipt chain lack a real-PC-corpus end-to-end proof from hash-verified assets through level load, source timers, material-complete rendering, resolved audio, and original save round-trip. Add skip-safe corpus gates before claiming playable parity.
  - 2026-07-23 update (Lane C, cycle 9): the DM2 V1 real-data tests that previously required explicit `DUNGEON.DAT` and/or `GRAPHICS.DAT` command-line arguments now resolve the canonical data root automatically: `argv[1]` overrides, otherwise `FIRESTAFF_DM2_DATA_DIR`, otherwise `$HOME/.firestaff/data/dm2/data`. Missing files cause a clean `SKIP: no local canonical DM2 data` exit instead of a build-only/failure state. Updated tests: `test_dm2_v1_g1_direct_*_runtime_real_data` (DB0 door, DB3 actuator, DB4 creature, DB5 weapon, DB9 container), `test_dm2_v1_g1_direct_root_*_real_data`, `test_dm2_v1_g1_direct_scene_classification_real_data`, `test_dm2_v1_g1_runtime_map_validation_real_data`, `test_dm2_v1_g1_scene_runtime_handoff_real_data`, `test_dm2_v1_world_model_g1_handoff_real_data`, `test_dm2_v1_g1_container_map_chip_real_data`, and `test_dm2_v1_g1_weapon_map_chip_real_data`. The twelve built executables pass against the canonical PC G1 corpus when data is present and skip cleanly when it is absent.
  - 2026-08-06 update: `test_dm2_v1_boot_profile_smoke` now also accepts
    `FIRESTAFF_DM2_DATA_DIR` as its read-only direct root before the legacy
    home-directory fallback. This runs the complete verified PC boot,
    GDAT-HUD, G1 dungeon-material, palette/light, and no-procedural-V2 route
    without copying, unpacking, or staging game data. Broader original
    SKSAVE parsing and the remaining runtime-state ownership still remain
    required before playable-parity claims.
  - 2026-08-06 update: the old `test_dm2_v1_save_load_real_data` no longer
    interprets arbitrary SKSave-header bytes as a champion name or looks only
    in the obsolete `dm2-extras` tree. It reads the configured corpus in
    place, verifies each authentic 42-byte DM2 header and the source-owned
    raw-dungeon prefix for `sksave0..3.dat/.bak`, and deliberately keeps the
    unbound SUPPRESS tail out of playable state.
  - 2026-08-06 update: the startup-menu action contract now exercises that
    same mounted real corpus. Its valid raw prefixes cannot create Continue
    or slot rows before the complete original `GAME_LOAD` stream is owned;
    the only available menu action remains New Game, which itself stays
    behind the original-data initialization gate.
- 🔧 Phase 4 - Mechanics parity: movement, interactions, shops/NPCs, doors, pressure plates, triggers, combat, magic, and timeline. **2026-06-16 combat parity landed:** `dm2_v1_combat.c` expanded from 30-line stub to full Phase 5 source-locked resolver. CTEST 49/49 + probe 13/13. **2026-06-17 spell + tech/magic parity landed:** `dm2_v1_spell.c` expanded with Phase 4 spell casting mechanics — `validate_runes()`, `mana_cost()`, `compute_chance()`, `can_cast()`, `cast_attempt()` returning full `DM2_SpellCastResult { success, mana_used, cooldown_ticks, skill_decay, effective_difficulty, effective_chance }`. Spell table updated with realistic `mana_per_rune` values (POTION=2-3, MISSILE=4-6, SUMMON=8, GENERAL=1-4) so the per-rune mana deduction contract matches DM2's SKULL.ASM behavior. `dm2_v1_tech_magic.c` expanded with `lookup()` (known-item catalog: crossbow/pistol/rifle/bomb/lantern/magic_battery/flame_orb/potions), `consume_charge()` (decrements available charges, supports unlimited -1), `hybrid_power()` (tech*25 + magic*25 capped at 100, returns 0 for non-hybrid items). Source-locked against skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER), 18159-18174 (ADD_RUNE_TO_TAIL), SkGlobal.cpp:966-1011 (dSpellsTable), SkGlobal.h:37-55 (MAXSPELL_ORIGINAL=34), SkWinCore.cpp:27038-27096 (spell→OBJECT_EFFECT), SKULL.ASM tech/magic item routines. CTEST `test_dm2_v1_spell_pc34_compat` 38/38 (spell table integrity, rune validation, mana cost formula, cast chance formula, cast attempt success/fail with cooldown + skill_decay, can_cast pre-check, OBJECT_EFFECT resolution, DM2-only spells, source evidence; tech/magic lookup, can_use respects tech/magic levels, hybrid requires BOTH, power cost manual/battery/mana/hybrid/empty, charge consumption including unlimited, hybrid power formula + cap). **2026-06-22 door/button toggle boundary landed:** new `dm2_door_apply_toggle_step(state, direction)` in `src/dm2/dm2_v1_door_mechanics.c` source-locked to ReDMCSB TIMELINE.C:803-806 (already-at-target early-out + single-tick ±1 step rule) + TIMELINE.C:750 (DESTROYED sticky) + DEFS.H:1039-1046 (door state C0..C5). Direction constants `DM2_DOOR_TOGGLE_DIR_OPEN` (0 ↔ C00_EFFECT_SET) and `DM2_DOOR_TOGGLE_DIR_CLOSE` (1 ↔ C01_EFFECT_CLEAR) match the EFFECT_* convention. New data-free CTest `test_dm2_v1_door_button_toggle_pc34_compat` 12/12 (4 single-direction advances to ±1 boundary, 2 sticky boundaries at OPEN and CLOSED, 2 DESTROYED-sticky cases, mid-state ±1 from CLOSED_1/2 in each direction, 8-step alternating round-trip CLOSED→CLOSED, pressure-plate DOOR_TOGGLE post-fire state 0..4, source-evidence citation). Companion DM2 V1 pressure-plate (`test_dm2_v1_pressure_plate_pc34_compat` 40/40) and trigger (`test_dm2_v1_trigger_pc34_compat` 32/32) tests still pass. **2026-07-22 DM2 V1 door-step timer wiring landed:** `DM2_V1_TIMER_STEP_DOOR` (0x01) now dispatches through the runtime source-order timer queue to a DM2-owned handler that mutates the dungeon square one state per tick and re-queues the next step until OPEN/CLOSED. Runtime counters (`door_step_timers`, `door_step_mutations`, `door_step_requeues`) and a public `DM2_V1_RuntimeDoorStepReceipt` expose the boundary. Smoke coverage in `test_dm2_v1_runtime_handoff_smoke` proves a CLOSED door steps 4→3→2→1→0 and stops. Source: skproject/SKULLWIN/c_tim_proc.cpp:4041 (0x01 dispatch), c_tim_proc.cpp:127+ (DM2_STEP_DOOR), ReDMCSB TIMELINE.C:750-810. **2026-07-23 DM2 V1 0x04 actuator tile subdispatch landed (Lane B, cycle 8):** added `DM2_V1_RuntimeActuatorTileReceipt`/`dm2_v1_runtime_actuator_tile_receipt()` and DM2-owned handlers for 0x04 classes 0 (wall mecha), 2 (pitfall), 4 (door), 5 (teleporter), and 6 (trickwall); class 1 stays on the existing CAII chain-walk path. Classes 0/2/4/5/6 bind at boot; class 1 stays gated on `think_binding_ready`. Pitfall toggles `FLOOR`↔`PIT` square type; door uses `dm2_door_apply_toggle_step` one step per tick; wall mecha/teleporter/trickwall are consumed, fail-closed counters. CTest `test_dm2_v1_proceed_timers_pc34_compat` and `test_dm2_v1_runtime_handoff_smoke` both pass (176/176 smoke). Remaining Phase 4 work: broader timeline timer matrix (remaining timer types), shops/NPCs.
- 🔧 Phase 5 - Creature/combat parity: creature AI table (64 entries with names + AI flags, 352-line implementation in `dm2_v1_creature.c` with spawn/tick/death_check) + combat resolver (now Phase 5-locked above) are source-locked. **2026-06-17 projectile routing + death sound landed:** new `dm2_v1_projectile_pc34_compat.c/h` provides the DM2→DM1 projectile bridge — maps DM2 creature `AttacksSpells` flags (12 bits: SHOOT/FIREBALL/LIGHTNING/DISPELL/POISON_CLOUD/POISON_BOLT/POISON_BLOB/PUSH_BACK) to DM1 `PROJECTILE_CATEGORY_*` + `PROJECTILE_SUBTYPE_*` via `dm2_v1_projectile_pick_category()`, then dispatches via F0810_PROJECTILE_Create_Compat. Three dispatch entry points: `dm2_v1_projectile_dispatch()` (auto-pick from creature AI flags), `dm2_v1_projectile_dispatch_spell()` (CCM 0x15 CAST_SPELL explicit subtype), `dm2_v1_projectile_dispatch_bomb()` (DM2 new area-effect). Plus magic-number fix in `dm2_v1_creature.c`: creature death sound now uses `DM2_SOUND_CREATURE_DEATH` constant instead of hardcoded `0x11`. New accessor `dm2_v1_creature_get_instance()` exposes creature pool read-only to the projectile module. Source-locked against SKULL.ASM:10620-10710 (SKULL_COMBAT_ResolveRanged), 11100-11200 (projectile routing), ReDMCSB PROJEXPL.C:76-92 (F0212), GROUP.C:1695-1770 (F0207 creature attack), skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE). CTEST `test_dm2_v1_projectile_pc34_compat` 23/23 (all 7 attack-flag → category mappings, dispatch invalid/dead/melee-only rejection, archer guard + amplifier dispatch, spell + bomb dispatch, 3 observability counters, reset, source evidence, magic-number constant check). **2026-06-22 projectile-vs-creature collision gate landed:** new `dm2_v1_projectile_creature_collision_pc34_compat.c/h` resolves the DM2-specific missile-redirect dispatch when a live projectile reaches a square with a creature instance. 5-branch priority order: NONMATERIAL > ABSORBS_MISSILE > REFLECTOR > TURNS_MISSILE > HIT. Deterministic damage formula `max(1, impact_attack - armor_class/2)`; HIT/ABSORBED/REFLECTED despawn the projectile, and tests/probe cover each branch plus invalid slot/source evidence. **2026-06-28 projectile step/drain gate landed:** the runtime now advances the Firestaff DM2 projectile cache once per tick, consumes per-slot kinetic energy with the one-step grace boundary, despawns drained slots through the same observable path, and rebuilds the M11 drain view from post-step survivors. Remaining work: advanced CCM (`DM2_PROCEED_CCM`) full implementation, full cell-content digest/map-change/teleporter effects, and broader real-route runtime evidence.
### DM2 V2.0 / V2.1 / V2.2

- DM2-015 — `skproject/SKULLWIN/c_gfx_pal.cpp`, `c_gfx_blit.cpp`, `c_gui_vp.cpp`, and `c_gui_draw.cpp`: `src/dm2/dm2_v2_*.c` and `dm2_v22_*.c` use enhanced/procedural or externally authored presentation assets without a complete verified-original V1 material input. Keep V2 modes gated behind the V1 original-data route, replace placeholder/stamp fallbacks with explicit unavailable states, and prove every displayed source-derived surface and palette handoff.
- 🔧 Phase 2 - Enhanced asset pipeline: `dm2_v2_asset_pipeline.c` (V2.1 EPX + V2.2 modern-asset fallback chain) is source-locked against SKULL.ASM T520/T560/T580/T600 + ReDMCSB DUNVIEW.C:575-586/148-157/2962-3047/3048-3070/3082-3095/3940-4015/4016-4050/4119-4270 + PANEL.C:418-428 + DATA.C:359-360; probe `firestaff_dm2_v2_phase2_asset_pipeline_probe` is green. **2026-06-19 DM2 V2.2 modern-asset module landed:** new `dm2_v22_modern_assets_pc34.c/.h` mirrors dm1/csb/theron/nexus modules with DM2 paths (`~/.firestaff/assets/dm2/modern/`) and DM2 source-locks (SKULL.ASM T520/T560/T600 + ReDMCSB DUNVIEW.C:2962-3047 outdoor). Ctest `test_dm2_v22_modern_assets_pc34` 33/33. **2026-06-19 DM2 V2.2 first-cut asset pack landed:** `.openclaw/tmp/dm2_v22_asset_author.py` (5 PNGs + manifest v1.0.0). Smoke: `dm2_v22_modern_assets_available()=1` end-to-end. **2026-06-29 T560 indoor route gate landed:** `DM2_V22_T560IndoorRoute` exposes all nine indoor D0..D2 x L/C/R route names, raw-cell discriminators, category/asset ids, clipped rects, and active/no-op state; `firestaff_dm2_v22_inplace_render_probe` is now 33/33 PASS with a synthetic cache and cache-type mismatch rejection. Remaining: real PBR hero art for DM2 via gpt-image-2 batch, real-runtime wire-up of `dm2_v22_viewport_swap_render()` from the DM2 V2 viewport draw path, outdoor T600 route-depth follow-up beyond the existing 3-cell synthetic paint, and per-mode pixel/material verification gates.
- 🔧 Phase 3 - Enhanced UI overlays: **2026-06-16 HUD runtime wire-up landed (this pass):** new `dm2_v2_hud_runtime.c/h` provides the V1→V2 HUD bridge layer (mirrors `csb_v2_hud_runtime.c`). API: init/shutdown, set_gate_config, set_party_gold, set_direction, set_level, set_champion, set_action_active, trigger_hit_flash, set_opacity, render (gated on DM2_V2_PHASE_DOMAIN_HUD, V1 framebuffer preserved when V1 active), is_active, force_active_for_test. Source-locked against SKULL.ASM T560, skproject/SKULLWIN/c_gui_vp.cpp, ReDMCSB PANEL.C F0354, DUNGEON.C F0260, COMMAND.C, DISPLAY.C, dm2_v2_phase_gate.h. **M11 wire-up:** `firestaff_game_loop.c` (src/engine) now calls `dm2_v2_hud_runtime_render(g_framebuffer, 320, 200)` right after the DM2 V2 smooth-movement viewport render, gated on phase gate (no-op when V1 active, no V1 chrome pollution). CMakeLists: `firestaff_dm2_v2` linked from `firestaff_m11`. Probe `firestaff_dm2_v2_hud_runtime_probe` 23/23 (init/shutdown, all 7 setters, gated render is no-op when V2 off, paints into fb when V2 on, opacity=0 short-circuits, force_active_for_test bypass, V1 framebuffer preserved, champion bar pixels, action strip pixels, null-fb safe, source evidence). Remaining work: actual HUD text/bitmap assets, more HUD widgets (inventory quick-view, action prompt).
### DM2 CLI launch

- DM2-016 — `skproject/SKULLWIN/main.cpp`, `fileio.cpp`, and `c_gdatfile.cpp`: `src/shared/asset_status_m12.c`, `src/dm2/dm2_v1_boot.c`, and CLI launch still need corpus-verified classification/materialisation for every supported PC variant and valid container before entering DM2. Preserve hash-based discovery, but reject demo, incomplete, or cross-version mixes before boot rather than normalising them into a generic launch profile.
- 🔧 DM2 extras/cross-version launch remains open for demo and non-PC extracted paths that need separate version classification/container normalization.
  **2026-08-07 PC-9821 catalog correction:** the authenticated retail
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair is now represented as `pc9821-ja` in the
  launcher catalog, separate from the PC-9801 demo. Its required dungeon hash
  follows the graphics-selected pair from `dm2_v1_boot.c`; other non-PC
  variants remain separately gated until their catalog and runtime owners are
  proven.


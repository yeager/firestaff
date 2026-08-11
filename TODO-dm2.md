# Firestaff TODO - DM2

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## Active Cycle 16 Jobs (DM2 only — continuous operation)

Per directive: DM2 only, auto mode. Lanes pull jobs from this file; the
orchestrator keeps them fed, assembles, and pushes. Fix synthetic paths when
real game data is available; batch small jobs into larger ones. Source-lock
against skproject (SKULLWIN/SKWIN); keep fail-closed where evidence is
missing. Do not push — the orchestrator pushes after assembly. Update this
file and DONE.md after every completed job.

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

- SKPROJECT-GAP-008 — **Title/menu GAME_LOAD is now split by route.** The
  authenticated FM Towns CD path has a verified `SHOW_MENU_SCREEN` pointer
  route and a source-owned NEW GAME transaction: the real DUNGEON.DAT/GDAT
  pair is admitted, an authentic mirror is selected, and M11 receives the
  committed session before the first active frame. This is covered by the
  opt-in `dm2_fmtowns_m11_gameplay_real_media` regression. Resume remains a
  separate source boundary: the FM Towns corpus contains no SKSAVE, so no
  save-derived party or dungeon may be invented or imported from the DOS
  edition.

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
that verified image route; do not derive overlays from Firestaff weather
enums or intensity.
live real-material renderer. Remaining V2.2 work is a renderer that consumes
clipping is still unproven. Remaining weather work is to bind real original

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- DM2-001 — `skproject/SKULLWIN/c_gdatfile.cpp` GDAT query/load path and `c_loadlevel.cpp` level materialisation: the hash-verified DOS EN/FR shared dungeon member is discovered and materialized through the normal scanner, and its typed GDAT ENT1 payload graph validates. PC G1 parsing bounds the real pre-map extension and exposes the proven `c_map.cpp` route: its 256-byte post-descriptor G1 block precedes the 480-word column-prefix table, which reaches the bounded 2360-word ground-stack table. The source-ordered `c_record.cpp` pool transform and DB3/DB4 continuation addresses are proven.

- DM2-002 — `skproject/SKULLWIN/c_dballoc.cpp`, `c_record.cpp`, `c_map.cpp`, and `c_moverec.cpp` database-record ownership: `src/dm2/dm2_v1_world_model.c`, `dm2_v1_world_state.c`, and `dm2_v1_runtime.c` retain reduced Firestaff records, including a stub save-state layout. Replace the parallel model with validated original record pools, links, maps, and relocation semantics.
  - **2026-08-05 c_move inventory correction:** the former
    `DM2_move_075f_1bc2` target-cell and `DM2_move_2c1d_028c` commit receipts
    were synthetic. In SKProject `c_move.cpp:2861` selects four candidate
    player positions using party state and `DM2_RANDBIT`; `:2914` searches an
    adjacent party member and returns its index or `-1`. Neither routine is
    collision nor movement commit. Both adapters now reject explicitly until
    the real party-position, RNG, and caller state are bound. Keep collision
    in the separately source-scoped runtime route; do not reuse these names
    to admit a DUNGEON.DAT movement result.

- DM2-003 — `skproject/SKULLWIN/c_timer.cpp`, `c_tim_proc.cpp`, `c_events.cpp`, and `c_eventqueue.cpp` timer order: `src/dm2/dm2_v1_timeline.c`, `dm2_v1_runtime.c`, `src/memory/`, and `src/engine/m11_game_view.c` do not execute the original timer-type matrix and still contain an M11 creature-tick simulation. Route every DM2 timer through a DM2-owned source-order dispatcher and remove host-side behavioural substitution.

  - 2026-07-21 update (round 23): the event-driven activation callers
    actuator subsystem), and the 1c9a_0247 dballoc tag system
    (host-owned preserved-GFX cache).
    game assets), zero new failures.  Remaining: runtime wiring of the

- DM2-004 — `skproject/SKULLWIN/c_input.cpp`, `c_keybd.cpp`, `c_tmouse.cpp`, `c_clickrect.cpp`, `c_buttons.cpp` UI event routing: `src/engine/m11_game_view.c`, `src/dm2/dm2_v1_startup_menu.c`, and `dm2_v1_inventory_panel.c` cover only bounded menu/viewport actions. The original `INTERFACE_GENERAL dt07/2` group spans are now materialized as typed primary/secondary/tail data; default door-button receipts now expose skproject `MAKE_BUTTON_CLICKABLE` rectnos 3/4 and reject custom wall-GFX buttons as non-clickable. The title-menu NEW path expands original `INTERFACE_GENERAL/0/dt04/0` rectangle `0xD7` and consumes it through M11; the hard-coded startup panel no longer accepts M11 clicks. The matching `0xD9` surface has a source-owned pointer receipt and is explicitly selector-unavailable, so it cannot fall through into a synthetic resume row. Both title actions now require the original primary mouse event; only the separate credits screen retains its common secondary-button dismissal. The title/menu indexed presentation now expands `dtPalIRGB`'s source 6-bit DAC channels to SDL's 8-bit RGBA after `DM2_CONVERT_DRIVERPALETTE`, while retaining raw GDAT palette bytes for receipts. Bind the original resume-selector state machine before it can create a resume action. Consume the remaining original click-rectangle, keyboard, mouse, held-button, and modal-dialog ordering. Unsupported controls must remain unavailable.

- DM2-006 — `skproject/SKWIN/c_creature.cpp` AI/death paths and `c_ai.cpp`: the bounded real-data chain `CREATURES[type] dtWordValue(0x05) -> CREATURE_AI row -> AIDefinition.w30/w32` is available as evidence for `DRAW_PUT_DOWN_ITEM`; it preserves the source w30 eligibility gate and still does not create a click target until owner records and rect expansion are both proven. Bind real GDAT AI records and reproduce source eligibility, possession, death, and cooldown ordering.

  - 2026-07-23 update (Lane E, cycle 16): the real-data drop route is now
    up the defense/BaseHP route, DUNGEON.DAT door-record evidence for the
    door-destruction table, and ALLOC_NEW_DBITEM item-record creation.
    locally. Remaining: a CREATURE_AI-proven graphics session to light

- DM2-007 — `skproject/SKULLWIN/c_events.cpp` `DM2_TRY_CAST_SPELL`, `DM2_FIND_SPELL_BY_RUNES`, `DM2_CAST_SPELL_PLAYER`, and `DM2_PROCEED_SPELL_FAILURE`: `EXTENDED_LOAD_SPELLS_DEFINITION` is a bounded GDAT `SPELL_DEF` receipt over exact dtWordValue fields 1-7 plus dtText field `0x18`. The fixed original table and live rune lookup are now source-exact, but DB object-effect resolution, projectile creation, timer effects and final UI feedback remain unbound. Unsupported object effects must remain unavailable rather than use a spell-index mapping.

  - 2026-08-06 update: M11 binds the SDL backend only after
    `dm2_v1_boot_startup_launch_alloc_with_language()` succeeds, then
    unbinds it in `M11_GameView_Shutdown()`. The real-data M11 startup gate
    now covers both sides, preventing a verified DM2 backend from leaking
    into an unverified or later game launch.

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
    those four hand backdrops became source-bound and fail-closed. At that
    point it remained unwired from normal gameplay because the live original
    champion formation, possession and hand-selection state had not yet been
    recovered; M11 could not infer that tuple from party order, pointer
    position or a Firestaff default.

  - 2026-08-11 update: GAME_LOAD now retains the source-owned
    `party.curacthero` and `party.curactmode` fields and the Towns renderer
    binds the selected champion's authentic action image when that state is a
    valid source selection. The source `handcooldown[hand]` byte now drives
    the original checker-pattern overlay, and the selected hand's real record
    link resolves through the authenticated pools to its real item image when
    command text resolves. Positive `CnNC` requirements now read the source
    record `w2` with exact `ADD_ITEM_CHARGE(object, 0)` semantics, including
    the special 16/17/18 cases; the probe never mutates the mounted pool. The
    source sleep/wake input now owns the one-bit overlay state; the full sleep
    tick cadence and charge-consuming action mutation remain. DB9
    `ContainerType()==0` hand items now follow the source's separate
    `IS_CONTAINER_MONEYBOX`/`IS_CONTAINER_CHEST` admission branch, including
    the authentic `CONTAINERS/cls2/dtText/0x40` lookup; no command-entry or
    charge requirement is invented for those containers. No hand selection is
    fabricated when the source fields are clear.

  - 2026-08-11 follow-up: the source `ACTIVATE_ACTION_HAND` state transition
    is now exposed through `dm2_v1_runtime_activate_action_hand`. It accepts
    only a live hero retained by the authenticated GAME_LOAD candidate and
    updates `party.curacthero/curactmode` together. The real FM-Towns M11
    gameplay regression exercises both hand selections. The native Towns
    rectangle/event owner is still required before pointer clicks can invoke
    this transition, and command-specific action execution remains fail-closed
    when the selected hand has no authenticated action entry.

  - 2026-08-11 follow-up: authenticated GAME_LOAD now exposes one source
    inventory transaction seam. The runtime exchanges a real
    `c_hero::item[30]` link with `LeaderPossession`, validates both links
    against the admitted record pool, preserves the `OBJECT_NULL` sentinel
    boundary, verifies the read-back, and rolls back on failure. The native
    inventory panel, remaining context/event ordering, and pouch/quiver/
    scabbard/backpack ownership are still open and remain unavailable.

  - 2026-08-11 follow-up: FM Towns M11 now admits the real
    `INTERFACE_CHARSHEET/0/dtImage/1` inventory frame after validating its
    global 255-colour source route, raw material receipt, decoded IMG3/U4
    pixels, and `RECT_1EE` RAW4 crop placement. The authenticated Towns
    inventory context table is now consumed by M11 for panel pointer routing
    and slot selection. Item movement/equip commits, source text, and the
    remaining non-equipment owners are still separate fail-closed work.

  - 2026-08-11 follow-up: the authenticated Towns `MOUSE_INPUT` event 71
    (`rect 0x8222`, source group `inventory.eye`) is now identified and
    routed through the source champion/status context. The current loose
    Towns corpus does not expose an authenticated RAW4 rectangle for this
    record, so M11 remains fail-closed and does not claim a clickable eye
    until that native geometry is recovered. Mouth/consume, status, moneybox,
    save, sleep/wake, and the remaining inventory owners remain unavailable
    until their live source state and native geometry are bound.

  - 2026-08-06 follow-up: command dispatch now propagates a rejected
    source-GDAT image callback, so a failed title/menu blit aborts the
    presentation transaction instead of being reported as successfully drawn.

  - 2026-08-06 update: the real PC English corpus regression now opens the
    direct DM2 data root without extraction and locks its `PAL_IRGB` route to
    Greatstone's documented system palette for IMG9 raw 0174/0175 credits and
    menu. The M11 startup test also proves the original Credits click,
    countdown, and either-button dismissal with that global palette. This
    closes palette provenance for the static menu/credits route; interactive
    packaged-app capture and the wider `GAME_LOAD` flow remain open.

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

  - **FM Towns save boundary:** the mounted FM Towns corpus contains the
    authenticated CD/runtime media but no FM Towns `SKSAVE` artifact. The
    external `Downloads/dm2` corpus now supplies eight authentic DOSBox
    `sksave0..3.dat/.bak` files. The real-data loader verifies all eight DOS
    files and 269 source-boundary checks pass, but they are still a different
    platform and are not evidence for a Towns save writer. `DM2_GAME_SAVE`
    and full `GAME_LOAD` ownership therefore remain fail-closed for FM Towns
    until an authentic Towns save is available and the platform-specific
    stream passes a copied load/write/load regression. Do not use the DOS
    files or a generated fixture to close this gate.

  - **2026-08-11 FM Towns HUD receipt:** the native v4 `CHAMPIONS` portrait
    route is covered for every authentic type 0..15 by
    `test_dm2_v1_fmtowns_hud_portraits_real_data`. This closes portrait
    material binding only; source champion selection, full inventory/dialogue
    semantics, and FM Towns SKSAVE ownership remain separate gaps.

  - **2026-08-11 FM Towns pointer subset:** the authenticated Towns
    `INTERFACE_GENERAL` RAW4 table now owns source movement events 1..6 and
    champion action-hand selection events 116..123. The M11 route converts
    the native 640x400 rectangles to the 320x200 presentation surface and is
    covered by `test_dm2_fmtowns_m11_gameplay_real_media`. This does not claim
    inventory/dialogue pointer ownership, or a complete viewport interaction
    map; those remain fail-closed until their source event/rectangle mappings
    are recovered. The three action-panel pointer events now reach the
    existing CMDSTR-backed command owner, but a command still fails closed
    when its authenticated item/action record is not admissible. The native
    viewport event 0x50/rect 0x0007 reaches the DM2 c_rwbb target resolver
    and no longer falls through to DM1's C080 front-cell/door/mirror handler.
    Sensor/object mutation remains fail-closed until the corresponding DM2
    c_events owners are bound to runtime state.

  - **2026-08-11 FM Towns input identity hardening:** the pointer owner now
    requires both the authenticated Towns `GRAPHICS.DAT` and the native
    `SKULL.EXP` MD5 (`0f4b44d286cbee35924a95e7d75ad7e5`). It also verifies the
    disassembled `SKULL.EXP` MOUSE_INPUT anchor for events `0x70..0x72`
    (`0x003b/0x003f/0x0040`) and the complete 264-record table span
    (FNV-1a `0x1500c4c9`) before enabling the existing pointer subset.
    The full source table is context-sensitive: the same rect IDs are reused
    by inventory, status, and action-panel branches. Those branches still
    require a source UI-context owner and are not promoted by geometry alone.

  - **2026-08-11 FM Towns MOUSE_INPUT receipt:** after authenticating the
    `SKULL.EXP` identity, the boot profile now retains the complete 264-record
    (1584-byte) source span in memory. The input owner exposes each raw
    event/flag/rect/mask candidate with its original record ordinal and
    re-hashes the retained bytes before returning them. This is an evidence
    API for the next context-bound UI owners; it does not make the candidates
    globally clickable, because Towns reuses rectangle IDs across branches.
    Route ordinal `117` is now bound back to the source `hand_panel.action_1`
    context, and source event `0x70`/112 (rune-quit) closes the active action
    panel through the M11 owner. The remaining context-specific candidates
    are still inventory/status/dialogue work, not generic hitboxes.

  - **2026-08-11 FM Towns dungeon-context guard:** the live dungeon pointer
    route now consults the authenticated source context inventory before
    emitting an event. A native rectangle shared with inventory, status, or
    dialogue is not sufficient by itself; records without a dungeon context
    remain unavailable until their own live owner is bound. This closes the
    cross-view hit-test leak without borrowing PC geometry or inventing a
    replacement control.

  - **2026-08-11 FM Towns inventory layout census:** the native RAW4 bridge
    now resolves 129 of the 166 source inventory route contexts. Ordinals
    47-49, 52-83, 99, and 110 have no matching rectangle in the authenticated
    Towns RAW4 set. They remain fail-closed; no PC rectangle or replacement
    control is substituted.

  - **2026-08-11 FM Towns M11 event bridge:** the authenticated Towns
    MOUSE_INPUT route now reaches the source panel-close event 11 after the
    full SKULL.EXP table receipt and native rectangle admission. The generic
    c_input sleep/wake callbacks remain available, but event 142/143 are not
    present in the authenticated 264-record pointer span and are therefore
    not claimed as pointer routes. The seven source-explicit equipment slots
    now commit through the authenticated item-slot transaction when their
    native pointer context is clicked. Rune, moneybox, status, and
    non-equipment inventory mutations remain fail-closed until their original
    record-chain owners are recovered.

  - **2026-08-11 source-session inventory links:** source-complete GAME_LOAD
    sessions now expose and update `c_hero::item[30]` through the runtime
    inventory API. Each non-empty write is checked against the admitted DB
    record pool; the old 32-bit host cache remains unavailable before a real
    source session. Raw DOSBox SKSave files still stop at the authenticated
    pre-link GAME_LOAD boundary and cannot be promoted to a playable resume.

  - **2026-08-11 FM Towns explicit UI-context routing:** the authenticated
    264-record `MOUSE_INPUT` receipt can now resolve a pointer through an
    explicitly selected source branch (dungeon, inventory, status, or
    dialogue). The route returns both the source semantic context and the
    native `INTERFACE_GENERAL` RAW4 rectangle; it never promotes shared
    rectangle IDs to global hitboxes or reuses PC geometry. The M11 dungeon
    owner continues to expose only the dungeon branch. Inventory/status/
    dialogue mutation owners still need to consume these receipts before
    those controls become playable.

  - **2026-08-11 FM Towns equipment-slot provenance:** the seven unambiguous
    source inventory groups (`hand_right`, `hand_left`, `head`, `body`,
    `legs`, `foot`, and `neck`) now decode to the original slots 0 through 5
    and 10 from ReDMCSB `DEFS.H`. Pouch, quiver, scabbard, and backpack groups
    remain unavailable; their ownership is not inferred from screen labels or
    PC geometry.

- 🔧 Phase 5 - Creature/combat parity: creature AI table (64 entries with names + AI flags, 352-line implementation in `dm2_v1_creature.c` with spawn/tick/death_check) + combat resolver (now Phase 5-locked above) are source-locked. **2026-06-17 projectile routing + death sound landed:** new `dm2_v1_projectile_pc34_compat.c/h` provides the DM2→DM1 projectile bridge — maps DM2 creature `AttacksSpells` flags (12 bits: SHOOT/FIREBALL/LIGHTNING/DISPELL/POISON_CLOUD/POISON_BOLT/POISON_BLOB/PUSH_BACK) to DM1 `PROJECTILE_CATEGORY_*` + `PROJECTILE_SUBTYPE_*` via `dm2_v1_projectile_pick_category()`, then dispatches via F0810_PROJECTILE_Create_Compat. Three dispatch entry points: `dm2_v1_projectile_dispatch()` (auto-pick from creature AI flags), `dm2_v1_projectile_dispatch_spell()` (CCM 0x15 CAST_SPELL explicit subtype), `dm2_v1_projectile_dispatch_bomb()` (DM2 new area-effect). Plus magic-number fix in `dm2_v1_creature.c`: creature death sound now uses `DM2_SOUND_CREATURE_DEATH` constant instead of hardcoded `0x11`. New accessor `dm2_v1_creature_get_instance()` exposes creature pool read-only to the projectile module. Source-locked against SKULL.ASM:10620-10710 (SKULL_COMBAT_ResolveRanged), 11100-11200 (projectile routing), ReDMCSB PROJEXPL.C:76-92 (F0212), GROUP.C:1695-1770 (F0207 creature attack), skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE). CTEST `test_dm2_v1_projectile_pc34_compat` 23/23 (all 7 attack-flag → category mappings, dispatch invalid/dead/melee-only rejection, archer guard + amplifier dispatch, spell + bomb dispatch, 3 observability counters, reset, source evidence, magic-number constant check). **2026-06-22 projectile-vs-creature collision gate landed:** new `dm2_v1_projectile_creature_collision_pc34_compat.c/h` resolves the DM2-specific missile-redirect dispatch when a live projectile reaches a square with a creature instance. 5-branch priority order: NONMATERIAL > ABSORBS_MISSILE > REFLECTOR > TURNS_MISSILE > HIT. Deterministic damage formula `max(1, impact_attack - armor_class/2)`; HIT/ABSORBED/REFLECTED despawn the projectile, and tests/probe cover each branch plus invalid slot/source evidence. **2026-06-28 projectile step/drain gate landed:** the runtime now advances the Firestaff DM2 projectile cache once per tick, consumes per-slot kinetic energy with the one-step grace boundary, despawns drained slots through the same observable path, and rebuilds the M11 drain view from post-step survivors. Remaining work: advanced CCM (`DM2_PROCEED_CCM`) full implementation, full cell-content digest/map-change/teleporter effects, and broader real-route runtime evidence.

### DM2 V2.0 / V2.1 / V2.2

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

# DM2 PC-DOS File_header continuation and champion activation (2026-08-07)

- [ ] Derive the PC-DOS record/map continuation after the 44-entry
  `File_header` from an original-loader trace. The former 28-map pseudo-header
  accidentally produced 16 champion mirrors and a DYN4 selection; it is not
  valid evidence and must not be restored. Champion selection remains gated
  until the real DB3/DB4 ownership and marker route are independently proven.

## DM2 Macintosh support

DM2 Macintosh is a separate 68k platform family, not a DOS or FM Towns
asset variant. The local corpus contains authentic English and Japanese Mac
archives, a French StuffIt image, and Mac-specific `GRAPHICS.DAT` fingerprints.
The large English retail ZIP and the smaller English "The First Chapter" demo
are admitted and read without extracting game files. Firestaff reads the raw
MODE1/2352 BIN, Apple Partition Map and HFS catalogue directly into RAM; the
demo additionally walks its genuine StuffIt 2 `DMFiles` member in RAM. The
retail and demo use separate hash-paired big-endian dungeon receipts.

- [x] Admit the authentic large US English retail ZIP independently as
  `mac-en-retail`; keep its HFS container as the runtime owner and never
  extract its game files to the staging directory.
- [x] Admit the smaller US English "The First Chapter" demo independently.
  The authentic installer is read in RAM, its `DMFiles/Graphics.dat` and
  6,535-byte `Dungeon.dat` are hash-verified, and its truncated big-endian
  File_header/map data enters the real dungeon loader. The leading bytes are
  the dungeon header, not a payload to expand.
- [ ] Admit the authentic Japanese 1.0 and French StuffIt editions
  independently once their container/resource-fork readers are verified.
- [ ] Add a source-owned Mac container/resource-fork reader for the verified
  CD/content archives, including StuffIt/HQX/resource-fork provenance. The
  reader must preserve the original file/resource identity and fail closed on
  flattened or ambiguous input.
- [ ] Bind the Mac big-endian `DUNGEON.DAT` and `GRAPHICS.DAT` pair to one
  platform-specific boot receipt. Japanese 16-colour and US English
  256-colour graphics must remain separate layouts and hashes.
- [ ] Extract and present the authentic Macintosh QuickTime `MooV` movies
  (`TITLE`, `STORY`, `SWOOSH`, `CREDITS`, and `ENDING`) from data/resource
  forks. Do not replace them with converted MP4 files in the source runtime;
  converted files may be verification derivatives only.
- [ ] Bind the US English Mac MIDI/SoundMusicSys resources and the Japanese
  CD-audio route separately. DOS HMP, FM Towns CD.DAT, and Amiga MOD paths are
  not fallbacks.
- [x] Add the source-locked English Macintosh keyboard/menu table for both
  admitted US versions. `dm2_v1_mac_input` covers champion/leader inventory,
  movement, freeze, Command-O/S/Q, entrance New, credits close, and the three
  wall-button columns. Queue-compatible actions are forwarded to the existing
  command boundary; Mac-only actions remain explicit and unavailable until a
  native Mac dispatcher owns them. Source: DMWeb Macintosh edition page.
- [~] Bind the Mac input table to the native M11/SDL runtime dispatcher. The
  admitted Mac profile now takes precedence over PC aliases for the English
  retail/demo gameplay route, and movement, champion inventory, leader
  inventory, freeze, wake, save and quit reach the existing M11 boundaries.
  Held-button timing, balloon help, and the three Mac wall-button actions
  still need their original runtime owners; wall actions remain explicit and
  fail closed instead of becoming a synthetic attack.
- [ ] Acquire an authentic Mac save corpus for both language families and
  verify native load/save round trips. A DOSBox `SKSAVE` or a generated save
  cannot close this gate.
- [ ] Add end-to-end Mac startup, viewport, inventory-cursor, movie, audio,
  input, save/load, and pixel/audio regression gates before claiming Mac
  gameplay support.

Required evidence: hash-identified Mac CD/content media, resource-fork
receipts, original Mac or emulator traces for menu/input/audio/movie timing,
and at least one authentic save per claimed edition.

# V1 Blocker Ledger — Firestaff DM1/V1 original-faithful mode

Last updated: 2026-04-25 (pass 58 landed)
Owner of this file: Pass 36 "honesty lock" (see `PASSLIST_29_36.md` §4.36)
Primary consumers: pass-38+ planning, `STATUS.md`

This is the **single source of truth** for what is still outstanding
between Firestaff and DM1 PC 3.4 English in V1 original-faithful mode
after Passes 29–35.  Every entry contains:

Reference sources that should be consulted whenever they materially unblock a blocker:
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/graphics.dat/graphics.dat.html`
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/dungeon.dat/dungeon.html`
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/title/title.html`
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/end/end.html`
- `http://greatstone.free.fr/dm/db_data/dm_pc_34/song.dat/song.dat.html`
- `http://dmweb.free.fr/community/redmcsb/`
- `http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z`
- `http://dmweb.free.fr/Stuff/ReDMCSB/Documentation/BugsAndChanges.htm`
- Local unpacked ReDMCSB archive:
  - `/Users/bosse/.openclaw/workspace-main/ReDMCSB_WIP20210206`
  - local Firestaff-facing analysis: `REDMCSB_LOCAL_ANALYSIS_2026-04-24.md`
  - current V1 target label: `I34E` = DM PC 3.4 English; `I34M` = multilingual PC 3.4 comparison target only
  - source roots: `Toolchains/Common/Source` plus `Toolchains/IBM PC/Source`
  - binary facit: `Reference/Original/I34E`; rebuilt/non-perfect comparison: `Reference/ReDMCSB/I34E/FIRES`

Use `BugsAndChanges.htm` to avoid treating ReDMCSB-only fixes as original DM1 PC 3.4 parity, and remember that ReDMCSB's broader DM/CSB source coverage is useful for later-version reconnaissance but does not override the DM1 PC 3.4 target in this ledger.

Critical local-analysis guardrail: `Documentation/Engine.htm` and `Documentation/BugsAndChanges.htm` are explicitly Atari-ST-scoped. They can point to functions, limits, bugs, and source identifiers, but V1 blockers may not be closed from those docs alone; check the relevant `I34E` conditional/source path or original runtime before marking `MATCHED`.

Every entry contains:

- short title
- area tag (`OWNERSHIP`, `VISUAL`, `BEHAVIOR`, `ASSET`, `TEXT`)
- evidence path or reason it is a blocker
- suggested next bounded pass label (pass-37, pass-38, …)

Entries are numbered in priority order (lower number = higher priority
for V1 convergence).  Priority follows the 1:1 analysis plan: ownership
first, then visual parity, then typography / honesty.

---

## 1. Compat-runtime integration of sensor enter/leave events  — **LANDED (pass 37)**
- **Area:** `OWNERSHIP`
- **Status:** Resolved for emission-side wiring.  Remaining scope is
  world mutation for sensor-effect teleports, which stays on the
  existing F0704 tile-type teleporter path in v1.
- **Pass 37 (landed, 2026-04-24):**
  - `F0888_ORCH_ApplyPlayerInput_Compat` now invokes
    `F0718_SENSOR_ProcessPartyEnterLeave_Compat` for the pre-move
    square (`SENSOR_EVENT_WALK_OFF`) and the post-resolve final square
    (`SENSOR_EVENT_WALK_ON`) on every successful party move.
  - Each produced `SensorEffect_Compat` is surfaced as a new
    `EMIT_SENSOR_EFFECT` (kind `0x0D`) emission on `TickResult_Compat`
    with payload `[effect.kind, effect.sensorType, triggerEvent,
    textIndex|destMapIndex]`.
  - Pass-37 probe
    `run_firestaff_m11_pass37_sensor_runtime_wiring_probe.sh` drives
    the real orchestrator with a two-sensor destination and a
    one-sensor origin and verifies 13/13 invariants including source
    ordering, WALK_ON/WALK_OFF split, payload contents, and that
    turn-only moves produce zero emissions.
  - All baseline gates stay green: Phase A 18/18, game view 361/361,
    M10 verify 20/20 phases, M11 verify end-to-end.
- **What is still NOT landed by pass 37:**
  - Teleport sensor effects do not yet mutate world state through
    the sensor path; the v1 path covers tile-type teleporters via
    F0704.  Sensor-type teleports still emit as markers only.
  - Non-teleport / non-text sensor types still flow through
    `SENSOR_EFFECT_UNSUPPORTED` by F0710 (unchanged since pass 32).
  - Creature movement does not invoke `F0718` (party-only scope,
    consistent with pass 32 §4.32.5).

## 2. Animating door states (1..3) still snapped to 0/4 — **LANDED (pass 38)**
- **Area:** `OWNERSHIP`
- **Status:** Resolved for the party-toggle path.  Animating
  intermediate states (1..3) are now owned by the compat layer via
  a timeline-driven stepper modeled on
  `F0241_TIMELINE_ProcessEvent1_DoorAnimation`.
- **Pass 38 (landed, 2026-04-24):**
  - `memory_door_action_pc34_compat.c/h` gained three new owners:
    `F0714_DOOR_ResolveAnimationEffect_Compat` (TOGGLE/SET/CLEAR
    resolution against current state), `F0713_DOOR_BuildAnimationEvent_Compat`
    (builds a `TIMELINE_EVENT_DOOR_ANIMATE` event with aux1=effect
    and fireAtTick=startTick), and `F0712_DOOR_StepAnimation_Compat`
    (walks one state per invocation: SET decrements 4→3→2→1→0,
    CLEAR increments 0→1→2→3→4; DESTROYED never animates).
  - `memory_tick_orchestrator_pc34_compat.c:F0887_ORCH_DispatchTimeline
    Events_Compat` now dispatches `TIMELINE_EVENT_DOOR_ANIMATE` through
    `F0712_DOOR_StepAnimation_Compat`, emits a `EMIT_DOOR_STATE` per
    step, emits a rattle `EMIT_SOUND_REQUEST` on every non-final step,
    and reschedules the event at `gameTick+1` until the final state
    is reached.  A defensive legacy-marker fallback keeps the M10 tick
    orchestrator probe’s invariant-19 (queued DOOR_ANIMATE dispatch
    on a dungeon-less unit world) green.
  - `m11_game_view.c:m11_toggle_front_door` no longer writes door
    state bits directly.  It resolves the effect, schedules the
    animation event, and advances the world by one tick through the
    real orchestrator; status/inspect text is now “DOOR OPENING” /
    “DOOR CLOSING” to reflect that subsequent ticks finish the walk.
  - Pass-38 probe
    `run_firestaff_m11_pass38_door_animation_probe.sh` drives the
    new owners directly and through the orchestrator; verifies 29/29
    invariants including the full 4-tick SET walk (4→3→2→1→0), the
    full 4-tick CLEAR walk (0→1→2→3→4), emission counts, rattle
    counts, reschedule-then-stop behavior, and idempotency at target.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases
    (including M10 tick orchestrator invariant 19), M11 verify
    end-to-end.
- **What is NOT covered by pass 38:**
  - Hazard branches in F0241: champion damage from a closing
    vertical/horizontal door on the party square, creature damage /
    kills from a closing door on a creature square.  The Pass 38
    stepper intentionally omits these; the BUG0_78 mask-parenthesis
    subtlety is not re-litigated here.
  - Sensor-driven door actuation re-entering the animation scheduler
    (still routed through the M11 shim in Pass 31 scope).  When a
    sensor effect of teleport/text fires on enter/leave (Pass 32/37
    emission path), it does not yet trigger a door-animation event.
  - Projectile-caused door *destruction* already flows through
    `TIMELINE_EVENT_DOOR_DESTRUCTION` and is unchanged.
- **Follow-up:** hazard branches + sensor-driven door actuation
  interaction are candidates for a future pass but are not tracked
  as a V1 blocker; the ledger entry is retired.

## 3. Creature walkability not unified with party F0706 — **LANDED (pass 39)**
- **Area:** `OWNERSHIP`
- **Status:** Resolved for square-element passability.  Creature and
  party walkability now share one source-faithful decoder via the new
  `F0707_MOVEMENT_IsSquarePassableForContext_Compat` owner; the pass-30
  stairs regression is prevented by an explicit creature-context
  branch.
- **Pass 39 (landed, 2026-04-24):**
  - `memory_movement_pc34_compat.h` / `.c` gained
    `F0707_MOVEMENT_IsSquarePassableForContext_Compat(dungeon,
    mapIndex, mapX, mapY, passContext)` with two contexts:
    `MOVEMENT_PASS_CTX_PARTY` (element/door decoding identical to
    F0706, stairs allowed as a party consequence square) and
    `MOVEMENT_PASS_CTX_CREATURE` (same decoding, stairs explicitly
    rejected per ReDMCSB GROUP.C / F0264_MOVE_IsSquareAccessibleForCreature
    — DM1 PC 3.4 creatures never step onto stairs).
  - `m11_game_view.c:m11_square_walkable_for_creature` is now a thin
    delegation to `F0707(..., MOVEMENT_PASS_CTX_CREATURE)`; the
    duplicated element/door switch has been removed.  Destroyed doors
    (state 5) are now walkable for creatures, matching the shared
    decoder (previously the custom M11 path rejected them, a
    divergence from F0706 — now removed).
  - `F0706_MOVEMENT_IsSquarePassable_Compat` itself is unchanged:
    party-side passability (including stairs) is preserved, so the
    pass-30 stairs behavior is intact.
  - Pass-39 probe
    `run_firestaff_m11_pass39_creature_walkability_probe.sh` drives
    F0707 directly across a 4x4 fixture covering every element type
    and door state 0..5; verifies 22/22 invariants including: party
    context equals F0706 for every square, creature context equals
    F0706 except stairs are blocked, creature context rejects both
    stairs-up and stairs-down, creature context accepts corridor /
    pit / teleporter / fakewall / open door / destroyed door,
    creature context rejects walls / closed door / animating door
    states 1..4, bounds and NULL safety match F0706, and F0706 itself
    still treats both stairs squares as passable for the party.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases,
    M11 verify end-to-end.  Previously-green pass-30 (15/15),
    pass-37 (13/13), and pass-38 (29/29) probes stay green.
- **What is NOT covered by pass 39:**
  - Thing-list occupancy checks for creature legality (another group
    on the square, party-square exceptions) remain in
    `m11_creature_try_move` — Pass 39 is strictly about square-element
    passability.  These AI-layer checks are not part of F0706/F0707
    scope.
  - Creature sensor enter/leave processing (Pass 32/37 are party-only,
    unchanged).
  - Creature interactions with stairs as a consequence square (none
    in DM1 PC 3.4 — creatures are blocked outright, which is the
    landed behavior).

## 4. Viewport region −28×−18 px vs DM1 original — **LOCKED (pass 40)**
- **Area:** `VISUAL`
- **Status:** Drift explicitly locked as a V1 `KNOWN_DIFF` with a
  source-anchored rationale and a quantified structural dependency on
  pass 42.  The DM1 viewport rectangle is now encoded as a named enum
  in `m11_game_view.c` (`M11_DM1_VIEWPORT_*`) so pass 42 can bind to
  the source anchor when it reroutes the invented chrome.
- **Pass 40 (landed, 2026-04-24):**
  - `m11_game_view.c` gained a new documentation enum
    `M11_DM1_VIEWPORT_X/Y/W/H = 0/33/224/136` alongside the unchanged
    runtime `M11_VIEWPORT_*`.  Source citation is in-line: DEFS.H
    lines 1997/2003 and COORD.C lines 81/82 from the local ReDMCSB
    dump at `redmcsb-output/I34E_I34M/`.
  - New diagnostic probe
    `firestaff_m11_pass40_viewport_lock_probe.c` +
    `run_firestaff_m11_pass40_viewport_lock_probe.sh` verify 22/22
    invariants including the DM1 anchor values, the measured
    Firestaff drift (−28×−18 px, −7 336 px²), the exact
    overlap rectangles between the DM1 viewport and every
    Firestaff invented chrome surface (total 2 914 px² across 7
    surfaces), and the correctness guard that the DM1 rectangle
    does not overlap the DM1 side-panel (x ≥ 224) or leave the
    320×200 framebuffer.
  - Evidence: `parity-evidence/pass40_viewport_lock.md` (source
    anchors, overlap table, dependency chain, reproducibility
    inputs, honesty trail).
  - Per-region overlay diffs refreshed under
    `parity-evidence/overlays/pass40/` for both the DM1 anchor
    rectangle and the current Firestaff rectangle against the
    pass-47 reference canvas.  Both remain high-delta because the
    reference canvas still carries no dungeon content (gated on
    blocker §11.1 DOSBox keystroke automation); pass 40 does not
    claim parity either way.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases, M11
    verify end-to-end.
- **What pass 40 did NOT change at the time:**
  - At pass 40, the runtime viewport rectangle was still
    `(12, 24, 196, 118)`. This was later superseded by the V1
    all-graphics work: normal V1 runtime now binds `M11_VIEWPORT_*`
    to the source DM1 viewport `(0, 33, 224, 136)`, and phase 77/79
    migrated/removed the old prototype probe geometry.
  - Binding the renderer to `M11_DM1_VIEWPORT_*` required the invented
    chrome reroute tracked as blocker §6 (pass 42); doing that at
    pass 40 would have destroyed 2 914 px² of clickable HUD.
  - No HUD layout or hit-testing changes.  No M10 touch.  No change
    to `PARITY_MATRIX_DM1_V1.md` status value for this row (it
    remains `KNOWN_DIFF`); only the rationale is updated to cite
    the new anchor + probe.
- **Follow-up:** retirement of this entry is blocked on pass 42
  (invented chrome reroute), which itself depends on pass 41
  (portrait stride drop from +8 to C69) and pass 47b (ZONES.H parse
  for source-faithful side-column placement).  Full dependency chain
  is in `parity-evidence/pass40_viewport_lock.md` §4.

## 5. Champion status-box stride +8 px per slot vs DEFS.H C69  — **LANDED (pass 41)**
- **Area:** `VISUAL`
- **Status:** Resolved for V1 original-faithful mode.  Stride is now
  bound to the DM1 DEFS.H anchor `C69_CHAMPION_STATUS_BOX_SPACING`
  (69 px) via a mode-aware helper; slot width is bound to the
  source graphic `C007_GRAPHIC_STATUS_BOX` (67 px).  V2 vertical-slice
  mode keeps its 77/71 geometry for binary compatibility with the
  pre-baked four-slot HUD sprite.
- **Pass 41 (landed, 2026-04-24):**
  - `m11_game_view.c` gained two new enum members,
    `M11_V1_PARTY_SLOT_STEP = 69` and `M11_V1_PARTY_SLOT_W = 67`,
    with source citations to DEFS.H:2157 and graphic C007.  The
    legacy `M11_PARTY_SLOT_STEP = 77` / `M11_PARTY_SLOT_W = 71`
    enum members are retained as the V2-mode fallback.
  - Two new static helpers, `m11_party_slot_step()` and
    `m11_party_slot_w()`, route the default (V1) path to the new
    values and V2 (opt-in via `FIRESTAFF_V2_VERTICAL_SLICE`) to the
    legacy values.
  - Both call sites (hit test in `M11_GameView_HandlePointer`;
    draw loop in `m11_draw_party_panel`) now go through the
    helpers.  The procedural fallback and the V1 double yellow
    active-champion highlight insets derive their widths from
    `slotW`, keeping the highlight strictly inside the 67-wide
    frame (outer 65, inner 63).
  - Pass-41 probe
    `run_firestaff_m11_pass41_status_box_stride_probe.sh` verifies
    24/24 invariants including the DEFS.H anchor (C69 == 69,
    C007 == 67x29), enum + helper wiring, non-overlapping V1 slot
    rectangles (2-px gaps), right edge fit in the 320x200
    framebuffer, and preservation of the pass-34 drift numbers
    (stride delta 8 px/slot, width delta 4 px/slot, 28 px total
    party-panel width saving).
  - Empirical screenshot diff in
    `parity-evidence/overlays/pass41/` shows the four V1 status
    boxes at x = 12, 81, 150, 219 (stride 69) instead of the
    previous x = 12, 89, 166, 243 (stride 77).
  - Evidence: `parity-evidence/pass41_status_box_stride.md`.
  - All baseline gates stay green: Phase A 18/18, M11 game-view
    361/361, launcher smoke PASS, M10 verify 20/20 phases, M11
    verify end-to-end.
- **What pass 41 does NOT change:**
  - Party-panel origin `(12, 160)` vs ZONES.H-anchored placement
    remains `BLOCKED_ON_REFERENCE` (blocker §11 / pass 47b).
  - No viewport, chrome, or bar-graph changes (blockers §4, §6, §7).
  - No V2 vertical-slice geometry change (pre-baked sprite stays
    aligned to 77 stride).

## 6. Firestaff-invented UI chrome (utility strip, control strip, prompt strip, status lozenge, inspect readout) — **LANDED (pass 42)**
- **Area:** `TEXT` (`VISUAL` adjacent)
- **Status:** Resolved for the V1 default path.  A V1 chrome-mode
  switch hides the last invented chrome surface still drawn in V1
  (the control strip at y=165) and reroutes player-facing status
  and inspect notifications into the source-faithful message-log
  surface at the bottom of the screen.
- **Pass 42 (landed, 2026-04-24):**
  - `m11_game_view.c` gained `m11_v1_chrome_mode_enabled()`,
    defaulting ON and opt-out via `FIRESTAFF_V1_CHROME=0`.  V2
    vertical-slice mode forces the switch OFF (V2 is not on the
    V1 parity path; the pre-baked HUD sprite relies on the
    legacy geometry).
  - The renderer now guards `m11_draw_control_strip(...)` behind
    the switch, suppressing the Firestaff-invented 88×14 px
    control row at (14, 165) when V1 chrome mode is on.  This
    retires the 352 px² control-strip overlap with the DM1
    viewport rectangle recorded in pass 40.
  - `m11_set_status` (82 sites) and `m11_set_inspect_readout`
    (68 + 57 direct snprintf sites) gained a pass-42 reroute
    path that pushes the player-facing payload into the rolling
    message log (`M11_MessageLog`, capacity 6) with forms
    `"ACTION - OUTCOME"` (yellow) and `"TITLE: DETAIL"`
    (light cyan).  The filter is the existing
    `m11_v1_message_is_player_facing` suppress list, so BOOT,
    PARTY MOVED, SPELL PANEL OPENED, RUNE *, IDLE TICK, etc. do
    not leak into the log.  A lookback against the last 3 log
    entries prevents double-logging when a companion
    `m11_log_event` call already surfaced the same key phrase
    (stair transitions, pit falls, spells).
  - The V1 bottom message surface is widened from 1 line at
    `y=149` to 3 lines at `y=149, 157, 165` (stride 8 px,
    matching DM1 TEXT.C).  The third line occupies the band
    vacated by the suppressed control strip.
  - Two new state fields `chromeRerouteLastStatus[96]` and
    `chromeRerouteLastInspect[128]` track per-surface last
    payload to suppress back-to-back duplicate pushes; they are
    zero-initialised by `memset` in `M11_GameView_Init` and not
    part of any save format.
  - Pass-42 probe
    `run_firestaff_m11_pass42_chrome_reduction_probe.sh` verifies
    32/32 invariants covering env policy, V2 override, payload
    form, key-phrase extraction, the player-facing suppress list
    (8 in / 1 out cases), the lookback dedup, pass-40 and pass-41
    enum preservation, the renderer call-site guard, the
    bottom-surface line-count toggle, and the presence of the
    pass-42 symbols in the source.
  - Evidence: `parity-evidence/pass42_chrome_reduction.md`.
  - All baseline gates stay green: Phase A 18/18, M11 game-view
    361/361, launcher smoke PASS, M10 verify 20/20 phases
    (M10 semantically untouched), M11 verify end-to-end.
  - `INV_GV_69` (stairs-up transition logs ASCENDED message) had
    its `logAfter > logBefore` guard relaxed to
    `logAfter >= logBefore` because the ring buffer (capacity 6)
    can saturate earlier in the stair sub-sequence after the
    reroute adds entries.  The substring-match assertion is
    unchanged; this is a probe-side correctness adjustment for
    the new logging-volume regime, not a weakening of the
    invariant.
- **What pass 42 does NOT change:**
  - The runtime viewport rectangle (still blocked on pass 47b
    ZONES.H parse even with the chrome reroute — the four
    pass-41 status-box rectangles still occupy the DM1
    rectangle’s y=160..168 band; see §4 above).
  - The numeric HP/stamina/mana readouts (§7, pass 43).
  - Any movement, combat, sensor, door, spell, or projectile
    behavior.  No M10 touch.
  - The 82 `m11_set_status` and 68 `m11_set_inspect_readout`
    call sites themselves — they still write the invented
    surfaces so `showDebugHUD` continues to render the status
    lozenge and inspect readout verbatim.
- **Follow-up:** champion bar-graph HUD (pass 43) and spell
  rune-label graphic (pass 44) are the next items on the V1
  parity path.

## 7. Champion HP/stamina/mana shown as numeric strings instead of bar graphs — **LANDED (pass 43)**
- **Area:** `VISUAL`
- **Status:** Resolved for the V1 default path.  Firestaff now renders
  source-faithful vertical champion bar graphs in the 67×29 status box
  instead of the invented horizontal strip, using the CHAMDRAW.C
  fill-from-bottom behavior and the recovered ZONES.H placement.
- **Pass 43 (landed, 2026-04-24):**
  - `m11_game_view.c` gained V1-only bar-graph geometry constants for
    the recovered status-box bar region (`x=43..66`, three 4×25
    containers at local x = 46/53/60, top y = 4 inside the 29-high
    frame) plus `m11_v1_bar_graphs_enabled()` to keep V2 vertical-slice
    mode on its legacy horizontal strip.
  - `m11_draw_party_panel` now renders HP/stamina/mana via three
    vertical 4×25 bars per champion, matching
    `CHAMDRAW.C:F0287_CHAMPION_DrawBarGraphs`: darkest-gray blank area,
    champion-color fill from the bottom, minimum 1-pixel fill when
    current > 0, and full blank for zero.
  - The champion-color mapping is now source-backed from local ReDMCSB
    `DATA.C:G0046_auc_Graphic562_ChampionColor = {7, 11, 8, 14}`
    (green, yellow, red, blue by slot) instead of the prior invented
    per-stat color scheme.
  - New bounded probe
    `run_firestaff_m11_pass43_bar_graph_probe.sh` verifies 10/10
    invariants including the exact bar origins (58/65/72 at y=164 for
    slot 0), the `{7,11,8,14}` source anchor, full-height / partial /
    1-pixel fill cases, all four champion colors, and the absence of
    the old left-anchored horizontal strip body.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases,
    M11 verify end-to-end.
- **What pass 43 does NOT change:**
  - Spell-panel rune labels remain text instead of the `C011` graphic
    blit (blocker §8 / pass 44).
  - Font-bank wiring, palette, viewport, and party-panel origin are
    unchanged (blockers §9, §10, §4, §11).
  - No M10 behavior or save-format touch.

## 8. Spell-panel rune labels still text, not `C011` graphic blit — **LANDED (pass 44)**
- **Area:** `VISUAL`
- **Status:** Resolved for the pass-44 visual target.  The spell-panel
  overlay now blits the native `C011_GRAPHIC_MENU_SPELL_AREA_LINES`
  14×13 cell slices for both the available-symbol row and the selected
  champion-symbol row instead of relying on the prior text-over-grid
  presentation alone.
- **Pass 44 (landed, 2026-04-24):**
  - `m11_game_view.c` gained pass-44 spell-label cell geometry
    (`14×13`, sourced from `C011`) plus a bounded
    `m11_blit_spell_label_cell(...)` helper that extracts line 2
    (`y=13`) for available symbols and line 3 (`y=26`) for champion
    symbols from graphic 11.
  - The spell-panel overlay now draws four selected-rune cells centered
    near the top of the panel and six available-rune cells centered in
    the active row using the native `C011` slices.  Existing text
    abbreviations remain overlaid as a temporary readability aid; the
    pass-44 visual gain is the recovered DM1 cell art, not font-bank
    parity.
  - New bounded probe
    `run_firestaff_m11_pass44_spell_label_probe.sh` verifies 8/8
    invariants including the `14×39` source asset size, the exact
    overlay placements (`selected x=83/y=43`, `available x=67/y=74`),
    and direct framebuffer matches against the top bands of the line-2
    and line-3 `C011` slices.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases,
    M11 verify end-to-end.
- **What pass 44 does NOT change:**
  - The spell panel still uses Firestaff's current text path for the
    overlaid rune abbreviations; pass 45 remains the font-bank pass.
  - No M10 behavior, save-format, palette, viewport, or party-panel
    geometry changes.

## 9. Custom 7-pixel font used in place of original GRAPHICS.DAT font atlas — **LANDED (pass 45)**
- **Area:** `TEXT`
- **Status:** Resolved for the V1 default path when original assets are
  present.  Firestaff now defaults `m11_draw_text(...)` to the original
  DM1 GRAPHICS.DAT font atlas through the existing `g_activeOriginalFont`
  delegation path, so generic panel text and the remaining spell-panel
  rune abbreviations no longer fall back to the custom 7-pixel glyph bank
  in normal V1 play.
- **Pass 45 (landed, 2026-04-24):**
  - Added bounded verification probe
    `run_firestaff_m11_pass45_font_bank_probe.sh` +
    `firestaff_m11_pass45_font_bank_probe.c`.
  - The probe verifies 8/8 invariants covering: resolved interface-font
    load success, `originalFontAvailable` activation, 6-pixel DM1 font
    metrics, title-strip glyphs matching a direct render from the loaded
    font atlas, and selected/available rune abbreviation text rendered
    directly from that atlas at the pass-44 placements.  The probe is
    intentionally font-focused and does not call `M11_GameView_Draw` with
    an uninitialized world.
  - Evidence: `parity-evidence/pass45_font_bank_wiring.md`.
  - All baseline gates stay green: Phase A 18/18, M11 game view
    361/361, M11 launcher smoke PASS, M10 verify 20/20 phases,
    M11 verify end-to-end.
- **What pass 45 does NOT change:**
  - Text content parity / over-labeling questions are not retired by
    this pass; only the font-bank rendering path is.
  - No palette work (blocker §10 / pass 46), viewport change, or
    M10 semantic change.

## 10. VGA palette compat layer still uses EGA-style colors — **LANDED (pass 46)**
- **Area:** `VISUAL` (`BEHAVIOR` adjacent for brightness)
- **Status:** Resolved for the base 16-color palette and the six
  brightness lookup tables exposed by `vga_palette_pc34_compat.c`.
  The compat seam now uses the source-backed DM1 PC 3.4 VGA DAC values,
  not the old EGA-style colors or a linear attenuation model.
- **Pass 46 (landed, 2026-04-24):**
  - Added bounded verification probe
    `run_firestaff_m11_pass46_vga_palette_probe.sh` +
    `firestaff_m11_pass46_vga_palette_probe.c`.
  - The probe verifies 7/7 invariants: 16 colors × 6 brightness levels,
    index 4 cyan `(0,219,219)` instead of EGA dark red, source-backed
    brown/tan/blue base colors, cyan invariant across all brightness
    levels, LIGHT5 retaining 8 residual non-black colors, sampled
    LIGHT1–LIGHT5 values matching recovered VIDEODRV.C tables, and
    rejection of out-of-range lookups.
  - Evidence: `parity-evidence/pass46_vga_palette_lookup.md`.
  - M10 VGA palette export probe remains green.
- **Pass 68 (landed, 2026-04-25):**
  - Added source-backed special palette arrays from local ReDMCSB
    `DRAWVIEW.C` (`G8147_CREDITS`, `G8148_ENTRANCE`) to
    `vga_palette_pc34_compat.c/h`.
  - Added indexed special-palette lookup seam
    `F9011_VGA_GetSpecialColorRgb_Compat(...)` /
    `G9013_auc_VgaPaletteSpecial_Compat` so callers can select
    credits vs entrance without falling back to the base palette.
  - Added bounded verification probe
    `run_firestaff_m11_pass68_special_palette_probe.sh` +
    `firestaff_m11_pass68_special_palette_probe.c` (6/6 invariants).
  - Evidence: `parity-evidence/pass68_special_palettes.md`.
- **What pass 46/pass 68 do NOT change:**
  - Creature palette replacement/rendering integration remains open.
  - Exact entrance/credits frontend palette-selection timing and pixel
    overlays remain open; pass 68 lands the source data + lookup seam
    only, not a full rendered-screen parity claim.
  - Swoosh palette behavior remains open if later evidence proves it is
    a distinct PC/VGA palette path.
  - No viewport/layout/pixel-overlay parity claim beyond the palette
    lookup seams.

## 11. ReDMCSB pixel overlays missing for viewport and side panel
- **Area:** `VISUAL`
- **Evidence:**
  - Pass 33 §4 and Pass 34 §3 both have `BLOCKED_ON_REFERENCE` rows
    for pixel-level placements because no ReDMCSB headless rasteriser
    is in-tree and no reference emulator capture has been produced.
  - Pass 47 (`parity-evidence/pass47_reference_capture_tooling.md`)
    landed a bounded tooling / reference path:
    - in-tree reference compositor
      `tools/redmcsb_reference_compose.py` + per-graphic palettised
      anchor PNGs in `reference-artifacts/anchors/` (10/10 anchors
      match DEFS.H pixel sizes);
    - overlay-diff helper `tools/redmcsb_overlay_diff.py` with
      DEFS.H-named regions;
    - deterministic DOSBox capture harness
      `scripts/dosbox_dm1_capture.sh` (config written, game staged;
     launch blocked only on user installing DOSBox-Staging / DOSBox-X).
  - Pass 47 does NOT unblock: (a) ZONES.H-driven side-column placement
    (ZONES.H is not in the local ReDMCSB dump), (b) viewport dungeon
    content without an emulator capture, (c) TEXT.C runtime strings.
- **Status:** partially unblocked.  Panel-chrome pixel overlays and
  the DEFS.H-anchored viewport bounding rect can now be diffed
  honestly.  Phase 70 also locks the source viewport base graphics
  (C079 ceiling `224x39`, C078 floor `224x97`) and current renderer
  draw-order inputs (`INV_GV_414/415`) so the next visual patch can
  change placement/z-order intentionally instead of rediscovering the
  asset seam.  Pass 70 follow-up captured six original DOS viewport
  crops and produced first Firestaff-vs-original diff stats; original
  media is no longer the blocker.  The remaining blocker is exact
  semantic input/state parity: live DOSBox key routing proved too
  fragile for authoritative state matching.  Pass 71 adds a
  Greatstone/SCK source-lock check for 22 critical `GRAPHICS.DAT`
  references plus the original `GRAPHICS.DAT`/`DUNGEON.DAT`/`SONG.DAT`
  hashes, so the next visual pass should build deterministic source
  states from locked original data instead of treating keyboard route
  guesses as truth.  Pass 72 adds exactly that first deterministic
  source-state anchor via `firestaff_m11_viewport_state_probe`: source
  party state `(map=0,x=1,y=3,dir=SOUTH)`, a 3x3 relative viewport
  neighborhood with thing-chain counts, and critical viewport asset
  dimensions from `GRAPHICS.DAT`. Pass 73 commits the reproducible
  layout-696/ZONES reconstruction (`zones_h_reconstruction.json`),
  replacing the old vague ZONES.H-missing blocker with a concrete source
  layout table. Pass 74 adds full-frame original-vs-Firestaff region
  measurements for viewport/action/spell/inventory surfaces; high deltas
  confirm semantic route mismatch remains. Pass 75 sheets the startup-menu
  candidate asset windows so menu overlay work has concrete source assets.
  Pass 76 locks the Firestaff six-capture fixture as state. Pass 77 fixes
  the earlier one-rune `after_cast` bug by entering low-power Ful Ir before
  `SPELL_CAST`; original route work can now target exact Firestaff states
  instead of misleading capture filenames.
- **Suggested pass:** pass-47b/pass-72 follow-up — ZONES.H parse (from
  `PANEL.C` + `COORD.C` layout-record init) and render from the locked
  DUNGEON.DAT/GRAPHICS.DAT viewport state plus `zones_h_reconstruction.json`;
  use DOSBox captures as comparison evidence, not as the source of route truth.

## 12. Tick-prefix (`Tn:`) on message log lines — **CANDIDATE HUD PASS**
- **Area:** `TEXT`
- **Status:** Candidate fix on `parallel/hud-pass-20260426090938`; not
  integrated to main yet.
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.3 — the tick
    prefix is a Firestaff convention, not source-faithful.
  - `parity-evidence/candidate_hud_tick_prefix_message_log.md` records
    the bounded HUD/UI fix.
  - `INV_GV_302A` verifies the boot message-log entry is stored without
    the old `T0: ` prefix.
- **Candidate pass:** strip leading `T<digits>: ` at the `m11_log_event`
  storage seam, preserving existing call sites while keeping synthetic
  tick chrome out of message-log text.

## 13. Animating door intermediate states handled upstream of F0715
- **Area:** `OWNERSHIP` (already noted in blocker #2 but tracked
  separately for timeline ownership)
- **Evidence:** same as #2.
- **Suggested pass:** pass-38 (same as #2).

## 14. M9/M10 behavioral probes verify internal consistency only, not original-match
- **Area:** `BEHAVIOR`
- **Evidence:** `PARITY_MATRIX_DM1_V1.md` §5 rows all `UNPROVEN`.
- **Suggested pass:** pass-49 — bind at least one high-value M10 probe
  (movement-champions or dungeon-doors-sensors) to a ReDMCSB-produced
  expected log.  Pass 47 landed the panel-chrome pixel-overlay path
  (`tools/redmcsb_overlay_diff.py` + `reference-artifacts/anchors/`),
  so the chrome side is unblocked; the behavioral-log side still
  needs either a DOSBox capture path or a state-dump walk of the
  probed path in the ReDMCSB source.

## 15. Audio samples are procedural placeholders
- **Area:** `ASSET`
- **Evidence:** `PARITY_MATRIX_DM1_V1.md` §7 rows "Sound samples
  (content) — SONG.DAT" and "… GRAPHICS.DAT SND3".
- **Pass 50 (landed, 2026-04-24):**
  - SONG.DAT DM PC v3.4 format fully documented from
    source (dmweb.free.fr) and empirically verified against the
    real file: `DM1_SONG_DAT_FORMAT.md`.
  - Standalone loader/decoder landed: `song_dat_loader_v1.[ch]`
    (DMCSB2 header, SEQ2 sequence, SND8 DPCM at 11025 Hz).
  - V1 probe landed: `probes/v1/firestaff_v1_song_dat_probe.c` +
    `run_firestaff_v1_song_dat_probe.sh`.  Against the real
    SONG.DAT the probe passes 6/6 invariants (header layout,
    file size, 10-item table, SEQ2 20-word sequence with end
    marker, all 9 SND8 items decode to exactly declared sample
    count).
  - `PARITY_MATRIX_DM1_V1.md` §7 narrowed: "Sound samples —
    SONG.DAT" status annotated with landed progress; a new
    explicit row tracks the GRAPHICS.DAT SND3 SFX bank gap.
  - Full landing log and evidence pointers in
    `PASS50_AUDIO_FINDINGS.md`.
- **Pass 51 (landed, 2026-04-24):**
  - GRAPHICS.DAT DM PC v3.4 English SND3 in-game SFX bank decoded in
    a standalone loader: `graphics_dat_snd3_loader_v1.[ch]`.
  - V1 probe landed: `probes/v1/firestaff_v1_graphics_dat_snd3_probe.c`
    + `run_firestaff_v1_snd3_probe.sh`.  Against the real
    `~/.firestaff/data/GRAPHICS.DAT` the probe passes 6/6 invariants:
    DMCSB2 signature/count/header, dmweb 33-item SND3 index set,
    all 33 verified headers/attrs/sample counts, all 33 unsigned PCM
    decodes at 6000 Hz (114157 total samples), and Greatstone
    Sound 00..32 labels.
  - Full landing log and evidence pointers in
    `PASS51_AUDIO_FINDINGS.md`; PASS log in
    `parity-evidence/pass51_v1_graphics_dat_snd3_probe.txt`.
- **Pass 52 (landed, 2026-04-24):**
  - Source-backed DM PC v3.4 sound-event → GRAPHICS.DAT SND3 mapping
    landed in `sound_event_snd3_map_v1.[ch]`, anchored to ReDMCSB
    `DEFS.H` (`M513_SOUND_COUNT = 35`, I34E sound macros) and
    `DATA.C` (`MEDIA719_I34E_I34M G0060_as_Graphic562_Sounds`).
  - V1 probe landed: `probes/v1/firestaff_v1_snd3_event_map_probe.c`
    + `run_firestaff_v1_snd3_event_map_probe.sh`.  Against the real
    `~/.firestaff/data/GRAPHICS.DAT` the probe passes 5/5 invariants:
    35 sound-event entries, expected source order, lookup bounds,
    coverage of all 33 SND3 items with only the source-backed door /
    explosion aliases, and populated SND3 manifest metadata.
  - Full landing log and evidence pointers in `PASS52_AUDIO_FINDINGS.md`;
    PASS log in `parity-evidence/pass52_v1_snd3_event_map_probe.txt`.
- **Pass 53 (landed, 2026-04-24):**
  - `audio_sdl_m11.[ch]` now opportunistically loads the decoded/mapped
    GRAPHICS.DAT SND3 bank when original assets are present, exposing 35
    event-index buffers via the pass-52 sound namespace while preserving
    procedural marker fallback when the asset is unavailable or disabled.
  - `m11_game_view.c` routes `EMIT_SOUND_REQUEST` payloads through
    `M11_Audio_EmitSoundIndex(...)`; non-`EMIT_SOUND_REQUEST` direct marker
    calls remain procedural fallback unless later passes map them explicitly.
  - Sample-rate policy is explicit: SND3 source PCM is 6000 Hz unsigned mono;
    M11 SDL remains 22050 Hz float mono; pass 53 linearly resamples each SND3
    buffer once at init and queues the resampled buffer at runtime.
  - V1 probe landed: `probes/m11/firestaff_m11_pass53_snd3_runtime_probe.c`
    + `run_firestaff_m11_pass53_snd3_runtime_probe.sh`.  Against the real
    `~/.firestaff/data/GRAPHICS.DAT` it passes 5/5 invariants covering the
    no-asset fallback, all-35 event-index load, resample count policy, and SDL
    queueing of a mapped decoded SND3 buffer.  Evidence in
    `PASS53_AUDIO_FINDINGS.md` and
    `parity-evidence/pass53_v1_snd3_runtime_probe.txt`.
- **Pass 54 (landed, 2026-04-24):**
  - `audio_sdl_m11.[ch]` now opportunistically loads original `SONG.DAT` when
    present, decodes all 9 SND8 music parts through `song_dat_loader_v1`,
    linearly resamples signed 11025 Hz mono PCM to the fixed 22050 Hz SDL float
    stream, and concatenates one pre-loop title-music phrase by walking SEQ2
    words.
  - The documented/probed bit-15 SEQ2 marker is handled narrowly: normal words
    append parts, the first bit-15 word stops the one-cycle build, and the low
    15 bits are recorded as the loop target.  For DM PC v3.4 EN that marker is
    `0x8001`, so the runtime records loop target part 1.  No continuous-loop
    cadence claim is made without an original capture.
  - New API: `M11_Audio_PlayTitleMusic(...)` queues the decoded/resampled
    one-cycle phrase when SDL audio is available; absent/malformed/disabled
    SONG.DAT remains a safe no-op (`FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG`).
  - V1 probe landed: `probes/m11/firestaff_m11_pass54_song_runtime_probe.c` +
    `run_firestaff_m11_pass54_song_runtime_probe.sh`.  Against the real
    `SONG.DAT` it passes 6/6 invariants covering no-asset no-op fallback,
    all-9-part load, SEQ2 marker handling, 11025→22050 resample/concatenate,
    and SDL queueing.  Evidence in `PASS54_AUDIO_FINDINGS.md` and
    `parity-evidence/pass54_v1_song_runtime_probe.txt`.
- **Pass 55 (landed, 2026-04-24):**
  - Source-backed action cues in `m11_game_view.c` now use mapped sound-event
    emissions instead of direct procedural markers where Pass 52 evidence is
    strong: `WAR CRY` → sound event 17 (`M619_SOUND_WAR_CRY`), `BLOW HORN` →
    sound event 18 (`M620_SOUND_BLOW_HORN`), and `SHOOT` / `THROW` → sound
    event 13 (`M563_SOUND_COMBAT_ATTACK...`).
  - New bounded audit script:
    `scripts/audit_m11_direct_audio_markers_pass55.py`, with PASS log in
    `parity-evidence/pass55_m11_direct_audio_marker_audit.txt`, verifies the
    converted event-index paths and locks the remaining direct-marker TODO
    buckets.
  - Remaining direct marker buckets are explicit, not claimed faithful:
    generic non-`EMIT_SOUND_REQUEST` tick emissions; `CALM` / `BRANDISH` /
    `CONFUSE`; `FIREBALL` / `DISPELL` / `LIGHTNING` cast-action cue; and
    `INVOKE` action cue.
  - Full landing log and evidence pointers in `PASS55_AUDIO_FINDINGS.md`.
- **Pass 125 (candidate, 2026-04-26):**
  - Tightened the fallback/no-audio runtime seam for future queue/order probes:
    valid `M11_Audio_EmitSoundIndex(...)` calls now preserve `lastSoundIndex`
    even when decoded SND3 cannot be queued and marker/no-audio fallback is
    used.  Direct `M11_Audio_EmitMarker(...)` calls still clear
    `lastSoundIndex`, so headless evidence can distinguish source-backed sound
    events from direct procedural markers.
  - This is metadata/ordering evidence only.  It does not claim waveform,
    overlap, continuous-loop, or wall-clock cadence parity.
  - New probe/evidence: `probes/m11/firestaff_m11_pass125_audio_event_order_probe.c`,
    `run_firestaff_m11_pass125_audio_event_order_probe.sh`, and
    `parity-evidence/pass125_audio_event_order_probe.txt` (4/4 PASS).
- **Pass 56 (landed, 2026-04-24):**
  - Added bounded DM PC 3.4 `TITLE` mapfile loader/player support in
    `title_dat_loader_v1.[ch]`, grounded in Greatstone's 59-item TITLE bank and
    the local original 12002-byte `TITLE` file.
  - The pass verifies 1:1 title-animation progression semantics only: item
    order (`AN`, `BR`, `P8`, `PL`, `EN`, `DL`..., second `PL`/`EN` segment,
    `DO`), 2 encoded images, 51 delta layers, 53 original 320×200 frame-bearing
    records, palette break segmentation of 37 + 16 frames, and `DO` stop
    handling.  No cadence/timing claim is made.
  - New probes/evidence:
    `run_firestaff_v1_title_dat_probe.sh` (8/8 invariants, real `TITLE`) and
    optional `probes/v1/firestaff_v1_pass56_greatstone_png_probe.py` (3/3
    invariants against local Greatstone source PNG reference when present), with
    logs in `parity-evidence/pass56_v1_title_dat_probe.txt` and
    `parity-evidence/pass56_v1_title_greatstone_png_probe.txt`.
  - Runtime/frontend drawing is intentionally not wired in this pass: EN/DL
    pixel payload decode/compositing, exact blit timing, and title-menu handoff
    remain explicit next gaps.
- **Pass 57 (landed, 2026-04-24):**
  - `title_dat_loader_v1.[ch]` now includes a bounded IMG1 renderer for the
    original DM PC 3.4 `TITLE` EN/DL payloads.  It decodes EN base images,
    applies DL transparent-skip delta compositing on one 320×200 V1 canvas,
    applies the source PL palettes, and emits 53 rendered frames in source item
    order.  It uses only the original local `TITLE` binary for V1 rendering.
  - New probes/evidence:
    `run_firestaff_v1_title_render_probe.sh` (6/6 invariants against the real
    `TITLE`) and `run_firestaff_v1_title_render_png_compare_probe.sh` (6/6 C
    renderer invariants plus 4/4 Greatstone-source PNG comparison invariants),
    with logs in `parity-evidence/pass57_v1_title_render_probe.txt` and
    `parity-evidence/pass57_v1_title_render_png_compare_probe.txt`.
  - Pixel/compositing behavior is now source-proved for all 53 frames: rendered
    RGB output matches the local Greatstone 320×200 EN/DL source PNG references
    byte-for-byte, and pass-57 fingerprints lock both palette-index composites
    (`0xb4e5d330`) and PL-applied RGB composites (`0x143fa969`).
  - No title frontend timing/cadence, wall-clock frame rate, palette-display
    timing, or title-menu handoff claim is made by pass 57.
- **Pass 58 (landed, 2026-04-24):**
  - Added `title_frontend_v1.[ch]`, a narrow adapter from the pass-57 original
    `TITLE` renderer into the V1/M9 frontend's 320×200 packed-4bpp screen
    bitmap format.  It uses the original-resolution local `TITLE` data only;
    no V2/upscaled title assets are part of this path.
  - `firestaff_m9_beta_harness.c --title-hold N` now attempts to publish the
    original `TITLE` frames through that frontend adapter, using deterministic
    implementation advance `((N-1) % 53) + 1`.  If no valid `TITLE` file is
    found or rendering fails, it preserves the pre-existing GRAPHICS.DAT graphic
    313 fallback path.
  - New probe/evidence: `run_firestaff_v1_title_frontend_probe.sh` verifies
    5/5 invariants: frontend use of original `TITLE` data, first/boundary/last
    frame reachability, 37+16 palette segmentation, deterministic wrap behavior
    labelled as implementation cadence, and packed-frame equality with sampled
    pass-57 renderer output.  M9 title-hold evidence in
    `parity-evidence/pass58_v1_title_frontend_m9_harness.txt` shows 54/54
    frontend frames published and 0 fallback frames with the local original
    `TITLE` present; `parity-evidence/pass58_v1_title_frontend_fallback_probe.txt`
    forces an invalid TITLE path and verifies the existing graphic-313 fallback
    remains reachable (0 original / 1 fallback frame).
  - Pass 58 still makes no original wall-clock timing, palette-display timing,
    or title-menu handoff parity claim.
- **Pass 59 (landed, 2026-04-24):**
  - Added a finite TITLE presentation decision helper in `title_frontend_v1.[ch]`.
    Steps 1..53 render the original source-backed TITLE frames once, step 53 is
    marked handoff-ready at the source `DO` boundary, and later implementation
    overrun steps hold frame 53 instead of wrapping back to frame 1.
  - Updated M9 `--title-hold N` to use that finite sequence policy before
    rendering, preserving original TITLE frame publication and the existing
    GRAPHICS.DAT graphic-313 fallback path when TITLE is unavailable.
  - New probe/evidence: `run_firestaff_v1_title_handoff_probe.sh` verifies 5/5
    deterministic handoff invariants without claiming original wall-clock
    cadence.  M9 evidence in
    `parity-evidence/pass59_v1_title_handoff_m9_harness.txt` shows 54/54
    frontend frames, 0 fallback frames, 2 handoff-ready frames, and 1 held-last
    frame for a 54-step title hold.
  - Pass 59 still makes no original wall-clock timing, palette-display timing,
    or emulator title-menu handoff parity claim.
- **Pass 61 (landed, 2026-04-24):**
  - Added an opt-in deterministic TITLE→menu handoff decision helper in
    `title_frontend_v1.[ch]`.  Pass-59 hold-last-frame behavior remains
    available when callers do not request menu entry; callers that do request
    menu entry switch to the menu surface on the first post-`DO` step while
    retaining frame 53 as the final TITLE evidence frame.
  - Added M9 `--title-menu-handoff N`, which publishes source-backed TITLE
    frames through step 53 and then publishes the caller-owned menu surface
    frame on step 54.  This is implementation policy only, not an emulator
    timing/cadence claim.
  - New probe/evidence: `run_firestaff_v1_title_menu_handoff_probe.sh` verifies
    5/5 deterministic handoff invariants, including the opt-out hold path.
    M9 evidence in
    `parity-evidence/pass61_v1_title_menu_handoff_m9_harness.txt` shows 53/53
    original TITLE frames, 0 fallback frames, 1 menu frame, 2 handoff-ready
    steps, and 1 entered-menu step for a 54-step publication.
  - Pass 61 still makes no original wall-clock timing, palette-display timing,
    or emulator title-menu handoff parity claim.
- **Pass 62 (landed, 2026-04-24):**
  - Added a targeted DOSBox Staging evidence harness for the original TITLE
    cadence/handoff investigation:
    `scripts/dosbox_dm1_title_cadence_pass62.sh`.
  - Gathered bounded raw DOSBox evidence under
    `verification-screens/pass62-dosbox-title-cadence/` showing that current
    automation reaches the DM PC 3.4 text selector, but does not cleanly reach
    the graphical TITLE runtime without an interactive selector key.  `DM VGA`
    still waits at the selector; simple input redirection either aborts on EOF
    or loops in selector prompts; direct VGA overlay execution is not a valid
    bypass.
  - Evidence note:
    `parity-evidence/pass62_v1_title_dosbox_capture_blocker.md` documents the
    commands, artifact paths/sizes, and blocker.  No V1 runtime code changed,
    and no original timing/cadence claim was added.
- **Pass 63 (landed, 2026-04-24):**
  - Added `scripts/dosbox_dm1_title_input_pass63.sh`, a bounded DOSBox
    Staging + `cliclick` wrapper that types the original setup selector
    choices (`VGA Graphics`, `No Sound`, `Mouse`) as host keystrokes.
  - Verified `--dry-run` and `--run`; the actual run reaches the graphical
    original `Dungeon Master` TITLE screen and writes local evidence under
    `verification-screens/pass63-dosbox-title-automation/`.
  - AppleScript/System Events keystroke injection was tested and rejected as
    not permitted on this Mac (`-1743`).  DOS redirection remains rejected per
    pass 62.  The stable local selector-handoff path is `cliclick` host input.
  - Evidence note:
    `parity-evidence/pass63_v1_title_dosbox_input_automation.md` documents the
    commands, failed methods, successful selector sequence, local artifact
    paths, and remaining clean-capture/cadence gaps.  No V1 runtime code
    changed, and no original timing/cadence claim was added.
- **Pass 64 (landed, 2026-04-24):**
  - Added `scripts/dosbox_dm1_title_clean_capture_pass64.sh`, a targeted
    DOSBox Staging capture wrapper for original TITLE evidence.
  - The wrapper keeps the pass-63 selector choices but posts the `1`/Return
    sequence directly to the DOSBox process and captures the DOSBox window by
    CGWindowID.  This avoids the detached-run foreground problem and unrelated
    host desktop overlays without keychain edits or destructive cleanup.
  - Verified `--dry-run` and `--run`; the run produced two clean, byte-identical
    original TITLE window captures under
    `verification-screens/pass64-clean-title-capture/`, each 137K, with SHA-256
    `8cfa3f074c78e2f694be33d6e5e4fcab503f27241cb30cdc3bd1032f8cbfb56a`.
  - Evidence note:
    `parity-evidence/pass64_v1_title_clean_window_capture.md` documents the
    method, commands, artifact paths/sizes, mode-13h log evidence, and remaining
    raw/cropped framebuffer timing gap.  No V1 runtime code changed, and no
    original timing/cadence claim was added.
- **Pass 65 (landed, 2026-04-25):**
  - Added `scripts/dosbox_dm1_title_crop_compare_pass65.py`, a deterministic
    bridge from pass-64 clean DOSBox window captures to 320x200 original TITLE
    evidence.
  - The script reads the graphics-mode DOSBox log line (`320x200 mode 13h`,
    scaled to `1067x667`), crops the centered game surface from the clean
    window capture, scans legal vertical offsets for the lowest-MAE alignment
    against the existing 53-frame source-backed Firestaff/Greatstone TITLE
    render dump, rescales with nearest-neighbour sampling, and normalizes only
    near-black DOSBox background pixels before metrics.
  - Verified against the pass-64 captures: both clean windows crop to
    `34,120,1067,667`, best-match `verification/pass58-title-render-dump/frame_0037.ppm`,
    MAE `1.693047`, max delta `12`, and identical cropped-frame SHA-256
    `965301f0786d829116ec53c81ad53efc8aa05cc9ec260113fd435b2692c63f6e`.
  - Evidence note:
    `parity-evidence/pass65_v1_title_crop_compare.md` documents the script,
    commands, artifact paths/sizes, comparison metrics, and why cadence remains
    unclaimed.  No V1 runtime code changed, and no original timing/cadence claim
    was added.
- **Pass 66 (landed, 2026-04-25):**
  - Added `scripts/dosbox_dm1_title_sequence_capture_pass66.sh` for timestamped
    targeted-window PNG sequences and `scripts/dosbox_dm1_title_sequence_match_pass66.py`
    to crop/downscale each captured still with the pass-65 bridge and match it
    against the 53 source-backed Firestaff/Greatstone TITLE frames.
  - Verified a 16-frame local sequence.  The manifest records before/after UTC
    timestamps, epoch-ns timing, SHA-256, and byte size for each targeted
    DOSBox-window capture.  The match report verifies the input SHA-256 values
    before writing cropped 320x200 evidence.
  - The run produced 16 captures, 6 unique window SHA-256 values, and 6 unique
    cropped normalized frames.  Seven rows matched the TITLE dump within the
    pass-65 tolerance envelope (`MAE <= 2.0`, max delta `12`), with best
    references `frame_0039.ppm`, `frame_0046.ppm`, and `frame_0037.ppm`.
  - Rows after that have high residuals (`MAE` about 16-53, max delta `255`) and
    are recorded as transition/non-TITLE rows, not source-backed TITLE cadence
    samples.
  - Evidence note:
    `parity-evidence/pass66_v1_title_timestamp_sequence.md` documents the new
    scripts, command results, artifact paths/sizes, timestamped match table, and
    why cadence remains unclaimed.  No V1 runtime code changed, and no original
    timing/cadence claim was added.
- **Pass 67 (landed, 2026-04-25):**
  - Added `scripts/dosbox_dm1_native_title_capture_pass67.sh`, which drives
    DOSBox Staging's native macOS screenshot mapper (`Cmd+F5`) with
    `default_image_capture_formats=raw`, and
    `scripts/dosbox_dm1_native_title_match_pass67.py`, which validates the raw
    PNG manifest and compares each 320x200 framebuffer image directly against
    the 53 source-backed TITLE frames.
  - Verified that native raw screenshots work: the retained run produced 18/18
    320x200 raw framebuffer PNGs.  Seven screenshots matched the TITLE dump
    within the pass-65 tolerance envelope, all best-matching `frame_0012.ppm`
    with `MAE 1.355042` and max delta `12`.
  - A tuning run at a later selector offset hit `frame_0030.ppm` repeatedly,
    proving that different source-backed TITLE stills can be captured natively;
    however, each single run remains too sparse/blocking to produce a monotonic
    source-frame timing trace.
  - Evidence note:
    `parity-evidence/pass67_v1_native_raw_title_capture.md` documents the native
    screenshot mapper behavior, command results, artifact paths/sizes, match
    metrics, and the precise remaining blocker.  No V1 runtime code changed,
    and no original timing/cadence claim was added.
- **Pass 68 candidate (parallel title/menu, 2026-04-26):**
  - Added a bounded non-runtime probe for the TITLE cadence/menu-layout gap:
    `probes/v1/firestaff_v1_title_menu_cadence_layout_probe.c` and
    `run_firestaff_v1_title_menu_cadence_layout_probe.sh`.
  - The probe re-verifies the original local `TITLE` manifest boundary
    (`12002` bytes, `59` records, `53` frames, `2 EN + 51 DL`, palette split
    `37 + 16`) and the deterministic Firestaff seam: frame 53 remains TITLE and
    handoff-ready, while step 54 switches to MENU only in opt-in handoff mode.
  - It also records GRAPHICS.DAT startup-menu candidate asset metrics for the
    existing boot-script windows: A `304..319`, B-left `360..367`, B-core
    `368..383`, B-right `384..391`, and B-wide `360..391`.  This is layout
    groundwork only; it identifies source-backed menu graphic windows and
    dimensions, not final 320x200 placement parity.
  - Evidence:
    `parity-evidence/pass68_v1_title_menu_cadence_layout.md` and
    `parity-evidence/pass68_v1_title_menu_cadence_layout_probe.txt`.  No V1
    runtime code changed, and no original timing/cadence claim was added.
- **Remaining gaps before V1 audio can be called
  original-faithful** (see `PASS50_AUDIO_FINDINGS.md` §5,
  `PASS51_AUDIO_FINDINGS.md` §5, `PASS52_AUDIO_FINDINGS.md` §5,
  `PASS53_AUDIO_FINDINGS.md` §4, `PASS54_AUDIO_FINDINGS.md` §4, and
  `PASS55_AUDIO_FINDINGS.md` §4):
  1. Original capture/proof of SFX/title-music cadence, prioritization,
     continuous-loop timing, and overlap.
  2. Source-backed title frontend start/stop timing for the new queue API.
  3. Resolve the four documented remaining direct-marker TODO buckets, either
     by source-backed event-index conversion or original runtime capture.
  4. Bug-faithful playback quirks/cataloging when relevant.
- **Remaining gaps before V1 TITLE animation can be called original-faithful:**
  1. Capture or source-prove original frame timing/cadence, palette application
     timing, and title-menu handoff.  Passes 59 and 61 define deterministic
     implementation cadence/handoff policy only.
  2. Pixel-compare the wired frontend output against a timestamped original
     emulator capture sequence; pass 66 proves the targeted-window bridge can
     capture timestamped changes, and pass 67 proves native raw 320x200 stills
     can hit source-backed TITLE frames, but neither route has produced a robust
     monotonic timing trace.
  3. Bypass the blocking DOSBox Staging screenshot mapper with a per-presented-
     frame raw framebuffer dump, or source-prove PC TITLE timing/handoff from
     the original/ReDMCSB PC-conditional path before making frontend timing
     claims.
- **Suggested follow-up pass:** instrument DOSBox Staging/SDL or use an emulator
  with frame-dump/debugger support (for example DOSBox-X or a custom Staging
  build) to dump the raw 320x200 framebuffer once per presented VGA frame.  In
  parallel, isolate the PC-target timing conditionals in ReDMCSB `TITLE.C` and
  the PC video wait path so native stills can be paired with source timing proof
  instead of sparse hotkey screenshots.

---

## All-graphics addendum: object icons, inventory fixture, and C011 capture gates — **LANDED (passes 84–99)**
- **Area:** `VISUAL` / `CAPTURE_GATES`
- **Status:** Resolved for the current action-hand, inventory-slot, and spell
  label-cell surfaces.  Full original screenshot overlays are still open; this
  addendum records the narrowed, source-backed parts so they do not regress into
  vague `UNPROVEN` status.
- **Passes 84–91 (landed, 2026-04-25):**
  - Action-hand empty cells now draw source empty-hand icon
    `C201_ICON_ACTION_ICON_EMPTY_HAND` from graphics `42..48`.
  - Action-hand items with `ActionSetIndex > 0` now draw source object icons
    through the `F0033_OBJECT_GetIconIndex` resolver instead of scaled viewport
    sprites; `ActionSetIndex == 0` items intentionally remain plain cyan.
  - The resolver covers source dynamic variants for lit torches (`G0029` charge
    bucket), charged weapons (`+1`), closed scrolls, compass direction, and
    charged water/Jewel Symal/Illumulet.
  - Inventory slot boxes now use `F0038_OBJECT_DrawIconInSlotBox` semantics:
    16×16 source object icons, direct blit, no action-area palette rewrite.
  - Action-area object icons apply `G0498` (`12 -> C04 cyan`); inventory icons
    preserve source colour 12.  This split is probe-backed.
- **Passes 92–99 (landed, 2026-04-25):**
  - `PARITY_MATRIX_DM1_V1.md` now records equipment/item icons as `MATCHED` for
    icon selection / atlas extraction / palette distinction in the current
    action + inventory slot surfaces.
  - `capture_firestaff_ingame_series.c` has a deterministic capture champion
    (`HALK`) with a dagger in the action/right hand, making both
    `02_ingame_turn_right_latest` and `06_ingame_inventory_panel_latest`
    reproducible object-icon fixtures.
  - `run_firestaff_m11_ingame_capture_smoke.sh` now checks the action screenshot
    for G0498 cyan dagger coverage and the inventory screenshot for preserved
    source dark gray dagger coverage.
  - The same capture smoke now checks `04_ingame_spell_panel_latest` for the
    native selected-rune `C011_GRAPHIC_MENU_SPELL_AREA_LINES` brown/red pattern.
  - Matrix counts were refreshed: equipment/item icons and C011 spell cells are
    counted as matched for the current surfaces; inventory screen is
    `KNOWN_DIFF (narrowed)` rather than generic `UNPROVEN`.
- **Representative gates:**
  - `firestaff_m11_game_view_probe`: `418/418 invariants passed`
  - `ctest`: `5/5 PASS`
  - `run_firestaff_m11_ingame_capture_smoke.sh`: `PASS` with action/inventory
    icon palette split and C011 selected-rune checks.
- **Remaining gaps:**
  - Pointer/held-object icon surfaces are not implemented/audited yet.
  - Exact inventory and spell-panel placement still require original overlay
    evidence.
  - Viewport content/draw-order parity remains separate from these right-panel
    and capture-fixture improvements.

---

## All-graphics addendum: invented overlay classification — **LANDED (passes 102–105)**
- **Area:** `VISUAL` / `V1_PARITY_SURFACES`
- **Status:** The matrix now explicitly separates source-backed DM1 V1 surfaces
  from Firestaff convenience/placeholder overlays so they cannot accidentally be
  counted as parity wins.
- **Map overlay (passes 102–104):**
  - Local ReDMCSB PC `NEWMAP.C` is map-transition plumbing, not a player automap
    renderer.
  - Firestaff's `mapOverlayActive` / `m11_draw_fullscreen_map(...)` surface is
    classified as `KNOWN_DIFF (debug-only)`.
  - Pass 103 changed runtime behavior: default V1 chrome mode ignores
    `M12_MENU_INPUT_MAP_TOGGLE` unless `showDebugHUD=1`.
  - Probe gates `INV_GV_181`, `INV_GV_181B`, and `INV_GV_197` lock both the
    debug path and the default ignored path.
- **Dialog/endgame overlays (pass 105):**
  - ReDMCSB `DIALOG.C:F0427_DIALOG_Draw` expands the source dialog-box graphic,
    prints `V3.4`, applies dialog patch zones for 1/2/4 choices, and uses source
    choice/text zones and colours.
  - Firestaff's current visible `TEXT PLAQUE`, `PRESS ANY KEY TO DISMISS`,
    `QUEST COMPLETE`, `VICTORY AT TICK ...`, and `ESC TO RETURN TO MENU` panels
    are classified as functional placeholders / `KNOWN_DIFF`, not visual parity.
- **Dialog narrowing (passes 107–121):**
  - Pass 107 keeps the most obvious placeholder/debug labels (`TEXT PLAQUE`,
    `PRESS ANY KEY TO DISMISS`, `VICTORY AT TICK ...`, `ESC TO RETURN TO MENU`)
    hidden from default V1 unless `showDebugHUD=1`.
  - Passes 109–121 wire the visible source dialog path: `C000_GRAPHIC_DIALOG_BOX`
    backdrop (GRAPHICS.DAT graphic `17`, `224×136`), C450 `V3.4`, C469/C471
    message zones, source-width split/line step, C462–C467 choice text zones,
    M621/M622/M623 patch graphics, and choice hit/selection flow.
  - Matrix status remains `KNOWN_DIFF (narrowed)`: dialog `F0427` is now
    substantially source-backed, but original overlay comparison is still needed.
- **Endgame narrowing (passes 122–123, 127, 2957–2976):**
  - Default V1 game-won overlay no longer uses the invented victory panel when
    source assets are available.
  - Wired source endgame graphics: `C006_GRAPHIC_THE_END` (graphic `6`),
    `C346_GRAPHIC_WALL_ORNAMENT_43_CHAMPION_MIRROR` (graphic `346`) in C412–C415,
    champion portrait blits in C416–C419, restart/quit boxes from `DATA.C`,
    champion names at x=87/y=14+48n, raw champion titles, and source skill-title
    lines using non-temporary lifecycle skill levels.
  - Pass 2957–2976 exposes the endgame geometry through probeable helpers so the
    renderer no longer carries a separate pocket of hardcoded C412–C419/text/
    button coordinates.
  - Remaining endgame gaps: timing/music/restart loop and original overlay
    comparison captures.
- **Representative gates:**
  - `firestaff_m11_game_view_probe`: `536/536 invariants passed` after the
    latest dialog/endgame/inventory source-backed overlay work.
  - `ctest`: `5/5 PASS`.
- **Remaining gaps:**
  - Capture original dialog/endgame frames and perform overlay comparison.
  - Source-bind endgame timing/music/restart loop if required for lock.
  - Keep the map overlay debug-only unless contrary source/runtime evidence is
    found.

---

## Priority groupings

- **High (pass-37 … pass-41):** ownership wiring (#1 — **LANDED pass
  37**), stairs/door edge cases (#2, #3), measurable visual drifts
  with DEFS.H anchors (#4, #5).
- **Medium (pass-42 … pass-46):** invented chrome + text-as-graphic
  rework (#6, #7, #8), font bank (#9 — **LANDED pass 45**), palette (#10 — **LANDED pass 46**).
- **Blocked / tooling (pass-47):** overlays need capture infra (#11).
- **Lower (pass-48 … pass-50):** cosmetic tick prefix (#12, dup #13),
  behavioral probe binding (#14), audio (#15).
- **Passes 50–52 landed (2026-04-24):** SONG.DAT format/decoder
  (7/7 PASS), GRAPHICS.DAT SND3 decoder (6/6 PASS), and sound-event
  → SND3 mapping (5/5 PASS) — see `PASS50_AUDIO_FINDINGS.md`,
  `PASS51_AUDIO_FINDINGS.md`, and `PASS52_AUDIO_FINDINGS.md`.
  Passes 53–54 landed asset-backed SND3 SFX queueing and SONG.DAT one-cycle
  title-music queueing, but cadence/overlap/continuous-loop proof remains
  blocked on original runtime capture.

---

## Scope that is NOT a V1 blocker

These are listed to keep the ledger honest about what pass-37+ should
NOT spend time on:

- **M10 semantics:** passes 29–36 preserved M10.  M10 parity is not
  on this ledger; it is its own workstream.
- **V2 / V3:** out of scope per `STATUS.md` product direction.
- **M12 launcher / modern menu:** not on the V1 parity path.
- **Repo reorg:** postponed until DM1/V1 lock; not a parity item.

---

## Status after Passes 29–35 (as of 2026-04-24)

- 4 ownership passes landed (29, 30, 31, 32) with probe-verified
  compat owners for post-move env, movement legality, door actuator,
  and multi-sensor enumeration.  Runtime *wiring* for sensor
  enter/leave is still #1 on this ledger.
- 2 visual-lock measurement passes landed (33, 34) with concrete
  numeric drifts recorded in `parity-evidence/pass33_*.md` and
  `pass34_*.md`.
- 1 typography audit landed (35) with full enumeration of V1
  text-emitting surfaces in `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md`.
- Pass 36 itself (this file + the matrix honesty pass) does not land
  new code or new probes.

**Passes 29–36 together have not completed DM1/V1 parity.**  The
blocker ledger records what remains.

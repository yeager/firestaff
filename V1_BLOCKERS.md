# V1 Blocker Ledger — Firestaff DM1/V1 original-faithful mode

Last updated: 2026-04-24 (pass 40 landed)
Owner of this file: Pass 36 "honesty lock" (see `PASSLIST_29_36.md` §4.36)
Primary consumers: pass-38+ planning, `STATUS.md`

This is the **single source of truth** for what is still outstanding
between Firestaff and DM1 PC 3.4 English in V1 original-faithful mode
after Passes 29–35.  Every entry contains:

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
- **What pass 40 does NOT change:**
  - The runtime viewport rectangle is still `(12, 24, 196, 118)`.
    Binding the renderer to `M11_DM1_VIEWPORT_*` requires the
    invented chrome reroute tracked as blocker §6 (pass 42); doing
    that at pass 40 would destroy 2 914 px² of clickable HUD.
  - No HUD layout or hit-testing changes.  No M10 touch.  No change
    to `PARITY_MATRIX_DM1_V1.md` status value for this row (it
    remains `KNOWN_DIFF`); only the rationale is updated to cite
    the new anchor + probe.
- **Follow-up:** retirement of this entry is blocked on pass 42
  (invented chrome reroute), which itself depends on pass 41
  (portrait stride drop from +8 to C69) and pass 47b (ZONES.H parse
  for source-faithful side-column placement).  Full dependency chain
  is in `parity-evidence/pass40_viewport_lock.md` §4.

## 5. Champion status-box stride +8 px per slot vs DEFS.H C69
- **Area:** `VISUAL`
- **Evidence:**
  - Pass 34 `parity-evidence/pass34_sidepanel_rectangle_table.md` §3
    — `M11_PARTY_SLOT_STEP=77` vs `C69_CHAMPION_STATUS_BOX_SPACING=69`
    (DEFS.H:1756).
  - `PARITY_MATRIX_DM1_V1.md` §1 "Party/champion region" now
    `KNOWN_DIFF` (+8 px stride).
- **Suggested pass:** pass-41 — set `M11_PARTY_SLOT_STEP` to 69 in V1
  mode, measure downstream impact on slot contents.

## 6. Firestaff-invented UI chrome (utility strip, control strip, prompt strip, status lozenge, inspect readout)
- **Area:** `TEXT` (`VISUAL` adjacent)
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.2, §2.4, §2.5,
    §2.6 — 82 `m11_set_status` sites + 68 inspect-readout sites +
    Inspect/Save/Load captions + control/prompt strips at y=165.
  - `PARITY_MATRIX_DM1_V1.md` §4 "Over-labeling" now `KNOWN_DIFF`
    with the enumeration.
- **Suggested pass:** pass-42 — add a V1 chrome-mode switch that hides
  these Firestaff-invented surfaces and reroutes their notifications
  into the source-faithful message-log surface.

## 7. Champion HP/stamina/mana shown as numeric strings instead of bar graphs
- **Area:** `VISUAL`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.7 — DM1 renders
    these via `CHAMDRAW.C` bar graphs (ZONES C187..C190), Firestaff
    renders numeric glyph strings.
- **Suggested pass:** pass-43 — replace the numeric strip with the
  bar-graph renderer.  Asset is already extracted.

## 8. Spell-panel rune labels still text, not `C011` graphic blit
- **Area:** `VISUAL`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.8 — source uses
    `C011_GRAPHIC_MENU_SPELL_AREA_LINES` (14×39) for rune glyphs.
  - Pass 34 asset-usage map confirms index 11 is already wired as
    `M11_GFX_SPELL_AREA_LINES` but currently as backdrop, not as
    the per-rune label.
- **Suggested pass:** pass-44 — route each entered-rune readout
  through a sub-sprite of graphic 11.

## 9. Custom 7-pixel font used in place of original GRAPHICS.DAT font atlas
- **Area:** `TEXT`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.1 — `m11_draw_text`
    has a delegation path through `g_activeOriginalFont` but it is
    not wired to the extracted font bank.
- **Suggested pass:** pass-45 — wire the extracted GRAPHICS.DAT font
  atlas into `g_activeOriginalFont` and make the delegate the default
  path.

## 10. VGA palette compat layer still uses EGA-style colors
- **Area:** `VISUAL` (`BEHAVIOR` adjacent for brightness)
- **Evidence:**
  - `PARITY_MATRIX_DM1_V1.md` §3 "Base palette" / "Brightness levels"
    / "Cyan invariant" / "Special palettes" rows — all `KNOWN_DIFF`
    with concrete deltas in `parity-evidence/dm1_v1_pass1_palette_and_assets.md`.
  - Predates Passes 29–36; left intentionally untouched so this work
    does not bundle palette changes in.
- **Suggested pass:** pass-46 — land the `recovered_palette.json`
  values in `vga_palette_pc34_compat.c`.  This is a single bounded
  swap; defer brightness / creature palettes to a follow-up.

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
  honestly.  Full viewport content overlay still requires the
  DOSBox path to run end-to-end (user must install DOSBox + we need
  a KeyMapper / autoexec keystroke plan).
- **Suggested pass:** pass-47b — ZONES.H parse (from `PANEL.C` +
  `COORD.C` layout-record init) and DOSBox keystroke automation plan.

## 12. Tick-prefix (`Tn:`) on message log lines
- **Area:** `TEXT`
- **Evidence:**
  - `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` Pass 35 §2.3 — the tick
    prefix is a Firestaff convention, not source-faithful.
- **Suggested pass:** pass-48 — drop the `Tn:` prefix in V1 mode
  (single-line change in `m11_log_event`).

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
- **Placeholder audio is still in use at runtime.**  Pass 50 did
  not wire decoded buffers into `audio_sdl_m11.c`.
- **Remaining gaps before V1 audio can be called
  original-faithful** (enumerated in `PASS50_AUDIO_FINDINGS.md`
  §5):
  1. GRAPHICS.DAT SND3 loader + probe (in-game SFX bank)
  2. Sound-event → SND3 index mapping table
  3. Runtime integration in `audio_sdl_m11.c` (replace
     procedural buffers with decoded ones, gated on presence
     of original assets)
  4. Title-music playback driver that walks SEQ2 words and
     concatenates SND8 buffers with the bit-15 loop-back
  5. Sample-rate handling (22050 stream vs 11025 SND8 vs
     6000 SND3 — resample or per-source reconfig)
  6. Bug-faithful playback quirks cataloged when relevant
- **Suggested follow-up pass:** pass-51 — GRAPHICS.DAT SND3
  loader + probe, mirrored on Pass 50's scope discipline.

---

## Priority groupings

- **High (pass-37 … pass-41):** ownership wiring (#1 — **LANDED pass
  37**), stairs/door edge cases (#2, #3), measurable visual drifts
  with DEFS.H anchors (#4, #5).
- **Medium (pass-42 … pass-46):** invented chrome + text-as-graphic
  rework (#6, #7, #8), font bank (#9), palette (#10).
- **Blocked / tooling (pass-47):** overlays need capture infra (#11).
- **Lower (pass-48 … pass-50):** cosmetic tick prefix (#12, dup #13),
  behavioral probe binding (#14), audio (#15).
- **Pass 50 landed (2026-04-24):** SONG.DAT DM PC v3.4 format +
  decoder + probe (6/6 PASS) — see `PASS50_AUDIO_FINDINGS.md`.
  Runtime wiring remains; placeholder audio is still in use.

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

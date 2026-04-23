# Firestaff DM1/V1 Pass List — Passes 29–36

Last updated: 2026-04-23
Author model: Opus 4.6 (planning only; no implementation)
Scope: **DM1 / PC 3.4 / English / V1 original-faithful mode**
Primary reference: **ReDMCSB** local dump at `../redmcsb-output/I34E_I34M/`

This document is **planning only**. It does not change source, does not reorganize the
repo, and does not claim any pass here is already done.

---

## 0. Program intent

Passes 29–36 are a **single coherent program**, not eight independent ideas. They
execute the two grounding analyses that now drive DM1/V1 convergence:

- `REDMCSB_FIRESTAFF_1TO1_ANALYSIS_AND_PLAN.md`
- `M11_OWNERSHIP_AUDIT_MOVEMENT_DOORS_ENV.md`

Those analyses say, with evidence:

1. Firestaff’s **observable behavior is still too often owned by custom M11 glue**
   (`m11_game_view.c`), especially for movement consequences, door toggles,
   pit/teleporter handling, and sensor side effects.
2. Firestaff’s **visual parity is under-measured**: we have many probes and
   screenshots but very few coordinate-locked overlays against ReDMCSB / original.
3. Firestaff’s **parity matrix still contains vague or unprovable claims**.

Passes 29–36 tackle these in three bands, in order:

| Band | Passes | Purpose |
|------|--------|---------|
| A. **Ownership migration (reduces custom M11 ownership)** | 29, 30, 31, 32 | Push observable behavior down out of `m11_game_view.c` into source-faithful compat/runtime owners that mirror ReDMCSB `MOVESENS.C`, `DUNGEON.C`, `COMMAND.C`. |
| B. **Measured visual-parity lock (locks visual parity, not generic polish)** | 33, 34 | Produce coordinate-locked overlays for viewport and side-panel components against ReDMCSB / original. Measured, not vibes. |
| C. **Dependency / honesty passes** | 35, 36 | Audit text-vs-graphics drift in V1 surfaces and then hard-lock the parity matrix to only source-backed MATCHED rows. |

Passes 29→32 directly implement the bounded slices recommended by the M11 ownership
audit ("Recommended next pass order", items A → D). Passes 33→34 implement the
"Visual parity workstream" from the 1:1 plan (V1, V2). Passes 35→36 are the
"Typography / player-facing text" slice plus the "Honesty lock" milestone (P4).

**No single pass in this list alone completes DM1/V1 parity.** Each pass states
this explicitly in its success criteria.

---

## 1. Out of scope for the whole program (29–36)

These are hard, cross-pass constraints:

- No repo reorganization (postponed until DM1/V1 lock).
- No V2/V3 visual or rendering work.
- No changes to M10 verify-gate semantics. M10 probes must stay green.
- No speculative rewrite of `main_loop_m11.c` or `m11_game_view.c` beyond the
  bounded ownership migrations described in each pass.
- No palette/color table changes (already addressed in commit `88a5283`).
- No projectile/explosion visual changes (already addressed in commits
  `5b7f46f`, `9cbadbe`, `9e21c2a`, `83fae5b`).
- No modern M12 launcher/menu work; M12 is explicitly not V1 parity progress.
- No "close enough" claims. If the probe is green but the screenshot or behavior
  is still visibly off DM1/ReDMCSB, say so in the pass report.
- Tracked repo content in English.

---

## 2. Verification gates referenced across passes

All passes must run this minimum set when they touch code. Every pass states
any extra probes it also requires.

- `./run_firestaff_m11_phase_a_probe.sh`
- `./run_firestaff_m11_game_view_probe.sh`
- `./run_firestaff_m11_launcher_smoke.sh`
- `./run_firestaff_m10_verify.sh "$HOME/.firestaff/data/GRAPHICS.DAT"`
- `./run_firestaff_m11_verify.sh`
- Plus the area-specific M10 probe(s) for the touched slice (e.g.
  `run_firestaff_m10_dungeon_doors_sensors_probe.sh`).

Baseline invariants that must remain green after every pass:

- Phase A: 18/18.
- M11 game view: 175/175.
- M11 audio: 4/4.

---

## 3. Ownership legend

Each pass is tagged with the ownership axis it primarily acts on:

- `OWNERSHIP` — pass reduces `m11_game_view.c` / `main_loop_m11.c` custom ownership
  and moves behavior into source-faithful compat/runtime (`memory_*_pc34_compat.*`).
- `VISUAL-LOCK` — pass produces measured, coordinate-locked comparison evidence
  against ReDMCSB / original and fixes measured drift.
- `HONESTY` — pass updates parity claims to only source-backed MATCHED rows.

---

## 4. Pass list

---

### Pass 29 — Post-move environment migration (pit + teleporter chain)

**Tags:** `OWNERSHIP`
**Maps to:** M11 ownership audit "Pass A — post-move environment migration".
**Dependencies:** none (cleanest first cut; moves already-visible behavior).

#### 4.29.1 Primary goal
Move observable post-move environment consequences — pit fall, teleporter
application, and chained teleporter/pit continuation — out of `m11_game_view.c`
and into compat-owned movement-result processing, so the ownership boundary
mirrors ReDMCSB `F0267_MOVE_GetMoveResult_CPSCE`.

#### 4.29.2 Exact scope
- Introduce or expand a compat-owned post-move resolution entrypoint inside
  `memory_movement_pc34_compat.c` (or a new sibling TU only if necessary) that
  owns:
  - pit detection and fall application (damage, map transition).
  - teleporter detection, lookup, application (target square + rotation).
  - chained teleporter/pit continuation with a bounded iteration cap equivalent
    to the cap already used in `m11_check_post_move_transitions`.
- Route the tick orchestrator (`memory_tick_orchestrator_pc34_compat.c`,
  `memory_tick_orchestrator_pc34_compat.h`) to call this compat owner after a
  party move, in place of the M11 helpers.
- In `m11_game_view.c`, reduce `m11_apply_tick` to invoking the compat owner
  and remove (or explicitly mark deprecated) the behavioral bodies of:
  - `m11_check_pit_fall`
  - `m11_check_teleporter`
  - `m11_check_post_move_transitions`
- Keep `m11_try_stairs_transition` out of scope in this pass (see Pass 30).

#### 4.29.3 Key Firestaff files likely involved
- `memory_movement_pc34_compat.c`, `memory_movement_pc34_compat.h`
- `memory_tick_orchestrator_pc34_compat.c`, `memory_tick_orchestrator_pc34_compat.h`
- `m11_game_view.c` (reduction only)
- `memory_sensor_execution_pc34_compat.c` (consult, not rewrite)

#### 4.29.4 Key ReDMCSB references
- `MOVESENS.C`:
  - `F0267_MOVE_GetMoveResult_CPSCE` (chained moves, pit/teleporter consequences)
- `DUNGEON.C` (square/thing semantics for pits and teleporters)
- `COMMAND.C` (command dispatch, to confirm it does **not** own these rules)

#### 4.29.5 Explicitly out of scope
- Door passability / open-close semantics (Pass 30, Pass 31).
- Sensor addition/removal (Pass 32).
- Wall-click / front-cell sensor behavior (Pass 31).
- Stairs traversal (Pass 30).
- Any render, text, or palette change.
- Any change to creature walkability helpers.

#### 4.29.6 Required verification / gates
- Minimum verification set from §2.
- `run_firestaff_m10_dungeon_doors_sensors_probe.sh` (regression).
- `run_firestaff_m10_dungeon_monsters_items_probe.sh` (regression).
- Add or extend a bounded scenario probe that exercises:
  1. party walks onto a pit square → falls + takes damage + map index updates.
  2. party walks onto a teleporter → target square and rotation applied.
  3. chained teleporter → pit → teleporter terminates under the iteration cap.
- Screenshot evidence (viewport + message log) for all three scenarios, stored
  under `verification-m11/` or equivalent.

#### 4.29.7 Success criteria
- `m11_apply_tick` no longer calls the three helper bodies for environment
  consequences; the compat owner does.
- All baseline invariants green.
- Scenario probes green.
- ReDMCSB code reference cited line-range in the commit body and in the pass
  report.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 30 — Movement legality migration (doors, fake walls, stairs, square semantics)

**Tags:** `OWNERSHIP`
**Maps to:** M11 ownership audit "Pass B — movement legality migration".
**Dependencies:** Pass 29 (shared compat movement surface area).

#### 4.30.1 Primary goal
Expand `F0702_MOVEMENT_TryMove_Compat` from a wall-only prefilter into a
source-semantic movement-legality owner that handles doors (passability by
open/closed state and door type), fake walls, and stairs consequence ownership,
so both party and creature movement can consult one source-faithful owner.

#### 4.30.2 Exact scope
- Extend `F0702_MOVEMENT_TryMove_Compat` to resolve:
  - door passability based on square door-state bits.
  - fake-wall passability per ReDMCSB square-semantics helpers.
  - stairs detection and mapping transition (owns what
    `m11_try_stairs_transition` owns today, minus any UI concerns).
- Migrate `m11_try_stairs_transition` behavior into the compat owner and
  reduce the M11 function to UI routing / input binding only, or deprecate it.
- Expose shared square-semantics helpers so `m11_square_walkable_for_creature`
  can delegate to them (wiring only — full AI unification is Pass 32/deferred).
- No sensor processing in this pass (Pass 32).

#### 4.30.3 Key Firestaff files likely involved
- `memory_movement_pc34_compat.c`, `memory_movement_pc34_compat.h`
- `memory_tick_orchestrator_pc34_compat.c` (delegation site for stairs)
- `m11_game_view.c` (reduction of `m11_try_stairs_transition`; delegation of
  `m11_square_walkable_for_creature`)
- Any dungeon/square accessor header used by the above.

#### 4.30.4 Key ReDMCSB references
- `MOVESENS.C`:
  - `F0267_MOVE_GetMoveResult_CPSCE` (adjacent vs teleport distinction,
    party/group side effects)
- `DUNGEON.C` (door definitions/attributes, fake-wall helpers,
  square-semantic helpers)
- `COMMAND.C` (dispatch boundary — confirms it does not own legality)

#### 4.30.5 Explicitly out of scope
- Sensor addition/removal processing (Pass 32).
- Front-door toggle / click-on-wall routing (Pass 31).
- Any render, text, or palette change.
- Projectile/group interaction consequences (tracked separately).

#### 4.30.6 Required verification / gates
- Minimum verification set from §2.
- `run_firestaff_m10_dungeon_doors_sensors_probe.sh`.
- `run_firestaff_m10_dungeon_tile_probe.sh` and
  `run_firestaff_m10_dungeon_things_probe.sh`.
- Add bounded probe cases:
  1. party is blocked by a closed door.
  2. party walks through an open door.
  3. party passes through a fake wall when original rules allow.
  4. party traverses up/down stairs; map index and orientation update per
     ReDMCSB semantics.
- Before/after scenario screenshots archived.

#### 4.30.7 Success criteria
- `F0702_MOVEMENT_TryMove_Compat` file header comment no longer says "doors /
  fake walls are future work".
- `m11_try_stairs_transition` is either removed or a pure delegation shim.
- `m11_square_walkable_for_creature` now shares helpers with the compat owner,
  with a tracked note if full unification is deferred.
- All baseline invariants green.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 31 — Door actuation + wall-click sensor routing migration

**Tags:** `OWNERSHIP`
**Maps to:** M11 ownership audit "Pass D — front-door / wall interaction
migration", grouped with wall-click routing because both touch the same
`m11_game_view.c` region and share a ReDMCSB owner.
**Dependencies:** Pass 30 (uses the shared square-semantics helpers).

#### 4.31.1 Primary goal
Remove direct M11 mutation of door-state bits and direct M11 interpretation of
wall/front-cell clicks. Replace both with compat/runtime-owned action paths
modeled on ReDMCSB `F0275_SENSOR_IsTriggeredByClickOnWall` and the door
actuator/action path inside `MOVESENS.C` / `DUNGEON.C`.

#### 4.31.2 Exact scope
- Introduce or extend a compat owner in `memory_sensor_execution_pc34_compat.c`
  (and/or a sibling TU) that handles:
  - door toggle / open-close actions (replacing
    `m11_toggle_front_door` direct square-bit mutation, including its synthetic
    door/sound emissions).
  - click-on-wall / front-cell sensor trigger routing (replacing ad-hoc door
    click handling in `m11_game_view.c`).
- Reduce `m11_toggle_front_door` to a thin input → compat-call shim, or remove
  entirely if the compat owner is directly callable.
- Reduce `m11_front_cell_is_door` to a pure query that delegates to the shared
  square-semantic helpers from Pass 30.

#### 4.31.3 Key Firestaff files likely involved
- `memory_sensor_execution_pc34_compat.c`,
  `memory_sensor_execution_pc34_compat.h`
- `memory_movement_pc34_compat.c` / `.h` (square-semantics callouts)
- `m11_game_view.c` (reduction only)
- `memory_tick_orchestrator_pc34_compat.c` (action dispatch wiring)

#### 4.31.4 Key ReDMCSB references
- `MOVESENS.C`:
  - `F0275_SENSOR_IsTriggeredByClickOnWall`
  - `F0270` / `F0271` (local sensor effect + rotation batching)
  - door action paths inside the sensor result family
- `DUNGEON.C` (door types, destroyed-door handling, door-state bit semantics)
- `COMMAND.C` (confirms command dispatch is not the door authority)

#### 4.31.5 Explicitly out of scope
- Sensor addition/removal on enter/leave (Pass 32).
- Any visual/geometry change to the viewport or panel (Pass 33, 34).
- Sound-engine changes beyond keeping synthetic emissions consistent.
- AI-driven door interactions (deferred, tracked).

#### 4.31.6 Required verification / gates
- Minimum verification set from §2.
- `run_firestaff_m10_dungeon_doors_sensors_probe.sh` (the central probe here).
- Add a bounded wall-click scenario probe with at least:
  1. click front cell while facing a closed door → door opens via compat path.
  2. click front cell while facing a pressure plate / button-equivalent wall →
     the compat sensor path is exercised, not the M11 helper.
- Message-log and screenshot evidence archived.

#### 4.31.7 Success criteria
- `m11_toggle_front_door` no longer owns world mutation — it is input glue only.
- No direct square-bit writes for door state in `m11_game_view.c`.
- All baseline invariants green.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 32 — Sensor addition/removal processing for party movement

**Tags:** `OWNERSHIP`
**Maps to:** M11 ownership audit "Pass C — sensor ownership migration".
**Dependencies:** Passes 29, 30, 31 (compat movement + door/actuator paths
must exist before sensor add/remove can hook in cleanly).

#### 4.32.1 Primary goal
Make Firestaff's compat layer own sensor enter/leave/add/remove processing for
party movement, modeled on ReDMCSB `F0276_SENSOR_ProcessThingAdditionOrRemoval`,
so that environment reactions (text, teleport, actuator side-effects) fire from
the runtime path rather than from probe-grade one-sensor `WALK_ON` code or from
M11 helpers.

#### 4.32.2 Exact scope
- Expand `memory_sensor_execution_pc34_compat.c` beyond the current
  "first sensor, WALK_ON, teleport/text only" probe surface to handle:
  - multi-sensor ordering on a square.
  - enter / leave / add / remove events on party movement.
  - at minimum the sensor-effect categories already referenced by existing V1
    screens (teleport, text, message, actuator triggers used by doors from
    Pass 31). Unsupported categories remain **explicitly tracked** as stubs in
    code comments and in the pass report.
- Hook the tick orchestrator so that compat movement emits sensor events on
  square leave and square enter for the party, and the compat sensor owner
  consumes them in the same order as ReDMCSB.
- Remove any remaining M11-side sensor execution if it still exists after
  Passes 29 and 31.

#### 4.32.3 Key Firestaff files likely involved
- `memory_sensor_execution_pc34_compat.c`,
  `memory_sensor_execution_pc34_compat.h`
- `memory_movement_pc34_compat.c` / `.h`
- `memory_tick_orchestrator_pc34_compat.c`
- `m11_game_view.c` (final reduction, if any remains)

#### 4.32.4 Key ReDMCSB references
- `MOVESENS.C`:
  - `F0276_SENSOR_ProcessThingAdditionOrRemoval`
  - `F0270` / `F0271` (local effect batching / rotation)
- `DUNGEON.C` (sensor category definitions)
- `TIMELINE.C` (only as consult, to confirm non-ownership of this flow)

#### 4.32.5 Explicitly out of scope
- Creature-group sensor interactions (party-only here).
- New sensor categories beyond what Passes 29–31 already required.
- Any refactor of the runtime tick orchestrator beyond hook points.
- All rendering / text / visual changes.

#### 4.32.6 Required verification / gates
- Minimum verification set from §2.
- `run_firestaff_m10_dungeon_doors_sensors_probe.sh`.
- Add bounded multi-sensor scenario probes:
  1. a square with two sensors in the original order → both fire in correct
     order on enter.
  2. a square with a leave-sensor → fires on leave, not on re-enter.
  3. a square with an add/remove-sensor (e.g. party moves an item off it) →
     compat owner receives the event.
- Evidence (logs + screenshots) archived.

#### 4.32.7 Success criteria
- `memory_sensor_execution_pc34_compat.c` header comment no longer says
  "probe-grade" for party movement.
- Compat owner documented as the runtime owner for party sensor events in
  `M11_OWNERSHIP_AUDIT_MOVEMENT_DOORS_ENV.md` (update the table rows).
- All baseline invariants green.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 33 — Viewport coordinate-locked overlay (measured visual parity)

**Tags:** `VISUAL-LOCK`
**Maps to:** 1:1 plan "Stage 5 / V1. Viewport rectangle, horizon, wall / floor /
ceiling composition".
**Dependencies:** none against 29–32 (parallel-safe, but scheduled after
ownership passes because fixing ownership can change which frames are
authoritative).

#### 4.33.1 Primary goal
Produce **measured, coordinate-locked overlay evidence** of the Firestaff
in-game viewport against ReDMCSB / original screenshots, and record the exact
pixel-level deltas. Replace any remaining "looks close" claims about the
viewport with a numeric drift table.

#### 4.33.2 Exact scope
- Capture reference frames:
  - ReDMCSB-driven frames where possible (using the local ReDMCSB dump).
  - Original DM1 PC 3.4 VGA capture frames already archived under
    `parity-evidence/` or a successor folder.
- Capture Firestaff frames at the same known DM1 scenarios
  (entry square, first corridor, first junction, facing closed door, facing
  open door, facing pit, facing teleporter).
- Produce side-by-side and difference overlays for each scenario at 320×200
  with documented X/Y origin.
- Produce a measured coordinate table for viewport anchors:
  `viewport_x`, `viewport_y`, `viewport_w`, `viewport_h`, horizon row,
  wall depth rows, floor rows, ceiling rows.
- File the measured deltas into `PARITY_MATRIX_DM1_V1.md` §1 "Viewport region
  bounds" and §1 "Main game screen" rows (which are currently `UNPROVEN`).
- **Docs + evidence only.** Any code change to match coordinates is a
  follow-up pass, not part of 33.

#### 4.33.3 Key Firestaff files likely involved
- `verification-screens/` (capture output)
- `parity-evidence/` (evidence filing)
- `PARITY_MATRIX_DM1_V1.md` (matrix updates only)
- `M11_V1_SCREENSHOT_GAP_SPEC.md` (reference for captures)
- Driver scripts: `run_firestaff_headless_driver.sh` and related.

#### 4.33.4 Key ReDMCSB references
- `DUNVIEW.C`, `DRAWVIEW.C` (viewport composition owners)
- `VIDEODRV.C` (mode / palette anchor — already leveraged in palette pass)
- `DEFS.H` constants: `C112_BYTE_WIDTH_VIEWPORT`, `C136_HEIGHT_VIEWPORT`,
  `C000_DERIVED_BITMAP_VIEWPORT`, `G8177_c_ViewportColorIndexOffset`.

#### 4.33.5 Explicitly out of scope
- Any change to `m11_game_view.c` rendering code.
- Palette changes.
- Side-panel measurement (Pass 34).
- Typography work (Pass 35).
- Any parity matrix row that is not in §1 of `PARITY_MATRIX_DM1_V1.md`.

#### 4.33.6 Required verification / gates
- Capture reproducibility: the driver script invocations used to produce
  Firestaff frames must be committed and reproducible.
- Minimum verification set from §2 (no code change expected, but probes
  must stay green because capture can touch the headless driver config).
- An artifact manifest for each scenario: source frame path, Firestaff frame
  path, diff frame path, measured table entry, commit SHA.

#### 4.33.7 Success criteria
- For each of the listed scenarios, a side-by-side overlay exists at the
  documented location.
- `PARITY_MATRIX_DM1_V1.md` §1 rows are updated from `UNPROVEN` to either
  `MATCHED` (with evidence path) or `KNOWN_DIFF` (with numeric delta).
- No row is marked `MATCHED` without an attached evidence path.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 34 — Side-panel component rectangle table (measured visual parity)

**Tags:** `VISUAL-LOCK`
**Maps to:** 1:1 plan "Stage 5 / V2. Side panel / portraits / action area /
inventory / spell panel".
**Dependencies:** Pass 33 (same capture infrastructure and coordinate origin).

#### 4.34.1 Primary goal
Produce a measured component rectangle table for the right-column side panel
(party HUD, action area, spell area, status boxes, message log, and inventory
overlay when open), each rectangle compared against ReDMCSB `PANEL.C`,
`CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C` and original screenshots. Fix drift
only in the parity matrix; code-side fixes are separate passes.

#### 4.34.2 Exact scope
- For each of the following components, produce a measured rectangle table
  entry (x, y, w, h, anchor) for both Firestaff and ReDMCSB/original:
  - party HUD slots (all 4).
  - portrait regions within a slot.
  - health / stamina / mana / load / injury strips.
  - action area frame.
  - action-menu rows (confirm output from pass 27/28 rendering fidelity work).
  - spell area frame.
  - rune cell grid + symbol readout bed.
  - left + right status boxes.
  - message-log rect and baseline grid.
  - inventory-overlay frame and slot grid (when open).
- Produce an asset usage map per component: GRAPHICS.DAT index(es) used vs
  ReDMCSB slot usage.
- Produce a text-vs-graphics flag per component (feeds Pass 35).
- File measured drift into `PARITY_MATRIX_DM1_V1.md` §1 and §2 rows.
- **Docs + evidence only.**

#### 4.34.3 Key Firestaff files likely involved
- `m11_game_view.c` (read-only reference for current rectangles)
- `verification-screens/`, `parity-evidence/`
- `PARITY_MATRIX_DM1_V1.md`
- `extracted-graphics-v1/` (asset index cross-reference)

#### 4.34.4 Key ReDMCSB references
- `PANEL.C` (panel composition)
- `CHAMPION.C` (champion slot content)
- `PORTRAIT.C` (portrait region rules)
- `SPELDRAW.C` (spell-area geometry and rune cells)
- `MENU.C` / `MENUDRAW.C` (only to confirm non-ownership for in-game panel)
- `DEFS.H` constants that pin panel geometry.

#### 4.34.5 Explicitly out of scope
- Viewport coordinates (Pass 33).
- Title / startup / menu visuals.
- Typography choice (Pass 35 gathers; no fix here).
- Any behavioral change in the action / spell / inventory logic.
- V2 HUD work.

#### 4.34.6 Required verification / gates
- Captures for panel components exist and are reproducible.
- Minimum verification set from §2 stays green.
- Evidence manifest as in Pass 33.

#### 4.34.7 Success criteria
- Measured rectangle table committed under `parity-evidence/` (or equivalent)
  with one row per component.
- Asset usage map committed.
- Text-vs-graphics flag list ready for Pass 35.
- `PARITY_MATRIX_DM1_V1.md` §1 panel rows and §2 sprite/asset rows updated
  to `MATCHED` (with evidence) or `KNOWN_DIFF` (with numeric delta).
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 35 — Typography and text-vs-graphics audit in V1 surfaces

**Tags:** `VISUAL-LOCK`, `HONESTY`
**Maps to:** 1:1 plan "Stage 5 / V4. Typography and player-facing text".
**Dependencies:** Pass 34 (consumes its text-vs-graphics flag list).

#### 4.35.1 Primary goal
Produce an auditable list of V1-visible surfaces in `m11_game_view.c` that
currently use custom / text-heavy rendering where DM1 / ReDMCSB uses original
graphics. Classify each instance as "replace with original graphic",
"keep as text (source-backed)", or "track as KNOWN_DIFF". No implementation.

#### 4.35.2 Exact scope
- Enumerate every in-game text emission path in `m11_game_view.c`, including:
  - status / inspect readouts.
  - message filtering / status text policy.
  - inventory / action / prompt panel text overlays.
  - any hard-coded caption or label.
- For each, cite ReDMCSB owner and expected graphic index (if any) from
  `TEXT.C`, `TEXT2.C`, or panel/inventory/spell component source.
- For each, produce a one-line classification:
  - `REPLACE_WITH_GRAPHIC <index>` — original used a bitmap.
  - `KEEP_AS_TEXT <TEXT.C/TEXT2.C ref>` — original used the text engine.
  - `KNOWN_DIFF` — Firestaff custom, no current source-faithful plan.
- Cross-reference each entry against extraction manifests under
  `extracted-graphics-v1/`.
- File into a new evidence doc (e.g. `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md`)
  and link from `PARITY_MATRIX_DM1_V1.md` §2 ("Text-tag fallbacks" row, etc.).
- **Docs only. No code changes.**

#### 4.35.3 Key Firestaff files likely involved
- `m11_game_view.c` (enumeration target, read-only)
- Font / text helpers referenced by `m11_game_view.c` (read-only)
- `extracted-graphics-v1/` (manifest cross-reference)
- `PARITY_MATRIX_DM1_V1.md` (link + row updates)

#### 4.35.4 Key ReDMCSB references
- `TEXT.C`, `TEXT2.C` (text engine ownership)
- `PANEL.C`, `CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C` (panel text vs graphic
  split)

#### 4.35.5 Explicitly out of scope
- Fixing any of the drift found (that becomes a follow-up pass per item).
- Adding new fonts or rendering features.
- Any V2 typography work.

#### 4.35.6 Required verification / gates
- Cross-reference completeness check: every text-emitting symbol in
  `m11_game_view.c` appears exactly once in the audit.
- Minimum verification set from §2 stays green (no code change expected).

#### 4.35.7 Success criteria
- `PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md` exists with full enumeration, one
  classification per entry, and ReDMCSB citation per entry.
- `PARITY_MATRIX_DM1_V1.md` §2 rows updated where the audit produces new
  `KNOWN_DIFF` entries.
- Pass report explicitly says: *"This pass alone does not complete DM1/V1
  parity."*

---

### Pass 36 — V1 parity honesty lock (matrix and V1 blocker ledger)

**Tags:** `HONESTY`
**Maps to:** 1:1 plan Milestone P4 "Honesty lock" and Milestone P0 "Baseline
truth pass".
**Dependencies:** Passes 29–35. This pass consumes their evidence and states
the remaining V1 gap as a single ledger.

#### 4.36.1 Primary goal
Bring `PARITY_MATRIX_DM1_V1.md` into a strictly source-backed state: every
`MATCHED` row has an evidence path, every vague claim is downgraded, and a
fresh, numbered V1 blocker ledger captures the outstanding work after passes
29–35.

#### 4.36.2 Exact scope
- Walk every row in `PARITY_MATRIX_DM1_V1.md`:
  - If `MATCHED` without an attached evidence path → downgrade to `UNPROVEN`
    or `KNOWN_DIFF` until evidence is attached.
  - If `UNPROVEN` but now has evidence from Passes 29–35 → promote to
    `MATCHED` and attach the path.
  - If `KNOWN_DIFF` → ensure numeric delta or file reference is present.
- Produce a new `V1_BLOCKERS.md` (or equivalent single file) listing the top
  outstanding V1 blockers, each entry containing:
  - short title.
  - area tag (`OWNERSHIP`, `VISUAL`, `BEHAVIOR`, `ASSET`, `TEXT`).
  - evidence path or reason it's a blocker.
  - suggested next bounded pass label (numbered pass-37, pass-38, …).
- Update `STATUS.md` to point at `V1_BLOCKERS.md` as the single source of
  "what's left for DM1/V1".
- Update the ownership table in `M11_OWNERSHIP_AUDIT_MOVEMENT_DOORS_ENV.md`
  to reflect what Passes 29–32 actually migrated (the table is currently a
  pre-migration snapshot).
- **Docs only.**

#### 4.36.3 Key Firestaff files likely involved
- `PARITY_MATRIX_DM1_V1.md`
- `M11_OWNERSHIP_AUDIT_MOVEMENT_DOORS_ENV.md`
- `STATUS.md`
- New: `V1_BLOCKERS.md`

#### 4.36.4 Key ReDMCSB references
- Whichever ReDMCSB files were cited by Passes 29–35 (cited again only
  where a downgrade/upgrade requires it).

#### 4.36.5 Explicitly out of scope
- Any code change.
- Any new probe.
- Any reorg of parity documents beyond the specific updates above.
- Creating a new parity matrix structure; use the existing one.

#### 4.36.6 Required verification / gates
- Row-by-row checklist: every `MATCHED` row has an evidence path link that
  resolves inside the repo.
- Every `KNOWN_DIFF` row has either a numeric delta or a file reference.
- `V1_BLOCKERS.md` entries each reference at least one pass or evidence
  artifact.

#### 4.36.7 Success criteria
- No `MATCHED` row lacks evidence.
- `V1_BLOCKERS.md` exists, is referenced from `STATUS.md`, and contains the
  prioritized list for pass-37 onward.
- The M11 ownership audit table reflects post-migration reality.
- Pass report explicitly says: *"Passes 29–36 together have not completed
  DM1/V1 parity; the V1 blocker ledger records what remains."*

---

## 5. Sequence summary

| Pass | Title (short) | Band | Reduces M11 ownership? | Locks visual parity? | Depends on |
|------|---------------|------|------------------------|----------------------|------------|
| 29 | Post-move env migration (pit/teleporter) | A. Ownership | Yes | No | — |
| 30 | Movement legality (doors/fake walls/stairs) | A. Ownership | Yes | No | 29 |
| 31 | Door actuation + wall-click sensor routing | A. Ownership | Yes | No | 30 |
| 32 | Sensor add/remove for party movement | A. Ownership | Yes | No | 29, 30, 31 |
| 33 | Viewport coordinate-locked overlay | B. Visual-lock | No | Yes (viewport) | — |
| 34 | Side-panel component rectangle table | B. Visual-lock | No | Yes (panel) | 33 |
| 35 | Typography and text-vs-graphics audit | B+C. Visual-lock + Honesty | No (documents) | Documents drift | 34 |
| 36 | V1 parity honesty lock + blocker ledger | C. Honesty | No (documents) | Documents drift | 29–35 |

This program is designed to land in order: Passes 29→32 remove the largest
ownership risks identified by the M11 audit; Passes 33→34 replace "looks
close" with measured drift; Passes 35→36 convert the repo's parity story from
vague claims to an evidence-backed ledger ready for pass-37 and beyond.

**Explicit non-claim:** executing all of 29–36 does **not** complete DM1/V1
parity. It converts Firestaff's state from "many reconstructed seams with
under-measured visuals and mixed ownership" into "source-backed ownership
for movement/doors/env, measured visuals for viewport and side-panel, and
an honest ledger of what remains" — which is the realistic next stop on the
path the 1:1 analysis and ownership audit laid out.

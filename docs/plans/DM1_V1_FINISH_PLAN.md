# DM1 V1 finish plan — consolidated, source-first

Updated: 2026-04-29
Current N2 head: `5ad4d70 Add pass161 C080 viewport click source gate`

## Objective

Finish DM1 PC 3.4 / V1 original-faithful mode without more scattered micro-passes.

Definition of done:

1. Normal V1 route can enter controllable dungeon gameplay with a real party.
2. The four must-have reference frames exist for both original and Firestaff:
   - `party_hud`
   - `gameplay_viewport`
   - `spell_panel`
   - `inventory_panel`
3. Those frames are compared by deterministic overlay/diff gates.
4. Remaining source-backed behavior deltas are either fixed or recorded as explicit `KNOWN_DIFF` with evidence.
5. Baseline gates pass and the final state is committed cleanly.

## Operating rule

ReDMCSB source is the oracle. Captures verify; captures do not invent behavior.

For every remaining pass:

```
question -> ReDMCSB source lines -> Firestaff implementation/probe -> evidence gate -> commit
```

No more generic “explore viewport/HUD/inventory” passes. Each pass must close a named blocker.

## Current ground truth

Recent source/evidence locks:

- `9557335` pass159: source-zone tables for viewport object/projectile/creature anchors.
- `5820772` pass160: ReDMCSB source-lock gate for title/entrance/C007/C080/CLIKVIEW/MOVESENS/G4055/DRAWVIEW.
- `5ad4d70` pass161: executable C007 viewport C080 route edge gate.

Remaining hard blocker:

- Original-route blocker: static no-party hash `48ed3743ab6a` / direct-start no-party signature.
- Until this is solved, runtime pixel parity claims are invalid even if source gates pass.

## Stop doing

- Stop spawning broad “viewport parity”, “HUD pass”, “inventory pass” workers with vague goals.
- Stop adding probes that only prove internal consistency unless they retire a matrix row.
- Stop chasing emulator screenshots before the route is party/control-ready.
- Stop splitting the same lane across parallel micro-workers.

## Workstreams, in order

### Lane A — unblock original party/control route

Goal: get original DOS DM1 into real party-controlled gameplay, not static no-party frames.

Source files to use first:

- `ENTRANCE.C` — `F0441_STARTEND_ProcessEntrance`, `F0438_STARTEND_OpenEntranceDoors`
- `TITLE.C` — title wait/handoff timing, `Delay(25L)`, `BUG0_71`
- `COMMAND.C` — input tables and command dispatch
- `CLIKVIEW.C` — C080 click-in-dungeon-view behavior
- `MOVESENS.C` — party/object movement and wall-click sensors

Deliverables:

1. A route-state classifier that distinguishes:
   - title/menu
   - entrance
   - static no-party dungeon
   - real party/control gameplay
2. One reproducible original route command script.
3. Captures with non-static, party-control-ready proof.
4. Evidence directory: `parity-evidence/verification/pass162_original_party_route_unblock/`.

Exit gate:

- Reject hash `48ed3743ab6a` as non-ready.
- Accept only route frames with visible party/control semantic markers and frame-to-frame state change from input.

### Lane B — overlay-ready frame quartet

Start only after Lane A passes.

Goal: produce the four comparable original/Firestaff frame pairs.

Frames:

1. `party_hud`
2. `gameplay_viewport`
3. `spell_panel`
4. `inventory_panel`

Deliverables:

- Raw `320x200` original frames.
- Matching Firestaff frames generated from equivalent state.
- JSON manifest with command route, source state, hashes, and semantic label.
- Evidence directory: `parity-evidence/verification/pass163_overlay_frame_quartet/`.

Exit gate:

- Manifest confirms dimensions, semantic classes, unique hashes, and matching state labels.

### Lane C — viewport/world visuals final overlay

Start only after Lane B has `gameplay_viewport`.

Source files:

- `DUNVIEW.C`
- `DRAWVIEW.C`
- `VIEWPORT.C`
- `DUNGEON.C`
- `OBJECT.C`
- `MOVESENS.C`

Scope:

- walls/floor/ceiling
- doors/buttons
- wall/floor/door ornaments
- floor items
- creatures/projectiles/explosions
- draw order/clipping/occlusion

Deliverables:

- Overlay diff gate with masks for known dynamic regions only.
- One patch sequence that fixes real deltas, not speculative visuals.
- Evidence directory: `parity-evidence/verification/pass164_viewport_pixel_overlay/`.

Exit gate:

- Pixel diff under agreed threshold or every residual is classified with source evidence.

### Lane D — HUD/champion/status final overlay

Start only after Lane B has `party_hud`.

Source files:

- `CHAMDRAW.C`
- `CHAMPION.C`
- `PANEL.C`
- `TEXT.C`
- layout-696 / `DEFS.H` zones

Scope:

- champion portraits/icons
- status boxes
- HP/stamina/mana bars
- name zones
- leader-hand object name `C017`
- message area `C015`

Deliverables:

- Overlay diff gate for top row + message area.
- Evidence directory: `parity-evidence/verification/pass165_hud_pixel_overlay/`.

Exit gate:

- HUD source geometry remains matched and original overlay residuals are fixed/classified.

### Lane E — inventory/spell final overlay

Start only after Lane B has `spell_panel` and `inventory_panel`.

Source files:

- `PANEL.C`
- `INVRTZON.C` / `PANEL.C` / inventory-related ReDMCSB source where applicable
- `CASTER.C`
- `SYMBOL.C`
- `OBJECT.C`

Scope:

- inventory panel backdrop and slots
- object icons / held object / G4055 behavior
- spell area, rune cells, spell labels
- food/water/poisoned panel graphics

Deliverables:

- Overlay diff gates for inventory and spell frames.
- Evidence directory: `parity-evidence/verification/pass166_inventory_spell_overlay/`.

Exit gate:

- Slot/icon/rune/panel residuals fixed or explicitly classified.

### Lane F — behavior audit from source, not screenshots

Can run in parallel with C/D/E only when RAM allows.

Source files:

- `MOVESENS.C`
- `TIMELINE.C`
- `CHAMPION.C`
- `GROUP.C`
- `PROJECTI.C`
- `SENSOR.C`
- `MAGIC.C`
- `ENDGAME.C`

Scope:

- click-in-viewport semantics after C080
- wall sensors / floor sensors
- object pickup/drop/throw
- doors and hazards
- pits/teleports/stairs
- combat/action timing
- endgame/restart loop

Deliverables:

- A small matrix of source-backed behavior invariants mapped to existing M10/M11 probes.
- Evidence directory: `parity-evidence/verification/pass167_source_behavior_audit/`.

Exit gate:

- No high-priority gameplay behavior row remains `UNPROVEN` without a named reason.

## Efficient execution model

When RAM gate allows workers:

- 500–1500 MB free: run exactly one worker, Lane A until unblocked.
- 1500–2800 MB free: run two workers: Lane A + one source audit that cannot touch the route.
- >2800 MB free: max five workers, but only these lanes:
  1. Lane A route unblock
  2. Lane B frame quartet
  3. Lane C viewport overlay
  4. Lane D HUD overlay
  5. Lane E inventory/spell overlay

Workers must be sequential within a lane. No duplicate lane workers.

When Mac RAM is below 500 MB:

- Do not spawn subagents.
- Continue only direct N2 source/probe work that is low-risk and bounded.

## Final completion gate

Before calling DM1 V1 complete:

```
cmake --build build -j2
ctest --test-dir build --output-on-failure
python3 tools/verify_v1_redmcsb_source_lock.py
python3 tools/verify_v1_viewport_source_zone_tables.py
# plus final overlay gates from passes 163-166
```

Also required:

- `git diff --check` clean.
- credential/leak scan clean.
- `PARITY_MATRIX_DM1_V1.md` updated so no finished row still says stale `UNPROVEN`/`KNOWN_DIFF`.
- final summary states exactly what is matched, what remains `KNOWN_DIFF`, and what is intentionally deferred.

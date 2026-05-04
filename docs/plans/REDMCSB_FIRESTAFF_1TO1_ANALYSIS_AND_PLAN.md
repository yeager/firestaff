# ReDMCSB ↔ Firestaff 1:1 Analysis and Practical Parity Plan

Last updated: 2026-04-23
Target: DM1 / PC 3.4 / English / V1 original-faithful mode

---

## 1. What this document is

This is a bounded code-and-structure comparison between:

- **Firestaff current HEAD**
- **local ReDMCSB source dump** under `../redmcsb-output/I34E_I34M/`
- existing Firestaff parity notes (`MASTER_1TO1_PARITY_PLAN.md`, `PARITY_MATRIX_DM1_V1.md`, `STATUS.md`, `M11_PLAN.md`, related probes)

This is **not** a claim that Firestaff is near 1:1. The practical conclusion is the opposite:

> Firestaff has a large amount of useful extraction, compat, and verification work, but it still diverges from ReDMCSB/original DM1 in three major ways:
> 1. the runtime path is reconstructed as many narrow seams rather than the original integrated flow,
> 2. the in-game/view/UI layer is still partly custom/presentational rather than source-faithful,
> 3. most gameplay/UI parity claims are still infrastructure-backed rather than original-backed.

If Firestaff and ReDMCSB disagree without strong evidence, assume Firestaff is wrong.

---

## 2. Bounded comparison summary

### 2.1 Code-shape difference

### ReDMCSB

The local ReDMCSB dump is still shaped like the original program families:

- core runtime / orchestration: `DM.C`, `GAMELOOP.C`, `COMMAND.C`, `TIMELINE.C`
- dungeon + world: `DUNGEON.C`, `GROUP.C`, `OBJECT.C`, `MOVESENS.C`, `LOADSAVE.C`
- rendering / presentation: `DUNVIEW.C`, `DRAWVIEW.C`, `PANEL.C`, `CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C`, `MENU.C`, `MENUDRAW.C`, `TEXT*.C`
- graphics / memory pipeline: `MEMORY.C`, `IMAGE*.C`, `BLIT*.C`, `VIDEODRV.C`
- audio / misc: `SOUND.C`, `MUSIC.C`, `STARTUP*.C`, `ENDGAME.C`, `DIALOG.C`

The code is ugly/original, but the **behavioral ownership boundaries are authentic**.

### Firestaff

Firestaff currently has ~399 `.c/.h` files versus ~139 `.C/.H` files in the ReDMCSB dump. The important difference is not just size; it is **fragmentation strategy**.

Firestaff is split into:

- many `memory_*_pc34_compat.*` seams (`memory_graphics_dat_*`, `memory_cache_*`, `memory_load_*`, etc.)
- separate M10 gameplay modules (`memory_movement_pc34_compat.c`, `memory_combat_pc34_compat.c`, `memory_magic_pc34_compat.c`, etc.)
- a much smaller but highly custom M11/M12 shell (`main_loop_m11.c`, `m11_game_view.c`, `render_sdl_m11.c`, `menu_startup_m12.c`, `menu_startup_render_modern_m12.c`)
- a very large probe/test surface

This has been good for extraction and proof, but it means Firestaff currently matches ReDMCSB **more as a library of reconstructed slices** than as a faithful integrated program.

### Practical implication

For 1:1 parity, Firestaff now needs to converge from:

- **“many validated local compat seams + custom game view”**

toward:

- **“one source-faithful runtime path whose visible outputs match ReDMCSB/original”**

without doing repo reorg first.

---

## 3. Major parity gap categories

## A. Functional behavior gaps

### A1. M10 engine coverage is broad, but not yet original-proven enough

Firestaff has real gameplay modules and many verification suites:

- `memory_tick_orchestrator_pc34_compat.c`
- `memory_movement_pc34_compat.c`
- `memory_combat_pc34_compat.c`
- `memory_creature_ai_pc34_compat.c`
- `memory_magic_pc34_compat.c`
- `memory_sensor_execution_pc34_compat.c`
- `memory_savegame_pc34_compat.c`
- `verification-m10/*`

But current evidence still mostly proves:

- internal consistency
- deterministic behavior
- bounded scenario correctness

more than it proves:

- exact DM1/ReDMCSB behavior for awkward edge cases
- exact menu → world → inventory → spell → combat consequence chains
- exact bug-profile match

### A2. Some visible gameplay behavior is still owned by M11 custom logic, not the original runtime path

`m11_game_view.c` currently contains substantial gameplay-adjacent logic that is visibly user-facing:

- quicksave handling
- item pickup/drop helpers
- spell panel input/buffer behavior
- post-move transitions
- pit fall / teleporter handling helpers
- rest/survival drain helpers
- creature AI support/helpers
- message filtering / status text policy
- inventory/pointer handling

Some of this may be acceptable as shell glue, but too much of it is still deciding observable DM1 behavior above the compat engine.

**For 1:1, observable behavior must live in or be directly driven by the source-faithful engine path, not by ad hoc M11 rules.**

### A3. Startup/menu/game-state flow is currently mixed across original-style and Firestaff-owned logic

The M9/M11/M12 path currently spans:

- `memory_graphics_dat_*`
- `boot_*_runtime_pc34_compat.*`
- `main_loop_m11.c`
- `menu_startup_m12.c`
- `menu_startup_render_modern_m12.c`
- `m11_game_view.c`

This means the project has a real boot/menu path, but not yet a cleanly source-faithful DM1 V1 front-end path. In practice:

- menu state logic is partly authentic and partly Firestaff-owned
- V1 and modern menu concerns are interleaved in runtime entry flow
- startup behavior is not yet clearly separated into “must match DM1” vs “modern layer can differ”

---

## B. Engine / runtime parity gaps

### B1. Firestaff mirrors ReDMCSB subsystems, but mostly as seam chains rather than original runtime blocks

The strongest example is the graphics/memory path:

- ReDMCSB: `MEMORY.C`, `IMAGE*.C`, `DIALOG.C`, `ENDGAME.C`, startup/display files
- Firestaff: many small modules such as `memory_graphics_dat_transaction_pc34_compat.c`, `memory_graphics_dat_slots_pc34_compat.c`, `memory_graphics_dat_startup_dispatch_pc34_compat.c`, `memory_cache_*`, `image_backend_pc34_compat.c`, `screen_bitmap_present_pc34_compat.c`

This is useful for proving extraction, but it also creates a parity risk:

- the seam graph can be locally correct while the whole-frame/runtime ownership is still wrong
- side effects and ordering can drift from the original integrated path
- “works under probes” can hide “not actually how ReDMCSB/game loop composes it”

### B2. The current main loop is not yet ReDMCSB-shaped

`main_loop_m11.c` is still a hybrid launcher/game driver that directly mixes:

- SDL event handling
- M12 launcher concerns
- runtime mode switching
- idle tick logic
- direct presentation branching
- direct M11 game-view orchestration

Compared with ReDMCSB’s original ownership split (`STARTUP*`, `MENU*`, `GAMELOOP.C`, `COMMAND.C`, `DM.C`), Firestaff’s top-level loop is still too custom and too cross-layer.

### B3. M11 plan and actual M11 differ materially

`M11_PLAN.md` still describes a larger modular destination (`render_dungeonview_m11`, `render_panels_m11`, `render_sprite_m11`, `input_sdl_m11`, etc.).

Current repo reality is smaller and more concentrated:

- `m11_game_view.c` has absorbed a lot of rendering + UI + interaction logic
- several planned modules are absent or only partially realized
- M12 menu work is already mixed into the running path

This mismatch matters because future parity work must target **current reality**, not the cleaner planned architecture.

### B4. Palette/rendering correctness has improved, but color/presentation integration is still fragile

`PARITY_MATRIX_DM1_V1.md` already documents major palette corrections and remaining render-path issues. The important runtime conclusion is:

- Firestaff now has good evidence for DM PC VGA palette semantics
- but presentation correctness still depends on a relatively thin custom render shell
- and many visible surfaces are not yet proven through end-to-end source-faithful composition

---

## C. Visual / presentation parity gaps

### C1. Viewport and panel geometry are still not source-faithful enough

ReDMCSB/original presentation ownership is spread across:

- `DUNVIEW.C`
- `DRAWVIEW.C`
- `PANEL.C`
- `CHAMPION.C`
- `PORTRAIT.C`
- `SPELDRAW.C`
- `MENU.C`
- `MENUDRAW.C`
- `VIDEODRV.C`

Firestaff’s visible in-game UI is still heavily concentrated in `m11_game_view.c`, with many hard-coded rectangles, colors, text overlays, and simplified panel treatments.

That is enough for playability, but not for 1:1.

### C2. Firestaff still uses custom, text-heavy, convenience-facing rendering in V1-visible surfaces

Observed directly in `m11_game_view.c`:

- custom text renderer and text styles
- custom status lines / inspect readouts
- custom message filtering policy
- custom inventory/action/prompt panel drawing logic
- bespoke viewport rectangle and panel constants

Even where assets are improving, the screen is still often constructed as **“Firestaff’s interpretation of DM”** instead of **“DM’s original component layout rendered through Firestaff.”**

### C3. M12 modern menu work currently leaks into the same executable/runtime path as V1

Files:

- `menu_startup_m12.c`
- `menu_startup_render_modern_m12.c`
- `branding_logo_m12.c`
- `card_art_m12.c`
- `asset_status_m12.c`

These are valid project workstreams, but for strict DM1/V1 parity they are secondary. Right now they still occupy top-level runtime attention and complexity that should not be allowed to distort the V1 parity sequence.

### C4. Screen-level parity is still under-measured

Existing docs already say this, but code comparison reinforces it:

- Firestaff has many screenshots and probes
- but very few final coordinate-locked overlay comparisons against ReDMCSB/original for viewport, panel, portrait, inventory, spell, and text surfaces

So the remaining visual risk is not abstract. It is specific: **the wrong things may simply still be in the wrong places, in the wrong layers, with the wrong assets or text treatment.**

---

## 4. Must-match vs can-wait

## Must match for DM1/V1 1:1

1. **Runtime ownership of observable gameplay**
   - movement, turning, door use, attacks, projectile/spell consequences, item use, pits/teleporters/stairs, death/rest/save-load
2. **Original-faithful front-end composition**
   - viewport size/placement, side panel layout, inventory/spell/action regions, text placement, portraits/icons/runes
3. **Original asset and palette semantics**
   - original graphics where DM1 used graphics, not text substitutes
4. **Original menu/startup behavior where it is part of V1 experience**
5. **Source-backed parity evidence**
   - screenshot overlays, state comparisons, scenario logs, not just passing internal probes

## Can wait until after DM1/V1 lock

1. broad repo reorganization
2. nicer modular cleanup of M11/M12
3. V2/V3 presentation ambitions
4. improved modern launcher UX beyond what is needed not to interfere with V1
5. aesthetic cleanup that does not change parity evidence or runtime correctness

---

## 5. Practical staged path to 1:1 parity

## Stage 1 — Lock the parity target and stop cross-layer drift

### Goal
Make the repo behave as if DM1/V1 is the only blocking target.

### Required work

1. **Freeze a DM1/V1 runtime ledger** inside the new workstream:
   - which visible/runtime paths are authoritative
   - which files are temporary/custom and must be retired or reduced before 1:1 claims

2. **Mark current ownership explicitly** for these paths:
   - startup/menu
   - in-game loop
   - viewport render
   - inventory/spell/action UI
   - message/status surfaces
   - item pickup/drop/use
   - spells
   - environmental transitions
   - save/load

3. **Stop treating M12 modern/startup work as part of parity progress** unless it directly unblocks V1.

### Deliverable
A concrete ownership map and V1 blocker list. This document is the first pass, but the next implementation pass should turn it into checked work items.

---

## Stage 2 — Re-anchor Firestaff’s visible runtime path to ReDMCSB’s program shape

### Goal
Reduce “custom game shell decides behavior” risk.

### Required work

1. **Map Firestaff top-level flow against ReDMCSB file families**

Create a simple matrix like:

- ReDMCSB `GAMELOOP.C` / `COMMAND.C` / `DM.C` → Firestaff runtime entry / command dispatch / world-tick owners
- ReDMCSB `DUNVIEW.C` / `DRAWVIEW.C` → Firestaff viewport owners
- ReDMCSB `PANEL.C` / `CHAMPION.C` / `PORTRAIT.C` / `SPELDRAW.C` → Firestaff side-panel owners
- ReDMCSB `MENU.C` / `MENUDRAW.C` / `STARTUP*.C` → Firestaff launcher/menu owners

2. **Move observable behavior down out of `m11_game_view.c` where needed**

Not by repo reorg, but by parity discipline:

- if logic changes player-visible behavior, make it engine/source-backed
- keep `m11_game_view.c` as renderer/input glue, not a behavior author

3. **Tighten the `main_loop_m11.c` boundary**

Short-term target:
- event pump
- command translation
- invoke source-faithful menu/game routines
- present frame

Not target:
- own gameplay rules
- own presentation semantics
- own V1-vs-modern policy beyond mode routing

### Deliverable
A smaller, more honest top-level control path that resembles ReDMCSB ownership even before any repo cleanup.

---

## Stage 3 — Functional parity workstream

### Goal
Turn M10/M11 correctness from “deterministic and plausible” into “ReDMCSB/original-matching.”

### Priority slices

#### F1. Movement / doors / environmental transitions
Files to compare first:
- ReDMCSB: `DUNGEON.C`, `MOVESENS.C`, `COMMAND.C`
- Firestaff: `memory_movement_pc34_compat.c`, `memory_sensor_execution_pc34_compat.c`, `m11_game_view.c`

Why first:
- movement/door/pit/stairs/teleporter drift is immediately player-visible
- some of this still appears partly M11-owned

#### F2. Inventory / item pickup-drop-use / action hand
Files to compare first:
- ReDMCSB: `PANEL.C`, `OBJECT.C`, `COMMAND.C`, `CHAMPION.C`
- Firestaff: `memory_champion_state_pc34_compat.c`, `memory_champion_lifecycle_pc34_compat.c`, `m11_game_view.c`

Why second:
- current M11 helper ownership risk is high
- inventory behavior and UI parity are tightly coupled

#### F3. Spell entry / spell panel / cast consequences
Files to compare first:
- ReDMCSB: `SPELDRAW.C`, `CASTER.C`, `COMMAND.C`
- Firestaff: `memory_magic_pc34_compat.c`, `m11_game_view.c`

Why third:
- current Firestaff spell flow still looks partly custom above engine level

#### F4. Creature AI / attacks / projectiles / runtime dynamics
Files to compare first:
- ReDMCSB: `GROUP.C`, `DUNGEON.C`, `TIMELINE.C`
- Firestaff: `memory_creature_ai_pc34_compat.c`, `memory_projectile_pc34_compat.c`, `memory_runtime_dynamics_pc34_compat.c`, `m11_game_view.c`

Why fourth:
- current AI/runtime feel can be “good enough playable” while still not 1:1

### Acceptance gate for each functional slice

Do not mark matched until each slice has:

- ReDMCSB/original scenario reference
- Firestaff scenario reproduction
- a specific pass/fail statement in parity docs
- no M11-only workaround left untracked

---

## Stage 4 — Engine/runtime parity workstream

### Goal
Make Firestaff’s runtime path behave like the original integrated machine, not just a set of passing seams.

### Priority slices

#### E1. Graphics/memory/load path convergence
Files/families:
- ReDMCSB: `MEMORY.C`, `IMAGE*.C`, `DIALOG.C`, `ENDGAME.C`, `VIDEODRV.C`
- Firestaff: `memory_graphics_dat_*`, `memory_cache_*`, `memory_load_*`, `image_*_pc34_compat.*`, `screen_bitmap_present_*`

Required outcome:
- document the exact end-to-end path used for V1 frames
- identify seams that are still probe-oriented rather than true runtime owners
- remove duplicate or parallel visible paths where they create ambiguity

#### E2. Top-level game-loop alignment
Files/families:
- ReDMCSB: `GAMELOOP.C`, `COMMAND.C`, `DM.C`, `STARTUP1.C`, `STARTUP2.C`, `MENU.C`
- Firestaff: `main_loop_m11.c`, `m11_game_view.c`, `menu_startup_m12.c`, `memory_tick_orchestrator_pc34_compat.c`, `boot_*_runtime_pc34_compat.*`

Required outcome:
- one clear V1 boot → menu → game → save/load → quit route
- fewer custom decision points outside compat/runtime path

#### E3. Bug-profile ledger
Required outcome:
- explicit list of DM1 target quirks/bugs that must be preserved
- no silent “cleanups” in V1

---

## Stage 5 — Visual parity workstream

### Goal
Make the screen match DM1/ReDMCSB, not just feel similar.

### Priority slices

#### V1. Viewport rectangle, horizon, wall/floor/ceiling composition
Compare:
- ReDMCSB `DUNVIEW.C`, `DRAWVIEW.C`, `VIDEODRV.C`
- Firestaff `m11_game_view.c`, render path helpers, bitmap/palette path

Required output:
- measured coordinate table
- overlay images
- list of remaining mismatches by depth/lateral cell class

#### V2. Side panel / portraits / action area / inventory / spell panel
Compare:
- ReDMCSB `PANEL.C`, `CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C`
- Firestaff `m11_game_view.c`, font/asset/render helpers

Required output:
- component rectangle table
- asset usage map
- text-vs-graphics mismatch list

#### V3. Title/menu/startup visuals
Compare:
- ReDMCSB `MENU.C`, `MENUDRAW.C`, `STARTUP*.C`, title assets from graphics path
- Firestaff `menu_startup_m12.c`, `menu_startup_render_modern_m12.c`, boot/menu runtime path

Required output:
- explicit split between V1-faithful startup/menu and modern menu path
- no parity claims based on modernized menu visuals

#### V4. Typography and player-facing text
Compare:
- ReDMCSB `TEXT.C`, `TEXT2.C`, UI panel/string owners
- Firestaff text render + message/status surfaces in `m11_game_view.c`

Required output:
- remove/track all helper-ish V1 text
- ensure original graphics are used where original graphics existed

---

## 6. Concrete milestone breakdown

## Milestone P0 — Baseline truth pass

**Output:** one updated parity ledger row set for top-level runtime ownership.

Tasks:
- map ReDMCSB file families to Firestaff owners
- annotate which current Firestaff owners are temporary/custom
- list top 10 V1 blockers

## Milestone P1 — Functional core parity

**Output:** movement/doors/env transitions, inventory/item flow, spell flow audited against ReDMCSB.

Tasks:
- compare and correct movement/door/pit/stairs/teleporter ownership
- compare and correct pickup/drop/use ownership
- compare and correct rune/cast flow ownership
- add original-backed scenarios to affected verification suites

## Milestone P2 — Runtime path parity

**Output:** one clear V1 runtime path from boot/menu into in-game loop with reduced custom shell logic.

Tasks:
- narrow `main_loop_m11.c`
- reduce `m11_game_view.c` behavioral ownership
- document exact frame-production/runtime chain
- remove ambiguity between V1 and modern/menu runtime paths

## Milestone P3 — Visual parity lock

**Output:** coordinate-backed viewport + side-panel + startup/menu comparison pack.

Tasks:
- viewport measurements and overlays
- panel measurements and overlays
- typography/asset gap audit
- fix remaining text-heavy/custom V1 surfaces

## Milestone P4 — Honesty lock

**Output:** updated matrix with only source-backed match claims.

Tasks:
- downgrade any vague parity claims
- attach evidence paths to all “MATCHED” rows
- leave unresolved items as `KNOWN_DIFF` / `UNPROVEN`

---

## 7. Suggested next slices for future GPT/Codex passes

### Best next slice 1 — movement/door/environment ownership audit

Why:
- bounded
- high functional value
- likely to expose where M11 still owns DM1 behavior

Specific compare targets:
- `memory_movement_pc34_compat.c`
- `memory_sensor_execution_pc34_compat.c`
- `m11_game_view.c`
- ReDMCSB `DUNGEON.C`, `MOVESENS.C`, `COMMAND.C`

### Best next slice 2 — spell panel / cast path audit

Why:
- current Firestaff spell flow is visibly custom-heavy
- easy to drift from original UI + behavior at the same time

Specific compare targets:
- `memory_magic_pc34_compat.c`
- `m11_game_view.c`
- ReDMCSB `SPELDRAW.C`, `CASTER.C`, `COMMAND.C`

### Best next slice 3 — side-panel geometry and text audit

Why:
- visually obvious
- practical screenshots/overlay output
- likely to remove fake “close enough” comfort

Specific compare targets:
- `m11_game_view.c`
- font/asset loaders
- ReDMCSB `PANEL.C`, `CHAMPION.C`, `PORTRAIT.C`, `SPELDRAW.C`

---

## 8. Short appendix/checklist for future passes

### Red flags: if any of these are true, parity is still not there

- a visible gameplay outcome depends on `m11_game_view.c` convenience logic rather than source-faithful engine flow
- a V1 screen is still mostly hard-coded boxes/text instead of original component composition
- a “matched” claim has no original/ReDMCSB evidence attached
- modern M12 menu work is being counted as V1 parity progress
- passing probes are being used as a substitute for original comparison

### Quick checklist before calling any area “1:1”

- [ ] ReDMCSB file owner(s) identified
- [ ] Firestaff file owner(s) identified
- [ ] observable behavior path documented
- [ ] screenshot/state/log evidence captured
- [ ] custom M11 fallback logic either removed or explicitly tracked
- [ ] parity matrix updated honestly

---

## 9. Bottom line

Firestaff is not blocked by lack of effort or lack of low-level work. It is blocked by **the last mile from reconstructed seams to source-faithful integrated behavior and presentation**.

The biggest practical gap areas are:

1. **too much observable behavior and presentation still concentrated in custom M11 code**, especially `m11_game_view.c` and the current top-level loop
2. **runtime ownership still does not resemble ReDMCSB closely enough at the whole-program level**
3. **visual parity is still under-measured and still partly text/custom-layout driven**

So the next right strategy is not broad new feature work.
It is:

- re-anchor ownership to ReDMCSB,
- push behavior down into the faithful path,
- measure the screen rigorously,
- and only then claim parity where evidence exists.

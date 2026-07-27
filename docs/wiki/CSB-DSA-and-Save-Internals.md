# CSB DSA and Save Internals

## Reference Model

CSB combines ReDMCSB dungeon behavior with CSBWin-specific extended-save and
DSA behavior. Firestaff treats a loaded DSA as authenticated source data, not
as a scripting convenience API.

## Extended Save Admission

The extension loader stages GAMEBLOCK2, CHARDESC, ITEM16, timers, game-info,
DSA action records, level-index data, and optional trace bitmap before a live
profile is changed. Failure in a later record invalidates the candidate rather
than preserving an earlier partial import.

The DSA record identity is the source tuple:

```text
(absolute_dsa_id, state, ordinal, source_action_pointer)
```

The pointer identity is retained at runtime. A host-created or copied action
cannot be passed to an authenticated runner merely because its words match.

## Actuator Binding

A type-47 DB3 actuator does not name a DSA directly. Its selector bits are
resolved through the imported source level table:

```text
selector = (DB3.word2 >> 7) & 0x1f
dsa_id   = DSALevelIndex[current_level][selector]
```

The binding rejects a wrong actuator type, a missing index, an undefined ID,
or an action absent from the staged extension. It never invents a default DSA
state or selector fallback.

## Supported DSA Execution

The admitted subset uses a scratch stack and candidate output state. It covers
source-shaped LOAD/STORE, local `DSAVARS`, owned globals, and pure stack
arithmetic/control forms. Parameters and globals publish together only after a
complete action is consumed.

`JUMP` and `GOSUB` use CSBWin `Execute` selection rules:

* state/column lookup chooses the first exact file-order action;
* JUMP transfers within the bounded execution frame;
* GOSUB records a one-frame nested transfer and preserves outer continuation;
* missing targets end the selection without a synthetic action;
* depth and transfer ceilings reject before publication.

World-mutating AMPERSAND forms, unsupported loads, malformed words, out of bank
variables/globals, and unowned pointers reject. A rejection leaves caller
parameters, global data, filter receipt, and dungeon state unchanged.

## Movement and Chain Semantics

CSB movement shares the M10 F0267 loaded-chain primitive. F0276 sensor results
are resolved first, then forwarded to F0272/F0268-style timed square-state
effects. Remote doors, pits, teleporters, and fakewalls cannot mutate directly
from an unbounded callback during chain movement.

## Raster Startup Contract

The startup sequence is an indexed-raster session, not a textual menu state.
PRESENTS, CHAOS, STRIKES BACK, C004 entrance, closed/opening doors, and HUD are
separate palette/geometry receipts. C001 has distinct palette phases; a title
phase cannot reuse DM1 art or an arbitrary palette index.

## Verification

```bash
./build/test_csb_v1_phase7_verification
./build/test_csb_v1_dsa_trigger_single_step_pc34_compat
./build/test_csb_v1_f0267_loaded_chain_pc34_compat
```

The remaining broad interpreter and EXPOOL classes must be implemented from
CSBWin evidence. They must not be filled in with a generic VM.

## Real Package DSA Receipt

`firestaff_csb_v1_csbwin_extended_dsa_handoff_probe` accepts only an original
CSBWin `Dungeon.dat` and an original extended `csbgame*.dat` save. It verifies
the production resume path publishes authenticated DSA actions and the saved
level-index table into the same runtime owner, then scans decoded source Thing
chains for a type-47 actuator whose selector resolves through that restored
table to an authenticated action. It also requires a non-empty source TIMER
heap and proves each serialized queue slot retains one exact live timeline
receipt (time, function, priority, coordinates, cell, and effect) before the
DSA owner is checked through one tick. This is queue correlation, not a claim
that any saved timer dispatched or executed a DSA action; unsupported timer
functions and DSA bytecode still fail closed at their existing boundaries. It
does not generate a save, DSA record, selector, actuator, timer, or fallback
action. Without both explicit paths (or `FIRESTAFF_CSBWIN_DUNGEON` and
`FIRESTAFF_CSBWIN_SAVE`), it skips.

## Admitted/Restored Timer Bridge

`csb_v1_dsa_admitted_restored_timer_bridge` covers the handoff between an
admitted (freshly executing) DSA timer and a restored (loaded-from-save)
timer: both paths must resolve to the same source timer identity and queue
slot before the bridge publishes state. This is tested alongside
`csb_v1_dsa_parameter_message_save_handoff`, which restores
`TT_ParameterMessage` handoff for function 101 across a save/resume boundary.

## Combat Helper Opcodes

Two DSA opcodes are tested as combat integration points rather than pure
stack primitives:

* **CausePoison** — poison application through the DSA action pipeline.
* **CountInjury** — injury/wound tally fetch used by combat and HUD damage
  paths.

Both are covered by the 12 DSA test files (9255 lines, 117 unique operations)
that make up Q-CSB-01, and both participate in the damage-character filter
used by Q-CSB-08 combat tests (Grey Lord combat, projectile speed, F0247
teleporter impact/retention, F0266 group move projectile receipt).

## Save Test Coverage (32 files)

Q-CSB-09 save/Utility Disk interop is covered by 32 test files spanning:

* save header build and read;
* native F0435 provenance (recorded only after a committed import, never on
  a rejected candidate);
* export/import round trips;
* the CSBWin save loader boundary (GAMEBLOCK1/body import rejects malformed
  non-empty DB11/EXPOOL tails before atomic runtime staging);
* the utility save transaction path (Utility Disk import, edit, inventory,
  dialogs, confirmations).

15 of the 32 files are executable test binaries; the remainder are shared
fixture/header support. All pass.

## Viewport Test Coverage (47 files)

Q-CSB-06 dungeon viewport geometry is covered by 47 viewport tests:

* walls D0-D3, all four sides;
* doors, including partly-open doors;
* floor and ceiling ornaments;
* pits, stairs, and teleporters;
* center fields, custom backgrounds (11 variants), footprints, door frames;
* projectile routing and metadata, item explosions.

F0115 first-object native graphics use the G0209 weapon[46]/armour[58]/
junk[52]/potion[21] tables with C10 blit (conditional horizontal flip).
Creature groups use per-creature transparency (G0219
`coordinateSet_transparentColor`) and D2/D3 palette remap tables
(G0221/G0222). Item/explosion composition accepts only a hash-verified
decoded `CSBGRAPHICS.DAT` surface and its source palette (C10 transparency);
source-bound object drawers suppress the older icon/marker fallback when
their real surface is unavailable.

## Timer Queue Restart Boundary

For a resumed CSBWin save, `TIMER` and `TimerQueue` remain source-owned data.
Core export retains their original array indexes, heap topology, sequence words,
and GAMEBLOCK2 timer counters only when every live timeline entry still has one
exact saved queue-slot receipt. A fired, replaced, duplicated, or unmapped event
breaks that receipt and causes CSBWin core export to reject rather than emitting
a reconstructed queue that could look valid while changing restart behavior.

At dispatch, a materialized CSBWin queue slot remains CSBWin-owned even if its
live timer receipt no longer validates. Firestaff consumes that event instead of
letting a numeric timer-function alias fall through to a generic M10 handler.
Only a complete source receipt can authorize a CSBWin timer mutation.

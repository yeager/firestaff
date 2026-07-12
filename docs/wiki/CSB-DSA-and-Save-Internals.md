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

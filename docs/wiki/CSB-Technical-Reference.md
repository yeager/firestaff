# Chaos Strikes Back (CSB) Technical Reference

## Scope

CSB uses ReDMCSB as its main reference and CSBWin for CSB-specific save,
DSA, champion, and input behavior. Launch needs hash-verified CSB graphics and
dungeon data; missing title, entrance, door, or HUD art is not replaced.

## Startup and Dungeon

PRESENTS, CHAOS, STRIKES BACK, entrance, door opening, and HUD handoff are
typed indexed-raster phases with independent palette/timing contracts. They
are not recolored DM1 assets. M12 passes a verified package session to M11.

The M10 F0267 route owns live loaded Thing chains, including cross-level pit
and teleporter movement. It preserves source-tail ordering and restores the
prior current-level context after a transfer.

## CSBWin DSA Boundary

Extended-save actions are checksum-authenticated and retain source `(dsa,
state, ordinal)` identity. Supported pure stack execution is transactional:
LOAD/STORE, local/global banks, and bounded JUMP/GOSUB transfer chains use the
first source-file-order `(state,column)` match. Type-47 `DSAselector` resolves
through authenticated `DSALevelIndex[level][selector]` data.

Forged actions, malformed extensions, unknown IDs, transfer/depth limits, and
world-mutating opcodes reject before state publication. This is deliberately
not a claim that every CSBWin opcode, ProcessDSAFilter path, or EXPOOL class is
implemented.

## Reference Limits

ReDMCSB and CSBWin answer different questions. ReDMCSB is the primary source
for original CSB engine behavior, original EVENT/timeline structure, media
branches, and original save-header contracts. It is not evidence for CSBWin
extensions.

| Area | Primary evidence | What ReDMCSB cannot prove |
|---|---|---|
| Original timers and dungeon mutation | ReDMCSB `TIMELINE.C`, `DUNGEON.C`, `GROUP.C` | CSBWin TIMER queue ownership and `timerObj6/8` semantics |
| CSBWin DSA/custom dungeons | CSBWin `DSA.cpp`, `data.cpp`, `SaveGame.cpp` | EXPOOL, type-47 selector tables, opcode behavior, or DSA state |
| Save interoperability | ReDMCSB `DEFS.H`, `LOADSAVE.C` plus per-media corpus | GAMEBLOCK2, ITEM16, extended tails, EXPOOL, and CSBWin continuation bytes |
| Mouse, audio, interrupt timing | target-media capture; CSBWin host sources for CSBWin paths | vector-dispatched `USIOSTUB.C`, `MUSCSTUB.C`, and interrupt timing as portable behavior |
| Title/entrance/HUD pixels | hash-identified original assets and capture | a universal palette, cadence, or renderer across `MEDIA*` branches |

The source archive contains platform dispatch and assembly segments rather
than a universal host specification. For example, `USIOSTUB.C` forwards mouse
and queued-input calls through library vectors, `MUSCSTUB.C` forwards music
calls, `VBLANK.C` installs interrupt handlers, and `GRAPH21.C` carries
media-specific CPSE/fuzzy-sector logic. These establish that a path exists;
they do not establish equivalent SDL ordering, sampled input, audio mixing,
or copy-protection results.

For any claimed original save format, preserve the evidence tuple:

```text
(media/version hash, original save bytes, parser receipt, runtime capture)
```

For any CSBWin DSA/save claim, preserve the additional tuple:

```text
(CSBWin save hash, timer queue slot, source timer index,
 DSA/EXPOOL record identity, selected source action)
```

Absent those tuples Firestaff must fail closed or state that the route is not
yet verified. The detailed work queue is `REDMCSB-CSB-GAP-001` through
`REDMCSB-CSB-GAP-013` in `TODO.md`. The additional boundaries make clear that
the ReDMCSB rebuild is not a PC-binary oracle, its source does not define a
complete PC boot-media contract, and historical bug notes require an explicit
Firestaff policy rather than automatic reproduction.

## Save and HUD Ownership

Save imports stage GAMEBLOCK, champions, items, timers, extensions, and trace
data before committing. The CSB raster boundary owns title, entrance, door, and
HUD palettes; unresolved skin/EXPOOL data cannot become substitute pixels.

## Verification

```bash
cmake --build build --target test_csb_v1_phase7_verification \
  test_csb_v1_dsa_trigger_single_step_pc34_compat --parallel
./build/test_csb_v1_phase7_verification
./build/test_csb_v1_dsa_trigger_single_step_pc34_compat
```

For authenticated DSA, actuator, save, and raster contracts, see [CSB DSA and
Save Internals](CSB-DSA-and-Save-Internals).

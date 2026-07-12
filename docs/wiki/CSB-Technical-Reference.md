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

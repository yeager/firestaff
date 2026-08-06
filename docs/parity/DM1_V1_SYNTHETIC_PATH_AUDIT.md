# DM1 V1 Synthetic-Path Audit

Date: 2026-08-06

## Scope

This audit covers the production DM1 V1 path used by the `firestaff` target,
not isolated contract probes or retired compatibility targets. The reference
is ReDMCSB's PC 3.4 path, especially `OBJECT.C`, `DUNVIEW.C`, `PANEL.C`, and
`DEFS.H`, with the local hash-verified DM1 PC 3.4 data set as the asset source.

## Production findings

| Area | Current source | Synthetic replacement found? | Evidence |
| --- | --- | --- | --- |
| Object names | `GRAPHICS.DAT` M564, resolved through the source icon index | No | `test_m11_dm1_real_object_corpus`, 611 records |
| Object icons and charge states | `DUNGEON.DAT` Thing bytes plus the PC 3.4 G0237 object table | No | `test_m11_dm1_real_object_corpus` |
| Floor and alcove objects | F0115 candidates, live Thing records, decoded GRAPHICS.DAT surfaces | No active fallback | `DM1-HOC-OBJECTS-001/002` remain capture-only |
| Wall ornaments, doors, stairs and inscriptions | F0107-F0115 source material receipts and decoded GRAPHICS.DAT pixels | No active fallback in authenticated M11 | Missing material is no-draw |
| Champion mirrors | C127 sensor data, C026 portrait atlas and source wall-cell geometry | No | Real HoC orientation and portrait probes |
| Held-object cursor | F0702 object icon pixels from the active source asset | No | `test_m11_dm1_real_object_names` |
| Legacy generic viewport | `src/engine/firestaff_viewport_renderer.c` | Not linked into the `firestaff` M11 target | Ninja target command audit |

## Non-production synthetic code

The repository still contains synthetic fixtures and compatibility helpers.
They are retained for deterministic contract tests, negative gates and
unsupported-data diagnostics. They must not be used as evidence of DM1 V1
asset support. The subtype name arrays in `m11_game_view.c` are likewise only
for non-authenticated compatibility sessions; authenticated DM1 resolves
names from M564 and fails closed when that table is unavailable.

The legacy generic game loop can render CSB through its separate compatibility
bridge, but it is not linked into the `firestaff` M11 executable. It must not
be revived as a DM1 visual fallback.

## Remaining real-data work

1. Capture the packaged macOS application showing the corrected HoC wall
   torch/holder, stairs, doors, inscriptions, objects and held-object cursor.
2. Add operator-owned PC34 saves containing real C13 event records and prove
   import, live adoption, write-back and original-engine round-trip.
3. Promote broader original DOS/macOS paired captures for F0344/F0345,
   wall/inscription/viewport depth and complete HoC presentation.

These are capture or corpus gaps, not reasons to reintroduce generated art,
host-font text, subtype-name guesses or procedural wall geometry.

## Verification

Against the local real DM1 data:

```text
test_m11_dm1_real_object_corpus  PASS (611 records)
test_m11_dm1_real_object_names   PASS (M564 name, F0702 cursor, pickup/drop)
dm1_v1_original_save_pc34_external_corpus       PASS
dm1_v1_original_save_pc34_backed_corpus_roundtrip PASS
```

The save tests use the two operator-supplied original PC34 saves documented
in `TODO.md` and `DONE.md`. No generated game-data bytes are used.

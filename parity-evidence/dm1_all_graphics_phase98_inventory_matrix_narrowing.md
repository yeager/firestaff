# DM1 all-graphics phase 98 — inventory screen matrix narrowing

## Problem

`PARITY_MATRIX_DM1_V1.md` still described the inventory screen as simply:

```text
Firestaff has inventory screen ... UNPROVEN
```

That was stale after phases 91, 93, and 95. We still do **not** have a full original inventory overlay, but the row should reflect the source-backed slot-icon work now proven by probes and capture smoke.

## Change

Updated the inventory-screen row to distinguish what is now proven from what remains open:

Proven/narrowed:

- inventory panel exists and deterministic capture fixture emits `06_ingame_inventory_panel_latest`
- inventory slot object drawing is source-bound through `OBJECT.C:F0038_OBJECT_DrawIconInSlotBox`
- object icons come from graphics `42..48`
- slot boxes use identified 18×18 graphics `C033`/`C034`/`C035`
- inventory preserves source colour 12 and does **not** apply the action-area `G0498` cyan remap

Still open:

- full inventory layout / panel composition is not overlaid against original runtime evidence

Status is now `KNOWN_DIFF (narrowed)`, not generic `UNPROVEN`.

## Gate

Documentation-only pass, backed by the previous gates:

```text
phase 95 capture smoke PASS
phase 95/96: 418/418 invariants passed
phase 95/96: ctest 5/5 PASS
```

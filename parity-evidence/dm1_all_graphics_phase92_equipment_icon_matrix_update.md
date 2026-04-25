# DM1 all-graphics phase 92 — equipment/item icon parity matrix update

## Problem

`PARITY_MATRIX_DM1_V1.md` still listed equipment/item icons as `UNPROVEN` with "Unknown mapping status", even after phases 84–91 source-bound the object-icon resolver and wired it into action-hand and inventory slot rendering.

## Change

Updated the matrix row to record the current evidenced state:

- source anchor: `OBJECT.C` `F0033_OBJECT_GetIconIndex` and `F0038_OBJECT_DrawIconInSlotBox`
- object-icon atlas: graphics `42..48`, 32 icons per graphic, 16×16
- covered dynamic variants:
  - empty hand `201`
  - lit torch `G0029` charge bucket
  - charged weapons `+1`
  - closed scroll
  - compass direction
  - charged water / Jewel Symal / Illumulet
- action-cell palette distinction: `G0498` remap `12 -> C04 cyan`
- inventory slot distinction: direct source icon blit without the action-area remap

Status is now `MATCHED` for icon selection / atlas extraction / palette distinction in current action + inventory slot surfaces, while explicitly keeping pointer/held-object surfaces and original screenshot overlay as unproven follow-up work.

## Gate

No code changed in this pass. The matrix update is backed by the previous source/probe gates:

```text
phase 91: 418/418 invariants passed
phase 91: ctest 5/5 PASS
```

# Pass 111 — V1 inventory slot namespace audit

Scope: DM1/V1 inventory lane, specifically inventory panel slot boxes and item-icon evidence. This pass does **not** claim original-runtime pixel parity; it tightens the Firestaff/source side before the next original overlay route pass.

## References checked

- Worker VM original archive data:
  - `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT`
  - `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT`
- Greatstone atlas index: `~/.openclaw/data/firestaff-greatstone-atlas/index/keyword_hits.json`
- ReDMCSB reference directory: `~/.openclaw/data/firestaff-redmcsb-source/`

Checksums verified by the pass-111 tool:

```text
2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e  GRAPHICS.DAT
d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85  DUNGEON.DAT
```

Note: the N2 `firestaff-redmcsb-source` pack is present but currently contains the documentation/locator pack rather than extracted `INVNTORY.C`/`OBJECT.C` C source. This was recorded as an evidence detail, not a blocker for this bounded audit, because the audited Firestaff code already carries the source-derived C507..C536 / F0038 comments and exported helpers.

## Result

`tools/pass111_v1_inventory_slot_namespace_audit.py` passed all checks and emitted machine-readable evidence:

- `parity-evidence/overlays/pass111_v1_inventory_slot_namespace_audit.json`

Verified points:

1. `kV1InventorySourceSlotBoxZones` exposes the full 30-entry source namespace `C507..C536`.
2. Equipment helper covers only `C507..C519`.
3. Backpack helper covers `C520..C536`, including the first backpack slot plus the 8×2 carried-object rows.
4. Inventory source slot boxes resolve to the C033 normal slot-box bitmap.
5. Bidirectional source-slot/champion-slot helpers are exported for probes.
6. Inventory icons remain on the direct `F0038_OBJECT_DrawIconInSlotBox` path, distinct from the action-area `G0498` palette remap path.
7. Greatstone item/object reference index is present on N2.
8. Original DM PC 3.4 `GRAPHICS.DAT` and `DUNGEON.DAT` checksums match the worker reference.

## Command run

```sh
cd ~/work/firestaff
./tools/pass111_v1_inventory_slot_namespace_audit.py
```

Output summary: 10 PASS / 0 FAIL.

## Boundary

This pass does not retire the inventory-screen row. It narrows the Firestaff/source side to slot namespace + icon-path evidence. Remaining work is still a semantically identical original inventory route and side-by-side overlay comparison for the panel layout/pixels.

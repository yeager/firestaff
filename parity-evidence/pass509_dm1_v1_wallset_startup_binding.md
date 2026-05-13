# Pass509 - DM1 V1 wall-set startup binding

Status: passed

## ReDMCSB anchors
- STARTUP2.C:625-639 - PC34 G2107_WallSet[C00..C14] startup allocation.
- STARTUP2.C:982-996 - flipped wall-set startup allocation uses the same GRAPHICS.DAT slots.
- DEFS.H:2351-2373,2428 - PC34 wall-set base/count and GRAPHICS.DAT indices 93..107.

## Variant anchors
- GRAPHICS.DAT - sha256 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e, bytes 363417
- DUNGEON.DAT - sha256 d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85, bytes 33357
- Greatstone manifest: DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.md (Result: PASS, PC34 GRAPHICS.DAT mismatches 0).

No original-runtime or pixel parity claim is made by this gate.

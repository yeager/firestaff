# pass764 DM1 V1 Chest Partial Drop To Floor While Chest Open

Status: pending local verification.

This pass adds a narrow DM1 V1 runtime regression for branch B: a visible
C30+/C537..C544 chest stack is partially split while the chest remains open.
The split amount enters the leader-hand route, is deterministically linked to a
synthetic floor cell, and the still-open chest keeps the remaining stack count
in its visible G0425 slot. Closing the chest then proves F0334 compacts the
remaining visible chain, not the dropped partial amount.

## Checked Files

- `include/firestaff/dm1/v1/chest/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.h`
- `src/dm1/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.c`
- `tests/test_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.c`
- `tools/verify_pass764_dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.py`
- `CMakeLists.txt`

## ReDMCSB Chain

- `CHEST.C F0333:30-67`: open chest guard, G0426 assignment, and visible G0425/C537..C544 materialization.
- `CHEST.C F0334:113-132`: close clears G0426 and compacts non-empty visible G0425 entries back into the container chain.
- `CHAMPION.C F0297:243-268`: leader-hand object placement and M516 load update route.
- `CHAMPION.C F0298:270-298`: leader-hand removal before floor insertion.
- `CHAMPION.C F0300:511-515`: C30+ removal from G0425.
- `CHAMPION.C F0301:606-614`: C30+ insertion back through G0425.
- `CHAMPION.C F0302:662-710`: chest slot click dispatch and leader-hand/slot update order.
- `COMMAND.C F0359:1973-1983` plus `F0359:1985-1990`: chest panel slot command dispatch and adjacent M568/C040 dispatch boundary.
- `OBJECT.C F0032:121-145` and `F0033:147-212`: object type/icon identity for redraw evidence.
- `BLITMASK.C F0133:30-33`: partial-mask bitmap draw boundary for the changed stack icon.
- `DUNGEON.C F0163:1796-1837`: floor-square link when MapX is non-negative.
- `DEFS.H:810-816`, `DEFS.H:2088`, `DEFS.H:3906-3913`: C30/M070, C10, and C537..C544 constants; evidence strings also name G0425, G0426, and M516.

## Expected Runtime Proof

- Open chest thing: `0x7640`.
- Source slot: C31 / zone C538.
- Initial visible stack: type `0x7641`, count `9`, unit weight `2`.
- Partial drop: count `3`, weight `6`.
- Remaining visible stack: count `6`, weight `12`.
- Deterministic landing: floor cell `(14,22)`, leader hand empty after F0298-style removal.
- Expected deterministic hash: `0xe50d0bc4`.

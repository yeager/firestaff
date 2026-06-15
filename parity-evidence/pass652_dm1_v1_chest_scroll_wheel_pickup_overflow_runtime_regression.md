# pass652 DM1 V1 Chest Scroll-Wheel Pickup Overflow Runtime Regression

Status: PASS

This pass adds a synthetic runtime regression gate for the DM1 V1 chest scroll-wheel path where both the leader hand and C544 are occupied. The source-locked route is `ROUTE_C544_REPLACEMENT`: F0302 swaps the occupied C544 item into the leader hand and redirects the old leader-hand item back into C544, then F0334 compacts only non-empty cells on close.

## Checked Files
- cmake: `CMakeLists.txt`
- header: `src/dm1/dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.h`
- module: `src/dm1/dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.c`
- test: `tests/test_dm1_v1_chest_scroll_wheel_pickup_overflow_pc34_compat.c`

## ReDMCSB Chain
- CHEST.C F0333:30-67 open materialization into C537..C544.
- CHEST.C F0334:113-132 close rewrite skipping `C0xFFFF_THING_NONE`.
- CHAMPION.C F0297/F0298/F0302 leader-hand and occupied chest-slot swap.
- PANEL.C, COMMAND.C, MOUSE.C, OBJECT.C, BLITMASK.C, and DEFS.H anchor strings are present in the runtime gate.

## Result
- Failures: 0
- Manifest: `parity-evidence/verification/pass652_dm1_v1_chest_scroll_wheel_pickup_overflow_runtime_regression/manifest.json`

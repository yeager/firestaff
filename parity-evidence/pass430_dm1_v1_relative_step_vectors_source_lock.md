# Pass430 — DM1 V1 relative step vectors source lock

Status: `PASS430_RELATIVE_STEP_VECTORS_SOURCE_LOCKED`

## ReDMCSB anchors
- `CLIKMENU.C:G0465_forward_counts` — `{'lines': [224, 228], 'values': [1, 0, -1, 0]}`
- `CLIKMENU.C:G0466_right_counts` — `{'lines': [229, 233], 'values': [0, 1, 0, -1]}`
- `CLIKMENU.C:F0366_command_index_to_F0150` — `[256, 269]`
- `DUNGEON.C:F0150_relative_coordinate_mutation` — `[1371, 1390, 1391]`

## Firestaff seams
- `dm1_v1_movement_pc34_compat.c` — static step_forward/right tables and cardinal dx/dy tables mirror ReDMCSB F0366/F0150 arithmetic
- `dm1_v1_movement_command_core_pc34_compat.c` — dequeued C003..C006 commands are converted to the same zero-based movement-arrow index before movement resolution
- `test_dm1_v1_movement_core_pc34_compat.c` — covers forward/back/strafe vectors for north and forward for east

Manifest: `parity-evidence/verification/pass430_dm1_v1_relative_step_vectors_source_lock/manifest.json`

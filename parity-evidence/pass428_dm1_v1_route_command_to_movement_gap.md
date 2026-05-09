# Pass428 — DM1 V1 route command → movement gap

Status: `PASS428_ROUTE_COMMAND_TO_MOVEMENT_GAP_CLOSED`

## ReDMCSB anchors
- `IO2.C:arrow_normalization` — `[47, 61]`
- `COMMAND.C:movement_keyboard_table` — `[677, 684]`
- `COMMAND.C:F0359_COMMAND_ProcessClick_CPSC` — `[1452, 1662]`
- `COMMAND.C:F0360_COMMAND_ProcessPendingClick` — `[1692, 1707]`
- `COMMAND.C:F0361_COMMAND_ProcessKeyPress` — `[1709, 1813]`
- `COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC` — `[2045, 2156]`
- `CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty` — `[142, 174]`
- `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty` — `[180, 346]`
- `DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` — `[1371, 1391]`
- `CHAMPION.C:F0310_CHAMPION_GetMovementTicks` — `[1180, 1215]`
- `MOVESENS.C:F0267_party_accepted_side_effects` — `[738, 783]`
- `GAMELOOP.C:cooldown_decrement_before_input_wait` — `[150, 155]`

## Firestaff seams
- `dm1_v1_input_command_queue_pc34_compat.c` — maps PC-34 key/route events to C001..C006, retains movement commands while cooldown-gated, dequeues only after gate clears, and discards after blocked step
- `dm1_v1_movement_command_core_pc34_compat.c` — dispatches dequeued turns to F0365-equivalent direction/sensor effects and steps to F0366-equivalent stamina, collision, group, timing, redraw effects
- `dm1_v1_movement_pipeline_pc34_compat.c` — writes cooldown/last-movement timing only after accepted step; M11 bridge decrements old cooldowns before F0380-compatible processing
- `tests` — ['test_dm1_v1_movement_command_core_pc34_compat', 'test_dm1_v1_command_movement_sensor_timing_pc34_compat', 'test_dm1_v1_movement_pipeline_pc34_compat']

Manifest: `parity-evidence/verification/pass428_dm1_v1_route_command_to_movement_gap/manifest.json`

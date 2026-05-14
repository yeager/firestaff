# Pass545 - DM1 V1 movement queue and sensor consequences

Status: PASS545_DM1_V1_MOVEMENT_QUEUE_SENSOR_CONSEQUENCES_LOCKED

## ReDMCSB Evidence
- COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC: COMMAND.C:2045-2829
- COMMAND.C:F0357_COMMAND_DiscardAllInput: COMMAND.C:1305-1378
- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty: CLIKMENU.C:180-351
- MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE: MOVESENS.C:316-910
- MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval: MOVESENS.C:1553-1794
- queueRetainPopDispatchSpan: COMMAND.C:2084-2155
- blockedDiscardSuccessBoundarySpan: CLIKMENU.C:293-345
- discardReservedPendingSpan: COMMAND.C:1327-1375
- moveSensorLeaveEnterSpan: MOVESENS.C:801-818
- sensorRotationDeferSpan: MOVESENS.C:1785-1793

## Closed Gap
successful movement dequeues one command, retains later queued input, runs destination sensor consequences, then applies cooldown; blocked movement dequeues the attempted move, discards nonreserved queued input, runs no movement sensor consequences, and does not assign successful-step cooldown.

## Local Gate
- CTest: pass545_dm1_v1_movement_queue_sensor_consequences
- Runtime: test_dm1_v1_movement_command_core_pc34_compat

## Not Claimed
- viewport wall drawing changes
- new DOS runtime capture
- full mutation of deferred sensor-list rotation beyond existing pass510 metadata

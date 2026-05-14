# Pass510 - DM1 V1 movement sensor rotation defer source lock

Status: PASS510_DM1_V1_MOVEMENT_SENSOR_ROTATION_DEFER_SOURCE_LOCKED

## ReDMCSB Evidence
- MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE: lines 316-910
- MOVESENS.C:F0270_SENSOR_TriggerLocalEffect: lines 1081-1098
- MOVESENS.C:F0271_SENSOR_ProcessRotationEffect: lines 1100-1152
- MOVESENS.C:F0272_SENSOR_TriggerEffect: lines 1154-1208
- MOVESENS.C:F0275_SENSOR_IsTriggeredByClickOnWall: lines 1309-1551
- MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval: lines 1553-1794

## Closed Gap
movement-triggered local sensor rotation is deferred: F0270 records the latest local rotation request, F0276/F0275 finish the whole sensor iteration, then F0271 rotates the sensor list once and clears the pending global

## Local Gate
- CTest: pass510_dm1_v1_movement_sensor_rotation_defer_source_lock
- Focused tests audited: dm1_v1_sensor_trigger_pc34_compat, dm1_v1_command_movement_sensor_timing_pc34_compat

## Not Claimed
- new runtime sensor-list mutation implementation
- original DOS breakpoint trace
- unsupported sensor effect materialization beyond the existing source-locked trigger metadata

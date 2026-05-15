# Pass559 - DM1 V1 gated movement pending-click queue replay

Status: **PASS559_DM1_V1_GATED_MOVEMENT_PENDING_CLICK_QUEUE_REPLAY_LOCKED**

Scope: source-lock and runtime-guard the F0380 branch where a cooldown-gated movement command remains at the queue head while a pending click is replayed behind it.

## ReDMCSB citations

- COMMAND.C:2045-2829 - COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC
- COMMAND.C:2095-2121 - COMMAND.C:gatedMovementPendingClickReplayBeforeDequeue

## Firestaff guards

- dm1_v1_input_command_queue_pc34_compat.c:286-328 - DM1_V1_InputCommandQueue_ProcessOnePc34Compat
- dm1_v1_input_command_queue_pc34_compat.c:303-318 - gatedMovementReturnsAfterPendingReplay
- test_dm1_v1_command_movement_sensor_timing_pc34_compat.c:pass559 labels - runtimeRegression
- build-pass559/test_dm1_v1_command_movement_sensor_timing_pc34_compat - runtimeExecutable
- dm1V1CommandMovementSensorTimingIntegrationOk=1 - runtimeOutputLastLine

## Gates

- /home/trv2/work/firestaff-worktrees/dm1v1-gated-pending-click-queue-pass559-20260515-bosse/build-pass559/test_dm1_v1_command_movement_sensor_timing_pc34_compat - reported dm1V1CommandMovementSensorTimingIntegrationOk=1

## Not claimed

- new original DOSBox runtime capture
- viewport pixel parity
- new movement engine behavior beyond gated queue replay order

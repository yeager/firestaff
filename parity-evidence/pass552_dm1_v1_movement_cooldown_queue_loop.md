# Pass552 DM1 V1 movement cooldown queue loop

Status: **PASS552_DM1_V1_MOVEMENT_COOLDOWN_QUEUE_LOOP_LOCKED**

Scope: source-lock the loop that decrements movement cooldowns before raw input and F0380 queue processing, then prove a gated movement command stays queued until that gate clears.

## ReDMCSB citations

- GAMELOOP.C:150-219 - GAMELOOP.C:cooldownInputQueueLoop
- COMMAND.C:2045-2829 - COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC
- COMMAND.C:2095-2155 - COMMAND.C:disabledMoveQueueRetention
- CLIKMENU.C:180-351 - CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty
- CLIKMENU.C:272-346 - CLIKMENU.C:successfulStepCooldown

## Firestaff guards

- dm1_v1_input_command_queue_pc34_compat.c:286-328 - queueProcessOne
- dm1_v1_input_command_queue_pc34_compat.c:303-318 - queueRetentionSpan
- dm1_v1_movement_timing_pc34_compat.c:85-95 - cooldownDecrement
- dm1_v1_movement_timing_pc34_compat.c:89-93 - cooldownDecrementSpan
- test_dm1_v1_movement_timing_pc34_compat - regression

## Gates

- /home/trv2/work/firestaff-worktrees/dm1v1-movement-cooldown-queue-loop-pass552-20260515-codex/build-pass552/test_dm1_v1_movement_timing_pc34_compat - reported dm1V1MovementTimingInvariantOk=1

## Not claimed

- new original DOSBox runtime capture
- viewport or wall pixel parity
- new movement engine behavior beyond the cooldown queue loop evidence

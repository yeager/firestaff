# Pass580 - DM1 V1 forward collision/timing source lock

Status: PASS580_DM1_V1_FORWARD_COLLISION_TIMING_LOCKED

## ReDMCSB audit anchors
- GAMELOOP.C:150-215 - GAMELOOP.C:cooldownBeforeInputAndQueue
- COMMAND.C:2045-2829 - COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC
- COMMAND.C:2075-2155 - COMMAND.C:queuedDispatchCooldownGate
- CLIKMENU.C:180-351 - CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty
- CLIKMENU.C:237-345 - CLIKMENU.C:forwardCollisionTiming
- CHAMPION.C:2025-2049 - CHAMPION.C:F0325_CHAMPION_DecrementStamina
- CHAMPION.C:2040-2048 - CHAMPION.C:staminaAndNonStaminaEffects

## Firestaff guards
- dm1_v1_input_command_queue_pc34_compat.c:526-568 - queueProcessOne
- dm1_v1_input_command_queue_pc34_compat.c:543-558 - queueGateSpan
- dm1_v1_input_command_queue_pc34_compat.c:294-321 - queueDiscard
- dm1_v1_input_command_queue_pc34_compat.c:307-320 - queueDiscardSpan
- dm1_v1_movement_command_core_pc34_compat.c:394-590 - movementCommandCore
- dm1_v1_movement_command_core_pc34_compat.c:414-581 - blockedBeforePartyCommit
- dm1_v1_movement_command_core_pc34_compat.c:192-240 - staminaSideEffects
- dm1_v1_action_xp_graphic560_pc34_compat.c:308-352 - f0325ClampPlan
- dm1_v1_movement_timing_pc34_compat.c:66-90 - successfulMovementTiming
- dm1_v1_movement_timing_pc34_compat.c:76-86 - successfulMovementTimingSpan

## Verification
- build/test_dm1_v1_movement_command_core_pc34_compat: dm1V1MovementCommandCoreInvariantOk=1
- build/test_dm1_v1_movement_timing_pc34_compat: dm1V1MovementTimingInvariantOk=1

## Scope guard
- new original DOSBox capture
- viewport/touch/rendering parity
- combat RNG or final wound materialization beyond the F0325/F0366 request seams

# Pass504 - DM1 V1 movement follow-up source lock

Status: PASS504_DM1_V1_MOVEMENT_FOLLOWUP_SOURCE_LOCKED

## ReDMCSB-first audit
- COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC: movement gates keep step commands queued, then dequeue one command and dispatch turns/steps.
- CLIKMENU.C:142-174 / F0365_COMMAND_ProcessTypes1To2_TurnParty: current-square leave/enter movement-result calls around party rotation, with no step cooldown.
- CLIKMENU.C:180-351 / F0366_COMMAND_ProcessTypes3To6_MoveParty: living-champion stamina before target-square legality, wall/door/fakewall/group blockers, blocked-input discard and one PC-34 VBlank, cooldown only after accepted movement.
- MOVESENS.C:316-999 / F0267_MOVE_GetMoveResult_CPSCE: accepted-move globals, scent/last-party-movement timing, teleporter/pit consequences, and source/destination sensor calls.
- MOVESENS.C:1553-1794 / F0276_SENSOR_ProcessThingAdditionOrRemoval: floor-sensor effect dispatcher used by movement-result.

## Firestaff guard
- Local movement seams checked: input command queue, movement command core, movement timing helper, and command/movement/sensor timing tests.
- Current movement blocker check: pass406 is green after building build/test_dm1_v1_movement_core_pc34_compat.

## Scope guard
- DM1 V1 movement only; no viewport/walls, CSB, DM2, Nexus, or pass435 capture files changed.
- Source/evidence lock plus executable gate coverage; no original DOS pixel overlay parity claim.

Manifest: parity-evidence/verification/pass504_dm1_v1_movement_followup_source_lock/manifest.json

# Pass549 - DM1 V1 passable door movement/timing

Status: PASS549_DM1_V1_PASSABLE_DOOR_MOVEMENT_TIMING_LOCKED

## Source audit

- CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty CLIKMENU.C:282-346 checks door state and only blocks states other than open, closed-one-fourth, and destroyed before falling through to F0267 and cooldown assignment.
- DEFS.H DEFS.H:1039-1044 defines the door states used by the condition.

## Firestaff lock

- memory_movement_pc34_compat.c memory_movement_pc34_compat.c:470-488 preserves doorState != 0 && != 1 && != 5 as the only door block branch, so states 1 and 5 fall through to MOVE_OK.
- dm1_v1_movement_command_core_pc34_compat.c dm1_v1_movement_command_core_pc34_compat.c:286-374 takes that MOVE_OK into party position mutation, destination sensors, successful-step timing, input-wait release, and viewport redraw.
- test_dm1_v1_movement_command_core_pc34_compat.c now covers one-fourth and destroyed door front-key commands through accepted movement/timing behavior.

## Boundary

This is deliberately not a pass514/pass388 capture follow-up. It adds no original runtime capture claim and no pixel/door-animation parity claim.

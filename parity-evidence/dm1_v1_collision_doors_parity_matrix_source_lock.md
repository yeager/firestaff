# DM1 V1 collision/doors parity matrix source lock

Status: DM1_V1_COLLISION_DOORS_PARITY_MATRIX_SOURCE_LOCKED

This gate narrows the stale collision/doors parity row from a bare UNPROVEN stub to source/runtime-backed evidence. It keeps the original DOS pixel/content parity blocker explicit.

## ReDMCSB locks

- PASS CLIKMENU.C:135-322 F0366_COMMAND_ProcessTypes3To6_MoveParty - Wall, closed-door, and closed solid-fakewall blockers are resolved before accepted movement reaches F0267.
- PASS CLIKVIEW.C:311-431 F0377_COMMAND_ProcessType80_ClickInDungeonView - Viewport door-button clicks require the source DOOR->Button path and otherwise fall through to wall/front-sensor handling.
- PASS MOVESENS.C:316-774 F0267_MOVE_GetMoveResult_CPSCE - Accepted collision outcomes mutate the party map tuple in the move-result path, not in a renderer-only helper.
- PASS DUNVIEW.C:8318-8610 F0128_DUNGEONVIEW_Draw_CPSF - Collision/door movement state is observable through the same direction/x/y tuple used by the viewport draw.

## Repo evidence

- PASS CMakeLists.txt contains `dm1_v1_movement_command_core_pc34_compat`
- PASS CMakeLists.txt contains `dm1_v1_door_button_click_pc34_compat`
- PASS CMakeLists.txt contains `dm1_v1_wall_collision_runtime_capture`
- PASS CMakeLists.txt contains `dm1_v1_wall_collision_capture_manifest_source_lock`
- PASS tests/test_dm1_v1_movement_command_core_pc34_compat.c contains `pass547 closed door movement blocked`
- PASS tests/test_dm1_v1_door_button_click_pc34_compat.c contains `door with button accepted`
- PASS parity-evidence/dm1_v1_wall_collision_capture_manifest_source_lock.md contains `not an original DOS pixel-parity claim`

## Parity matrix

- row updated: True
- still old UNPROVEN stub: False

## Non-claims

- does not promote original DOS pixel/content parity
- does not remove the need for original-backed overlay cases
- does not change Firestaff movement or viewport runtime behavior

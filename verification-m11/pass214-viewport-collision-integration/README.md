# Pass214 — DM1 V1 viewport/collision integration probe

## Scope

This probe connects the current DM1 V1 movement pipeline to collision queries and the 3D viewport state without creating PNG/PPM evidence.

Primary ReDMCSB source files identified:

- `VIEWPORT.C`: `F0564_VIEWPORT_InitializeBitPlanes`, `F0565_VIEWPORT_SetPalette`, `F0566_VIEWPORT_BlitToScreen` — 224x136 viewport bitplanes and screen blit.
- `DUNVIEW.C`: `F0128_DUNGEONVIEW_Draw_CPSF` — depth-ordered viewport draw, parity flip `(mapX + mapY + direction) & 1`, floor/ceiling dirty handling.
- `DRAWVIEW.C`: `F0097_DUNGEONVIEW_DrawViewport` — viewport-present request/palette bridge.
- `COMMAND.C`: `F0380_COMMAND_ProcessQueue_CPSC` — one queued command per process call, movement-disabled gate.
- `CLIKMENU.C`: `F0365_COMMAND_ProcessTypes1To2_TurnParty`, `F0366_COMMAND_ProcessTypes3To6_MoveParty` — turn/step dispatch; wall, door, fake-wall, group collision gate; cooldown set.
- `MOVESENS.C`: `F0267_MOVE_GetMoveResult_CPSCE`, `F0276_SENSOR_ProcessThingAdditionOrRemoval` — move result, sensor leave/enter, pit/teleporter chains.
- `GAMELOOP.C`: draw cadence and cooldown decrement (`F0128` in the loop, disabled movement ticks at lines 150-155).

Closest current Firestaff modules:

- `dm1_v1_movement_pipeline_pc34_compat.[ch]`
- `dm1_v1_movement_command_core_pc34_compat.[ch]`
- `dm1_v1_collision_door_pc34_compat.[ch]`
- `dm1_v1_viewport_3d_pc34_compat.[ch]`

## Movement-frame synchronization observed

Route output: `viewport_collision_route.tsv` and `viewport_collision_route.json`.

- Accepted step frames set `viewportDirty=1`, call `dm1_viewport_3d_draw_frame`, then `dm1_viewport_3d_present`.
- Accepted turn frames also set `viewportDirty=1` and redraw/present from the rotated direction.
- Blocked collision frames (`closed_door_block`, `group_block`) keep the party state unchanged and do not redraw/present (`viewportDirty=0`).
- Step cooldown is source-faithful: accepted steps set disabled movement ticks; the probe drains them between route commands and records the count in `cooldown_drained` so each command can be tested deterministically.
- Collision query runs before each step and records target cell, door state/passability, and pipeline group blocking.

## Probe artifacts

- `probes/m11/firestaff_dm1_v1_viewport_collision_integration_probe.c`
- `run_firestaff_dm1_v1_viewport_collision_integration_probe.sh`
- `verification-m11/pass214-viewport-collision-integration/probe.log`
- `verification-m11/pass214-viewport-collision-integration/viewport_collision_route.tsv`
- `verification-m11/pass214-viewport-collision-integration/viewport_collision_route.json`

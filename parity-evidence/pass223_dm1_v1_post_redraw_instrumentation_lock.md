# Pass223 — DM1 V1 post-command redraw instrumentation lock

Status: `PASS_SOURCE_LOCKED_POST_REDRAW_INSTRUMENTATION_POINTS`

This is a JSON-only source-locked blocker/instruction gate. It commits no PNG/PPM capture artifacts.

## Required observable chain

- `command_accepted_queue_dispatch`
- `turn_handler_applies_direction_and_releases_wait OR step_handler_resolves_destination_and_releases_wait`
- `move_result_mutates_party_coordinates for step commands`
- `game_loop_draws_mutated_party_state`
- `dungeon_view_consumes_state_before_viewport_request`
- `viewport_requested_then_vertical_blank_returned`
- `viewport_bitmap_blitted_to_screen`

## Instrumentation points

- PASS `command_accepted_queue_dispatch` — `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC`
  - observe: Record accepted command id and queue index immediately after L1160_i_Command is loaded and before the turn/step handler call returns.
- PASS `turn_handler_applies_direction_and_releases_wait` — `CLIKMENU.C:142-173` / `F0365_COMMAND_ProcessTypes1To2_TurnParty`
  - observe: After the turn handler returns, record G0308_i_PartyDirection and G0321_B_StopWaitingForPlayerInput; this is command accepted -> turn state applied.
- PASS `step_handler_resolves_destination_and_releases_wait` — `CLIKMENU.C:180-328` / `F0366_COMMAND_ProcessTypes3To6_MoveParty`
  - observe: After the move-result call returns, record accepted command plus G0306_i_PartyMapX/G0307_i_PartyMapY/G0310_i_DisabledMovementTicks; this is command accepted -> step state applied.
- PASS `move_result_mutates_party_coordinates` — `MOVESENS.C:442-443` / `F0267_MOVE_GetMoveResult_CPSCE`
  - observe: Record destination map X/Y at the coordinate assignment seam for successful party moves.
- PASS `game_loop_draws_mutated_party_state` — `GAMELOOP.C:88-91` / `F0002_MAIN_GameLoop_CPSDF`
  - observe: Record draw-call arguments (direction, mapX, mapY) at the game loop call site after command processing has released the wait loop.
- PASS `dungeon_view_consumes_state_before_viewport_request` — `DUNVIEW.C:8318-8610` / `F0128_DUNGEONVIEW_Draw_CPSF`
  - observe: Record the consumed P0183/P0184/P0185 tuple just before F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW).
- PASS `viewport_requested_then_vertical_blank_returned` — `DRAWVIEW.C:709-722` / `F0097_DUNGEONVIEW_DrawViewport`
  - observe: Record viewport request timestamp/counter before M526_WaitVerticalBlank and a returned-from-vblank counter after it returns.
- PASS `viewport_bitmap_blitted_to_screen` — `DRAWVIEW.C:840-842` / `E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF`
  - observe: Record the vblank blit counter for C007_ZONE_VIEWPORT. This is the viewport-present half of the proof.

## Promotion rule for the next original-runner probe

A movement/turn capture is promotable only if its JSON trace links one accepted command to a strictly later state mutation/draw tuple and then to a strictly later viewport vblank blit. Key delivery, menu churn, or repeated raw frame SHA is not enough.

Non-claims: no source patching, no DOSBox screenshots, no PNG/PPM output, no pixel parity claim.

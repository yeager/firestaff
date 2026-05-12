# Pass224 — DM1 V1 runtime/state probe scaffold

Status: `BLOCKED_MISSING_ORIGINAL_RUNTIME_STATE_HOOK_API`

Scope: JSON-only runtime/state probe contract for the pass223 post-redraw seams. No screenshots, PNGs, or PPMs are produced or committed.

## ReDMCSB audit citations

- PASS `command_accepted_queue_dispatch` — `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC` -> event `command_accepted`
- PASS `turn_handler_applies_direction_and_releases_wait` — `CLIKMENU.C:142-173` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` -> event `turn_state_applied`
- PASS `step_handler_resolves_destination_and_releases_wait` — `CLIKMENU.C:180-328` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` -> event `step_state_applied`
- PASS `move_result_mutates_party_coordinates` — `MOVESENS.C:442-443` / `F0267_MOVE_GetMoveResult_CPSCE` -> event `party_coordinates_mutated`
- PASS `game_loop_draws_mutated_party_state` — `GAMELOOP.C:88-91` / `F0002_MAIN_GameLoop_CPSDF` -> event `game_loop_draw_tuple`
- PASS `dungeon_view_consumes_state_before_viewport_request` — `DUNVIEW.C:8318-8610` / `F0128_DUNGEONVIEW_Draw_CPSF` -> event `dungeon_view_consumed_tuple`
- PASS `viewport_requested_then_vertical_blank_returned` — `DRAWVIEW.C:709-722` / `F0097_DUNGEONVIEW_DrawViewport` -> event `viewport_request_vblank_return`
- PASS `viewport_bitmap_blitted_to_screen` — `DRAWVIEW.C:840-842` / `E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF` -> event `viewport_present`

## Runtime trace chain

- one command_accepted event exists for the route label
- a later turn_state_applied or step_state_applied event references the accepted command sequence
- for step commands, a later party_coordinates_mutated event records the committed destination
- a later game_loop_draw_tuple/dungeon_view_consumed_tuple uses the mutated direction/x/y tuple
- a later viewport_request_vblank_return records G0324_B_DrawViewportRequested before and after M526_WaitVerticalBlank
- a later viewport_present event records the C007_ZONE_VIEWPORT blit

## Runtime API audit

- classification: `blocked/missing-original-runtime-state-hook-api`
- hook capable in committed runner files: `False`
- tools: `{'dosbox': '/usr/bin/dosbox', 'dosbox-debug': '/usr/bin/dosbox-debug', 'dosbox-x': '/usr/bin/dosbox-x', 'gdb': '/usr/bin/gdb', 'xvfb-run': '/usr/bin/xvfb-run', 'xdotool': '/usr/bin/xdotool'}`

## Exact blocker

`pass224_missing_original_runtime_state_hook_api`

A debugger/emulator bridge that binds ReDMCSB source seams to the loaded stock DM.EXE and can emit JSON events for G0432/G0433 command dequeue, G0308/G0306/G0307/G0310/G0321 state mutations, F0128 draw arguments, G0324 viewport-request/vblank, and C007_ZONE_VIEWPORT blit without using PNG/PPM screenshots.

Acceptable implementations:
- DOSBox-X/debugger or dosbox-debug breakpoint script with a symbol/address map from ReDMCSB functions/globals to the loaded DM.EXE image
- a custom DOSBox/Staging frame/runtime hook that logs the listed globals and call-site hits as JSON
- a real-mode debugger/gdbstub bridge that can stop/log at the listed PC addresses and continue without screenshot artifacts

## Existing attempt signal

- attempt: `verification-screens/pass212-n2-state-aware-movement-probe`
- class counts: `{'dungeon_gameplay': 6}`
- duplicate SHA counts: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- first duplicate sha12: `48ed3743ab6a`

Non-claims: no source patching, no DOSBox screenshots, no PNG/PPM output, no pixel parity claim.

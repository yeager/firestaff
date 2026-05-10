# Pass498 — DM1 V1 original post-command state-delta boundary

Status: `PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA`

## Decision

Original DM1 V1 capture is blocked specifically at the post-gameplay, post-command state-delta boundary: the next evidence must connect a real F0380-dequeued movement/turn command through F0365/F0366 to the subsequent F0128 composition and F0097/VIDRV present, and must not collapse to the repeated static 48ed gameplay frame.

## Source path that the next capture must prove

- `GAMELOOP.C:90,164,215-219` / `F0002_MAIN_GameLoop_CPSDF` — A promotable post-command frame must be after the wait loop exits and the next outer-loop F0128 consumes the updated party tuple. ok=`True`
- `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC` — The source-visible state delta must be tied to a real dequeued command, not just host click/key labels. ok=`True`
- `CLIKMENU.C:142-174` / `F0365_COMMAND_ProcessTypes1To2_TurnParty` — Accepted turn commands must prove the F0365 stop-wait write and party-direction mutation before the redraw boundary. ok=`True`
- `CLIKMENU.C:180-347` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` — Accepted move commands must prove the F0366 destination/move-result path, not a repeated static gameplay capture. ok=`True`
- `DUNVIEW.C:8318-8610` / `F0128_DUNGEONVIEW_Draw_CPSF` — The post-command state delta becomes capture-eligible only once F0128 composes G0296 for the updated direction/X/Y tuple. ok=`True`
- `DRAWVIEW.C:709-858` / `F0097_DUNGEONVIEW_DrawViewport` — A screenshot/viewport crop is promotable only at or after the F0097/VIDRV present boundary after the matching F0128. ok=`True`

## Current blocker evidence

- pass487 status: `PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED`
- pass495 status: `PASS495_F0365_F0366_RUNTIME_STOPS_PROVEN_ORIGINAL_STATIC_CAPTURE_STILL_BLOCKED`
- pass497 status: `PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED`
- post-entry gameplay hash repeated: `True` / `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`
- post-entry region stats repeated: `True`
- filename/route-label drift rows: `5`
- F0365/F0366 runtime stop chain previously closed: `True`

## Promotion predicate for the next run

- post-gameplay command observed through F0380 pop/load/decrement
- matching F0365 turn or F0366 move handler observed after that command
- G0321 stop-wait transition and game-time tick allow the wait loop to exit
- later F0128 consumes the resulting direction/map-X/map-Y tuple
- F0097/VIDRV present boundary is reached for the same shot
- raw viewport/fullframe hash or region fingerprint differs from the repeated 48ed static gameplay frame, unless the command is explicitly proven blocked/no-op by source state

## Reject as non-promotable

- host click/key route labels without F0380/F0365/F0366 evidence
- BPLIST or setup echoes without strict post-Running stops
- repeated 48ed static gameplay captures after entry
- filenames whose labels drift from the route action
- F0128/F0097 address bindings without a command-state predecessor

## Gate

- `python3 tools/verify_pass498_dm1_v1_original_post_command_state_delta_boundary.py`

Manifest: `parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json`

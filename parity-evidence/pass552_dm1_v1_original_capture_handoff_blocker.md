# Pass552 - DM1 V1 original capture handoff blocker

Status: BLOCKED_PASS552_FRESH_CAPTURE_HANDOFF_STOPS_AT_EMPTY_F0380

## Decision

Fresh original capture handoff is blocked before pass548 overlay capture: pass388 proves the armed route reaches F0380, but it does not prove F0361 entry, G0432/G2153 enqueue, positive queue count before F0380 pop, or dispatch. pass548 therefore correctly remains non-promotable while its bounded live capture times out/no pass475 manifest is available.

## ReDMCSB source audit

- IO2.C:27-61 / F0540_INPUT_Crawcin - i34e_raw_key_normalization
- COMMAND.C:636-685 / G0459_as_Graphic561_SecondaryKeyboardInput_Movement - i34e_movement_keyboard_table
- COMMAND.C:1709-1813 / F0361_COMMAND_ProcessKeyPress - f0361_keyboard_enqueue
- GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF - gameloop_keyboard_before_f0380
- COMMAND.C:2045-2156 / F0380_COMMAND_ProcessQueue_CPSC - f0380_pop_dispatch
- CLIKMENU.C:142-347 / F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty - turn_step_party_handlers
- MOVESENS.C:316-450 / F0267_MOVE_GetMoveResult_CPSCE - successful_step_tuple_commit
- DUNVIEW.C:8318-8611 / F0128_DUNGEONVIEW_Draw_CPSF - draw_to_present_boundary
- DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport - pc34_viewport_present

## Runtime artifact inputs

- pass388: parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/manifest.json - BLOCKED_PASS388_B_ORIGINAL_ROUTE_NEVER_HITS_ENQUEUE_BRANCH
- pass514: parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json - BLOCKED_PASS514_KEYBOARD_INPUT_DELIVERED_BUT_NO_F0361_ENQUEUE_BEFORE_EMPTY_F0380
- pass548: parity-evidence/verification/pass548_dm1_v1_original_overlay_capture_progress/manifest.json - BLOCKED_PASS548_LIVE_CAPTURE_TIMEOUT

## Promotion rule

Do not promote pass548/pass475 overlay pixels until one fresh runtime transcript observes the F0540/M528 key value, F0361 queue slot/count write, F0380 positive pop/decrement/dispatch, party tuple or typed blocker, F0128, and F0097/VIDRV boundary in order.

## Evidence

- Manifest: parity-evidence/verification/pass552_dm1_v1_original_capture_handoff_blocker/manifest.json

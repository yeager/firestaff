# Pass513 - DM1 V1 I34E route-key transcript contract

Status: BLOCKED_PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_REQUIRED

## Decision

The remaining blocker is not another Firestaff movement implementation patch. It is a missing original PC/I34E route-key transcript with enough source-visible state to bind keyboard-buffer token, command queue delta, F0380 pop/dispatch, party tuple delta, and viewport present boundary.

## ReDMCSB source audit

- PASS IO2.C:27-61 / F0540_INPUT_Crawcin - the transcript must record the concrete M528/F0540 value after PC/I34E arrow normalization, not a host-side route label
- PASS COMMAND.C:636-685 / G0459_as_Graphic561_SecondaryKeyboardInput_Movement - F0361 can queue movement only after the drained key matches the active I34E secondary keyboard table
- PASS GAMELOOP.C:164-219 / F0002_MAIN_GameLoop_CPSDF - the original route chain is keyboard-buffer drain, queue processing, then wait-loop boundary
- PASS COMMAND.C:1734-1812 / F0361_COMMAND_ProcessKeyPress - a transcript must include F0361's table match, queue slot write, last-index write, and G2153 increment
- PASS COMMAND.C:2075-2127,2150-2156 / F0380_COMMAND_ProcessQueue_CPSC - the same command must be observed leaving F0380 with first-index/count delta and turn/move dispatch
- PASS CLIKMENU.C:142-174,237-270,293-347 / F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty - post-dispatch evidence must distinguish turn mutation, blocked-step discard, and successful-step movement/cooldown side effects
- PASS MOVESENS.C:738-818 / F0267_MOVE_GetMoveResult_CPSCE - successful movement transcripts must bind the source-committed tuple and timing/sensor side effects
- PASS DUNVIEW.C:8318-8611 / F0128_DUNGEONVIEW_Draw_CPSF - viewport/HUD captures are promotable only after F0128 consumes the post-command direction/X/Y tuple
- PASS DRAWVIEW.C:709-858 / F0097_DUNGEONVIEW_DrawViewport - route-labeled screenshots must be tied to the PC34 viewport present/blit boundary

## Required prior gates

- PASS pass504_keyboard_buffer_state_delta_blocker: PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED
- PASS pass509_original_overlay_keyboard_buffer_blocker: PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED
- PASS pass511_movement_original_route_contract: PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED
- PASS pass512_movement_cross_reference_audit: PASS512_DM1_V1_MOVEMENT_CROSS_REFERENCE_AUDIT

## Accepted I34E key rows

- 0x004B (K) -> C001_COMMAND_TURN_LEFT -> F0365
- 0x004C (L) -> C003_COMMAND_MOVE_FORWARD -> F0366
- 0x004D (M) -> C002_COMMAND_TURN_RIGHT -> F0365
- 0x004F (O) -> C006_COMMAND_MOVE_LEFT -> F0366
- 0x0050 (P) -> C005_COMMAND_MOVE_BACKWARD -> F0366
- 0x0051 (Q) -> C004_COMMAND_MOVE_RIGHT -> F0366

## Required transcript fields

- routeId
- sampleIndex
- inputSource
- rawKeyCode
- normalizedKeyCode
- m527WasNonEmpty
- m528Value
- f0361Table
- f0361Command
- f0361QueueSlot
- g0434Before
- g0434After
- g2153BeforeEnqueue
- g2153AfterEnqueue
- f0380Command
- g0433Before
- g0433After
- g2153BeforePop
- g2153AfterPop
- dispatchHandler
- partyBeforeMap
- partyBeforeX
- partyBeforeY
- partyBeforeDir
- partyAfterMap
- partyAfterX
- partyAfterY
- partyAfterDir
- blockedOrNoopReason
- f0128Direction
- f0128MapX
- f0128MapY
- f0097Presented
- capturePath
- captureSha256

## Machine-checkable transcript validation

- Optional promotion gate: FIRESTAFF_PASS513_TRANSCRIPT=path/to/transcript.json python3 tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py
- Provided: False
- Validation status: not_provided
- Minimum turnRows: 1
- Minimum successfulStepRows: 1
- Minimum blockedOrNoopRows: 1
- Binding: M527 non-empty before M528 read
- Binding: M528/F0540 normalized value equals a COMMAND.C I34E movement table code
- Binding: F0361 writes that command into G0432 and increments G2153
- Binding: F0380 pops the same command and decrements G2153
- Binding: F0365 or F0366 is reached for that command
- Binding: F0128/F0097 consumes and presents the matching post-command party tuple before capture
- Binding: capturePath exists and captureSha256 matches the captured bytes

## Reject as non-promotable

- route labels without rawKeyCode plus normalizedKeyCode plus M528 value
- F0361 entry/exit records without G0432 slot, G0434 delta, and G2153 increment
- F0380 records where G2153 is zero, command is gated by movement cooldown, or no matching pop/decrement occurs
- state-delta screenshots lacking the preceding F0365/F0366 handler and F0128/F0097 boundary
- repeated capture hashes unless the transcript proves a source-owned blocked/no-op route

## Candidate transcript-like artifacts

- parity-evidence/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof.md size=1401
- parity-evidence/pass284_dm1_v1_f0380_dequeue_ordering_proof.md size=3103
- parity-evidence/pass289_dm1_v1_f0380_dispatch_equivalent_proof.md size=4305
- parity-evidence/pass293_dm1_v1_direct_f0380_hook_address_window.md size=2058
- parity-evidence/pass296_dm1_v1_input_tuple_proof_without_direct_f0380.md size=3975
- parity-evidence/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.md size=1350
- parity-evidence/pass317_dm1_v1_f0380_f0097_direct_probe_attempt.md size=1351
- parity-evidence/pass342_dm1_v1_redmcsb_f0361_f0380_binding_map.md size=6531
- parity-evidence/pass381_dm1_v1_route_f0380_transition_source_path.md size=1554
- parity-evidence/pass384_dm1_v1_f0380_runtime_breakpoint_chain.md size=1343
- parity-evidence/pass387_dm1_v1_f0380_queue_pop_eligibility.md size=2792
- parity-evidence/pass504_dm1_v1_keyboard_buffer_state_delta_blocker.md size=2441
- parity-evidence/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.md size=3153
- parity-evidence/pass514_dm1_v1_i34e_runtime_transcript_capture_path.md size=1004
- parity-evidence/pass93_original_route_key_explore_diagnostic.md size=3781
- parity-evidence/verification/pass232_command_f0380_source_seam.json size=3267
- parity-evidence/verification/pass275_dm1_v1_debugger_bpm_runtime_hook_attempt/route_keylog.txt size=305
- parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/dosbox_debug_noise_reduced.clean.txt size=500000
- parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json size=31833
- parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/route_noise_reduced_keylog.json size=4311
- parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/seed_MEMDUMP.TXT size=480
- parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof/manifest.json size=5569
- parity-evidence/verification/pass289_dm1_v1_f0380_dispatch_equivalent_proof/manifest.json size=6346
- parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/dosbox_debug_candidate_window.clean.txt size=700000
- parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/live_code_window_dump.clean.txt size=3235
- parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/manifest.json size=14931
- parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/route_candidate_window_keylog.json size=3931
- parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json size=8377
- parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.json size=7580
- parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt/dosbox_debug_noise_reduced.clean.txt size=223872
- parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt/manifest.json size=16880
- parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt/route_noise_reduced_keylog.json size=4316
- parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt.json size=7623
- parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/dosbox_debug_noise_reduced.clean.txt size=207048
- parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/manifest.json size=15654
- parity-evidence/verification/pass317_dm1_v1_f0380_f0097_direct_probe_attempt/route_noise_reduced_keylog.json size=4310
- parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/route_keylog.json size=3939
- parity-evidence/verification/pass327_dm1_v1_non_tmux_runtime_evidence_fallback/non_tmux_route_keylog.json size=1090
- parity-evidence/verification/pass329_dm1_v1_direct_pty_breakpoint_arming_timing/arm_at_stable_load_menu_prompt_route_keylog.json size=2077
- parity-evidence/verification/pass329_dm1_v1_direct_pty_breakpoint_arming_timing/pre_arm_before_route_route_keylog.json size=2079
- parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/pre_arm_before_route_route_keylog.json size=2080
- parity-evidence/verification/pass331_dm1_v1_route_to_viewport_redraw_path/probe_route_keylog.json size=2078
- parity-evidence/verification/pass333_dm1_v1_keypad_mode_command_queue_probe/probe_route_keylog.json size=2262
- parity-evidence/verification/pass335_dm1_v1_keyboard_table_route_readiness/probe_route_keylog.json size=1733
- parity-evidence/verification/pass342_dm1_v1_redmcsb_f0361_f0380_binding_map/manifest.json size=2275
- parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/post_load_arm_before_route_route_keylog.json size=2080
- parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/pass379_route_keylog.json size=2077
- parity-evidence/verification/pass381_dm1_v1_route_f0380_transition_source_path/manifest.json size=5157
- parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/manifest.json size=4796
- parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_command_log.json size=2907
- parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_route_keylog.json size=2300
- parity-evidence/verification/pass384_dm1_v1_f0380_runtime_breakpoint_chain/pass384_runtime.clean.txt size=8556
- parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/pass385_route_keylog.json size=2298
- parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/pass386_click_route_keylog.json size=1903
- parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/pass386_keyboard_route_keylog.json size=1688
- parity-evidence/verification/pass387_dm1_v1_f0380_queue_pop_eligibility/manifest.json size=1937
- parity-evidence/verification/pass387_keyboard_f0361_queue_write/pass387_keyboard_route_keylog.json size=1643
- parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/pass388_click_route_keylog.json size=1905
- parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/pass388_keyboard_route_keylog.json size=1690
- parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/pass391_route_keylog.json size=1951
- parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker/manifest.json size=5057
- parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker/manifest.json size=10565
- parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json size=8206
- verification-screens/pass112-n2-stable-hud-route/original-viewpoint-route-keys.log size=586
- verification-screens/pass209-delayed-click-zone-route/original_viewport_route_keys.swift size=5604
- verification-screens/pass209-delayed-click-zone-route/original_viewport_route_keys_xdotool.sh size=3664
- verification-screens/pass210-n2-original-movement-route-fresh/original_viewport_route_keys.swift size=5604
- verification-screens/pass210-n2-original-movement-route-fresh/original_viewport_route_keys_xdotool.sh size=3664
- verification-screens/pass304-original-pc34-wall-comparator-batch-A/original_viewport_route_keys.swift size=5604
- verification-screens/pass304-original-pc34-wall-comparator-batch-A/original_viewport_route_keys_xdotool.sh size=3664
- verification-screens/pass376-original-route/original-viewpoint-route-keys.log size=796
- verification-screens/pass376-original-route/original_viewport_route_keys.swift size=5604
- verification-screens/pass376-original-route/original_viewport_route_keys_xdotool.sh size=3664
- verification-screens/pass378-source-portrait-sixshot-retry/original-viewpoint-route-keys.log size=616
- verification-screens/pass378-source-portrait-sixshot-retry/original_viewport_route_keys.swift size=5604
- verification-screens/pass378-source-portrait-sixshot-retry/original_viewport_route_keys_xdotool.sh size=3664
- verification-screens/pass487-n2-original-pc34-click-primitives-route/original_viewport_route_keys.swift size=6062
- verification-screens/pass487-n2-original-pc34-click-primitives-route/original_viewport_route_keys_xdotool.sh size=3995
- verification-screens/pass505-original-overlay-mouse-route-recapture/original-viewpoint-route-keys.log size=873
- verification-screens/pass505-original-overlay-mouse-route-recapture/original_viewport_route_keys.swift size=6062
- verification-screens/pass505-original-overlay-mouse-route-recapture/original_viewport_route_keys_xdotool.sh size=3995
- verification-screens/pass94-hall-map-enter-diagnostic/original_viewport_route_keys.swift size=5604
- verification-screens/pass94-hall-map-enter-diagnostic/original_viewport_route_keys_xdotool.sh size=3664

## Non-claims

- no DOSBox/FIRES/original runtime capture was launched by this verifier
- no original-vs-Firestaff pixel parity is claimed
- no runtime movement code is changed
- candidate transcript-like files are listed for triage only and are not promoted

## Gate

- python3 tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py

Manifest: parity-evidence/verification/pass513_dm1_v1_i34e_route_key_transcript_contract/manifest.json

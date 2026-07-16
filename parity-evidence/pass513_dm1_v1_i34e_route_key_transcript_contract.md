# Pass513 - DM1 V1 I34E route-key transcript contract

Status: FAIL_PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_CONTRACT

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

- FAIL pass504_keyboard_buffer_state_delta_blocker: None
- FAIL pass509_original_overlay_keyboard_buffer_blocker: FAIL_PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER
- FAIL pass511_movement_original_route_contract: None
- FAIL pass512_movement_cross_reference_audit: None

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

- parity-evidence/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.md size=3446
- parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker/manifest.json size=9794
- verification-screens/pass1052-dm1-original-route-24h-turncycle/original_viewport_route_keys.swift size=6062
- verification-screens/pass1052-dm1-original-route-24h-turncycle/original_viewport_route_keys_xdotool.sh size=3995
- verification-screens/pass1052-dm1-original-route-24h-turncycle/pass513_i34e_route_key_transcript_scaffold.json size=8229
- verification-screens/pass1053-dm1-original-champion-candidate-panel/original-viewpoint-route-keys.log size=609
- verification-screens/pass1055-dm1-original-closed-door-collision/original-viewpoint-route-keys.log size=1940
- verification-screens/pass1055-dm1-original-closed-door-collision/original_viewport_route_keys.swift size=6062
- verification-screens/pass1055-dm1-original-closed-door-collision/original_viewport_route_keys_xdotool.sh size=3995
- verification-screens/pass1055-dm1-original-closed-door-collision/pass513_i34e_route_key_transcript_scaffold.json size=3990
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
- verification-screens/pass513-dm1-v1-promoted-transcript/promoted_transcript.json size=7764
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

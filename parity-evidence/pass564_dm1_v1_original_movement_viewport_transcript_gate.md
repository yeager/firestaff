# Pass564 - DM1 V1 original movement viewport transcript gate

- Status: BLOCKED_PASS564_RUNTIME_TRANSCRIPT_CHAIN_MISSING
- Manifest: parity-evidence/verification/pass564_dm1_v1_original_movement_viewport_transcript_gate/manifest.json

## Decision

Original DM1 V1 movement/viewport/wall capture remains blocked until one bounded canonical PC34 run records C254 slot decode, IO2 movement key, F0361 enqueue, F0380 pop, F0365/F0366 dispatch, party tuple mutation or intentional blocked-step proof, then F0128 draw and F0097 viewport present for the same route token.

## Required runtime event order
- c254_vector_decoded
- io_driver_slot0_f8090_slot1_f8091
- io2_f0540_movement_key
- f0361_enqueue_g0432_g2153_increment
- f0380_pop_g2153_decrement
- f0365_or_f0366_dispatch
- party_tuple_mutated_or_confirmed_blocked
- f0128_draw_tuple
- f0097_viewport_present

## ReDMCSB locks
- PASS GAMELOOP.C:90-219 F0002_MAIN_GameLoop_CPSDF - The runtime transcript must bind input drain and F0380 dispatch to the next loop draw that consumes G0308/G0306/G0307.
- PASS IO2.C:37-184 F0540_INPUT_Crawcin / F0539_INPUT_Cconis - A movement key is source-visible only after C254 slot0/slot1 are decoded and IO2 returns the normalized movement code.
- PASS COMMAND.C:1086-1765 F0361_COMMAND_ProcessKeyPress - The transcript must show the IO2 key reaches F0361 and writes G0432/G2153.
- PASS COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC - The transcript must show a non-empty queue pop and source dispatch before any viewport capture is promoted.
- PASS CLIKMENU.C:135-274 F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty - Movement/turn parity requires source-visible party tuple mutation, not only queued input.
- PASS DUNVIEW.C:2101-8610 F0128_DUNGEONVIEW_Draw_CPSF - A promoted original frame must be after F0128 composes the viewport for the mutated tuple and hands it to F0097.
- PASS DRAWVIEW.C:709-859 F0097_DUNGEONVIEW_DrawViewport - Viewport/wall capture needs the present-side F0097 boundary, not only F0128 entry.

## Inputs

- pass563: BLOCKED_PASS563_PC34_C254_CHAIN_INCOMPLETE
- runtime transcript: not_provided

## Non-claims
- no DOSBox runtime transcript is fabricated by this gate
- no original screenshot or pixel parity is promoted without the required transcript
- no Firestaff movement or viewport runtime behavior is changed
- no DANNESBURK input touched
- no push

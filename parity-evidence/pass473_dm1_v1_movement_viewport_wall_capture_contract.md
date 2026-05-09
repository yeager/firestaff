# Pass473 — DM1 V1 movement/viewport/wall capture contract

Status: PASS473_SOURCE_STOP_CLICK_PRIMITIVE_CAPTURE_CONTRACT_READY

Scope: source-stop and click-primitive gate only. No DOSBox run and no pixel parity claim.

## Next promotable click primitives
- turn_left C001 center=[247, 135] box=[234, 261, 125, 145]
- move_forward C003 center=[276, 135] box=[263, 289, 125, 145]
- turn_right C002 center=[304, 135] box=[291, 318, 125, 145]
- move_left C006 center=[247, 157] box=[234, 261, 147, 167]
- move_backward C005 center=[276, 157] box=[263, 289, 147, 167]
- move_right C004 center=[304, 157] box=[291, 318, 147, 167]
- hall_portrait_c080 C080 center=[111, 82] box=[0, 223, 33, 168]

## Source-stop contract
- initial_tuple — fresh Hall/map tuple decoded from canonical DUNGEON.DAT before movement
- post_turn — after CLIKMENU.C F0365 mutates G0308_i_PartyDirection and sensors are processed
- post_step — after CLIKMENU.C F0366 calls F0267 and sets G0310_i_DisabledMovementTicks
- post_viewport_present — after DUNVIEW.C F0128 draws G0296 for tuple and DRAWVIEW.C F0097 blits C007_ZONE_VIEWPORT
- wall_comparator_input — use only fresh 224x136 viewport crops from post_viewport_present with locked data hashes; pass376/pass304 frames remain review-only unless recaptured under this contract

## ReDMCSB locks
- COMMAND.C:106-114 — PC34 live capture can use absolute left-click centers for movement and viewport commands; arbitrary xdotool points are not evidence. ok=True
- COMMAND.C:397-403 — I34E source dispatch maps mouse input through the same movement/viewport command space. ok=True
- COMMAND.C:2045-2156 — Capture labels are source-valid only after the queued command reaches turn/step handlers, not at click/highlight time. ok=True
- CLIKMENU.C:142-174 — Turn captures must stop after source direction mutation and sensor re-entry, before the next label is accepted. ok=True
- CLIKMENU.C:180-347 — Step captures must stop after legality, move-result, and cooldown mutation; blocked moves are a separate non-movement label. ok=True
- DUNGEON.C:1371-1392 — Movement/viewport labels must bind to source relative-step vectors, not emulator visual guesses. ok=True
- DUNVIEW.C:2962-3110 — Wall evidence must be captured after the viewport base is cleared and wall/door bitmaps are composited into G0296. ok=True
- DUNVIEW.C:8318-8611 — The comparable viewport is the F0128 draw for the mutated direction/X/Y tuple followed by presentation. ok=True
- DRAWVIEW.C:709-858 — The 224x136 crop must be taken after the PC34 viewport present seam, not from setup echo or pre-blit bitmap artifacts. ok=True

## Prior evidence used as blockers/context only
- pass449 parity-evidence/verification/pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate/hall_candidate_framebuffer_compare.json exists=True status=BLOCKED_FRAMEBUFFER_ARTIFACTS_OR_MANIFEST_MISSING sha256=8632df2b810d8b14ea077f77b6157764154bbd52c1ea6ed1d3bf6a42602c133c
- pass450 parity-evidence/verification/pass450_dm1_v1_hall_completion_audit_matrix/manifest.json exists=True status=PASS_WITH_BLOCKERS sha256=6e615f388cfe68bf31c07d9d12f53a8a53ed69ebb157c6b2bf4ebe59ba041576
- pass451 parity-evidence/verification/pass451_dm1_v1_hall_original_capture_storage_policy/manifest.json exists=True status=PASS_PASS451_EXTERNAL_CAPTURE_STORAGE_POLICY_READY sha256=12416ea1703ecc4d02b77f1eb39a870326fc3f02bc69280ffc084d755684b549
- pass466 parity-evidence/verification/pass466_dm1_v1_initial_hall_c080_source_stop_capture_path/manifest.json exists=True status=PASS466_SOURCE_STOP_CAPTURE_PATH_LOCKED_TERMINAL_HUD_ROWS_READY_FOR_RECAPTURE sha256=54a9227355e5237300ffceb8c9c9a3e6b4527cac5992255df080dabf2b5a33ec
- pass472_inputs parity-evidence/verification/pass451_dm1_v1_hall_original_capture_storage_policy/manifest.json exists=True status=PASS_PASS451_EXTERNAL_CAPTURE_STORAGE_POLICY_READY sha256=12416ea1703ecc4d02b77f1eb39a870326fc3f02bc69280ffc084d755684b549

## Narrowed decision
The next live N2 capture must recapture movement/viewport/wall frames using the locked PC34 click centers above, then label frames only after F0365/F0366 state mutation and F0128→F0097 viewport presentation. Existing pass304/pass376/pass449/pass450/pass451/pass466/pass472 artifacts remain context or readiness evidence, not parity inputs for this gate.

## Non-claims
No live original recapture, no Firestaff-vs-original pixel parity, and no promotion of stale viewport/wall frames is claimed.
